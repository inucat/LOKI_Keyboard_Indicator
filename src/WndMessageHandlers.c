#include <windows.h>

// void wmhGenericHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void wmhCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void wmhTrayIconClicked(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void wmhCtxMenuPressed(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void wmhKeyHooked(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// How are these two different from each other?
void wmhThemeChanged(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void wmhSettingsChanged(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void wmhQueryEndSession(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void wmhAppClosing(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void wmhDestroyWindow(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


void wmhCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void wmhTrayIconClicked(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void wmhCtxMenuPressed(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void wmhKeyHooked(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// How are these two different from each other?
void wmhThemeChanged(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void wmhSettingsChanged(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void wmhQueryEndSession(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void wmhAppClosing(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void
wmhDestroyWindow
(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    UnhookWindowsHookEx(hKeyHook);
    DestroyMenu(hMenu);
    for (int i=0; i < MAX_TRAYICONS; i++) {
        Shell_NotifyIcon(NIM_DELETE, &nid[i]);          // Remove Icons from the tray
    }
    PostQuitMessage(0);
}
