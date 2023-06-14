package com.onsem

import android.content.Context
import android.util.Log
import androidx.test.platform.app.InstrumentationRegistry
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.newSingleThreadContext
import kotlinx.coroutines.runBlocking
import org.junit.Assert.*
import org.junit.Test
import java.io.BufferedReader
import java.io.ByteArrayOutputStream
import java.io.IOException
import java.io.InputStream
import java.io.InputStreamReader
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
        assertEquals("Je ne sais pas comment on fait un cheesecake.", textToNotKnowing("Comment fait-on un cheesecake ?", linguisticDb))
        assertEquals("Je ne sais pas sauter.", textToNotKnowing("saute", linguisticDb))
        assertEquals("", textToNotKnowing("je suis ton ami", linguisticDb))
    }


    private fun outputterToStr(
        executionData: ExecutionData
    ): String {
        var res = ""
        if (executionData.hasData()) {
            if (executionData.text.isNotEmpty())
                res += "robot: ${executionData.text}\n"
            if (executionData.resourceLabel == "resLabel") {
                res += executionData.resourceValue
                if (executionData.resourceParameters.isNotEmpty()) {
                    res += "("
                    for (param in executionData.resourceParameters) {
                        if (param.value.isNotEmpty())
                           res += param.key + "=" + param.value[0]
                    }
                    res += ")"
                }
            }
        }

        for (toRunSequencially in executionData.toRunSequencially)
            res += outputterToStr(toRunSequencially)
        return res
    }

    /*
    @Test
    fun checkMatching() {
        val targetContext: Context = InstrumentationRegistry.getInstrumentation().targetContext

        runBlocking {
            val semanticMemory = SemanticMemory()
            val linguisticDatabase = LinguisticDatabase(targetContext.assets)
            learnSayCommand(semanticMemory, linguisticDatabase)
            allowToInformTheUserHowToTeach(semanticMemory)

            val inputStream = targetContext.resources.openRawResource(R.raw.robot_triggers)
            val inputStreamReader = InputStreamReader(inputStream)
            val bufferedReader = BufferedReader(inputStreamReader)
            while (true) {
                val line = bufferedReader.readLine() ?: break
                val lineSplitted = line.split('#')
                if (lineSplitted.size < 4)
                    continue
                val label = lineSplitted[1]
                val id = lineSplitted[2]
                val text = lineSplitted[3]

                if (label == "trigger") {
                    val parameters = mutableMapOf<String, Array<String>>()
                    for (i in lineSplitted.indices) {
                        if (i > 3) {
                            val paramSplitted = lineSplitted[i].split('=')
                            if (paramSplitted.size > 1) {
                                val paraValues = parameters[paramSplitted[0]]?.toMutableList()?: mutableListOf()
                                paraValues.add(paramSplitted[1])
                                parameters[paramSplitted[0]] = paraValues.toTypedArray()
                            }
                        }
                    }

                    addPlannerActionToMemory(
                        text,
                        "resLabel",
                        id,
                        parameters,
                        locale,
                        semanticMemory,
                        linguisticDatabase
                    )
                } else if (label == "checkTrigger") {

                    withSemanticExpression(
                        text,
                        true,
                        locale,
                        SemanticSourceEnum.ASR,
                        semanticMemory,
                        linguisticDatabase
                    ) { inputSemanticExpression ->
                        val jniOutputter = JiniOutputter()
                        reactFromTrigger(
                            inputSemanticExpression,
                            locale,
                            semanticMemory,
                            linguisticDatabase,
                            jniOutputter
                        )

                        val outputStr = outputterToStr(jniOutputter.rootExecutionData)

                        if (outputStr != id)
                            Log.e("MachingError", "trigger: $text, expected: $id, get: $outputStr")
                    }
                }
            }
        }
    }
*/


}