#pragma once

#include <stdbool.h>
#include <windows.h>

bool fileExists(const char* fileName);
void centerWindow(HWND hwnd);
void getBuildDate(wchar_t* buffer, size_t size);