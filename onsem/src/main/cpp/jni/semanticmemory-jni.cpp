#include "semanticmemory-jni.hpp"
#include <sstream>
#include <onsem/semantictotext/semanticmemory/semantictracker.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticmemory/semanticbehaviordefinition.hpp>
#include "onsem-jni.h"
#include "jobjectstocpptypes.hpp"
#include "linguisticdatabase-jni.hpp"
#include "semanticenumsindexes.hpp"
#include "semanticexpression-jni.hpp"

using namespace onsem;


struct SemanticMemoryWithTrackers {
    SemanticMemory semanticMemory;
    std::list<std::shared_ptr<SemanticTracker>> semanticMemoryTrackers;
    std::set<jint> trackersId;
    std::list<jint> reachedValuesFromTrackerCache;
    mystd::observable::Connection infActionAddedConnection;
    std::map<std::string, std::string> varToValue;
    std::list<std::string> factsToAdd;
};


namespace {
    std::map<jint, SemanticMemoryWithTrackers> _idToSemanticMemoryWithTrackers;

    SemanticMemoryWithTrackers &_getSemanticMemoryWithTrackers(int pSemanticMemoryId) {
        auto it = _idToSemanticMemoryWithTrackers.find(pSemanticMemoryId);
        if (it == _idToSemanticMemoryWithTrackers.end()) {
            std::stringstream ssErrorMessage;
            ssErrorMessage << "wrong semantic memory id: " << pSemanticMemoryId;
            throw std::runtime_error(ssErrorMessage.str());
        }
        return it->second;
    }

    SemanticMemory &_getSemanticMemory(int pSemanticMemoryId) {
        return _getSemanticMemoryWithTrackers(pSemanticMemoryId).semanticMemory;
    }

}


SemanticMemory &getSemanticMemory(JNIEnv *env, jobject pSemanticMemory) {
    return _getSemanticMemory(toDisposableWithIdId(env, pSemanticMemory));
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_onsem_SemanticMemoryKt_newMemory(
        JNIEnv *env, jclass /*clazz*/) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jint>(env, [&]() {
        return protectByMutexWithReturn<jint>([&]() {
            jint newLocalMemory = findMissingKey(_idToSemanticMemoryWithTrackers);
            auto& newMemTracker = _idToSemanticMemoryWithTrackers[newLocalMemory];
            return newLocalMemory;
        });
    }, -1);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_SemanticMemoryKt_linkASubMemory(
        JNIEnv *env, jclass /*clazz*/, jint mainSemanticId, jint subSemanticId) {
    protectByMutex([&]() {
        _idToSemanticMemoryWithTrackers[mainSemanticId].semanticMemory.memBloc.subBlockPtr =
                &_idToSemanticMemoryWithTrackers[subSemanticId].semanticMemory.memBloc;
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_SemanticMemoryKt_setCurrentUserId(
        JNIEnv *env, jclass /*clazz*/, jint semanticMemoryId, jstring jcurrentUserId) {
    protectByMutex([&]() {
        auto currentUserId = toString(env, jcurrentUserId);
        _idToSemanticMemoryWithTrackers[semanticMemoryId].semanticMemory.setCurrUserId(
                currentUserId);
    });
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_onsem_SemanticMemoryKt_getCurrentUserId(
        JNIEnv *env, jclass /*clazz*/, jint semanticMemoryId) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jstring>(env, [&]() {
        return protectByMutexWithReturn<jstring>([&]() {
            auto userId = _idToSemanticMemoryWithTrackers[semanticMemoryId].semanticMemory.getCurrUserId();
            return env->NewStringUTF(userId.c_str());
        });
    }, jstring());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_SemanticMemoryKt_clearLocalInformationButNotTheSubBlocMemory(
        JNIEnv *env, jclass /*clazz*/, jint semanticMemoryId) {
    protectByMutex([&]() {
        _idToSemanticMemoryWithTrackers[semanticMemoryId].semanticMemory.clearLocalInformationButNotTheSubBloc();
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_onsem_SemanticMemoryKt_linkUserIdToFullName(
        JNIEnv *env, jclass /*clazz*/, jint semanticMemoryId,
        jstring juserId, jstring jfullname, jobject linguisticDatabaseJObj) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jobject>(env, [&]() {
        return protectByMutexWithReturn<jobject>([&]() {
            auto userId = toString(env, juserId);
            auto fullname = toString(env, jfullname);
            std::istringstream fullnameIss(fullname);
            std::vector<std::string> names{std::istream_iterator<std::string>{fullnameIss},
                                           std::istream_iterator<std::string>{}};
            auto &lingDb = getLingDb(env, linguisticDatabaseJObj);
            auto &semanticMemoryWithTrackers = _getSemanticMemoryWithTrackers(semanticMemoryId);
            auto &semanticMemory = semanticMemoryWithTrackers.semanticMemory;
            auto semExp = converter::agentIdWithNameToSemExp(userId, names);
            memoryOperation::resolveAgentAccordingToTheContext(semExp, semanticMemory, lingDb);
            return newExpressionWithLinks(
                    env,
                    memoryOperation::inform(std::move(semExp), semanticMemory, lingDb));
        });
    }, nullptr);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_SemanticMemoryKt_subscribeToLearnedBehaviors(
        JNIEnv *env, jclass /*clazz*/, jint semanticMemoryId, jobject linguisticDatabaseJObj) {
    protectByMutex([&]() {
        auto &semanticMemoryWithTrackers = _getSemanticMemoryWithTrackers(semanticMemoryId);
        auto &semanticMemory = semanticMemoryWithTrackers.semanticMemory;
        auto &lingDb = getLingDb(env, linguisticDatabaseJObj);

        semanticMemoryWithTrackers.infActionAddedConnection.disconnect();
        semanticMemoryWithTrackers.infActionAddedConnection =
                semanticMemory.memBloc.infActionAdded.connectUnsafe(
                        [&](intSemId, const GroundedExpWithLinks* pMemorySentencePtr) {
                            if (pMemorySentencePtr != nullptr) {
                                auto textProcToRobot = TextProcessingContext::getTextProcessingContextToRobot(
                                        SemanticLanguageEnum::FRENCH);
                                auto textProcFromRobot = TextProcessingContext::getTextProcessingContextFromRobot(
                                        SemanticLanguageEnum::FRENCH);
                                auto behaviorDef = SemanticMemoryBlock::extractActionFromMemorySentence(
                                        *pMemorySentencePtr);
                                UniqueSemanticExpression formulation1;
                                UniqueSemanticExpression formulation2;
                                converter::getInfinitiveToTwoDifferentPossibleWayToAskForIt(
                                        formulation1, formulation2, std::move(behaviorDef.label));
                                std::map<std::string, std::string> varToValue;
                                converter::semExpToText(varToValue["comportement_appris"],
                                                        std::move(formulation1),
                                                        textProcToRobot, false, semanticMemory, lingDb,
                                                        nullptr);
                                converter::semExpToText(varToValue["comportement_appris_2"],
                                                        std::move(formulation2),
                                                        textProcToRobot, false, semanticMemory, lingDb,
                                                        nullptr);
                                converter::semExpToText(varToValue["comportement_appris_resultat"],
                                                        converter::getFutureIndicativeAssociatedForm(
                                                                std::move(behaviorDef.composition)),
                                                        textProcFromRobot, false, semanticMemory,
                                                        lingDb, nullptr);


                                semanticMemoryWithTrackers.factsToAdd.emplace_back("robot_learnt_a_behavior");
                                for (const auto& currVarToValue : varToValue)
                                  semanticMemoryWithTrackers.varToValue[currVarToValue.first] = currVarToValue.second;
                            }
                        });
    });
}


extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_onsem_SemanticMemoryKt_flushFactsToAdd(
        JNIEnv *env, jclass /*clazz*/, jint semanticMemoryId) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jobjectArray>(env, [&]() {
        return protectByMutexWithReturn<jobjectArray>([&]() {
            auto &semanticMemoryWithTrackers = _getSemanticMemoryWithTrackers(semanticMemoryId);

            jobjectArray result;
            result = (jobjectArray)env->NewObjectArray(semanticMemoryWithTrackers.factsToAdd.size(),
                                                       env->FindClass("java/lang/String"),
                                                       env->NewStringUTF(""));

            jsize arrayElt = 0;
            for (const auto& currReference : semanticMemoryWithTrackers.factsToAdd)
                env->SetObjectArrayElement(result, arrayElt++, env->NewStringUTF(currReference.c_str()));
            semanticMemoryWithTrackers.factsToAdd.clear();
            return result;
        });
    }, nullptr);
}


extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_onsem_SemanticMemoryKt_flushVariablesToValue(
        JNIEnv *env, jclass /*clazz*/, jint semanticMemoryId) {
    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jobjectArray>(env, [&]() {
        return protectByMutexWithReturn<jobjectArray>([&]() {
            auto &semanticMemoryWithTrackers = _getSemanticMemoryWithTrackers(semanticMemoryId);

            jobjectArray result;
            result = (jobjectArray)env->NewObjectArray(semanticMemoryWithTrackers.varToValue.size() * 2,
                                                       env->FindClass("java/lang/String"),
                                                       env->NewStringUTF(""));

            jsize arrayElt = 0;
            for (const auto& currVarToValue : semanticMemoryWithTrackers.varToValue) {
                env->SetObjectArrayElement(result, arrayElt++, env->NewStringUTF(currVarToValue.first.c_str()));
                env->SetObjectArrayElement(result, arrayElt++, env->NewStringUTF(currVarToValue.second.c_str()));
            }
            semanticMemoryWithTrackers.varToValue.clear();
            return result;
        });
    }, nullptr);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_SemanticMemoryKt_deleteMemory(
        JNIEnv *env, jclass /*clazz*/, jint memoryId) {
    protectByMutex([&]() {
        _idToSemanticMemoryWithTrackers.erase(memoryId);
    });
}


// Only for debug to spot a potential leak
std::size_t getNumberOfSemanticMemoryObjects() {
    return _idToSemanticMemoryWithTrackers.size();
}