#include <jni.h>
#include <memory>
#include <mutex>
#include <iostream>
#include "jobjectstocpptypes.hpp"
#include "semanticenumsindexes.hpp"
#include "androidlog.hpp"
#include "textprocessingcontext-jni.hpp"
#include "linguisticdatabase-jni.hpp"
#include "semanticmemory-jni.hpp"
#include "semanticexpression-jni.hpp"
#include "onsem-jni.h"
#include <onsem/semantictotext/triggers.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>


using namespace onsem;


extern "C"
JNIEXPORT jstring JNICALL
Java_com_onsem_TriggersKt_reactFromTriggerCpp(
        JNIEnv *env, jclass /*clazz*/,
        jobject semanticExpressionJObj,
        jobject locale,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj,
        jobject jExecutor) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jstring>(env, [&]() {
        return protectByMutexWithReturn<jstring>([&] {
            auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
            auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
            auto &semExp = getSemExp(env, semanticExpressionJObj);

            mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
            triggers::match(
                    reaction, semanticMemory, semExp->clone(),
                    lingDb);

            if (!reaction)
                return env->NewStringUTF("");
            auto reactionType = SemExpGetter::extractContextualAnnotation(**reaction);
            auto language = toLanguage(env, locale);
            executeRobotStr(env, language, semanticMemory, lingDb, std::move(*reaction), jExecutor, &*semExp);
            return env->NewStringUTF(contextualAnnotation_toStr(reactionType).c_str());
        });
    }, nullptr);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_TriggersKt_addTrigger(
        JNIEnv *env, jclass /*clazz*/,
        jstring triggerJStr,
        jstring answerJStr,
        jobject locale,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj) {
    convertCppExceptionsToJavaExceptions(env, [&]() {
        protectByMutex([&] {
            auto language = toLanguage(env, locale);
            auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
            auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
            {
                auto triggerStr = toString(env, triggerJStr);
                auto textProcessingContextToRobot = TextProcessingContext::getTextProcessingContextToRobot(
                        language);
                auto triggerSemExp = converter::textToContextualSemExp(triggerStr,
                                                                       textProcessingContextToRobot,
                                                                       SemanticSourceEnum::UNKNOWN,
                                                                       lingDb);

                auto answerStr = toString(env, answerJStr);
                auto textProcessingContextFromRobot = TextProcessingContext::getTextProcessingContextFromRobot(
                        language);
                auto answerSemExp = converter::textToContextualSemExp(answerStr,
                                                                      textProcessingContextFromRobot,
                                                                      SemanticSourceEnum::UNKNOWN,
                                                                      lingDb);

                triggers::add(std::move(triggerSemExp), std::move(answerSemExp), semanticMemory, lingDb);
            }
        });
    });
}


extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_TriggersKt_addTriggerToAResource(
        JNIEnv *env, jclass /*clazz*/,
        jstring triggerJStr,
        jstring resourceTypeJStr,
        jstring resourceIdJStr,
        jobject parametersJObj,
        jobject locale,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj) {
    convertCppExceptionsToJavaExceptions(env, [&]() {
        protectByMutex([&] {
            auto language = toLanguage(env, locale);
            auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
            auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
            {
                auto triggerStr = toString(env, triggerJStr);
                auto textProcessingContextToRobot = TextProcessingContext::getTextProcessingContextToRobot(
                        language);
                auto triggerSemExp = converter::textToContextualSemExp(triggerStr,
                                                                       textProcessingContextToRobot,
                                                                       SemanticSourceEnum::UNKNOWN,
                                                                       lingDb);

                std::map<std::string, std::vector<std::string>> parameters;
                JavaHashMapToStlStringStringVectorMap(env, parametersJObj, parameters);

                TextProcessingContext paramQuestionProcContext(SemanticAgentGrounding::me,
                                                               SemanticAgentGrounding::currentUser,
                                                               language);
                paramQuestionProcContext.isTimeDependent = false;
                auto resourceTypeStr = toString(env, resourceTypeJStr);
                auto resourceIdStr = toString(env, resourceIdJStr);
                auto answerGrd = std::make_unique<SemanticResourceGrounding>(resourceTypeStr, language, resourceIdStr);

                std::cout << "trigger parameters: " << std::endl;
                for (auto& currParameter : parameters)
                {
                    std::cout << "currParameter: " << currParameter.first << std::endl;
                    for (auto& currQuestion : currParameter.second)
                    {
                        std::cout << "currQuestion: " << currQuestion << std::endl;
                        auto paramSemExp = converter::textToContextualSemExp(currQuestion,
                                                                             paramQuestionProcContext,
                                                                             SemanticSourceEnum::UNKNOWN, lingDb);
                        answerGrd->resource.parameterLabelsToQuestions[currParameter.first].emplace_back(std::move(paramSemExp));
                    }
                }

                auto resourceSemExp = std::make_unique<GroundedExpression>(std::move(answerGrd));

                triggers::add(std::move(triggerSemExp), std::move(resourceSemExp),
                              semanticMemory, lingDb);
            }
        });
    });
}

