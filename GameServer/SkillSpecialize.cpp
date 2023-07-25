#include "stdafx.h"
#include "SkillSpecialize.h"
#include "configread.h"

CSkillSpecialize g_SkillSpec;

CSkillSpecialize::CSkillSpecialize(void) : m_Lua(true) {}
CSkillSpecialize::~CSkillSpecialize(void) {}
void CSkillSpecialize::LoadScript() {
	this->m_Lua.DoFile(g_ConfigRead.GetPath("\\Scripts\\Specialization\\SkillSpec.lua"));
}

int CSkillSpecialize::CalcStatBonus(OBJECTSTRUCT *lpObj, int iSkill) {
	if (!lpObj) return FALSE;
	if (lpObj->Type != OBJ_USER) return FALSE;

	int iSkillBonus = 0;

	this->m_Lua.Generic_Call("SkillSpec_GetBonusValue", "iiiiiii>i", (int)lpObj->Class, iSkill, int(lpObj->CharacterData->Strength + lpObj->AddStrength), int(lpObj->CharacterData->Dexterity + lpObj->AddDexterity), int(lpObj->CharacterData->Vitality + lpObj->AddVitality), int(lpObj->CharacterData->Energy + lpObj->AddEnergy), int(lpObj->Leadership + lpObj->AddLeadership), &iSkillBonus);
	return iSkillBonus;
}
