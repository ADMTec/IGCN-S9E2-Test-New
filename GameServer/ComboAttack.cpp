#include "stdafx.h"
#include "ComboAttack.h"
#include "user.h"
#include "MasterLevelSkillTreeSystem.h"
// GS-N 0.99.60T 0x004A42C0 : Completed
//	GS-N	1.00.18	JPN	0x004C2270	-	Completed

CComboAttack gComboAttack;

CComboAttack::CComboAttack() {
	return;
}
CComboAttack::~CComboAttack() {
	return;
}
int CComboAttack::GetSkillPos(int skillnum) {
	if (g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skillnum) == 326 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skillnum) == 327 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skillnum) == 328 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skillnum) == 329) return 0;
	else if (g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skillnum) == 330 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skillnum) == 332) return 1;
	else if (g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skillnum) == 333 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skillnum) == 331) return 1;
	else if (g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skillnum) == 336 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skillnum) == 339 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skillnum) == 342) return 1;
	else if (g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skillnum) == 337 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skillnum) == 340 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skillnum) == 343) return 1;
	else if (g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skillnum) == 344 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skillnum) == 346) return 1;

	switch (skillnum) {
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:			return 0;			break;
		case 41:
		case 42:
		case 43:
		case 232:			return 1;			break;
		default:			return -1;			break;
	}
}
BOOL CComboAttack::CheckCombo(int aIndex, int skillnum) {
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJ_USER) return FALSE;	
	if (lpObj->CharacterData->ComboSkillquestClear != false) {
		int sp = this->GetSkillPos(skillnum);

		if (sp == 0) {
			lpObj->CharacterData->comboSkill.ProgressIndex = 0;
			lpObj->CharacterData->comboSkill.dwTime = GetTickCount() + 3000;
			lpObj->CharacterData->comboSkill.Skill[0] = skillnum;
		} else if (sp == 1) {
			if (lpObj->CharacterData->comboSkill.Skill[0] == 0xFFFF) {
				lpObj->CharacterData->comboSkill.Init();
				return FALSE;
			}

			int Time = GetTickCount();

			if (lpObj->CharacterData->comboSkill.dwTime < GetTickCount()) {
				lpObj->CharacterData->comboSkill.Init();
				return FALSE;
			}

			if (lpObj->CharacterData->comboSkill.ProgressIndex == 0) {
				lpObj->CharacterData->comboSkill.ProgressIndex = 1;
				lpObj->CharacterData->comboSkill.dwTime = GetTickCount() + 3000;
				lpObj->CharacterData->comboSkill.Skill[1] = skillnum;
			} else if (lpObj->CharacterData->comboSkill.Skill[1] != skillnum) {
				lpObj->CharacterData->comboSkill.Init();

				if ((GetTickCount() - lpObj->CharacterData->comboSkill.dwTime) < 1000) return FALSE;
				
				return TRUE;
			} else lpObj->CharacterData->comboSkill.Init();
		} else {
			lpObj->CharacterData->comboSkill.ProgressIndex = -1;
			lpObj->CharacterData->comboSkill.dwTime = 0;
			lpObj->CharacterData->comboSkill.Skill[0] = -1;
		}
	}

	return FALSE;
}
			


