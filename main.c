#include <windows.h>
#include <commctrl.h>

#include "main.h"
#include "password.h"
#include "ui.h"
#include "util.h"
#include "settings.h"
#include "state.h"

#pragma warning(disable : 4996)
#pragma comment(lib, "comctl32.lib")

// Forward declaration of the window procedure
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


// Program entry point and main message loop
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ PWSTR lpCmdLine,
    _In_ int nCmdShow
) {

    // Load saved user configuration from settings.json
    loadSettings();

    MSG msg;
    WNDCLASSW wc = { 0 };

    // Define the window class used by the application
    wc.lpszClassName = L"Password Generator";
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDR_ICO_MAIN));
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc.lpfnWndProc = WndProc;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    // Register the window class with the operating system
    RegisterClassW(&wc);

    // Create the main application window
    CreateWindowW(
        wc.lpszClassName,
        L"Password Generator",
        WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE,
        0,
        0,
        640,
        250,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    // Standard Windows message processing loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}


// Main window message handler
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    LPNMUPDOWN lpnmud;
    INT code;
    HGLOBAL hGlobal;
    wchar_t* pGlobal;

    switch (msg) {

    // Initialize all UI elements when the window is first created
    case WM_CREATE:

        createMenuItems(hwnd);
        createCheckBoxes(hwnd);
        createOtherControls(hwnd);
        centerWindow(hwnd);

        // Generate the initial password displayed in the UI
        SetWindowTextW(hOut, L"");
        passWord = generatePassword();

        if (passWord != 0) {
            SetWindowTextW(hOut, passWord);
        }

        break;


    // Handle button presses, checkbox toggles, and menu selections
    case WM_COMMAND:

        // Read current checkbox states
        incNumbers = IsDlgButtonChecked(hwnd, ID_INCNUMBERS);
        incSymbols = IsDlgButtonChecked(hwnd, ID_INCSYMBOLS);
        incLowerChars = IsDlgButtonChecked(hwnd, ID_INCLOWERCHARS);
        incUpperChars = IsDlgButtonChecked(hwnd, ID_INCUPPERCHARS);
        avoidAmbChars = IsDlgButtonChecked(hwnd, ID_AVOIDAMBCHARS);

        switch (LOWORD(wParam)) {

        // Toggle "Include Numbers"
        case ID_INCNUMBERS:

            if (incNumbers) {
                CheckDlgButton(hwnd, ID_INCNUMBERS, BST_UNCHECKED);
            }
            else {
                CheckDlgButton(hwnd, ID_INCNUMBERS, BST_CHECKED);
            }

            break;

        // Toggle "Include Symbols"
        case ID_INCSYMBOLS:

            if (incSymbols) {
                CheckDlgButton(hwnd, ID_INCSYMBOLS, BST_UNCHECKED);
            }
            else {
                CheckDlgButton(hwnd, ID_INCSYMBOLS, BST_CHECKED);
            }

            break;

        // Toggle "Include Lowercase Characters"
        case ID_INCLOWERCHARS:

            if (incLowerChars) {
                CheckDlgButton(hwnd, ID_INCLOWERCHARS, BST_UNCHECKED);
            }
            else {
                CheckDlgButton(hwnd, ID_INCLOWERCHARS, BST_CHECKED);
            }

            break;

        // Toggle "Include Uppercase Characters"
        case ID_INCUPPERCHARS:

            if (incUpperChars) {
                CheckDlgButton(hwnd, ID_INCUPPERCHARS, BST_UNCHECKED);
            }
            else {
                CheckDlgButton(hwnd, ID_INCUPPERCHARS, BST_CHECKED);
            }

            break;

        // Toggle "Avoid Ambiguous Characters"
        case ID_AVOIDAMBCHARS:

            if (avoidAmbChars) {
                CheckDlgButton(hwnd, ID_AVOIDAMBCHARS, BST_UNCHECKED);
            }
            else {
                CheckDlgButton(hwnd, ID_AVOIDAMBCHARS, BST_CHECKED);
            }

            break;

        // Generate a new password
        case ID_GENERATEBUTTON:

            SetWindowTextW(hOut, L"");
            passWord = generatePassword();

            if (passWord == 0) {
                MessageBoxW(hwnd, L"Not possible!", L"Error", MB_OK);
            }
            else {
                SetWindowTextW(hOut, passWord);
            }

            break;

        // Copy the generated password to the system clipboard
        case ID_COPYBUTTON:

            passTextLength = GetWindowTextLengthW(hOut) + 1;

            GetWindowTextW(hOut, passText, passTextLength);

            // Allocate memory for clipboard transfer
            hGlobal = GlobalAlloc(GHND | GMEM_SHARE, passTextLength * sizeof(wchar_t));

            if (hGlobal == NULL) {
                MessageBoxW(hwnd, L"GlobalAlloc() returned NULL", L"Error", MB_OK);
                return 1;
            }

            pGlobal = (wchar_t*)GlobalLock(hGlobal);

            if (pGlobal == NULL) {
                MessageBoxW(hwnd, L"GlobalLock() returned NULL", L"Error", MB_OK);
                GlobalFree(hGlobal);
                return 1;
            }

            memcpy(pGlobal, passText, passTextLength * sizeof(wchar_t));

            GlobalUnlock(hGlobal);

            // Attempt to open the system clipboard
            if (!OpenClipboard(hwnd)) {
                GlobalFree(hGlobal);
                MessageBoxW(hwnd, L"Unable to open clipboard", L"Error", MB_OK);
                return 1;
            }

            // Transfer ownership of the memory block to the clipboard
            EmptyClipboard();
            SetClipboardData(CF_UNICODETEXT, hGlobal);
            CloseClipboard();

            break;

        // Display the About dialog
        case IDM_HELP_ABOUT:

            MessageBoxW(
                hwnd,
                L"Author: David Courtney\n\nBuild Date: 07 March 2026\n",
                L"About",
                MB_OK
            );

            break;

        // Save current configuration to settings.json
        case IDM_FILE_SAVE:

            saveSettings();
            MessageBoxW(hwnd, L"Settings Saved", L"OK", MB_OK);

            break;

        // Exit the application
        case IDM_FILE_QUIT:

            SendMessage(hwnd, WM_CLOSE, 0, 0);

            break;
        }

        break;


    // Handle password length changes from the up/down control
    case WM_NOTIFY:

        code = ((LPNMHDR)lParam)->code;

        if (code == UDN_DELTAPOS) {

            lpnmud = (NMUPDOWN*)lParam;

            passwordLength = lpnmud->iPos + lpnmud->iDelta;

            if (passwordLength < MIN_PASS_LENGTH) {
                passwordLength = MIN_PASS_LENGTH;
            }

            if (passwordLength > MAX_PASS_LENGTH) {
                passwordLength = MAX_PASS_LENGTH;
            }
        }

        break;


    // Clean shutdown when the window is destroyed
    case WM_DESTROY:

        PostQuitMessage(0);

        break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}