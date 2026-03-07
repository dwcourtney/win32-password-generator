#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

#include "state.h"
#include "main.h"

// Pointer to the currently generated password
wchar_t* passWord;

// Global password buffers (extra slot reserved for null terminator)
wchar_t passwordArray[MAX_PASS_LENGTH + 1] = { 0 };
wchar_t passText[MAX_PASS_LENGTH + 1] = { 0 };

// Password configuration state
int8_t passwordLength;
int8_t passTextLength;

// Handle to the output textbox control
HWND hOut;

// User option flags controlling allowed character classes
bool incNumbers;
bool incSymbols;
bool incLowerChars;
bool incUpperChars;
bool avoidAmbChars;

// Path to the JSON settings file
const char* jsonSettingsFile = "settings.json";