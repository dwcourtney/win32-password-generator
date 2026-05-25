#include <windows.h>
#include <commctrl.h>

#include "main.h"
#include "password.h"
#include "ui.h"
#include "util.h"
#include "settings.h"
#include "state.h"
#include <string.h>
#include <sal.h>
#include <stdio.h>

#pragma warning(disable : 4996)
#pragma comment(lib, "comctl32.lib")

// Forward declaration of the window procedure
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK AboutWndProc(HWND, UINT, WPARAM, LPARAM);

static bool generatePasswordChoice(HWND hPasswordTextBox) {

    passWord = generatePassword();

    if (passWord == 0) {
        return false;
    }

    SetWindowTextW(hPasswordTextBox, passWord);

    return true;
}

static bool generatePasswordChoices(void) {

    SetWindowTextW(hOut, L"");
    SetWindowTextW(hOut2, L"");
    SetWindowTextW(hOut3, L"");
    SetWindowTextW(hOut4, L"");

    if (!generatePasswordChoice(hOut)) {
        return false;
    }

    if (!generatePasswordChoice(hOut2)) {
        return false;
    }

    if (!generatePasswordChoice(hOut3)) {
        return false;
    }

    if (!generatePasswordChoice(hOut4)) {
        return false;
    }

    return true;
}

static int copyPasswordText(HWND hwnd, HWND hPasswordTextBox) {

    HGLOBAL hGlobal;
    wchar_t* pGlobal;

    passTextLength = GetWindowTextLengthW(hPasswordTextBox) + 1;

    GetWindowTextW(hPasswordTextBox, passText, passTextLength);

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

    return 0;
}

static bool isPasswordTextBox(HWND hwnd) {

    return hwnd == hOut ||
        hwnd == hOut2 ||
        hwnd == hOut3 ||
        hwnd == hOut4;
}

static void showAboutDialog(HWND hwnd) {

    static bool aboutClassRegistered = false;
    wchar_t buildDate[64];
    RECT ownerRect = { 0 };
    RECT aboutRect = { 0 };
    HWND hAbout;
    HWND hControl;
    MSG msg;
    int aboutWidth = 360;
    int aboutHeight = 220;
    int labelX = 20;
    int labelWidth = 135;
    int valueX = 165;
    int valueWidth = 160;
    int rowHeight = 24;
    int rowY = 35;

    if (!aboutClassRegistered) {

        WNDCLASSW wc = { 0 };

        wc.lpszClassName = L"Password Generator About";
        wc.hInstance = GetModuleHandleW(NULL);
        wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
        wc.lpfnWndProc = AboutWndProc;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);

        RegisterClassW(&wc);
        aboutClassRegistered = true;
    }

    getBuildDate(buildDate, 64);

    hAbout = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"Password Generator About",
        L"About",
        WS_CAPTION | WS_SYSMENU | WS_POPUP,
        0,
        0,
        aboutWidth,
        aboutHeight,
        hwnd,
        NULL,
        GetModuleHandleW(NULL),
        NULL
    );

    if (hAbout == NULL) {
        return;
    }

    hControl = CreateWindowW(L"static", L"Author:",
        WS_VISIBLE | WS_CHILD | SS_RIGHT,
        labelX, rowY, labelWidth, rowHeight,
        hAbout, NULL, NULL, NULL);
    SendMessageW(hControl, WM_SETFONT, (WPARAM)hUiFont, TRUE);

    hControl = CreateWindowW(L"static", L"David Courtney",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        valueX, rowY, valueWidth, rowHeight,
        hAbout, NULL, NULL, NULL);
    SendMessageW(hControl, WM_SETFONT, (WPARAM)hUiFont, TRUE);

    rowY += rowHeight;

    hControl = CreateWindowW(L"static", L"Originally Created:",
        WS_VISIBLE | WS_CHILD | SS_RIGHT,
        labelX, rowY, labelWidth, rowHeight,
        hAbout, NULL, NULL, NULL);
    SendMessageW(hControl, WM_SETFONT, (WPARAM)hUiFont, TRUE);

    hControl = CreateWindowW(L"static", L"August 2020",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        valueX, rowY, valueWidth, rowHeight,
        hAbout, NULL, NULL, NULL);
    SendMessageW(hControl, WM_SETFONT, (WPARAM)hUiFont, TRUE);

    rowY += rowHeight;

    hControl = CreateWindowW(L"static", L"Build Date:",
        WS_VISIBLE | WS_CHILD | SS_RIGHT,
        labelX, rowY, labelWidth, rowHeight,
        hAbout, NULL, NULL, NULL);
    SendMessageW(hControl, WM_SETFONT, (WPARAM)hUiFont, TRUE);

    hControl = CreateWindowW(L"static", buildDate,
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        valueX, rowY, valueWidth, rowHeight,
        hAbout, NULL, NULL, NULL);
    SendMessageW(hControl, WM_SETFONT, (WPARAM)hUiFont, TRUE);

    hControl = CreateWindowW(L"button", L"OK",
        WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        (aboutWidth - 90) / 2, 140, 90, 30,
        hAbout, (HMENU)IDOK, NULL, NULL);
    SendMessageW(hControl, WM_SETFONT, (WPARAM)hUiFont, TRUE);

    if (GetWindowRect(hwnd, &ownerRect) && GetWindowRect(hAbout, &aboutRect)) {

        int ownerWidth = ownerRect.right - ownerRect.left;
        int ownerHeight = ownerRect.bottom - ownerRect.top;
        int actualAboutWidth = aboutRect.right - aboutRect.left;
        int actualAboutHeight = aboutRect.bottom - aboutRect.top;

        SetWindowPos(
            hAbout,
            HWND_TOP,
            ownerRect.left + (ownerWidth - actualAboutWidth) / 2,
            ownerRect.top + (ownerHeight - actualAboutHeight) / 2,
            0,
            0,
            SWP_NOSIZE
        );
    }

    EnableWindow(hwnd, FALSE);
    ShowWindow(hAbout, SW_SHOW);

    while (IsWindow(hAbout) && GetMessageW(&msg, NULL, 0, 0)) {

        if (!IsDialogMessageW(hAbout, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    EnableWindow(hwnd, TRUE);
    SetActiveWindow(hwnd);
}

static void showSettingsSavedDialog(HWND hwnd) {

    static bool settingsSavedClassRegistered = false;
    RECT ownerRect = { 0 };
    RECT dialogRect = { 0 };
    HWND hDialog;
    HWND hControl;
    MSG msg;
    int dialogWidth = 260;
    int dialogHeight = 190;

    if (!settingsSavedClassRegistered) {

        WNDCLASSW wc = { 0 };

        wc.lpszClassName = L"Password Generator Settings Saved";
        wc.hInstance = GetModuleHandleW(NULL);
        wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
        wc.lpfnWndProc = AboutWndProc;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);

        RegisterClassW(&wc);
        settingsSavedClassRegistered = true;
    }

    hDialog = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"Password Generator Settings Saved",
        L"OK",
        WS_CAPTION | WS_SYSMENU | WS_POPUP,
        0,
        0,
        dialogWidth,
        dialogHeight,
        hwnd,
        NULL,
        GetModuleHandleW(NULL),
        NULL
    );

    if (hDialog == NULL) {
        return;
    }

    hControl = CreateWindowW(L"static", L"Settings Saved",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        20, 42, dialogWidth - 40, 28,
        hDialog, NULL, NULL, NULL);
    SendMessageW(hControl, WM_SETFONT, (WPARAM)hUiFont, TRUE);

    hControl = CreateWindowW(L"button", L"OK",
        WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        (dialogWidth - 90) / 2, 105, 90, 30,
        hDialog, (HMENU)IDOK, NULL, NULL);
    SendMessageW(hControl, WM_SETFONT, (WPARAM)hUiFont, TRUE);

    if (GetWindowRect(hwnd, &ownerRect) && GetWindowRect(hDialog, &dialogRect)) {

        int ownerWidth = ownerRect.right - ownerRect.left;
        int ownerHeight = ownerRect.bottom - ownerRect.top;
        int actualDialogWidth = dialogRect.right - dialogRect.left;
        int actualDialogHeight = dialogRect.bottom - dialogRect.top;

        SetWindowPos(
            hDialog,
            HWND_TOP,
            ownerRect.left + (ownerWidth - actualDialogWidth) / 2,
            ownerRect.top + (ownerHeight - actualDialogHeight) / 2,
            0,
            0,
            SWP_NOSIZE
        );
    }

    EnableWindow(hwnd, FALSE);
    ShowWindow(hDialog, SW_SHOW);

    while (IsWindow(hDialog) && GetMessageW(&msg, NULL, 0, 0)) {

        if (!IsDialogMessageW(hDialog, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    EnableWindow(hwnd, TRUE);
    SetActiveWindow(hwnd);
}

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
        690,
        370,
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

    switch (msg) {

    // Initialize all UI elements when the window is first created
    case WM_CREATE:

        createMenuItems(hwnd);
        createCheckBoxes(hwnd);
        createOtherControls(hwnd);
        centerWindow(hwnd);

        hPasswordBkBrush = CreateSolidBrush(RGB(255, 255, 255));

        // Generate the initial passwords displayed in the UI
        generatePasswordChoices();

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

            incSymbols = IsDlgButtonChecked(hwnd, ID_INCSYMBOLS);
            EnableWindow(GetDlgItem(hwnd, ID_MAXSYMBOLSEDIT), incSymbols);
            EnableWindow(GetDlgItem(hwnd, ID_MAXSYMBOLSUPDOWN), incSymbols);

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

            if (!generatePasswordChoices()) {
                MessageBoxW(hwnd, L"Not possible!", L"Error", MB_OK);
            }

            break;

        // Copy the first generated password to the system clipboard
        case ID_COPYBUTTON:

            return copyPasswordText(hwnd, hOut);

        // Copy the second generated password to the system clipboard
        case ID_COPYBUTTON2:

            return copyPasswordText(hwnd, hOut2);

        // Copy the third generated password to the system clipboard
        case ID_COPYBUTTON3:

            return copyPasswordText(hwnd, hOut3);

        // Copy the fourth generated password to the system clipboard
        case ID_COPYBUTTON4:

            return copyPasswordText(hwnd, hOut4);

            break;

        // Display the About dialog
        case IDM_HELP_ABOUT:

            showAboutDialog(hwnd);

            break;

        // Save current configuration to settings.json
        case IDM_FILE_SAVE:

            saveSettings();
            showSettingsSavedDialog(hwnd);

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

            if (GetDlgCtrlID(lpnmud->hdr.hwndFrom) == ID_UPDOWN) {

                passwordLength = lpnmud->iPos + lpnmud->iDelta;

                if (passwordLength < MIN_PASS_LENGTH) {
                    passwordLength = MIN_PASS_LENGTH;
                }

                if (passwordLength > MAX_PASS_LENGTH) {
                    passwordLength = MAX_PASS_LENGTH;
                }
            }
            else if (GetDlgCtrlID(lpnmud->hdr.hwndFrom) == ID_MAXSYMBOLSUPDOWN) {

                maxSymbols = lpnmud->iPos + lpnmud->iDelta;

                if (maxSymbols < 0) {
                    maxSymbols = 0;
                }

                if (maxSymbols > MAX_PASS_LENGTH) {
                    maxSymbols = MAX_PASS_LENGTH;
                }
            }
        }

        break;

    // Keep read-only password textboxes visually white
    case WM_CTLCOLORSTATIC:

        if (isPasswordTextBox((HWND)lParam) && hPasswordBkBrush != NULL) {
            SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
            SetBkColor((HDC)wParam, RGB(255, 255, 255));
            return (LRESULT)hPasswordBkBrush;
        }

        break;


    // Clean shutdown when the window is destroyed
    case WM_DESTROY:

        if (hUiFont != NULL) {
            DeleteObject(hUiFont);
            hUiFont = NULL;
        }

        if (hPasswordFont != NULL) {
            DeleteObject(hPasswordFont);
            hPasswordFont = NULL;
        }

        if (hPasswordBkBrush != NULL) {
            DeleteObject(hPasswordBkBrush);
            hPasswordBkBrush = NULL;
        }

        PostQuitMessage(0);

        break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK AboutWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {

    case WM_COMMAND:

        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            DestroyWindow(hwnd);
            return 0;
        }

        break;

    case WM_CLOSE:

        DestroyWindow(hwnd);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
