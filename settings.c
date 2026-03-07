#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "settings.h"
#include "state.h"
#include "util.h"
#include "main.h"
#include "cJSON/cJSON.h"

#pragma warning(disable : 4996)

int8_t saveSettings(void) {

    FILE* filePointer;

    // Create a JSON object representing the current program settings
    cJSON* json = cJSON_CreateObject();
    if (json == NULL) {
        return 1;
    }

    // Populate the JSON object with current configuration values
    cJSON_AddBoolToObject(json, "incNumbers", incNumbers);
    cJSON_AddBoolToObject(json, "incSymbols", incSymbols);
    cJSON_AddBoolToObject(json, "incLowerChars", incLowerChars);
    cJSON_AddBoolToObject(json, "incUpperChars", incUpperChars);
    cJSON_AddBoolToObject(json, "avoidAmbChars", avoidAmbChars);
    cJSON_AddNumberToObject(json, "passwordLength", passwordLength);

    // Convert the JSON object into a compact JSON string
    char* jsonStr = cJSON_PrintUnformatted(json);
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
    incNumbers = true;
    incSymbols = true;
    incLowerChars = true;
    incUpperChars = true;
    avoidAmbChars = false;
    passwordLength = DEFAULT_PASS_LENGTH;

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
        data = cJSON_GetObjectItemCaseSensitive(json, "incNumbers");
        if (cJSON_IsBool(data) && cJSON_IsFalse(data)) {
            incNumbers = false;
        }

        data = cJSON_GetObjectItemCaseSensitive(json, "incSymbols");
        if (cJSON_IsBool(data) && cJSON_IsFalse(data)) {
            incSymbols = false;
        }

        data = cJSON_GetObjectItemCaseSensitive(json, "incLowerChars");
        if (cJSON_IsBool(data) && cJSON_IsFalse(data)) {
            incLowerChars = false;
        }

        data = cJSON_GetObjectItemCaseSensitive(json, "incUpperChars");
        if (cJSON_IsBool(data) && cJSON_IsFalse(data)) {
            incUpperChars = false;
        }

        data = cJSON_GetObjectItemCaseSensitive(json, "avoidAmbChars");
        if (cJSON_IsBool(data) && cJSON_IsTrue(data)) {
            avoidAmbChars = true;
        }

        data = cJSON_GetObjectItemCaseSensitive(json, "passwordLength");

        if (cJSON_IsNumber(data) && data->valueint != 0) {

            passwordLength = data->valueint;

            if (passwordLength < MIN_PASS_LENGTH || passwordLength > MAX_PASS_LENGTH) {
                passwordLength = DEFAULT_PASS_LENGTH;
            }
        }

        // Free the parsed JSON object
        cJSON_Delete(json);
    }

    return 0;
}