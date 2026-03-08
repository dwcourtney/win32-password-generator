#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "util.h"
#include <string.h>

#pragma warning(disable : 4996)

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

void getBuildDate(wchar_t* buffer, size_t size)
{
    const char* build = __DATE__;

    char mon[4] = { 0 }; // ensure zero-termination to satisfy static analysis
    int day;
    int year;

    // Use secure function and check return value to satisfy C6031.
    int ret = sscanf_s(build, "%3s %d %d", mon, (unsigned)sizeof(mon), &day, &year);
    if (ret != 3) {
        // Ensure buffer is a valid empty string on failure.
        if (size > 0) buffer[0] = L'\0';
        return;
    }

    const wchar_t* month = L"";

    // Use strncmp to compare only the first 3 characters (defensive against any non-NUL termination)
    if (strncmp(mon, "Jan", 3) == 0) month = L"January";
    else if (strncmp(mon, "Feb", 3) == 0) month = L"February";
    else if (strncmp(mon, "Mar", 3) == 0) month = L"March";
    else if (strncmp(mon, "Apr", 3) == 0) month = L"April";
    else if (strncmp(mon, "May", 3) == 0) month = L"May";
    else if (strncmp(mon, "Jun", 3) == 0) month = L"June";
    else if (strncmp(mon, "Jul", 3) == 0) month = L"July";
    else if (strncmp(mon, "Aug", 3) == 0) month = L"August";
    else if (strncmp(mon, "Sep", 3) == 0) month = L"September";
    else if (strncmp(mon, "Oct", 3) == 0) month = L"October";
    else if (strncmp(mon, "Nov", 3) == 0) month = L"November";
    else if (strncmp(mon, "Dec", 3) == 0) month = L"December";

    swprintf(buffer, size, L"%02d %ls %d", day, month, year);
}