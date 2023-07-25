#include "stdafx.h"
#include "ObjCalCharacter.h"
#include "Gamemain.h"
#include "DarkSpirit.h"
#include "SetItemOption.h"
#include "TLog.h"
#include "ItemSocketOptionSystem.h"
#include "BuffEffect.h"
#include "BuffEffectSlot.h"
#include "PentagramSystem.h"
#include "configread.h"
#include "MasterLevelSkillTreeSystem.h"
#include "MuunSystem.h"
#include "ItemOptionTypeMng.h"
#include "StatSpecialize.h"

CObjCalCharacter gObjCalCharacter;

CObjCalCharacter::CObjCalCharacter() : m_Lua(true) {}
CObjCalCharacter::~CObjCalCharacter() {}
void CObjCalCharacter::Init() {
	this->m_Lua.DoFile(g_ConfigRead.GetPath("\\Scripts\\Character\\CalcCharacter.lua"));
}
void CObjCalCharacter::CHARACTER_Calc(int aIndex) {
	if (gObj[aIndex].Type != OBJ_USER) return;

	LPOBJ lpObj = &gObj[aIndex];
	float addLifePercent = 0;
	float addManaPercent = 0;
	float addStamPercent = 0;
	int Strength = 0;
	int Dexterity = 0;
	int Vitality = 0;
	int Energy = 0;
	int Leadership = 0;
	CItem* Right = &lpObj->pInventory[0];
	CItem* Left  = &lpObj->pInventory[1];
	CItem* Helmet = &lpObj->pInventory[2];
	CItem* Armor = &lpObj->pInventory[3];
	CItem* Pants = &lpObj->pInventory[4];
	CItem* Gloves = &lpObj->pInventory[5];
	CItem* Boots = &lpObj->pInventory[6];
	CItem* Amulet = &lpObj->pInventory[9];
	CItem* Helper = &lpObj->pInventory[8];
	CItem* Wing = &lpObj->pInventory[7];
	CItem* RightRing = &lpObj->pInventory[10];
	CItem* LeftRing = &lpObj->pInventory[11];
	CItem* Pentagram = &lpObj->pInventory[236];

	lpObj->HaveWeaponInHand = true;

	if (Right->IsItem() == FALSE && Left->IsItem() == FALSE) lpObj->HaveWeaponInHand = false;
	else if (Left->IsItem() == FALSE && Right->m_Type == ITEMGET(4,15)) lpObj->HaveWeaponInHand = false;
	else if (Right->IsItem() == FALSE) {
		int iType = Left->m_Type / MAX_SUBTYPE_ITEMS;

		if (Left->m_Type == ITEMGET(4,7)) lpObj->HaveWeaponInHand = false;
		else if (iType == 6) lpObj->HaveWeaponInHand = false;
	}

	lpObj->AddLife = 0;
	lpObj->AddMana = 0;
	lpObj->AddStamina = 0;
	lpObj->iAddShield = 0;
	lpObj->MonsterDieGetMoney = 0;
	lpObj->MonsterDieGetLife = 0;
	lpObj->MonsterDieGetMana = 0;
	lpObj->LifeRegeneration = 0;
	lpObj->EXC_ReflectDamageTaken = 0;
	lpObj->EXC_ReduceDamageTaken = 0;
	lpObj->SkillLongSpearChange = false;

	int iItemIndex;
	BOOL bIsChangeItem;	// lc34

	for (iItemIndex = 0; iItemIndex < MAX_PLAYER_EQUIPMENT; iItemIndex++) {
		if (lpObj->pInventory[iItemIndex].IsItem() != FALSE) lpObj->pInventory[iItemIndex].m_IsValidItem = true;
	}

	if (lpObj->pInventory[236].IsItem() != FALSE) lpObj->pInventory[236].m_IsValidItem = true;
	do {
		lpObj->CharacterData->ANCIENT_MaxPhysicalDamage = 0;
		lpObj->CharacterData->ANCIENT_MinPhysicalDamage = 0;
		lpObj->CharacterData->ANCIENT_PhysicalDamage = 0;
		lpObj->CharacterData->ANCIENT_StaminaRegeneration = 0;
		lpObj->CharacterData->ANCIENT_CriticalStrikeChance = 0;
		lpObj->CharacterData->ANCIENT_CriticalDamage = 0;
		lpObj->CharacterData->ANCIENT_ExcellentStrikeChance = 0;
		lpObj->CharacterData->ANCIENT_ExcellentDamage = 0;
		lpObj->CharacterData->ANCIENT_SkillDamage = 0;
		lpObj->AddStrength = 0;
		lpObj->AddDexterity = 0;
		lpObj->AddVitality = 0;
		lpObj->AddEnergy = 0;
		lpObj->AddLeadership = 0;

		for (int x = 0; x <= 6; x++) lpObj->CharacterData->ANCIENT_AttributeDamage[x] = 0;

		lpObj->CharacterData->ANCIENT_StunDuration = 0;
		lpObj->CharacterData->ANCIENT_Defense = 0;
		lpObj->CharacterData->ANCIENT_SpellDamage = 0;
		lpObj->CharacterData->ANCIENT_BurnDamage = 0;
		lpObj->CharacterData->ANCIENT_ElementalAilmentsDuration = 0;
		lpObj->CharacterData->ANCIENT_FatalStrikeChance = 0;
		lpObj->CharacterData->ANCIENT_LethalStrikeChance = 0;
		lpObj->CharacterData->ANCIENT_TwoHandedWeaponDamage = 0;
		lpObj->CharacterData->ANCIENT_HitRating = 0;
		lpObj->CharacterData->ANCIENT_BurnChance = 0;
		lpObj->CharacterData->ANCIENT_DefenseWithShields = 0;
		lpObj->CharacterData->ANCIENT_DualWieldDamage = 0;
		lpObj->CharacterData->ANCIENT_ItemDropRate = 0;
		lpObj->CharacterData->IsFullSetItem = false;
		lpObj->CharacterData->m_WingExcOption.Clear();
		memset(lpObj->C_ResistanceBonus, 0, sizeof(lpObj->C_ResistanceBonus));
		memset(&lpObj->CharacterData->m_PentagramOptions, 0, sizeof(lpObj->CharacterData->m_PentagramOptions));
		bIsChangeItem = 0;
		g_StatSpec.ClearUserOption(lpObj);
		g_BuffEffect.SetPrevEffect(lpObj);
		this->CHARACTER_CalcSetItemStats(lpObj);
		this->CalcSetItemOption(lpObj);
		g_MasterLevelSkillTreeSystem.SetItemMLPassiveSkill(lpObj, Right->GetDetailItemType());
		g_MasterLevelSkillTreeSystem.SetItemMLPassiveSkill(lpObj, Left->GetDetailItemType());
		g_MasterLevelSkillTreeSystem.SetWingMLPassiveSkill(lpObj, Wing->m_Type);
		g_MasterLevelSkillTreeSystem.SetPetItemMLPassiveSkill(lpObj, Helper->m_Type);

		if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_TOUGHNESS)) {
			int iOption;

			BUFF_GetBuffEffectValue(lpObj, BUFFTYPE_TOUGHNESS, &iOption, 0);
			lpObj->AddVitality += iOption;
		}

		if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_IMPROVED_TOUGHNESS)) {
			int iOption;

			BUFF_GetBuffEffectValue(lpObj, BUFFTYPE_IMPROVED_TOUGHNESS, &iOption, 0);
			lpObj->AddVitality += iOption;
		}

		if (lpObj->Class == CLASS_RAGEFIGHTER && lpObj->CharacterData->ISBOT == false) {
			if (lpObj->CharacterData->Mastery.ML_RF_Intelligence > 0.0) lpObj->AddEnergy += lpObj->CharacterData->Mastery.ML_RF_Intelligence;
			if (lpObj->CharacterData->Mastery.ML_RF_Resilience > 0.0) lpObj->AddVitality += lpObj->CharacterData->Mastery.ML_RF_Resilience;
			if (lpObj->CharacterData->Mastery.ML_RF_Quickness > 0.0) {
				EnterCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
				lpObj->CharacterData->AgilityCheckDelay = GetTickCount();
				lpObj->AddDexterity += lpObj->CharacterData->Mastery.ML_RF_Quickness;
				LeaveCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
			}

			if (lpObj->CharacterData->Mastery.ML_RF_Brawn > 0.0) lpObj->AddStrength += lpObj->CharacterData->Mastery.ML_RF_Brawn;
		} else {
			if (lpObj->CharacterData->Mastery.ML_GENERAL_Intelligence > 0.0) lpObj->AddEnergy += lpObj->CharacterData->Mastery.ML_GENERAL_Intelligence;
			if (lpObj->CharacterData->Mastery.ML_GENERAL_Resilience > 0.0) lpObj->AddVitality += lpObj->CharacterData->Mastery.ML_GENERAL_Resilience;
			if (lpObj->CharacterData->Mastery.ML_GENERAL_Quickness > 0.0) {
				EnterCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
				lpObj->CharacterData->AgilityCheckDelay = GetTickCount();
				lpObj->AddDexterity += lpObj->CharacterData->Mastery.ML_GENERAL_Quickness;
				LeaveCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
			}

			if (lpObj->CharacterData->Mastery.ML_GENERAL_Brawn > 0.0) lpObj->AddStrength += lpObj->CharacterData->Mastery.ML_GENERAL_Brawn;
		}

		if (lpObj->CharacterData->Mastery.ML_DL_Leadership > 0.0) lpObj->AddLeadership += lpObj->CharacterData->Mastery.ML_DL_Leadership;
		if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_BLESS) == TRUE) {
			int iOption;

			BUFF_GetBuffEffectValue(lpObj, BUFFTYPE_BLESS, &iOption, 0);
			EnterCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
			lpObj->CharacterData->AgilityCheckDelay = GetTickCount();
			lpObj->AddVitality += iOption;
			lpObj->AddStrength += iOption;
			lpObj->AddDexterity += iOption;
			lpObj->AddEnergy += iOption;

			if (lpObj->Class == CLASS_DARKLORD) lpObj->AddLeadership += iOption;

			LeaveCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
		}


		for (iItemIndex = 0; iItemIndex < MAX_PLAYER_EQUIPMENT; iItemIndex++) {
			if (lpObj->pInventory[iItemIndex].IsItem() != FALSE && lpObj->pInventory[iItemIndex].m_IsValidItem != false) {
				if (this->ValidItem(lpObj, &lpObj->pInventory[iItemIndex], iItemIndex) != FALSE) lpObj->pInventory[iItemIndex].m_IsValidItem = true;
				else {
					lpObj->pInventory[iItemIndex].m_IsValidItem  = false;
					g_BuffEffect.ClearPrevEffect(lpObj);
					bIsChangeItem = TRUE;
				}
			}
		}
	} while (bIsChangeItem != FALSE);

	if (lpObj->pInventory[236].IsItem() == TRUE) {
		if (this->ValidItem(lpObj, &lpObj->pInventory[236], 236)) {
			lpObj->pInventory[236].m_IsValidItem = true;
			g_PentagramSystem.CalcPentagramItem(aIndex, &lpObj->pInventory[236]);
		} else lpObj->pInventory[236].m_IsValidItem = false;
	} else g_PentagramSystem.ClearPentagramItem(aIndex);
	
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 317) == 20 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 595) == 20) lpObj->AddEnergy += (lpObj->CharacterData->Energy + lpObj->AddEnergy) * 0.05;			// Intelligence
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 318) == 20 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 596) == 20) lpObj->AddVitality += (lpObj->CharacterData->Vitality + lpObj->AddVitality) * 0.05;	// Resillience
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 319) == 20 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 597) == 20) {																						// Quickness
		EnterCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
		lpObj->CharacterData->AgilityCheckDelay = GetTickCount();
		lpObj->AddDexterity += (lpObj->CharacterData->Dexterity + lpObj->AddDexterity) * 0.05;
		LeaveCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
	}

	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 320) == 20 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 598) == 20) lpObj->AddStrength += (lpObj->CharacterData->Strength + lpObj->AddStrength) * 0.05;	// Brawn
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 506) == 20) lpObj->AddLeadership += (lpObj->Leadership + lpObj->AddLeadership) * 0.05;																				// Leadership

	Strength = lpObj->CharacterData->Strength + lpObj->AddStrength;
	Dexterity = lpObj->CharacterData->Dexterity + lpObj->AddDexterity;
	Vitality = lpObj->CharacterData->Vitality + lpObj->AddVitality;
	Energy = lpObj->CharacterData->Energy + lpObj->AddEnergy;
	Leadership = lpObj->Leadership + lpObj->AddLeadership;

	if (lpObj->Class == CLASS_ELF) {
		if ((Right->m_Type >= ITEMGET(4,8) && Right->m_Type < ITEMGET(4,15)) ||
			 (Left->m_Type >= ITEMGET(4,0) && Left->m_Type < ITEMGET(4,7)) ||
			  Right->m_Type == ITEMGET(4,16) || Left->m_Type == ITEMGET(4,20) ||
			  Left->m_Type == ITEMGET(4,21) || Left->m_Type == ITEMGET(4,22) ||
			  Left->m_Type == ITEMGET(4,23) || Left->m_Type == ITEMGET(4,24) ||
			  Left->m_Type == ITEMGET(4,25) || Right->m_Type == ITEMGET(4,26) ||
			  Left->m_Type == ITEMGET(4,27) || Right->m_Type == ITEMGET(4,18) ||
			  Right->m_Type == ITEMGET(4,19) || Left->m_Type == ITEMGET(4,17)) {
			if ((Right->IsItem() != FALSE && Right->m_IsValidItem == false) || (Left->IsItem() != FALSE && Left->m_IsValidItem == false)) this->m_Lua.Generic_Call("ElfWithoutBowDamageCalc", "iiii>iiii", Strength, Dexterity, Vitality, Energy, &lpObj->m_AttackDamageMinLeft, &lpObj->m_AttackDamageMinRight, &lpObj->m_AttackDamageMaxLeft, &lpObj->m_AttackDamageMaxRight);
			else this->m_Lua.Generic_Call("ElfWithBowDamageCalc", "iiii>iiii", Strength, Dexterity, Vitality, Energy, &lpObj->m_AttackDamageMinLeft, &lpObj->m_AttackDamageMinRight, &lpObj->m_AttackDamageMaxLeft, &lpObj->m_AttackDamageMaxRight);
		} else this->m_Lua.Generic_Call("ElfWithoutBowDamageCalc", "iiii>iiii", Strength, Dexterity, Vitality, Energy, &lpObj->m_AttackDamageMinLeft, &lpObj->m_AttackDamageMinRight, &lpObj->m_AttackDamageMaxLeft, &lpObj->m_AttackDamageMaxRight);
	} else if (lpObj->Class == CLASS_KNIGHT) this->m_Lua.Generic_Call("KnightDamageCalc", "iiii>iiii", Strength, Dexterity, Vitality, Energy, &lpObj->m_AttackDamageMinLeft, &lpObj->m_AttackDamageMinRight, &lpObj->m_AttackDamageMaxLeft, &lpObj->m_AttackDamageMaxRight);
	else if (lpObj->Class == CLASS_MAGICGLADIATOR) this->m_Lua.Generic_Call("GladiatorDamageCalc", "iiii>iiii", Strength, Dexterity, Vitality, Energy, &lpObj->m_AttackDamageMinLeft, &lpObj->m_AttackDamageMinRight, &lpObj->m_AttackDamageMaxLeft, &lpObj->m_AttackDamageMaxRight);
	else if (lpObj->Class == CLASS_DARKLORD) this->m_Lua.Generic_Call("LordDamageCalc", "iiiii>iiii", Strength, Dexterity, Vitality, Energy, Leadership, &lpObj->m_AttackDamageMinLeft, &lpObj->m_AttackDamageMinRight, &lpObj->m_AttackDamageMaxLeft, &lpObj->m_AttackDamageMaxRight);
	else if (lpObj->Class == CLASS_SUMMONER) this->m_Lua.Generic_Call("SummonerDamageCalc", "iiii>iiii", Strength, Dexterity, Vitality, Energy, &lpObj->m_AttackDamageMinLeft, &lpObj->m_AttackDamageMinRight, &lpObj->m_AttackDamageMaxLeft, &lpObj->m_AttackDamageMaxRight);
	else if (lpObj->Class == CLASS_RAGEFIGHTER) this->m_Lua.Generic_Call("RageFighterDamageCalc", "iiii>iiii", Strength, Dexterity, Vitality, Energy, &lpObj->m_AttackDamageMinLeft, &lpObj->m_AttackDamageMinRight, &lpObj->m_AttackDamageMaxLeft, &lpObj->m_AttackDamageMaxRight);
	else if (lpObj->Class == CLASS_WIZARD) this->m_Lua.Generic_Call("WizardDamageCalc", "iiii>iiii", Strength, Dexterity, Vitality, Energy, &lpObj->m_AttackDamageMinLeft, &lpObj->m_AttackDamageMinRight, &lpObj->m_AttackDamageMaxLeft, &lpObj->m_AttackDamageMaxRight);
	else if (lpObj->Class == CLASS_GROWLANCER) this->m_Lua.Generic_Call("GrowLancerDamageCalc", "iiii>iiii", Strength, Dexterity, Vitality, Energy, &lpObj->m_AttackDamageMinLeft, &lpObj->m_AttackDamageMinRight, &lpObj->m_AttackDamageMaxLeft, &lpObj->m_AttackDamageMaxRight);

	g_StatSpec.CalcStatOption(lpObj, STAT_OPTION_INC_ATTACK_POWER);

	if (lpObj->Class == CLASS_ELF) {
		lpObj->m_AttackDamageMinRight += lpObj->CharacterData->Mastery.ML_ELF_Stability + lpObj->CharacterData->Mastery.ML_ELF_WeaponMastery;
		lpObj->m_AttackDamageMaxRight += lpObj->CharacterData->Mastery.ML_ELF_Recklessness + lpObj->CharacterData->Mastery.ML_ELF_WeaponMastery;
		lpObj->m_AttackDamageMinLeft += lpObj->CharacterData->Mastery.ML_ELF_Stability + lpObj->CharacterData->Mastery.ML_ELF_WeaponMastery;
		lpObj->m_AttackDamageMaxLeft += lpObj->CharacterData->Mastery.ML_ELF_Recklessness + lpObj->CharacterData->Mastery.ML_ELF_WeaponMastery;
	} else if (lpObj->Class == CLASS_RAGEFIGHTER) {
		lpObj->m_AttackDamageMinRight += lpObj->CharacterData->Mastery.ML_RF_Stability + lpObj->CharacterData->Mastery.ML_GENERAL_WeaponMastery;
		lpObj->m_AttackDamageMaxRight += lpObj->CharacterData->Mastery.ML_RF_Recklessness + lpObj->CharacterData->Mastery.ML_GENERAL_WeaponMastery;
		lpObj->m_AttackDamageMinLeft += lpObj->CharacterData->Mastery.ML_RF_Stability + lpObj->CharacterData->Mastery.ML_GENERAL_WeaponMastery;
		lpObj->m_AttackDamageMaxLeft += lpObj->CharacterData->Mastery.ML_RF_Recklessness + lpObj->CharacterData->Mastery.ML_GENERAL_WeaponMastery;
	} else if (lpObj->Class == CLASS_KNIGHT || lpObj->Class == CLASS_MAGICGLADIATOR) {
		lpObj->m_AttackDamageMinRight += lpObj->CharacterData->Mastery.ML_DK_Stability + lpObj->CharacterData->Mastery.ML_GENERAL_WeaponMastery;
		lpObj->m_AttackDamageMaxRight += lpObj->CharacterData->Mastery.ML_DK_Recklessness + lpObj->CharacterData->Mastery.ML_GENERAL_WeaponMastery;
		lpObj->m_AttackDamageMinLeft += lpObj->CharacterData->Mastery.ML_DK_Stability + lpObj->CharacterData->Mastery.ML_GENERAL_WeaponMastery;
		lpObj->m_AttackDamageMaxLeft += lpObj->CharacterData->Mastery.ML_DK_Recklessness + lpObj->CharacterData->Mastery.ML_GENERAL_WeaponMastery;
	} else if (lpObj->Class == CLASS_DARKLORD) {
		int nAddAttack = 0;

		if (lpObj->CharacterData->Mastery.ML_DL_DelusionalGrandeur > 0.0) {
			if (lpObj->pInventory[0].GetDetailItemType() == ITEM_DARKLORD_SCEPTER || lpObj->pInventory[1].GetDetailItemType() == ITEM_DARKLORD_SCEPTER) nAddAttack = (lpObj->AddLeadership + lpObj->Leadership) / lpObj->CharacterData->Mastery.ML_DL_DelusionalGrandeur;
		}

		lpObj->m_AttackDamageMinRight += lpObj->CharacterData->Mastery.ML_DK_Stability + lpObj->CharacterData->Mastery.ML_GENERAL_WeaponMastery + nAddAttack;
		lpObj->m_AttackDamageMaxRight += lpObj->CharacterData->Mastery.ML_DK_Recklessness + lpObj->CharacterData->Mastery.ML_GENERAL_WeaponMastery + nAddAttack;
		lpObj->m_AttackDamageMinLeft += lpObj->CharacterData->Mastery.ML_DK_Stability + lpObj->CharacterData->Mastery.ML_GENERAL_WeaponMastery + nAddAttack;
		lpObj->m_AttackDamageMaxLeft += lpObj->CharacterData->Mastery.ML_DK_Recklessness + lpObj->CharacterData->Mastery.ML_GENERAL_WeaponMastery + nAddAttack;
	}

	lpObj->pInventory[7].PlusSpecial(&lpObj->m_AttackDamageMinRight, 80);
	lpObj->pInventory[7].PlusSpecial(&lpObj->m_AttackDamageMaxRight, 80);
	lpObj->pInventory[7].PlusSpecial(&lpObj->m_AttackDamageMinLeft, 80);
	lpObj->pInventory[7].PlusSpecial(&lpObj->m_AttackDamageMaxLeft, 80);

	int AddLeadership = 0;

	if (lpObj->pInventory[7].IsItem() != FALSE && lpObj->pInventory[7].m_IsValidItem != false) AddLeadership += lpObj->pInventory[7].m_Leadership;
	if (Right->m_Type != -1) {
		if (Right->m_IsValidItem != false) {
			if (Right->m_Type >= ITEMGET(5,0) && Right->m_Type <= ITEMGET(6,0)) {
				lpObj->m_AttackDamageMinRight += Right->m_DamageMin / 2;
				lpObj->m_AttackDamageMaxRight += Right->m_DamageMax / 2;
			} else {
				lpObj->m_AttackDamageMinRight += Right->m_DamageMin;
				lpObj->m_AttackDamageMaxRight += Right->m_DamageMax;
			}
		}

		if (lpObj->pInventory[0].m_SkillChange != FALSE) lpObj->SkillLongSpearChange = true;

		lpObj->pInventory[0].PlusSpecial(&lpObj->m_AttackDamageMinRight, 80);
		lpObj->pInventory[0].PlusSpecial(&lpObj->m_AttackDamageMaxRight, 80);
	}
		
	if (Left->m_Type != -1)	{
		if (Left->m_IsValidItem != false) {
			lpObj->m_AttackDamageMinLeft += Left->m_DamageMin;
			lpObj->m_AttackDamageMaxLeft += Left->m_DamageMax;
		}

		lpObj->pInventory[1].PlusSpecial(&lpObj->m_AttackDamageMinLeft, 80);
		lpObj->pInventory[1].PlusSpecial(&lpObj->m_AttackDamageMaxLeft, 80);
	}

	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 320) >= 10 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 598) >= 10) {				// Brawn
		lpObj->m_AttackDamageMinLeft *= 1.05;
		lpObj->m_AttackDamageMinRight *= 1.05;
		lpObj->m_AttackDamageMaxLeft *= 1.05;
		lpObj->m_AttackDamageMaxRight *= 1.05;
	}

	lpObj->C_CriticalStrikeChance = 0;
	lpObj->C_ExcellentStrikeChance = 0;
	lpObj->pInventory[0].PlusSpecial(&lpObj->C_CriticalStrikeChance, 84);
	lpObj->pInventory[1].PlusSpecial(&lpObj->C_CriticalStrikeChance, 84);
	lpObj->pInventory[2].PlusSpecial(&lpObj->C_CriticalStrikeChance, 84);
	lpObj->pInventory[3].PlusSpecial(&lpObj->C_CriticalStrikeChance, 84);
	lpObj->pInventory[4].PlusSpecial(&lpObj->C_CriticalStrikeChance, 84);
	lpObj->pInventory[5].PlusSpecial(&lpObj->C_CriticalStrikeChance, 84);
	lpObj->pInventory[6].PlusSpecial(&lpObj->C_CriticalStrikeChance, 84);
	lpObj->pInventory[7].PlusSpecial(&lpObj->C_CriticalStrikeChance, 84);
	lpObj->C_CriticalStrikeChance += BUFF_GetTotalBuffEffectValue(lpObj, EFFECTTYPE_INCREASE_CRITICAL_STRIKE_CHANCE);
	lpObj->C_ExcellentStrikeChance += BUFF_GetTotalBuffEffectValue(lpObj, EFFECTTYPE_EXCELLENTDAMAGE);

	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_ARCANE_FOCUS) && g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 383) >= 10) lpObj->C_ExcellentStrikeChance += lpObj->CharacterData->Mastery.ML_ExcellentStrike;
	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_EXCELLENT_CONCENTRATION) && lpObj->CharacterData->Mastery.iMpsIncExcellentDamageRate_Darklord > 0.0) lpObj->C_ExcellentStrikeChance += lpObj->CharacterData->Mastery.iMpsIncExcellentDamageRate_Darklord;
	if (lpObj->Class == CLASS_RAGEFIGHTER) {
		if (lpObj->CharacterData->Mastery.ML_RF_CriticalBlows > 0.0) lpObj->C_CriticalStrikeChance += lpObj->CharacterData->Mastery.ML_RF_CriticalBlows;
		if (lpObj->CharacterData->Mastery.ML_RF_ExcellentBlows > 0.0) lpObj->C_ExcellentStrikeChance += lpObj->CharacterData->Mastery.ML_RF_ExcellentBlows;
	} else {
		if (lpObj->CharacterData->Mastery.ML_GENERAL_CriticalBlows > 0.0) lpObj->C_CriticalStrikeChance += lpObj->CharacterData->Mastery.ML_GENERAL_CriticalBlows;
		if (lpObj->CharacterData->Mastery.ML_GENERAL_ExcellentBlows > 0.0) lpObj->C_ExcellentStrikeChance += lpObj->CharacterData->Mastery.ML_GENERAL_ExcellentBlows;
	}

	switch (lpObj->Class) {
		case CLASS_ELF:											this->m_Lua.Generic_Call("ElfMagicDamageCalc", "i>ii", Energy, &lpObj->m_MagicDamageMin, &lpObj->m_MagicDamageMax);																								break;
		case CLASS_KNIGHT:										this->m_Lua.Generic_Call("KnightMagicDamageCalc", "i>ii", Energy, &lpObj->m_MagicDamageMin, &lpObj->m_MagicDamageMax);																							break;
		case CLASS_MAGICGLADIATOR:								this->m_Lua.Generic_Call("GladiatorMagicDamageCalc", "i>ii", Energy, &lpObj->m_MagicDamageMin, &lpObj->m_MagicDamageMax);																						break;
		case CLASS_DARKLORD:									this->m_Lua.Generic_Call("LordMagicDamageCalc", "i>ii", Energy, &lpObj->m_MagicDamageMin, &lpObj->m_MagicDamageMax);																							break;
		case CLASS_RAGEFIGHTER:									this->m_Lua.Generic_Call("RageFighterMagicDamageCalc", "i>ii", Energy, &lpObj->m_MagicDamageMin, &lpObj->m_MagicDamageMax);																						break;
		case CLASS_SUMMONER:									this->m_Lua.Generic_Call("SummonerMagicDamageCalc", "i>iiii", Energy, &lpObj->m_MagicDamageMin, &lpObj->m_MagicDamageMax, &lpObj->m_CurseDamageMin, &lpObj->m_CurseDamageMax);									break;
		case CLASS_WIZARD:										this->m_Lua.Generic_Call("WizardMagicDamageCalc", "i>ii", Energy, &lpObj->m_MagicDamageMin, &lpObj->m_MagicDamageMax);																							break;
		case CLASS_GROWLANCER:									this->m_Lua.Generic_Call("GrowLancerMagicDamageCalc", "i>ii", Energy, &lpObj->m_MagicDamageMin, &lpObj->m_MagicDamageMax);																						break;
	}

	g_StatSpec.CalcStatOption(lpObj, STAT_OPTION_INC_MAGIC_DAMAGE);
	g_StatSpec.CalcStatOption(lpObj, STAT_OPTION_INC_CURSE_DAMAGE);

	if (lpObj->Class == CLASS_SUMMONER) {
		lpObj->m_MagicDamageMin += lpObj->CharacterData->Mastery.ML_SUM_VoidReach + lpObj->CharacterData->Mastery.ML_SUM_SpellMastery;
		lpObj->m_MagicDamageMax += lpObj->CharacterData->Mastery.ML_SUM_ArcaneFlows + lpObj->CharacterData->Mastery.ML_SUM_SpellMastery;
		lpObj->m_CurseDamageMin += lpObj->CharacterData->Mastery.ML_SUM_VoidReach + lpObj->CharacterData->Mastery.ML_SUM_SpellMastery;
		lpObj->m_CurseDamageMax += lpObj->CharacterData->Mastery.ML_SUM_ArcaneFlows + lpObj->CharacterData->Mastery.ML_SUM_SpellMastery;
	}

	if (lpObj->Class == CLASS_MAGICGLADIATOR) {
		lpObj->m_MagicDamageMin += lpObj->CharacterData->Mastery.ML_GENERAL_ArcaneSubtlety;
		lpObj->m_MagicDamageMax += lpObj->CharacterData->Mastery.ML_GENERAL_ArcaneCombustion;
		lpObj->m_MagicDamageMin += lpObj->CharacterData->Mastery.ML_MG_MagicMastery;
		lpObj->m_MagicDamageMax += lpObj->CharacterData->Mastery.ML_MG_MagicMastery;

		// Magic Mastery
		if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 488) >= 10) {
			lpObj->m_MagicDamageMin += lpObj->m_MagicDamageMin * 10 / 100;
			lpObj->m_MagicDamageMax += lpObj->m_MagicDamageMax * 10 / 100;
		}
	}

	if (lpObj->Class == CLASS_WIZARD) {
		lpObj->m_MagicDamageMin += lpObj->CharacterData->Mastery.ML_GENERAL_ArcaneSubtlety;
		lpObj->m_MagicDamageMax += lpObj->CharacterData->Mastery.ML_GENERAL_ArcaneCombustion;
		lpObj->m_MagicDamageMin += lpObj->CharacterData->Mastery.ML_MG_MagicMastery;
		lpObj->m_MagicDamageMax += lpObj->CharacterData->Mastery.ML_MG_MagicMastery;
	}

	lpObj->pInventory[7].PlusSpecial(&lpObj->m_MagicDamageMin, 81);
	lpObj->pInventory[7].PlusSpecial(&lpObj->m_MagicDamageMax, 81);
	lpObj->pInventory[7].PlusSpecial(&lpObj->m_CurseDamageMin, 113);
	lpObj->pInventory[7].PlusSpecial(&lpObj->m_CurseDamageMax, 113);

	if (Right->m_Type != -1) {
		if (lpObj->pInventory[0].m_Type == ITEMGET(0,31) || lpObj->pInventory[0].m_Type == ITEMGET(0,21) || lpObj->pInventory[0].m_Type == ITEMGET(0,23) || lpObj->pInventory[0].m_Type == ITEMGET(0,25) || lpObj->pInventory[0].m_Type == ITEMGET(0,28) || lpObj->pInventory[0].m_Type == ITEMGET(0,30)) {
			lpObj->pInventory[0].PlusSpecial(&lpObj->m_MagicDamageMin, 80);
			lpObj->pInventory[0].PlusSpecial(&lpObj->m_MagicDamageMax, 80);
		} else {
			lpObj->pInventory[0].PlusSpecial(&lpObj->m_MagicDamageMin, 81);
			lpObj->pInventory[0].PlusSpecial(&lpObj->m_MagicDamageMax, 81);
		}
	}

	if (Left->m_Type != -1) {
		Left->PlusSpecial(&lpObj->m_CurseDamageMin, 113);
		Left->PlusSpecial(&lpObj->m_CurseDamageMax, 113);
	}

	lpObj->HitRating = (Strength + Dexterity) / 2;
	lpObj->HitRating += lpObj->pInventory[5].ItemDefense();
	this->m_Lua.Generic_Call("CalcAttackSpeed", "ii>ii", lpObj->Class, Dexterity, &lpObj->AttackSpeed, &lpObj->MagicSpeed);
	g_StatSpec.CalcStatOption(lpObj, STAT_OPTION_INC_ATTACK_SPEED);

	bool bRight = false;
	bool bLeft = false;

	if (Right->m_Type != ITEMGET(4,7) && Right->m_Type != ITEMGET(4,15) && Right->m_Type >= ITEMGET(0,0) && Right->m_Type < ITEMGET(6,0)) {
		if (Right->m_IsValidItem != false) bRight = true;
	}

	if (Left->m_Type != ITEMGET(4,7) && Left->m_Type != ITEMGET(4,15) && Left->m_Type >= ITEMGET(0,0) && Left->m_Type < ITEMGET(6,0)) {
		if (Left->m_IsValidItem != false) bLeft = true;
	}

	if (bRight != false && bLeft != false) {
		if (Left->m_IsValidItem != false && Right->m_IsValidItem != false) {
			lpObj->AttackSpeed += (Right->AttackSpeed + Left->AttackSpeed) / 2;
			lpObj->MagicSpeed += (Right->AttackSpeed + Left->AttackSpeed) / 2;
		}
	} else if (bRight != false) {
		if (Right->m_IsValidItem != false) {
			lpObj->AttackSpeed += Right->AttackSpeed;
			lpObj->MagicSpeed += Right->AttackSpeed;
		}
	} else if (bLeft != false) {
		if (Left->m_IsValidItem != false) {
			lpObj->AttackSpeed += Left->AttackSpeed;
			lpObj->MagicSpeed += Left->AttackSpeed;
		}
	}

	if (Gloves->m_Type != -1) {
		if (Gloves->m_IsValidItem != false)	{
			lpObj->AttackSpeed += Gloves->AttackSpeed;
			lpObj->MagicSpeed += Gloves->AttackSpeed;
		}
	}

	if (Helper->m_Type != -1) {
		if (Helper->m_IsValidItem != false) {
			lpObj->AttackSpeed += Helper->AttackSpeed;
			lpObj->MagicSpeed += Helper->AttackSpeed;
		}
	}

	if (Amulet->m_Type != -1) {
		if (Amulet->m_IsValidItem != false) {
			lpObj->AttackSpeed += Amulet->AttackSpeed;
			lpObj->MagicSpeed += Amulet->AttackSpeed;
		}
	}

	if (Wing->IsItem() == TRUE && Wing->m_IsValidItem == true) {
		if (Wing->m_Durability > 0.0f) {
			switch (Wing->m_Type) {
				case ITEMGET(13, 30):																// Lord's Cape
				case ITEMGET(13, 262):			lpObj->AddStamina += 100;				break;			// Cloak of Death
				case ITEMGET(12, 40):			lpObj->AddStamina += 150;				break;			// Emperor's Cape
			}
		}
	}

	if (Wing->IsItem() == TRUE && Wing->m_IsValidItem == true && Wing->m_Type == ITEMGET(12,268)) {
		lpObj->m_AttackDamageMinRight += 100 + (Wing->m_Level * 5);
		lpObj->m_AttackDamageMaxRight += 100 + (Wing->m_Level * 5);
		lpObj->m_AttackDamageMinLeft += 100 + (Wing->m_Level * 5);
		lpObj->m_AttackDamageMaxLeft += 100 + (Wing->m_Level * 5);
		lpObj->m_MagicDamageMin += 100 + (Wing->m_Level * 5);
		lpObj->m_MagicDamageMax += 100 + (Wing->m_Level * 5);
		lpObj->m_CurseDamageMin += 100 + (Wing->m_Level * 5);
		lpObj->m_CurseDamageMax += 100 + (Wing->m_Level * 5);
	}

	// Wizard's Ring
	if ((lpObj->pInventory[10].IsItem() == TRUE && lpObj->pInventory[10].m_Type == ITEMGET(13,20) && lpObj->pInventory[10].m_Level == 0 && lpObj->pInventory[10].m_Durability > 0.0f) || (lpObj->pInventory[11].IsItem() == TRUE && lpObj->pInventory[11].m_Type == ITEMGET(13,20) && lpObj->pInventory[11].m_Level == 0 && lpObj->pInventory[11].m_Durability > 0.0f)) {
		lpObj->m_AttackDamageMinRight += lpObj->m_AttackDamageMinRight * g_ConfigRead.pet.WizardRingAddDamage / 100;
		lpObj->m_AttackDamageMaxRight += lpObj->m_AttackDamageMaxRight * g_ConfigRead.pet.WizardRingAddDamage / 100;
		lpObj->m_AttackDamageMinLeft += lpObj->m_AttackDamageMinLeft * g_ConfigRead.pet.WizardRingAddDamage / 100;
		lpObj->m_AttackDamageMaxLeft += lpObj->m_AttackDamageMaxLeft * g_ConfigRead.pet.WizardRingAddDamage / 100;
		lpObj->m_MagicDamageMin += lpObj->m_MagicDamageMin * g_ConfigRead.pet.WizardRingAddMagicDamage / 100;
		lpObj->m_MagicDamageMax += lpObj->m_MagicDamageMax * g_ConfigRead.pet.WizardRingAddMagicDamage / 100;
		lpObj->AttackSpeed += g_ConfigRead.pet.WizardRingAddAttackSpeed;
		lpObj->MagicSpeed += g_ConfigRead.pet.WizardRingAddAttackSpeed;
	}
	// Lethal Wizard's Ring
	if ((lpObj->pInventory[10].IsItem() == TRUE && lpObj->pInventory[10].m_Type == ITEMGET(13,107) && lpObj->pInventory[10].m_Level == 0 && lpObj->pInventory[10].m_Durability > 0.0f) || (lpObj->pInventory[11].IsItem() == TRUE && lpObj->pInventory[11].m_Type == ITEMGET(13,107) && lpObj->pInventory[11].m_Level == 0 && lpObj->pInventory[11].m_Durability > 0.0f)) {
		lpObj->m_AttackDamageMinRight += lpObj->m_AttackDamageMinRight * g_ConfigRead.pet.LethalRingAddDamage / 100;
		lpObj->m_AttackDamageMaxRight += lpObj->m_AttackDamageMaxRight * g_ConfigRead.pet.LethalRingAddDamage / 100;
		lpObj->m_AttackDamageMinLeft += lpObj->m_AttackDamageMinLeft * g_ConfigRead.pet.LethalRingAddDamage / 100;
		lpObj->m_AttackDamageMaxLeft += lpObj->m_AttackDamageMaxLeft * g_ConfigRead.pet.LethalRingAddDamage / 100;
		lpObj->m_MagicDamageMin += lpObj->m_MagicDamageMin * g_ConfigRead.pet.LethalRingAddMagicDamage / 100;
		lpObj->m_MagicDamageMax += lpObj->m_MagicDamageMax * g_ConfigRead.pet.LethalRingAddMagicDamage / 100;
		lpObj->AttackSpeed += g_ConfigRead.pet.LethalRingAddAttackSpeed;
		lpObj->MagicSpeed += g_ConfigRead.pet.LethalRingAddAttackSpeed;
	}
	// Rudolph
	if (Helper->m_Type == ITEMGET(13, 67) && Helper->IsPeriodItemExpire() == FALSE) {
		for (int x = 0; x < 7; x++) lpObj->C_Resistance[x] += (char)1;
	}
	// Unicorn
	if (Helper->m_Type == ITEMGET(13, 106) && Helper->IsPeriodItemExpire() == FALSE) {
		lpObj->Defense += g_ConfigRead.pet.UnicornAddDefense;
		lpObj->AddStamina += 100;
	}
	// Skeletal Dragon
	if (Helper->m_Type == ITEMGET(13, 123) && Helper->IsPeriodItemExpire() == FALSE) {
		lpObj->m_AttackDamageMinRight += lpObj->m_AttackDamageMinRight * g_ConfigRead.pet.EliteSkeletonPetAddDamage / 100;
		lpObj->m_AttackDamageMaxRight += lpObj->m_AttackDamageMaxRight * g_ConfigRead.pet.EliteSkeletonPetAddDamage / 100;
		lpObj->m_AttackDamageMinLeft += lpObj->m_AttackDamageMinLeft * g_ConfigRead.pet.EliteSkeletonPetAddDamage / 100;
		lpObj->m_AttackDamageMaxLeft += lpObj->m_AttackDamageMaxLeft * g_ConfigRead.pet.EliteSkeletonPetAddDamage / 100;
		lpObj->m_MagicDamageMin += lpObj->m_MagicDamageMin * g_ConfigRead.pet.EliteSkeletonPetAddDamage / 100;
		lpObj->m_MagicDamageMax += lpObj->m_MagicDamageMax * g_ConfigRead.pet.EliteSkeletonPetAddDamage / 100;
		lpObj->AttackSpeed += g_ConfigRead.pet.EliteSkeletonPetAddAttackSpeed;
		lpObj->MagicSpeed += g_ConfigRead.pet.EliteSkeletonPetAddAttackSpeed;
	}

	if (lpObj->Class) lpObj->m_DetectSpeedHackTime = (gAttackSpeedTimeLimit - lpObj->AttackSpeed * gDecTimePerAttackSpeed);
	else lpObj->m_DetectSpeedHackTime = (gAttackSpeedTimeLimit - (2 * lpObj->MagicSpeed) * gDecTimePerAttackSpeed);
	if (lpObj->m_DetectSpeedHackTime < gMinimumAttackSpeedTime) lpObj->m_DetectSpeedHackTime = gMinimumAttackSpeedTime;

	this->m_Lua.Generic_Call("CalcAttackSuccessRate_PvM", "iiiii>i", lpObj->Class, Strength, Dexterity,	Leadership, (lpObj->Level + lpObj->CharacterData->MasterLevel), &lpObj->CharacterData->HitRating);
	this->m_Lua.Generic_Call("CalcDefenseSuccessRate_PvM", "ii>i", lpObj->Class, Dexterity,	&lpObj->EvasionRating);
	this->m_Lua.Generic_Call("CalcAttackSuccessRate_PvP", "iii>d", lpObj->Class, (lpObj->CharacterData->Dexterity + lpObj->AddDexterity), lpObj->Level + lpObj->CharacterData->MasterLevel, &lpObj->CharacterData->HitRatingPvP);
	this->m_Lua.Generic_Call("CalcDefenseSuccessRate_PvP", "iii>d", lpObj->Class, (lpObj->CharacterData->Dexterity + lpObj->AddDexterity), lpObj->Level + lpObj->CharacterData->MasterLevel, &lpObj->CharacterData->EvasionRatingPvP);
	g_StatSpec.CalcStatOption(lpObj, STAT_OPTION_INC_ATTACK_RATE);
	g_StatSpec.CalcStatOption(lpObj, STAT_OPTION_INC_ATTACK_RATE_PVP);
	g_StatSpec.CalcStatOption(lpObj, STAT_OPTION_INC_DEFENSE_RATE);
	g_StatSpec.CalcStatOption(lpObj, STAT_OPTION_INC_DEFENSE_RATE_PVP);

	if (Left->m_Type != -1)	{
		if (Left->m_IsValidItem != false) {
			lpObj->EvasionRating += Left->EvasionRating;
			lpObj->pInventory[1].PlusSpecial(&lpObj->EvasionRating, 82);

			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 399) == 20 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 437) == 20 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 528) == 20) lpObj->Defense += Left->ItemDefense() * 0.2;		// Shield Mastery
		}
	}

	bool Success = true;

	if (lpObj->Class == CLASS_MAGICGLADIATOR) {
		for (int j = 3; j <= 6; j++) {
			if (lpObj->pInventory[j].m_Type == -1) {
				Success = false;
				break;
			}

			if (lpObj->pInventory[j].m_IsValidItem == false) {
				Success = false;
				break;
			}
		}
	} else if (lpObj->Class == CLASS_RAGEFIGHTER) {
		for  (int l = 2; l <= 6; l++) {
			if (l == 5) continue;
			if (lpObj->pInventory[l].m_Type == -1) {
				Success = false;
				break;
			}

			if (lpObj->pInventory[l].m_IsValidItem == false) {
				Success = false;
				break;
			}
		}
	} else {
		for  (int k = 2; k <= 6; k++)	{
			if (lpObj->pInventory[k].m_Type == -1) {
				Success = false;
				break;
			}

			if (lpObj->pInventory[k].m_IsValidItem == false) {
				Success = false;
				break;
			}
		}
	}

	int Level11Count = 0;
	int Level10Count = 0;
	int Level12Count = 0;
	int Level13Count = 0;
	int Level14Count = 0;
	int Level15Count = 0;
	float EvRatePercent = 1;
	float EvRateSetBonus = 1;
	float DefensePercent = 1;

	if (Success != false) {
		int in;

		if (lpObj->Class == CLASS_MAGICGLADIATOR) {
			in = lpObj->pInventory[3].m_Type % MAX_SUBTYPE_ITEMS;

			if (in != ITEMGET(0,15) && in != ITEMGET(0,20) && in != ITEMGET(0,23) && in != ITEMGET(0,33) && in != ITEMGET(0,32) && in != ITEMGET(0,37)) Success = false;
			else {
				Level13Count++;

				for (int m = 3; m<= 6; m++) {
					if (in != (lpObj->pInventory[m].m_Type % MAX_SUBTYPE_ITEMS)) Success = false;
					if (lpObj->pInventory[m].m_Level > 14) Level15Count++;
					else if (lpObj->pInventory[m].m_Level > 13) Level14Count++;
					else if (lpObj->pInventory[m].m_Level > 12) Level13Count++;
					else if (lpObj->pInventory[m].m_Level > 11) Level12Count++;
					else if (lpObj->pInventory[m].m_Level > 10) Level11Count++;
					else if (lpObj->pInventory[m].m_Level > 9) Level10Count++;
				}
			}
		} else {
			in = lpObj->pInventory[2].m_Type % MAX_SUBTYPE_ITEMS;

			for (int m = 2; m <= 6; m++) {
				if (in != (lpObj->pInventory[m].m_Type % MAX_SUBTYPE_ITEMS)) Success = false;
				if (lpObj->pInventory[m].m_Level > 14) Level15Count++;
				else if (lpObj->pInventory[m].m_Level > 13) Level14Count++;
				else if (lpObj->pInventory[m].m_Level > 12) Level13Count++;
				else if (lpObj->pInventory[m].m_Level > 11) Level12Count++;
				else if (lpObj->pInventory[m].m_Level > 10) Level11Count++;
				else if (lpObj->pInventory[m].m_Level > 9) Level10Count++;
			}
		}

		if (Success != false) EvRatePercent += 0.1;		// Switch -> EvRateSetBonus
	}


	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 319) >= 10 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 597) >= 10) EvRatePercent += 0.1;				// Quickness
	if (lpObj->Class == CLASS_RAGEFIGHTER) {
		if (lpObj->CharacterData->Mastery.ML_RF_SuddenMovements > 0.0) lpObj->EvasionRating += lpObj->CharacterData->Mastery.ML_RF_SuddenMovements;
	} else if (lpObj->CharacterData->Mastery.ML_GENERAL_SuddenMovements > 0.0) lpObj->EvasionRating += lpObj->CharacterData->Mastery.ML_GENERAL_SuddenMovements;

	lpObj->EvasionRating += lpObj->CharacterData->Mastery.ML_GENERAL_ShieldExpertise;
	lpObj->EvasionRating += lpObj->CharacterData->Mastery.ML_ELF_ShieldExpertise;
	lpObj->EvasionRating += lpObj->CharacterData->Mastery.ML_DL_ShieldExpertise;
	this->m_Lua.Generic_Call("CalcDefense", "ii>i", lpObj->Class, Dexterity, &lpObj->Defense);
	g_StatSpec.CalcStatOption(lpObj, STAT_OPTION_INC_DEFENSE);
	lpObj->Defense += lpObj->pInventory[2].ItemDefense();
	lpObj->Defense += lpObj->pInventory[3].ItemDefense();
	lpObj->Defense += lpObj->pInventory[4].ItemDefense();
	lpObj->Defense += lpObj->pInventory[5].ItemDefense();
	lpObj->Defense += lpObj->pInventory[6].ItemDefense();
	lpObj->Defense += lpObj->pInventory[1].ItemDefense();
	lpObj->Defense += lpObj->pInventory[7].ItemDefense();

	if (lpObj->pInventory[7].IsItem() == TRUE && lpObj->pInventory[7].m_IsValidItem == true) lpObj->Defense += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDefense;
	if (lpObj->m_btInvenPetPos != FALSE && lpObj->pInventory[lpObj->m_btInvenPetPos].m_JewelOfHarmonyOption == TRUE) {
		if (lpObj->pInventory[lpObj->m_btInvenPetPos].m_Type == ITEMGET(13,4) && lpObj->pInventory[lpObj->m_btInvenPetPos].m_Durability > 0.0f)	{
			int addDefense = 0;

			this->m_Lua.Generic_Call("CalcDarkHorseDefenseBonus", "ii>i", Dexterity, lpObj->pInventory[lpObj->m_btInvenPetPos].m_PetItem_Level, &addDefense);
			lpObj->Defense += addDefense;

			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 510) == 20) EvRatePercent += 0.1;
		}
	}

	if ((Level15Count + Level14Count + Level13Count + Level12Count + Level11Count + Level10Count) >= 5)	{
		if (Success != false) {
			if (Level15Count == 5) lpObj->Defense += lpObj->Defense * 30 / 100;
			else if (Level14Count == 5 || (Level14Count + Level15Count) == 5) lpObj->Defense += lpObj->Defense * 25 / 100;
			else if (Level13Count == 5 || (Level13Count + Level14Count + Level15Count) == 5) lpObj->Defense += lpObj->Defense * 20 / 100;
			else if (Level12Count == 5 || (Level12Count + Level13Count + Level14Count + Level15Count) == 5) lpObj->Defense += lpObj->Defense * 15 / 100;
			else if (Level11Count == 5 || (Level11Count + Level12Count + Level13Count + Level14Count + Level15Count) == 5) lpObj->Defense += lpObj->Defense * 10 / 100;
			else if (Level10Count == 5 || (Level10Count + Level11Count + Level12Count + Level13Count + Level14Count + Level15Count) == 5) lpObj->Defense += lpObj->Defense * 5 / 100;
		}
	}

	if (lpObj->Class == CLASS_RAGEFIGHTER) lpObj->Defense += lpObj->CharacterData->Mastery.ML_RF_PhysicalResistance;
	else lpObj->Defense += lpObj->CharacterData->Mastery.ML_GENERAL_PhysicalResistance;
	if (lpObj->CharacterData->Mastery.iMpsMonkAddVitalToDefense > 0.0) lpObj->Defense += Vitality / lpObj->CharacterData->Mastery.iMpsMonkAddVitalToDefense;

	BOOL nFullSet = TRUE;

	if (lpObj->Class == CLASS_MAGICGLADIATOR) {
		if (Armor->IsItem() == FALSE) nFullSet = FALSE;
		if (Gloves->IsItem() == FALSE) nFullSet = FALSE;
		if (Pants->IsItem() == FALSE) nFullSet = FALSE;
		if (Boots->IsItem() == FALSE) nFullSet = FALSE;
	}

	if (lpObj->Class == CLASS_RAGEFIGHTER) {
		if (Helmet->IsItem() == FALSE) nFullSet = FALSE;
		if (Armor->IsItem() == FALSE) nFullSet = FALSE;
		if (Pants->IsItem() == FALSE) nFullSet = FALSE;
		if (Boots->IsItem() == FALSE) nFullSet = FALSE;
	} else {
		if (Helmet->IsItem() == FALSE) nFullSet = FALSE;
		if (Armor->IsItem() == FALSE) nFullSet = FALSE;
		if (Gloves->IsItem() == FALSE) nFullSet = FALSE;
		if (Pants->IsItem() == FALSE) nFullSet = FALSE;
		if (Boots->IsItem() == FALSE) nFullSet = FALSE;
	}

	if (nFullSet == TRUE) {
		if (lpObj->Class == CLASS_RAGEFIGHTER) lpObj->Defense += lpObj->CharacterData->Mastery.ML_RF_Valor;
		else lpObj->Defense += lpObj->CharacterData->Mastery.ML_GENERAL_Valor;
	}

	if ((LeftRing->IsItem() == TRUE && LeftRing->m_Type == ITEMGET(13,10)) || (RightRing->IsItem() == TRUE && RightRing->m_Type == ITEMGET(13,10))) {																				// Transformation Ring
		if (LeftRing->m_Level == 5 || RightRing->m_Level == 5) addLifePercent += 0.2;
	} else if ((LeftRing->IsItem() == TRUE && LeftRing->m_Type == ITEMGET(13,39)) || (RightRing->IsItem() == TRUE && RightRing->m_Type == ITEMGET(13,39))) {																		// Chief Skeleton Warrior's Ring
		lpObj->Defense += (lpObj->Defense * g_ConfigRead.pet.EliteSkeletonRingAddDefense / 100);
		lpObj->AddLife += lpObj->Level + lpObj->CharacterData->MasterLevel;
	} else if ((LeftRing->IsItem() == TRUE && LeftRing->m_Type == ITEMGET(13,41)) || (RightRing->IsItem() == TRUE && RightRing->m_Type == ITEMGET(13,41))) {																		// Christmas Ring
		lpObj->m_AttackDamageMinRight += lpObj->m_AttackDamageMinRight * g_ConfigRead.pet.ChristmasRingAddDamage / 100;
		lpObj->m_AttackDamageMaxRight += lpObj->m_AttackDamageMaxRight * g_ConfigRead.pet.ChristmasRingAddDamage / 100;
		lpObj->m_AttackDamageMinLeft += lpObj->m_AttackDamageMinLeft * g_ConfigRead.pet.ChristmasRingAddDamage / 100;
		lpObj->m_AttackDamageMaxLeft += lpObj->m_AttackDamageMaxLeft * g_ConfigRead.pet.ChristmasRingAddDamage / 100;
		lpObj->m_MagicDamageMin += lpObj->m_MagicDamageMin * g_ConfigRead.pet.ChristmasRingAddMagicDamage / 100;
		lpObj->m_MagicDamageMax += lpObj->m_MagicDamageMax * g_ConfigRead.pet.ChristmasRingAddMagicDamage / 100;
	} else if ((LeftRing->IsItem() == TRUE && LeftRing->m_Type == ITEMGET(13,42)) || (RightRing->IsItem() == TRUE && RightRing->m_Type == ITEMGET(13,42))) {																		// GM Ring
		lpObj->C_Resistance[R_COLD] += (char)255;
		lpObj->C_Resistance[R_POISON] += (char)255;
		lpObj->C_Resistance[R_LIGHTNING] += (char)255;
		lpObj->C_Resistance[R_FIRE] += (char)255;
		lpObj->C_Resistance[R_EARTH] += (char)255;
		lpObj->C_Resistance[R_WIND] += (char)255;
		lpObj->C_Resistance[R_WATER] += (char)255;
	} else if ((LeftRing->IsItem() == TRUE && LeftRing->m_Type == ITEMGET(13,76)) || (RightRing->IsItem() == TRUE && RightRing->m_Type == ITEMGET(13,76))) {																		// Panda Ring
		lpObj->m_AttackDamageMinRight += g_ConfigRead.pet.PandaRingAddDamage;
		lpObj->m_AttackDamageMaxRight += g_ConfigRead.pet.PandaRingAddDamage;
		lpObj->m_AttackDamageMinLeft += g_ConfigRead.pet.PandaRingAddDamage;
		lpObj->m_AttackDamageMaxLeft += g_ConfigRead.pet.PandaRingAddDamage;
		lpObj->m_MagicDamageMin += g_ConfigRead.pet.PandaRingAddDamage;
		lpObj->m_MagicDamageMax += g_ConfigRead.pet.PandaRingAddDamage;
		lpObj->m_CurseDamageMin += g_ConfigRead.pet.PandaRingAddDamage;
		lpObj->m_CurseDamageMax += g_ConfigRead.pet.PandaRingAddDamage;
	} else if ((LeftRing->IsItem() == TRUE && LeftRing->m_Type == ITEMGET(13,77)) || (RightRing->IsItem() == TRUE && RightRing->m_Type == ITEMGET(13,77))) {																		// Brown Panda Ring
		lpObj->m_AttackDamageMinRight += g_ConfigRead.pet.BrownPandaRingAddDamage;
		lpObj->m_AttackDamageMaxRight += g_ConfigRead.pet.BrownPandaRingAddDamage;
		lpObj->m_AttackDamageMinLeft += g_ConfigRead.pet.BrownPandaRingAddDamage;
		lpObj->m_AttackDamageMaxLeft += g_ConfigRead.pet.BrownPandaRingAddDamage;
		lpObj->m_MagicDamageMin += g_ConfigRead.pet.BrownPandaRingAddDamage;
		lpObj->m_MagicDamageMax += g_ConfigRead.pet.BrownPandaRingAddDamage;
		lpObj->m_CurseDamageMin += g_ConfigRead.pet.BrownPandaRingAddDamage;
		lpObj->m_CurseDamageMax += g_ConfigRead.pet.BrownPandaRingAddDamage;
	} else if ((LeftRing->IsItem() == TRUE && LeftRing->m_Type == ITEMGET(13,78)) || (RightRing->IsItem() == TRUE && RightRing->m_Type == ITEMGET(13,78))) {																		// Pink Panda Ring
		lpObj->m_AttackDamageMinRight += g_ConfigRead.pet.PinkPandaRingAddDamage;
		lpObj->m_AttackDamageMaxRight += g_ConfigRead.pet.PinkPandaRingAddDamage;
		lpObj->m_AttackDamageMinLeft += g_ConfigRead.pet.PinkPandaRingAddDamage;
		lpObj->m_AttackDamageMaxLeft += g_ConfigRead.pet.PinkPandaRingAddDamage;
		lpObj->m_MagicDamageMin += g_ConfigRead.pet.PinkPandaRingAddDamage;
		lpObj->m_MagicDamageMax += g_ConfigRead.pet.PinkPandaRingAddDamage;
		lpObj->m_CurseDamageMin += g_ConfigRead.pet.PinkPandaRingAddDamage;
		lpObj->m_CurseDamageMax += g_ConfigRead.pet.PinkPandaRingAddDamage;
	} else if ((LeftRing->IsItem() == TRUE && LeftRing->m_Type == ITEMGET(13,122)) || (RightRing->IsItem() == TRUE && RightRing->m_Type == ITEMGET(13,122))) {																		// Skeletal Ring
		lpObj->m_AttackDamageMinRight += g_ConfigRead.pet.SkeletonRingAddDamage;
		lpObj->m_AttackDamageMaxRight += g_ConfigRead.pet.SkeletonRingAddDamage;
		lpObj->m_AttackDamageMinLeft += g_ConfigRead.pet.SkeletonRingAddDamage;
		lpObj->m_AttackDamageMaxLeft += g_ConfigRead.pet.SkeletonRingAddDamage;
		lpObj->m_MagicDamageMin += g_ConfigRead.pet.SkeletonRingAddDamage;
		lpObj->m_MagicDamageMax += g_ConfigRead.pet.SkeletonRingAddDamage;
		lpObj->m_CurseDamageMin += g_ConfigRead.pet.SkeletonRingAddDamage;
		lpObj->m_CurseDamageMax += g_ConfigRead.pet.SkeletonRingAddDamage;
		lpObj->AttackSpeed += 10;
		lpObj->MagicSpeed += 10;
	} else if ((LeftRing->IsItem() == TRUE && LeftRing->m_Type == ITEMGET(13, 163)) || (RightRing->IsItem() == TRUE && RightRing->m_Type == ITEMGET(13, 163))) {																	// Robot Knight Ring
		lpObj->Defense += g_ConfigRead.pet.RobotKnightRingAddDefense;
		lpObj->m_AttackDamageMinRight += g_ConfigRead.pet.RobotKnightRingAddDamage;
		lpObj->m_AttackDamageMaxRight += g_ConfigRead.pet.RobotKnightRingAddDamage;
		lpObj->m_AttackDamageMinLeft += g_ConfigRead.pet.RobotKnightRingAddDamage;
		lpObj->m_AttackDamageMaxLeft += g_ConfigRead.pet.RobotKnightRingAddDamage;
		lpObj->m_MagicDamageMin += g_ConfigRead.pet.RobotKnightRingAddDamage;
		lpObj->m_MagicDamageMax += g_ConfigRead.pet.RobotKnightRingAddDamage;
		lpObj->m_CurseDamageMin += g_ConfigRead.pet.RobotKnightRingAddDamage;
		lpObj->m_CurseDamageMax += g_ConfigRead.pet.RobotKnightRingAddDamage;
	} else if ((LeftRing->IsItem() == TRUE && LeftRing->m_Type == ITEMGET(13, 164)) || (RightRing->IsItem() == TRUE && RightRing->m_Type == ITEMGET(13, 164))) {																	// Mini Robot Ring
		lpObj->AttackSpeed += g_ConfigRead.pet.MiniRobotRingAddAttackSpeed;
		lpObj->AttackSpeed += g_ConfigRead.pet.MiniRobotRingAddAttackSpeed;
		lpObj->m_AttackDamageMinRight += g_ConfigRead.pet.MiniRobotRingAddDamage;
		lpObj->m_AttackDamageMaxRight += g_ConfigRead.pet.MiniRobotRingAddDamage;
		lpObj->m_AttackDamageMinLeft += g_ConfigRead.pet.MiniRobotRingAddDamage;
		lpObj->m_AttackDamageMaxLeft += g_ConfigRead.pet.MiniRobotRingAddDamage;
		lpObj->m_MagicDamageMin += g_ConfigRead.pet.MiniRobotRingAddDamage;
		lpObj->m_MagicDamageMax += g_ConfigRead.pet.MiniRobotRingAddDamage;
		lpObj->m_CurseDamageMin += g_ConfigRead.pet.MiniRobotRingAddDamage;
		lpObj->m_CurseDamageMax += g_ConfigRead.pet.MiniRobotRingAddDamage;
	} else if ((LeftRing->IsItem() == TRUE && LeftRing->m_Type == ITEMGET(13, 165)) || (RightRing->IsItem() == TRUE && RightRing->m_Type == ITEMGET(13, 165))) {																	// Great Heavenly Mage Ring
		lpObj->m_AttackDamageMinRight += g_ConfigRead.pet.MageRingAddDamage;
		lpObj->m_AttackDamageMaxRight += g_ConfigRead.pet.MageRingAddDamage;
		lpObj->m_AttackDamageMinLeft += g_ConfigRead.pet.MageRingAddDamage;
		lpObj->m_AttackDamageMaxLeft += g_ConfigRead.pet.MageRingAddDamage;
		lpObj->m_MagicDamageMin += g_ConfigRead.pet.MageRingAddDamage;
		lpObj->m_MagicDamageMax += g_ConfigRead.pet.MageRingAddDamage;
		lpObj->m_CurseDamageMin += g_ConfigRead.pet.MageRingAddDamage;
		lpObj->m_CurseDamageMax += g_ConfigRead.pet.MageRingAddDamage;
	} else if ((LeftRing->IsItem() == TRUE && LeftRing->m_Type == ITEMGET(13,169) && LeftRing->m_Durability > 0.0) || (RightRing->IsItem() == TRUE && RightRing->m_Type == ITEMGET(13,169) && RightRing->m_Durability > 0.0)) {		// Decoration Ring
		lpObj->m_AttackDamageMinRight += lpObj->m_AttackDamageMinRight * g_ConfigRead.pet.DecorationRingAddDamage / 100;
		lpObj->m_AttackDamageMaxRight += lpObj->m_AttackDamageMaxRight * g_ConfigRead.pet.DecorationRingAddDamage / 100;
		lpObj->m_AttackDamageMinLeft += lpObj->m_AttackDamageMinLeft * g_ConfigRead.pet.DecorationRingAddDamage / 100;
		lpObj->m_AttackDamageMaxLeft += lpObj->m_AttackDamageMaxLeft * g_ConfigRead.pet.DecorationRingAddDamage / 100;
		lpObj->m_MagicDamageMin += lpObj->m_MagicDamageMin * g_ConfigRead.pet.DecorationRingAddDamage / 100;
		lpObj->m_MagicDamageMax += lpObj->m_MagicDamageMax * g_ConfigRead.pet.DecorationRingAddDamage / 100;
		lpObj->AttackSpeed += g_ConfigRead.pet.DecorationRingAddAttackSpeed;
		lpObj->MagicSpeed += g_ConfigRead.pet.DecorationRingAddAttackSpeed;
	} else if ((LeftRing->IsItem() == TRUE && LeftRing->m_Type == ITEMGET(13,170) && LeftRing->m_Durability > 0.0) || (RightRing->IsItem() == TRUE && RightRing->m_Type == ITEMGET(13,170) && RightRing->m_Durability > 0.0)) {		// Blessed Decoration Ring
		lpObj->m_AttackDamageMinRight += lpObj->m_AttackDamageMinRight * g_ConfigRead.pet.BlessedDecorationRingAddDamage / 100;
		lpObj->m_AttackDamageMaxRight += lpObj->m_AttackDamageMaxRight * g_ConfigRead.pet.BlessedDecorationRingAddDamage / 100;
		lpObj->m_AttackDamageMinLeft += lpObj->m_AttackDamageMinLeft * g_ConfigRead.pet.BlessedDecorationRingAddDamage / 100;
		lpObj->m_AttackDamageMaxLeft += lpObj->m_AttackDamageMaxLeft * g_ConfigRead.pet.BlessedDecorationRingAddDamage / 100;
		lpObj->m_MagicDamageMin += lpObj->m_MagicDamageMin * g_ConfigRead.pet.BlessedDecorationRingAddDamage / 100;
		lpObj->m_MagicDamageMax += lpObj->m_MagicDamageMax * g_ConfigRead.pet.BlessedDecorationRingAddDamage / 100;
		lpObj->AttackSpeed += g_ConfigRead.pet.BlessedDecorationRingAddAttackSpeed;
		lpObj->MagicSpeed += g_ConfigRead.pet.BlessedDecorationRingAddAttackSpeed;
	} else if ((LeftRing->IsItem() == TRUE && LeftRing->m_Type == ITEMGET(13, 268)) || (RightRing->IsItem() == TRUE && RightRing->m_Type == ITEMGET(13, 268))) {																	// Dark Transformation Ring
		lpObj->m_AttackDamageMinRight += g_ConfigRead.pet.DSFRingAddDamage;
		lpObj->m_AttackDamageMaxRight += g_ConfigRead.pet.DSFRingAddDamage;
		lpObj->m_AttackDamageMinLeft += g_ConfigRead.pet.DSFRingAddDamage;
		lpObj->m_AttackDamageMaxLeft += g_ConfigRead.pet.DSFRingAddDamage;
		lpObj->m_MagicDamageMin += g_ConfigRead.pet.DSFRingAddDamage;
		lpObj->m_MagicDamageMax += g_ConfigRead.pet.DSFRingAddDamage;
		lpObj->m_CurseDamageMin += g_ConfigRead.pet.DSFRingAddDamage;
		lpObj->m_CurseDamageMax += g_ConfigRead.pet.DSFRingAddDamage;
		lpObj->AttackSpeed += g_ConfigRead.pet.DSFRingAddAttackSpeed;
		lpObj->MagicSpeed += g_ConfigRead.pet.DSFRingAddAttackSpeed;
		lpObj->Defense += g_ConfigRead.pet.DSFRingAddDefense;
		lpObj->AddLife += g_ConfigRead.pet.DSFRingAddHP;
	}

	if (lpObj->pInventory[8].m_Type == ITEMGET(13,0)) lpObj->AddLife += g_ConfigRead.pet.AngelAddHP;		// Guardian Angel
	// Golden Fenrir
	if (lpObj->pInventory[lpObj->m_btInvenPetPos].m_Type == ITEMGET(13,37) && lpObj->pInventory[lpObj->m_btInvenPetPos].m_NewOption == 4 && lpObj->pInventory[lpObj->m_btInvenPetPos].m_Durability > 0.0f && lpObj->pInventory[lpObj->m_btInvenPetPos].m_JewelOfHarmonyOption == TRUE) {
		lpObj->AddLife += 200;
		lpObj->AddMana += 200;
	}
	
	// Dinorant
	if (lpObj->pInventory[lpObj->m_btInvenPetPos].m_Type == ITEMGET(13,3) && lpObj->pInventory[lpObj->m_btInvenPetPos].m_Durability > 0.0 && lpObj->pInventory[lpObj->m_btInvenPetPos].m_JewelOfHarmonyOption == TRUE) lpObj->pInventory[lpObj->m_btInvenPetPos].PlusSpecial(&lpObj->AddStamina, 103);
	// Dark Crow
	if (lpObj->Type == OBJ_USER && lpObj->CharacterData->ISBOT == false) gDarkSpirit[lpObj->m_Index- g_ConfigRead.server.GetObjectStartUserIndex()].Set(lpObj->m_Index, &lpObj->pInventory[1]);
	// Summoner Books
	if (lpObj->Class == CLASS_SUMMONER && Left->IsItem()) lpObj->m_CurseSpell = Left->IsCurseSpell();

	int nMuunItemEffectValue = 0;

	if (g_CMuunSystem.GetMuunItemValueOfOptType(lpObj, MUUN_INC_ATTACK_POWER, &nMuunItemEffectValue, 0) == 1) {
		lpObj->m_AttackDamageMaxLeft += nMuunItemEffectValue;
		lpObj->m_AttackDamageMinLeft += nMuunItemEffectValue;
		lpObj->m_AttackDamageMaxRight += nMuunItemEffectValue;
		lpObj->m_AttackDamageMinRight += nMuunItemEffectValue;
		lpObj->m_MagicDamageMin += nMuunItemEffectValue;
		lpObj->m_MagicDamageMax += nMuunItemEffectValue;
		lpObj->m_CurseDamageMin += nMuunItemEffectValue;
		lpObj->m_CurseDamageMax += nMuunItemEffectValue;
	}

	if (g_CMuunSystem.GetMuunItemValueOfOptType(lpObj, MUUN_INC_MAX_ATTACK_POWER, &nMuunItemEffectValue, 0) == 1) {
		lpObj->m_AttackDamageMaxLeft += nMuunItemEffectValue;
		lpObj->m_AttackDamageMaxRight += nMuunItemEffectValue;
		lpObj->m_MagicDamageMax += nMuunItemEffectValue;
		lpObj->m_CurseDamageMax += nMuunItemEffectValue;
	}

	lpObj->pInventory[9].PlusSpecialPercentEx(&lpObj->AddStamina, lpObj->MaxStamina, 173);
	lpObj->pInventory[10].PlusSpecialPercentEx(&lpObj->AddMana, lpObj->MaxMana, 172);
	lpObj->pInventory[11].PlusSpecialPercentEx(&lpObj->AddMana, lpObj->MaxMana, 172);

	CItem* rItem[3];
	
	rItem[0] = &lpObj->pInventory[10];
	rItem[1] = &lpObj->pInventory[11];
	rItem[2] = &lpObj->pInventory[9];

	#define GET_MAX_RESISTANCE(x, y, z) (((((x) > (y)) ? (x) : (y)) > (z)) ? (((x) > (y)) ? (x) : (y)) : (z))	

	lpObj->C_Resistance[R_COLD] = GET_MAX_RESISTANCE(rItem[0]->m_Resistance[R_COLD], rItem[1]->m_Resistance[R_COLD], rItem[2]->m_Resistance[R_COLD]);
	lpObj->C_Resistance[R_POISON] = GET_MAX_RESISTANCE(rItem[0]->m_Resistance[R_POISON], rItem[1]->m_Resistance[R_POISON], rItem[2]->m_Resistance[R_POISON]);
	lpObj->C_Resistance[R_LIGHTNING] = GET_MAX_RESISTANCE(rItem[0]->m_Resistance[R_LIGHTNING], rItem[1]->m_Resistance[R_LIGHTNING], rItem[2]->m_Resistance[R_LIGHTNING]);
	lpObj->C_Resistance[R_FIRE] = GET_MAX_RESISTANCE(rItem[0]->m_Resistance[R_FIRE], rItem[1]->m_Resistance[R_FIRE], rItem[2]->m_Resistance[R_FIRE]);
	lpObj->C_Resistance[R_EARTH] = GET_MAX_RESISTANCE(rItem[0]->m_Resistance[R_EARTH], rItem[1]->m_Resistance[R_EARTH], rItem[2]->m_Resistance[R_EARTH]);
	lpObj->C_Resistance[R_WIND] = GET_MAX_RESISTANCE(rItem[0]->m_Resistance[R_WIND], rItem[1]->m_Resistance[R_WIND], rItem[2]->m_Resistance[R_WIND]);
	lpObj->C_Resistance[R_WATER] = GET_MAX_RESISTANCE(rItem[0]->m_Resistance[R_WATER], rItem[1]->m_Resistance[R_WATER], rItem[2]->m_Resistance[R_WATER]);

	if (lpObj->Class == CLASS_RAGEFIGHTER) {
		lpObj->C_ResistanceBonus[R_COLD] = (BYTE)lpObj->C_ResistanceBonus[R_COLD] + (BYTE)lpObj->CharacterData->Mastery.ML_RF_ColdAffinity;
		lpObj->C_ResistanceBonus[R_POISON] = (BYTE)lpObj->C_ResistanceBonus[R_POISON] + (BYTE)lpObj->CharacterData->Mastery.ML_RF_Immunity;
		lpObj->C_ResistanceBonus[R_LIGHTNING] = (BYTE)lpObj->C_ResistanceBonus[R_LIGHTNING] + (BYTE)lpObj->CharacterData->Mastery.ML_RF_Resistivity;
	} else {
		lpObj->C_ResistanceBonus[R_COLD] = (BYTE)lpObj->C_ResistanceBonus[R_COLD] + (BYTE)lpObj->CharacterData->Mastery.ML_GENERAL_ColdAffinity;
		lpObj->C_ResistanceBonus[R_POISON] = (BYTE)lpObj->C_ResistanceBonus[R_POISON] + (BYTE)lpObj->CharacterData->Mastery.ML_GENERAL_Immunity;
		lpObj->C_ResistanceBonus[R_LIGHTNING] = (BYTE)lpObj->C_ResistanceBonus[R_LIGHTNING] + (BYTE)lpObj->CharacterData->Mastery.ML_GENERAL_Resistivity;
	}

	lpObj->CharacterData->C_CriticalStrikeResistance = 0;
	lpObj->CharacterData->C_ExcellentStrikeResistance = 0;
	lpObj->CharacterData->C_LethalStrikeResistance = 0;
	lpObj->CharacterData->C_FatalStrikeResistance = 0;
	lpObj->CharacterData->C_ShieldPenetrationResistance = 0;
	lpObj->CharacterData->C_StunResistance = 0;

	if (BUFF_IsBuffEffectType(lpObj, BUFFTYPE_IMPROVED_LIGHTNING_REFLEXES, EFFECTTYPE_MASTERED_REFLEXES_RESIST)) lpObj->CharacterData->C_CriticalStrikeResistance += 1;
	else if (BUFF_IsBuffEffectType(lpObj, BUFFTYPE_IMPROVED_LIGHTNING_REFLEXES, EFFECTTYPE_MASTERED_REFLEXES_MAX)) lpObj->CharacterData->C_CriticalStrikeResistance += 1, lpObj->CharacterData->C_LethalStrikeResistance += 1;
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 302) == 20 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 580) == 20) lpObj->CharacterData->C_ShieldPenetrationResistance += 1;
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 309) == 20 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 587) == 20) lpObj->CharacterData->C_StunResistance += 1;
	if (BUFF_IsBuffEffectType(lpObj, BUFFTYPE_ENHANCE, EFFECTTYPE_INCREASE_STUN_RESISTANCE)) lpObj->CharacterData->C_StunResistance += 2;
	for (int i = 0; i < 3; i++) {
		switch (rItem[i]->m_Type) {
			case ITEMGET(13, 171):		// Pendant of Immortality
				for (int j = 0; j < 6; j++)	{
					if (rItem[i]->m_Type == ITEMGET(g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].ItemType, g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].ItemIndex)) {
						lpObj->CharacterData->C_FatalStrikeResistance += g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].LevelValue[rItem[i]->m_Level];

						if (rItem[i]->IsExtItem() == TRUE) lpObj->CharacterData->C_FatalStrikeResistance += 2;
					}
				}
			break;

			case ITEMGET(13, 172):		// Pendant of Resilience
				for (int j = 0; j < 6; j++)	{
					if (rItem[i]->m_Type == ITEMGET(g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].ItemType, g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].ItemIndex)) {
						lpObj->CharacterData->C_ShieldPenetrationResistance += g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].LevelValue[rItem[i]->m_Level];

						if (rItem[i]->IsExtItem() == TRUE) lpObj->CharacterData->C_ShieldPenetrationResistance += 2;
					}
				}
			break;

			case ITEMGET(13, 173):		// Ring of Deflection
				for (int j = 0; j < 6; j++) {
					if (rItem[i]->m_Type == ITEMGET(g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].ItemType, g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].ItemIndex)) {
						lpObj->CharacterData->C_LethalStrikeResistance += g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].LevelValue[rItem[i]->m_Level];

						if (rItem[i]->IsExtItem() == TRUE) lpObj->CharacterData->C_LethalStrikeResistance += 2;
					}
				}
			break;

			case ITEMGET(13, 174):		// Ring of Stability
				for (int j = 0; j < 6; j++)	{
					if (rItem[i]->m_Type == ITEMGET(g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].ItemType, g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].ItemIndex)) {
						lpObj->CharacterData->C_StunResistance += g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].LevelValue[rItem[i]->m_Level];

						if (rItem[i]->IsExtItem() == TRUE) {
							if (rItem[i]->m_Level) lpObj->CharacterData->C_StunResistance += 2;
							else lpObj->CharacterData->C_StunResistance++;
						}
					}
				}
			break;

			case ITEMGET(13, 175):		// Ring of Negation
				for (int j = 0; j < 6; j++) {
					if (rItem[i]->m_Type == ITEMGET(g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].ItemType, g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].ItemIndex)) {
						lpObj->CharacterData->C_ExcellentStrikeResistance += g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].LevelValue[rItem[i]->m_Level];

						if (rItem[i]->IsExtItem() == TRUE) lpObj->CharacterData->C_ExcellentStrikeResistance += 2;
					}
				}
			break;

			case ITEMGET(13, 176):		// Ring of Vigilance
				for (int j = 0; j < 6; j++)	{
					if (rItem[i]->m_Type == ITEMGET(g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].ItemType, g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].ItemIndex)) {
						lpObj->CharacterData->C_CriticalStrikeResistance += g_ItemOptionTypeMng.m_AccessoryItemOptionValue[j].LevelValue[rItem[i]->m_Level];

						if (rItem[i]->IsExtItem() == TRUE) lpObj->CharacterData->C_CriticalStrikeResistance += 2;
					}
				}
			break;
		}
	}

	g_kItemSystemFor380.ApplyFor380Option(lpObj);
	g_kJewelOfHarmonySystem.SetApplyStrengthenItem(lpObj);
	g_SocketOptionSystem.SetApplySocketEffect(lpObj);
	INVENTORY_PremiumEquipment(lpObj);

	if (lpObj->Class == CLASS_RAGEFIGHTER) {
		lpObj->AddLife += lpObj->CharacterData->Mastery.ML_RF_Vigor;
		lpObj->AddStamina += lpObj->CharacterData->Mastery.ML_RF_Vitality;
		lpObj->iAddShield += lpObj->CharacterData->Mastery.ML_RF_GladiatorSpirit;
		lpObj->AddMana += lpObj->CharacterData->Mastery.ML_RF_RisingSpirit;
	} else {
		lpObj->AddLife += lpObj->CharacterData->Mastery.ML_GENERAL_Vigor;
		lpObj->AddStamina += lpObj->CharacterData->Mastery.ML_GENERAL_Vitality;
		lpObj->iAddShield += lpObj->CharacterData->Mastery.ML_GENERAL_GladiatorSpirit;
		lpObj->AddMana += lpObj->CharacterData->Mastery.ML_GENERAL_RisingSpirit;
	}

	addManaPercent += lpObj->CharacterData->Mastery.iMpsMaxManaRate / 100.0;

	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_INNERVATION)) addStamPercent += lpObj->CharacterData->Mastery.ML_DK_Innervation / 100.0;
	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_IMPROVED_MANA_SHIELD)) addManaPercent += lpObj->CharacterData->Mastery.ML_DW_ImprovedManaShield / 100.0;
	if (BUFF_IsBuffEffectType(lpObj, BUFFTYPE_IMPROVED_TOUGHNESS, EFFECTTYPE_IMPROVED_TOUGHNESS)) DefensePercent += 0.05;
	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_LIGHTNING_REFLEXES) || BUFF_TargetHasBuff(lpObj, BUFFTYPE_IMPROVED_LIGHTNING_REFLEXES)) {
		int value = 0;

		BUFF_GetBuffEffectValue(lpObj, BUFFTYPE_LIGHTNING_REFLEXES, &value, NULL);
		BUFF_GetBuffEffectValue(lpObj, BUFFTYPE_IMPROVED_LIGHTNING_REFLEXES, &value, NULL);
		EvRatePercent += value / 100.00;
	}

	g_ItemOptionTypeMng.CalcExcOptionEffect(lpObj);
	g_ItemOptionTypeMng.CalcWingOptionEffect(lpObj);
	this->CHARACTER_SetItemApply(lpObj);
	this->PremiumItemApply(lpObj);
	CHARACTER_NextExpCal(lpObj);

	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_VOID_AFFINITY)) {
		lpObj->m_AttackDamageMinLeft += lpObj->CharacterData->Mastery.ML_SUM_VoidAffinity;
		lpObj->m_AttackDamageMaxLeft += lpObj->CharacterData->Mastery.ML_SUM_VoidAffinity;
		lpObj->m_AttackDamageMinRight += lpObj->CharacterData->Mastery.ML_SUM_VoidAffinity;
		lpObj->m_AttackDamageMaxRight += lpObj->CharacterData->Mastery.ML_SUM_VoidAffinity;
		lpObj->m_MagicDamageMin += lpObj->CharacterData->Mastery.ML_SUM_VoidAffinity;
		lpObj->m_MagicDamageMax += lpObj->CharacterData->Mastery.ML_SUM_VoidAffinity;
		lpObj->m_CurseDamageMin += lpObj->CharacterData->Mastery.ML_SUM_VoidAffinity;
		lpObj->m_CurseDamageMax += lpObj->CharacterData->Mastery.ML_SUM_VoidAffinity;
	}

	if ((Left->m_Type >= ITEMGET(4,0) && Left->m_Type < ITEMGET(4,7)) || Left->m_Type == ITEMGET(4,17) || Left->m_Type == ITEMGET(4,20) || Left->m_Type == ITEMGET(4,21) || Left->m_Type == ITEMGET(4,22) || Left->m_Type == ITEMGET(4,23) || Left->m_Type == ITEMGET(4,24) || Left->m_Type == ITEMGET(4,25) || Left->m_Type == ITEMGET(4,27)) {
		if (Right->m_Type == ITEMGET(4,15) && Right->m_Level == 1) {
			lpObj->m_AttackDamageMinLeft += (WORD)(lpObj->m_AttackDamageMinLeft * 0.03f + 1.0f);
			lpObj->m_AttackDamageMaxLeft += (WORD)(lpObj->m_AttackDamageMaxLeft * 0.03f + 1.0f);
		} else if (Right->m_Type == ITEMGET(4,15) && Right->m_Level == 2) {
			lpObj->m_AttackDamageMinLeft += (WORD)(lpObj->m_AttackDamageMinLeft * 0.05f + 1.0f);
			lpObj->m_AttackDamageMaxLeft += (WORD)(lpObj->m_AttackDamageMaxLeft * 0.05f + 1.0f);
		} else if (Right->m_Type == ITEMGET(4,15) && Right->m_Level == 3) {
			lpObj->m_AttackDamageMinLeft += (WORD)(lpObj->m_AttackDamageMinLeft * 0.07f + 1.0f);
			lpObj->m_AttackDamageMaxLeft += (WORD)(lpObj->m_AttackDamageMaxLeft * 0.07f + 1.0f);
		}
	} else if ((Right->m_Type >= ITEMGET(4,8) && Right->m_Type < ITEMGET(4,16)) || Right->m_Type == ITEMGET(4,18) || Right->m_Type == ITEMGET(4,19) || Right->m_Type == ITEMGET(4,26)) {
		if (Left->m_Type == ITEMGET(4,7) && Left->m_Level == 1)	{
			lpObj->m_AttackDamageMinRight += (WORD)(lpObj->m_AttackDamageMinRight * 0.03f + 1.0f);
			lpObj->m_AttackDamageMaxRight += (WORD)(lpObj->m_AttackDamageMaxRight * 0.03f + 1.0f);
		} else if (Left->m_Type == ITEMGET(4,7) && Left->m_Level == 2) {
			lpObj->m_AttackDamageMinRight += (WORD)(lpObj->m_AttackDamageMinRight * 0.05f + 1.0f);
			lpObj->m_AttackDamageMaxRight += (WORD)(lpObj->m_AttackDamageMaxRight * 0.05f + 1.0f);
		} else if (Left->m_Type == ITEMGET(4,7) && Left->m_Level == 3) {
			lpObj->m_AttackDamageMinRight += (WORD)(lpObj->m_AttackDamageMinRight * 0.07f + 1.0f);
			lpObj->m_AttackDamageMaxRight += (WORD)(lpObj->m_AttackDamageMaxRight * 0.07f + 1.0f);
		}
	}

	if (lpObj->Class == CLASS_KNIGHT || lpObj->Class == CLASS_MAGICGLADIATOR || lpObj->Class == CLASS_DARKLORD) {
		if (Right->m_Type != -1 && Left->m_Type != -1) {
			if (Right->m_Type >= ITEMGET(0,0) && Right->m_Type < ITEMGET(4,0) && Left->m_Type >= ITEMGET(0,0) && Left->m_Type < ITEMGET(4,0)) {
				if (Right->m_Type == Left->m_Type && (lpObj->Class == CLASS_KNIGHT || lpObj->Class == CLASS_MAGICGLADIATOR)) this->m_Lua.Generic_Call("CalcTwoSameWeaponBonus", "iiii>iiii", lpObj->m_AttackDamageMinLeft, lpObj->m_AttackDamageMaxLeft, lpObj->m_AttackDamageMinRight, lpObj->m_AttackDamageMaxRight, &lpObj->m_AttackDamageMinLeft, &lpObj->m_AttackDamageMinRight, &lpObj->m_AttackDamageMaxLeft, &lpObj->m_AttackDamageMaxRight);
				else this->m_Lua.Generic_Call("CalcTwoDifferentWeaponBonus", "iiii>iiii", lpObj->m_AttackDamageMinLeft, lpObj->m_AttackDamageMaxLeft, lpObj->m_AttackDamageMinRight, lpObj->m_AttackDamageMaxRight, &lpObj->m_AttackDamageMinLeft, &lpObj->m_AttackDamageMinRight, &lpObj->m_AttackDamageMaxLeft, &lpObj->m_AttackDamageMaxRight);
			}	
		}
	}

	if (lpObj->Class == CLASS_RAGEFIGHTER) {
		if (Right->m_Type != -1 && Left->m_Type != -1) {
			if (Right->m_Type >= ITEMGET(0,0) && Right->m_Type < ITEMGET(4,0) && Left->m_Type >= ITEMGET(0,0) && Left->m_Type < ITEMGET(4,0)) this->m_Lua.Generic_Call("CalcRageFighterTwoWeaponBonus", "iiii>iiii", lpObj->m_AttackDamageMinLeft, lpObj->m_AttackDamageMaxLeft, lpObj->m_AttackDamageMinRight, lpObj->m_AttackDamageMaxRight, &lpObj->m_AttackDamageMinLeft, &lpObj->m_AttackDamageMinRight, &lpObj->m_AttackDamageMaxLeft, &lpObj->m_AttackDamageMaxRight);
		}
	}

	if (COMPANION_Demon(lpObj)) {																																// Demon
		lpObj->AttackSpeed += g_ConfigRead.pet.DemonAddAttackSpeed;
		lpObj->MagicSpeed += g_ConfigRead.pet.DemonAddAttackSpeed;
	} else if (COMPANION_GuardianSpirit(lpObj)) {																												// Guardian Spirit
		lpObj->AddLife += g_ConfigRead.pet.SafeGuardAddHP;
	} else if (COMPANION_Satan(lpObj)) {																														// Satan
		lpObj->AttackSpeed += 10;
		lpObj->MagicSpeed += 10;
	} else if (lpObj->pInventory[8].IsItem() == TRUE && lpObj->pInventory[8].m_Type == ITEMGET(13, 80) && lpObj->pInventory[8].m_Durability > 0.0) {			// Panda
		lpObj->Defense += g_ConfigRead.pet.PandaPetAddDefense;
		lpObj->m_MagicDefense += g_ConfigRead.pet.PandaPetAddDefense;
	}

	g_BuffEffect.SetNextEffect(lpObj);
	this->CalcMLSkillItemOption(lpObj);
	this->CHARACTER_CalcShieldPoint(lpObj);

	if (lpObj->iShield > (lpObj->iMaxShield + lpObj->iAddShield)) {
		lpObj->iShield = lpObj->iMaxShield + lpObj->iAddShield;
		GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
	}

	int buffLifePercent = 0;
	int buffValue = 0;
	int atkSpeedInc = 100;
	int atkSpeedDec = 100;
	float AttackSpeedPercent = 0;

	if (BUFF_IsBuffEffectType(lpObj, BUFFTYPE_EMPOWER, EFFECTTYPE_INCREASE_ATTACK_SPEED_PERCENT)) atkSpeedInc += 10;
	if (BUFF_IsBuffEffectType(lpObj, BUFFTYPE_ETERNAL_ARROW, EFFECTTYPE_INCREASE_ATTACK_SPEED_PERCENT) || BUFF_IsBuffEffectType(lpObj, BUFFTYPE_INFINITY_ARROW, EFFECTTYPE_INCREASE_ATTACK_SPEED_PERCENT)) atkSpeedInc += 10;

	atkSpeedDec -= BUFF_GetTotalBuffEffectValue(lpObj, EFFECTTYPE_DECREASE_ATTACK_SPEED_PERCENT);
	BUFF_GetBuffEffectValue(lpObj, BUFFTYPE_ENHANCE, NULL, &buffValue);
	lpObj->EXC_ReduceDamageTaken += buffValue;
	BUFF_GetBuffEffectValue(lpObj, BUFFTYPE_IMPROVED_TOUGHNESS, NULL, &buffLifePercent);
	addLifePercent += buffLifePercent / 100.00;

	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 317) >= 10 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 595) >= 10) addManaPercent += 0.05;							// Intelligence
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 318) >= 10 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 596) >= 10) addLifePercent += 0.05;							// Resillience
	if (lpObj->Class == CLASS_WIZARD && g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 386) >= 10) addManaPercent += 0.10;																// Arcane Brilliance
	if (lpObj->Class == CLASS_SUMMONER && g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 457) >= 10) addManaPercent += 0.05;															// Spell Mastery

	lpObj->Defense *= DefensePercent;
	lpObj->EvasionRating *= EvRatePercent;
	AttackSpeedPercent = atkSpeedInc / 100.00;
	lpObj->AttackSpeed *= AttackSpeedPercent;
	lpObj->MagicSpeed *= AttackSpeedPercent;
	AttackSpeedPercent = atkSpeedDec / 100.00;
	lpObj->AttackSpeed *= AttackSpeedPercent;
	lpObj->MagicSpeed *= AttackSpeedPercent;
	lpObj->AddLife += (int)(lpObj->MaxLife * addLifePercent);
	lpObj->AddMana += (int)(lpObj->MaxMana * addManaPercent);
	lpObj->AddStamina += (int)(lpObj->MaxStamina * addStamPercent);

	GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->MaxLife + lpObj->AddLife, 0xFE, 0, lpObj->iMaxShield + lpObj->iAddShield);
	GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, lpObj->MaxMana + lpObj->AddMana, 0xFE, 0, lpObj->MaxStamina + lpObj->AddStamina);
	g_StatSpec.SendOptionList(lpObj);
	GSProtocol.PROTOCOL_AttackSpeedSend(lpObj->m_Index);
}
void CObjCalCharacter::CHARACTER_CalcSetItemStats(LPOBJ lpObj) {
	if (lpObj->Type != OBJ_USER) return;
	for (int i = 0;  i< INVETORY_WEAR_SIZE; i++) {
		if (lpObj->pInventory[i].IsSetItem() && lpObj->pInventory[i].m_IsValidItem && lpObj->pInventory[i].m_Durability != 0.0f) {
			int AddStatType = lpObj->pInventory[i].GetAddStatType();

			switch (AddStatType) {
				case 1:
					lpObj->pInventory[i].SetItemPlusSpecialStat(&lpObj->AddStrength, 196);
				break;

				case 2:
					EnterCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
					lpObj->CharacterData->AgilityCheckDelay = GetTickCount();
					lpObj->pInventory[i].SetItemPlusSpecialStat(&lpObj->AddDexterity, 197);
					LeaveCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
				break;

				case 3:
					lpObj->pInventory[i].SetItemPlusSpecialStat(&lpObj->AddEnergy, 198);
				break;

				case 4:
					lpObj->pInventory[i].SetItemPlusSpecialStat(&lpObj->AddVitality, 199);
				break;
			}
		}
	}
}
void CObjCalCharacter::GetSetItemOption(LPOBJ lpObj, LPBYTE pSetOptionTable, LPBYTE pSetOptionCountTable, int * pSetOptionCount) {
	*pSetOptionCount = 0;
	int RightSetOptionIndex = -1;
	int RightRingSetOptionIndex = -1;

	for (int i = 0; i < INVETORY_WEAR_SIZE; i++) {
		if (lpObj->pInventory[i].m_Durability <= 0.0) continue;

		int iSetItemType = lpObj->pInventory[i].IsSetItem();

		if (iSetItemType) {
			if (lpObj->pInventory[i].m_IsValidItem) {
				int iSetItemOption = gSetItemOption.GetOptionTableIndex(lpObj->pInventory[i].m_Type, iSetItemType);

				if (iSetItemOption != -1) {
					if (i == 0) RightSetOptionIndex = iSetItemOption;
					else if (i == 1) {
						if (RightSetOptionIndex == iSetItemOption)	{
							if (lpObj->pInventory[0].GetWeaponType() == lpObj->pInventory[1].GetWeaponType() && lpObj->pInventory[0].IsSetItem()) continue;
						}
					}
					
					if (i == 10) RightRingSetOptionIndex = iSetItemOption;
					else if (i == 11 ) {
						if (lpObj->pInventory[10].m_Type == lpObj->pInventory[11].m_Type && lpObj->pInventory[10].IsSetItem()) continue;
					}

					BOOL bFound = FALSE;

					for (int n = 0; n < *pSetOptionCount; n++) {
						if (pSetOptionTable[n] == iSetItemOption) {
							pSetOptionCountTable[n]++;
							bFound = TRUE;
							break;
						}
					}

					if (bFound == FALSE) {
						pSetOptionTable[*pSetOptionCount] = iSetItemOption;
						pSetOptionCountTable[*pSetOptionCount]++;
						*pSetOptionCount += 1;
					}
				}
			}
		}
	}
}
void CObjCalCharacter::CalcSetItemOption(LPOBJ lpObj) {
	BYTE SetOptionTable[29];
	BYTE SetOptionCountTable[29];
	int SetOptionCount = 0;
	int op1;
	int op2;
	int op3;
	int op4;
	int op5;
	int opvalue1;
	int opvalue2;
	int opvalue3;
	int opvalue4;
	int opvalue5;

	memset(SetOptionTable, 0, sizeof(SetOptionTable));
	memset(SetOptionCountTable, 0, sizeof(SetOptionCountTable));
	this->GetSetItemOption(lpObj, SetOptionTable, SetOptionCountTable, &SetOptionCount);

	for (int optioncount = 0; optioncount < SetOptionCount; optioncount++) {
		int OptionTableIndex = SetOptionTable[optioncount];
		int OptionTableCount = SetOptionCountTable[optioncount];

		if (OptionTableCount >= 2) {
			gSetItemOption.IsRequireClass(OptionTableIndex, lpObj->Class, lpObj->CharacterData->ChangeUP);

			for (int tablecnt = 0; tablecnt < (OptionTableCount - 1); tablecnt++) {
				if (gSetItemOption.GetSetOption(OptionTableIndex, tablecnt,	op1, op2, opvalue1, opvalue2, lpObj->Class, lpObj->CharacterData->ChangeUP)) {
					this->CHARACTER_SetItemIncreaseStats(lpObj, op1, opvalue1);
					this->CHARACTER_SetItemIncreaseStats(lpObj, op2, opvalue2);
				}
			}

			if (gSetItemOption.GetMaxSetOptionCount(OptionTableIndex) < OptionTableCount) {
				lpObj->CharacterData->IsFullSetItem = true;
				gSetItemOption.GetGetFullSetOption(OptionTableIndex, op1, op2, op3, op4, op5, opvalue1, opvalue2, opvalue3, opvalue4, opvalue5,	lpObj->Class, lpObj->CharacterData->ChangeUP);
				this->CHARACTER_SetItemIncreaseStats(lpObj, op1, opvalue1);
				this->CHARACTER_SetItemIncreaseStats(lpObj, op2, opvalue2);
				this->CHARACTER_SetItemIncreaseStats(lpObj, op3, opvalue3);
				this->CHARACTER_SetItemIncreaseStats(lpObj, op4, opvalue4);
				this->CHARACTER_SetItemIncreaseStats(lpObj, op5, opvalue5);
			}		
		}
	}

	for (int optioncount = 0; optioncount < SetOptionCount; optioncount++) {
		int OptionTableIndex = SetOptionTable[optioncount];
		int OptionTableCount = SetOptionCountTable[optioncount];

		if (OptionTableCount >= 2) {
			for (int tablecnt = 0; tablecnt < (OptionTableCount - 1); tablecnt++) {
				if (gSetItemOption.GetSetOption(OptionTableIndex, tablecnt, op1, op2, opvalue1, opvalue2, lpObj->Class, lpObj->CharacterData->ChangeUP))	{
					this->CHARACTER_SetItemBonus(lpObj, op1, opvalue1);
					this->CHARACTER_SetItemBonus(lpObj, op2, opvalue2);
				}
			}
				
			if (gSetItemOption.GetMaxSetOptionCount(OptionTableIndex) < OptionTableCount) {
				lpObj->CharacterData->IsFullSetItem = true;
				gSetItemOption.GetGetFullSetOption(OptionTableIndex, op1, op2, op3, op4, op5, opvalue1, opvalue2, opvalue3, opvalue4, opvalue5,	lpObj->Class, lpObj->CharacterData->ChangeUP);
				this->CHARACTER_SetItemBonus(lpObj, op1, opvalue1);
				this->CHARACTER_SetItemBonus(lpObj, op2, opvalue2);
				this->CHARACTER_SetItemBonus(lpObj, op3, opvalue3);
				this->CHARACTER_SetItemBonus(lpObj, op4, opvalue4);
				this->CHARACTER_SetItemBonus(lpObj, op5, opvalue5);
			}		
		}
	}
}
void CObjCalCharacter::CHARACTER_SetItemIncreaseStats(LPOBJ lpObj, int option, int ivalue) {
	if (lpObj->Type != OBJ_USER) return;
	if (option == -1) return;

	switch (option)	{
		case ANCIENT_OPTION_STRENGTH:				lpObj->AddStrength += ivalue;						break;	// 0
		case ANCIENT_OPTION_AGILITY:																			// 1
			EnterCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
			lpObj->CharacterData->AgilityCheckDelay = GetTickCount();
			lpObj->AddDexterity += ivalue;
			LeaveCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
		break;

		case ANCIENT_OPTION_ENERGY:					lpObj->AddEnergy += ivalue;							break;	// 2
		case ANCIENT_OPTION_VITALITY:				lpObj->AddVitality += ivalue;						break;	// 3
		case ANCIENT_OPTION_COMMAND:				lpObj->AddLeadership += ivalue;						break;	// 4
	}
}
void CObjCalCharacter::CHARACTER_SetItemBonus(LPOBJ lpObj, int option, int ivalue) {
	if (option == -1) return;
	if (ivalue == 0) {
		g_Log.Add("Error iValue == 0, option %d",option);
		return;
	}

	switch (option) {
		case ANCIENT_OPTION_MINIMUM_PHYSICAL_DAMAGE:									lpObj->CharacterData->ANCIENT_MinPhysicalDamage += ivalue;																					break;	// 5
		case ANCIENT_OPTION_MAXIMUM_PHYSICAL_DAMAGE:									lpObj->CharacterData->ANCIENT_MaxPhysicalDamage += ivalue;																					break;	// 6
		case ANCIENT_OPTION_SPELL_DAMAGE:												lpObj->CharacterData->ANCIENT_SpellDamage += ivalue;																						break;	// 7
		case ANCIENT_OPTION_PHYSICAL_DAMAGE:											lpObj->CharacterData->ANCIENT_PhysicalDamage += ivalue;																					break;	// 8
		case ANCIENT_OPTION_HIT_RATING:													lpObj->CharacterData->ANCIENT_HitRating += ivalue;																							break;	// 9
		case ANCIENT_OPTION_DEFENSE:													lpObj->CharacterData->ANCIENT_Defense += ivalue;																							break;	// 10
		case ANCIENT_OPTION_MAXIMUM_LIFE:												lpObj->AddLife += ivalue;																												break;	// 11
		case ANCIENT_OPTION_MAXIMUM_MANA:												lpObj->AddMana += ivalue;																												break;	// 12
		case ANCIENT_OPTION_MAXIMUM_STAMINA:											lpObj->AddStamina += ivalue;																													break;	// 13
		case ANCIENT_OPTION_STAMINA_REGENERATION:										lpObj->CharacterData->ANCIENT_StaminaRegeneration += ivalue;																				break;	// 14
		case ANCIENT_OPTION_CRITICAL_STRIKE:											lpObj->CharacterData->ANCIENT_CriticalStrikeChance += ivalue;																				break;	// 15
		case ANCIENT_OPTION_CRITICAL_DAMAGE:											lpObj->CharacterData->ANCIENT_CriticalDamage += ivalue;																					break;	// 16
		case ANCIENT_OPTION_EXCELLENT_STRIKE:											lpObj->CharacterData->ANCIENT_ExcellentStrikeChance += ivalue;																				break;	// 17
		case ANCIENT_OPTION_EXCELLENT_DAMAGE:											lpObj->CharacterData->ANCIENT_ExcellentDamage += ivalue;																					break;	// 18
		case ANCIENT_SKILL_DAMAGE:														lpObj->CharacterData->ANCIENT_SkillDamage += ivalue;																						break;	// 19
		case ANCIENT_OPTION_LETHAL_STRIKE:												lpObj->CharacterData->ANCIENT_LethalStrikeChance += ivalue;																				break;	// 20
		case ANCIENT_OPTION_FATAL_STRIKE:												lpObj->CharacterData->ANCIENT_FatalStrikeChance += ivalue;																					break;	// 21
		case ANCIENT_OPTION_DEFENSE_WITH_SHIELDS:										lpObj->CharacterData->ANCIENT_DefenseWithShields += ivalue;																				break;	// 22
		case ANCIENT_OPTION_TWO_HANDED_WEAPON_DAMAGE:									lpObj->CharacterData->ANCIENT_TwoHandedWeaponDamage += ivalue;																				break;	// 23
		case ANCIENT_OPTION_STUN_DURATION:												lpObj->CharacterData->ANCIENT_StunDuration += ivalue;																						break;	// 24
		case ANCIENT_OPTION_BURN_DAMAGE:												lpObj->CharacterData->ANCIENT_BurnDamage += ivalue;																						break;	// 25
		case ANCIENT_OPTION_BURN_CHANCE:												lpObj->CharacterData->ANCIENT_BurnChance += ivalue;																						break;	// 26
		case ANCIENT_OPTION_DUAL_WIELD_DAMAGE:											lpObj->CharacterData->ANCIENT_DualWieldDamage += ivalue;																					break;	// 27
		case ANCIENT_OPTION_ITEM_DROP_RATE:												lpObj->CharacterData->ANCIENT_ItemDropRate += ivalue;																						break;	// 28
		case ANCIENT_OPTION_ICE_ATTRIBUTE_DAMAGE:										lpObj->CharacterData->ANCIENT_AttributeDamage[R_COLD] += ivalue;																			break;	// 29
		case ANCIENT_OPTION_POISON_ATTRIBUTE_DAMAGE:									lpObj->CharacterData->ANCIENT_AttributeDamage[R_POISON] += ivalue;																			break;	// 30
		case ANCIENT_OPTION_LIGHTNING_ATTRIBUTE_DAMAGE:									lpObj->CharacterData->ANCIENT_AttributeDamage[R_LIGHTNING] += ivalue;																		break;	// 31
		case ANCIENT_OPTION_FIRE_ATTRIBUTE_DAMAGE:										lpObj->CharacterData->ANCIENT_AttributeDamage[R_FIRE] += ivalue;																			break;	// 32
		case ANCIENT_OPTION_EARTH_ATTRIBUTE_DAMAGE:										lpObj->CharacterData->ANCIENT_AttributeDamage[R_EARTH] += ivalue;																			break;	// 33
		case ANCIENT_OPTION_WIND_ATTRIBUTE_DAMAGE:										lpObj->CharacterData->ANCIENT_AttributeDamage[R_WIND] += ivalue;																			break;	// 34
		case ANCIENT_OPTION_WATER_ATTRIBUTE_DAMAGE:										lpObj->CharacterData->ANCIENT_AttributeDamage[R_WATER] += ivalue;																			break;	// 35
		case ANCIENT_OPTION_ELEMENTAL_AILMENTS_DURATION:								lpObj->CharacterData->ANCIENT_ElementalAilmentsDuration += ivalue;																			break;	// 36
	}
}
void CObjCalCharacter::CHARACTER_SetItemApply(LPOBJ lpObj) {
	lpObj->m_AttackDamageMinLeft += lpObj->CharacterData->ANCIENT_MinPhysicalDamage + lpObj->CharacterData->ANCIENT_PhysicalDamage;
	lpObj->m_AttackDamageMinRight += lpObj->CharacterData->ANCIENT_MinPhysicalDamage + lpObj->CharacterData->ANCIENT_PhysicalDamage;
	lpObj->m_AttackDamageMaxLeft += lpObj->CharacterData->ANCIENT_MaxPhysicalDamage + lpObj->CharacterData->ANCIENT_PhysicalDamage;
	lpObj->m_AttackDamageMaxRight += lpObj->CharacterData->ANCIENT_MaxPhysicalDamage + lpObj->CharacterData->ANCIENT_PhysicalDamage;
	lpObj->m_MagicDamageMin += lpObj->m_MagicDamageMin * lpObj->CharacterData->ANCIENT_SpellDamage / 100;
	lpObj->m_MagicDamageMax += lpObj->m_MagicDamageMax * lpObj->CharacterData->ANCIENT_SpellDamage / 100;
	lpObj->AddLife += INT(lpObj->AddVitality * DCInfo.DefClass[lpObj->Class].VitalityToLife);
	lpObj->AddMana += INT(lpObj->AddEnergy * DCInfo.DefClass[lpObj->Class].EnergyToMana);
	lpObj->C_CriticalStrikeChance += lpObj->CharacterData->ANCIENT_CriticalStrikeChance;
	lpObj->C_ExcellentStrikeChance += lpObj->CharacterData->ANCIENT_ExcellentStrikeChance;

	if (lpObj->pInventory[10].IsSetItem()) lpObj->pInventory[10].PlusSpecialSetRing((LPBYTE)lpObj->C_ResistanceBonus);
	if (lpObj->pInventory[11].IsSetItem()) lpObj->pInventory[11].PlusSpecialSetRing((LPBYTE)lpObj->C_ResistanceBonus);
	if (lpObj->pInventory[9].IsSetItem()) lpObj->pInventory[9].PlusSpecialSetRing((LPBYTE)lpObj->C_ResistanceBonus);
	if (lpObj->CharacterData->ANCIENT_TwoHandedWeaponDamage) {
		if (!lpObj->pInventory[1].IsItem() && !lpObj->pInventory[0].IsItem()) lpObj->CharacterData->ANCIENT_TwoHandedWeaponDamage = 0;
		if (lpObj->pInventory[1].IsItem() && lpObj->pInventory[1].m_TwoHand == FALSE) lpObj->CharacterData->ANCIENT_TwoHandedWeaponDamage = 0;
		if (lpObj->pInventory[0].IsItem() && lpObj->pInventory[0].m_TwoHand == FALSE) lpObj->CharacterData->ANCIENT_TwoHandedWeaponDamage = 0;
	}

	if ((lpObj->MaxLife + lpObj->AddLife) < lpObj->Life) {
		lpObj->Life = lpObj->MaxLife + lpObj->AddLife;
		GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
	}

	PLAYER_CalcStamina(lpObj->m_Index);

	if ((lpObj->MaxMana + lpObj->AddMana) < lpObj->Mana) {
		lpObj->Mana = lpObj->MaxMana + lpObj->AddMana;
		GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, lpObj->Mana, 0xFF, 0, lpObj->Stamina);
	}

	lpObj->Defense += lpObj->CharacterData->ANCIENT_Defense;

	if (lpObj->pInventory[1].m_Type >= ITEMGET(6,0) && lpObj->pInventory[1].m_Type < ITEMGET(7,0)) lpObj->Defense += lpObj->Defense * lpObj->CharacterData->ANCIENT_DefenseWithShields / 100;
}
void CObjCalCharacter::CalcMLSkillItemOption(LPOBJ lpObj) {
	if (lpObj->Type != OBJ_USER) return;

	if (lpObj->CharacterData->Mastery.ML_DK_TwoHandedSwordMastery > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_TWOHAND_SWORD && lpObj->pInventory[1].GetDetailItemType() != ITEM_TWOHAND_SWORD) {
			lpObj->CharacterData->Mastery.ML_DK_TwoHandedSwordMastery = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DK_TwoHandedSwordExpertise > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_TWOHAND_SWORD && lpObj->pInventory[1].GetDetailItemType() != ITEM_TWOHAND_SWORD) {
			lpObj->CharacterData->Mastery.ML_DK_TwoHandedSwordExpertise = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DW_TwoHandedStaffMastery > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() == ITEM_TWOHAND_STAFF || lpObj->pInventory[1].GetDetailItemType() == ITEM_TWOHAND_STAFF) {
			lpObj->m_MagicDamageMin += lpObj->CharacterData->Mastery.ML_DW_TwoHandedStaffMastery;
			lpObj->m_MagicDamageMax += lpObj->CharacterData->Mastery.ML_DW_TwoHandedStaffMastery;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DW_OneHandedStaffMastery > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() == ITEM_ONEHAND_STAFF || lpObj->pInventory[1].GetDetailItemType() == ITEM_ONEHAND_STAFF) {
			lpObj->m_MagicDamageMin += lpObj->CharacterData->Mastery.ML_DW_OneHandedStaffMastery;
			lpObj->m_MagicDamageMax += lpObj->CharacterData->Mastery.ML_DW_OneHandedStaffMastery;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_SUM_StaffMastery > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() == ITEM_SUMMONER_STAFF || lpObj->pInventory[1].GetDetailItemType() == ITEM_SUMMONER_STAFF) {
			lpObj->m_MagicDamageMin += lpObj->CharacterData->Mastery.ML_SUM_StaffMastery;
			lpObj->m_MagicDamageMax += lpObj->CharacterData->Mastery.ML_SUM_StaffMastery;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_GENERAL_ShieldMastery > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() == ITEM_SHIELD || lpObj->pInventory[1].GetDetailItemType() == ITEM_SHIELD) {
			lpObj->Defense += lpObj->CharacterData->Mastery.ML_GENERAL_ShieldMastery;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_ELF_ShieldMastery > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() == ITEM_SHIELD || lpObj->pInventory[1].GetDetailItemType() == ITEM_SHIELD)	{
			lpObj->Defense += lpObj->CharacterData->Mastery.ML_ELF_ShieldMastery;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DL_ShieldMastery > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() == ITEM_SHIELD || lpObj->pInventory[1].GetDetailItemType() == ITEM_SHIELD)	{
			lpObj->Defense += lpObj->CharacterData->Mastery.ML_DL_ShieldMastery;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DW_OneHandedStaffExpertise > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() == ITEM_ONEHAND_STAFF || lpObj->pInventory[1].GetDetailItemType() == ITEM_ONEHAND_STAFF) {
			lpObj->AttackSpeed += lpObj->CharacterData->Mastery.ML_DW_OneHandedStaffExpertise;
			lpObj->MagicSpeed += lpObj->CharacterData->Mastery.ML_DW_OneHandedStaffExpertise;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_SUM_CurseExpertise > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() == ITEM_SUMMONER_BOOK || lpObj->pInventory[1].GetDetailItemType() == ITEM_SUMMONER_BOOK) {
			lpObj->AttackSpeed += lpObj->CharacterData->Mastery.ML_SUM_CurseExpertise;
			lpObj->MagicSpeed += lpObj->CharacterData->Mastery.ML_SUM_CurseExpertise;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_SUM_CurseMastery > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() == ITEM_SUMMONER_BOOK || lpObj->pInventory[1].GetDetailItemType() == ITEM_SUMMONER_BOOK) {
			lpObj->m_CurseDamageMin += lpObj->CharacterData->Mastery.ML_SUM_CurseMastery;
			lpObj->m_CurseDamageMax += lpObj->CharacterData->Mastery.ML_SUM_CurseMastery;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DW_TwoHandedStaffExpertise > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_TWOHAND_STAFF && lpObj->pInventory[1].GetDetailItemType() != ITEM_TWOHAND_STAFF) {
			lpObj->CharacterData->Mastery.ML_DW_TwoHandedStaffExpertise = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_ELF_CrossbowExpertise > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_CROSSBOW && lpObj->pInventory[1].GetDetailItemType() != ITEM_CROSSBOW)	{
			lpObj->CharacterData->Mastery.ML_ELF_CrossbowExpertise = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_SUM_StaffExpertise > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_SUMMONER_STAFF && lpObj->pInventory[1].GetDetailItemType() != ITEM_SUMMONER_STAFF)	{
			lpObj->CharacterData->Mastery.ML_SUM_StaffExpertise = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DL_ScepterExpertise > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_DARKLORD_SCEPTER && lpObj->pInventory[1].GetDetailItemType() != ITEM_DARKLORD_SCEPTER) {
			lpObj->CharacterData->Mastery.ML_DL_ScepterExpertise = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_GENERAL_ShieldExpertise > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_SHIELD && lpObj->pInventory[1].GetDetailItemType() != ITEM_SHIELD) {
			lpObj->CharacterData->Mastery.ML_GENERAL_ShieldExpertise = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_ELF_ShieldExpertise > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_SHIELD && lpObj->pInventory[1].GetDetailItemType() != ITEM_SHIELD)	{
			lpObj->CharacterData->Mastery.ML_ELF_ShieldExpertise = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DL_ShieldExpertise > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_SHIELD && lpObj->pInventory[1].GetDetailItemType() != ITEM_SHIELD)	{
			lpObj->CharacterData->Mastery.ML_DL_ShieldExpertise = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DK_OneHandedSwordExpertise > 0.0)	{
		if (lpObj->pInventory[0].GetDetailItemType() == ITEM_ONEHAND_SWORD)	{
			lpObj->AttackSpeed += lpObj->CharacterData->Mastery.ML_DK_OneHandedSwordExpertise;
			lpObj->MagicSpeed += lpObj->CharacterData->Mastery.ML_DK_OneHandedSwordExpertise;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DK_OneHandedSwordMastery > 0.0)	{
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_ONEHAND_SWORD && lpObj->pInventory[1].GetDetailItemType() != ITEM_ONEHAND_SWORD) {
			lpObj->CharacterData->Mastery.ML_DK_OneHandedSwordMastery = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DK_MaceMastery > 0.0)	{
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_MACE && lpObj->pInventory[1].GetDetailItemType() != ITEM_MACE)	{
			lpObj->CharacterData->Mastery.ML_DK_MaceMastery = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DK_MaceExpertise > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_MACE && lpObj->pInventory[1].GetDetailItemType() != ITEM_MACE) {
			lpObj->CharacterData->Mastery.ML_DK_MaceExpertise = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DK_SpearMastery > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_SPEAR && lpObj->pInventory[1].GetDetailItemType() != ITEM_SPEAR) {
			lpObj->CharacterData->Mastery.ML_DK_SpearMastery = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DK_SpearExpertise > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_SPEAR && lpObj->pInventory[1].GetDetailItemType() != ITEM_SPEAR) {
			lpObj->CharacterData->Mastery.ML_DK_SpearExpertise = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_ELF_BowMastery > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_BOW && lpObj->pInventory[1].GetDetailItemType() != ITEM_BOW) {
			lpObj->CharacterData->Mastery.ML_ELF_BowMastery = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DL_ScepterMastery > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_DARKLORD_SCEPTER && lpObj->pInventory[1].GetDetailItemType() != ITEM_DARKLORD_SCEPTER)	{
			lpObj->CharacterData->Mastery.ML_DL_ScepterMastery = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DL_Commander > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_DARKLORD_SCEPTER && lpObj->pInventory[1].GetDetailItemType() != ITEM_DARKLORD_SCEPTER)	{
			lpObj->CharacterData->Mastery.ML_DL_Commander = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_DL_DelusionalGrandeur > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_DARKLORD_SCEPTER && lpObj->pInventory[1].GetDetailItemType() != ITEM_DARKLORD_SCEPTER)	{
			lpObj->CharacterData->Mastery.ML_DL_DelusionalGrandeur = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_ELF_BowExpertise > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() == ITEM_BOW || lpObj->pInventory[1].GetDetailItemType() == ITEM_BOW) {
			lpObj->AttackSpeed += lpObj->CharacterData->Mastery.ML_ELF_BowExpertise;
			lpObj->MagicSpeed += lpObj->CharacterData->Mastery.ML_ELF_BowExpertise;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_ELF_CrossbowMastery > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_CROSSBOW && lpObj->pInventory[1].GetDetailItemType() != ITEM_CROSSBOW) {
			lpObj->CharacterData->Mastery.ML_ELF_CrossbowMastery = 0.0;
		}
	}

	if (lpObj->CharacterData->Mastery.iMpsIncDarkHorseDefense > 0.0 && lpObj->pInventory[lpObj->m_btInvenPetPos].m_Type == ITEMGET(13, 4)) {
		lpObj->Defense += lpObj->CharacterData->Mastery.iMpsIncDarkHorseDefense;
	}

	if (lpObj->CharacterData->Mastery.ML_RF_FistMastery > 0.0) {
		if (lpObj->pInventory[0].GetDetailItemType() != ITEM_RAGEFIGHTER_WEAPON && lpObj->pInventory[1].GetDetailItemType() != ITEM_RAGEFIGHTER_WEAPON)	{
			lpObj->CharacterData->Mastery.ML_RF_FistMastery = 0.0;
		}
	}

	if (lpObj->pInventory[7].IsItem() == TRUE && lpObj->pInventory[7].m_IsValidItem == true) {
		switch (lpObj->Class) {
			case CLASS_KNIGHT:
			case CLASS_ELF:
			case CLASS_DARKLORD:
			case CLASS_RAGEFIGHTER:
				lpObj->m_AttackDamageMinRight += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage;
				lpObj->m_AttackDamageMaxRight += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage;
				lpObj->m_AttackDamageMinLeft += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage;
				lpObj->m_AttackDamageMaxLeft += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage;
			break;

			case CLASS_WIZARD:
				lpObj->m_MagicDamageMin += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage;
				lpObj->m_MagicDamageMax += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage;
			break;

			case CLASS_SUMMONER:
				lpObj->m_MagicDamageMin += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage;
				lpObj->m_MagicDamageMax += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage;
				lpObj->m_CurseDamageMin += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage;
				lpObj->m_CurseDamageMax += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage;
			break;

			case CLASS_MAGICGLADIATOR:
				lpObj->m_AttackDamageMinRight += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage;
				lpObj->m_AttackDamageMaxRight += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage;
				lpObj->m_AttackDamageMinLeft += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage;
				lpObj->m_AttackDamageMaxLeft += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage;
				lpObj->m_MagicDamageMin += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage;
				lpObj->m_MagicDamageMax += lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage;
			break;
		}
	}

	if (lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage > 0.0) {
		if (lpObj->pInventory[7].IsItem() == FALSE || lpObj->pInventory[7].m_IsValidItem == false) lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage = 0.0;
	}
}
BOOL CObjCalCharacter::ValidItem(LPOBJ lpObj, CItem * lpItem, int pos) {
	if (g_ConfigRead.data.common.CheckValidItem == false) return TRUE;

	LPITEM_ATTRIBUTE p = &ItemAttribute[lpItem->m_Type];
	
	if (p->RequireStrength != 0) {
		if ((lpObj->CharacterData->Strength + lpObj->AddStrength) < (lpItem->m_RequireStrength - lpItem->m_HJOpStrength)) return FALSE;
	}

	if (p->RequireDexterity != 0) {
		if ((lpObj->CharacterData->Dexterity + lpObj->AddDexterity) < (lpItem->m_RequireDexterity - lpItem->m_HJOpDexterity)) return FALSE;
	}

	if (p->RequireEnergy != 0) {
		if ((lpObj->CharacterData->Energy + lpObj->AddEnergy) < lpItem->m_RequireEnergy) return FALSE;
	}

	if (p->RequireLevel != 0) {
		if ((lpObj->Level) < lpItem->m_RequireLevel) return FALSE;
	}

	if (lpItem->m_Type >= ITEMGET(0,0) && lpItem->m_Type <= ITEMGET(11, 0)) {
		if (p->RequireVitality)	{
			if ((lpObj->CharacterData->Vitality + lpObj->AddVitality) < lpItem->m_RequireVitality) return FALSE;
		}

		if (p->RequireLeadership) {
			if ((lpObj->Leadership + lpObj->AddLeadership) < lpItem->m_RequireLeaderShip) return FALSE;
		}
	}

	return TRUE;
}
void CObjCalCharacter::CHARACTER_CalcShieldPoint(LPOBJ lpObj) {
	int iMaxShieldPoint = 0;
	int iExpressionA = (lpObj->CharacterData->Strength + lpObj->AddStrength) + (lpObj->CharacterData->Dexterity + lpObj->AddDexterity) + (lpObj->CharacterData->Vitality + lpObj->AddVitality) + (lpObj->CharacterData->Energy + lpObj->AddEnergy);

	if (lpObj->Class == CLASS_DARKLORD) iExpressionA += lpObj->Leadership + lpObj->AddLeadership;
	if (g_iShieldGageConstB == 0) g_iShieldGageConstB = 30;

	int Level = lpObj->Level + lpObj->CharacterData->MasterLevel;
	int iExpressionB = (Level * Level) / g_iShieldGageConstB;

	if (g_iShieldGageConstA == 0) g_iShieldGageConstA = 12;

	iMaxShieldPoint = (iExpressionA * g_iShieldGageConstA) / 10 + iExpressionB  + lpObj->Defense;
	lpObj->iMaxShield = iMaxShieldPoint;
}
void CObjCalCharacter::PremiumItemApply(LPOBJ lpObj) {
	for (int i = 9; i < 12; ++i) {
		if (lpObj->pInventory[i].m_Durability <= 0.0 || lpObj->pInventory[i].IsPeriodItemExpire() == TRUE) continue;
		switch (lpObj->pInventory[i].m_Type) {
			case ITEMGET(13,109):
				lpObj->LifeRegeneration += 3;
				lpObj->AddMana += (short)(lpObj->MaxMana * 4 / 100.0f);
			break;

			case ITEMGET(13,110):
				lpObj->LifeRegeneration += 3;
				lpObj->AddLife += (short)(lpObj->MaxLife * 4 / 100.0f);
			break;

			case ITEMGET(13,111):
				lpObj->LifeRegeneration += 3;
				lpObj->MonsterDieGetMoney += 30;
			break;

			case ITEMGET(13,112):
				lpObj->LifeRegeneration += 3;
				lpObj->EXC_ReduceDamageTaken += 4;
			break;

			case ITEMGET(13,113):
				lpObj->LifeRegeneration += 3;
				lpObj->C_ExcellentStrikeChance += 10;
			break;

			case ITEMGET(13,114):
				lpObj->LifeRegeneration += 3;
				lpObj->AttackSpeed += 7;
				lpObj->MagicSpeed += 7;
			break;

			case ITEMGET(13,115):
				lpObj->LifeRegeneration += 3;
				lpObj->MonsterDieGetMana += 1;
			break;

			case ITEMGET(13,128):
				lpObj->C_CriticalStrikeChance += 10;
			break;

			case ITEMGET(13,129):
				lpObj->C_ExcellentStrikeChance += 10;
			break;

			case ITEMGET(13,130):
				lpObj->AddLife += 50;
			break;

			case ITEMGET(13,131):
				lpObj->AddMana += 50;
			break;

			case ITEMGET(13,132):
				lpObj->AddLife += 100;
				lpObj->iAddShield += 500;
			break;

			case ITEMGET(13,133):
				lpObj->AddMana += 100;
				lpObj->AddStamina += 500;
			break;

			case ITEMGET(13,134):
			break;
		}
	}
}