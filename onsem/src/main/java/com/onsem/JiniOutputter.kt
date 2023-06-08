package com.onsem

import android.util.Log


class JiniOutputter {

    fun exposeText(text: String) {
        val newElt = getOrCreateNewElt()
        newElt.text = text
    }

    fun exposeResource(label: String, value: String, parameters: Map<String, Array<String>>) {
        val newElt = getOrCreateNewElt()
        newElt.resourceLabel = label
        newElt.resourceValue = value
        newElt.resourceParameters = parameters;
    }

    fun beginOfScope(linkStr: String) {
        if (_stack.isNotEmpty()) {
            val link = Link.valueOf(linkStr)
            val childLink = if (link == Link.IN_BACKGROUND) link else _stack.last().link
            val currElt = getLasExecInStack()
            val childList  = linkToChildList(currElt, childLink)
            childList.add(ExecutionData())
            val newElt = childList.last()
            _stack.add(LinkAndExecutionData(link, newElt))
        }
    }

    fun endOfScope() {
        if (_stack.size > 1)
            _stack.removeLast()
    }

    fun resourceNbOfTimes(numberOfTimes: Int) {
        getLasExecInStack().setResourceNbOfTimesRecursive(numberOfTimes)
    }

    fun insideScopeNbOfTimes(numberOfTimes: Int) {
        getLasExecInStack().numberOfTimes = numberOfTimes;
    }

    private fun getOrCreateNewElt(): ExecutionData {
        val newElt = getLasExecInStack()
        if (newElt.hasData() || newElt.hasChildren())
        {
            if (_stack.isNotEmpty()) {
                val childList  = linkToChildList(newElt, _stack.last().link)
                childList.add(ExecutionData())
                return childList.last()
            }
        }
        return newElt
    }

    private fun getLasExecInStack() = if (_stack.isNotEmpty()) _stack.last().executionData else rootExecutionData

    enum class Link {
        AND,
        THEN,
        THEN_REVERSED,
        IN_BACKGROUND
    }

    fun linkToChildList(executionData: ExecutionData, pLink: Link): MutableList<ExecutionData> =
        when (pLink) {
            Link.AND -> executionData.toRunInParallel;
            Link.IN_BACKGROUND -> executionData.toRunInBackground;
            else -> executionData.toRunSequencially;
        }

    data class LinkAndExecutionData(
        val link: Link,
        val executionData: ExecutionData
        )

    val rootExecutionData = ExecutionData()
    private val _stack = mutableListOf(LinkAndExecutionData(Link.THEN, rootExecutionData))
}