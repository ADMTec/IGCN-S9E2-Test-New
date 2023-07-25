#include "stdafx.h"
#include "DSProtocol.h"
#include "protocol.h"
#include "user.h"
#include "GameMain.h"
#include "MasterSkillSystem.h"
#include "TLog.h"
#include "ObjCalCharacter.h"
#include "winutil.h"
#include "VipSys.h"
#include "configread.h"

CMasterLevelSystem::CMasterLevelSystem() {
	this->gMasterExperience = NULL;
}

CMasterLevelSystem::~CMasterLevelSystem() {
	delete [] this->gMasterExperience;
}

bool CMasterLevelSystem::ML_LevelUp(LPOBJ lpObj, UINT64& addexp, int iMonsterType, const char * szEventType) {
	if (lpObj->Type != OBJ_USER) {
		g_Log.AddC(TColor::Red, "[ERROR] lpObj->Type != OBJ_USER (%s)(%d)", __FILE__, __LINE__);
		return false;
	}

	g_Log.Add("[%s] Master Experience : Map[%s]-(%d,%d) [%s][%s](%d)(%d) %I64d + %I64d MonsterClass : %d", szEventType, Lang.GetMap(0, lpObj->MapNumber), lpObj->X, lpObj->Y, lpObj->AccountID, lpObj->Name, lpObj->Level, lpObj->CharacterData->MasterLevel, lpObj->CharacterData->MasterExperience - addexp, addexp, iMonsterType);

	if (lpObj->CharacterData->ChangeUP != 2) {
		g_Log.AddC(TColor::Red, "[ERROR] lpObj->CharacterData->ChangeUp != 2 (%s)(%d)", __FILE__, __LINE__);
		return false;
	}

	if (lpObj->CharacterData->MasterLevel >= g_ConfigRead.data.common.MLUserMaxLevel) {
		lpObj->CharacterData->MasterExperience = this->gMasterExperience[lpObj->CharacterData->MasterLevel];
		GSProtocol.GCServerMsgStringSend(Lang.GetText(0, 45), lpObj->m_Index, 1);
		return false;
	}

	gObjSetExpPetItem(lpObj->m_Index, addexp);

	if (lpObj->CharacterData->MasterExperience < lpObj->CharacterData->MasterNextExp) return true;
	while (lpObj->CharacterData->MasterExperience >= lpObj->CharacterData->MasterNextExp) {
		if (lpObj->CharacterData->MasterLevel >= g_ConfigRead.data.common.MLUserMaxLevel) {
			lpObj->CharacterData->MasterExperience = this->gMasterExperience[lpObj->CharacterData->MasterLevel];
			GSProtocol.GCServerMsgStringSend(Lang.GetText(0, 45), lpObj->m_Index, 1);
			return false;
		}

		addexp = lpObj->CharacterData->MasterExperience - lpObj->CharacterData->MasterNextExp;
		lpObj->CharacterData->MasterLevel++;
		lpObj->CharacterData->MasterNextExp = this->ML_CalcNextMLExp(lpObj);

		if (g_ConfigRead.data.reset.iBlockMLPointAfterResets == -1 || lpObj->CharacterData->m_iResets < g_ConfigRead.data.reset.iBlockMLPointAfterResets) lpObj->CharacterData->MasterPoint += g_ConfigRead.data.common.MLPointPerLevel;
		
		gObjCalCharacter.CHARACTER_Calc(lpObj->m_Index);
		lpObj->MaxLife += DCInfo.DefClass[lpObj->Class].LevelLife;
		lpObj->MaxMana += DCInfo.DefClass[lpObj->Class].LevelMana;
		lpObj->Life = lpObj->MaxLife;
		lpObj->Mana = lpObj->MaxMana;
		lpObj->Life = lpObj->MaxLife + lpObj->AddLife;
		lpObj->Mana = lpObj->MaxMana + lpObj->AddMana;
		gObjCalCharacter.CHARACTER_CalcShieldPoint(lpObj);
		lpObj->iShield = lpObj->iMaxShield + lpObj->iAddShield;
		GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
		PLAYER_CalcStamina(lpObj->m_Index);
		gObjCalcMaxLifePower(lpObj->m_Index);
		GSProtocol.GCMasterLevelUpMsgSend(lpObj->m_Index);
		g_Log.Add("[Mastering System] MLevel Up (ML:%d) (Point:%d) (%s)(%s)", lpObj->CharacterData->MasterLevel, lpObj->CharacterData->MasterPoint, lpObj->AccountID, lpObj->Name);
	}

	return true;
}

UINT64 CMasterLevelSystem::ML_CalcNextMLExp(LPOBJ lpObj) {
	if(lpObj->Type != OBJ_USER)	return FALSE;
	return this->gMasterExperience[lpObj->CharacterData->MasterLevel+1];
}

bool CMasterLevelSystem::ML_IsMasterCharacter(LPOBJ lpObj) {
	if(lpObj->Type != OBJ_USER)	return false;
	if(lpObj->CharacterData->ChangeUP == 2 && lpObj->Level >= g_ConfigRead.data.common.UserMaxLevel)	return true;
	return false;
}

bool CMasterLevelSystem::ML_ReceiveExp(LPOBJ lpObj, LPOBJ lpTargetObj) {
	int iMonsterMinLevel = g_ConfigRead.data.common.MLMonsterMinLevel;

	if (lpObj->Type != OBJ_USER) return FALSE;
	if (lpObj->CharacterData->VipType != 0) iMonsterMinLevel = g_VipSystem.GetMLMonsterMinLevel(lpObj);
	if(this->ML_IsMasterCharacter(lpObj)) {
		if(lpTargetObj->Level < iMonsterMinLevel) return FALSE;
	}

	return TRUE;
}

void CMasterLevelSystem::ML_SetExpTable() {
	if (this->gMasterExperience != NULL) delete [] this->gMasterExperience;
	this->gMasterExperience = new UINT64[g_ConfigRead.data.common.MLUserMaxLevel+1];

	if (this->gMasterExperience == NULL) {
		g_Log.MsgBox("error - memory allocation failed");
		return;
	}

	this->gMasterExperience[0] = 0;
	MULua * TempLua = new MULua(false);
	TempLua->DoFile(g_ConfigRead.GetPath("\\Scripts\\Misc\\ExpCalc.lua"));
	double exp = 0.0;

	for (int n = 1; n <= g_ConfigRead.data.common.MLUserMaxLevel; n++) {
		TempLua->Generic_Call("SetExpTable_Master", "ii>d", n, g_ConfigRead.data.common.UserMaxLevel, &exp);
		this->gMasterExperience[n] = exp;
		g_Log.Add("[MASTER EXP] [MASTERLEVEL %d] [MLEXP %I64d]", n, this->gMasterExperience[n]);
	}

	delete TempLua;
	g_Log.Add("Master exp setting exp table is completed");
}

void CMasterLevelSystem::SendMLData(LPOBJ lpObj) {
	if (lpObj->Type != OBJ_USER) return;
	PMSG_MASTER_INFO_SEND pMsg;
	PHeadSubSetB((LPBYTE)&pMsg, 0xF3, 0x50, sizeof(pMsg));
	pMsg.MasterLevel = lpObj->CharacterData->MasterLevel;
	pMsg.MLExpHHH = SET_NUMBERH(SET_NUMBERHW(HIDWORD(lpObj->CharacterData->MasterExperience)));
	pMsg.MLExpHHL = SET_NUMBERL(SET_NUMBERHW(HIDWORD(lpObj->CharacterData->MasterExperience)));
	pMsg.MLExpHLH = SET_NUMBERH(SET_NUMBERLW(HIDWORD(lpObj->CharacterData->MasterExperience)));
	pMsg.MLExpHLL = SET_NUMBERL(SET_NUMBERLW(HIDWORD(lpObj->CharacterData->MasterExperience)));
	pMsg.MLExpLHH = SET_NUMBERH(SET_NUMBERHW(LODWORD(lpObj->CharacterData->MasterExperience)));
	pMsg.MLExpLHL = SET_NUMBERL(SET_NUMBERHW(LODWORD(lpObj->CharacterData->MasterExperience)));
	pMsg.MLExpLLH = SET_NUMBERH(SET_NUMBERLW(LODWORD(lpObj->CharacterData->MasterExperience)));
	pMsg.MLExpLLL = SET_NUMBERL(SET_NUMBERLW(LODWORD(lpObj->CharacterData->MasterExperience)));
	pMsg.MLNextExpHHH = SET_NUMBERH(SET_NUMBERHW(HIDWORD(lpObj->CharacterData->MasterNextExp)));
	pMsg.MLNextExpHHL = SET_NUMBERL(SET_NUMBERHW(HIDWORD(lpObj->CharacterData->MasterNextExp)));
	pMsg.MLNextExpHLH = SET_NUMBERH(SET_NUMBERLW(HIDWORD(lpObj->CharacterData->MasterNextExp)));
	pMsg.MLNextExpHLL = SET_NUMBERL(SET_NUMBERLW(HIDWORD(lpObj->CharacterData->MasterNextExp)));
	pMsg.MLNextExpLHH = SET_NUMBERH(SET_NUMBERHW(LODWORD(lpObj->CharacterData->MasterNextExp)));
	pMsg.MLNextExpLHL = SET_NUMBERL(SET_NUMBERHW(LODWORD(lpObj->CharacterData->MasterNextExp)));
	pMsg.MLNextExpLLH = SET_NUMBERH(SET_NUMBERLW(LODWORD(lpObj->CharacterData->MasterNextExp)));
	pMsg.MLNextExpLLL = SET_NUMBERL(SET_NUMBERLW(LODWORD(lpObj->CharacterData->MasterNextExp)));
	pMsg.MasterPoint = lpObj->CharacterData->MasterPoint;
	pMsg.MaxLife = lpObj->MaxLife+lpObj->AddLife;
	pMsg.MaxMana = lpObj->MaxMana+lpObj->AddMana;
	pMsg.MaxShield = lpObj->iMaxShield+lpObj->iAddShield;
	pMsg.MaxStamina = lpObj->MaxStamina+lpObj->AddStamina;
	IOCP.DataSend(lpObj->m_Index, (LPBYTE)&pMsg, pMsg.h.size);
}

void CMasterLevelSystem::InitData(LPOBJ lpObj) {
	if (lpObj->Type != OBJ_USER) return;
	lpObj->CharacterData->MasterLevel = 0;
	lpObj->CharacterData->MasterPoint = 0;
	lpObj->CharacterData->MasterExperience = 0;
	lpObj->CharacterData->MasterNextExp = this->ML_CalcNextMLExp(lpObj);
	g_Log.Add("[Mastering System] [%s][%s] Set First Data after Quest", lpObj->AccountID, lpObj->Name);
	this->SendMLData(lpObj);
}

int CMasterLevelSystem::GetDieDecExpRate(LPOBJ lpObj) {
	if (this->ML_IsMasterCharacter(lpObj) == FALSE) return -1;
	int DecRate = 0;
	if (lpObj->m_PK_Level <= 3) DecRate = 7;
	else if (lpObj->m_PK_Level == 4) DecRate = 20;
	else if(lpObj->m_PK_Level == 5)	DecRate = 30;
	else if(lpObj->m_PK_Level >= 6)	DecRate = 40;
	return DecRate;
}

int CMasterLevelSystem::GetDieDecMoneyRate(LPOBJ lpObj) {
	if(this->ML_IsMasterCharacter(lpObj) == FALSE)	return -1;
	return 4;
}