/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * GitHub Client Library - enables checking for latest release and downloading assets to file system
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/githubClient/githubClient.h
 * Description: Exported functions and classes.
 *
 * Exported Functions/Classes:
 * - github: Class definition
 * - getLatestRelease: Get latest release
 * - getLastError: Get last error
 * - whitespace: Whitespace
 * - startDocument: Start document
 * - key: Key
 * - value: Value
 * - endArray: End array
 * - endObject: End object
 * - endDocument: End document
 * - startArray: Start array
 * - startObject: Start object
 */
#pragma once
#include <JsonListener.h>
#include <JsonStreamingParser.h>
#include <md5Utils.h>

#define MAX_RELEASE_ASSETS 16   //  The maximum number of release asset details that will be read and stored

class github: public JsonListener {

    private:
        const char* apiHost = "api.github.com";
        String apiGetLatestRelease = "/repos/gadec-uk/departures-board/releases/latest";
        String currentKey = "";
        String currentArray = "";
        String currentObject = "";
        String previousObject = "";

        String lastErrorMsg = "";

        String assetURL;
        String assetName;
        md5Utils md5;

    public:
        String accessToken;
        String releaseId;
        String releaseDescription;
        int releaseAssets;
        String releaseAssetURL[MAX_RELEASE_ASSETS];
        String releaseAssetName[MAX_RELEASE_ASSETS];

        github(String repository, String token);

/**
 * @brief Get latest release
 * @return Return value
 */
        bool getLatestRelease();
        //bool downloadAssetToLittleFS(String url, String filename);

        String getLastError();

/**
 * @brief Whitespace
 * @param c
 * @return Return value
 */
        virtual void whitespace(char c);
/**
 * @brief Start document
 * @return Return value
 */
        virtual void startDocument();
/**
 * @brief Key
 * @param key
 * @return Return value
 */
        virtual void key(String key);
/**
 * @brief Value
 * @param value
 * @return Return value
 */
        virtual void value(String value);
/**
 * @brief End array
 * @return Return value
 */
        virtual void endArray();
/**
 * @brief End object
 * @return Return value
 */
        virtual void endObject();
/**
 * @brief End document
 * @return Return value
 */
        virtual void endDocument();
/**
 * @brief Start array
 * @return Return value
 */
        virtual void startArray();
/**
 * @brief Start object
 * @return Return value
 */
        virtual void startObject();
};
