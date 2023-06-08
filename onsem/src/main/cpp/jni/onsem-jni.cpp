#include "onsem-jni.h"
#include "jobjectstocpptypes.hpp"
#include <regex>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <set>
#include <memory>
#include <jni.h>
#include <onsem/common/keytostreams.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticresourcegrounding.hpp>
#include <onsem/texttosemantic/linguisticanalyzer.hpp>
#include <onsem/semantictotext/type/naturallanguageexpression.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticmemory/semantictracker.hpp>
#include "onsem/semantictotext/semanticmemory/links/expressionwithlinks.hpp"
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/triggers.hpp>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/outputter/outputtercontext.hpp>
#include <onsem/semantictotext/outputter/executiondataoutputter.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include "semanticenumsindexes.hpp"
#include "androidlog.hpp"
#include "textprocessingcontext-jni.hpp"
#include "linguisticdatabase-jni.hpp"
#include "semanticmemory-jni.hpp"
#include "semanticexpression-jni.hpp"

using namespace onsem;


namespace {
    /// Any JNI function that will deal with references maintained across the JNI should lock this mutex.
    std::mutex _jniReferencesMutex;
    std::map<jint, std::shared_ptr<ExpressionWithLinks>> _idToExpWrapperForMemory;


    struct JiniOutputter : public ExecutionDataOutputter {
        JiniOutputter(SemanticMemory &pSemanticMemory,
                      const linguistics::LinguisticDatabase &pLingDb,
                      JNIEnv *env,
                      jobject jOutputter,
                      bool pInformAboutWhatWasDone)
                : ExecutionDataOutputter(pSemanticMemory, pLingDb),
                  _env(env),
                  _jiniOutputterClass(env->FindClass("com/onsem/JiniOutputter")),
                  _jOutputter(jOutputter),
                  _informAboutWhatWasDone(pInformAboutWhatWasDone) {
        }

        ~JiniOutputter() override = default;

        void _exposeText(const std::string& pText,
                         SemanticLanguageEnum pLanguage) override
        {
            jmethodID exposeTextFun = _env->GetMethodID(_jiniOutputterClass, "exposeText",
                                                        "(Ljava/lang/String;)V");
            _env->CallVoidMethod(_jOutputter, exposeTextFun, _env->NewStringUTF(pText.c_str()));
            if (_informAboutWhatWasDone)
                ExecutionDataOutputter::_exposeText(pText, pLanguage);
        }

        void _exposeResource(const SemanticResource& pResource,
                             const std::map<std::string, std::vector<std::string>>& pParameters) override
        {
            jmethodID exposeResourceFun = _env->GetMethodID(_jiniOutputterClass, "exposeResource",
                                                            "(Ljava/lang/String;Ljava/lang/String;Ljava/util/Map;)V");
            _env->CallVoidMethod(_jOutputter, exposeResourceFun,
                                 _env->NewStringUTF(pResource.label.c_str()),
                                 _env->NewStringUTF(pResource.value.c_str()),
                                 stlStringVectorStringMapToJavaHashMap(_env, pParameters));
            if (_informAboutWhatWasDone)
                ExecutionDataOutputter::_exposeResource(pResource, pParameters);
        }

        void _beginOfScope(Link pLink) override
        {
            jmethodID beginOfScopeFun = _env->GetMethodID(_jiniOutputterClass, "beginOfScope",
                                                          "(Ljava/lang/String;)V");

            std::string linkStr;
            switch (pLink)
            {
                case onsem::VirtualOutputter::Link::AND:
                    linkStr = "AND";
                    break;
                case onsem::VirtualOutputter::Link::THEN:
                    linkStr = "THEN";
                    break;
                case onsem::VirtualOutputter::Link::THEN_REVERSED:
                    linkStr = "THEN_REVERSED";
                    break;
                case onsem::VirtualOutputter::Link::IN_BACKGROUND:
                    linkStr = "IN_BACKGROUND";
                    break;
            }
            _env->CallVoidMethod(_jOutputter, beginOfScopeFun,
                                 _env->NewStringUTF(linkStr.c_str()));
        }

        void _endOfScope() override
        {
            jmethodID endOfScopeFun = _env->GetMethodID(_jiniOutputterClass, "endOfScope",
                                                        "()V");
            _env->CallVoidMethod(_jOutputter, endOfScopeFun);
        }

        void _resourceNbOfTimes(int pNumberOfTimes) override
        {
            jmethodID resourceNbOfTimesFun = _env->GetMethodID(_jiniOutputterClass, "resourceNbOfTimes",
                                                               "(I)V");
            _env->CallVoidMethod(_jOutputter, resourceNbOfTimesFun, pNumberOfTimes);
        }

        void _insideScopeNbOfTimes(int pNumberOfTimes) override
        {
            jmethodID insideScopeNbOfTimesFun = _env->GetMethodID(_jiniOutputterClass, "insideScopeNbOfTimes",
                                                                  "(I)V");
            _env->CallVoidMethod(_jOutputter, insideScopeNbOfTimesFun, pNumberOfTimes);
        }


    private:
        JNIEnv *_env;
        jclass _jiniOutputterClass;
        jobject _jOutputter;
        bool _informAboutWhatWasDone;
    };

}


void runOutputter(
        JNIEnv *env,
        SemanticLanguageEnum pLanguage,
        SemanticMemory& pSemMemory,
        linguistics::LinguisticDatabase& pLingDb,
        const SemanticExpression& pSemExp,
        jobject jOutputter,
        bool pInformAboutWhatWasDone,
        const SemanticExpression* pInputSemExpPtr) {
    auto outContext = TextProcessingContext::getTextProcessingContextFromRobot(pLanguage);
    OutputterContext outputterContext(outContext);
    outputterContext.inputSemExpPtr = pInputSemExpPtr;
    std::string answer;
    JiniOutputter outputter(pSemMemory, pLingDb, env, jOutputter, pInformAboutWhatWasDone);
    outputter.processSemExp(pSemExp, outputterContext);
    if (pInformAboutWhatWasDone)
        outputter.rootExecutionData.run(pSemMemory, pLingDb);
}


void
convertCppExceptionsToJavaExceptions(JNIEnv *env, const std::function<void()> &pFunction) {
    try {
        pFunction();
    } catch (const std::exception &e) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"), e.what());
    }
}

void protectByMutex(const std::function<void()> &pFunction) {
    std::lock_guard<std::mutex> lock(_jniReferencesMutex);
    pFunction();
}

template<typename T>
T protectByMutexWithReturn(const std::function<T()> &pFunction) {
    std::lock_guard<std::mutex> lock(_jniReferencesMutex);
    return pFunction();
}


template jobject protectByMutexWithReturn<jobject>(const std::function<jobject()> &pFunction);

template jstring protectByMutexWithReturn<jstring>(const std::function<jstring()> &pFunction);

template jint protectByMutexWithReturn<jint>(const std::function<jint()> &pFunction);

template jobjectArray protectByMutexWithReturn<jobjectArray>(const std::function<jobjectArray()> &pFunction);


jobject newExpressionWithLinks(
        JNIEnv *env,
        const std::shared_ptr<ExpressionWithLinks> &pExp) {
    if (!pExp)
        throw std::runtime_error("the ExpressionWrapperForMemory is empty");
    jint newKey = findMissingKey(_idToExpWrapperForMemory);
    _idToExpWrapperForMemory.emplace(newKey, pExp);
    jclass expressionWrapperForMemoryClass = env->FindClass(
            "com/onsem/ExpressionWithLinks");
    jmethodID expressionWrapperForMemoryConstructor =
            env->GetMethodID(expressionWrapperForMemoryClass, "<init>", "(I)V");
    return env->NewObject(expressionWrapperForMemoryClass, expressionWrapperForMemoryConstructor,
                          newKey);
}


jint JNI_OnLoad(JavaVM* /*vm*/, void* /*reserved*/) {
#ifdef COUT_TO_ANDROID_LOG
    // Also initialize the forwarding of logs to Android.
    std::cout.rdbuf(new forward_to_android);
    // TODO: write this in a way that does not leak and does not conflict with other libraries
#endif // COUT_TO_ANDROID_LOG
    return JNI_VERSION_1_6;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_OnsemKt_deleteExpressionWithLinks(
        JNIEnv *env, jclass /*clazz*/, jint expressionWrapperForMemoryId) {
    convertCppExceptionsToJavaExceptions(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto it = _idToExpWrapperForMemory.find(expressionWrapperForMemoryId);
        // The object can be already deleted if it was used to uninform (because after that call the object is not usable anymore)
        if (it != _idToExpWrapperForMemory.end())
            _idToExpWrapperForMemory.erase(it);
    });
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_onsem_OnsemKt_isAProperNoun(
        JNIEnv *env, jclass /*clazz*/, jstring jtext, jint linguisticDatabaseId) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jboolean>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto text = toString(env, jtext);
        auto &lingDb = getLingDb(linguisticDatabaseId);
        return linguistics::isAProperNoun(text, lingDb);
    }, false);
}


extern "C"
JNIEXPORT jobject JNICALL
Java_com_onsem_OnsemKt_inform(
        JNIEnv *env, jclass /*clazz*/,
        jobject semanticExpressionJObj,
        jobject locale,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj,
        jobject jOutputter,
        jboolean informAboutWhatWasDone) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jobject>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
        auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
        auto &semExp = getSemExp(env, semanticExpressionJObj);

        std::list<UniqueSemanticExpression> reactions;
        auto connection = semanticMemory.memBloc.actionProposalSignal.connectUnsafe([&](UniqueSemanticExpression& pUSemExp) {
            reactions.emplace_back(pUSemExp->clone());
        });

        auto res = newExpressionWithLinks(
                env,
                memoryOperation::inform(
                        semExp->clone(),
                        semanticMemory, lingDb));

        semanticMemory.memBloc.actionProposalSignal.disconnectUnsafe(connection);
        for (auto& currReaction : reactions) {
            auto language = toLanguage(env, locale);
            runOutputter(env, language, semanticMemory, lingDb, *currReaction, jOutputter,
                         informAboutWhatWasDone, &*semExp);
        }
        return res;
    }, nullptr);
}


extern "C"
JNIEXPORT jobject JNICALL
Java_com_onsem_OnsemKt_informAxiom(
        JNIEnv *env, jclass /*clazz*/,
        jobject semanticExpressionJObj,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jobject>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
        auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
        auto &semExp = getSemExp(env, semanticExpressionJObj);
        return newExpressionWithLinks(
                env,
                memoryOperation::informAxiom(
                        semExp->clone(),
                        semanticMemory, lingDb));
    }, nullptr);
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_onsem_OnsemKt_reactCpp(
        JNIEnv *env, jclass /*clazz*/,
        jobject semanticExpressionJObj,
        jobject locale,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj,
        jobject jOutputter,
        jboolean informAboutWhatWasDone) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jstring>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
        auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
        auto &semExp = getSemExp(env, semanticExpressionJObj);

        mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
        memoryOperation::react(
                reaction, semanticMemory, semExp->clone(),
                lingDb);

        if (!reaction)
            return env->NewStringUTF("");
        auto reactionType = SemExpGetter::extractContextualAnnotation(**reaction);
        auto language = toLanguage(env, locale);
        runOutputter(env, language, semanticMemory, lingDb, **reaction, jOutputter,
                     informAboutWhatWasDone, &*semExp);
        return env->NewStringUTF(contextualAnnotation_toStr(reactionType).c_str());
    }, nullptr);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_onsem_OnsemKt_teachBehaviorCpp(
        JNIEnv *env, jclass /*clazz*/,
        jobject semanticExpressionJObj,
        jobject locale,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj,
        jobject jOutputter,
        jboolean informAboutWhatWasDone) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jstring>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
        auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
        auto &semExp = getSemExp(env, semanticExpressionJObj);

        mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
        memoryOperation::teach(
                reaction, semanticMemory, semExp->clone(),
                lingDb, memoryOperation::SemanticActionOperatorEnum::BEHAVIOR);

        if (!reaction)
            return env->NewStringUTF("");
        auto reactionType = SemExpGetter::extractContextualAnnotation(**reaction);
        auto language = toLanguage(env, locale);
        runOutputter(env, language, semanticMemory, lingDb, **reaction, jOutputter,
                     informAboutWhatWasDone, &*semExp);
        return env->NewStringUTF(contextualAnnotation_toStr(reactionType).c_str());
    }, nullptr);
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_onsem_OnsemKt_callOperatorsCpp(
        JNIEnv *env, jclass /*clazz*/,
        jobjectArray operatorsJObj,
        jobject semanticExpressionJObj,
        jobject locale,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj,
        jobject jOutputter) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jstring>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
        auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
        auto &semExp = getSemExp(env, semanticExpressionJObj);

        bool informAboutWhatWasDone = false;
        mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
        int size = env->GetArrayLength(operatorsJObj);
        for (int i = 0; i < size; ++i) {
            auto operatorJObj = reinterpret_cast<jobject>(env->GetObjectArrayElement(
                    operatorsJObj, i));
            auto javaOperatorEnum = toJavaOperatorEnum(env, operatorJObj, getSemanticEnumsIndexes(env));
            env->DeleteLocalRef(operatorJObj);

            switch (javaOperatorEnum)
            {
                case JavaOperatorEnum::REACTFROMTRIGGER:
                {
                    triggers::match(
                            reaction, semanticMemory, semExp->clone(),
                            lingDb);
                    break;
                }
                case JavaOperatorEnum::TEACHBEHAVIOR:
                {
                    memoryOperation::teach(
                            reaction, semanticMemory, semExp->clone(),
                            lingDb, memoryOperation::SemanticActionOperatorEnum::BEHAVIOR);
                    informAboutWhatWasDone = true;
                    break;
                }
                case JavaOperatorEnum::RESOLVECOMMAND:
                {
                    reaction = memoryOperation::resolveCommand(*semExp, semanticMemory, lingDb);
                    informAboutWhatWasDone = true;
                    break;
                }
                case JavaOperatorEnum::TEACHCONDITION:
                {
                    memoryOperation::teach(
                            reaction, semanticMemory, semExp->clone(),
                            lingDb, memoryOperation::SemanticActionOperatorEnum::CONDITION);
                    informAboutWhatWasDone = true;
                    break;
                }
                case JavaOperatorEnum::EXECUTEFROMCONDITION:
                {
                    reaction = memoryOperation::executeFromCondition(*semExp, semanticMemory, lingDb);
                    informAboutWhatWasDone = true;
                    break;
                }
            }
            if (reaction)
                break;
        }

        if (!reaction)
            return env->NewStringUTF("");
        auto reactionType = SemExpGetter::extractContextualAnnotation(**reaction);
        auto language = toLanguage(env, locale);
        runOutputter(env, language, semanticMemory, lingDb, **reaction, jOutputter,
                     informAboutWhatWasDone, &*semExp);
        return env->NewStringUTF(contextualAnnotation_toStr(reactionType).c_str());
    }, nullptr);
}



extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_OnsemKt_forget(
        JNIEnv *env, jclass /*clazz*/, jobject expressionWrapperForMemoryJObj,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj) {
    convertCppExceptionsToJavaExceptions(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto expressionWrapperForMemoryId = toDisposableWithIdId(env,
                                                                 expressionWrapperForMemoryJObj);

        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
        auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
        auto it = _idToExpWrapperForMemory.find(expressionWrapperForMemoryId);
        if (it == _idToExpWrapperForMemory.end()) {
            std::stringstream ss;
            ss << "expression wrapper for memory id " << expressionWrapperForMemoryId
               << " is not found";
            throw std::runtime_error(ss.str());
        }
        semanticMemory.memBloc.removeExpression(*it->second, lingDb, nullptr);
        _idToExpWrapperForMemory.erase(it);
    });
}



extern "C"
JNIEXPORT jobject JNICALL
Java_com_onsem_OnsemKt_notKnowing(
        JNIEnv *env, jclass /*clazz*/,
        jobject semanticExpressionJObj,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jobject>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
        auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
        auto &semExp = getSemExp(env, semanticExpressionJObj);
        return semanticExpressionPtrToJobject(env, memoryOperation::notKnowing(*semExp));
    }, nullptr);
}


extern "C"
JNIEXPORT jobject JNICALL
Java_com_onsem_OnsemKt_answer(
        JNIEnv *env, jclass /*clazz*/,
        jobject semanticExpressionJObj,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jobject>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
        auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
        auto &semExp = getSemExp(env, semanticExpressionJObj);
        return semanticExpressionPtrToJobject(env, memoryOperation::answer(semExp->clone(), false,
                                                                           semanticMemory, lingDb));
    }, nullptr);
}


extern "C"
JNIEXPORT jobject JNICALL
Java_com_onsem_OnsemKt_execute(
        JNIEnv *env, jclass /*clazz*/,
        jobject semanticExpressionJObj,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jobject>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
        auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
        auto &semExp = getSemExp(env, semanticExpressionJObj);
        return semanticExpressionPtrToJobject(env, memoryOperation::execute(*semExp, semanticMemory,
                                                                            lingDb));
    }, nullptr);
}


extern "C"
JNIEXPORT jobject JNICALL
Java_com_onsem_OnsemKt_executeFromCondition(
        JNIEnv *env, jclass /*clazz*/,
        jobject semanticExpressionJObj,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jobject>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
        auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
        auto &semExp = getSemExp(env, semanticExpressionJObj);
        return semanticExpressionPtrToJobject(env, memoryOperation::executeFromCondition(
                *semExp, semanticMemory, lingDb));
    }, nullptr);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_onsem_OnsemKt_sayFeedback(
        JNIEnv *env, jclass /*clazz*/,
        jobject semanticExpressionJObj,
        jobject typeOfFeedbackJObj,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jobject>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
        auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
        auto &semExp = getSemExp(env, semanticExpressionJObj);
        auto typeOfFeedback = toTypeOfFeedback(env, typeOfFeedbackJObj,
                                               getSemanticEnumsIndexes(env));
        return semanticExpressionPtrToJobject(env,
                                              memoryOperation::sayFeedback(*semExp, typeOfFeedback,
                                                                           semanticMemory, lingDb));
    }, nullptr);
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_onsem_OnsemKt_categorizeCpp(
        JNIEnv *env, jclass /*clazz*/,
        jobject semanticExpressionJObj) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jstring>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto &semExp = getSemExp(env, semanticExpressionJObj);
        auto textCategory = memoryOperation::categorize(*semExp);
        return env->NewStringUTF(semanticExpressionCategory_toStr(textCategory).c_str());
    }, nullptr);
}



extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_OnsemKt_learnSayCommand(
        JNIEnv *env, jclass /*clazz*/,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj) {
    convertCppExceptionsToJavaExceptions(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
        auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
        memoryOperation::learnSayCommand(semanticMemory, lingDb);
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_OnsemKt_allowToInformTheUserHowToTeach(
        JNIEnv *env, jclass /*clazz*/,
        jobject semanticMemoryJObj) {
    convertCppExceptionsToJavaExceptions(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
        memoryOperation::allowToInformTheUserHowToTeach(semanticMemory);
    });
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_onsem_OnsemKt_getStringReportOfTheNumberOfObjectsInMemoryToSpotLeakForDebug(
        JNIEnv *env, jclass /*clazz*/) {
    std::lock_guard<std::mutex> lock(_jniReferencesMutex);
    std::stringstream ss;
    {
        auto numberOfObjects = _idToExpWrapperForMemory.size();
        if (numberOfObjects > 0)
            ss << " ExpWrapperForMemory(" << numberOfObjects << ")";
    }
    {
        auto numberOfObjects = getNumberOfTextProcessingContextObjects();
        if (numberOfObjects > 0)
            ss << " TextProcessingContext(" << numberOfObjects << ")";
    }
    {
        auto numberOfObjects = getNumberOfSemanticMemoryObjects();
        if (numberOfObjects > 0)
            ss << " SemanticMemory(" << numberOfObjects << ")";
    }
    {
        auto numberOfObjects = getNumberOfSemanticExpressionObjects();
        if (numberOfObjects > 0)
            ss << " SemanticExpression(" << numberOfObjects << ")";
    }
    {
        auto numberOfObjects = getNumberOfLinguisticDatabasesObjects();
        if (numberOfObjects > 0)
            ss << " LinguisticDatabase(" << numberOfObjects << ")";
        auto numberCreatedSinceBeginOfRunTime = getNumberOfLinguisticDatabasesCreatedSinceBeginOfRunTime();
        if (numberCreatedSinceBeginOfRunTime > 1)
            ss << " LinguisticDatabaseCreatedSinceBeginOfRunTime("
               << numberCreatedSinceBeginOfRunTime << ")";
    }
    return env->NewStringUTF(ss.str().c_str());
}



extern "C"
JNIEXPORT jstring JNICALL
Java_com_onsem_OnsemKt_getLocaleFromText(
        JNIEnv *env, jclass /*clazz*/,
        jstring textJStr,
        jobject linguisticDatabaseJObj) {
    std::string languageStr = "un";
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jstring>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto textStr = toString(env, textJStr);
        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
        auto language = linguistics::getLanguage(textStr, lingDb);
        if (language == SemanticLanguageEnum::FRENCH)
            languageStr = "fr";
        else if (language == SemanticLanguageEnum::ENGLISH)
            languageStr = "en";
        return env->NewStringUTF(languageStr.c_str());
    }, env->NewStringUTF(languageStr.c_str()));
}

