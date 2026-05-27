#include <windows.h>
#include <commctrl.h>

#include "ui.h"
#include "main.h"
#include "state.h"

#define SETTINGS_GROUP_X              10
#define SETTINGS_GROUP_Y              5
#define SETTINGS_GROUP_WIDTH          260
#define SETTINGS_GROUP_HEIGHT         285
#define SETTINGS_GROUP_TOP_PADDING    26
#define SETTINGS_GROUP_BOTTOM_PADDING 22

static void createFonts(void) {

    if (hUiFont == NULL) {
        hUiFont = CreateFontW(
            -15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
        );
    }

    if (hPasswordFont == NULL) {
        hPasswordFont = CreateFontW(
            -16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas"
        );
    }

    if (hPasswordItalicFont == NULL) {
        hPasswordItalicFont = CreateFontW(
            -16, 0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas"
        );
    }
}

static void setUiFont(HWND hwnd) {

    if (hUiFont != NULL) {
        SendMessageW(hwnd, WM_SETFONT, (WPARAM)hUiFont, TRUE);
    }
}

static void setPasswordFont(HWND hwnd) {

    if (hPasswordFont != NULL) {
        SendMessageW(hwnd, WM_SETFONT, (WPARAM)hPasswordFont, TRUE);
    }
}

static void centerPasswordText(HWND hwnd, UINT width, UINT height) {

    HDC hdc;
    HFONT hOldFont = NULL;
    TEXTMETRICW textMetric = { 0 };
    RECT textRect = { 0 };
    int textY;

    if (hPasswordFont == NULL) {
        return;
    }

    hdc = GetDC(hwnd);

    if (hdc == NULL) {
        return;
    }

    hOldFont = (HFONT)SelectObject(hdc, hPasswordFont);
    if (!GetTextMetricsW(hdc, &textMetric)) {
        SelectObject(hdc, hOldFont);
        ReleaseDC(hwnd, hdc);
        return;
    }

    SelectObject(hdc, hOldFont);
    ReleaseDC(hwnd, hdc);

    textY = ((int)height - textMetric.tmHeight) / 2;

    if (textY < 0) {
        textY = 0;
    }

    textRect.left = 3;
    textRect.top = textY;
    textRect.right = (LONG)width - 3;
    textRect.bottom = (LONG)height;

    SendMessageW(hwnd, EM_SETRECTNP, 0, (LPARAM)&textRect);
}

static void addCopyTooltip(HWND hwnd, HWND hToolTip, HWND hButton) {

    TOOLINFOW toolInfo = { 0 };

    toolInfo.cbSize = sizeof(TOOLINFOW);
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    toolInfo.hwnd = hwnd;
    toolInfo.uId = (UINT_PTR)hButton;
    toolInfo.lpszText = L"Copy password";

    SendMessageW(hToolTip, TTM_ADDTOOLW, 0, (LPARAM)&toolInfo);
}

// Create the checkbox controls for password configuration
void createCheckBoxes(HWND hwnd) {

    HWND hControl;
    int checkBoxHeight = 32;
    int checkBoxWidth = 230;
    int checkBoxXpos = 30;
    int checkBoxYpos = SETTINGS_GROUP_Y + SETTINGS_GROUP_TOP_PADDING;
    int checkBoxYdiff = 29;

    createFonts();

    // Settings group box
    hControl = CreateWindowW(L"button", L"Settings",
        WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
        SETTINGS_GROUP_X, SETTINGS_GROUP_Y, SETTINGS_GROUP_WIDTH, SETTINGS_GROUP_HEIGHT,
        hwnd, NULL, NULL, NULL);
    setUiFont(hControl);

    // Allow Numbers checkbox
    hControl = CreateWindowW(L"button", L"Allow Numbers",
        WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
        checkBoxXpos, checkBoxYpos, checkBoxWidth, checkBoxHeight,
        hwnd, (HMENU)ID_INCNUMBERS, NULL, NULL);
    setUiFont(hControl);

    CheckDlgButton(hwnd, ID_INCNUMBERS, incNumbers ? BST_CHECKED : BST_UNCHECKED);

    // Allow Symbols checkbox
    checkBoxYpos += checkBoxYdiff;
    hControl = CreateWindowW(L"button", L"Allow Symbols",
        WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
        checkBoxXpos, checkBoxYpos, checkBoxWidth, checkBoxHeight,
        hwnd, (HMENU)ID_INCSYMBOLS, NULL, NULL);
    setUiFont(hControl);

    CheckDlgButton(hwnd, ID_INCSYMBOLS, incSymbols ? BST_CHECKED : BST_UNCHECKED);

    // Allow Lowercase checkbox
    checkBoxYpos += checkBoxYdiff;
    hControl = CreateWindowW(L"button", L"Allow Lowercase Characters",
        WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
        checkBoxXpos, checkBoxYpos, checkBoxWidth, checkBoxHeight,
        hwnd, (HMENU)ID_INCLOWERCHARS, NULL, NULL);
    setUiFont(hControl);

    CheckDlgButton(hwnd, ID_INCLOWERCHARS, incLowerChars ? BST_CHECKED : BST_UNCHECKED);

    // Allow Uppercase checkbox
    checkBoxYpos += checkBoxYdiff;
    hControl = CreateWindowW(L"button", L"Allow Uppercase Characters",
        WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
        checkBoxXpos, checkBoxYpos, checkBoxWidth, checkBoxHeight,
        hwnd, (HMENU)ID_INCUPPERCHARS, NULL, NULL);
    setUiFont(hControl);

    CheckDlgButton(hwnd, ID_INCUPPERCHARS, incUpperChars ? BST_CHECKED : BST_UNCHECKED);

    // Avoid ambiguous characters checkbox
    checkBoxYpos += checkBoxYdiff;
    hControl = CreateWindowW(L"button", L"Avoid Ambiguous Characters",
        WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
        checkBoxXpos, checkBoxYpos, checkBoxWidth, checkBoxHeight,
        hwnd, (HMENU)ID_AVOIDAMBCHARS, NULL, NULL);
    setUiFont(hControl);

    CheckDlgButton(hwnd, ID_AVOIDAMBCHARS, avoidAmbChars ? BST_CHECKED : BST_UNCHECKED);
}


// Create buttons, edit box, and numeric up-down control
void createOtherControls(HWND hwnd) {

    HWND hUpDown;
    HWND hEdit;
    HWND hMaxSymbolsUpDown;
    HWND hMaxSymbolsEdit;
    HWND hToolTip;
    HWND hControl;
    UINT boxWidth = 472;
    UINT passwordBoxWidth = 412;
    UINT copyButtonWidth = 52;
    UINT generateButtonHeight = 34;
    UINT passwordBoxHeight = 30;
    int contentTop = SETTINGS_GROUP_Y + SETTINGS_GROUP_TOP_PADDING;
    int contentBottom = SETTINGS_GROUP_Y + SETTINGS_GROUP_HEIGHT - SETTINGS_GROUP_BOTTOM_PADDING;
    int passwordGap = (
        contentBottom -
        contentTop -
        generateButtonHeight -
        (4 * passwordBoxHeight)
    ) / 4;
    int passwordY1 = contentTop + generateButtonHeight + passwordGap;
    int passwordY2 = passwordY1 + passwordBoxHeight + passwordGap;
    int passwordY3 = passwordY2 + passwordBoxHeight + passwordGap;
    int passwordY4 = contentBottom - passwordBoxHeight;

    createFonts();

    // Generate password button
    hGenerateButton = CreateWindowW(L"Button", L"Generate Passwords",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        290, contentTop, boxWidth, generateButtonHeight,
        hwnd, (HMENU)ID_GENERATEBUTTON, NULL, NULL);
    setUiFont(hGenerateButton);

    // Password output textboxes
    hPasswordBox1 = CreateWindowW(L"Edit", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER | ES_READONLY | ES_MULTILINE,
        290, passwordY1, passwordBoxWidth, passwordBoxHeight,
        hwnd, (HMENU)ID_PASSWORDTEXTBOX, NULL, NULL);
    setPasswordFont(hPasswordBox1);
    centerPasswordText(hPasswordBox1, passwordBoxWidth, passwordBoxHeight);

    hPasswordBox2 = CreateWindowW(L"Edit", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER | ES_READONLY | ES_MULTILINE,
        290, passwordY2, passwordBoxWidth, passwordBoxHeight,
        hwnd, (HMENU)ID_PASSWORDTEXTBOX2, NULL, NULL);
    setPasswordFont(hPasswordBox2);
    centerPasswordText(hPasswordBox2, passwordBoxWidth, passwordBoxHeight);

    hPasswordBox3 = CreateWindowW(L"Edit", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER | ES_READONLY | ES_MULTILINE,
        290, passwordY3, passwordBoxWidth, passwordBoxHeight,
        hwnd, (HMENU)ID_PASSWORDTEXTBOX3, NULL, NULL);
    setPasswordFont(hPasswordBox3);
    centerPasswordText(hPasswordBox3, passwordBoxWidth, passwordBoxHeight);

    hPasswordBox4 = CreateWindowW(L"Edit", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER | ES_READONLY | ES_MULTILINE,
        290, passwordY4, passwordBoxWidth, passwordBoxHeight,
        hwnd, (HMENU)ID_PASSWORDTEXTBOX4, NULL, NULL);
    setPasswordFont(hPasswordBox4);
    centerPasswordText(hPasswordBox4, passwordBoxWidth, passwordBoxHeight);

    // Copy buttons
    hCopyButton1 = CreateWindowW(L"Button", L"Copy",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        710, passwordY1, copyButtonWidth, passwordBoxHeight,
        hwnd, (HMENU)ID_COPYBUTTON, NULL, NULL);
    setUiFont(hCopyButton1);

    hCopyButton2 = CreateWindowW(L"Button", L"Copy",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        710, passwordY2, copyButtonWidth, passwordBoxHeight,
        hwnd, (HMENU)ID_COPYBUTTON2, NULL, NULL);
    setUiFont(hCopyButton2);

    hCopyButton3 = CreateWindowW(L"Button", L"Copy",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        710, passwordY3, copyButtonWidth, passwordBoxHeight,
        hwnd, (HMENU)ID_COPYBUTTON3, NULL, NULL);
    setUiFont(hCopyButton3);

    hCopyButton4 = CreateWindowW(L"Button", L"Copy",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        710, passwordY4, copyButtonWidth, passwordBoxHeight,
        hwnd, (HMENU)ID_COPYBUTTON4, NULL, NULL);
    setUiFont(hCopyButton4);

    // Initialize common controls for up-down control
    INITCOMMONCONTROLSEX icex = { 0 };
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_UPDOWN_CLASS | ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    // Tooltips for copy buttons
    hToolTip = CreateWindowExW(
        WS_EX_TOPMOST,
        TOOLTIPS_CLASSW,
        NULL,
        WS_POPUP | TTS_ALWAYSTIP,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        hwnd,
        NULL,
        NULL,
        NULL
    );

    if (hToolTip != NULL) {
        addCopyTooltip(hwnd, hToolTip, GetDlgItem(hwnd, ID_COPYBUTTON));
        addCopyTooltip(hwnd, hToolTip, GetDlgItem(hwnd, ID_COPYBUTTON2));
        addCopyTooltip(hwnd, hToolTip, GetDlgItem(hwnd, ID_COPYBUTTON3));
        addCopyTooltip(hwnd, hToolTip, GetDlgItem(hwnd, ID_COPYBUTTON4));
    }

    // Status bar for password space
    hStatus = CreateWindowW(STATUSCLASSNAMEW, L"",
        WS_VISIBLE | WS_CHILD,
        0, 0, 0, 0,
        hwnd, (HMENU)ID_STATUSBAR, NULL, NULL);
    setUiFont(hStatus);
    SendMessageW(hStatus, WM_SIZE, 0, 0);

    // Up-down control for password length
    hUpDown = CreateWindowW(UPDOWN_CLASSW, NULL,
        WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT,
        0, 0, 0, 0,
        hwnd, (HMENU)ID_UPDOWN, NULL, NULL);

    // Numeric edit box for password length
    hEdit = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDITW, NULL,
        WS_CHILD | WS_VISIBLE | ES_RIGHT,
        30, 195, 50, 28,
        hwnd, (HMENU)ID_EDIT, NULL, NULL);
    setUiFont(hEdit);

    // Label for password length control
    hControl = CreateWindowW(L"static", L"Password Length",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        95, 195, 160, 30,
        hwnd, NULL, NULL, NULL);
    setUiFont(hControl);

    // Attach the up-down control to the edit box
    SendMessageW(hUpDown, UDM_SETBUDDY, (WPARAM)hEdit, 0);
    SendMessageW(hUpDown, UDM_SETRANGE, 0, MAKELPARAM(MAX_PASS_LENGTH, MIN_PASS_LENGTH));
    SendMessageW(hUpDown, UDM_SETPOS32, 0, passwordLength);

    // Up-down control for maximum symbols
    hMaxSymbolsUpDown = CreateWindowW(UPDOWN_CLASSW, NULL,
        WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT,
        0, 0, 0, 0,
        hwnd, (HMENU)ID_MAXSYMBOLSUPDOWN, NULL, NULL);

    // Numeric edit box for maximum symbols
    hMaxSymbolsEdit = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDITW, NULL,
        WS_CHILD | WS_VISIBLE | ES_RIGHT,
        30, 234, 50, 28,
        hwnd, (HMENU)ID_MAXSYMBOLSEDIT, NULL, NULL);
    setUiFont(hMaxSymbolsEdit);

    // Label for maximum symbols control
    hControl = CreateWindowW(L"static", L"Max Symbols",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        95, 234, 160, 30,
        hwnd, NULL, NULL, NULL);
    setUiFont(hControl);

    // Attach the up-down control to the edit box
    SendMessageW(hMaxSymbolsUpDown, UDM_SETBUDDY, (WPARAM)hMaxSymbolsEdit, 0);
    SendMessageW(hMaxSymbolsUpDown, UDM_SETRANGE, 0, MAKELPARAM(passwordLength, 0));
    SendMessageW(hMaxSymbolsUpDown, UDM_SETPOS32, 0, maxSymbols);
    EnableWindow(hMaxSymbolsEdit, incSymbols);
    EnableWindow(hMaxSymbolsUpDown, incSymbols);
}


// Create the application menu bar
void createMenuItems(HWND hwnd) {

    HMENU hMenubar;
    HMENU fileMenu;
    HMENU helpMenu;

    hMenubar = CreateMenu();
    fileMenu = CreateMenu();
    helpMenu = CreateMenu();

    // File menu
    AppendMenuW(fileMenu, MF_STRING, IDM_FILE_SAVE, L"&Save Settings");
    AppendMenuW(fileMenu, MF_STRING, IDM_FILE_QUIT, L"&Quit");

    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)fileMenu, L"&File");

    // Help menu
    AppendMenuW(helpMenu, MF_STRING, IDM_HELP_INFO, L"&Info");
    AppendMenuW(helpMenu, MF_STRING, IDM_HELP_ABOUT, L"&About");

    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)helpMenu, L"&Help");

    // Attach the menu bar to the window
    SetMenu(hwnd, hMenubar);
}
