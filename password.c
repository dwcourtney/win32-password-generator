/*
PSEUDOCODE / DETAILED PLAN:

- Purpose:
  Generate a cryptographically secure random password into the external
  wchar_t buffer `passwordArray` honoring user-selected character classes and
  avoiding ambiguous characters if requested.

- Steps:
  1. Validate `passwordLength` is within 1..MAX_PASS_LENGTH.
  2. Ensure at least one character class flag is enabled.
  3. Build an `allowed` character pool by iterating printable ASCII 33..126:
     - Skip digits if `incNumbers == false`.
     - Skip punctuation if `incSymbols == false`.
     - Skip lowercase letters if `incLowerChars == false`.
     - Skip uppercase letters if `incUpperChars == false`.
     - If `avoidAmbChars == true`, exclude ambiguous characters: 0,1,I,O,i,l,o,|.
     - Append allowed characters to `allowed` array and increment `allowedCount`.
  4. If `allowedCount == 0` return failure (NULL/0).
  5. Compute unbiased rejection sampling limit: limit = 256 - (256 % allowedCount).
  6. For each password position:
     - Repeatedly request one random byte until < limit.
     - Map to index = randomByte % allowedCount.
     - Store allowed[index] into `passwordArray[i]`.
  7. Shuffle the generated characters via Fisher–Yates using the same unbiased
     rejection sampling technique for each swap index.
  8. Null-terminate `passwordArray` and return it.

- Fix for lnt-uninitialized-local:
  Initialize local variables that static analyzers may flag as possibly
  uninitialized even if runtime initialization happens later:
  - Initialize `randomByte`, `index`, and the `allowed` array to zero.
  This does not change runtime logic but satisfies static analysis.
*/

#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <windows.h>

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

    // Initialize locals to satisfy static analyzers (lnt-uninitialized-local).
    // These initializations are harmless and keep runtime behavior unchanged.
    uint8_t randomByte = 0;
    uint8_t index = 0;

    // Maximum printable ASCII 33..126 inclusive => 94 characters
    wchar_t allowed[94] = {0};
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
            uint8_t j = 0;

            do {
                if (BCryptGenRandom(NULL, &randomByte, 1, BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) {
                    return 0;
                }
            } while (randomByte >= shuffleLimit);

            j = randomByte % (i + 1);

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