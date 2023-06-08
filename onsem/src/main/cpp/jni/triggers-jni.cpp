#include <jni.h>
#include <memory>
#include <mutex>
#include "jobjectstocpptypes.hpp"
#include "semanticenumsindexes.hpp"
#include "androidlog.hpp"
#include "textprocessingcontext-jni.hpp"
#include "linguisticdatabase-jni.hpp"
#include "semanticmemory-jni.hpp"
#include "semanticexpression-jni.hpp"
#include "onsem-jni.h"
#include "onsem/texttosemantic/languagedetector.hpp"
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/metadataexpression.hpp>
#include <onsem/semantictotext/triggers.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>

using namespace onsem;

namespace {
    UniqueSemanticExpression _createResourceSemExp(JNIEnv *env,
                                                   jstring resourceTypeJStr,
                                                   jstring resourceIdJStr,
                                                   jobject parametersJObj,
                                                   SemanticLanguageEnum pLanguage,
                                                   const UniqueSemanticExpression& pTriggerSemExp,
                                                   const linguistics::LinguisticDatabase &pLingDb) {
        auto resourceTypeStr = toString(env, resourceTypeJStr);
        auto resourceIdStr = toString(env, resourceIdJStr);

        std::map<std::string, std::vector<std::string>> parameters;
        JavaHashMapToStlStringStringVectorMap(env, parametersJObj, parameters);

        TextProcessingContext paramQuestionProcContext(SemanticAgentGrounding::me,
                                                       SemanticAgentGrounding::currentUser,
                                                       pLanguage);
        paramQuestionProcContext.isTimeDependent = false;
        auto answerGrd = std::make_unique<SemanticResourceGrounding>(resourceTypeStr, pLanguage,
                                                                     resourceIdStr);

        for (auto &currParameter: parameters) {
            for (auto &currQuestion: currParameter.second) {
                SemanticMemory semMemory;
                memoryOperation::inform(
                        std::make_unique<MetadataExpression>
                                (SemanticSourceEnum::WRITTENTEXT, UniqueSemanticExpression(), pTriggerSemExp->clone()),
                        semMemory, pLingDb);
                auto paramSemExp = converter::textToContextualSemExp(currQuestion,
                                                                     paramQuestionProcContext,
                                                                     SemanticSourceEnum::UNKNOWN, pLingDb);
                memoryOperation::mergeWithContext(paramSemExp, semMemory, pLingDb);
                answerGrd->resource.parameterLabelsToQuestions[currParameter.first].emplace_back(std::move(paramSemExp));
            }
        }
        return std::make_unique<GroundedExpression>(std::move(answerGrd));
    }

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
                auto resourceSemExp = _createResourceSemExp(env, resourceTypeJStr, resourceIdJStr, parametersJObj, language, triggerSemExp, lingDb);

                triggers::add(std::move(triggerSemExp), std::move(resourceSemExp),
                              semanticMemory, lingDb);
            }
        });
    });
}


extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_TriggersKt_addPlannerActionToMemory(
        JNIEnv *env, jclass /*clazz*/,
        jstring triggerJStr,
        jstring itIsAnActionIdJStr,
        jstring actionIdJStr,
        jobject parametersJObj,
        jobject locale,
        jobject semanticMemoryJObj,
        jobject linguisticDatabaseJObj) {

    auto triggerStr = toString(env, triggerJStr);

    auto language = toLanguage(env, locale);
    auto &semanticMemory = getSemanticMemory(env, semanticMemoryJObj);
    auto &lingDb = getLingDb(env, linguisticDatabaseJObj);

    if (!triggerStr.empty()) {
        SemanticLanguageEnum textLanguage = language == SemanticLanguageEnum::UNKNOWN ?
                                            linguistics::getLanguage(triggerStr, lingDb) : language;

        TextProcessingContext triggerProcContext(SemanticAgentGrounding::currentUser,
                                                 SemanticAgentGrounding::me,
                                                 textLanguage);
        triggerProcContext.setUsAsEverybody();
        triggerProcContext.isTimeDependent = false;
        auto actionSemExp = converter::textToSemExp(triggerStr, triggerProcContext, lingDb);


        auto itIsAnActionIdStr = toString(env, itIsAnActionIdJStr);
        auto actionIdStr = toString(env, actionIdJStr);
        std::map<std::string, std::vector<std::string>> parameters;
        JavaHashMapToStlStringStringVectorMap(env, parametersJObj, parameters);

        auto outputResourceGrdExp =
                std::make_unique<GroundedExpression>(
                        converter::createResourceWithParameters(itIsAnActionIdStr, actionIdStr, parameters,
                                                                *actionSemExp, lingDb, textLanguage));

        if (textLanguage == SemanticLanguageEnum::UNKNOWN)
            textLanguage = semanticMemory.defaultLanguage;
        mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
        auto infinitiveActionSemExp = converter::imperativeToInfinitive(*actionSemExp);
        if (infinitiveActionSemExp)
        {
            auto inputSemExpInMemory = memoryOperation::teachSplitted(reaction, semanticMemory,
                                                                      (*infinitiveActionSemExp)->clone(), outputResourceGrdExp->clone(),
                                                                      lingDb, memoryOperation::SemanticActionOperatorEnum::BEHAVIOR);

            triggers::add(std::move(*infinitiveActionSemExp), outputResourceGrdExp->clone(),
                          semanticMemory, lingDb);
        }

        triggers::add(std::move(actionSemExp),
                      std::move(outputResourceGrdExp),
                      semanticMemory, lingDb);
    }
}


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
            runOutputter(env, language, semanticMemory, lingDb, **reaction, jExecutor,
                         false, &*semExp);
            return env->NewStringUTF(contextualAnnotation_toStr(reactionType).c_str());
        });
    }, nullptr);
}
