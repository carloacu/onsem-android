package com.onsem


/// Class to do many replacements in a string.
class StringReplacer(isCaseSensitive: Boolean, haveSeparatorBetweenWords: Boolean) : DisposableWithId(newStringReplacer(isCaseSensitive, haveSeparatorBetweenWords)) {

    companion object {
        init {
            ensureInitialized()
        }

        /**
         * @brief Construct a Replacer.
         * @param isCaseSensitive If the matching will be case sensitive.
         * @param haveSeparatorBetweenWords If we expect separators between words.
         */
        private external fun newStringReplacer(isCaseSensitive : Boolean, haveSeparatorBetweenWords: Boolean): Int
    }

    /**
     * @brief Notify by what a string should be replaced by.
     * @param patternToSearch String a pattern to search.
     * @param output Output to put instead of the corresponding pattern to search.
     */
    external fun addReplacementPattern(patternToSearch: String, output: String)

    /**
     * @brief Take an input string and return the corresponding string after applying the replacement patterns.
     * @param input Input string.
     * @return The corresponding string after applying the replacement patterns.
     */
    external fun doReplacements(input: String): String

    /**
     * @brief Implementation of the free of the C++ memory for this object.<br/>
     * After this call this object will not be usable anymore.
     * @param id Identifier of this object.
     */
    external override fun disposeImplementation(id: Int)
}



