#include <windows.h>
#include <stdbool.h>
#include <wchar.h>
#include <commctrl.h>
#include <stdint.h>
#include <strsafe.h>
#include <time.h>
#include "main.h"
#include "cJSON/cJSON.h"

#pragma warning(disable : 4996)
#pragma comment(lib, "comctl32.lib")

#define ID_INCNUMBERS        1
#define ID_INCSYMBOLS        2
#define ID_INCLOWERCHARS     3
#define ID_INCUPPERCHARS     4
#define ID_AVOIDAMBCHARS     5
#define ID_GENERATEBUTTON    6
#define ID_COPYBUTTON        7
#define ID_PASSWORDTEXTBOX   8
#define ID_UPDOWN            9
#define ID_EDIT             10
#define ID_STATIC           11
#define IDM_FILE_SAVE       12
#define IDM_FILE_QUIT       13
#define IDM_HELP_ABOUT      14

#define MIN_PASS_LENGTH      8
#define MAX_PASS_LENGTH     32

#define DEFAULT_PASS_LENGTH 16

// Function Prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
wchar_t* generatePassword(void);
bool fileExists(const char* filename);
void centerWindow(HWND hwnd);
void createCheckBoxes(HWND hwnd);
void createMenuItems(HWND hwnd);
void createOtherControls(HWND hwnd);
int8_t loadSettings(void);
int8_t saveSettings(void);

// Global Variables
wchar_t* passWord;
wchar_t passwordArray[MAX_PASS_LENGTH] = { 0 };
wchar_t passText[MAX_PASS_LENGTH];
int8_t passwordLength;
int8_t passTextLength;
HWND hOut;
bool incNumbers;
bool incSymbols;
bool incLowerChars;
bool incUpperChars;
bool avoidAmbChars;
char* jsonSettingsFile = "settings.json";

// main()
int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR lpCmdLine,
	_In_ int nCmdShow
) {

	srand((unsigned int)time(NULL));
	loadSettings();
	MSG msg;
	WNDCLASSW wc = { 0 };
	wc.lpszClassName = L"Password Generator";
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDR_ICO_MAIN));
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpfnWndProc = WndProc;
	wc.hCursor = LoadCursor(0, IDC_ARROW);

	RegisterClassW(&wc);

	// WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME removes WS_THICKFRAME
	// from WS_OVERLAPPEDWINDOW, which disables windows resizing,
	// but you can still use the maximize window button.
	//
	// WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU disables window
	// resizing and disables the maximize window button.

	CreateWindowW(wc.lpszClassName, L"Password Generator",
		WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE,
		0, 0, 640, 250, 0, 0, hInstance, 0);

	// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmessage
	// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translatemessage
	// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dispatchmessage

	// Microsoft recommends handling this while loop slightly different, but
	// for this small password generator program, this should be fine.
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

// This is where all the UI handling is done.
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg,
	WPARAM wParam, LPARAM lParam) {

	LPNMUPDOWN lpnmud;
	INT code;
	HGLOBAL hGlobal;
	PSTR pGlobal;

	switch (msg) {

	// Things to do when the program is first started.
	// Primarily, build the UI. We also center the window.
	case WM_CREATE:
		createMenuItems(hwnd);
		createCheckBoxes(hwnd);
		createOtherControls(hwnd);
		centerWindow(hwnd);

		// Generate a password by default
		SetWindowTextW(hOut, L"");
		passWord = generatePassword();
		if (passWord != 0) {
			SetWindowTextW(hOut, passWord);
		}
		break;

	case WM_COMMAND:

		incNumbers = IsDlgButtonChecked(hwnd, ID_INCNUMBERS);
		incSymbols = IsDlgButtonChecked(hwnd, ID_INCSYMBOLS);
		incLowerChars = IsDlgButtonChecked(hwnd, ID_INCLOWERCHARS);
		incUpperChars = IsDlgButtonChecked(hwnd, ID_INCUPPERCHARS);
		avoidAmbChars = IsDlgButtonChecked(hwnd, ID_AVOIDAMBCHARS);

		switch (LOWORD(wParam)) {

		// Checkbox for Include Numbers
		case ID_INCNUMBERS:
			if (incNumbers) {
				CheckDlgButton(hwnd, ID_INCNUMBERS, BST_UNCHECKED);
			}
			else {
				CheckDlgButton(hwnd, ID_INCNUMBERS, BST_CHECKED);
			}
			break;

		// Checkbox for Include Symbols
		case ID_INCSYMBOLS:
			if (incSymbols) {
				CheckDlgButton(hwnd, ID_INCSYMBOLS, BST_UNCHECKED);
			}
			else {
				CheckDlgButton(hwnd, ID_INCSYMBOLS, BST_CHECKED);
			}
			break;

		// Checkbox for Include Lowercase Characters
		case ID_INCLOWERCHARS:
			if (incLowerChars) {
				CheckDlgButton(hwnd, ID_INCLOWERCHARS, BST_UNCHECKED);
			}
			else {
				CheckDlgButton(hwnd, ID_INCLOWERCHARS, BST_CHECKED);
			}
			break;

		// Checkbox for Include Uppercase Characters
		case ID_INCUPPERCHARS:
			if (incUpperChars) {
				CheckDlgButton(hwnd, ID_INCUPPERCHARS, BST_UNCHECKED);
			}
			else {
				CheckDlgButton(hwnd, ID_INCUPPERCHARS, BST_CHECKED);
			}
			break;

		// Checkbox for Avoid Ambiguous Characters
		case ID_AVOIDAMBCHARS:
			if (avoidAmbChars) {
				CheckDlgButton(hwnd, ID_AVOIDAMBCHARS, BST_UNCHECKED);
			}
			else {
				CheckDlgButton(hwnd, ID_AVOIDAMBCHARS, BST_CHECKED);
			}
			break;

		// Generate Button
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

		// Copy to Clipboard Button
		case ID_COPYBUTTON:
			passTextLength = GetWindowTextLength(hOut) + 1;
			GetWindowText(hOut, (LPWSTR)passText, passTextLength);
			hGlobal = GlobalAlloc(GHND | GMEM_SHARE, passTextLength * sizeof(wchar_t));
			if (hGlobal == NULL) {
				MessageBoxW(hwnd, L"GlobalAlloc() returned NULL", L"OK", MB_OK);
				return 1;
			}
			pGlobal = (char*)GlobalLock(hGlobal);
			if (pGlobal == NULL) {
				MessageBoxW(hwnd, L"GlobalLock() returned NULL", L"OK", MB_OK);
				return 1;
			}
			memcpy(pGlobal, passText, passTextLength * sizeof(wchar_t));

			GlobalUnlock(hGlobal);
			OpenClipboard(hwnd);
			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT, hGlobal);
			CloseClipboard();
			break;

		// About Menu Item
		case IDM_HELP_ABOUT:
			MessageBoxW(hwnd, L"Author: David Courtney\n\nBuild Date: 22-DEC-2023\n", L"About", MB_OK);
			break;

		// Save Settings Menu Item
		case IDM_FILE_SAVE:
			saveSettings();
			MessageBoxW(hwnd, L"Settings Saved", L"OK", MB_OK);
			break;

		// Quit Menu Item
		case IDM_FILE_QUIT:
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;

		}

		break;

	// Handle the up/down counter for the Password Length item
	case WM_NOTIFY:

		code = ((LPNMHDR)lParam)->code;

		//if (code == UDN_DELTAPOS) {
		//if (code == UDN_FIRST - 1) {
		//if (code == (0U - 721U) - 1) {
		if (code == 4294966575U - 1) {

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

	// User clicked X to shutdown the program
	case WM_DESTROY:

		PostQuitMessage(0);
		break;
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// This function simply centers the window when the program is started.
void centerWindow(HWND hwnd) {

	RECT rc = { 0 };

	GetWindowRect(hwnd, &rc);
	int32_t win_w = rc.right - rc.left;
	int32_t win_h = rc.bottom - rc.top;

	int32_t screen_w = GetSystemMetrics(SM_CXSCREEN);
	int32_t screen_h = GetSystemMetrics(SM_CYSCREEN);

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

void createCheckBoxes(HWND hwnd) {

	uint8_t checkBoxHeight = 30;
	uint8_t checkBoxWidth = 220;
	uint8_t checkBoxXpos = 20;
	uint8_t checkBoxYpos = 20;
	uint8_t checkBoxYdiff = 25;

	CreateWindowW(L"button", L"Include Numbers",
		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
		checkBoxXpos, checkBoxYpos, checkBoxWidth, checkBoxHeight, hwnd, (HMENU)ID_INCNUMBERS,
		NULL, NULL);

	if (incNumbers) {
		CheckDlgButton(hwnd, ID_INCNUMBERS, BST_CHECKED);
	}
	else {
		CheckDlgButton(hwnd, ID_INCNUMBERS, BST_UNCHECKED);
	}

	checkBoxYpos = checkBoxYpos + checkBoxYdiff;
	CreateWindowW(L"button", L"Include Symbols",
		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
		checkBoxXpos, checkBoxYpos, checkBoxWidth, checkBoxHeight, hwnd, (HMENU)ID_INCSYMBOLS,
		NULL, NULL);

	if (incSymbols) {
		CheckDlgButton(hwnd, ID_INCSYMBOLS, BST_CHECKED);
	}
	else {
		CheckDlgButton(hwnd, ID_INCSYMBOLS, BST_UNCHECKED);
	}

	checkBoxYpos = checkBoxYpos + checkBoxYdiff;
	CreateWindowW(L"button", L"Include Lowercase Characters",
		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
		checkBoxXpos, checkBoxYpos, checkBoxWidth, checkBoxHeight, hwnd, (HMENU)ID_INCLOWERCHARS,
		NULL, NULL);

	if (incLowerChars) {
		CheckDlgButton(hwnd, ID_INCLOWERCHARS, BST_CHECKED);
	}
	else {
		CheckDlgButton(hwnd, ID_INCLOWERCHARS, BST_UNCHECKED);
	}

	checkBoxYpos = checkBoxYpos + checkBoxYdiff;
	CreateWindowW(L"button", L"Include Uppercase Characters",
		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
		checkBoxXpos, checkBoxYpos, checkBoxWidth, checkBoxHeight, hwnd, (HMENU)ID_INCUPPERCHARS,
		NULL, NULL);

	if (incUpperChars) {
		CheckDlgButton(hwnd, ID_INCUPPERCHARS, BST_CHECKED);
	}
	else {
		CheckDlgButton(hwnd, ID_INCUPPERCHARS, BST_UNCHECKED);
	}

	checkBoxYpos = checkBoxYpos + checkBoxYdiff;
	CreateWindowW(L"button", L"Avoid Ambiguous Characters",
		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
		checkBoxXpos, checkBoxYpos, checkBoxWidth, checkBoxHeight, hwnd, (HMENU)ID_AVOIDAMBCHARS,
		NULL, NULL);

	if (avoidAmbChars) {
		CheckDlgButton(hwnd, ID_AVOIDAMBCHARS, BST_CHECKED);
	}
	else {
		CheckDlgButton(hwnd, ID_AVOIDAMBCHARS, BST_UNCHECKED);
	}

}

void createOtherControls(HWND hwnd) {

	HWND hUpDown, hEdit;
	UINT boxWidth = 330;

	CreateWindowW(L"Button", L"Generate",
		WS_VISIBLE | WS_CHILD | SS_CENTER,
		280, 20, boxWidth, 30, hwnd, (HMENU)ID_GENERATEBUTTON,
		NULL, NULL);

	hOut = CreateWindowW(L"Edit", L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER,
		280, 80, boxWidth, 20, hwnd, (HMENU)ID_PASSWORDTEXTBOX,
		NULL, NULL);

	CreateWindowW(L"Button", L"Copy to Clipboard",
		WS_VISIBLE | WS_CHILD | SS_CENTER,
		280, 130, boxWidth, 30, hwnd, (HMENU)ID_COPYBUTTON,
		NULL, NULL);

	INITCOMMONCONTROLSEX icex = { 0, 0 };

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_UPDOWN_CLASS;
	InitCommonControlsEx(&icex);

	hUpDown = CreateWindowW(UPDOWN_CLASSW, NULL, WS_CHILD | WS_VISIBLE
		| UDS_SETBUDDYINT | UDS_ALIGNRIGHT,
		0, 0, 0, 0, hwnd, (HMENU)ID_UPDOWN, NULL, NULL);

	hEdit = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDITW, NULL, WS_CHILD
		| WS_VISIBLE | ES_RIGHT, 20, 150, 45, 25, hwnd,
		(HMENU)ID_EDIT, NULL, NULL);

	CreateWindowW(L"static", L"Password Length",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		80, 150, 160, 30, hwnd, NULL,
		NULL, NULL);

	SendMessageW(hUpDown, UDM_SETBUDDY, (WPARAM)hEdit, 0);
	SendMessageW(hUpDown, UDM_SETRANGE, 0, MAKELPARAM(MAX_PASS_LENGTH, MIN_PASS_LENGTH));
	SendMessageW(hUpDown, UDM_SETPOS32, 0, passwordLength);
}

void createMenuItems(HWND hwnd) {

	HMENU hMenubar;
	HMENU fileMenu;
	HMENU helpMenu;

	hMenubar = CreateMenu();
	fileMenu = CreateMenu();
	helpMenu = CreateMenu();

	AppendMenuW(fileMenu, MF_STRING, IDM_FILE_SAVE, L"&Save Settings");
	AppendMenuW(fileMenu, MF_STRING, IDM_FILE_QUIT, L"&Quit");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)fileMenu, L"&File");
	AppendMenuW(helpMenu, MF_STRING, IDM_HELP_ABOUT, L"&About");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)helpMenu, L"&Help");

	SetMenu(hwnd, hMenubar);
}

// This generatePassword() function does the actual work of generating the password.
wchar_t* generatePassword(void) {

	uint8_t randomNumber = 0;
	uint8_t startingValue = 0;
	uint8_t characterRange = 94;
	bool hasNumbers = false;
	bool hasSymbols = false;
	bool hasLowerChars = false;
	bool hasUpperChars = false;
	bool validPassword = false;

	if (incNumbers || incSymbols || incLowerChars || incUpperChars) {
		startingValue = 33;
	}

	// If no checkboxes were checked, we have nothing to do.
	else {
		return 0;
	}

// Error checking for fringe cases. Only applicable for really short passwords.
#if MIN_PASS_LENGTH < 4
	if (passwordLength <= 3) {
		if (incNumbers && incSymbols && incLowerChars && incUpperChars && passwordLength <= 3) {
			return 0;
		}

		if (incNumbers && incSymbols && incLowerChars && passwordLength <= 2) {
			return 0;
		}

		if (incNumbers && incSymbols && incUpperChars && passwordLength <= 2) {
			return 0;
		}

		if (incNumbers && incLowerChars && incUpperChars && passwordLength <= 2) {
			return 0;
		}

		if (incSymbols && incLowerChars && incUpperChars && passwordLength <= 2) {
			return 0;
		}

		if (incNumbers && (incSymbols || incLowerChars || incUpperChars) && passwordLength <= 1) {
			return 0;
		}

		if (incSymbols && (incNumbers || incLowerChars || incUpperChars) && passwordLength <= 1) {
			return 0;
		}

		if (incLowerChars && (incNumbers || incSymbols || incUpperChars) && passwordLength <= 1) {
			return 0;
		}

		if (incUpperChars && (incNumbers || incSymbols || incLowerChars) && passwordLength <= 1) {
			return 0;
		}
	}
#endif

	// Zero out passwordArray before every generation.
	memset(passwordArray, 0, MAX_PASS_LENGTH * sizeof(wchar_t));

	do {
		// reset requirements before each attempt
		hasNumbers = false;
		hasSymbols = false;
		hasLowerChars = false;
		hasUpperChars = false;

		// Generate random password
		for (uint8_t count = 0; count < passwordLength; count++) {
			randomNumber = rand() % characterRange + startingValue;

			// This block checks to make sure the random number that was generated can be
			// used according to our conditions. If the condition is not met, we decrement
			// our count by 1 so we can generate a new number and try again.
			if (!incNumbers && isdigit(randomNumber)) {
				count--;
			}
			else if (!incSymbols && ispunct(randomNumber)) {
				count--;
			}
			else if (!incLowerChars && islower(randomNumber)) {
				count--;
			}
			else if (!incUpperChars && isupper(randomNumber)) {
				count--;
			}
			// avoid the number zero, the number one, the uppercase letter I, the uppercase letter O, the lower case letter i
			// the lowercase letter l, the lowercase letter o, and the pipe symbol |
			else if (avoidAmbChars && (randomNumber == 48 || randomNumber == 49 || randomNumber == 73 || randomNumber == 79 || randomNumber == 105 || randomNumber == 108 || randomNumber == 111 || randomNumber == 124)) {
				count--;
			}

			// The generated number meets our conditions, so now we'll process the random number.
			else {
				passwordArray[count] = randomNumber;

				if (incNumbers && isdigit(randomNumber)) {
					hasNumbers = true;
				}
				else if (incSymbols && ispunct(randomNumber)) {
					hasSymbols = true;
				}
				else if (incLowerChars && islower(randomNumber)) {
					hasLowerChars = true;
				}
				else if (incUpperChars && isupper(randomNumber)) {
					hasUpperChars = true;
				}
			}
		}

		// The password has been generated, but now we need
		// to test to see if password meets requirements.
		if (incNumbers && (!hasNumbers)) {
			validPassword = false;
		}
		else if (incSymbols && (!hasSymbols)) {
			validPassword = false;
		}
		else if (incLowerChars && (!hasLowerChars)) {
			validPassword = false;
		}
		else if (incUpperChars && (!hasUpperChars)) {
			validPassword = false;
		}
		else {
			validPassword = true;
		}

		// Keep generating passwords until the requirements are met
	} while (validPassword == false);

	return passwordArray;
}

// Simple file check function. Return false if the
// file does not exist, return true if it does.
bool fileExists(const char* fileName) {
	/* try to open file to read */
	FILE* filePointer;
	filePointer = fopen(fileName, "r");
	if (filePointer == NULL) {
		return false;
	}
	fclose(filePointer);
	return true;
}

// This function will save settings to a file.
int8_t saveSettings(void) {

	FILE* filePointer;

	// cJSON
	cJSON* json = cJSON_CreateObject();

	cJSON_AddBoolToObject(json, "incNumbers", incNumbers);
	cJSON_AddBoolToObject(json, "incSymbols", incSymbols);
	cJSON_AddBoolToObject(json, "incLowerChars", incLowerChars);
	cJSON_AddBoolToObject(json, "incUpperChars", incUpperChars);
	cJSON_AddBoolToObject(json, "avoidAmbChars", avoidAmbChars);
	cJSON_AddNumberToObject(json, "passwordLength", passwordLength);

	// convert the cJSON object to a JSON string 
	char* jsonStr = cJSON_Print(json);

	// write the JSON string to a file 
	filePointer = fopen(jsonSettingsFile, "w");
	if (filePointer == NULL) {
		printf("Error: Unable to open the file.\n");
		return 1;
	}
	fputs(jsonStr, filePointer);
	fclose(filePointer);

	return 0;
}

// This function will load settings from a file (if the file exists).
// If no settings file exists, this function defines defaults to use instead.
int8_t loadSettings(void) {

	FILE* filePointer;
	cJSON* json = cJSON_CreateObject();

	// Initialize values. If settings file exits, these values will be
	// overwritten with our saved values.
	incNumbers = true;
	incSymbols = true;
	incLowerChars = true;
	incUpperChars = true;
	avoidAmbChars = false;
	passwordLength = DEFAULT_PASS_LENGTH;

	if (fileExists(jsonSettingsFile)) {
		filePointer = fopen(jsonSettingsFile, "r");

		// read the file contents into a string 
		char buffer[1024];
		size_t len = fread(buffer, 1, sizeof(buffer), filePointer);
		fclose(filePointer);

		// parse the JSON data 
		cJSON* json = cJSON_Parse(buffer);
		if (json == NULL) {
			const char* error_ptr = cJSON_GetErrorPtr();
			if (error_ptr != NULL) {
				printf("Error: %s\n", error_ptr);
			}
			cJSON_Delete(json);
			return 1;
		}

		// access the JSON data
		cJSON* data;

		data = cJSON_GetObjectItemCaseSensitive(json, "incNumbers");
		if (cJSON_IsBool(data) && cJSON_IsFalse(data)) {
			incNumbers = false;
		}

		data = cJSON_GetObjectItemCaseSensitive(json, "incSymbols");
		if (cJSON_IsBool(data) && cJSON_IsFalse(data)) {
			incSymbols = false;
		}

		data = cJSON_GetObjectItemCaseSensitive(json, "incLowerChars");
		if (cJSON_IsBool(data) && cJSON_IsFalse(data)) {
			incLowerChars = false;
		}

		data = cJSON_GetObjectItemCaseSensitive(json, "incUpperChars");
		if (cJSON_IsBool(data) && cJSON_IsFalse(data)) {
			incUpperChars = false;
		}

		data = cJSON_GetObjectItemCaseSensitive(json, "avoidAmbChars");
		if (cJSON_IsBool(data) && cJSON_IsTrue(data)) {
			avoidAmbChars = true;
		}

		data = cJSON_GetObjectItemCaseSensitive(json, "passwordLength");
		if (cJSON_IsNumber && data->valueint != 0) {
			passwordLength = data->valueint;
			if (passwordLength < MIN_PASS_LENGTH || passwordLength > MAX_PASS_LENGTH) {
				passwordLength = DEFAULT_PASS_LENGTH;
			}
		}

		// delete the JSON object 
		cJSON_Delete(json);

	}

	return 0;
}
