package com.onsem.util

import com.onsem.JavaExecutor

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