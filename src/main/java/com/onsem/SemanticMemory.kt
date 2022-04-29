package com.onsem

import com.onsem.LinguisticDatabase
import java.lang.RuntimeException


/**
 * A semantic memory that can be used to store an history and used to retrieve information.
 */
class SemanticMemory : DisposableWithId(newMemory()) {

    private var counterOfUsage = 0
    private var isUsingSubMemory: SemanticMemory? = null

    companion object {
        init {
            ensureInitialized()
        }

        const val robotAgentId = "me"
    }

    /**
     * Have this object using also the content of another memory (subSemantic).
     * @param subSemantic Semantic memory that will not be modified and that his content will be used.
     */
    fun linkASubMemory(subSemantic: SemanticMemory) {
        if (isDisposed)
            throw RuntimeException("the memory is already disposed")
        if (isUsingSubMemory != null)
            throw RuntimeException("a sub memory is already linked to this one")
        linkASubMemory(id, subSemantic.id)
        isUsingSubMemory = subSemantic
        ++subSemantic.counterOfUsage
    }

    /**
     * Set the id corresponding of the user currently interacting.
     */
    fun setCurrentUserId(currentUserId: String) {
        setCurrentUserId(id, currentUserId)
    }

    /**
     * Get the id corresponding of the user currently interacting.
     */
    fun getCurrentUserId(): String {
        return getCurrentUserId(id)
    }

    /**
     * Clear all the data except the data stored inside the sub memory if any (cf linkASubMemory() function).
     */
    fun clearLocalInformationButNotTheSubBloc() {
        clearLocalInformationButNotTheSubBlocMemory(id)
    }

    fun subscribeToLearnedBehaviors(linguisticDatabase: LinguisticDatabase) {
        subscribeToLearnedBehaviors(id, linguisticDatabase)
    }

    fun flushFactsToAdd(): Array<String> {
        return flushFactsToAdd(id)
    }

    fun flushVariablesToValue(): Array<String> {
        return flushVariablesToValue(id)
    }

    /**
     * Clear all the data except the data stored inside the sub memory if any (cf linkASubMemory() function).
     */
    fun linkUserIdToFullName(
        userId: String,
        fullname: String,
        linguisticDatabase: LinguisticDatabase
    ): ExpressionHandleInMemory? {
        return linkUserIdToFullName(id, userId, fullname, linguisticDatabase)
    }

    override fun disposeImplementation(id: Int) {
        if (counterOfUsage > 0)
            throw RuntimeException("$counterOfUsage other memory(s) is pointing to this one, please dispose the memory(s) that is using this memory first. (done by function linkASubMemory)")
        isUsingSubMemory?.let { --it.counterOfUsage }
        deleteMemory(id)
    }
}



private external fun newMemory(): Int
private external fun linkASubMemory(mainSemanticId: Int, subSemanticId: Int)
private external fun setCurrentUserId(memoryId: Int, currentUserId: String)
private external fun getCurrentUserId(memoryId: Int): String
private external fun clearLocalInformationButNotTheSubBlocMemory(memoryId: Int)
private external fun linkUserIdToFullName(
    memoryId: Int,
    userId: String,
    fullname: String,
    linguisticDatabase: LinguisticDatabase
): ExpressionHandleInMemory?

private external fun subscribeToLearnedBehaviors(memoryId: Int, linguisticDatabase: LinguisticDatabase)
private external fun flushFactsToAdd(memoryId: Int): Array<String>
private external fun flushVariablesToValue(memoryId: Int): Array<String>
private external fun deleteMemory(memoryId: Int)
