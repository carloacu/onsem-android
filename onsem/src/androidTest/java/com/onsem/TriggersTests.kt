package com.onsem

import android.content.Context
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Assert.*
import org.junit.Test
import java.util.*

class TriggersTests {

    private fun reactFromTriggerStr(locale: Locale, input: String, semanticMemory: SemanticMemory, linguisticDb: LinguisticDatabase): String {
        val textProcessingContext = TextProcessingContext(toRobot = true, locale)
        val semExp = textToSemanticExpression(input, textProcessingContext, SemanticSourceEnum.UNKNOWN,
            semanticMemory, linguisticDb)
        val jiniOutputter = JiniOutputter()
        reactFromTrigger(semExp, locale, semanticMemory, linguisticDb, jiniOutputter)
        return jiniOutputter.rootExecutionData.toStr()
    }

    @Test
    fun triggersFrench() {
        val targetContext: Context = InstrumentationRegistry.getInstrumentation().targetContext
        val semanticMemory = SemanticMemory()
        val linguisticDb = LinguisticDatabase(targetContext.assets)
        val locale = Locale.FRENCH

        val parameters = HashMap<String, Array<String>>();
        parameters["distance"] = arrayOf("combien de mètres")

        addTriggerToAResource("Avance", "mission", "avance-id", parameters, locale, semanticMemory, linguisticDb)

        assertEquals("onResource(mission, avance-id, {distance=0,3 mètre})",
            reactFromTriggerStr(locale, "Avance de 30 centimètres", semanticMemory, linguisticDb))
        assertEquals("onResource(mission, avance-id, {distance=0,2 mètre})",
            reactFromTriggerStr(locale, "J'aimerais que tu avances de 20 centimètres", semanticMemory, linguisticDb))
    }

    @Test
    fun triggersEnglish() {
        val targetContext: Context = InstrumentationRegistry.getInstrumentation().targetContext
        val semanticMemory = SemanticMemory()
        val linguisticDb = LinguisticDatabase(targetContext.assets)
        val locale = Locale.ENGLISH

        addTriggerToAResource("who are you", "mission", "reaction-id", mapOf(), locale, semanticMemory, linguisticDb)

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