#include "jobjectstocpptypes.hpp"
#include "semanticenumsindexes.hpp"
#include <list>
#include <sstream>


using namespace onsem;

namespace {

    /**
     * Convert a jobject to an enum value.
     * @param env JNI environment.
     * @param jobj jobject.
     * @param enumClassName Name of the java enum.
     * @param javaOrdinalToCpp Indexes from java orinal to cpp enum values.
     */
    template<typename ENUM_TYPE>
    ENUM_TYPE _toEnum(
            JNIEnv *env,
            jobject jobj,
            const std::string &enumClassName,
            const std::vector<ENUM_TYPE> &javaOrdinalToCpp) {
        // Get the ordinal
        jclass enumClass = env->FindClass(enumClassName.c_str());
        jmethodID ordinalFun = env->GetMethodID(enumClass, "ordinal", "()I");
        int ordinal = env->CallIntMethod(jobj, ordinalFun);
        // Report if the ordinal is not valid
        if (ordinal < 0 || ordinal >= javaOrdinalToCpp.size()) {
            std::stringstream ss;
            ss << "invalid " << enumClassName << " ordinal " << ordinal;
            throw std::runtime_error(ss.str());
        }
        // Return the corresponding enum value
        return javaOrdinalToCpp[ordinal];
    }

    SemanticVerbTense _toVerbTense(
            JNIEnv *env,
            jobject verbTenseJobj,
            const SemanticEnumsIndexes &pSemanticEnumsIndexes) {
        return _toEnum(env, verbTenseJobj, pSemanticEnumsIndexes.verbTenseClassName,
                       pSemanticEnumsIndexes.javaOrdinalVerbTenseToCpp);
    }

    GrammaticalType _toGrammaticalType(
            JNIEnv *env,
            jobject grammaticalTypeJobj,
            const SemanticEnumsIndexes &pSemanticEnumsIndexes) {
        return _toEnum(env, grammaticalTypeJobj, pSemanticEnumsIndexes.grammaticalTypeClassName,
                       pSemanticEnumsIndexes.javaOrdinalGrammaticalTypeToCpp);
    }

    VerbGoalEnum _toVerbGoal(
            JNIEnv *env,
            jobject verbGoalJobj,
            const SemanticEnumsIndexes &pSemanticEnumsIndexes) {
        return _toEnum(env, verbGoalJobj, pSemanticEnumsIndexes.verbGoalClassName,
                       pSemanticEnumsIndexes.javaOrdinalVerbGoalToCpp);
    }

    bool _toPolarity(
            JNIEnv *env,
            jobject naturalLanguagePolarityJobj,
            const SemanticEnumsIndexes &pSemanticEnumsIndexes) {
        return _toEnum(env, naturalLanguagePolarityJobj, pSemanticEnumsIndexes.naturalLanguagePolarityClassName,
                       pSemanticEnumsIndexes.javaOrdinalNaturalLanguagePolarityToCpp) == NaturalLanguagePolarity::IDENTICAL;
    }

    QuantityType _toQuantity(
            JNIEnv *env,
            jobject quantityJobj,
            const SemanticEnumsIndexes &pSemanticEnumsIndexes) {
        return _toEnum(env, quantityJobj, pSemanticEnumsIndexes.quantityTypeClassName,
                       pSemanticEnumsIndexes.javaOrdinalQuantityTypeToCpp);
    }

    SemanticReferenceType _toReference(
            JNIEnv *env,
            jobject referenceJobj,
            const SemanticEnumsIndexes &pSemanticEnumsIndexes) {
        return _toEnum(env, referenceJobj, pSemanticEnumsIndexes.referenceTypeClassName,
                       pSemanticEnumsIndexes.javaOrdinalReferenceTypeToCpp);
    }
}


SemanticLanguageEnum toLanguage(JNIEnv *env, jobject locale) {
    jclass localeClass = env->FindClass("java/util/Locale");
    jmethodID getLanguageFun = env->GetMethodID(localeClass, "getLanguage", "()Ljava/lang/String;");
    auto languageJStr = reinterpret_cast<jstring>(env->CallObjectMethod(locale, getLanguageFun));
    const std::string languageStr = toString(env, languageJStr);
    env->DeleteLocalRef(languageJStr);
    if (languageStr == "fr")
        return SemanticLanguageEnum::FRENCH;
    if (languageStr == "en")
        return SemanticLanguageEnum::ENGLISH;
    if (languageStr == "ja")
        return SemanticLanguageEnum::JAPANESE;
    throw std::runtime_error("language " + languageStr + " is not supported");
}


SemanticTypeOfFeedback toTypeOfFeedback(
        JNIEnv *env,
        jobject typeOfFeedbackJobj,
        const SemanticEnumsIndexes &pSemanticEnumsIndexes) {
    return _toEnum(env, typeOfFeedbackJobj, pSemanticEnumsIndexes.typeOfFeedbackClassName,
                   pSemanticEnumsIndexes.javaOrdinalTypeOfFeedbackToCpp);
}


SemanticSourceEnum toSourceEnum(
        JNIEnv *env,
        jobject sourceEnumJobj,
        const SemanticEnumsIndexes &pSemanticEnumsIndexes) {
    return _toEnum(env, sourceEnumJobj, pSemanticEnumsIndexes.semanticSourceEnumClassName,
                   pSemanticEnumsIndexes.javaOrdinalSemanticSourceEnumToCpp);
}

std::string toString(JNIEnv *env, jstring inputString) {
    if (env == nullptr)
        return "";
    const char *cstring;
    if (!(cstring = env->GetStringUTFChars(inputString, nullptr)))
        return "";
    std::string string = cstring;
    env->ReleaseStringUTFChars(inputString, cstring);
    return string;
}


jint toDisposableWithIdId(JNIEnv *env, jobject object) {
    jclass semanticMemoryClass = env->FindClass("com/onsem/DisposableWithId");
    jmethodID getIdFun = env->GetMethodID(semanticMemoryClass, "getId", "()I");
    return env->CallIntMethod(object, getIdFun);
}



jobjectArray stlStringVectorToJavaArray(JNIEnv *env, const std::vector<std::string>& stdVector) {
    jobjectArray result;
    result = (jobjectArray)env->NewObjectArray(stdVector.size(),
                                               env->FindClass("java/lang/String"),
                                               env->NewStringUTF(""));

    jsize arrayElt = 0;
    for (const auto& currElt : stdVector)
        env->SetObjectArrayElement(result, arrayElt++, env->NewStringUTF(currElt.c_str()));
    return result;
}

std::vector<std::string> javaArrayToStlStringVector(JNIEnv *env, jobjectArray jStrArray) {
    std::vector<std::string> res;
    int size = env->GetArrayLength(jStrArray);
    for (int i = 0; i < size; ++i) {
        auto resourceLabelJStr = reinterpret_cast<jstring>(env->GetObjectArrayElement(
                jStrArray, i));
        res.emplace_back(toString(env, resourceLabelJStr));
        env->DeleteLocalRef(resourceLabelJStr);
    }
    return res;
}


jobject stlStringStringMapToJavaHashMap(JNIEnv *env, const std::map<std::string, std::string>& map) {
    jclass mapClass = env->FindClass("java/util/HashMap");
    if(mapClass == NULL)
        return NULL;

    jmethodID init = env->GetMethodID(mapClass, "<init>", "()V");
    jobject hashMap = env->NewObject(mapClass, init);
    jmethodID put = env->GetMethodID(mapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    std::map<std::string, std::string>::const_iterator citr = map.begin();
    for( ; citr != map.end(); ++citr) {
        jstring keyJava = env->NewStringUTF(citr->first.c_str());
        jstring valueJava = env->NewStringUTF(citr->second.c_str());

        env->CallObjectMethod(hashMap, put, keyJava, valueJava);

        env->DeleteLocalRef(keyJava);
        env->DeleteLocalRef(valueJava);
    }

    jobject hashMapGobal = static_cast<jobject>(env->NewGlobalRef(hashMap));
    env->DeleteLocalRef(hashMap);
    env->DeleteLocalRef(mapClass);

    return hashMapGobal;
}


// Based on android platform code from: /media/jni/android_media_MediaMetadataRetriever.cpp
void JavaHashMapToStlStringStringVectorMap(JNIEnv *env, jobject hashMap, std::map<std::string, std::vector<std::string>>& mapOut) {
    // Get the Map's entry Set.
    jclass mapClass = env->FindClass("java/util/Map");
    if (mapClass == nullptr) {
        return;
    }
    jmethodID entrySet =
            env->GetMethodID(mapClass, "entrySet", "()Ljava/util/Set;");
    if (entrySet == nullptr) {
        return;
    }
    jobject set = env->CallObjectMethod(hashMap, entrySet);
    if (set == nullptr) {
        return;
    }
    // Obtain an iterator over the Set
    jclass setClass = env->FindClass("java/util/Set");
    if (setClass == nullptr) {
        return;
    }
    jmethodID iterator =
            env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
    if (iterator == nullptr) {
        return;
    }
    jobject iter = env->CallObjectMethod(set, iterator);
    if (iter == nullptr) {
        return;
    }
    // Get the Iterator method IDs
    jclass iteratorClass = env->FindClass("java/util/Iterator");
    if (iteratorClass == nullptr) {
        return;
    }
    jmethodID hasNext = env->GetMethodID(iteratorClass, "hasNext", "()Z");
    if (hasNext == nullptr) {
        return;
    }
    jmethodID next =
            env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");
    if (next == nullptr) {
        return;
    }
    // Get the Entry class method IDs
    jclass entryClass = env->FindClass("java/util/Map$Entry");
    if (entryClass == nullptr) {
        return;
    }
    jmethodID getKey =
            env->GetMethodID(entryClass, "getKey", "()Ljava/lang/Object;");
    if (getKey == nullptr) {
        return;
    }
    jmethodID getValue =
            env->GetMethodID(env->FindClass("java/util/Map$Entry"), "getValue", "()Ljava/lang/Object;");
    if (getValue == nullptr) {
        return;
    }
    // Iterate over the entry Set
    while (env->CallBooleanMethod(iter, hasNext)) {
        jobject entry = env->CallObjectMethod(iter, next);
        jstring key = (jstring) env->CallObjectMethod(entry, getKey);
        auto value = javaArrayToStlStringVector(env, (jobjectArray)env->CallObjectMethod(entry, getValue));
        const char* keyStr = env->GetStringUTFChars(key, nullptr);
        if (!keyStr) {  // Out of memory
            return;
        }

        mapOut.insert(std::make_pair(std::string(keyStr), value));

        env->DeleteLocalRef(entry);
        env->ReleaseStringUTFChars(key, keyStr);
        env->DeleteLocalRef(key);
    }
}