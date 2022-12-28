#include <jni.h>
#include <cstddef>
#include <map>
#include <memory>
#include <sstream>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/semantictotext/recommendations.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include "linguisticdatabase-jni.hpp"
#include "onsem-jni.h"
#include "jobjectstocpptypes.hpp"
#include "semanticexpression-jni.hpp"


using namespace onsem;

namespace {
    std::map<jint, std::unique_ptr<SemanticRecommendationsContainer>> _idToRecommendationContainer;

    SemanticRecommendationsContainer &_getRecommendationsContainer(int pLingDbId) {
        auto it = _idToRecommendationContainer.find(pLingDbId);
        if (it == _idToRecommendationContainer.end()) {
            std::stringstream ssErrorMessage;
            ssErrorMessage << "wrong linguistic database id: " << pLingDbId;
            throw std::runtime_error(ssErrorMessage.str());
        }
        return *it->second;
    }

    SemanticRecommendationsContainer &_getRecommendationsContainer(JNIEnv *env, jobject pLingDb) {
        return _getRecommendationsContainer(toDisposableWithIdId(env, pLingDb));
    }
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_onsem_RecommendationsFinderKt_newRecommendationsFinder(
        JNIEnv *env, jclass /*clazz*/,
        jint linguisticDatabaseId) {

    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jint>(env, [&]() {

        jint id = 0;
        protectByMutex([&] {
            auto &lingDb = getLingDb(linguisticDatabaseId);
            id = findMissingKey(_idToRecommendationContainer);

            auto recommendationContainer = std::make_unique<SemanticRecommendationsContainer>();
            addGroundingCoef(recommendationContainer->goundingsToCoef,
                             std::make_unique<GroundedExpression>(
                                     std::make_unique<SemanticAgentGrounding>(
                                             SemanticAgentGrounding::currentUser)),
                             1, lingDb);
            addGroundingCoef(recommendationContainer->goundingsToCoef,
                             std::make_unique<GroundedExpression>(
                                     std::make_unique<SemanticAgentGrounding>(
                                             SemanticAgentGrounding::me)),
                             1, lingDb);
            _idToRecommendationContainer.emplace(id, std::move(recommendationContainer));
        });
        return id;
    }, -1);
}



extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_RecommendationsFinderKt_deleteRecommendationsFinder(
        JNIEnv *env, jclass /*clazz*/, jint id) {
    protectByMutex([&] {
        _idToRecommendationContainer.erase(id);
    });
}


extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_RecommendationsFinderKt_addRecommendation(
        JNIEnv *env, jclass /*clazz*/,
        jobject recommendationsFinderJObj,
        jstring textJStr,
        jstring recommendationIdJStr,
        jobject locale,
        jobject linguisticDatabaseJObj) {
    convertCppExceptionsToJavaExceptions(env, [&]() {
        protectByMutex([&] {
            auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
            auto &recommendationContainer = _getRecommendationsContainer(env,
                                                                         recommendationsFinderJObj);

            auto language = toLanguage(env, locale);
            auto textStr = toString(env, textJStr);
            auto recommendationIdStr = toString(env, recommendationIdJStr);

            auto textProcessingContextToRobot = TextProcessingContext::getTextProcessingContextToRobot(
                    language);
            auto semExp = converter::textToContextualSemExp(textStr,
                                                            textProcessingContextToRobot,
                                                            SemanticSourceEnum::UNKNOWN,
                                                            lingDb);
            addARecommendation(recommendationContainer, std::move(semExp), recommendationIdStr,
                               lingDb);
        });
    });
}


extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_onsem_RecommendationsFinderKt_getRecommendations(
        JNIEnv *env, jclass /*clazz*/,
        jobject recommendationsFinderJObj,
        jobject semExpJObj,
        jobject linguisticDatabaseJObj) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jobjectArray>(env, [&]() {
        return protectByMutexWithReturn<jobjectArray>([&] {
            auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
            auto &semExp = getSemExp(env, semExpJObj);
            auto &recommendationContainer = _getRecommendationsContainer(env,
                                                                         recommendationsFinderJObj);
            std::map<int, std::set<std::string>> recommendations;
            getRecommendations(recommendations, 100, *semExp, recommendationContainer, lingDb);

            std::list<std::string> recommendationsToReturn;
            std::size_t maxNbOfRecommendations = 3;
            auto itSetOfRecomendations = recommendations.end();
            while (itSetOfRecomendations != recommendations.begin()) {
                --itSetOfRecomendations;
                for (const auto &currRecommendation : itSetOfRecomendations->second) {
                    --maxNbOfRecommendations;
                    if (maxNbOfRecommendations > 0)
                        recommendationsToReturn.emplace_back(currRecommendation);
                    else
                        break;
                }
            }

            jobjectArray result;
            result = (jobjectArray) env->NewObjectArray(recommendationsToReturn.size(),
                                                        env->FindClass("java/lang/String"),
                                                        env->NewStringUTF(""));

            jsize arrayElt = 0;
            for (const auto& currRecommendation : recommendationsToReturn)
                env->SetObjectArrayElement(result, arrayElt++,
                                           env->NewStringUTF(currRecommendation.c_str()));
            return result;
        });
    }, nullptr);
}
