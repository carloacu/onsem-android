package com.onsem

import android.content.Context
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Assert.*
import org.junit.Test
import java.util.*

class OnsemTests {

    val locale = Locale.FRENCH

    private fun textToCategory(text: String, linguisticDb: LinguisticDatabase): ExpressionCategory {
        val semanticMemory = SemanticMemory()
        val textProcessingContext = TextProcessingContext(toRobot = true, locale)
        val semExp = textToSemanticExpression(
            text, textProcessingContext, SemanticSourceEnum.UNKNOWN,
            semanticMemory, linguisticDb
        )
        return categorize(semExp)
    }

    private fun textToNotKnowing(text: String, linguisticDb: LinguisticDatabase): String {
        val semanticMemory = SemanticMemory()
        val textProcessingContext = TextProcessingContext(toRobot = true, locale)
        val semExp = textToSemanticExpression(
            text, textProcessingContext, SemanticSourceEnum.UNKNOWN,
            semanticMemory, linguisticDb
        )
        val notKnowingSemExp = notKnowing(semExp, semanticMemory, linguisticDb) ?: return ""
        return semanticExpressionToText(notKnowingSemExp, locale, semanticMemory, linguisticDb)
    }

    @Test
    fun categorize() {
        val targetContext: Context = InstrumentationRegistry.getInstrumentation().targetContext
        val linguisticDb = LinguisticDatabase(targetContext.assets)
        assertEquals(ExpressionCategory.QUESTION, textToCategory("qui es-tu", linguisticDb))
        assertEquals(ExpressionCategory.COMMAND, textToCategory("saute", linguisticDb))
        assertEquals(
            ExpressionCategory.AFFIRMATION,
            textToCategory("Je suis ton ami", linguisticDb)
        )
        assertEquals(ExpressionCategory.NOMINALGROUP, textToCategory("Un robot", linguisticDb))
        assertEquals(
            ExpressionCategory.CONDITION,
            textToCategory("Si il pleut alors on ne va pas sortir.", linguisticDb)
        )
        assertEquals(
            ExpressionCategory.CONDITIONTOCOMMAND,
            textToCategory("Si tu vois quelqu'un dis bonjour", linguisticDb)
        )
        assertEquals(
            ExpressionCategory.EXTERNALTEACHING,
            textToCategory("Je vais t'apprendre à saluer", linguisticDb)
        )
        assertEquals(
            ExpressionCategory.ACTIONDEFINITION,
            textToCategory("Sauter, c'est dire je saute", linguisticDb)
        )
    }

    @Test
    fun notKnowing() {
        val targetContext: Context = InstrumentationRegistry.getInstrumentation().targetContext
        val linguisticDb = LinguisticDatabase(targetContext.assets)
        assertEquals("Je ne sais pas qui je suis.", textToNotKnowing("qui es-tu", linguisticDb))
        assertEquals("Je ne sais pas sauter.", textToNotKnowing("saute", linguisticDb))
        assertEquals("", textToNotKnowing("je suis ton ami", linguisticDb))
    }
}