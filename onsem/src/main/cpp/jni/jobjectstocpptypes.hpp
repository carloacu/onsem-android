#ifndef SEMANTIC_ANDROID_JOBJECTSTOCPPTYPES_HPP
#define SEMANTIC_ANDROID_JOBJECTSTOCPPTYPES_HPP

#include <map>
#include <string>
#include <jni.h>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/common/enum/semanticsourceenum.hpp>
#include <onsem/semantictotext/enum/semantictypeoffeedback.hpp>
#include "javaoperatorenum.hpp"


struct SemanticEnumsIndexes;

/**
 * All these jni conversions functions are called without thread protections.
 */


/**
 * Class to store a jobject.
 * The reference of this jobject will be deleted from the jni environment at the destruction of this object.
 */
class shared_jobject {
public:
    shared_jobject(
            JNIEnv *env,
            jobject object)
            : _env(env),
              _object(object) {}
    ~shared_jobject() {
        _env->DeleteLocalRef(_object);
    }
    jobject get() { return _object; }

private:
    JNIEnv* _env;
    jobject _object;
};


std::string toString(JNIEnv *env, jstring inputString);

onsem::SemanticLanguageEnum toLanguage(JNIEnv *env, jobject locale);

onsem::SemanticTypeOfFeedback toTypeOfFeedback(
        JNIEnv *env,
        jobject typeOfFeedbackJobj,
        const SemanticEnumsIndexes &pSemanticEnumsIndexes);

onsem::SemanticSourceEnum toSourceEnum(
        JNIEnv *env,
        jobject sourceEnumJobj,
        const SemanticEnumsIndexes &pSemanticEnumsIndexes);

JavaOperatorEnum toJavaOperatorEnum(
        JNIEnv *env,
        jobject operatorEnumJobj,
        const SemanticEnumsIndexes &pSemanticEnumsIndexes);

jint toDisposableWithIdId(JNIEnv *env, jobject object);



jobjectArray stlStringVectorToJavaArray(JNIEnv *env, const std::vector<std::string>& stdVector);

std::vector<std::string> javaArrayToStlStringVector(JNIEnv *env, jobjectArray jStrArray);

jobject stlStringStringMapToJavaHashMap(JNIEnv *env, const std::map<std::string, std::string>& map);

// Based on android platform code from: /media/jni/android_media_MediaMetadataRetriever.cpp
void JavaHashMapToStlStringStringVectorMap(JNIEnv *env, jobject hashMap, std::map<std::string, std::vector<std::string>>& mapOut);


#endif // SEMANTIC_ANDROID_JOBJECTSTOCPPTYPES_HPP
