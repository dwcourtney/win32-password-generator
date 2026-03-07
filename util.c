#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#pragma warning(disable : 4996)

#include "util.h"


// Check if a file exists by attempting to open it for reading
bool fileExists(const char* fileName) {

    FILE* filePointer;

    filePointer = fopen(fileName, "r");

    if (filePointer == NULL) {
        return false;
    }

    fclose(filePointer);

    return true;
}


// Center the specified window on the primary display
void centerWindow(HWND hwnd) {

    // Validate the window handle before proceeding
    if (hwnd == NULL) {
        return;
    }

    RECT rc = { 0 };

    // Retrieve the current window position and size
    if (!GetWindowRect(hwnd, &rc)) {
        return;
    }

    int32_t win_w = rc.right - rc.left;
    int32_t win_h = rc.bottom - rc.top;

    // Retrieve the primary screen dimensions
    int32_t screen_w = GetSystemMetrics(SM_CXSCREEN);
    int32_t screen_h = GetSystemMetrics(SM_CYSCREEN);

    // Move the window to the center of the screen without resizing it
    SetWindowPos(
        hwnd,
        HWND_TOP,
        (screen_w - win_w) / 2,
        (screen_h - win_h) / 2,
        0,
        0,
        SWP_NOSIZE
    );
}