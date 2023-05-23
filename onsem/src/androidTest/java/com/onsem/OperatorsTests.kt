package com.onsem

import android.content.Context
import android.util.Log
import androidx.test.platform.app.InstrumentationRegistry
import com.onsem.util.TestExecutor
import org.junit.Assert.*
import org.junit.Test
import java.util.*

class OperatorsTests {

    private fun reactFromOperatorsStr(
        operator: List<SemanticOperator>,
        locale: Locale, input: String, semanticMemory: SemanticMemory, linguisticDb: LinguisticDatabase): String {
        val textProcessingContext = TextProcessingContext(toRobot = true, locale)
        val semExp = textToSemanticExpression(input, textProcessingContext, SemanticSourceEnum.UNKNOWN,
            semanticMemory, linguisticDb)
        val testExecutor = TestExecutor()
        callOperators(operator.toTypedArray(), semExp, locale, semanticMemory, linguisticDb, testExecutor)
        return testExecutor.str
    }

    private fun teachBehaviorStr(
        locale: Locale, input: String, semanticMemory: SemanticMemory, linguisticDb: LinguisticDatabase): String {
        val textProcessingContext = TextProcessingContext(toRobot = true, locale)
        val semExp = textToSemanticExpression(input, textProcessingContext, SemanticSourceEnum.UNKNOWN,
            semanticMemory, linguisticDb)
        val testExecutor = TestExecutor()
        teachBehavior(semExp, locale, semanticMemory, linguisticDb, testExecutor)
        return testExecutor.str
    }


    @Test
    fun operatorsTests() {
        val targetContext: Context = InstrumentationRegistry.getInstrumentation().targetContext
        val semanticMemory = SemanticMemory()
        val linguisticDb = LinguisticDatabase(targetContext.assets)
        val locale = Locale.FRENCH
        learnSayCommand(semanticMemory, linguisticDb)

        val parameters = HashMap<String, Array<String>>();
        parameters["distance"] = arrayOf("combien de mètres")

        addTriggerToAResource("Avance", "mission", "avance-id", parameters, locale, semanticMemory, linguisticDb)
        // Test with and without REACTFROMTRIGGER
        assertEquals("onResource(mission, avance-id, {distance=0,3 mètre})",
            reactFromOperatorsStr(listOf(SemanticOperator.REACTFROMTRIGGER), locale, "Avance de 30 centimètres", semanticMemory, linguisticDb))
        assertEquals("",
            reactFromOperatorsStr(listOf(SemanticOperator.TEACHBEHAVIOR), locale, "Avance de 30 centimètres", semanticMemory, linguisticDb))
        assertEquals("onResource(mission, avance-id, {distance=0,3 mètre})",
            reactFromOperatorsStr(listOf(SemanticOperator.TEACHBEHAVIOR, SemanticOperator.REACTFROMTRIGGER),
                locale, "Avance de 30 centimètres", semanticMemory, linguisticDb))

        // Test with and without teaching of behavior
        assertEquals("",
            reactFromOperatorsStr(listOf(SemanticOperator.REACTFROMTRIGGER), locale, "Pour profiter il faut dire je vais me baigner", semanticMemory, linguisticDb))
        assertEquals("",
            reactFromOperatorsStr(listOf(SemanticOperator.TEACHBEHAVIOR, SemanticOperator.RESOLVECOMMAND,
                SemanticOperator.REACTFROMTRIGGER), locale, "Profite", semanticMemory, linguisticDb))
        assertEquals("onTextToSay(Ok pour profiter il faut dire je me baignerai. Et puis ?)",
            reactFromOperatorsStr(listOf(SemanticOperator.TEACHBEHAVIOR, SemanticOperator.RESOLVECOMMAND,
                SemanticOperator.REACTFROMTRIGGER), locale, "Pour profiter il faut dire je vais me baigner", semanticMemory, linguisticDb))
        assertEquals("onTextToSay(Ok pour profiter il faut dire je me baignerai et puis il faut dire c'est fini. Et puis ?)",
            reactFromOperatorsStr(listOf(SemanticOperator.TEACHBEHAVIOR, SemanticOperator.RESOLVECOMMAND,
                SemanticOperator.REACTFROMTRIGGER), locale, "il faut dire c'est fini", semanticMemory, linguisticDb))
        assertEquals("onTextToSay(Je me baignerai.)onTextToSay(C'est fini.)",
            reactFromOperatorsStr(listOf(SemanticOperator.TEACHBEHAVIOR, SemanticOperator.RESOLVECOMMAND,
                SemanticOperator.REACTFROMTRIGGER), locale, "Profite", semanticMemory, linguisticDb))
    }

    @Test
    fun teachBehaviorTest() {
        val targetContext: Context = InstrumentationRegistry.getInstrumentation().targetContext
        val semanticMemory = SemanticMemory()
        val linguisticDb = LinguisticDatabase(targetContext.assets)
        val locale = Locale.FRENCH
        learnSayCommand(semanticMemory, linguisticDb)
        assertEquals("onTextToSay(Ok pour profiter il faut dire je profite. Et puis ?)",
            teachBehaviorStr(locale, "Pour profiter il faut dire je profite", semanticMemory, linguisticDb))
    }

}