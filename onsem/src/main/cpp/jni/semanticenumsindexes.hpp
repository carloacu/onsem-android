#ifndef SEMANTIC_ANDROID_SEMANTICENUMSINDEXES_HPP
#define SEMANTIC_ANDROID_SEMANTICENUMSINDEXES_HPP

#include <jni.h>
#include <vector>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include <onsem/common/enum/grammaticaltype.hpp>
#include <onsem/common/enum/verbgoalenum.hpp>
#include <onsem/common/enum/semanticreferencetype.hpp>
#include <onsem/common/enum/semanticsourceenum.hpp>
#include <onsem/semantictotext/enum/semantictypeoffeedback.hpp>
#include <onsem/semantictotext/enum/semanticexpressioncategory.hpp>
#include "javaoperatorenum.hpp"


enum class NaturalLanguagePolarity {
    IDENTICAL,
    OPPOSITE
};

enum class QuantityType {
    ONE,
    MANY
};


/**
 * Class to help the conversion of semantic java enums to cpp enums.
 * The indexes of the vectors correspond to the ordinal of the java values.
 * The values of the vectors correspond to the cpp enum values.
 */
struct SemanticEnumsIndexes {
    SemanticEnumsIndexes(JNIEnv *env);

    std::string verbTenseClassName;
    std::vector<onsem::SemanticVerbTense> javaOrdinalVerbTenseToCpp;
    std::string grammaticalTypeClassName;
    std::vector<onsem::GrammaticalType> javaOrdinalGrammaticalTypeToCpp;
    std::string verbGoalClassName;
    std::vector<onsem::VerbGoalEnum> javaOrdinalVerbGoalToCpp;
    std::string naturalLanguagePolarityClassName;
    std::vector<NaturalLanguagePolarity> javaOrdinalNaturalLanguagePolarityToCpp;
    std::string quantityTypeClassName;
    std::vector<QuantityType> javaOrdinalQuantityTypeToCpp;
    std::string referenceTypeClassName;
    std::vector<onsem::SemanticReferenceType> javaOrdinalReferenceTypeToCpp;
    std::string typeOfFeedbackClassName;
    std::vector<onsem::SemanticTypeOfFeedback> javaOrdinalTypeOfFeedbackToCpp;
    std::string semanticSourceEnumClassName;
    std::vector<onsem::SemanticSourceEnum> javaOrdinalSemanticSourceEnumToCpp;
    std::string expressionCategoryEnumClassName;
    std::vector<onsem::SemanticExpressionCategory> javaOrdinalSemanticExpressionCategoryEnumToCpp;
    std::string javaOperatorEnumClassName;
    std::vector<JavaOperatorEnum> javaOrdinalJavaOperatorEnumToCpp;
};

const SemanticEnumsIndexes &getSemanticEnumsIndexes(JNIEnv *env);

#endif // SEMANTIC_ANDROID_SEMANTICENUMSINDEXES_HPP
