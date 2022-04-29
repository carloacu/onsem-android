package com.onsem


enum class VerbTense {
    PRESENT,
    PUNCTUALPRESENT,
    PAST,
    PUNCTUALPAST
}

enum class GrammaticalType {
    SUBJECT,
    OBJECT,
    OWNER
}

enum class VerbGoal {
    ABILITY,
    ADVICE,
    CONDITIONAL,
    MANDATORY,
    NOTIFICATION,
    POSSIBILITY
}

enum class NaturalLanguagePolarity {
    IDENTICAL,
    OPPOSITE
}

enum class QuantityType {
    ONE,
    MANY
}

enum class ReferenceType {
    DEFINITE,
    INDEFINITE,
    UNDEFINED
}

enum class SemanticTypeOfFeedback {
    ASK_FOR_ADDITIONAL_INFORMATION,
    REACT_ON_SIMILARITIES,
    SENTIMENT
}

enum class SemanticSourceEnum {
    ASR,
    EVENT,
    WRITTENTEXT,
    TTS,
    SEMREACTION,
    METHODCALL,
    PROPERTY,
    UNKNOWN
}


enum class ContextualAnnotation {
    NOTIFYSOMETHINGWILLBEDONE,
    TEACHINGFEEDBACK,
    EXTERNALTEACHINGREQUEST,
    BEHAVIOR,
    ANSWER,
    QUESTION,
    FEEDBACK,
    BEHAVIORNOTFOUND,
    ANSWERNOTFOUND,
    REMOVEALLCONDITIONS,
    PROACTIVE,
    EMPTY
}


fun _getContextualAnnotationFromStr(contAnnotationStr: String?): ContextualAnnotation {
    if (contAnnotationStr == null || contAnnotationStr.isEmpty())
        return ContextualAnnotation.EMPTY
    if (contAnnotationStr == "notify_something_will_be_done")
        return ContextualAnnotation.NOTIFYSOMETHINGWILLBEDONE
    if (contAnnotationStr == "teaching_feedback")
        return ContextualAnnotation.TEACHINGFEEDBACK
    if (contAnnotationStr == "external_teaching_request")
        return ContextualAnnotation.EXTERNALTEACHINGREQUEST
    if (contAnnotationStr == "behavior")
        return ContextualAnnotation.BEHAVIOR
    if (contAnnotationStr == "answer")
        return ContextualAnnotation.ANSWER
    if (contAnnotationStr == "question")
        return ContextualAnnotation.QUESTION
    if (contAnnotationStr == "feedback")
        return ContextualAnnotation.FEEDBACK
    if (contAnnotationStr == "behavior_not_found")
        return ContextualAnnotation.BEHAVIORNOTFOUND
    if (contAnnotationStr == "answer_not_found")
        return ContextualAnnotation.ANSWERNOTFOUND
    if (contAnnotationStr == "remove_all_conditions")
        return ContextualAnnotation.REMOVEALLCONDITIONS
    if (contAnnotationStr == "proactive")
        return ContextualAnnotation.PROACTIVE
    return ContextualAnnotation.EMPTY
}

