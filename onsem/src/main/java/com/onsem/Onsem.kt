package com.onsem

import com.onsem.LinguisticDatabase
import java.lang.System.loadLibrary
import java.util.*



/**
 * Function to load the C++ library.
 * It's internal because the calls are hidden in the constructor of the objects.
 */
var isLoaded = false
internal fun ensureInitialized() = synchronized(isLoaded) {
    if (!isLoaded) {
        loadLibrary("onsem-jni")
        isLoaded = true
    }
}


/**
 * A disposable class that store an id and do the dispose code only at the first call.
 */
abstract class DisposableWithId(
    val id: Int
) {
    var isDisposed = false
        private set

    fun dispose() {
        if (!isDisposed) {
            disposeImplementation(id)
            isDisposed = true
        }
    }

    abstract fun disposeImplementation(id: Int)
}


abstract class JavaExecutor {
    abstract fun onTextToSay(text: String)
    abstract fun onResource(label: String, value: String)
}



/**
 * Class that represent a semantic expression that has been added into a semantic memory.
 * These objects are needed to forget an expression.
 * The dispose() function of this class only destroys the handle.
 * If you want to remove it from the semantic memory call forget() before to dispose this object.
 */
class ExpressionHandleInMemory private constructor(id: Int) : DisposableWithId(id) {
    override fun disposeImplementation(id: Int) {
        deleteExpressionWrapperForMemory(id)
    }
}

private external fun deleteExpressionWrapperForMemory(id:Int)


/**
 * Find if the text corresponds to a first name.
 * @param text Text to find if it corresponds to a first name.
 * @param linguisticDatabase Linguistic database for the linguistic processing.
 * @return True if the text corresponds to a first name, false otherwise.
 */
fun isAName(text: String, linguisticDatabase: LinguisticDatabase): Boolean {
    val spacePos = text.indexOf(' ')
    if (spacePos == -1)
        return isAProperNoun(text, linguisticDatabase.id)
    return isAProperNoun(text.substring(0, spacePos), linguisticDatabase.id)
}

private external fun isAProperNoun(text: String, linguisticDatabaseId: Int): Boolean


/**
 * Reactions to input texts.
 * =========================
 */

external fun executeRobotStr(
    text: String,
    locale: Locale,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase,
    executor: JavaExecutor
)

/**
 * Add a semantic expression in the memory.
 * @param semanticExpression Semantic expression to add to the semantic memory.
 * @param semanticMemory Semantic memory.
 * @param linguisticDatabase Linguistic database for the linguistic processing.
 * @return The semantic wrapper that represents the expression in the memory.
 */
external fun inform(
    semanticExpression: SemanticExpression,
    locale: Locale,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase,
    executor: JavaExecutor
): ExpressionHandleInMemory?


/**
 * Add a semantic expression in the memory that cannot be contradicted (replaced by another information more recent that contradicts it).
 * @param semanticExpression Semantic expression to add to the semantic memory.
 * @param semanticMemory Semantic memory.
 * @param linguisticDatabase Linguistic database for the linguistic processing.
 * @return The semantic wrapper that represents the expression in the memory.
 */
external fun informAxiom(
    semanticExpression: SemanticExpression,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase
): ExpressionHandleInMemory?


fun react(
    semanticExpression: SemanticExpression,
    locale: Locale,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase,
    executor: JavaExecutor
): ContextualAnnotation {
    return _getContextualAnnotationFromStr(
        reactCpp(
            semanticExpression,
            locale,
            semanticMemory,
            linguisticDatabase,
            executor
        )
    )
}

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

/**
 * Remove a semantic expression for a semantic memory.
 * @param expressionWrapperForMemory Semantic expression that has been added into the memory previously.
 * @param semanticMemory Semantic memory.
 * @param linguisticDatabase Linguistic database for the linguistic processing.
 */
external fun forget(
    expressionWrapperForMemory: ExpressionHandleInMemory,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase
)


/**
 * Answer by the negative to any question or order.
 * @param semanticExpression Semantic expression representing the input question or order.
 * @param semanticMemory Semantic memory.
 * @param linguisticDatabase Linguistic database for the linguistic processing.
 * @return A semantic expression that represent the answer. Null is returned if no answer is found.
 */
external fun notKnowing(
    semanticExpression: SemanticExpression,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase
): SemanticExpression?



/**
 * Answer a semantic expression according to a semantic memory.
 * @param semanticExpression Semantic expression representing the input question.
 * @param semanticMemory Semantic memory.
 * @param linguisticDatabase Linguistic database for the linguistic processing.
 * @return A semantic expression that represent the answer. Null is returned if no answer is found.
 */
external fun answer(
    semanticExpression: SemanticExpression,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase
): SemanticExpression?


/**
 * Converts a semantic expression at imperative form to his implementation for execution.
 * The implementation can be a text to say (if learnSayCommand() was called before) or some facts to add in the world.
 * ex1: "say hi" -> "hi"                    (text to say)
 * ex2: "greet" -> (was_greeted ?human_1)   (fact to add in the world)
 * @param semanticExpression Semantic expression representing the input text.
 * @param semanticMemory Semantic memory.
 * @param linguisticDatabase Linguistic database for the linguistic processing.
 * @return A semantic expression that represent the answer. Null is returned if no answer is found.
 */
external fun execute(
    semanticExpression: SemanticExpression,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase
): SemanticExpression?


/**
 * Converts an affirmative semantic expression to the implementation of a linked action.
 * The implementation can be a text to say (if learnSayCommand() was called before) or some facts to add in the world.
 * ex1: if this sentence was informed "say hi when somebody is interested"
 *      executeFromTrigger("I am interested") -> "hi"                        (text to say)
 * ex2: if this sentence was informed "greet when somebody is interested"
 *      executeFromTrigger("I am interested") -> (was_greeted ?human_1)      (fact to add in the world)
 * @param semanticExpression Semantic expression representing the input text.
 * @param semanticMemory Semantic memory.
 * @param linguisticDatabase Linguistic database for the linguistic processing.
 * @return A semantic expression that represent the answer. Null is returned if no answer is found.
 */
external fun executeFromTrigger(
    semanticExpression: SemanticExpression,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase
): SemanticExpression?




/**
 * Provide a feedback on a semantic expression.
 * 3 types of feedbacks are possible:
 *  sentiment: react emotionally to a positive or negative sentence, by saying "Thanks, you are kind" or "It is not kind"
 *  similarities: generate a sentence that point to a similarity, ex: "I like chocolate" -> "Paul likes chocolate too."
 *  ask for more information: generate a question to know mre about the input, ex: "I swam" -> "When did you swim?"
 * @param semanticExpression Semantic expression representing the input text.
 * @param typeOfFeedback Choose among sentiment, similarities or ask for more information.
 * @param semanticMemory Semantic memory.
 * @param linguisticDatabase Linguistic database for the linguistic processing.
 * @return A semantic expression that represent the answer. Null is returned if no answer is found.
 */
external fun sayFeedback(
    semanticExpression: SemanticExpression,
    typeOfFeedback: SemanticTypeOfFeedback,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase
): SemanticExpression?


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
    locale: Locale,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase
)

external fun addPlannerActionToMemory(
    trigger: String,
    itIsAnActionId: String,
    actionId: String,
    locale: Locale,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase
)


/**
 * Enable the possibility to execute the say command. (with the execute() function for example)
 * @param semanticMemory Semantic memory.
 * @param linguisticDatabase Linguistic database for the linguistic processing.
 */
external fun learnSayCommand(
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase
)

external fun allowToInformTheUserHowToTeach(
    semanticMemory: SemanticMemory
)


/**
 * Get a report of the number of the semantic objects currently in memory. (for debug only)
 */
external fun getStringReportOfTheNumberOfObjectsInMemoryToSpotLeakForDebug(): String


external fun getLocaleFromText(
    text: String,
    linguisticDatabase: LinguisticDatabase
): String




private external fun reactCpp(
    semanticExpression: SemanticExpression,
    locale: Locale,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase,
    executor: JavaExecutor
): String?


private external fun reactFromTriggerCpp(
    semanticExpression: SemanticExpression,
    locale: Locale,
    semanticMemory: SemanticMemory,
    linguisticDatabase: LinguisticDatabase,
    executor: JavaExecutor
): String?