#include <stdio.h>
#include <windows.h>
#include "resource.h"
#include "SHA1.h"

#define WM_USER_TRAY (WM_USER + 1)

// Global variables
HINSTANCE g_hInstance;
HMENU g_hTrayMenu;
enum enumWindowState { Show, Hidden } g_enumWindowState;

// Command-line options
bool g_bHideImmediately = false;
bool g_bSecretMode = false;

// Set password dialog procedure
INT_PTR CALLBACK ProcDlgSetPassword(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_COMMAND)
    {
        if (wParam == IDOK)
        {
            char szPassword[1024];
            char szConfirm[1024];
            GetDlgItemText(hWnd, IDC_SET_PASSWORD, szPassword, 1024);
            GetDlgItemText(hWnd, IDC_CONFIRM, szConfirm, 1024);
            for (unsigned int i = 0; i < strlen(szPassword); i++)
            {
                szPassword[i] = toupper(szPassword[i]);
            }
            for (unsigned int i = 0; i < strlen(szConfirm); i++)
            {
                szConfirm[i] = toupper(szConfirm[i]);
            }

            if (strcmp(szPassword, szConfirm) != 0)
            {
                MessageBox(hWnd, "Passwords do not match!", "Error", MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            if (strlen(szPassword) == 0)
            {
                MessageBox(hWnd, "Password length is zero!", "Error", MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            // Open HKEY_CURRENT_USER\Software
            HKEY hSoftwareKey;
            if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software", 
                0, KEY_CREATE_SUB_KEY, &hSoftwareKey) != 0) // Key not exist
            {
                MessageBox(hWnd, "Cannot open HKEY_CURRENT_USER\\Software!", 
                    "Error", MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            // Create HKEY_CURRENT_USER\Software\ScreenLock
            HKEY hScreenLockKey;
            if (RegCreateKeyEx(hSoftwareKey, "ScreenLock", 
                0, NULL, 0, KEY_SET_VALUE, NULL, &hScreenLockKey, NULL) != 0)
            {
                MessageBox(hWnd, "Cannot create HKEY_CURRENT_USER\\Software\\ScreenLock!", 
                    "Error", MB_OK | MB_ICONWARNING);
                RegCloseKey(hSoftwareKey);
                return TRUE;
            }

            // Calculate SHA1
            SHA1Context stContext;
            uint8_t digest[SHA1HashSize];
            char digestHex[41];

            SHA1Reset(&stContext);
            SHA1Input(&stContext, (uint8_t *)szPassword, strlen(szPassword));
            SHA1Result(&stContext, digest);
            sprintf_s(digestHex, 41, 
                "%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X"
                "%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X", 
                digest[0], digest[1], digest[2], digest[3], digest[4],
                digest[5], digest[6], digest[7], digest[8], digest[9],
                digest[10], digest[11], digest[12], digest[13], digest[14],
                digest[15], digest[16], digest[17], digest[18], digest[19]);

            // Create HKEY_CURRENT_USER\Software\ScreenLock\Password
            if (RegSetValueEx(hScreenLockKey, "Password", 
                0, REG_SZ, (BYTE *)digestHex, 41) != 0)
            {
                MessageBox(hWnd, "Cannot create registry value!", 
                    "Error", MB_OK | MB_ICONWARNING);
                RegCloseKey(hScreenLockKey);
                RegCloseKey(hSoftwareKey);
                return TRUE;
            }

            MessageBox(hWnd, "Password created successfully", 
                "Screen Locker", MB_OK | MB_ICONINFORMATION);
            EndDialog(hWnd, 0);
            return TRUE;
        }
        else if (wParam == IDCANCEL)
        {
            EndDialog(hWnd, 0);
            return TRUE;
        }
    }
    return FALSE;
}

// Modify password dialog procedure
INT_PTR CALLBACK ProcDlgModifyPassword(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_COMMAND)
    {
        if (wParam == IDOK)
        {
            char szOldPassword[1024];
            char szNewPassword[1024];
            char szNewConfirm[1024];
            GetDlgItemText(hWnd, IDC_OLD_PASSWORD, szOldPassword, 1024);
            GetDlgItemText(hWnd, IDC_NEW_PASSWORD, szNewPassword, 1024);
            GetDlgItemText(hWnd, IDC_NEW_CONFIRM, szNewConfirm, 1024);
            for (unsigned int i = 0; i < strlen(szOldPassword); i++)
            {
                szOldPassword[i] = toupper(szOldPassword[i]);
            }
            for (unsigned int i = 0; i < strlen(szNewPassword); i++)
            {
                szNewPassword[i] = toupper(szNewPassword[i]);
            }
            for (unsigned int i = 0; i < strlen(szNewConfirm); i++)
            {
                szNewConfirm[i] = toupper(szNewConfirm[i]);
            }

            if (strcmp(szNewPassword, szNewConfirm) != 0)
            {
                MessageBox(hWnd, "New passwords do not match!", "Error", MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            if (strlen(szNewPassword) == 0)
            {
                MessageBox(hWnd, "Password length is zero!", "Error", MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            // Open HKEY_CURRENT_USER\Software
            HKEY hSoftwareKey;
            if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software", 
                0, KEY_CREATE_SUB_KEY, &hSoftwareKey) != 0) // Key not exist
            {
                MessageBox(hWnd, "Cannot open HKEY_CURRENT_USER\\Software!", 
                    "Error", MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            // Create HKEY_CURRENT_USER\Software\ScreenLock
            HKEY hScreenLockKey;
            if (RegCreateKeyEx(hSoftwareKey, "ScreenLock", 
                0, NULL, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &hScreenLockKey, NULL) != 0)
            {
                MessageBox(hWnd, "Cannot create HKEY_CURRENT_USER\\Software\\ScreenLock!", 
                    "Error", MB_OK | MB_ICONWARNING);
                RegCloseKey(hSoftwareKey);
                return TRUE;
            }

            // Calculate SHA1
            SHA1Context stContext;
            uint8_t digest[SHA1HashSize];
            char oldDigestHex[41];
            char newDigestHex[41];

            SHA1Reset(&stContext);
            SHA1Input(&stContext, (uint8_t *)szOldPassword, strlen(szOldPassword));
            SHA1Result(&stContext, digest);
            sprintf_s(oldDigestHex, 41, 
                "%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X"
                "%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X", 
                digest[0], digest[1], digest[2], digest[3], digest[4],
                digest[5], digest[6], digest[7], digest[8], digest[9],
                digest[10], digest[11], digest[12], digest[13], digest[14],
                digest[15], digest[16], digest[17], digest[18], digest[19]);

            SHA1Reset(&stContext);
            SHA1Input(&stContext, (uint8_t *)szNewPassword, strlen(szNewPassword));
            SHA1Result(&stContext, digest);
            sprintf_s(newDigestHex, 41, 
                "%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X"
                "%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X", 
                digest[0], digest[1], digest[2], digest[3], digest[4],
                digest[5], digest[6], digest[7], digest[8], digest[9],
                digest[10], digest[11], digest[12], digest[13], digest[14],
                digest[15], digest[16], digest[17], digest[18], digest[19]);

            // Read HKEY_CURRENT_USER\Software\ScreenLock\Password
            char oldHash[1024];
            DWORD len = 1024;
            if (RegGetValue(hScreenLockKey, 0, "Password", RRF_RT_REG_SZ, 0, oldHash, &len) != 0)
            {
                MessageBox(hWnd, "Cannot get registry value!", 
                    "Error", MB_OK | MB_ICONWARNING);
                RegCloseKey(hScreenLockKey);
                RegCloseKey(hSoftwareKey);
                return TRUE;
            }

            if (strcmp(oldHash, oldDigestHex) != 0)
            {
                MessageBox(hWnd, "Password is not correct!", 
                    "Error", MB_OK | MB_ICONWARNING);
                RegCloseKey(hScreenLockKey);
                RegCloseKey(hSoftwareKey);
                return TRUE;
            }

            if (RegSetValueEx(hScreenLockKey, "Password", 
                0, REG_SZ, (BYTE *)newDigestHex, 41) != 0)
            {
                MessageBox(hWnd, "Cannot set registry value!", 
                    "Error", MB_OK | MB_ICONWARNING);
                RegCloseKey(hScreenLockKey);
                RegCloseKey(hSoftwareKey);
                return TRUE;
            }

            MessageBox(hWnd, "Password updated successfully", 
                "Screen Locker", MB_OK | MB_ICONINFORMATION);
            EndDialog(hWnd, 0);
            return TRUE;
        }
        else if (wParam == IDCANCEL)
        {
            EndDialog(hWnd, 0);
        }
    }
    return FALSE;
}

// Message handlers
void OnInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    NOTIFYICONDATA nti; 

    // Load icon
    HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(ICO_MAIN));
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

    if (!g_bSecretMode)
    {
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
    }

    // Set window size
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 1920, 1280, 0);

    // Hide window when necessary
    if (g_bHideImmediately)
    {
        ShowCursor(FALSE);
        ShowWindow(hWnd, SW_SHOW);
        g_enumWindowState = Show;
    }
    else
    {
        g_enumWindowState = Hidden;
    }

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
            ShowCursor(FALSE);
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
    // Get password hash
    HKEY hSoftwareKey;
    char szPasswordHash[1024];
    DWORD dwLen = 1024;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software", 0, KEY_QUERY_VALUE, &hSoftwareKey) == 0)
    {
        if (RegGetValue(hSoftwareKey, "ScreenLock", "Password", 
            RRF_RT_REG_SZ, 0, szPasswordHash, &dwLen) != 0)
        {
            memset(szPasswordHash, 0, 1024);
        }
        RegCloseKey(hSoftwareKey);
    }

    // Update password
    static char buf[32];
    for (int i = 31; i > 0; i--)
    {
        buf[i] = buf[i - 1];
    }
    buf[0] = (char)wParam;

    // Compare with the SHA1 encrypted password stored in system registry
    if (strlen(szPasswordHash) != 0)
    {
        for (int len = 1; len <= 32; len++)
        {
            // Reverse input
            char password[32];
            for (int i = 0; i < len; i++)
            {
                password[i] = buf[len - i - 1];
            }

            // Calculate hash
            SHA1Context stContext;
            uint8_t digest[SHA1HashSize];
            char digestHex[41];

            SHA1Reset(&stContext);
            SHA1Input(&stContext, (uint8_t *)password, len);
            SHA1Result(&stContext, digest);
            sprintf_s(digestHex, 41, 
                "%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X"
                "%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X""%02X", 
                digest[0], digest[1], digest[2], digest[3], digest[4],
                digest[5], digest[6], digest[7], digest[8], digest[9],
                digest[10], digest[11], digest[12], digest[13], digest[14],
                digest[15], digest[16], digest[17], digest[18], digest[19]);

            // Password match
            if (strcmp(digestHex, szPasswordHash) == 0)
            {
                ShowCursor(TRUE);
                ShowWindow(hWnd, SW_HIDE);
                g_enumWindowState = Hidden;
                return;
            }
        }
    }

    // Backdoor match (replace the magic string as you wish)
#ifndef NO_BACKDOOR
    if (buf[13] == 'T' && buf[12] == 'H' && buf[11] == 'E' && 
        buf[10] == 'D' && buf[9]  == 'O' && buf[8]  == 'G' &&
        buf[7]  == 'L' && buf[6]  == 'I' && buf[5]  == 'S' &&
        buf[4]  == 'T' && buf[3]  == 'E' && buf[2]  == 'N' && buf[1] == 'E' && buf[0] == 'R')
    {
        ShowCursor(TRUE);
        ShowWindow(hWnd, SW_HIDE);
        g_enumWindowState = Hidden;
    }
#endif
}

void OnHotKey(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if (g_enumWindowState == Hidden)
    {
        ShowCursor(FALSE);
        ShowWindow(hWnd, SW_SHOW);
        g_enumWindowState = Show;
    }
}

void OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if (wParam == IDM_LOCK_NOW)
    {
        ShowCursor(FALSE);
        ShowWindow(hWnd, SW_SHOW);
        g_enumWindowState = Show;
    }
    else if (wParam == IDM_SET_PASSWORD)
    {
        // 1. If HKCU\Software\ScreenLock does not exist, display the set password window
        // 2. If HKCU\Software\ScreenLock\Password does not exist, display the set password window
        // 3. Otherwise, display the modify password window
        HKEY hScreenLockKey;
        if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\ScreenLock", 
            0, KEY_QUERY_VALUE, &hScreenLockKey) != 0) // Key not exist
        {
            RegCloseKey(hScreenLockKey);
            DialogBoxParam(g_hInstance, "DLG_SET_PASSWORD", hWnd, ProcDlgSetPassword, 0);
        }
        else if (RegGetValue(hScreenLockKey, 0, "Password", 
            RRF_RT_REG_SZ, 0, 0, 0) != 0) // Value not exist
        {
            RegCloseKey(hScreenLockKey);
            DialogBoxParam(g_hInstance, "DLG_SET_PASSWORD", hWnd, ProcDlgSetPassword, 0);
        }
        else
        {
            DialogBoxParam(g_hInstance, "DLG_MODIFY_PASSWORD", hWnd, ProcDlgModifyPassword, 0);
        }
        
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

#define PROCESS_MSG(MSG,HANDLER) if(uMsg == MSG) { HANDLER(hWnd, wParam, lParam); return TRUE; }

// Main dialog procedure
INT_PTR CALLBACK ProcDlgMain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PROCESS_MSG(WM_INITDIALOG, OnInitDialog) // Init
    PROCESS_MSG(WM_PAINT,      OnPaint)
    PROCESS_MSG(WM_TIMER,      OnTimer)
    PROCESS_MSG(WM_KEYDOWN,    OnKeyDown)
    PROCESS_MSG(WM_HOTKEY,     OnHotKey) // Ctrl + Alt + H (Lock screen)
    PROCESS_MSG(WM_COMMAND,    OnCommand)
    PROCESS_MSG(WM_USER_TRAY,  OnUserTray) // Tray icon messages

    return FALSE;
}

#undef PROCESS_MSG

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
