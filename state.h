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
extern HWND hGenerateButton;
extern HWND hCopyButton1;
extern HWND hCopyButton2;
extern HWND hCopyButton3;
extern HWND hCopyButton4;
extern HWND hStatus;
extern HFONT hUiFont;
extern HFONT hPasswordFont;
extern HFONT hPasswordItalicFont;
extern HBRUSH hPasswordBkBrush;
extern HBRUSH hPasswordImpossibleBkBrush;
extern bool passwordGenerationPossible;

// User option flags
extern bool allowNumbers;
extern bool allowSymbols;
extern bool allowLowerChars;
extern bool allowUpperChars;
extern bool avoidAmbChars;

// Path to settings JSON file
extern const char* jsonSettingsFile;
