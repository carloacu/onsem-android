package com.onsem

import org.junit.Assert.*
import org.junit.Test


class StringReplacerTests {


    @Test
    fun testSomeReplacements() {
        val replacerWithSeparators = StringReplacer(
            isCaseSensitive = true,
            haveSeparatorBetweenWords = true
        )
        val replacerWithoutSeparators = StringReplacer(
            isCaseSensitive = true,
            haveSeparatorBetweenWords = false
        )

        replacerWithSeparators.addReplacementPattern("^a", "salut")
        replacerWithoutSeparators.addReplacementPattern("^a", "salut")
        replacerWithSeparators.addReplacementPattern("toto", "titi")
        replacerWithoutSeparators.addReplacementPattern("toto", "titi")
        replacerWithSeparators.addReplacementPattern("^comment tu vas ?$", "ça va ?")
        replacerWithoutSeparators.addReplacementPattern("^comment tu vas ?$", "ça va ?")

        assertEquals("titi", replacerWithSeparators.doReplacements("toto"))
        assertEquals("titi", replacerWithoutSeparators.doReplacements("toto"))
        assertEquals("atoto", replacerWithSeparators.doReplacements("atoto"))
        assertEquals("saluttiti", replacerWithoutSeparators.doReplacements("atoto"))
        assertEquals("totob", replacerWithSeparators.doReplacements("totob"))
        assertEquals("titib", replacerWithoutSeparators.doReplacements("totob"))
        assertEquals("titi b", replacerWithSeparators.doReplacements("toto b"))
        assertEquals("titi b", replacerWithoutSeparators.doReplacements("toto b"))

        assertEquals("ça va ?", replacerWithSeparators.doReplacements("comment tu vas ?"))
        assertEquals("ça va ?", replacerWithoutSeparators.doReplacements("comment tu vas ?"))
        assertEquals(" comment tu vas ?", replacerWithSeparators.doReplacements(" comment tu vas ?"))
        assertEquals(" comment tu vas ?", replacerWithoutSeparators.doReplacements(" comment tu vas ?"))
        assertEquals("comment tu vas ? ", replacerWithSeparators.doReplacements("comment tu vas ? "))
        assertEquals("comment tu vas ? ", replacerWithoutSeparators.doReplacements("comment tu vas ? "))

        assertEquals("abc", replacerWithSeparators.doReplacements("abc"))
        assertEquals("salutbc", replacerWithoutSeparators.doReplacements("abc"))

        assertEquals("salut bc", replacerWithSeparators.doReplacements("a bc"))
        assertEquals("salut bc", replacerWithoutSeparators.doReplacements("a bc"))

        val replacerWithSeparatorsNotCaseSensitive = StringReplacer(
            isCaseSensitive = false,
            haveSeparatorBetweenWords = true
        )
        replacerWithSeparatorsNotCaseSensitive.addReplacementPattern("^a", "salut")
        replacerWithSeparatorsNotCaseSensitive.addReplacementPattern("Toto", "titi")
        replacerWithSeparatorsNotCaseSensitive.addReplacementPattern("^comment tu vas ?$", "ça va ?")

        assertEquals("A bc", replacerWithSeparators.doReplacements("A bc"))
        assertEquals("salut bc", replacerWithSeparatorsNotCaseSensitive.doReplacements("A bc"))
        assertEquals("titi", replacerWithSeparatorsNotCaseSensitive.doReplacements("toto"))
        assertEquals("titi", replacerWithSeparatorsNotCaseSensitive.doReplacements("Toto"))
        assertEquals("Toto", replacerWithSeparators.doReplacements("Toto"))

        replacerWithSeparators.dispose()
        replacerWithoutSeparators.dispose()
        replacerWithSeparatorsNotCaseSensitive.dispose()
    }

}