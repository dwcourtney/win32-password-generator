#include <windows.h>
#include <commctrl.h>

#include "main.h"
#include "password.h"
#include "ui.h"
#include "util.h"
#include "settings.h"
#include "state.h"
#include <ctype.h>
#include <string.h>
#include <sal.h>
#include <stdio.h>

#pragma warning(disable : 4996)
#pragma comment(lib, "comctl32.lib")

// Forward declaration of the window procedure
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK AboutWndProc(HWND, UINT, WPARAM, LPARAM);

#define BIGINT_MAX_DIGITS 128

typedef struct BigInt {
    int digits[BIGINT_MAX_DIGITS];
    int length;
} BigInt;

static wchar_t passwordSpaceStatusText[512];

static bool generatePasswordChoice(HWND hPasswordTextBox) {

    passWord = generatePassword();

    if (passWord == 0) {
        return false;
    }

    SetWindowTextW(hPasswordTextBox, passWord);

    return true;
}

static bool generatePasswordChoices(void) {

    SetWindowTextW(hPasswordBox1, L"");
    SetWindowTextW(hPasswordBox2, L"");
    SetWindowTextW(hPasswordBox3, L"");
    SetWindowTextW(hPasswordBox4, L"");

    if (!generatePasswordChoice(hPasswordBox1)) {
        return false;
    }

    if (!generatePasswordChoice(hPasswordBox2)) {
        return false;
    }

    if (!generatePasswordChoice(hPasswordBox3)) {
        return false;
    }

    if (!generatePasswordChoice(hPasswordBox4)) {
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

    return hwnd == hPasswordBox1 ||
        hwnd == hPasswordBox2 ||
        hwnd == hPasswordBox3 ||
        hwnd == hPasswordBox4;
}

static void bigIntSetZero(BigInt* value) {

    memset(value->digits, 0, sizeof(value->digits));
    value->length = 1;
}

static void bigIntSetUInt(BigInt* value, uint32_t number) {

    bigIntSetZero(value);

    if (number == 0) {
        return;
    }

    value->length = 0;

    while (number > 0 && value->length < BIGINT_MAX_DIGITS) {
        value->digits[value->length++] = number % 10;
        number /= 10;
    }
}

static void bigIntAdd(BigInt* value, const BigInt* addend) {

    int carry = 0;
    int maxLength = value->length > addend->length ? value->length : addend->length;

    for (int i = 0; i < maxLength || carry; i++) {

        int sum = carry;

        if (i < value->length) {
            sum += value->digits[i];
        }

        if (i < addend->length) {
            sum += addend->digits[i];
        }

        value->digits[i] = sum % 10;
        carry = sum / 10;

        if (i >= value->length) {
            value->length = i + 1;
        }
    }
}

static void bigIntMultiplyUInt(BigInt* value, uint32_t factor) {

    uint32_t carry = 0;

    if (factor == 0) {
        bigIntSetZero(value);
        return;
    }

    for (int i = 0; i < value->length; i++) {

        uint32_t product = (uint32_t)value->digits[i] * factor + carry;

        value->digits[i] = product % 10;
        carry = product / 10;
    }

    while (carry > 0 && value->length < BIGINT_MAX_DIGITS) {
        value->digits[value->length++] = carry % 10;
        carry /= 10;
    }
}

static void bigIntMultiply(BigInt* value, const BigInt* factor) {

    BigInt result;

    bigIntSetZero(&result);

    for (int i = 0; i < value->length; i++) {

        int carry = 0;

        for (int j = 0; j < factor->length || carry; j++) {

            int index = i + j;
            int product = result.digits[index] + carry;

            if (j < factor->length) {
                product += value->digits[i] * factor->digits[j];
            }

            result.digits[index] = product % 10;
            carry = product / 10;

            if (index >= result.length) {
                result.length = index + 1;
            }
        }
    }

    while (result.length > 1 && result.digits[result.length - 1] == 0) {
        result.length--;
    }

    *value = result;
}

static void bigIntPowerUInt(BigInt* value, uint32_t base, int exponent) {

    bigIntSetUInt(value, 1);

    for (int i = 0; i < exponent; i++) {
        bigIntMultiplyUInt(value, base);
    }
}

static void bigIntCombination(BigInt* value, int n, int k) {

    BigInt row[MAX_PASS_LENGTH + 1];

    if (k < 0 || k > n) {
        bigIntSetZero(value);
        return;
    }

    for (int i = 0; i <= n; i++) {
        bigIntSetZero(&row[i]);
    }

    bigIntSetUInt(&row[0], 1);

    for (int i = 1; i <= n; i++) {

        for (int j = i; j > 0; j--) {
            bigIntAdd(&row[j], &row[j - 1]);
        }
    }

    *value = row[k];
}

static void bigIntFormat(const BigInt* value, wchar_t* buffer, size_t size) {

    wchar_t plain[BIGINT_MAX_DIGITS + 1] = { 0 };
    wchar_t grouped[BIGINT_MAX_DIGITS + (BIGINT_MAX_DIGITS / 3) + 1] = { 0 };
    int plainIndex = 0;
    int groupedIndex = 0;

    for (int i = value->length - 1; i >= 0; i--) {
        plain[plainIndex++] = (wchar_t)(L'0' + value->digits[i]);
    }

    plain[plainIndex] = L'\0';

    for (int i = 0; i < plainIndex; i++) {

        if (i > 0 && ((plainIndex - i) % 3) == 0) {
            grouped[groupedIndex++] = L',';
        }

        grouped[groupedIndex++] = plain[i];
    }

    grouped[groupedIndex] = L'\0';

    swprintf(buffer, size, L"Possible Passwords: %ls", grouped);
}

static void syncSettingsFromControls(HWND hwnd) {

    BOOL translated = FALSE;
    UINT value;

    incNumbers = IsDlgButtonChecked(hwnd, ID_INCNUMBERS) == BST_CHECKED;
    incSymbols = IsDlgButtonChecked(hwnd, ID_INCSYMBOLS) == BST_CHECKED;
    incLowerChars = IsDlgButtonChecked(hwnd, ID_INCLOWERCHARS) == BST_CHECKED;
    incUpperChars = IsDlgButtonChecked(hwnd, ID_INCUPPERCHARS) == BST_CHECKED;
    avoidAmbChars = IsDlgButtonChecked(hwnd, ID_AVOIDAMBCHARS) == BST_CHECKED;

    value = GetDlgItemInt(hwnd, ID_EDIT, &translated, FALSE);

    if (translated) {

        if (value < MIN_PASS_LENGTH) {
            value = MIN_PASS_LENGTH;
        }

        if (value > MAX_PASS_LENGTH) {
            value = MAX_PASS_LENGTH;
        }

        passwordLength = (int8_t)value;
    }

    value = GetDlgItemInt(hwnd, ID_MAXSYMBOLSEDIT, &translated, FALSE);

    if (translated) {

        if (value > MAX_PASS_LENGTH) {
            value = MAX_PASS_LENGTH;
        }

        maxSymbols = (int8_t)value;
    }
}

static void updatePasswordSpaceStatus(HWND hwnd) {

    BigInt total;
    uint32_t symbolCount = 0;
    uint32_t nonSymbolCount = 0;
    uint32_t poolCount = 0;

    if (hStatus == NULL) {
        return;
    }

    syncSettingsFromControls(hwnd);

    for (uint8_t c = 33; c <= 126; c++) {

        if (!incNumbers && isdigit(c)) continue;
        if (!incSymbols && ispunct(c)) continue;
        if (!incLowerChars && islower(c)) continue;
        if (!incUpperChars && isupper(c)) continue;

        if (avoidAmbChars &&
            (c == '0' || c == '1' ||
                c == 'I' || c == 'O' ||
                c == 'i' || c == 'l' ||
                c == 'o' || c == '|'))
        {
            continue;
        }

        if (ispunct(c)) {
            symbolCount++;
        }
        else {
            nonSymbolCount++;
        }
    }

    poolCount = symbolCount + nonSymbolCount;

    if (poolCount == 0) {
        bigIntSetZero(&total);
    }
    else if (!incSymbols || symbolCount == 0 || maxSymbols >= passwordLength) {
        bigIntPowerUInt(&total, poolCount, passwordLength);
    }
    else {

        int maxSymbolCount = maxSymbols < passwordLength ? maxSymbols : passwordLength;

        bigIntSetZero(&total);

        for (int i = 0; i <= maxSymbolCount; i++) {

            BigInt term;
            BigInt multiplier;

            bigIntCombination(&term, passwordLength, i);

            bigIntPowerUInt(&multiplier, symbolCount, i);
            bigIntMultiply(&term, &multiplier);

            bigIntPowerUInt(&multiplier, nonSymbolCount, passwordLength - i);
            bigIntMultiply(&term, &multiplier);

            bigIntAdd(&total, &term);
        }
    }

    bigIntFormat(&total, passwordSpaceStatusText, sizeof(passwordSpaceStatusText) / sizeof(passwordSpaceStatusText[0]));
    SendMessageW(hStatus, SB_SETTEXTW, SBT_OWNERDRAW, 0);
    InvalidateRect(hStatus, NULL, TRUE);
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

static void showErrorDialog(HWND hwnd) {

    static bool errorClassRegistered = false;
    RECT ownerRect = { 0 };
    RECT dialogRect = { 0 };
    HWND hDialog;
    HWND hControl;
    MSG msg;
    int dialogWidth = 260;
    int dialogHeight = 190;

    if (!errorClassRegistered) {

        WNDCLASSW wc = { 0 };

        wc.lpszClassName = L"Password Generator Error";
        wc.hInstance = GetModuleHandleW(NULL);
        wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
        wc.lpfnWndProc = AboutWndProc;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);

        RegisterClassW(&wc);
        errorClassRegistered = true;
    }

    hDialog = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"Password Generator Error",
        L"Error",
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

    hControl = CreateWindowW(L"static", L"Not possible!",
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
        810,
        400,
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
        updatePasswordSpaceStatus(hwnd);

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
                showErrorDialog(hwnd);
            }

            break;

        // Copy the first generated password to the system clipboard
        case ID_COPYBUTTON:

            return copyPasswordText(hwnd, hPasswordBox1);

        // Copy the second generated password to the system clipboard
        case ID_COPYBUTTON2:

            return copyPasswordText(hwnd, hPasswordBox2);

        // Copy the third generated password to the system clipboard
        case ID_COPYBUTTON3:

            return copyPasswordText(hwnd, hPasswordBox3);

        // Copy the fourth generated password to the system clipboard
        case ID_COPYBUTTON4:

            return copyPasswordText(hwnd, hPasswordBox4);

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
            return 0;
        }

        updatePasswordSpaceStatus(hwnd);
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

            updatePasswordSpaceStatus(hwnd);
        }

        break;

    case WM_SIZE:

        if (hStatus != NULL) {
            SendMessageW(hStatus, WM_SIZE, 0, 0);
        }

        break;

    case WM_DRAWITEM:
    {
        DRAWITEMSTRUCT* drawItem = (DRAWITEMSTRUCT*)lParam;

        if (drawItem != NULL && drawItem->CtlID == ID_STATUSBAR) {

            RECT textRect = drawItem->rcItem;
            HFONT hOldFont = NULL;

            textRect.left += 10;

            FillRect(drawItem->hDC, &drawItem->rcItem, GetSysColorBrush(COLOR_3DFACE));
            SetBkMode(drawItem->hDC, TRANSPARENT);
            SetTextColor(drawItem->hDC, GetSysColor(COLOR_WINDOWTEXT));

            if (hUiFont != NULL) {
                hOldFont = (HFONT)SelectObject(drawItem->hDC, hUiFont);
            }

            DrawTextW(
                drawItem->hDC,
                passwordSpaceStatusText,
                -1,
                &textRect,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX
            );

            if (hOldFont != NULL) {
                SelectObject(drawItem->hDC, hOldFont);
            }

            return TRUE;
        }

        break;
    }

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

    case WM_SYSCOMMAND:

        if ((wParam & 0xFFF0) == SC_MOVE) {
            return 0;
        }

        break;

    case WM_NCLBUTTONDOWN:
    case WM_NCLBUTTONDBLCLK:

        if (wParam == HTCAPTION) {
            return 0;
        }

        break;

    case WM_NCHITTEST:
    {
        LRESULT hitTest = DefWindowProcW(hwnd, msg, wParam, lParam);

        if (hitTest == HTCAPTION) {
            return HTCLIENT;
        }

        return hitTest;
    }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
