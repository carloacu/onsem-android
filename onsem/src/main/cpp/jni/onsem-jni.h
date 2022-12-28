#ifndef SEMANTIC_ANDROID_SEMANTIC_JNI_H
#define SEMANTIC_ANDROID_SEMANTIC_JNI_H

#include <jni.h>
#include <mutex>
#include <map>
#include "onsem/semantictotext/semanticmemory/links/expressionwithlinks.hpp"

namespace onsem {
    struct ExpressionWithLinks;
    struct SemanticMemory;
    struct SemanticExpression;
}

void executeRobotStr(
        JNIEnv *env,
        onsem::SemanticLanguageEnum pLanguage,
        onsem::SemanticMemory& pSemMemory,
        onsem::linguistics::LinguisticDatabase& pLingDb,
        onsem::UniqueSemanticExpression pUSemExp,
        jobject jExecutor,
        const onsem::SemanticExpression* pInputSemExpPtr);

void convertCppExceptionsToJavaExceptions(JNIEnv *env, const std::function<void()> &pFunction);


/// Any JNI function that will deal with references maintained across the JNI should use this lock.
void protectByMutex(const std::function<void()>& pFunction);
template<typename T>
T protectByMutexWithReturn(const std::function<T()>& pFunction);

template<typename T>
jint findMissingKey(const std::map<jint, T> &pIdToObj) {
    jint currentId = 0;
    for (const auto &currMemory : pIdToObj) {
        int nextId = currentId + 1;
        if (currMemory.first > nextId)
            return nextId;
        currentId = currMemory.first;
    }
    return currentId + 1;
}

/**
 * Convert C++ exception to java exception and as java exceptions don't stop the flow,
 * we need to return an object that corresponds to what the JNI wants. (that is why we have a default value)
 */
template<typename T>
T convertCppExceptionsToJavaExceptionsAndReturnTheResult(
        JNIEnv *env,
        const std::function<T()> &pFunction,
        const T &pDefaultReturn) {
    try {
        return pFunction();
    } catch (const std::exception &e) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"), e.what());
    }
    return pDefaultReturn;
}

jobject newExpressionWithLinks(
        JNIEnv *env,
        const std::shared_ptr<onsem::ExpressionWithLinks> &pExp);


#endif // SEMANTIC_ANDROID_SEMANTIC_JNI_H
