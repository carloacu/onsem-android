package com.onsem

import android.content.res.AssetManager
import java.util.*


/**
 * Linguistic database necessary for the linguistic processing.
 * @param assetManager Assert manager to access to files stored in the assets.
 */
class LinguisticDatabase(
    assetManager: AssetManager,
    linguisticDatabasesRootFolder: String = "linguistic"
) : DisposableWithId(
    newLinguisticDatabase(
        assetManager,
        arrayOf(Locale.ENGLISH, Locale.FRENCH, Locale.JAPANESE),
        linguisticDatabasesRootFolder
    )
) {

    companion object {
        init {
            ensureInitialized()
        }
    }

    override fun disposeImplementation(id: Int) {
        deleteLinguisticDatabase(id)
    }
}





private external fun newLinguisticDatabase(
    assetManager: AssetManager,
    locales: Array<Locale>,
    linguisticDatabasesRootFolder: String
): Int

private external fun deleteLinguisticDatabase(linguisticDatabaseId: Int)

