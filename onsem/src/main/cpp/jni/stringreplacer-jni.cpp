#include <jni.h>
#include <onsem/common/utility/string.hpp>
#include "onsem-jni.h"
#include "jobjectstocpptypes.hpp"

using namespace onsem;

namespace {
    std::map<jint, mystd::Replacer> _idToStringReplacer;
    std::mutex _jniStringReplacerMutex;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_onsem_StringReplacer_00024Companion_newStringReplacer(JNIEnv *env, jobject thiz,
                                                               jboolean is_case_sensitive,
                                                               jboolean have_separator_between_words) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jint>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniStringReplacerMutex);
        jint newId = findMissingKey(_idToStringReplacer);
        _idToStringReplacer.emplace(newId, mystd::Replacer(is_case_sensitive, have_separator_between_words));
        return newId;
    }, -1);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_StringReplacer_addReplacementPattern(JNIEnv *env, jobject thiz,
                                                    jstring pattern_to_search, jstring output) {
    convertCppExceptionsToJavaExceptions(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniStringReplacerMutex);
        auto it = _idToStringReplacer.find(toDisposableWithIdId(env, thiz));
        if (it == _idToStringReplacer.end())
            return;
        it->second.addReplacementPattern(toString(env, pattern_to_search), toString(env, output));
    });
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_onsem_StringReplacer_doReplacements(JNIEnv *env, jobject thiz, jstring input) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jstring>(env, [&]() {
        std::lock_guard<std::mutex> lock(_jniStringReplacerMutex);
        auto it = _idToStringReplacer.find(toDisposableWithIdId(env, thiz));
        if (it == _idToStringReplacer.end())
            return input;
        return env->NewStringUTF(it->second.doReplacements(toString(env, input)).c_str());
    }, nullptr);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_StringReplacer_disposeImplementation(JNIEnv *env, jobject thiz, jint id) {
    std::lock_guard<std::mutex> lock(_jniStringReplacerMutex);
    _idToStringReplacer.erase(toDisposableWithIdId(env, thiz));
}