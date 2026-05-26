#pragma once

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include "main.h"

// Pointer to the currently generated password
extern wchar_t* passWord;

// Global password buffers (+1 for null terminator)
extern wchar_t passwordArray[MAX_PASS_LENGTH + 1];
extern wchar_t passText[MAX_PASS_LENGTH + 1];

// Password length state
extern int8_t passwordLength;
extern int8_t passTextLength;
extern int8_t maxSymbols;

// Handle to password output textbox
extern HWND hPasswordBox1;
extern HWND hPasswordBox2;
extern HWND hPasswordBox3;
extern HWND hPasswordBox4;
extern HWND hStatus;
extern HFONT hUiFont;
extern HFONT hPasswordFont;
extern HBRUSH hPasswordBkBrush;

// User option flags
extern bool incNumbers;
extern bool incSymbols;
extern bool incLowerChars;
extern bool incUpperChars;
extern bool avoidAmbChars;

// Path to settings JSON file
extern const char* jsonSettingsFile;
