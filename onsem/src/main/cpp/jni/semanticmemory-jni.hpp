#ifndef SEMANTIC_ANDROID_SEMANTICMEMORY_JNI_HPP
#define SEMANTIC_ANDROID_SEMANTICMEMORY_JNI_HPP

#include <cstddef>
#include <jni.h>

namespace onsem {
    struct SemanticMemory;
}

onsem::SemanticMemory &getSemanticMemory(JNIEnv *env, jobject pSemanticMemory);


// Only for debug to spot a potential leak
std::size_t getNumberOfSemanticMemoryObjects();

#endif // SEMANTIC_ANDROID_SEMANTICMEMORY_JNI_HPP
