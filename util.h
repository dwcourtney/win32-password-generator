#pragma once

#include <stdbool.h>
#include <windows.h>

bool fileExists(const char* fileName);
void centerWindow(HWND hwnd);
int messageBoxCentered(HWND hwnd, LPCWSTR text, LPCWSTR caption, UINT type);
void getBuildDate(wchar_t* buffer, size_t size);
