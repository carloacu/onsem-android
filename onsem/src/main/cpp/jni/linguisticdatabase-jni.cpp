#include "linguisticdatabase-jni.hpp"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "onsem-jni.h"
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include "jobjectstocpptypes.hpp"
#include "keytoassetstreams.hpp"


using namespace onsem;

namespace {
    std::map<jint, linguistics::LinguisticDatabase> _idToLingDb;
    std::size_t numberOfLinguisticDatabasesCreatedSinceBeginOfRunTime = 0;
}


linguistics::LinguisticDatabase &getLingDb(int pLingDbId) {
    auto it = _idToLingDb.find(pLingDbId);
    if (it == _idToLingDb.end()) {
        std::stringstream ssErrorMessage;
        ssErrorMessage << "wrong linguistic database id: " << pLingDbId;
        throw std::runtime_error(ssErrorMessage.str());
    }
    return it->second;
}

linguistics::LinguisticDatabase &getLingDb(JNIEnv *env, jobject pLingDb) {
    return getLingDb(toDisposableWithIdId(env, pLingDb));
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_onsem_LinguisticDatabaseKt_newLinguisticDatabase(
        JNIEnv *env, jclass /*clazz*/, jobject assetManager, jobjectArray localesArray,
        jstring jlinguisticDatabasesRootFolder) {

    // This relative path is hard coded in the binary that generates the databases.
    const std::string linguisticFolder = toString(env, jlinguisticDatabasesRootFolder);
    const std::string binaryDatabaseFolder = linguisticFolder + "/databases";
    const std::string binaryDatabaseFolderWithSlash = binaryDatabaseFolder + "/";

    return convertCppExceptionsToJavaExceptionsAndReturnTheResult<jint>(env, [&]() {
        AAssetManager *assetMgr = AAssetManager_fromJava(env, assetManager);

        std::set<SemanticLanguageEnum> languages;
        int size = env->GetArrayLength(localesArray);
        for (int i = 0; i < size; ++i) {
            shared_jobject locale(env, env->GetObjectArrayElement(localesArray, i));
            languages.insert(toLanguage(env, locale.get()));
        }
        languages.insert(SemanticLanguageEnum::UNKNOWN);

        LinguisticDatabaseStreamsWithStorage iStreams;
        iStreams.addConceptFStream(assetMgr, binaryDatabaseFolder + "/concepts.bdb");

        for (auto language : languages) {
            auto languageFileName = semanticLanguageEnum_toLanguageFilenameStr(language);
            iStreams.addMainDicFile(language, binaryDatabaseFolderWithSlash + languageFileName +
                                              "database.bdb",
                                    assetMgr);
            iStreams.addAnimationsFile(language,
                                       binaryDatabaseFolderWithSlash + languageFileName +
                                       "animations.bdb",
                                       assetMgr);
            iStreams.addSynthesizerFile(language,
                                        binaryDatabaseFolderWithSlash + languageFileName +
                                        "synthesizer.bdb",
                                        assetMgr);

            if (language != SemanticLanguageEnum::UNKNOWN) {
                for (auto secondLanguage : languages) {
                    if (language != secondLanguage &&
                        secondLanguage != SemanticLanguageEnum::UNKNOWN) {
                        auto filename = binaryDatabaseFolder + "/" +
                                        semanticLanguageEnum_toLegacyStr(language) + "_to_" +
                                        semanticLanguageEnum_toLegacyStr(secondLanguage) +
                                        ".bdb";
                        iStreams.addFile(language, secondLanguage, filename, assetMgr);
                    }
                }
            }
        }

        {
            std::string wordsrelativePathsFilename =
                    linguisticFolder + "/wordsrelativePaths.txt";
            std::istream wordsrelativePathsFile(
                    new AssetStreambuf(assetMgr, wordsrelativePathsFilename));
            const std::string wordsFolderWithSlash =
                    linguisticFolder + "/dynamicdictionary/words/";
            std::string line;
            while (getline(wordsrelativePathsFile, line))
                if (!line.empty())
                    iStreams.addDynamicContentFStream(assetMgr, wordsFolderWithSlash + line);
        }

        {
            std::string treeConvertionsPathsFilename =
                    linguisticFolder + "/treeConvertionsPaths.txt";
            std::istream treeConvertionsPathsFile(
                    new AssetStreambuf(assetMgr, treeConvertionsPathsFilename));
            SemanticLanguageEnum currentLanguage = SemanticLanguageEnum::UNKNOWN;
            const std::string treeConversionsFolderWithSlash =
                    linguisticFolder + "/dynamicdictionary/treeconversions/";
            std::string line;
            while (getline(treeConvertionsPathsFile, line)) {
                if (line.empty())
                    continue;
                if (line[0] == '#')
                    currentLanguage = semanticLanguageEnum_fromLanguageFilenameStr(
                            line.substr(1, line.size() - 1));
                else
                    iStreams.addConversationsFile(currentLanguage,
                                                  treeConversionsFolderWithSlash + line,
                                                  assetMgr);
            }
        }

        jint lingDbId = 0;
        protectByMutex([&] {
            ++numberOfLinguisticDatabasesCreatedSinceBeginOfRunTime;
            lingDbId = findMissingKey(_idToLingDb);
            _idToLingDb.emplace(lingDbId, iStreams.linguisticDatabaseStreams);
        });
        return lingDbId;
    }, -1);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_onsem_LinguisticDatabaseKt_deleteLinguisticDatabase(
        JNIEnv *env, jclass /*clazz*/, jint linguisticDatabaseId) {
    protectByMutex([&] {
        _idToLingDb.erase(linguisticDatabaseId);
    });
}


// Only for debug to spot a potential leak
std::size_t getNumberOfLinguisticDatabasesObjects() {
    return _idToLingDb.size();
}

// Only for debug to spot a potential leak
std::size_t getNumberOfLinguisticDatabasesCreatedSinceBeginOfRunTime() {
    return numberOfLinguisticDatabasesCreatedSinceBeginOfRunTime;
}
