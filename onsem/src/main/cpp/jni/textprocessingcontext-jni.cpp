#include "textprocessingcontext-jni.hpp"
#include <sstream>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include "onsem-jni.h"
#include "jobjectstocpptypes.hpp"


using namespace onsem;

namespace {
    std::map<jint, TextProcessingContext> _idToTextProcessingContext;

    jint _newTextProcessingContext(TextProcessingContext &&pTextProcessingContext) {
        int newTPC = findMissingKey(_idToTextProcessingContext);
        _idToTextProcessingContext.emplace(newTPC, std::move(pTextProcessingContext));
        return newTPC;
    }

    TextProcessingContext &_getTextProcessingContext(int pTextProcessingContextId) {
        auto it = _idToTextProcessingContext.find(pTextProcessingContextId);
        if (it == _idToTextProcessingContext.end()) {
            std::stringstream ssErrorMessage;
            ssErrorMessage << "wrong text processing context id: " << pTextProcessingContextId;
            throw std::runtime_error(ssErrorMessage.str());
        }
        return it->second;
    }
}

TextProcessingContext &getTextProcessingContext(JNIEnv *env, jobject pTextProcessingContext) {
    return _getTextProcessingContext(toDisposableWithIdId(env, pTextProcessingContext));
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_onsem_TextProcessingContextKt_newTextProcessingContext(
        JNIEnv *env, jclass /*clazz*/, jboolean toRobot, jobject locale,
        jobjectArray resourceLabelArray) {
    return protectByMutexWithReturn<jint>([&]() {
        auto language = toLanguage(env, locale);

        auto textProcFromRobot = [&]() {
            if (toRobot)
               return TextProcessingContext::getTextProcessingContextToRobot(language);
            return TextProcessingContext::getTextProcessingContextFromRobot(language);
        }();
        textProcFromRobot.setUsAsEverybody();
        textProcFromRobot.vouvoiement = true;
        std::vector<std::string> resourceLabels;
        int size = env->GetArrayLength(resourceLabelArray);
        for (int i = 0; i < size; ++i) {
            auto resourceLabelJStr = reinterpret_cast<jstring>(env->GetObjectArrayElement(
                    resourceLabelArray, i));
            resourceLabels.emplace_back(toString(env, resourceLabelJStr));
            env->DeleteLocalRef(resourceLabelJStr);
        }
        textProcFromRobot.cmdGrdExtractorPtr =
                std::make_shared<ResourceGroundingExtractor>(resourceLabels);
        return _newTextProcessingContext(std::move(textProcFromRobot));
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_TextProcessingContextKt_deleteTextProcessingContext(
        JNIEnv *env, jclass /*clazz*/, jint textProcessingContextId) {
    protectByMutex([&]() {
        _idToTextProcessingContext.erase(textProcessingContextId);
    });
}


// Only for debug to spot a potential leak
std::size_t getNumberOfTextProcessingContextObjects() {
    return _idToTextProcessingContext.size();
}