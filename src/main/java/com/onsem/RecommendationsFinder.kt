package com.onsem

import com.onsem.LinguisticDatabase
import java.util.*

class RecommendationsFinder(
    linguisticDatabase: LinguisticDatabase
) : DisposableWithId(newRecommendationsFinder(linguisticDatabase.id)) {

    companion object {
        init {
            ensureInitialized()
        }
    }

    override fun disposeImplementation(id: Int) {
        deleteRecommendationsFinder(id)
    }

    fun addRecommendation(
        text: String,
        recommendationId: String,
        locale: Locale,
        linguisticDatabase: LinguisticDatabase
    ) {
        addRecommendation(this, text, recommendationId, locale, linguisticDatabase)
    }

    fun getRecommendations(
        semanticExpression: SemanticExpression,
        linguisticDatabase: LinguisticDatabase
    ) : Array<String> {
        return getRecommendations(this, semanticExpression, linguisticDatabase)
    }
}



private external fun newRecommendationsFinder(linguisticDatabaseId: Int): Int

private external fun deleteRecommendationsFinder(id: Int)

external fun addRecommendation(
    recommendationsFinder: RecommendationsFinder,
    text: String,
    recommendationId: String,
    locale: Locale,
    linguisticDatabase: LinguisticDatabase
)

external fun getRecommendations(
    recommendationsFinder: RecommendationsFinder,
    semanticExpression: SemanticExpression,
    linguisticDatabase: LinguisticDatabase
) : Array<String>

