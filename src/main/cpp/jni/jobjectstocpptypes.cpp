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
