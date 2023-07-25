#include "stdafx.h"
#include "CharacterFrame.h"

char szFormat_1[4] = "%s";
char szFormat_2[4] = "%d";
char szFormat_3[12] = "%s %s %s";

char szSetLevel[16] = "SetLevelText";
char szSetMLevel[16] = "SetMLevelText";
char szSetReset[16] = "SetResetText";
char szSetGReset[16] = "SetGResetText";
char szSetResetVisible[20] = "SetResetVisible";
char szSetGResetVisible[20] = "SetGResetVisible";
int iResetVisibleUI = 0; //0:false 1:true
int iGResetVisibleUI = 0;

DWORD pCFrame;
int iLevel = 0;
int iMLevel = 0;
int iResetUI = 0;
int iGResetUI = 0;
char buffer[20];

int iInitOnce = 0;
char szInitTextExtended[20] = "InitTextExtended";


char szLabelMLevel[] = "MLV"; //Change text here, but don't make it too long, won't fit the box.
char szLabelReset[] = "Reset";
char szLabelGReset[] = "GReset";


void __declspec(naked) SetLevelResetUI()
{
    _asm 
    {
        mov eax, 0x0851ACC4;
        mov eax, [eax];
        movzx eax, word ptr[eax + 0x10C];
        mov iLevel, eax;
        mov eax, 0x08B97850
        movzx eax, word ptr[eax];
        mov iMLevel, eax;

        mov edx, 0x00969F14;
        call edx;
        mov ecx, eax;
        mov edx, 0x007F7818;
        call edx;
        mov pCFrame, eax;
    }
    if(pCFrame)
    {
        if (!iInitOnce) {
            iInitOnce = 1;
            pInvokeUI(pCFrame, szInitTextExtended, szFormat_3, szLabelMLevel, szLabelReset, szLabelGReset);
        }
        sprintf(buffer, "%d", iLevel);
        pInvokeUI(pCFrame, szSetLevel, szFormat_1, buffer);
        sprintf(buffer, "%d", iMLevel);
        pInvokeUI(pCFrame, szSetMLevel, szFormat_1, buffer);
        
        if (iResetVisibleUI) {
            sprintf(buffer, "%d", iResetUI);
            pInvokeUI(pCFrame, szSetReset, szFormat_1, buffer);
        }
        else {
            pInvokeUI(pCFrame, szSetResetVisible, szFormat_2, 0);
        }
        if (iGResetVisibleUI) {
            sprintf(buffer, "%d", iGResetUI);
            pInvokeUI(pCFrame, szSetGReset, szFormat_1, buffer);
        }
        else {
            pInvokeUI(pCFrame, szSetGResetVisible, szFormat_2, 0);
        }

    }
    _asm 
    {
        mov edx, JMPBACK_SET_LEVEL_TEXT;
        jmp edx;
    }
}