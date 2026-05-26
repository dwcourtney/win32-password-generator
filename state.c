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
int8_t maxSymbols;

// Handle to the output textbox control
HWND hPasswordBox1;
HWND hPasswordBox2;
HWND hPasswordBox3;
HWND hPasswordBox4;
HWND hGenerateButton;
HWND hCopyButton1;
HWND hCopyButton2;
HWND hCopyButton3;
HWND hCopyButton4;
HWND hStatus;
HFONT hUiFont;
HFONT hPasswordFont;
HFONT hPasswordItalicFont;
HBRUSH hPasswordBkBrush;
HBRUSH hPasswordImpossibleBkBrush;
bool passwordGenerationPossible = true;

// User option flags controlling allowed character classes
bool incNumbers;
bool incSymbols;
bool incLowerChars;
bool incUpperChars;
bool avoidAmbChars;

// Path to the JSON settings file
const char* jsonSettingsFile = "settings.json";
