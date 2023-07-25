// CrywolfUtil.cpp: implementation of the CCrywolfUtil class.
//	GS-N	1.00.18	JPN	0x0056F8E0	-	Completed
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CrywolfUtil.h"
#include "DSProtocol.h"
#include "MapServerManager.h"
#include "TNotice.h"
#include "classdef.h"
#include "TLog.h"
#include "Gamemain.h"
#include "MasterLevelSkillTreeSystem.h"
#include "configread.h"

CCrywolfUtil UTIL;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCrywolfUtil::CCrywolfUtil() { 
	return;
}

CCrywolfUtil::~CCrywolfUtil() {
	return;
}

void CCrywolfUtil::SendMapServerGroupMsg(LPSTR lpszMsg, ...) {
	if (!lpszMsg) return;
	char szBuffer[512] = "";
	va_list	pArguments;
	va_start(pArguments, lpszMsg);
	vsprintf(szBuffer, lpszMsg, pArguments);
	va_end(pArguments);
	GS_GDReqMapSvrMsgMultiCast(g_MapServerManager.GetMapSvrGroup(), szBuffer);
}

void CCrywolfUtil::SendAllUserAnyData(LPBYTE lpMsg, int iSize) {
	for (int i = g_ConfigRead.server.GetObjectStartUserIndex(); i < g_ConfigRead.server.GetObjectMax(); i++) {
		if (gObj[i].Connected == PLAYER_PLAYING) {
			if (gObj[i].Type == OBJ_USER) IOCP.DataSend(i, lpMsg, iSize);
		}
	}
}

void CCrywolfUtil::SendAllUserAnyMsg(int iType, LPSTR lpszMsg, ...) {
	if (!lpszMsg) return;
	char szBuffer[512] = "";
	va_list	pArguments;
	va_start(pArguments, lpszMsg);
	vsprintf(szBuffer, lpszMsg, pArguments);
	va_end(pArguments);
	PMSG_NOTICE pNotice;

	switch (iType) {
		case 1:
			TNotice::MakeNoticeMsg(&pNotice, 0, szBuffer);
			TNotice::SetNoticeProperty(&pNotice, 10, _ARGB(255, 255, 200, 80), 1, 0, 20);
			TNotice::SendNoticeToAllUser(&pNotice);
		break;

		case 2:
			TNotice::MakeNoticeMsg(&pNotice, 0, szBuffer);
			TNotice::SendNoticeToAllUser(&pNotice);
		break;
	}
}

void CCrywolfUtil::SendCrywolfUserAnyData(LPBYTE lpMsg, int iSize) {
	for (int i = g_ConfigRead.server.GetObjectStartUserIndex(); i < g_ConfigRead.server.GetObjectMax(); i++) {
		if (gObj[i].Connected == PLAYER_PLAYING) {
			if (gObj[i].Type == OBJ_USER) {
				if (gObj[i].MapNumber == MAP_INDEX_CRYWOLF_FIRSTZONE) IOCP.DataSend(i, lpMsg, iSize);
			}
		}
	}
}

void CCrywolfUtil::SendCrywolfUserAnyMsg(int iType, LPSTR lpszMsg, ...) {
	if (!lpszMsg) return;
	char szBuffer[512] = "";
	va_list	pArguments;
	va_start(pArguments, lpszMsg);
	vsprintf(szBuffer, lpszMsg, pArguments);
	va_end(pArguments);
	PMSG_NOTICE pNotice;

	switch (iType) {
		case 1:
			TNotice::MakeNoticeMsg(&pNotice, 0, szBuffer);
			TNotice::SetNoticeProperty(&pNotice, 10, _ARGB(255, 255, 200, 80), 1, 0, 20);
		break;

		case 2:
			TNotice::MakeNoticeMsg(&pNotice, 0, szBuffer);
		break;
	}

	for (int i = g_ConfigRead.server.GetObjectStartUserIndex(); i < g_ConfigRead.server.GetObjectMax(); i++) {
		if (gObj[i].Connected == PLAYER_PLAYING) {
			if (gObj[i].Type == OBJ_USER) {
				if (gObj[i].MapNumber == MAP_INDEX_CRYWOLF_FIRSTZONE) IOCP.DataSend(i, (LPBYTE)&pNotice, pNotice.h.size);
			}
		}
	}

}

#pragma warning (disable : 4101)
void CCrywolfUtil::SendCrywolfChattingMsg(int iObjIndex, LPSTR lpszMsg, ...) {
	return;
	LPOBJ lpObj;
	char szBuffer[512];
	va_list pArguments;
	char szChat[60];

	for (int i; i < MaxViewportMonster; i++) {
		if (lpObj->VpPlayer2[i].state != FALSE)	{
			int iTargetNumber = lpObj->VpPlayer2[i].number;
			if (ObjectMaxRange(iTargetNumber) != FALSE) GSProtocol.PROTOCOL_NPCChatSend(lpObj, szChat, iTargetNumber);
		}
	}
}
#pragma warning (default : 4101)

int CCrywolfUtil::CalcRewardEXP(int iUserIndex, UINT64& iAddExp) {
	if (ObjectMaxRange(iUserIndex) == FALSE) return FALSE;
	if (gObj[iUserIndex].Type != OBJ_USER) return FALSE;

	gObjSetExpPetItem(iUserIndex, iAddExp);

	if (g_MasterLevelSkillTreeSystem.ML_IsMasterCharacter(&gObj[iUserIndex])) gObj[iUserIndex].CharacterData->MasterExperience += iAddExp;
	else gObj[iUserIndex].CharacterData->Experience += iAddExp;

	CHARACTER_LevelUp(&gObj[iUserIndex], iAddExp, 0, "Crywolf");
	return iAddExp;
}