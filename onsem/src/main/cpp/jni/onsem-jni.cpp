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
#include <onsem/semantictotext/executor/executorcontext.hpp>
#include <onsem/semantictotext/executor/textexecutor.hpp>
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


    struct JniExecutor : public TextExecutor {
        JniExecutor(SemanticMemory &pSemanticMemory,
                    const linguistics::LinguisticDatabase &pLingDb,
                    VirtualExecutorLogger &pLogOnSynchronousExecutionCase,
                    JNIEnv *env,
                    jobject jExecutor)
                : TextExecutor(pSemanticMemory, pLingDb, pLogOnSynchronousExecutionCase),
                  _env(env),
                  _javaExecutorClass(env->FindClass("com/onsem/JavaExecutor")),
                  _jExecutor(jExecutor) {
        }

        virtual ~JniExecutor() {}

        FutureVoid _exposeText(
                const std::string &pText,
                SemanticLanguageEnum pLanguage,
                const FutureVoid &pStopRequest) override {
            jmethodID onTextToSayFun = _env->GetMethodID(_javaExecutorClass, "onTextToSay",
                                                         "(Ljava/lang/String;)V");
            _env->CallVoidMethod(_jExecutor, onTextToSayFun, _env->NewStringUTF(pText.c_str()));
            return VirtualExecutor::_exposeText(pText, pLanguage, pStopRequest);
        }


        FutureVoid _exposeResource(const SemanticResource &pResource,
                                   const SemanticExpression* pInputSemExpPtr,
                                   const FutureVoid &pStopRequest) override {
            std::map<std::string, std::vector<std::string>> parameters;
            if (!pResource.parameterLabelsToQuestions.empty())
            {
                std::cout << "if (!pResource.parameterLabelsToQuestions.empty()) TRUE" << std::endl;
                _extractParameters(parameters, pResource.parameterLabelsToQuestions,
                                   pResource.language, pInputSemExpPtr);
            } else {
                std::cout << "if (!pResource.parameterLabelsToQuestions.empty()) FAlSE" << std::endl;
            }



            std::map<std::string, std::string> parametersSimplified;
            for (auto& currParameter : parameters) {
                std::cout << "currResourceParameter: " << currParameter.first << std::endl;
                for (auto& currParameterValue : currParameter.second) {
                    std::cout << "currResourceParameterValue: " << currParameterValue << std::endl;
                    parametersSimplified[currParameter.first] = currParameterValue;
                    break;
                }
            }

            jmethodID onResourceFun = _env->GetMethodID(_javaExecutorClass, "onResource",
                                                        "(Ljava/lang/String;Ljava/lang/String;Ljava/util/Map;)V");
            _env->CallVoidMethod(_jExecutor, onResourceFun,
                                 _env->NewStringUTF(pResource.label.c_str()),
                                 _env->NewStringUTF(pResource.value.c_str()),
                                 stlStringStringMapToJavaHashMap(_env, parametersSimplified));

            _addLogAutoResource(pResource, parameters);
            return FutureVoid();
          }

    private:
        JNIEnv *_env;
        jclass _javaExecutorClass;
        jobject _jExecutor;
    };

}


void executeRobotStr(
        JNIEnv *env,
        SemanticLanguageEnum pLanguage,
        SemanticMemory& pSemMemory,
        linguistics::LinguisticDatabase& pLingDb,
        UniqueSemanticExpression pUSemExp,
        jobject jExecutor,
        const SemanticExpression* pInputSemExpPtr) {
    auto outContext = TextProcessingContext::getTextProcessingContextFromRobot(pLanguage);
    auto execContext = std::make_shared<ExecutorContext>(outContext);
    execContext->inputSemExpPtr = pInputSemExpPtr;
    std::string answer;
    DefaultExecutorLogger logger(answer);
    JniExecutor textExec(pSemMemory, pLingDb, logger, env, jExecutor);
    textExec.runSemExp(std::move(pUSemExp), execContext);
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

template jintArray protectByMutexWithReturn<jintArray>(const std::function<jintArray()> &pFunction);

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


jint JNI_OnLoad(JavaVM *vm, void *reserved) {
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
JNIEXPORT void JNICALL
Java_com_onsem_OnsemKt_executeRobotStr(
        JNIEnv *env, jclass /*clazz*/,
        jstring textJStr,
        jobject locale,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj,
        jobject jExecutor) {
    convertCppExceptionsToJavaExceptions(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto language = toLanguage(env, locale);
        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
        auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
        {
            auto textStr = toString(env, textJStr);
            auto textProcessingContextFromRobot = TextProcessingContext::getTextProcessingContextFromRobot(
                    language);
            auto textSemExp = converter::textToContextualSemExp(textStr,
                                                                textProcessingContextFromRobot,
                                                                SemanticSourceEnum::UNKNOWN,
                                                                lingDb);

            executeRobotStr(env, language, semanticMemory, lingDb, std::move(textSemExp), jExecutor, nullptr);
        }
    });
}



extern "C"
JNIEXPORT jobject JNICALL
Java_com_onsem_OnsemKt_inform(
        JNIEnv *env, jclass /*clazz*/,
        jobject semanticExpressionJObj,
        jobject locale,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj,
        jobject jExecutor) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jobject>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
        auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
        auto &semExp = getSemExp(env, semanticExpressionJObj);

        std::list<UniqueSemanticExpression> reactions;
        auto connection = semanticMemory.memBloc.actionProposalSignal.connectUnsafe([&](UniqueSemanticExpression& pUSemExp) {
            reactions.push_back(pUSemExp->clone());
        });

        auto res = newExpressionWithLinks(
                env,
                memoryOperation::inform(
                        semExp->clone(),
                        semanticMemory, lingDb));

        semanticMemory.memBloc.actionProposalSignal.disconnectUnsafe(connection);
        for (auto& currReaction : reactions) {
            auto language = toLanguage(env, locale);
            executeRobotStr(env, language, semanticMemory, lingDb, std::move(currReaction), jExecutor, &*semExp);
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
        jobject jExecutor) {
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
        executeRobotStr(env, language, semanticMemory, lingDb, std::move(*reaction), jExecutor, &*semExp);
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
Java_com_onsem_OnsemKt_executeFromTrigger(
        JNIEnv *env, jclass /*clazz*/,
        jobject semanticExpressionJObj,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jobject>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniReferencesMutex);
        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
        auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
        auto &semExp = getSemExp(env, semanticExpressionJObj);
        return semanticExpressionPtrToJobject(env, memoryOperation::executeFromTrigger(*semExp,
                                                                                       semanticMemory,
                                                                                       lingDb));
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

