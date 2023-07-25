#pragma once
#define HOOK_SET_LEVEL_TEXT 0x009A90B1
#define JMPBACK_SET_LEVEL_TEXT 0x009A90CA
#define pInvokeUI ((DWORD(__stdcall*)(DWORD pFrame, char * method, char * format ...)) 0x0095A6EA)

void SetLevelResetUI();

extern int iResetVisibleUI;
extern int iGResetVisibleUI;
extern int iResetUI;
extern int iGResetUI;