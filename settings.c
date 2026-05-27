#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "settings.h"
#include "state.h"
#include "util.h"
#include "main.h"
#include "cJSON/cJSON.h"

#pragma warning(disable : 4996)

static bool loadBoolSetting(cJSON* json, const char* name, bool defaultValue) {

    cJSON* data = cJSON_GetObjectItemCaseSensitive(json, name);

    if (cJSON_IsBool(data)) {
        return cJSON_IsTrue(data);
    }

    return defaultValue;
}

int8_t saveSettings(void) {

    FILE* filePointer;

    // Create a JSON object representing the current program settings
    cJSON* json = cJSON_CreateObject();
    if (json == NULL) {
        return 1;
    }

    // Populate the JSON object with current configuration values
    cJSON_AddBoolToObject(json, "allowNumbers", allowNumbers);
    cJSON_AddBoolToObject(json, "allowSymbols", allowSymbols);
    cJSON_AddBoolToObject(json, "allowLowerChars", allowLowerChars);
    cJSON_AddBoolToObject(json, "allowUpperChars", allowUpperChars);
    cJSON_AddBoolToObject(json, "avoidAmbChars", avoidAmbChars);
    cJSON_AddNumberToObject(json, "passwordLength", passwordLength);
    cJSON_AddNumberToObject(json, "maxSymbols", maxSymbols);

    // Convert the JSON object into a formatted JSON string
    char* jsonStr = cJSON_Print(json);
    if (jsonStr == NULL) {
        cJSON_Delete(json);
        return 1;
    }

    // Open the settings file for writing
    filePointer = fopen(jsonSettingsFile, "w");
    if (filePointer == NULL) {
        printf("Error: Unable to open the file.\n");
        cJSON_free(jsonStr);
        cJSON_Delete(json);
        return 1;
    }

    // Write the JSON string to disk
    if (fputs(jsonStr, filePointer) == EOF) {
        fclose(filePointer);
        cJSON_free(jsonStr);
        cJSON_Delete(json);
        return 1;
    }

    fclose(filePointer);

    // Release memory allocated by cJSON
    cJSON_free(jsonStr);
    cJSON_Delete(json);

    return 0;
}

int8_t loadSettings(void) {

    FILE* filePointer;

    // Initialize default configuration values
    allowNumbers = true;
    allowSymbols = true;
    allowLowerChars = true;
    allowUpperChars = true;
    avoidAmbChars = true;
    passwordLength = DEFAULT_PASS_LENGTH;
    maxSymbols = DEFAULT_MAX_SYMBOLS;

    // Attempt to load settings from file if it exists
    if (fileExists(jsonSettingsFile)) {

        filePointer = fopen(jsonSettingsFile, "r");
        if (filePointer == NULL) {
            return 1;
        }

        // Read the file contents into a buffer
        char buffer[1024];
        size_t len = fread(buffer, 1, sizeof(buffer) - 1, filePointer);
        fclose(filePointer);

        if (len == 0) {
            return 1;
        }

        buffer[len] = '\0';

        // Parse the JSON data into a cJSON object
        cJSON* json = cJSON_Parse(buffer);
        if (json == NULL) {
            return 1;
        }

        cJSON* data;

        // Extract and apply each configuration setting
        allowNumbers = loadBoolSetting(json, "allowNumbers", allowNumbers);
        allowSymbols = loadBoolSetting(json, "allowSymbols", allowSymbols);
        allowLowerChars = loadBoolSetting(json, "allowLowerChars", allowLowerChars);
        allowUpperChars = loadBoolSetting(json, "allowUpperChars", allowUpperChars);
        avoidAmbChars = loadBoolSetting(json, "avoidAmbChars", avoidAmbChars);

        data = cJSON_GetObjectItemCaseSensitive(json, "passwordLength");

        if (cJSON_IsNumber(data) && data->valueint != 0) {

            passwordLength = data->valueint;

            if (passwordLength < MIN_PASS_LENGTH || passwordLength > MAX_PASS_LENGTH) {
                passwordLength = DEFAULT_PASS_LENGTH;
            }
        }

        data = cJSON_GetObjectItemCaseSensitive(json, "maxSymbols");

        if (cJSON_IsNumber(data)) {

            maxSymbols = data->valueint;

            if (maxSymbols < 0 || maxSymbols > passwordLength) {
                maxSymbols = DEFAULT_MAX_SYMBOLS;

                if (maxSymbols > passwordLength) {
                    maxSymbols = passwordLength;
                }
            }
        }

        // Free the parsed JSON object
        cJSON_Delete(json);
    }

    return 0;
}
