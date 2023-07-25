#pragma once

extern LRESULT OldWindowProc;
extern LRESULT NewWindowProc;
extern HWND* MainWindow;

//#define pPlayerState			*(int*)0x1206058 // Need to find
#define	pCursorX				*(int*)0x8B97684 // ???
#define	pCursorY				*(int*)0x8B97688 // ???

BOOL KeyboardSetHook(BOOL bState);
BOOL MouseSetHook(BOOL bState);
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void WindowState();
void NotifyIcon(int bState);

extern HINSTANCE hDLLInst;