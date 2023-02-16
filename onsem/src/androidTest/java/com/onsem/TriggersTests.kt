package com.onsem

import android.content.Context
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Assert.*
import org.junit.Test
import java.util.*

class TriggersTests {

    class TestExecutor : JavaExecutor() {
        var str: String = ""

        override fun onTextToSay(text: String) {
            str += "onTextToSay($text)"
        }

        override fun onResource(
            label: String,
            value: String,
            parameters: Map<String, String>
        ) {
            str += "onResource($label, $value, $parameters)"
        }
    }

    private fun reactFromTriggerStr(locale: Locale, input: String, semanticMemory: SemanticMemory, linguisticDb: LinguisticDatabase): String {
        val textProcessingContext = TextProcessingContext(toRobot = true, locale)
        val semExp = textToSemanticExpression(input, textProcessingContext, SemanticSourceEnum.UNKNOWN,
            semanticMemory, linguisticDb)
        val testExecutor = TestExecutor()
        reactFromTrigger(semExp, locale, semanticMemory, linguisticDb, testExecutor)
        return testExecutor.str
    }

    @Test
    fun triggersFrench() {
        val targetContext: Context = InstrumentationRegistry.getInstrumentation().targetContext
        val semanticMemory = SemanticMemory()
        val linguisticDb = LinguisticDatabase(targetContext.assets)
        val locale = Locale.FRENCH

        val parameters = HashMap<String, Array<String>>();
        parameters["distance"] = arrayOf(
            "De combien dois-je avancer en mètres ?",
            "De combien dois-je aller vers l'avant en mètres ?",
            "De combien dois-je aller tout droit en mètres ?")

        addTriggerToAResource("Avance", "mission", "avance-id", parameters, locale, semanticMemory, linguisticDb)

        assertEquals("onResource(mission, avance-id, {distance=0,3 mètre})",
            reactFromTriggerStr(locale, "Avance de 30 centimètres", semanticMemory, linguisticDb))
    }

    @Test
    fun triggersEnglish() {
        val targetContext: Context = InstrumentationRegistry.getInstrumentation().targetContext
        val semanticMemory = SemanticMemory()
        val linguisticDb = LinguisticDatabase(targetContext.assets)
        val locale = Locale.ENGLISH

        addTriggerToAResource("tell me who you are", "mission", "reaction-id", mapOf(), locale, semanticMemory, linguisticDb)

        assertEquals("onResource(mission, reaction-id, {})",
            reactFromTriggerStr(locale, "tell me who you are", semanticMemory, linguisticDb))
    }

    @Test
    fun triggersSpanish() {
        val targetContext: Context = InstrumentationRegistry.getInstrumentation().targetContext
        val semanticMemory = SemanticMemory()
        val linguisticDb = LinguisticDatabase(targetContext.assets)
        val locale = Locale("es", "ES")

        addTriggerToAResource("Dime quien eres", "mission", "reaction-es-id", mapOf(), locale, semanticMemory, linguisticDb)

        assertEquals("onResource(mission, reaction-es-id, {})",
            reactFromTriggerStr(locale, "Dime quien eres", semanticMemory, linguisticDb))
    }

}