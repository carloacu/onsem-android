package com.onsem

import android.content.Context
import android.util.Log
import androidx.test.platform.app.InstrumentationRegistry
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
        val jiniOutputter = JiniOutputter()
        callOperators(operator.toTypedArray(), semExp, locale, semanticMemory, linguisticDb, jiniOutputter)
        return jiniOutputter.rootExecutionData.toStr()
    }

    private fun teachBehaviorStr(
        locale: Locale, input: String, semanticMemory: SemanticMemory, linguisticDb: LinguisticDatabase): String {
        val textProcessingContext = TextProcessingContext(toRobot = true, locale)
        val semExp = textToSemanticExpression(input, textProcessingContext, SemanticSourceEnum.UNKNOWN,
            semanticMemory, linguisticDb)
        val jiniOutputter = JiniOutputter()
        teachBehavior(semExp, locale, semanticMemory, linguisticDb, jiniOutputter, true)
        return jiniOutputter.rootExecutionData.toStr()
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

        addPlannerActionToMemory("Avance", "mission", "avance-id", parameters, locale, semanticMemory, linguisticDb)

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
        assertEquals("Ok pour profiter il faut dire je me baignerai. Et puis ?",
            reactFromOperatorsStr(listOf(SemanticOperator.TEACHBEHAVIOR, SemanticOperator.RESOLVECOMMAND,
                SemanticOperator.REACTFROMTRIGGER), locale, "Pour profiter il faut dire je vais me baigner", semanticMemory, linguisticDb))
       assertEquals("Ok pour profiter il faut dire je me baignerai et puis il faut avancer de 20 centimètres. Et puis ?",
            reactFromOperatorsStr(listOf(SemanticOperator.TEACHBEHAVIOR, SemanticOperator.RESOLVECOMMAND,
                SemanticOperator.REACTFROMTRIGGER), locale, "il faut avancer de 20 centimètres", semanticMemory, linguisticDb))
        assertEquals("Ok pour profiter il faut dire je me baignerai, puis il faut avancer de 20 centimètres et puis il faut dire c'est fini. Et puis ?",
            reactFromOperatorsStr(listOf(SemanticOperator.TEACHBEHAVIOR, SemanticOperator.RESOLVECOMMAND,
                SemanticOperator.REACTFROMTRIGGER), locale, "il faut dire c'est fini", semanticMemory, linguisticDb))

        assertEquals("Je me baignerai.\tTHEN\tonResource(mission, avance-id, {distance=0,2 mètre})\tTHEN\tC'est fini.",
            reactFromOperatorsStr(listOf(SemanticOperator.TEACHBEHAVIOR, SemanticOperator.RESOLVECOMMAND,
                SemanticOperator.REACTFROMTRIGGER), locale, "Profite", semanticMemory, linguisticDb))
    }


    @Test
    fun testComposition() {
        val targetContext: Context = InstrumentationRegistry.getInstrumentation().targetContext
        val semanticMemory = SemanticMemory()
        val linguisticDb = LinguisticDatabase(targetContext.assets)
        val locale = Locale.FRENCH
        learnSayCommand(semanticMemory, linguisticDb)

        addPlannerActionToMemory("Avance", "mission", "avance-id",
            mapOf("distance" to arrayOf("combien de mètres")), locale, semanticMemory, linguisticDb)

        addPlannerActionToMemory("mets une musique", "mission", "put-music-id",
            mapOf("object" to arrayOf("quoi")), locale, semanticMemory, linguisticDb)

        val howManyDegreesParameterQuestion = mapOf("angle" to arrayOf("combien de degrés"))
        addPlannerActionToMemory("Tourne à droite", "mission", "turn-right-id",
            howManyDegreesParameterQuestion, locale, semanticMemory, linguisticDb)
        addPlannerActionToMemory("Tourne à gauche", "mission", "turn-left-id",
            howManyDegreesParameterQuestion, locale, semanticMemory, linguisticDb)

        assertEquals("Ok pour faire un carré il faut avancer de 30 centimètres. Et puis ?",
            reactFromOperatorsStr(listOf(SemanticOperator.TEACHBEHAVIOR, SemanticOperator.RESOLVECOMMAND,
                SemanticOperator.REACTFROMTRIGGER), locale, "pour faire un carré il faut avancer de 30 centimètres", semanticMemory, linguisticDb))

        assertEquals("Ok pour faire un carré il faut avancer de 30 centimètres et puis il faut tourner de 90 degrés à droite. Et puis ?",
            reactFromOperatorsStr(listOf(SemanticOperator.TEACHBEHAVIOR, SemanticOperator.RESOLVECOMMAND,
                SemanticOperator.REACTFROMTRIGGER), locale, "il faut tourner à droite de 90 degrés", semanticMemory, linguisticDb))

        assertEquals("Ok pour faire un carré il faut avancer de 30 centimètres, puis il faut tourner de 90 degrés à droite et puis il faut répéter tout 3 fois. Et puis ?",
            reactFromOperatorsStr(listOf(SemanticOperator.TEACHBEHAVIOR, SemanticOperator.RESOLVECOMMAND,
                SemanticOperator.REACTFROMTRIGGER), locale, "il faut répéter tout ça 3 fois", semanticMemory, linguisticDb))

        assertEquals("Ok pour faire un carré il faut avancer de 30 centimètres, puis il faut tourner de 90 degrés à droite, puis il faut répéter tout 3 fois et puis il faut tourner à gauche. Et puis ?",
            reactFromOperatorsStr(listOf(SemanticOperator.TEACHBEHAVIOR, SemanticOperator.RESOLVECOMMAND,
                SemanticOperator.REACTFROMTRIGGER), locale, "il faut tourner à gauche", semanticMemory, linguisticDb))

        assertEquals("(\t(\tonResource(mission, avance-id, {distance=0,3 mètre})\tTHEN\tonResource(mission, turn-right-id, {angle=90 degrés})\t)\tNUMBER_OF_TIMES: 4\t)\tTHEN\t(\tonResource(mission, turn-left-id, {})\t)",
            reactFromOperatorsStr(listOf(SemanticOperator.TEACHBEHAVIOR, SemanticOperator.RESOLVECOMMAND,
                SemanticOperator.REACTFROMTRIGGER), locale, "fais un carré", semanticMemory, linguisticDb))
    }


    @Test
    fun teachBehaviorTest() {
        val targetContext: Context = InstrumentationRegistry.getInstrumentation().targetContext
        val semanticMemory = SemanticMemory()
        val linguisticDb = LinguisticDatabase(targetContext.assets)
        val locale = Locale.FRENCH
        learnSayCommand(semanticMemory, linguisticDb)
        assertEquals("Ok pour profiter il faut dire je profite. Et puis ?",
            teachBehaviorStr(locale, "Pour profiter il faut dire je profite", semanticMemory, linguisticDb))
    }

}