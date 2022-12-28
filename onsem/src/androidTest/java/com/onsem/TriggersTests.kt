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


    @Test
    fun triggers() {
        val targetContext: Context = InstrumentationRegistry.getInstrumentation().targetContext
        val semanticMemory = SemanticMemory()
        val linguisticDb = LinguisticDatabase(targetContext.assets)

        val parameters = HashMap<String, Array<String>>();
        parameters["distance"] = arrayOf(
            "De combien dois-je avancer en centimètres ?",
            "De combien dois-je aller vers l'avant en centimètres ?",
            "De combien dois-je aller tout droit en centimètres ?")

        addTriggerToAResource("Avance", "mission", "avance-id", parameters, Locale.FRENCH, semanticMemory, linguisticDb)

        val textProcessingContext = TextProcessingContext(toRobot = true, Locale.FRENCH)
        val semExp = textToSemanticExpression("Avance de 2 mètres", textProcessingContext, SemanticSourceEnum.UNKNOWN,
            semanticMemory, linguisticDb)
        val testExecutor = TestExecutor()
        reactFromTrigger(semExp, Locale.FRENCH, semanticMemory, linguisticDb, testExecutor)
        assertEquals("onResource(mission, avance-id, {distance=200 centimètres})", testExecutor.str)
    }

}