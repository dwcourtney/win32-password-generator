#include <windows.h>
#include <commctrl.h>

#include "ui.h"
#include "main.h"
#include "state.h"

// Create the checkbox controls for password configuration
void createCheckBoxes(HWND hwnd) {

    int checkBoxHeight = 30;
    int checkBoxWidth = 220;
    int checkBoxXpos = 30;
    int checkBoxYpos = 25;
    int checkBoxYdiff = 25;

    // Settings group box
    CreateWindowW(L"button", L"Settings",
        WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
        10, 5, 250, 250,
        hwnd, NULL, NULL, NULL);

    // Include Numbers checkbox
    CreateWindowW(L"button", L"Include Numbers",
        WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
        checkBoxXpos, checkBoxYpos, checkBoxWidth, checkBoxHeight,
        hwnd, (HMENU)ID_INCNUMBERS, NULL, NULL);

    CheckDlgButton(hwnd, ID_INCNUMBERS, incNumbers ? BST_CHECKED : BST_UNCHECKED);

    // Include Symbols checkbox
    checkBoxYpos += checkBoxYdiff;
    CreateWindowW(L"button", L"Include Symbols",
        WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
        checkBoxXpos, checkBoxYpos, checkBoxWidth, checkBoxHeight,
        hwnd, (HMENU)ID_INCSYMBOLS, NULL, NULL);

    CheckDlgButton(hwnd, ID_INCSYMBOLS, incSymbols ? BST_CHECKED : BST_UNCHECKED);

    // Include Lowercase checkbox
    checkBoxYpos += checkBoxYdiff;
    CreateWindowW(L"button", L"Include Lowercase Characters",
        WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
        checkBoxXpos, checkBoxYpos, checkBoxWidth, checkBoxHeight,
        hwnd, (HMENU)ID_INCLOWERCHARS, NULL, NULL);

    CheckDlgButton(hwnd, ID_INCLOWERCHARS, incLowerChars ? BST_CHECKED : BST_UNCHECKED);

    // Include Uppercase checkbox
    checkBoxYpos += checkBoxYdiff;
    CreateWindowW(L"button", L"Include Uppercase Characters",
        WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
        checkBoxXpos, checkBoxYpos, checkBoxWidth, checkBoxHeight,
        hwnd, (HMENU)ID_INCUPPERCHARS, NULL, NULL);

    CheckDlgButton(hwnd, ID_INCUPPERCHARS, incUpperChars ? BST_CHECKED : BST_UNCHECKED);

    // Avoid ambiguous characters checkbox
    checkBoxYpos += checkBoxYdiff;
    CreateWindowW(L"button", L"Avoid Ambiguous Characters",
        WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
        checkBoxXpos, checkBoxYpos, checkBoxWidth, checkBoxHeight,
        hwnd, (HMENU)ID_AVOIDAMBCHARS, NULL, NULL);

    CheckDlgButton(hwnd, ID_AVOIDAMBCHARS, avoidAmbChars ? BST_CHECKED : BST_UNCHECKED);
}


// Create buttons, edit box, and numeric up-down control
void createOtherControls(HWND hwnd) {

    HWND hUpDown;
    HWND hEdit;
    HWND hMaxSymbolsUpDown;
    HWND hMaxSymbolsEdit;
    UINT boxWidth = 330;

    // Generate password button
    CreateWindowW(L"Button", L"Generate",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        280, 25, boxWidth, 30,
        hwnd, (HMENU)ID_GENERATEBUTTON, NULL, NULL);

    // Output textbox
    hOut = CreateWindowW(L"Edit", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER,
        280, 120, boxWidth, 20,
        hwnd, (HMENU)ID_PASSWORDTEXTBOX, NULL, NULL);

    // Copy button
    CreateWindowW(L"Button", L"Copy to Clipboard",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        280, 205, boxWidth, 30,
        hwnd, (HMENU)ID_COPYBUTTON, NULL, NULL);

    // Initialize common controls for up-down control
    INITCOMMONCONTROLSEX icex = { 0 };
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&icex);

    // Up-down control for password length
    hUpDown = CreateWindowW(UPDOWN_CLASSW, NULL,
        WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT,
        0, 0, 0, 0,
        hwnd, (HMENU)ID_UPDOWN, NULL, NULL);

    // Numeric edit box for password length
    hEdit = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDITW, NULL,
        WS_CHILD | WS_VISIBLE | ES_RIGHT,
        30, 170, 45, 25,
        hwnd, (HMENU)ID_EDIT, NULL, NULL);

    // Label for password length control
    CreateWindowW(L"static", L"Password Length",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        90, 170, 160, 30,
        hwnd, NULL, NULL, NULL);

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
        30, 205, 45, 25,
        hwnd, (HMENU)ID_MAXSYMBOLSEDIT, NULL, NULL);

    // Label for maximum symbols control
    CreateWindowW(L"static", L"Max Symbols",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        90, 205, 160, 30,
        hwnd, NULL, NULL, NULL);

    // Attach the up-down control to the edit box
    SendMessageW(hMaxSymbolsUpDown, UDM_SETBUDDY, (WPARAM)hMaxSymbolsEdit, 0);
    SendMessageW(hMaxSymbolsUpDown, UDM_SETRANGE, 0, MAKELPARAM(MAX_PASS_LENGTH, 0));
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
    AppendMenuW(helpMenu, MF_STRING, IDM_HELP_ABOUT, L"&About");

    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)helpMenu, L"&Help");

    // Attach the menu bar to the window
    SetMenu(hwnd, hMenubar);
}
