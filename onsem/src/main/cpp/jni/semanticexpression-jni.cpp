#include "semanticexpression-jni.hpp"
#include <sstream>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/outputter/outputtercontext.hpp>
#include <onsem/semantictotext/outputter/virtualoutputter.hpp>
#include "onsem-jni.h"
#include "jobjectstocpptypes.hpp"
#include "linguisticdatabase-jni.hpp"
#include "textprocessingcontext-jni.hpp"
#include "semanticmemory-jni.hpp"
#include "semanticenumsindexes.hpp"

using namespace onsem;

namespace {
    std::map<jint, UniqueSemanticExpression> _idToUniqueSemanticExpression;

    jobject _semanticExpressionIdToJobject(JNIEnv *env, jint semExpId) {
        jclass semanticExperssionClass = env->FindClass(
                "com/onsem/SemanticExpression");
        jmethodID semanticExpressionConstructor =
                env->GetMethodID(semanticExperssionClass, "<init>", "(I)V");
        return env->NewObject(semanticExperssionClass, semanticExpressionConstructor, semExpId);
    }

    const UniqueSemanticExpression &_getSemExp(int pSemExpId) {
        auto it = _idToUniqueSemanticExpression.find(pSemExpId);
        if (it == _idToUniqueSemanticExpression.end()) {
            std::stringstream ssErrorMessage;
            ssErrorMessage << "wrong semantic expression id: " << pSemExpId;
            throw std::runtime_error(ssErrorMessage.str());
        }
        return it->second;
    }
}

const UniqueSemanticExpression &getSemExp(JNIEnv *env, jobject pSemExp) {
    return _getSemExp(toDisposableWithIdId(env, pSemExp));
}

jobject semanticExpressionToJobject(JNIEnv *env, UniqueSemanticExpression pSemExp) {
    jint newId = findMissingKey(_idToUniqueSemanticExpression);
    _idToUniqueSemanticExpression.emplace(newId, std::move(pSemExp));
    return _semanticExpressionIdToJobject(env, newId);
}

jobject semanticExpressionPtrToJobject(JNIEnv *env,
                                       mystd::unique_propagate_const<UniqueSemanticExpression> pSemExpPtr) {
    if (pSemExpPtr)
        return semanticExpressionToJobject(env, std::move(*pSemExpPtr));
    return nullptr;
}


extern "C"
JNIEXPORT jobject JNICALL
Java_com_onsem_SemanticExpressionKt_textToSemanticExpression(
        JNIEnv *env, jclass /*clazz*/, jstring jtext,
        jobject textProcessingContextJobj,
        jobject sourceJobj,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jobject>(env, [&]() {
        return protectByMutexWithReturn<jobject>([&]() {
        auto text = toString(env, jtext);
            auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
            auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
            auto &textProcessingContext = getTextProcessingContext(env, textProcessingContextJobj);
            auto sourceEnum = toSourceEnum(env, sourceJobj, getSemanticEnumsIndexes(env));
            auto semExp = converter::textToContextualSemExp(text, textProcessingContext, sourceEnum, lingDb);
            memoryOperation::mergeWithContext(semExp, semanticMemory, lingDb);
            return semanticExpressionToJobject(env, std::move(semExp));
        });
    }, nullptr);
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_onsem_SemanticExpressionKt_semanticExpressionToText(
        JNIEnv *env, jclass /*clazz*/,
        jobject semanticExpressionJobj,
        jobject locale,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jstring>(env, [&]() {
        return protectByMutexWithReturn<jstring>([&]() {

            auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
            auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
            auto language = toLanguage(env, locale);
            auto &semExp = getSemExp(env, semanticExpressionJobj);
            auto textProcFromRobot = TextProcessingContext::getTextProcessingContextFromRobot(
                    language);
            textProcFromRobot.vouvoiement = true;
            std::string res;
            converter::semExpToText(res, semExp->clone(), textProcFromRobot, false, semanticMemory,
                                    lingDb, nullptr);

            return env->NewStringUTF(res.c_str());
        });
    }, nullptr);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_SemanticExpressionKt_deleteSemanticExpression(
        JNIEnv *env, jclass /*clazz*/, jint semanticexpressionId) {
    protectByMutex([&]() {
        _idToUniqueSemanticExpression.erase(semanticexpressionId);
    });
}


// Only for debug to spot a potential leak
std::size_t getNumberOfSemanticExpressionObjects() {
    return _idToUniqueSemanticExpression.size();
}