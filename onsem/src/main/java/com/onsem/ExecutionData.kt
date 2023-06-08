package com.onsem


class ExecutionData {
    var text: String = ""

    var resourceLabel: String = ""
    var resourceValue: String = ""
    var resourceParameters: Map<String, Array<String>> = mapOf()
    var resourceNbOfTimes: Int = 1

    var numberOfTimes: Int = 1
    val toRunSequencially = mutableListOf<ExecutionData>()
    val toRunInParallel = mutableListOf<ExecutionData>()
    val toRunInBackground = mutableListOf<ExecutionData>()

    fun hasData(): Boolean = true
    fun hasChildren(): Boolean = true

    fun setResourceNbOfTimesRecursive(pNumberOfTimes: Int) {
        if (hasData())
        {
            resourceNbOfTimes = pNumberOfTimes
            return
        }
        if (toRunInBackground.isNotEmpty())
        {
            toRunInBackground.last().setResourceNbOfTimesRecursive(pNumberOfTimes)
            return
        }
        if (toRunInParallel.isNotEmpty())
        {
            toRunInParallel.last().setResourceNbOfTimesRecursive(pNumberOfTimes)
            return
        }
        if (toRunSequencially.isNotEmpty())
        {
            toRunSequencially.last().setResourceNbOfTimesRecursive(pNumberOfTimes)
            return
        }
    }

    fun toStr(pHasAlreadyData: Boolean = false): String {
        val dataStr = dataToStr()
        var res = dataStr

        if (resourceNbOfTimes > 1)
        {
            val newRes = "$res\tNUMBER_OF_TIMES: $resourceNbOfTimes"
            res = if (pHasAlreadyData || toRunInParallel.isNotEmpty() || toRunSequencially.isNotEmpty() || toRunInBackground.isNotEmpty())
                "(\t$newRes\t)"
            else
                newRes
        }

        fun printList(pListToPrint: List<ExecutionData>, pSeparator: String) {
            if (pListToPrint.isNotEmpty())
            {
                var newRes = res
                for (currElt in pListToPrint)
                {
                    if (currElt.hasData() || currElt.hasChildren())
                    {
                        if (newRes.isNotEmpty())
                            newRes += "\t" + pSeparator + "\t"
                        newRes += currElt.toStr(newRes.isNotEmpty() || pListToPrint.size > 1)
                    }
                }
                res = if (pHasAlreadyData || res != dataStr || toRunInBackground.isNotEmpty())
                    "(\t$newRes\t)"
                else
                    newRes
            }
        }
        printList(toRunInParallel, "AND")
        printList(toRunSequencially, "THEN")

        if (toRunInBackground.isNotEmpty())
        {
            var newRes = res
            for (currElt in toRunInBackground)
            {
                newRes += "\tIN_BACKGROUND: " +
                        currElt.toStr(newRes.isNotEmpty() || toRunInBackground.size > 1)
            }
            res = if (pHasAlreadyData || res != dataStr)
                "(\t$newRes\t)"
            else
                newRes
        }

        if (numberOfTimes > 1)
            res = "(\t$res\tNUMBER_OF_TIMES: $numberOfTimes\t)"
        return res
    }


    private fun dataToStr(): String {
        if (text.isNotEmpty())
            return text

        if (resourceLabel.isNotEmpty())
            return "onResource($resourceLabel, $resourceValue, {${parameterToStr(resourceParameters)}})"
        return ""
    }

    private fun parameterToStr(parameters: Map<String, Array<String>>): String {
        var res = ""
        for (currParam in parameters)
        {
            if (res.isNotEmpty())
                res += ", "
            res += currParam.key + "="
            var firstIteration = true
            for (currValue in currParam.value)
            {
                if (firstIteration)
                    firstIteration = false
                else
                    res += "|"
                res += currValue
            }
        }
        return res
    }
}