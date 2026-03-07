#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <windows.h>
#include <bcrypt.h>

#include "password.h"

#pragma comment(lib, "bcrypt.lib")

#define MAX_PASS_LENGTH 32

extern wchar_t passwordArray[MAX_PASS_LENGTH + 1];
extern int8_t passwordLength;
extern bool incNumbers;
extern bool incSymbols;
extern bool incLowerChars;
extern bool incUpperChars;
extern bool avoidAmbChars;

wchar_t* generatePassword(void) {

    uint8_t randomByte;
    uint8_t index;

    wchar_t allowed[94];
    uint8_t allowedCount = 0;

    // Validate password length bounds
    if (passwordLength < 1 || passwordLength > MAX_PASS_LENGTH) {
        return 0;
    }

    // Ensure at least one character category is enabled
    if (!(incNumbers || incSymbols || incLowerChars || incUpperChars)) {
        return 0;
    }

    // Build the allowed character pool based on user settings
    // We iterate through printable ASCII characters 33–126 and include
    // only those that pass the user's filtering rules.
    for (uint8_t c = 33; c <= 126; c++) {

        if (!incNumbers && isdigit(c)) continue;
        if (!incSymbols && ispunct(c)) continue;
        if (!incLowerChars && islower(c)) continue;
        if (!incUpperChars && isupper(c)) continue;

        // Optionally remove visually ambiguous characters
        if (avoidAmbChars &&
            (c == '0' || c == '1' ||
                c == 'I' || c == 'O' ||
                c == 'i' || c == 'l' ||
                c == 'o' || c == '|'))
        {
            continue;
        }

        // Add character to allowed pool
        allowed[allowedCount++] = (wchar_t)c;
    }

    // Abort if filtering removed all characters
    if (allowedCount == 0) {
        return 0;
    }

    // Compute rejection limit once to avoid modulo bias
    uint32_t limit = 256 - (256 % allowedCount);

    // Generate each password character using unbiased rejection sampling
    for (uint8_t i = 0; i < passwordLength; i++) {

        do {
            if (BCryptGenRandom(NULL, &randomByte, 1, BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) {
                return 0;
            }
        } while (randomByte >= limit);

        index = randomByte % allowedCount;
        passwordArray[i] = allowed[index];
    }

    // Shuffle characters using Fisher–Yates to randomize final ordering
    if (passwordLength > 1) {

        for (uint8_t i = passwordLength - 1; i > 0; i--) {

            uint32_t shuffleLimit = 256 - (256 % (i + 1));

            do {
                if (BCryptGenRandom(NULL, &randomByte, 1, BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) {
                    return 0;
                }
            } while (randomByte >= shuffleLimit);

            uint8_t j = randomByte % (i + 1);

            wchar_t temp = passwordArray[i];
            passwordArray[i] = passwordArray[j];
            passwordArray[j] = temp;
        }
    }

    // Null-terminate the password buffer
    passwordArray[passwordLength] = L'\0';

    // Return generated password
    return passwordArray;
}