package com.onsem

import java.util.*

/**
 * Struct used to represent semantically a text.
 * It is like a natural language expression but this structure is more complex and it is not editable by hand by the user.
 */
class SemanticExpression private constructor(id: Int) : DisposableWithId(id) {
    companion object {
        init {
            ensureInitialized()
        }
    }

    override fun disposeImplementation(id: Int) {
        deleteSemanticExpression(id)
    }
}


/**
 * Construct a semantic expression, call a function and delete the semantic expression.
 */
fun <T> withSemanticExpression(
    text: String,
    textProcessingContext: TextProcessingContext,
    sourceEnum: SemanticSourceEnum,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase,
    block: (SemanticExpression) -> T
): T {
    val semExp =
        textToSemanticExpression(text, textProcessingContext, sourceEnum, semanticMemory, linguisticDatabase)
    return try {
        block(semExp)
    } finally {
        semExp.dispose()
    }
}


/**
 * Construct a semantic expression and a text processing context, call a function and delete the semantic expression and the text processing context.
 */
suspend fun <T> withSemanticExpression(
    text: String,
    toRobot: Boolean,
    locale: Locale,
    sourceEnum: SemanticSourceEnum,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase,
    block: suspend (SemanticExpression) -> T
): T {
    val textProcessingContext = TextProcessingContext(toRobot, locale)
    val semExp =
        textToSemanticExpression(text, textProcessingContext, sourceEnum, semanticMemory, linguisticDatabase)
    return try {
        block(semExp)
    } finally {
        semExp.dispose()
        textProcessingContext.dispose()
    }
}


/**
 * Convert a text to a semantic expression.
 * @param text Text to convert.
 * @param textProcessingContext Context for the conversion (author of the text, language, ...)
 * @param semanticMemory Semantic Memory.
 * @param linguisticDatabase Linguistic database.
 * @return Semantic expression corresponding to the input text.
 */
external fun textToSemanticExpression(
    text: String,
    textProcessingContext: TextProcessingContext,
    sourceEnum: SemanticSourceEnum,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase
): SemanticExpression


/**
 * Convert a semantic expression to a text.
 * @param semanticExpression Semantic expression to convert.
 * @param locale Locale of the output text.
 * @param semanticMemory Semantic Memory.
 * @param linguisticDatabase Linguistic database.
 * @return Text corresponding to the input semantic expression.
 */
external fun semanticExpressionToText(
    semanticExpression: SemanticExpression,
    locale: Locale,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase
): String


data class TextWithPotentialLabel(
    val text: String,
    val label: String = ""
)

fun semanticExpressionWithToTextWithResourceSplit(
    semanticExpression: SemanticExpression,
    locale: Locale,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase,
    resourcePrefixes: Array<String>
): List<TextWithPotentialLabel> {
    val textWithResourcePrinted = semanticExpressionToTextWithResourcePrinted(
        semanticExpression,
        locale,
        semanticMemory,
        linguisticDatabase
    )
    val textOrResourceList = textWithResourcePrinted.split('\\')
    return textOrResourceList.mapNotNull { text ->
        var res: TextWithPotentialLabel? = null
        for (prefix in resourcePrefixes) {
            val prefixWithEqual = "$prefix="
            if (text.startsWith(prefixWithEqual) && text.length > prefixWithEqual.length) {
                res = TextWithPotentialLabel(
                    text.substring(prefixWithEqual.length, text.length),
                    prefix
                )
                break
            }
        }
        if (text.isEmpty())
            null
        else
            res ?: TextWithPotentialLabel(text)
    }
}


private external fun deleteSemanticExpression(semanticExpressionId: Int)

private external fun semanticExpressionToTextWithResourcePrinted(
    semanticExpression: SemanticExpression,
    locale: Locale,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase
): String
