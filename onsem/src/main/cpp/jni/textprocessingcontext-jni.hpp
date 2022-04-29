#ifndef SEMANTIC_ANDROID_TEXTPROCESSINGCONTEXT_JNI_HPP
#define SEMANTIC_ANDROID_TEXTPROCESSINGCONTEXT_JNI_HPP

#include <cstddef>
#include <jni.h>
namespace onsem {
    struct TextProcessingContext;
}


onsem::TextProcessingContext &getTextProcessingContext(JNIEnv *env, jobject pTextProcessingContext);



// Only for debug to spot a potential leak
std::size_t getNumberOfTextProcessingContextObjects();



#endif // SEMANTIC_ANDROID_TEXTPROCESSINGCONTEXT_JNI_HPP
