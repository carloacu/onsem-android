#ifndef SEMANTIC_ANDROID_SEMANTICEXPRESSION_JNI_HPP
#define SEMANTIC_ANDROID_SEMANTICEXPRESSION_JNI_HPP

#include <cstddef>
#include <jni.h>
#include <onsem/common/utility/unique_propagate_const.hpp>

namespace onsem {
    struct UniqueSemanticExpression;
}

const onsem::UniqueSemanticExpression &getSemExp(JNIEnv *env, jobject pSemExp);
jobject semanticExpressionPtrToJobject(JNIEnv *env, onsem::mystd::unique_propagate_const<onsem::UniqueSemanticExpression> pSemExpPtr);


// Only for debug to spot a potential leak
std::size_t getNumberOfSemanticExpressionObjects();


#endif // SEMANTIC_ANDROID_SEMANTICEXPRESSION_JNI_HPP
