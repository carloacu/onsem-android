#include "semanticenumsindexes.hpp"
#include "jobjectstocpptypes.hpp"
#include <sstream>

using namespace onsem;

namespace {
    static const std::map<std::string, SemanticVerbTense> _javaToCppPartOfVerbTenses{
            {"PRESENT",         SemanticVerbTense::PRESENT},
            {"PUNCTUALPRESENT", SemanticVerbTense::PUNCTUALPRESENT},
            {"PAST",            SemanticVerbTense::PAST},
            {"PUNCTUALPAST",    SemanticVerbTense::PUNCTUALPAST}
    };
    static const std::map<std::string, GrammaticalType> _javaToCppGrammaticalTypes{
            {"SUBJECT", GrammaticalType::SUBJECT},
            {"OBJECT",  GrammaticalType::OBJECT},
            {"OWNER",   GrammaticalType::OWNER},
    };
    static const std::map<std::string, VerbGoalEnum> _javaToCppVerbGoals{
            {"ABILITY",      VerbGoalEnum::ABILITY},
            {"ADVICE",       VerbGoalEnum::ADVICE},
            {"CONDITIONAL",  VerbGoalEnum::CONDITIONAL},
            {"MANDATORY",    VerbGoalEnum::MANDATORY},
            {"NOTIFICATION", VerbGoalEnum::NOTIFICATION},
            {"POSSIBILITY",  VerbGoalEnum::POSSIBILITY},
    };
    static const std::map<std::string, NaturalLanguagePolarity> _javaToCppNaturalLanguagePolarity{
            {"IDENTICAL", NaturalLanguagePolarity::IDENTICAL},
            {"OPPOSITE",  NaturalLanguagePolarity::OPPOSITE}
    };
    static const std::map<std::string, QuantityType> _javaToCppQuantityType{
            {"ONE", QuantityType::ONE},
            {"MANY",  QuantityType::MANY}
    };
    static const std::map<std::string, SemanticReferenceType> _javaToCppReferenceType{
            {"DEFINITE", SemanticReferenceType::DEFINITE},
            {"INDEFINITE",  SemanticReferenceType::INDEFINITE},
            {"UNDEFINED",  SemanticReferenceType::UNDEFINED}
    };
    static const std::map<std::string, SemanticTypeOfFeedback> _javaToCppTypeOfFeedback{
            {"ASK_FOR_ADDITIONAL_INFORMATION", SemanticTypeOfFeedback::ASK_FOR_ADDITIONAL_INFORMATION},
            {"REACT_ON_SIMILARITIES",  SemanticTypeOfFeedback::REACT_ON_SIMILARITIES},
            {"SENTIMENT",  SemanticTypeOfFeedback::SENTIMENT}
    };
    static const std::map<std::string, SemanticSourceEnum> _javaToCppSemanticSourceEnum{
            {"ASR", SemanticSourceEnum::ASR},
            {"EVENT",  SemanticSourceEnum::EVENT},
            {"WRITTENTEXT",  SemanticSourceEnum::WRITTENTEXT},
            {"TTS",  SemanticSourceEnum::TTS},
            {"SEMREACTION",  SemanticSourceEnum::SEMREACTION},
            {"METHODCALL",  SemanticSourceEnum::METHODCALL},
            {"PROPERTY",  SemanticSourceEnum::PROPERTY},
            {"UNKNOWN",  SemanticSourceEnum::UNKNOWN}
    };

    int _getOrdinal(
            JNIEnv *env,
            jclass pJClass,
            const char *pEnumValue,
            const char *pClassNameReturnType,
            jmethodID ordinalFun) {
        jfieldID enumField = env->GetStaticFieldID(pJClass, pEnumValue, pClassNameReturnType);
        shared_jobject enumObject(env, env->GetStaticObjectField(pJClass, enumField));
        return env->CallIntMethod(enumObject.get(), ordinalFun);
    }

    template<typename ENUM_TYPE>
    std::vector<ENUM_TYPE> _getVectorOfCppEnumValues(
            JNIEnv *env,
            const std::string &enumClassName,
            const std::map<std::string, ENUM_TYPE> &pJavaEnumValuesToCppEnumValues) {
        std::stringstream enumClassNameReturnTypeSs;
        enumClassNameReturnTypeSs << "L" << enumClassName << ";";
        auto enumClassNameReturnTypeStr = enumClassNameReturnTypeSs.str();
        const char *enumClassNameReturnType = enumClassNameReturnTypeStr.c_str();
        jclass enumClass = env->FindClass(enumClassName.c_str());
        jmethodID ordinalFun = env->GetMethodID(enumClass, "ordinal", "()I");

        auto map = std::map<int, ENUM_TYPE>();
        for (const auto &currElt : pJavaEnumValuesToCppEnumValues)
            map.emplace(_getOrdinal(env, enumClass, currElt.first.c_str(),
                                    enumClassNameReturnType,
                                    ordinalFun), currElt.second);
        std::vector<ENUM_TYPE> res(map.size());
        int currentIndex = 0;
        for (const auto &currElt : map) {
            if (currElt.first != currentIndex) {
                std::stringstream ss;
                ss << "the mapping of an enum of " << enumClassName
                   << " from java to cpp is missing. Or the ordinals of "
                   << enumClassName << " are not from 0 to x without any gap";
                throw std::runtime_error(ss.str());
            }
            res[currentIndex++] = currElt.second;
        }
        return res;
    }
}


SemanticEnumsIndexes::SemanticEnumsIndexes(JNIEnv *env)
        : verbTenseClassName("com/onsem/VerbTense"),
          javaOrdinalVerbTenseToCpp(
                  _getVectorOfCppEnumValues(env,
                                            verbTenseClassName,
                                            _javaToCppPartOfVerbTenses)),
          grammaticalTypeClassName("com/onsem/GrammaticalType"),
          javaOrdinalGrammaticalTypeToCpp(
                  _getVectorOfCppEnumValues(env,
                                            grammaticalTypeClassName,
                                            _javaToCppGrammaticalTypes)),
          verbGoalClassName("com/onsem/VerbGoal"),
          javaOrdinalVerbGoalToCpp(
                  _getVectorOfCppEnumValues(env,
                                            verbGoalClassName,
                                            _javaToCppVerbGoals)),
          naturalLanguagePolarityClassName("com/onsem/NaturalLanguagePolarity"),
          javaOrdinalNaturalLanguagePolarityToCpp(
                  _getVectorOfCppEnumValues(env,
                                            naturalLanguagePolarityClassName,
                                            _javaToCppNaturalLanguagePolarity)),
          quantityTypeClassName("com/onsem/QuantityType"),
          javaOrdinalQuantityTypeToCpp(
                  _getVectorOfCppEnumValues(env,
                                            quantityTypeClassName,
                                            _javaToCppQuantityType)),
          referenceTypeClassName("com/onsem/ReferenceType"),
          javaOrdinalReferenceTypeToCpp(
                  _getVectorOfCppEnumValues(env,
                                            referenceTypeClassName,
                                            _javaToCppReferenceType)),
          typeOfFeedbackClassName("com/onsem/SemanticTypeOfFeedback"),
          javaOrdinalTypeOfFeedbackToCpp(
                  _getVectorOfCppEnumValues(env,
                                            typeOfFeedbackClassName,
                                            _javaToCppTypeOfFeedback)),
          semanticSourceEnumClassName("com/onsem/SemanticSourceEnum"),
          javaOrdinalSemanticSourceEnumToCpp(
                  _getVectorOfCppEnumValues(env,
                                            semanticSourceEnumClassName,
                                            _javaToCppSemanticSourceEnum)) {
}

const SemanticEnumsIndexes &getSemanticEnumsIndexes(JNIEnv *env) {
    static std::map<JNIEnv *, SemanticEnumsIndexes> envToIndexes;
    auto it = envToIndexes.find(env);
    if (it != envToIndexes.end())
        return it->second;
    return envToIndexes.emplace(env, SemanticEnumsIndexes(env)).first->second;
}