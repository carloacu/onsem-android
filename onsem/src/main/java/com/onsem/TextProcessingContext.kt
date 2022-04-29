package com.onsem

import java.util.*

/**
 * Class that specify the context needed to convert a semantic expression to/from a text.
 * @param toRobot True if the author is the user, False if the author is the robot.
 * @param locale Locale that is considered for the text.
 * @param resourceLabels Resource labels stored in the text. (eg: for the text "anything \resLabel=resValue\" the resource label is "resLabel")
 */
class TextProcessingContext(
    toRobot: Boolean,
    locale: Locale,
    resourceLabels: Array<String> = arrayOf()
) : DisposableWithId(newTextProcessingContext(toRobot, locale, resourceLabels)) {
    companion object {
        init {
            ensureInitialized()
        }
    }

    override fun disposeImplementation(id: Int) {
        deleteTextProcessingContext(id)
    }
}

fun <T> withTextProcessingContext(
    toRobot: Boolean,
    locale: Locale,
    resourceLabels: Array<String> = arrayOf(),
    block: (TextProcessingContext) -> T
): T {
    val textProcessingContext = TextProcessingContext(toRobot, locale, resourceLabels)
    return try {
        block(textProcessingContext)
    } finally {
        textProcessingContext.dispose()
    }
}


private external fun newTextProcessingContext(
    toRobot: Boolean,
    locale: Locale,
    resourceLabels: Array<String>
): Int

private external fun deleteTextProcessingContext(textProcessingContextId: Int)
