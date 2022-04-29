#ifndef SEMANTIC_ANDROID_KEYTOFASSETSTREAMS_HPP
#define SEMANTIC_ANDROID_KEYTOFASSETSTREAMS_HPP

#include <streambuf>
#include <jni.h>
#include <iostream>
#include <string>
#include <memory>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <onsem/common/keytostreams.hpp>
#include <onsem/common/utility/make_unique.hpp>
#include <onsem/texttosemantic/linguisticanalyzer.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>



/**
 * Class to convert a asset filename to a std streambuf.
 */
class AssetStreambuf : public std::streambuf {
public:
    AssetStreambuf(AAssetManager *manager, const std::string &filename)
            : manager(manager) {
        asset = AAssetManager_open(manager, filename.c_str(), AASSET_MODE_STREAMING);
        buffer.resize(1024);

        setg(0, 0, 0);
        setp(&buffer.front(), &buffer.front() + buffer.size());
    }

    virtual ~AssetStreambuf() {
        sync();
        AAsset_close(asset);
    }

    std::streambuf::int_type underflow() override {
        auto bufferPtr = &buffer.front();
        auto counter = AAsset_read(asset, bufferPtr, buffer.size());

        if (counter == 0)
            return traits_type::eof();
        if (counter < 0) //error, what to do now?
            return traits_type::eof();

        setg(bufferPtr, bufferPtr, bufferPtr + counter);

        return traits_type::to_int_type(*gptr());
    }

    std::streambuf::int_type overflow(std::streambuf::int_type value) override {
        return traits_type::eof();
    };

    int sync() override {
        std::streambuf::int_type result = overflow(traits_type::eof());

        return traits_type::eq_int_type(result, traits_type::eof()) ? -1 : 0;
    }

private:
    AAssetManager *manager;
    AAsset *asset;
    std::vector<char> buffer;
};


/**
 * Class to convert a asset filename to a std istream.
 */
class AssetIstream : public std::istream {
public:
    AssetIstream(AAssetManager *manager, const std::string &file)
            : std::istream(new AssetStreambuf(manager, file)) {
    }

    AssetIstream(const std::string &file)
            : std::istream(new AssetStreambuf(manager, file)) {
    }

    virtual ~AssetIstream() {
        delete rdbuf();
    }

    static void setAssetManager(AAssetManager *m) {
        manager = m;
    }

private:
    static AAssetManager *manager;
};


/**
 * Class to store the istreams to construct a linguistic database.
 */
struct LinguisticDatabaseStreamsWithStorage {
    std::list<std::unique_ptr<std::istream>> assetStreams;
    onsem::linguistics::LinguisticDatabaseStreams linguisticDatabaseStreams;

    void addConceptFStream(
            AAssetManager *pAssetMgr,
            const std::string &pFilename) {
        assetStreams.push_back(
                std::make_unique<AssetIstream>(pAssetMgr, pFilename));
        linguisticDatabaseStreams.concepts = &*assetStreams.back();
    }

    void addDynamicContentFStream(
            AAssetManager *pAssetMgr,
            const std::string &pFilename) {
        assetStreams.push_back(
                std::make_unique<AssetIstream>(pAssetMgr, pFilename));
        linguisticDatabaseStreams.dynamicContentStreams.push_back(&*assetStreams.back());
    }

    void addMainDicFile(
            onsem::SemanticLanguageEnum pLanguage,
            const std::string &pFilename,
            AAssetManager *pAssetMgr) {
        assetStreams.push_back(std::make_unique<AssetIstream>(pAssetMgr, pFilename));
        linguisticDatabaseStreams.languageToStreams[pLanguage].mainDicToStream = assetStreams.back().get();
    }

    void addAnimationsFile(
            onsem::SemanticLanguageEnum pLanguage,
            const std::string &pFilename,
            AAssetManager *pAssetMgr) {
        assetStreams.push_back(std::make_unique<AssetIstream>(pAssetMgr, pFilename));
        linguisticDatabaseStreams.languageToStreams[pLanguage].animationsToStream = assetStreams.back().get();
    }

    void addSynthesizerFile(
            onsem::SemanticLanguageEnum pLanguage,
            const std::string &pFilename,
            AAssetManager *pAssetMgr) {
        assetStreams.push_back(std::make_unique<AssetIstream>(pAssetMgr, pFilename));
        linguisticDatabaseStreams.languageToStreams[pLanguage].synthesizerToStream = assetStreams.back().get();
    }

    void addFile(
            onsem::SemanticLanguageEnum pInLanguage,
            onsem::SemanticLanguageEnum pOutLanguage,
            const std::string &pFilename,
            AAssetManager *pAssetMgr) {
        assetStreams.push_back(std::make_unique<AssetIstream>(pAssetMgr, pFilename));
        linguisticDatabaseStreams.languageToStreams[pInLanguage].
                translationStreams[pOutLanguage] = &*assetStreams.back();
    }


    void addConversationsFile(
            onsem::SemanticLanguageEnum pLanguage,
            const std::string &pFilename,
            AAssetManager *pAssetMgr) {
        assetStreams.push_back(std::make_unique<AssetIstream>(pAssetMgr, pFilename));
        linguisticDatabaseStreams.languageToStreams[pLanguage].conversionsStreams.emplace(
                pFilename, &*assetStreams.back());
    }

};


#endif // SEMANTIC_ANDROID_KEYTOFASSETSTREAMS_HPP
