package com.onsem

import java.util.*




external fun addTrigger(
    trigger: String,
    answer: String,
    locale: Locale,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase
)


external fun addTriggerToAResource(
    trigger: String,
    resourceType: String,
    resourceId: String,
    parameters: Map<String, Array<String>>,
    locale: Locale,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase
)

external fun addPlannerActionToMemory(
    trigger: String,
    itIsAnActionId: String,
    actionId: String,
    parameters: Map<String, Array<String>>,
    locale: Locale,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase
)

fun reactFromTrigger(
    semanticExpression: SemanticExpression,
    locale: Locale,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase,
    executor: JavaExecutor
): ContextualAnnotation {
    return _getContextualAnnotationFromStr(
        reactFromTriggerCpp(
            semanticExpression,
            locale,
            semanticMemory,
            linguisticDatabase,
            executor
        )
    )
}

private external fun reactFromTriggerCpp(
    semanticExpression: SemanticExpression,
    locale: Locale,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase,
    executor: JavaExecutor
): String?