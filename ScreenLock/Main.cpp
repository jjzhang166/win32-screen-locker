#include <windows.h>
#include "resource.h"

#define WM_USER_TRAY (WM_USER + 1)

// Global variables
HINSTANCE g_hInstance;
HMENU g_hTrayMenu;
enum enumWindowState { Show, Hidden } g_enumWindowState;

// Command-line options
bool g_bHideImmediately = false;
bool g_bSecretMode = false;

// Message handlers
void OnInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    NOTIFYICONDATA nti; 

    // Load icon
    HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(ICO_MAIN));
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

    // Load tray icon menu
    g_hTrayMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDM_TRAY)); 
    g_hTrayMenu = GetSubMenu(g_hTrayMenu, 0);

    // Create tray icon
    nti.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(ICO_MAIN)); 
    nti.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE; 
    nti.hWnd = hWnd; 
    nti.uID = 0;
    nti.uCallbackMessage = WM_USER_TRAY; 
    strcpy_s(nti.szTip, sizeof(nti.szTip), "Screen Locker"); 

    Shell_NotifyIcon(NIM_ADD, &nti); 

    // Set window size
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 1920, 1280, 0);

    // Hide cursor
    ShowCursor(FALSE);

    // Hide window
    g_enumWindowState = Hidden;

    // Start timer
    SetTimer(hWnd, 1, 100, NULL);

    // Register hot key
    if (RegisterHotKey(hWnd, 1, MOD_CONTROL | MOD_ALT, 'H') == 0)
    {
        MessageBox(hWnd, 
            "Hot key Ctrl + Alt + H has been registered!", "Warning", MB_OK | MB_ICONWARNING);
    }
}

void OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT stPS;
    BeginPaint(hWnd, &stPS);

    // Fill black
    SelectObject(stPS.hdc, GetStockObject(BLACK_BRUSH));
    Rectangle(stPS.hdc, 0, 0, 1920, 1280);

    EndPaint(hWnd, &stPS);
}

void OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if (g_enumWindowState == Hidden)
    {
        static DWORD dwLastInput = 0;

        LASTINPUTINFO stInfo = { sizeof(stInfo) };
        GetLastInputInfo(&stInfo);

        // Update the time of last input event
        if (stInfo.dwTime != dwLastInput)
            dwLastInput = stInfo.dwTime;

        // Display window when it's idle for one minute
        DWORD dwTime = GetTickCount();
        if (dwTime - dwLastInput > 60 * 1000)
        {
            ShowWindow(hWnd, SW_SHOW);
            g_enumWindowState = Show;
        }
    }
    else
    {
        SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

void OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    static char buf[5];
    buf[4] = buf[3];
    buf[3] = buf[2];
    buf[2] = buf[1];
    buf[1] = buf[0];
    buf[0] = (char)wParam;

    if (buf[4] == 'P' && buf[3] == 'U' && buf[2] == 'P' && buf[1] == 'P' && buf[0] == 'Y')
    {
        ShowWindow(hWnd, SW_HIDE);
        g_enumWindowState = Hidden;
    }
}

void OnHotKey(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if (g_enumWindowState == Hidden)
    {
        ShowWindow(hWnd, SW_SHOW);
        g_enumWindowState = Show;
    }
}

void OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if (wParam == IDM_LOCK_NOW)
    {
        ShowWindow(hWnd, SW_SHOW);
        g_enumWindowState = Show;
    }
    else if (wParam == IDM_SET_PASSWORD)
    {

    }
    else if (wParam == IDM_EXIT)
    {
        // Delete Tray Icon
        NOTIFYICONDATA nti; 

        nti.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(ICO_MAIN)); 
        nti.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE; 
        nti.hWnd = hWnd; 
        nti.uID = 0;
        nti.uCallbackMessage = WM_USER_TRAY; 
        strcpy_s(nti.szTip, sizeof(nti.szTip), "Screen Locker"); 

        Shell_NotifyIcon(NIM_DELETE, &nti);

        // Exit
        PostQuitMessage(0);
    }
}

void OnUserTray(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if (lParam == WM_RBUTTONDOWN) // Show popup menu
    {
        POINT point;
        GetCursorPos(&point); 
        TrackPopupMenu(g_hTrayMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hWnd, NULL);
    }
}

// Dialog procedure
INT_PTR CALLBACK ProcDlgMain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    #define PROCESS_MSG(MSG,HANDLER) if(uMsg == MSG) { HANDLER(hWnd, wParam, lParam); return TRUE; }

    PROCESS_MSG(WM_INITDIALOG, OnInitDialog) // Init
    PROCESS_MSG(WM_PAINT,      OnPaint)
    PROCESS_MSG(WM_TIMER,      OnTimer)
    PROCESS_MSG(WM_KEYDOWN,    OnKeyDown)
    PROCESS_MSG(WM_HOTKEY,     OnHotKey) // Ctrl + Alt + H (Lock screen)
    PROCESS_MSG(WM_COMMAND,    OnCommand)
    PROCESS_MSG(WM_USER_TRAY,  OnUserTray) // Tray icon messages

    #undef PROCESS_MSG

    return FALSE;
}

// WinMain
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
    MSG stMsg;
    g_hInstance = hInstance;

    // Usage
    const char szUsage[] = 
        "ScreenLock.exe [-h] [-i] [-s]\n"
        "\n"
        "-h:\tPrint this help.\n"
        "-i:\tLock screen immediately.\n"
        "-s:\tSecret mode (do not display tray icon).";

    // Parse command-line parameters
    if (strcmp(lpCmdLine, "-h") == 0)
    {
        MessageBox(NULL, szUsage, "Usage", MB_OK | MB_ICONINFORMATION);
        return 0;
    }
    else if (strcmp(lpCmdLine, "-i") == 0)
    {
        g_bHideImmediately = true;
    }
    else if (strcmp(lpCmdLine, "-s") == 0)
    {
        g_bSecretMode = true;
    }
    else if (strcmp(lpCmdLine, "-i -s") == 0 ||
             strcmp(lpCmdLine, "-s -i") == 0)
    {
        g_bHideImmediately = true;
        g_bSecretMode = true;
    }
    else if (strcmp(lpCmdLine, "") == 0)
    {
        // Nothing to do here
    }
    else
    {
        MessageBox(NULL, szUsage, "Usage", MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    // Display the window
    CreateDialogParam(g_hInstance, TEXT("DLG_MAIN"), NULL, ProcDlgMain, 0);

    // Message Loop
    while (GetMessage(&stMsg, NULL, 0, 0) != 0)
    {
        TranslateMessage(&stMsg);
        DispatchMessage(&stMsg);
    }

    // Exit
    return 0;
}
