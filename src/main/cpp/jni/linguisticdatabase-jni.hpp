#ifndef SEMANTIC_ANDROID_LINGUISTICDATABASE_JNI_HPP
#define SEMANTIC_ANDROID_LINGUISTICDATABASE_JNI_HPP

#include <cstddef>
#include <jni.h>

namespace onsem {
    namespace linguistics {
        struct LinguisticDatabase;
    }
}

onsem::linguistics::LinguisticDatabase &getLingDb(int pLingDbId);
onsem::linguistics::LinguisticDatabase &getLingDb(JNIEnv *env, jobject pLingDb);



// Only for debug to spot a potential leak
std::size_t getNumberOfLinguisticDatabasesObjects();

// Only for debug to spot a potential leak
std::size_t getNumberOfLinguisticDatabasesCreatedSinceBeginOfRunTime();

#endif // SEMANTIC_ANDROID_LINGUISTICDATABASE_JNI_HPP
