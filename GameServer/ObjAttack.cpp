#include "stdafx.h"
#include "ObjAttack.h"
#include "BattleSoccerManager.h"
#include "DarkSpirit.h"
#include "ObjUseSkill.h"
#include "gObjMonster.h"
#include "BloodCastle.h"
#include "ChaosCastle.h"
#include "Gamemain.h"
#include "BuffEffect.h"
#include "BuffEffectSlot.h"
#include "PentagramSystem.h"
#include "configread.h"
#include "GensSystem.h"
#include "NewPVP.h"
#include "ImperialGuardian.h"
#include "MasterLevelSkillTreeSystem.h"
#include "LuckyItemManager.h"
#include "Crywolf.h"
#include "LastManStanding.h"
#include "MuunSystem.h"
#include "AcheronGuardianEvent.h"
#include "ObjCalCharacter.h"
#include "OfflineLevelling.h"
#include "SkillSpecialize.h"
#include "GameSecurity.h"
#include "MineSystem.h"
#include "SkillSafeZoneUse.h"

CObjAttack gclassObjAttack;

// Initialization
CObjAttack::CObjAttack() : m_Lua(true) {
	return;
}
CObjAttack::~CObjAttack() {
	return;
}
void CObjAttack::Init() {
	this->m_Lua.DoFile(g_ConfigRead.GetPath("\\Scripts\\Character\\CalcCharacter.lua"));
}

// Attack Action
BOOL CObjAttack::ATTACK_Normal(LPOBJ lpObj, LPOBJ lpTargetObj, CMagicInf* lpMagic, int magicsend, unsigned char MSBFlag, int AttackDamage, BOOL bCombo, BYTE RFAttack, BYTE byReflect, BYTE byPentagramAttack) {
	/*if (lpObj->Type == OBJ_USER) {
		gGameSecurity.DebugInfo(lpObj->m_Index);
	}*/
	float manaBurn = 0;
	int lethalBonus = 0;
	int skillSuccess = 0;
	LPOBJ lpCallObj;
	LPOBJ lpCallTargetObj;
	int MsgDamage = 0;
	int ManaChange = 0;
	int iTempShieldDamage = 0;
	int iTotalShieldDamage = 0;
	int LifeChange = 0;
	int iOption = 0;
	int comboSkill = FALSE;
	BOOL bDragonKickSDAttackSuccess = FALSE;
	BOOL bCommonAttackMiss = FALSE;
	BOOL bHaveWingOption_FullMana = FALSE;
	BOOL bHaveWingOption_FullHP = FALSE;

	if ((lpObj->Authority & 2) == 2 || (lpTargetObj->Authority & 2) == 2) return FALSE;
	if ((lpObj->Authority & 32) == 32 || (lpTargetObj->Authority & 32) == 32) {
		if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_INVISABLE) == TRUE) return FALSE;
		if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_INVISABLE) == TRUE) return FALSE;
	}

	if (lpTargetObj->m_bOffLevel == true && g_OffLevel.m_General.Immortal == 1) return FALSE;
	if (lpTargetObj->Type == OBJ_USER && lpTargetObj->CharacterData->ISBOT == true) return FALSE;
	if (lpObj->MapNumber != lpTargetObj->MapNumber) return FALSE;
	if (g_MineSystem.IsTwinkle(lpTargetObj->Class)) {
		g_Log.AddC(TColor::Red, "[Anti-Hack] [%d][%s][%s] Attempt to attack Pandora Twinkle", lpObj->m_Index, lpObj->AccountID, lpObj->Name);
		AntiHackLog->Output("[Anti-Hack] [%d][%s][%s] Attempt to attack Pandora Twinkle", lpObj->m_Index, lpObj->AccountID, lpObj->Name);
		return FALSE;
	}

	if (lpTargetObj->Class == 104 || lpTargetObj->Class == 105 || lpTargetObj->Class == 106 || lpTargetObj->Class == 523 || lpTargetObj->Class == 689) return FALSE;
	if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE) {
		if (g_Crywolf.GetCrywolfState() == 3 || g_Crywolf.GetCrywolfState() == 5) {
			if (CRYWOLF_MAP_RANGE(lpTargetObj->MapNumber)) {
				if (lpTargetObj->Type == OBJ_MONSTER) return FALSE;
			}
		}
	}

	if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) {
		if (lpObj->MapNumber == g_LastManStanding.m_Cfg.iPVPMap) {
			if (g_LastManStanding.m_Rooms[0].bState == 2) { // state same for all rooms
				if (lpObj->CharacterData->RegisterdLMS == 1 && lpTargetObj->CharacterData->RegisterdLMS == 1) return FALSE;
			}
		}
	}

	if (lpObj->Type == OBJ_USER && !byReflect && g_ConfigRead.antihack.EnabledAntiRefTimeCheck) {
		if (GetTickCount() - lpObj->CharacterData->m_dwMSBFlagAttackDisable < g_ConfigRead.antihack.AntiRefCheckTime) return FALSE;
	}

	if (lpTargetObj->Type == OBJ_USER && lpObj->Type == OBJ_USER) {
		if (g_GensSystem.IsPkEnable(lpObj, lpTargetObj) == FALSE) return FALSE;
		if (lpObj->PartyNumber != -1 && lpTargetObj->PartyNumber != -1) {
			if (lpTargetObj->PartyNumber == lpObj->PartyNumber) return FALSE;
		}
	}

	if (g_ArcaBattle.IsArcaBattleServer() == TRUE && g_AcheronGuardianEvent.IsPlayStart() == false) {
		if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER && g_ArcaBattle.IsPkEnable(lpObj, lpTargetObj) == FALSE) return FALSE;
		if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_MONSTER && g_ArcaBattle.IsEnableAttackObelisk(lpObj, lpTargetObj->Class) == FALSE) return FALSE;
	}

	if (lpTargetObj->Type == OBJ_USER && g_NewPVP.IsDuel(*lpTargetObj) && g_NewPVP.IsSafeState(*lpTargetObj)) return FALSE;
	if (lpObj->Type == OBJ_USER && g_NewPVP.IsDuel(*lpObj) && g_NewPVP.IsSafeState(*lpObj)) return FALSE;
	if (lpTargetObj->Type == OBJ_USER && g_NewPVP.IsObserver(*lpTargetObj)) return FALSE;
	if (lpObj->Type == OBJ_USER && g_NewPVP.IsObserver(*lpObj)) return FALSE;
	if (g_iUseCharacterAutoRecuperationSystem && lpObj->Level <= g_iCharacterRecuperationMaxLevel) {
		if (lpObj->Type == OBJ_USER) lpObj->m_iAutoRecuperationTime = GetTickCount();
		if (lpTargetObj->Type == OBJ_USER) lpTargetObj->m_iAutoRecuperationTime = GetTickCount();
	}

	if (lpObj->Type == OBJ_USER) lpObj->dwShieldAutoRefillTimer = GetTickCount();
	if (lpTargetObj->Type == OBJ_USER) lpTargetObj->dwShieldAutoRefillTimer = GetTickCount();

	int skill = 0;
	int skillLevel = 0;

	if (lpMagic) {
		skill = lpMagic->m_Skill;
		skillLevel = lpMagic->m_Level;
	}

	int skillAttr = MagicDamageC.MAGIC_GetSkillAttr(skill);

	skillSuccess = TRUE;

	if (lpObj->Type == OBJ_USER && lpObj->CharacterData->GuildNumber > 0) {
		if (lpObj->CharacterData->lpGuild) {
			if (lpObj->CharacterData->lpGuild->WarState) {
				if (lpObj->CharacterData->lpGuild->WarType == 1) {
					if (!GetBattleSoccerGoalMove(0)) return TRUE;
				}
			}

			if (lpObj->CharacterData->lpGuild->WarState) {
				if (lpObj->CharacterData->lpGuild->WarType == 0) {
					if (lpTargetObj->Type == OBJ_MONSTER) return TRUE;
				}
			}
		}
	}

	if (lpTargetObj->Type == OBJ_MONSTER) {
		if (lpTargetObj->m_iMonsterBattleDelay > 0)	return TRUE;
		if (lpTargetObj->Class == 200) {
			if (skill) gObjMonsterStateProc(lpTargetObj, 7, lpObj->m_Index, 0);
			else gObjMonsterStateProc(lpTargetObj, 6, lpObj->m_Index, 0);
			if (magicsend) GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, skill, lpTargetObj->m_Index, skillSuccess);
			return TRUE;
		}

		if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_MONSTER_MAGIC_IMMUNE) == TRUE) {
			BOOL bCheckAttackIsMagicType = gObjCheckAttackTypeMagic(lpObj->Class, skill);

			if (bCheckAttackIsMagicType == TRUE) {
				GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, skill, lpTargetObj->m_Index, 0);
				return TRUE;
			}

			if (lpTargetObj->Class == 673) {
				GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
				return TRUE;
			}
		}

		if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_MONSTER_MELEE_IMMUNE) == TRUE) {
			BOOL bCheckAttackIsMagicType = gObjCheckAttackTypeMagic(lpObj->Class, skill);

			if (bCheckAttackIsMagicType == FALSE) {
				GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
				return TRUE;
			}

			if (lpTargetObj->Class == 673) {
				GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
				return TRUE;
			}
		}

		if (lpTargetObj->m_MonsterSkillElementInfo.m_iSkillElementImmuneTime > 0) {
			if (lpTargetObj->m_MonsterSkillElementInfo.m_iSkillElementImmuneNumber == skill) return TRUE;
		}

		if (lpTargetObj->m_MonsterSkillElementInfo.m_iSkillElementImmuneAllTime > 0) {
			GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 32, 0);
			return TRUE;
		}

		if (lpTargetObj->Class == 523) return TRUE;
		if (lpTargetObj->Class >= 524 && lpTargetObj->Class <= 528 && lpTargetObj->Class != 526) {
			if (g_ImperialGuardian.IsAttackAbleMonster(lpTargetObj->m_Index) == false) return TRUE;
		}
	}

	if (lpObj->Type == OBJ_USER && lpObj->CharacterData->ISBOT == false) {
		if (!gObjIsConnected(lpObj)) return FALSE;
		if (lpObj->m_Change == 8) {
			skill = 1;
			lpMagic = &DefMagicInf[1];
			magicsend = TRUE;
		}

		gDarkSpirit[lpObj->m_Index - g_ConfigRead.server.GetObjectStartUserIndex()].SetTarget(lpTargetObj->m_Index);
		g_CMuunSystem.SetTarget(lpObj, lpTargetObj->m_Index);
	}

	if (lpTargetObj->Type == OBJ_USER) {
		if (!gObjIsConnected(lpTargetObj)) return FALSE;
	}

	if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_MONSTER) {
		if (lpObj->m_RecallMon >= 0) {
			if (lpObj->m_RecallMon == lpTargetObj->m_Index) return FALSE;
		}
	}

	if (!gObjAttackQ(lpTargetObj) && lpObj->m_bOffLevel == false) return FALSE;
	if (g_ConfigRead.antihack.EnableAttackBlockInSafeZone == TRUE) {
		BYTE btAttr = MapC[lpObj->MapNumber].GetAttr(lpObj->X, lpObj->Y);

		if ((btAttr & 1) == 1) {
			if (lpMagic) {
				if (g_SkillSafeZone.CanUseSkill(lpObj, lpMagic) == false) return FALSE;
			} else return FALSE;
		}
	}

	if (lpObj->m_RecallMon >= 0) gObjCallMonsterSetEnemy(lpObj, lpTargetObj->m_Index);
	if (lpTargetObj->m_RecallMon >= 0 && (lpObj->Class >= 100 && lpObj->Class <= 106)) gObjCallMonsterSetEnemy(lpTargetObj, lpObj->m_Index);

	lpObj->m_TotalAttackCount++;

	if (AttackDamage == 0) {
		if (skill != 76) {
			if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_INFINITY_ARROW) == FALSE && BUFF_TargetHasBuff(lpObj, BUFFTYPE_ETERNAL_ARROW) == FALSE) {
				if (!this->ATTACK_DecreaseArrow(lpObj)) return FALSE;
			}
		}
	}

	if (this->CheckAttackArea(lpObj, lpTargetObj) == FALSE)	return FALSE;

	lpCallObj = lpObj;

	if (lpObj->Type == OBJ_MONSTER) {
		if (lpObj->m_RecallMon >= 0) lpCallObj = &gObj[lpObj->m_RecallMon];
	}

	lpCallTargetObj = lpTargetObj;

	if (lpTargetObj->Type == OBJ_MONSTER) {
		if (lpTargetObj->m_RecallMon >= 0) lpCallTargetObj = &gObj[lpTargetObj->m_RecallMon];
	}

	if (g_GensSystem.IsMapBattleZone(lpObj->MapNumber) == FALSE && this->PkCheck(lpCallObj, lpTargetObj) == FALSE) return FALSE;

	int Strength = 0;
	int Dexterity = 0;
	int Vitality = 0;
	int Energy = 0;

	if (lpObj->Type == OBJ_USER) {
		Strength = lpObj->CharacterData->Strength + lpObj->AddStrength;
		Dexterity = lpObj->CharacterData->Dexterity + lpObj->AddDexterity;
		Vitality = lpObj->CharacterData->Vitality + lpObj->AddVitality;
		Energy = lpObj->CharacterData->Energy + lpObj->AddEnergy;
	}

	BOOL bIsOnDuel = gObjDuelCheck(lpObj, lpTargetObj);

	if (bIsOnDuel) {
		lpObj->m_iDuelTickCount = GetTickCount();
		lpTargetObj->m_iDuelTickCount = GetTickCount();
	}

	MSBFlag = 0;
	MsgDamage = 0;
	skillSuccess = FALSE;

	BOOL bAllMiss = FALSE;

	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_INVISABLE) == TRUE && (lpObj->Authority & 0x20) != 0x20) gObjUseSkill.SIEGE_RemoveCloakingEffect(lpObj->m_Index);

	BOOL bDamageReflect = FALSE;
	int iTargetDefense = 0;
	BOOL bPentagramBossMonster = FALSE;

	if (lpTargetObj->m_iPentagramAttributePattern == 2)	bPentagramBossMonster = TRUE;
	if (AttackDamage > 0 || bPentagramBossMonster) {
		if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE) {
			if (g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) {
				if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) {
					if (lpObj->MapNumber == MAP_INDEX_CASTLESIEGE && lpTargetObj->MapNumber == MAP_INDEX_CASTLESIEGE) {
						if (lpObj->m_btCsJoinSide == lpTargetObj->m_btCsJoinSide) {
							if (g_CastleSiege.m_bCastleSiegeFriendlyFire == false) AttackDamage = 0;
							else AttackDamage = AttackDamage * g_CastleSiege.CastleSiegeSelfDmgReductionPercent / 100;
						}
						else if (g_ShieldSystemOn == 0) AttackDamage = AttackDamage * g_CastleSiege.CastleSiegeDmgReductionPercent / 100;
					}
				}
			}

			if (g_CastleSiege.GetCastleState() != CASTLESIEGE_STATE_STARTSIEGE) {
				if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_MONSTER) {
					if (lpObj->MapNumber == MAP_INDEX_CASTLESIEGE && lpTargetObj->MapNumber == MAP_INDEX_CASTLESIEGE) {
						if (lpTargetObj->Class == 277 || lpTargetObj->Class == 283) return FALSE;
					}
				}
			}
		}

		if (skill == 216) {
			if (g_ShieldSystemOn == TRUE) {
				if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) {
					if (!this->ATTACK_MissCheckPvP(lpObj, lpTargetObj, skill, skillSuccess, magicsend, bAllMiss, 0)) return FALSE;
				} else {
					if (!this->ATTACK_MissCheck(lpObj, lpTargetObj, skill, skillSuccess, magicsend, bAllMiss, 0)) return FALSE;
				}
			} else {
				if (!this->ATTACK_MissCheck(lpObj, lpTargetObj, skill, skillSuccess, magicsend, bAllMiss, 0)) return FALSE;
			}
		}

		if (skill != 79 && skill != 216 && !bPentagramBossMonster) {
			bDamageReflect = TRUE;
			skillAttr = -1;
			MsgDamage = 4;
		}

		if (skill == 215 || skill == 455) bCombo = 0;
		if (bPentagramBossMonster) AttackDamage = 0;
		if (lpObj->Type != OBJ_USER || lpTargetObj->Type != OBJ_USER) {
			lpTargetObj->Life -= AttackDamage;

			if (lpTargetObj->Life <= 0.0) lpTargetObj->Life = 0.0;
		} else {
			if (skill == 265 || skill == 564 || skill == 566) {
				int nSuccessRate = 0, nDecreaseSDRate = 0;
				gObjUseSkill.m_Lua.Generic_Call("DragonFist_ShieldPenetration", "i>ii", Energy, &nSuccessRate, &nDecreaseSDRate);

				if (rand() % 100 < nSuccessRate) {
					bDragonKickSDAttackSuccess = TRUE;

					int iDragonKickSDDamage = nDecreaseSDRate * lpTargetObj->iShield / 100;

					lpTargetObj->iShield -= iDragonKickSDDamage;
					lpTargetObj->Life -= AttackDamage;
					iTotalShieldDamage += iDragonKickSDDamage;
				} else {
					iTempShieldDamage = this->ATTACK_CalcShieldDamage(lpObj, lpTargetObj, AttackDamage);
					lpTargetObj->iShield -= iTempShieldDamage;
					lpTargetObj->Life -= AttackDamage - iTempShieldDamage;
					iTotalShieldDamage += iTempShieldDamage;
				}
			} else {
				iTempShieldDamage = this->ATTACK_CalcShieldDamage(lpObj, lpTargetObj, AttackDamage);
				lpTargetObj->iShield -= iTempShieldDamage;
				lpTargetObj->Life -= AttackDamage - iTempShieldDamage;
				iTotalShieldDamage += iTempShieldDamage;
			}

			if (lpTargetObj->Life <= 0.0) lpTargetObj->Life = 0.0;
		}
	} else if (AttackDamage == 0) {
		if (g_ShieldSystemOn == TRUE) {
			if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) {
				if (!this->ATTACK_MissCheckPvP(lpObj, lpTargetObj, skill, skillSuccess, magicsend, bAllMiss, RFAttack)) return FALSE;
			} else {
				if ((lpObj->Type != OBJ_MONSTER || !lpObj->m_iPentagramMainAttribute) && !this->ATTACK_MissCheck(lpObj, lpTargetObj, skill, skillSuccess, magicsend, bAllMiss, RFAttack)) return FALSE;
			}
		} else {
			if (!this->ATTACK_MissCheck(lpObj, lpTargetObj, skill, skillSuccess, magicsend, bAllMiss, RFAttack)) return FALSE;
		}

		int iOriginTargetDefense = 0;
		int targetdefense = this->ATTACK_CalcTargetDefense(lpObj, lpTargetObj, MsgDamage, iOriginTargetDefense, skill);

		if (lpTargetObj->Type == OBJ_USER) {
			if (lpTargetObj->pInventory[7].IsItem() && lpTargetObj->pInventory[7].m_IsValidItem == true) targetdefense += lpTargetObj->CharacterData->Mastery.ML_GENERAL_WingsDefense;
			if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_AMPLIFY_DAMAGE)) {
				float fDefense = Dexterity / 3;
				float fValue = (float)BUFF_GetTotalBuffEffectValue(lpTargetObj, 32);
				float fPercent = (40 - fValue) / 100;

				fPercent = (0.1f > fPercent) ? 0.1f : fPercent;
				targetdefense -= fDefense * fPercent;
				iOriginTargetDefense -= fDefense * fPercent;
			}

			if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_IMPROVED_AMPLIFY_DAMAGE) || BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_DIMENSIONAL_AFFINITY) || BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_VOID_AFFINITY)) {
				float fDefense = Dexterity / 3;
				float fValue = (float)BUFF_GetTotalBuffEffectValue(lpTargetObj, 32);
				float fPercent = (40 - (fValue + lpTargetObj->CharacterData->Mastery.ML_SUM_ImprovedAmplifyDamage)) / 100;

				fPercent = (0.1f > fPercent) ? 0.1f : fPercent;
				targetdefense -= fDefense * fPercent;
				iOriginTargetDefense -= fDefense * fPercent;
			}
		}

		int iCurseValue;

		if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_WEAKNESS)) {
			iCurseValue = 0;
			BUFF_GetBuffEffectValue(lpTargetObj, DEBUFFTYPE_WEAKNESS, &iCurseValue, NULL);
			targetdefense -= targetdefense * iCurseValue / 100;
			iOriginTargetDefense -= iOriginTargetDefense * iCurseValue / 100;
		}

		if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_IMPROVED_WEAKNESS)) {
			iCurseValue = 0;
			BUFF_GetBuffEffectValue(lpTargetObj, DEBUFFTYPE_IMPROVED_WEAKNESS, &iCurseValue, NULL);
			targetdefense -= targetdefense * iCurseValue / 100;
			iOriginTargetDefense -= iOriginTargetDefense * iCurseValue / 100;
		}

		iTargetDefense = iOriginTargetDefense;

		int iFenrirAttackDmg = 0;
		float charIncPercent = 1;
		float itemIncPercent = 1;
		float masteryIncPercent = 1;
		float buffIncPercent = 1;
		float wingsIncPercent = 1;
		float globalIncPercent = 1;
		float charDecPercent = 1;
		float itemDecPercent = 1;
		float masteryDecPercent = 1;
		float buffDecPercent = 1;
		float wingsDecPercent = 1;
		float globalDecPercent = 1;

		if (skill == 19 || skill == 20 || skill == 21 || skill == 23 || skill == 56 || skill == 41 || skill == 47 || skill == 42 || skill == 49 || skill == 43 || skill == 22 || skill == 55 || skill == 44 || skill == 57 || skill == 74 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skill) == 330 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skill) == 481 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skill) == 336 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skill) == 331 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skill) == 490 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skill) == 482 || skill == 235 || skill == 232 || skill == 236 || skill == 238 || skill == 326 || skill == 327 || skill == 328 || skill == 329 || skill == 479 || skill == 337 || skill == 332 || skill == 333 || skill == 344 || skill == 346 || skill == 411 || skill == 431 || skill == 492 || skill == 494 || skill == 523 || skill == 493) {
			AttackDamage = this->ATTACK_CalcPhysicalDamage(lpObj, lpTargetObj, targetdefense, lpMagic, charIncPercent, buffDecPercent, MsgDamage);

			if (AttackDamage > 0) gObjWeaponDurDown(lpObj, lpTargetObj, 0);
		} else if (skill == 76 && lpObj->Type == OBJ_USER) {
			if (lpObj->Class == CLASS_KNIGHT || lpObj->Class == CLASS_MAGICGLADIATOR || lpObj->Class == CLASS_RAGEFIGHTER) iFenrirAttackDmg = lpObj->CharacterData->Strength / 3 + lpObj->CharacterData->Dexterity / 5 + lpObj->CharacterData->Vitality / 5 + lpObj->CharacterData->Energy / 7;	// #formula
			else if (lpObj->Class == CLASS_WIZARD || lpObj->Class == CLASS_SUMMONER) iFenrirAttackDmg = lpObj->CharacterData->Strength / 5 + lpObj->CharacterData->Dexterity / 5 + lpObj->CharacterData->Vitality / 7 + lpObj->CharacterData->Energy / 3;	// #formula
			else if (lpObj->Class == CLASS_ELF) iFenrirAttackDmg = lpObj->CharacterData->Strength / 5 + lpObj->CharacterData->Dexterity / 3 + lpObj->CharacterData->Vitality / 7 + lpObj->CharacterData->Energy / 5;	// #formula
			else iFenrirAttackDmg = lpObj->CharacterData->Strength / 5 + lpObj->CharacterData->Dexterity / 5 + lpObj->CharacterData->Vitality / 7 + lpObj->CharacterData->Energy / 3 + lpObj->Leadership / 3;	// #formula
			if (iFenrirAttackDmg < 0) iFenrirAttackDmg = 0;

			AttackDamage = (iFenrirAttackDmg + lpMagic->m_DamageMin) + (rand() % (lpMagic->m_DamageMax - lpMagic->m_DamageMin + 1)) - targetdefense;
		} else {
			if ((lpObj->Class == CLASS_WIZARD || lpObj->Class == CLASS_SUMMONER || lpObj->Class == CLASS_MAGICGLADIATOR) && skill) {
				AttackDamage = this->ATTACK_CalcMagicDamage(lpObj, lpTargetObj, targetdefense, lpMagic, itemIncPercent, MsgDamage);

				if (AttackDamage > 0) gObjWeaponDurDown(lpObj, lpTargetObj, 1);
			} else {
				AttackDamage = this->ATTACK_CalcPhysicalDamage(lpObj, lpTargetObj, targetdefense, lpMagic, charIncPercent, buffDecPercent, MsgDamage);

				if (AttackDamage > 0) gObjWeaponDurDown(lpObj, lpTargetObj, 0);
			}
		}
		// Critical Damage
		if (MsgDamage == 3) {
			int iOption = BUFF_GetTotalBuffEffectValue(lpObj, EFFECTTYPE_INCREASE_CRITICAL_DAMAGE);
			
			if (iFenrirAttackDmg > 0) AttackDamage = iFenrirAttackDmg + lpMagic->m_DamageMax - targetdefense;

			AttackDamage += iOption;
			AttackDamage += lpObj->CharacterData->ANCIENT_CriticalDamage;
			AttackDamage += lpObj->CharacterData->JewelOfHarmonyEffect.CriticalDamage;
		}

		int tlevel;

		if (bAllMiss) AttackDamage = (AttackDamage * 30) / 100;
		if (lpObj->Type == OBJ_USER) tlevel = (lpObj->Level + lpObj->CharacterData->MasterLevel) / 10;
		else tlevel = lpObj->Level / 10;
		if (AttackDamage < tlevel) {
			if (tlevel < 1) tlevel = 1;

			AttackDamage = tlevel;
		}

		// Reduce Damage Taken Modifier
		if (lpTargetObj->EXC_ReduceDamageTaken) {
			if (lpTargetObj->EXC_ReduceDamageTaken > g_ConfigRead.calc.MaximumDamageDec) lpTargetObj->EXC_ReduceDamageTaken = g_ConfigRead.calc.MaximumDamageDec;

			charDecPercent -= (int)lpTargetObj->EXC_ReduceDamageTaken / 100.00;			//AttackDamage -= ((AttackDamage * (int)lpTargetObj->EXC_ReduceDamageTaken) / 100);
		}

		// Ancient Sets Attribute Damage Bonus
		if (lpObj->Type == OBJ_USER && skillAttr != -1) {
			if (lpObj->CharacterData->ANCIENT_AttributeDamage[skillAttr] > 0) {
				float value = (float)lpObj->CharacterData->ANCIENT_AttributeDamage[skillAttr] / 100;

				charIncPercent += value;	//AttackDamage += AttackDamage * value;
			}
		}

		COMPANION_DurabilityDecrease(lpTargetObj, AttackDamage);

		if (lpTargetObj->m_btInvenPetPos != 0 && lpTargetObj->m_wInvenPet != (WORD)-1) gObjInvenPetDamage(lpTargetObj, AttackDamage);
		if (lpTargetObj->Live) {
			// Summoner Books
			if (lpObj->pInventory[1].GetDetailItemType() == ITEM_SUMMONER_BOOK && lpObj->pInventory[1].m_Special[0] == skill) {
				if (skill == SUM_SPIRIT_OF_FLAMES) {
					if (lpObj->CharacterData->Mastery.ML_SUM_ImprovedSahamutt > 0.0f) AttackDamage += lpObj->CharacterData->Mastery.ML_SUM_ImprovedSahamutt;
				}
				else if (skill == SUM_SPIRIT_OF_ANGUISH) {
					if (lpObj->CharacterData->Mastery.ML_SUM_ImprovedNeil > 0.0f) AttackDamage += lpObj->CharacterData->Mastery.ML_SUM_ImprovedNeil;
					if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 452) >= 10) lethalBonus += 25;
				}
				else if (skill == SUM_SPIRIT_OF_TERROR) {
					if (lpObj->CharacterData->Mastery.ML_SUM_ImprovedPhantasm > 0.0f) AttackDamage += lpObj->CharacterData->Mastery.ML_SUM_ImprovedPhantasm;
					if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 450) == 20) manaBurn += 0.1;
				}
			}

			// Summoner Wings Implicit
			if (lpTargetObj->pInventory[7].IsItem() == TRUE && lpTargetObj->pInventory[7].m_IsValidItem == true) {
				CItem* SumWings = &lpTargetObj->pInventory[7];

				if (SumWings->m_Durability > 0.0f) {
					switch (SumWings->m_Type) {
						case ITEMGET(12, 41):			AttackDamage -= 10;				break;			// Cursed Wings
						case ITEMGET(12, 42):			AttackDamage -= 25;				break;			// Illusion Wings
						case ITEMGET(12, 43):			AttackDamage -= 50;				break;			// Dimensional Wings
					}
				}
			}

			if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) AttackDamage += lpObj->CharacterData->PvPEquipmentMods.Damage;
			if (!byReflect && lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) {
				if (lpObj->CharacterData->Mastery.ML_DK_TwoHandedSwordExpertise > 0.0) AttackDamage += lpObj->CharacterData->Mastery.ML_DK_TwoHandedSwordExpertise;
				if (lpObj->CharacterData->Mastery.ML_DW_TwoHandedStaffExpertise > 0.0) AttackDamage += lpObj->CharacterData->Mastery.ML_DW_TwoHandedStaffExpertise;
				if (lpObj->CharacterData->Mastery.ML_ELF_CrossbowExpertise > 0.0) AttackDamage += lpObj->CharacterData->Mastery.ML_ELF_CrossbowExpertise;
				if (lpObj->CharacterData->Mastery.ML_SUM_StaffExpertise > 0.0) AttackDamage += lpObj->CharacterData->Mastery.ML_SUM_StaffExpertise;
				if (lpObj->CharacterData->Mastery.ML_DL_ScepterExpertise > 0.0) AttackDamage += lpObj->CharacterData->Mastery.ML_DL_ScepterExpertise;
			}

			switch (skill) {
				case 19:
				case 20:
				case 21:
				case 22:
				case 23:
				case 41:
				case 42:
				case 43:
				case 44:
				case 46:
				case 49:
				case 51:
				case 52:
				case 55:
				case 56:
				case 57:
				case 60:
				case 61:
				case 62:
				case 65:
				case 74:
				case 78:
				case 232:								gObjUseSkill.m_Lua.Generic_Call("EnergySkillBonus", "iii>i", lpObj->Class, AttackDamage, Energy, &AttackDamage);											break;
				case 236:								gObjUseSkill.m_Lua.Generic_Call("EnergySkillBonus", "iii>i", lpObj->Class, AttackDamage, Energy, &AttackDamage);
														BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_FLAMESTRKE_VISUAL, 0, 0, 0, 0, 1);																				break;
				case 238:								gObjUseSkill.m_Lua.Generic_Call("EnergySkillBonus", "iii>i", lpObj->Class, AttackDamage, Energy, &AttackDamage);
														if (lpObj->Class == 561) {
															lpTargetObj->DelayActionTime = 800;
															lpTargetObj->DelayLevel = 1;
															lpTargetObj->lpAttackObj = lpObj;
															AttackDamage = 1000;
														}																																							break;

				case 47:
					if (lpObj->pInventory[lpObj->m_btInvenPetPos].m_Type == ITEMGET(13, 3) || lpObj->pInventory[lpObj->m_btInvenPetPos].m_Type == ITEMGET(13, 2) || lpObj->pInventory[lpObj->m_btInvenPetPos].m_Type == ITEMGET(13, 37)) gObjUseSkill.m_Lua.Generic_Call("Impale", "iii>i", lpObj->Class, AttackDamage, Energy, &AttackDamage);
				break;

				case 76:
					if (lpObj->Type != OBJ_USER) break;
					if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) {
						if (AttackDamage > 0) {
							if (g_ConfigRead.server.GetServerType() != SERVER_CASTLE || (lpObj->m_btCsJoinSide == FALSE || (lpObj->m_btCsJoinSide != lpTargetObj->m_btCsJoinSide))) {
								int iEquipmentPos = rand() % 5 + 2;
								CItem* lpEquipment = &lpTargetObj->pInventory[iEquipmentPos];

								if (lpEquipment && lpEquipment->IsItem()) {
									int iDurDecValue = 50;

									if (lpTargetObj->Class == CLASS_RAGEFIGHTER) {
										if (lpTargetObj->CharacterData->Mastery.ML_RF_Efficiency > 0.0) iDurDecValue = lpTargetObj->CharacterData->Mastery.ML_RF_EfficiencyLevel + 50;
									} else if (lpTargetObj->CharacterData->Mastery.ML_GENERAL_Efficiency > 0.0) iDurDecValue = lpTargetObj->CharacterData->Mastery.ML_GENERAL_EfficiencyLevel + 50;

									if (iDurDecValue < 0) iDurDecValue = 1;

									int iDurEquipment = lpEquipment->m_Durability * iDurDecValue / 100.0f;

									for (int iBuffIndex = 0; iBuffIndex < lpTargetObj->m_BuffEffectCount; iBuffIndex++) {
										if (lpTargetObj->m_BuffEffectList[iBuffIndex].BuffIndex == BUFFTYPE_CHARM_PROTECTITEM) {
											iDurEquipment = lpEquipment->m_Durability;
											break;
										}
									}

									if (g_LuckyItemManager.IsLuckyItemEquipment(lpEquipment->m_Type)) iDurEquipment = lpEquipment->m_Durability;

									lpEquipment->m_Durability = iDurEquipment;

									if (lpEquipment->m_Durability < 0.0f) lpEquipment->m_Durability = 0.0f;

									GSProtocol.GCItemDurSend(lpTargetObj->m_Index, iEquipmentPos, lpEquipment->m_Durability, 0);
								}
							}
						}
					}

					gObjUseSkill.m_Lua.Generic_Call("PlasmaStorm", "iii>i", AttackDamage, lpObj->Level, lpObj->CharacterData->MasterLevel, &AttackDamage);
				break;

				case 214:
					if (lpTargetObj->Type == OBJ_MONSTER) gObjUseSkill.m_Lua.Generic_Call("DrainLife_PvE", "ii>i", Energy, lpTargetObj->Level, &LifeChange);
					else if (lpTargetObj->Type == OBJ_USER) gObjUseSkill.m_Lua.Generic_Call("DrainLife_PvP", "ii>i", Energy, AttackDamage, &LifeChange);

					lpObj->Life += LifeChange;

					if ((lpObj->MaxLife + lpObj->AddLife) < lpObj->Life) lpObj->Life = lpObj->MaxLife + lpObj->AddLife;

					GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
				break;

				case 216:								gObjUseSkill.SplashDamage(lpObj, lpTargetObj, 216, AttackDamage, 1, 50);																					break;
				case 219: {
					int SuccessRate = 0;
					int Time = 0;

					if (lpTargetObj->Type == OBJ_MONSTER) gObjUseSkill.m_Lua.Generic_Call("Sleep_PvE", "iii>ii", Energy, lpObj->m_CurseSpell, lpTargetObj->Level, &SuccessRate, &Time);
					else if (lpTargetObj->Type == OBJ_USER) gObjUseSkill.m_Lua.Generic_Call("Sleep_PvP", "iiii>ii", Energy, lpObj->m_CurseSpell, lpObj->Level, lpTargetObj->Level, &SuccessRate, &Time);
					if (CHARACTER_DebuffProc(SuccessRate)) {
						if (Time >= 0) {
							if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 524) >= 10 && BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_IMPROVED_STONESKIN)) {
								if (BUFF_IsStrongerBuff(lpTargetObj, DEBUFFTYPE_SLEEP, Time, 0)) GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, SUM_SLEEP, lpTargetObj->m_Index, 0);
								else BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_SLEEP, EFFECTTYPE_SLEEP, Time, 0, 0, Time);
							} else GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
						} else GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
					} else GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);

					return FALSE;
				}
				break;

				case 221: {
					int SuccessRate = 0;
					int Value = 0;
					int Time = 0;

					if (lpTargetObj->Type == OBJ_MONSTER) gObjUseSkill.m_Lua.Generic_Call("Enfeeble_PvE", "iii>iii", Energy, lpObj->m_CurseSpell, lpTargetObj->Level, &SuccessRate, &Value, &Time);
					else if (lpTargetObj->Type == OBJ_USER) gObjUseSkill.m_Lua.Generic_Call("Enfeeble_PvP", "iiii>iii", Energy, lpObj->m_CurseSpell, lpObj->Level, lpTargetObj->Level, &SuccessRate, &Value, &Time);
					if (CHARACTER_DebuffProc(SuccessRate) == FALSE) {
						GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
						return FALSE;
					} else {
						if (Time >= 1) {
							if (BUFF_IsStrongerBuff(lpTargetObj, DEBUFFTYPE_ENFEEBLE, Value, 0)) {
								GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, skill, lpTargetObj->m_Index, 0);
								return FALSE;
							} else {
								BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_ENFEEBLE, EFFECTTYPE_DECREASE_DAMAGE, Value, 0, 0, Time);
								return TRUE;
							}
						} else {
							GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
							return FALSE;
						}
					}

					return FALSE;
				}
				break;

				case 222: {
					int SuccessRate = 0;
					int Time = 0;
					int Value = 0;

					if (lpTargetObj->Type == OBJ_MONSTER) gObjUseSkill.m_Lua.Generic_Call("Weakness_PvE", "iii>iii", Energy, lpObj->m_CurseSpell, lpTargetObj->Level, &SuccessRate, &Value, &Time);
					else if (lpTargetObj->Type == OBJ_USER) gObjUseSkill.m_Lua.Generic_Call("Weakness_PvP", "iiii>iii", Energy, lpObj->m_CurseSpell, lpObj->Level, lpTargetObj->Level, &SuccessRate, &Value, &Time);
					if (CHARACTER_DebuffProc(SuccessRate) == FALSE) {
						GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
						return FALSE;
					} else {
						if (Time >= 1) {
							if (BUFF_IsStrongerBuff(lpTargetObj, DEBUFFTYPE_WEAKNESS, Value, 0)) {
								GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, skill, lpTargetObj->m_Index, FALSE);
								return FALSE;
							} else {
								BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_WEAKNESS, EFFECTTYPE_DECREASE_DEFENSE_SUM, Value, 0, 0, Time);
								return TRUE;
							}
						} else {
							GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
							return FALSE;
						}
					}

					return FALSE;
				}
				break;

				case SUM_CHAINLIGHTNING:				gObjUseSkill.m_Lua.Generic_Call("ChainLightning", "ii>i", AttackDamage, bCombo, &AttackDamage);
														bCombo = 0;																																					break;
				case 230:								BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_LIGHTNINGSHOCK_VISUAL, 0, 0, 0, 0, 1);																			break;
				case 237:								BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_THUNDERSTORM_VISUAL, 0, 0, 0, 0, 1);																				break;
				case 250:								AttackDamage *= 2;																																			break;	// Selupan
				case 251:								AttackDamage *= 2.2;																																		break;	// Selupan
				case 252:								AttackDamage *= 2.3;																																		break;	// Selupan
				case 253:								AttackDamage *= 2.5;																																		break;	// Selupan
				case 260:
				case 261:
				case 262:
				case RF_CHARGE:
				case RF_PHOENIXSHOT:					gObjUseSkill.m_Lua.Generic_Call("RageFighterVitalitySkillBonus", "ii>i", AttackDamage, Vitality, &AttackDamage);											break;
				case RF_DARKSIDE:
				case RF_MOLTENSTRIKE:					gObjUseSkill.m_Lua.Generic_Call("RageFighterEnergySkillBonus", "iii>i", AttackDamage, Vitality, Energy, &AttackDamage);										break;
				case RF_DRAGONFIST:						gObjUseSkill.m_Lua.Generic_Call("DragonFist", "iiii>i", AttackDamage, Dexterity, Vitality, Energy, &AttackDamage);											break;
				case AT_SKILL_SPIN_STEP:				gObjUseSkill.m_Lua.Generic_Call("GrowLancerSpinStep", "ii>i", AttackDamage, Strength, &AttackDamage);														break;
				case AT_SKILL_HARSH_STRIKE:				gObjUseSkill.m_Lua.Generic_Call("GrowLancerHarshStrike", "ii>i", AttackDamage, Dexterity, &AttackDamage);													break;
				case AT_SKILL_MAGIC_PIN:
				case AT_SKILL_MAGIC_PIN_EXPLOSION:		gObjUseSkill.m_Lua.Generic_Call("GrowLancerMagicPin", "ii>i", AttackDamage, Strength, &AttackDamage);														break;
				case AT_SKILL_BRECHE:					gObjUseSkill.m_Lua.Generic_Call("GrowLancerBreche", "ii>i", AttackDamage, Dexterity, &AttackDamage);														break;
				case AT_SKILL_SHINING_PEAK:				gObjUseSkill.m_Lua.Generic_Call("GrowLancerShiningPeak", "iii>i", AttackDamage, Strength, Dexterity, &AttackDamage);										break;
				case AT_SKILL_CLASH: {
					int iAccessoryEffect = 0;

					if (lpTargetObj->Type == OBJ_USER) iAccessoryEffect = lpTargetObj->CharacterData->C_StunResistance;

					int iStunRate = 100 - iAccessoryEffect;

					if (rand() % 100 < (100 - iStunRate)) BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_STUN, 0, 0, 0, 0, 2);
				}
				break;

				default: {
					if (g_MasterLevelSkillTreeSystem.CheckMasterLevelSkill(skill)) {
						double fSkillValue = g_MasterLevelSkillTreeSystem.ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
						double fPrevBase = 0.0;
						int iMLSBase = g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skill);

						switch (iMLSBase) {
							case 326:
							case 328:
							case 329:
							case 330:
							case 331:
							case 332:
							case 333:
							case 336:
							case 337:
							case 339:
							case 340:
							case 343:
							case 344:
							case 346:
							case 416:
							case 424:
							case 479:
							case 481:
							case 482:
							case 490:
							case 508:
							case 512:
							case 516:
							case 518:			gObjUseSkill.m_Lua.Generic_Call("EnergySkillBonus", "iii>i", lpObj->Class, AttackDamage, Energy, &AttackDamage);															break;
							case 509:			gObjUseSkill.m_Lua.Generic_Call("EnergySkillBonus", "iii>i", lpObj->Class, AttackDamage, Energy, &AttackDamage);
												if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, iMLSBase) == 20) {
													if (GetItemKindA(lpObj->pInventory[0].m_Type) == ITEM_KIND_A_WEAPON && GetItemKindB(lpObj->pInventory[0].m_Type) == ITEM_KIND_B_SCEPTER) {
														int damage = lpObj->pInventory[0].m_Magic / 2;

														damage -= damage * lpObj->pInventory[0].m_CurrentDurabilityState;
														masteryIncPercent += damage / 100.00;
													}
												}																																											break;		// Improved Force Wave
							case 514:			gObjUseSkill.m_Lua.Generic_Call("EnergySkillBonus", "iii>i", lpObj->Class, AttackDamage, Energy, &AttackDamage);
												if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_BURN)) {
													if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, iMLSBase) >= 10) masteryIncPercent += 0.10;
													if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, iMLSBase) == 20 && CHARACTER_DebuffProc(25)) {
														int buffTime = BUFF_GetRemainingDuration(lpTargetObj, DEBUFFTYPE_BURN);
														int buffValue = 0;
														int bonusDmg = 0;

														BUFF_GetBuffEffectValue(lpTargetObj, DEBUFFTYPE_BURN, &buffValue, NULL);
														bonusDmg = buffTime * buffValue;
														g_BuffEffect.BUFFEFFECT_DamageOverTime(lpTargetObj, bonusDmg);
														BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_BURN);
													}
												}																																											break;		// Fireblast
							case 551:
							case 554:
							case 552:
							case 555:
							case 558:
							case 562:			gObjUseSkill.m_Lua.Generic_Call("RageFighterVitalitySkillBonus", "ii>i", AttackDamage, Vitality, &AttackDamage);															break;
							case 559:
							case 563:			AttackDamage += Dexterity / 8; 
												gObjUseSkill.m_Lua.Generic_Call("RageFighterEnergySkillBonus", "iii>i", AttackDamage, Vitality, Energy, &AttackDamage);														break;
							case 560:
							case 561:			gObjUseSkill.m_Lua.Generic_Call("RageFighterEnergySkillBonus", "iii>i", AttackDamage, Vitality, Energy, &AttackDamage);
												if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 560) == 20 && BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_BURN)) masteryIncPercent += 0.15;							break;
							case 564:			g_MasterLevelSkillTreeSystem.m_Lua.Generic_Call("ImprovedDragonFist", "idiii>i", AttackDamage, fSkillValue, Dexterity, Vitality, Energy, &AttackDamage);					break;
							case 566:			fPrevBase = g_MasterLevelSkillTreeSystem.GetBrandOfMasterSkillValue(lpObj, skill, 1);
												g_MasterLevelSkillTreeSystem.m_Lua.Generic_Call("ImprovedDragonFist", "idiii>i", AttackDamage, fPrevBase, Dexterity, Vitality, Energy, &AttackDamage);						break;
							case 327:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, iMLSBase) == 20) lethalBonus += 10;
												gObjUseSkill.m_Lua.Generic_Call("EnergySkillBonus", "iii>i", lpObj->Class, AttackDamage, Energy, &AttackDamage);															break;
							case 342:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, iMLSBase) >= 10 && lpTargetObj->Life < (lpTargetObj->MaxLife + lpTargetObj->AddLife) * 0.3) lethalBonus += 20;
												gObjUseSkill.m_Lua.Generic_Call("EnergySkillBonus", "iii>i", lpObj->Class, AttackDamage, Energy, &AttackDamage);															break;
							case 392:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, iMLSBase) == 20 && BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_BURN)) {
													int buffTime = BUFF_GetRemainingDuration(lpTargetObj, DEBUFFTYPE_BURN);
													int buffValue = 0;
													int bonusDmg = 0;

													BUFF_GetBuffEffectValue(lpTargetObj, DEBUFFTYPE_BURN, &buffValue, NULL);
													bonusDmg = buffTime * buffValue;
													g_BuffEffect.BUFFEFFECT_DamageOverTime(lpTargetObj, bonusDmg);
													BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_BURN);
												}																																											break;		// Supernova
							case 379:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, iMLSBase) == 20 && BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_SHOCK)) masteryIncPercent += 0.30;					break;		// Improved Lightning
							case 381:
							case 486:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, iMLSBase) == 20 && BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_BURN)) masteryIncPercent += 0.10;						break;
							case 497: 			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, iMLSBase) == 20 && BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_TRAUMA)) masteryIncPercent += 0.30;					break;		// Improved Earthen Prison
							case 459: {
								int iSuccessRate = 0;
								int iEffectTime = 0;
								int iEffectValue = 0;

								if (lpTargetObj->Type == OBJ_MONSTER) g_MasterLevelSkillTreeSystem.m_Lua.Generic_Call("SummonerWeakness_Monster", "iii>iii", Energy, lpObj->m_CurseSpell, lpTargetObj->Level, &iSuccessRate, &iEffectValue, &iEffectTime);
								else if (lpTargetObj->Type == OBJ_USER) g_MasterLevelSkillTreeSystem.m_Lua.Generic_Call("SummonerWeakness_PvP", "iiii>iii", Energy, lpObj->m_CurseSpell, lpObj->Level, lpTargetObj->Level, &iSuccessRate, &iEffectValue, &iEffectTime);
								if (!CHARACTER_DebuffProc(iSuccessRate)) {
									GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
									return FALSE;
								}

								iEffectTime += iEffectTime * fSkillValue / 100;
								iEffectValue += fSkillValue;

								if (iEffectTime < 1) {
									GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
									return FALSE;
								}

								if (BUFF_IsStrongerBuff(lpTargetObj, DEBUFFTYPE_IMPROVED_ENFEEBLE, iEffectValue, 0)) {
									GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, 459, lpTargetObj->m_Index, FALSE);
									return FALSE;
								} else {
									BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_IMPROVED_ENFEEBLE, EFFECTTYPE_DECREASE_DAMAGE, iEffectValue, 0, 0, iEffectTime);
									return TRUE;
								}
							}
							break;

							case 460: {
								int iSuccessRate = 0;
								int iEffectTime = 0;
								int iEffectValue = 0;

								if (lpTargetObj->Type == OBJ_MONSTER) g_MasterLevelSkillTreeSystem.m_Lua.Generic_Call("SummonerInnovation_Monster", "iii>iii", Energy, lpObj->m_CurseSpell, lpTargetObj->Level, &iSuccessRate, &iEffectValue, &iEffectTime);
								else if (lpTargetObj->Type == OBJ_USER) g_MasterLevelSkillTreeSystem.m_Lua.Generic_Call("SummonerInnovation_PvP", "iiii>iii", Energy, lpObj->m_CurseSpell, lpObj->Level, lpTargetObj->Level, &iSuccessRate, &iEffectValue, &iEffectTime);
								if (!CHARACTER_DebuffProc(iSuccessRate)) {
									GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
									return FALSE;
								}

								iEffectTime += iEffectTime * fSkillValue / 100;
								iEffectValue += fSkillValue;

								if (iEffectTime < 1) {
									GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
									return FALSE;
								}

								if (BUFF_IsStrongerBuff(lpTargetObj, DEBUFFTYPE_IMPROVED_WEAKNESS, iEffectValue, 0)) {
									GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, 460, lpTargetObj->m_Index, FALSE);
									return FALSE;
								} else {
									BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_IMPROVED_WEAKNESS, EFFECTTYPE_DECREASE_DEFENSE_SUM, iEffectValue, 0, 0, iEffectTime);
									return TRUE;
								}
							}
							break;

							case 461:
							case 463:			return FALSE;
							case 492:			gObjUseSkill.m_Lua.Generic_Call("EnergySkillBonus", "iii>i", lpObj->Class, AttackDamage, Energy, &AttackDamage);
												BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_FLAMESTRKE_VISUAL, 0, 0, 0, 0, 1);																								break;
							case 494:			gObjUseSkill.m_Lua.Generic_Call("EnergySkillBonus", "iii>i", lpObj->Class, AttackDamage, Energy, &AttackDamage);
												BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_FLAMESTRKE_VISUAL, 0, 0, 0, 0, 1);																								break;
							case 493:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, iMLSBase) >= 10 && BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_BURN)) masteryIncPercent += 0.15;
												gObjUseSkill.m_Lua.Generic_Call("EnergySkillBonus", "iii>i", lpObj->Class, AttackDamage, Energy, &AttackDamage);															break;
							case 496:			BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_THUNDERSTORM_VISUAL, 0, 0, 0, 0, 1);																								break;
							case 519:			gObjUseSkill.m_Lua.Generic_Call("EnergySkillBonus", "iii>i", lpObj->Class, AttackDamage, Energy, &AttackDamage);																	// Improved Spark
												if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, iMLSBase) >= 10) lethalBonus += 50;
												if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, iMLSBase) == 20 && BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_SHOCK)) masteryIncPercent += 0.50;
												BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_SHOCK);																														break;
							case 520:			gObjUseSkill.m_Lua.Generic_Call("EnergySkillBonus", "iii>i", lpObj->Class, AttackDamage, Energy, &AttackDamage);															break;	// Burning Echoes
							case 523:			gObjUseSkill.m_Lua.Generic_Call("EnergySkillBonus", "iii>i", lpObj->Class, AttackDamage, Energy, &AttackDamage);																	// Improved Darkness
								if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, iMLSBase) >= 10) {
									int DurationBonus = 0;

									if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_IMPROVED_DARKNESS)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_IMPROVED_DARKNESS);
									if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, iMLSBase) == 20) DurationBonus += 1;

									BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_IMPROVED_DARKNESS, 0, 25, 0, 0, 3 + DurationBonus);
								}
							break;

							case 462: {
								fPrevBase = g_MasterLevelSkillTreeSystem.GetBrandOfMasterSkillValue(lpObj, skill, 1);

								int nAddHP2 = 0;

								if (lpTargetObj->Type == OBJ_MONSTER) g_MasterLevelSkillTreeSystem.m_Lua.Generic_Call("SummonerDrainLife_Monster_Level2", "ii>i", Energy, lpTargetObj->Level, &nAddHP2);
								else if (lpTargetObj->Type == OBJ_USER) g_MasterLevelSkillTreeSystem.m_Lua.Generic_Call("SummonerDrainLife_PvP_Level2", "ii>i", Energy, AttackDamage, &nAddHP2);

								int tmpLife2 = lpObj->Life + nAddHP2;

								if ((lpObj->AddLife + lpObj->MaxLife) >= tmpLife2) lpObj->Life = tmpLife2;
								else lpObj->Life = lpObj->AddLife + lpObj->MaxLife;

								GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);

								if (CHARACTER_DebuffProc(30)) {
									BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_SIPHON_LIFE, EFFECTTYPE_DOT_VAMPIRIC, fSkillValue, 0, lpObj->m_Index, 5);
									lpTargetObj->lpAttackObj = lpObj;
								}
							}
							break;

							case 454: {
								int iSuccessRate = 0;
								int iEffectTime = 0;

								if (lpTargetObj->Type == OBJ_MONSTER) g_MasterLevelSkillTreeSystem.m_Lua.Generic_Call("Sleep_Monster", "iii>ii", Energy, lpObj->m_CurseSpell, lpTargetObj->Level, &iSuccessRate, &iEffectTime);
								else if (lpTargetObj->Type == OBJ_USER) g_MasterLevelSkillTreeSystem.m_Lua.Generic_Call("Sleep_PvP", "iiii>ii", Energy, lpObj->m_CurseSpell, lpObj->Level, lpTargetObj->Level, &iSuccessRate, &iEffectTime);
								if (!CHARACTER_DebuffProc(iSuccessRate + fSkillValue)) {
									GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
									return FALSE;
								}

								if (iEffectTime < 1) {
									GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
									return FALSE;
								}

								if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 524) >= 10 && BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_IMPROVED_STONESKIN)) {
									GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
									return FALSE;
								}

								if (BUFF_IsStrongerBuff(lpTargetObj, DEBUFFTYPE_SLEEP, iEffectTime, 0)) {
									GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, 219, lpTargetObj->m_Index, FALSE);
									return FALSE;
								} else {
									BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_SLEEP, EFFECTTYPE_SLEEP, iEffectTime, 0, 0, iEffectTime);
									return TRUE;
								}
							}
							break;

							case 455:			g_MasterLevelSkillTreeSystem.m_Lua.Generic_Call("ChainLightning_Master_Summoner", "ii>i", AttackDamage, bCombo, &AttackDamage);
												bCombo = 0;																																									break;
							case 456:			BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_LIGHTNINGSHOCK_VISUAL, 0, 0, 0, 0, 1);																							break;
							case 458: {
								int nAddHP = 0;

								if (lpTargetObj->Type == OBJ_MONSTER) g_MasterLevelSkillTreeSystem.m_Lua.Generic_Call("SummonerDrainLife_Monster_Level1", "ii>i", Energy, lpTargetObj->Level, &nAddHP);
								else if (lpTargetObj->Type == OBJ_USER) g_MasterLevelSkillTreeSystem.m_Lua.Generic_Call("SummonerDrainLife_PvP_Level1", "ii>i", Energy, AttackDamage, &nAddHP);

								int tmpLife = lpObj->Life + nAddHP;

								if ((lpObj->AddLife + lpObj->MaxLife) >= tmpLife) lpObj->Life = tmpLife;
								else lpObj->Life = lpObj->AddLife + lpObj->MaxLife;

								GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
							}
							break;

							default:																																														break;
						}
					}
				}
			}
			// Mounts/Companions/Wings Effects
			if (COMPANION_Satan(lpObj)) {
				if (lpObj->Class != CLASS_RAGEFIGHTER) lpObj->Life -= 3.0f;
				else {
					if (skill == 263 || skill == 559 || skill == 563 || skill == 269 || skill == 262 || skill == 558 || skill == 562) lpObj->Life -= 4.0f;
					else if (skill == 265 || skill == 564 || skill == 566) lpObj->Life -= 100.0f;
					else if (skill == 260 || skill == 261 || skill == 264 || skill == 551 || skill == 554 || skill == 552 || skill == 555 || skill == 560 || skill == 561 || skill == 270) lpObj->Life -= 2.0f;
					else lpObj->Life -= 3.0f;
				}

				if (lpObj->Life < 0.0f) lpObj->Life = 0.0f;
				else itemIncPercent += g_ConfigRead.pet.SatanAddDamage / 100.00; //AttackDamage += AttackDamage * g_ConfigRead.pet.SatanAddDamage / 100;

				GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
			} else if (COMPANION_Demon(lpObj)) {
				if (lpObj->Class != CLASS_RAGEFIGHTER) lpObj->Life -= 4.0f;
				else {
					if (skill == 263 || skill == 559 || skill == 563 || skill == 269 || skill == 262 || skill == 558 || skill == 562) lpObj->Life -= 5.0f;
					else if (skill == 265 || skill == 564 || skill == 566) lpObj->Life -= 100.0f;
					else if (skill == 260 || skill == 261 || skill == 264 || skill == 551 || skill == 554 || skill == 552 || skill == 555 || skill == 560 || skill == 561 || skill == 270) lpObj->Life -= 3.0f;
					else lpObj->Life -= 4.0f;
				}

				if (lpObj->Life < 0.0f) lpObj->Life = 0.0f;
				else itemIncPercent += g_ConfigRead.pet.DemonAddDamage / 100.00; //AttackDamage += AttackDamage * g_ConfigRead.pet.DemonAddDamage / 100;

				GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
			}

			if (COMPANION_GuardianAngel(lpTargetObj)) {
				if (AttackDamage > 1) itemDecPercent -= g_ConfigRead.pet.AngelAddDefense / 100.00; //AttackDamage -= AttackDamage * g_ConfigRead.pet.AngelAddDefense / 100;
			} else if (COMPANION_GuardianSpirit(lpTargetObj)) {
				if (AttackDamage > 1) itemDecPercent -= g_ConfigRead.pet.SafeGuardAddDefense / 100.00; //AttackDamage -= AttackDamage * g_ConfigRead.pet.SafeGuardAddDefense / 100;
			}

			if (CHECK_Wings(lpObj)) {
				CItem* Wing = &lpObj->pInventory[7];
				int LifeDec = 0;
				int percent = 0;

				g_ConfigRead.m_ItemCalcLua.Generic_Call("Wings_DecreaseHP", "iiii>i", Wing->m_Type, (int)Wing->m_Durability, lpObj->Class, skill, &LifeDec);
				lpObj->Life -= LifeDec;

				if (lpObj->Life < 0.0f) lpObj->Life = 0.0f;
				else g_ConfigRead.m_ItemCalcLua.Generic_Call("Wings_CalcIncAttack", "ii>i", Wing->m_Type, Wing->m_Level, &percent);

				wingsIncPercent += percent / 100.00;
				GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
			}

			if (lpTargetObj->Type == OBJ_USER) {
				CItem* Wing = &lpTargetObj->pInventory[7];
				float fHPRecoveryRate = 0.0;
				float fManaRecoveryRate = 0.0;

				if (AttackDamage > 1) {
					double WingDamageBlock;
					int percent = 0;

					if (lpTargetObj->CharacterData->Mastery.iMpsAddWingDamageBlock <= 0.0) WingDamageBlock = 0.0;
					else WingDamageBlock = lpTargetObj->CharacterData->Mastery.iMpsAddWingDamageBlock;

					g_ConfigRead.m_ItemCalcLua.Generic_Call("Wings_CalcAbsorb", "iid>i", Wing->m_Type, Wing->m_Level, WingDamageBlock, &percent);
					wingsDecPercent -= percent / 100.00;
				}

				if (CHECK_Wings(lpTargetObj)) {
					if (lpTargetObj->CharacterData->m_WingExcOption.iWingOpRecoveryHP != 0) fHPRecoveryRate = lpTargetObj->CharacterData->m_WingExcOption.iWingOpRecoveryHP;
					if (lpTargetObj->CharacterData->m_WingExcOption.iWingOpRecoveryMana != 0) fManaRecoveryRate = lpTargetObj->CharacterData->m_WingExcOption.iWingOpRecoveryMana;
				}

				if (lpTargetObj->Class == CLASS_RAGEFIGHTER) {
					if (lpTargetObj->CharacterData->Mastery.ML_RF_LastStand > 0.0) fHPRecoveryRate += lpTargetObj->CharacterData->Mastery.ML_RF_LastStand;
				} else if (lpTargetObj->CharacterData->Mastery.ML_GENERAL_LastStand > 0.0) fHPRecoveryRate += lpTargetObj->CharacterData->Mastery.ML_GENERAL_LastStand;
				if (rand() % 100 < fHPRecoveryRate) gObjAddMsgSendDelay(lpTargetObj, 13, lpTargetObj->m_Index, 100, 0);
				if (lpTargetObj->Class == CLASS_RAGEFIGHTER) {
					if (lpTargetObj->CharacterData->Mastery.ML_RF_ArcaneRecovery > 0.0) fManaRecoveryRate += lpTargetObj->CharacterData->Mastery.ML_RF_ArcaneRecovery;
				} else if (lpTargetObj->CharacterData->Mastery.ML_GENERAL_ArcaneRecovery > 0.0) fManaRecoveryRate += lpTargetObj->CharacterData->Mastery.ML_GENERAL_ArcaneRecovery;
				if (rand() % 100 < fManaRecoveryRate) gObjAddMsgSendDelay(lpTargetObj, 14, lpTargetObj->m_Index, 100, 0);
				if (lpObj->Type == OBJ_USER && lpObj->Class == CLASS_RAGEFIGHTER) {
					if (lpObj->CharacterData->Mastery.ML_RF_Overpower > 0.0) {
						if (rand() % 100 < lpObj->CharacterData->Mastery.ML_RF_Overpower) gObjAddMsgSendDelay(lpObj, 15, lpTargetObj->m_Index, 100, 0);
					}
				} else if (lpObj->Type == OBJ_USER && lpObj->CharacterData->Mastery.ML_GENERAL_Overpower > 0.0) {
					if (rand() % 100 < lpObj->CharacterData->Mastery.ML_GENERAL_Overpower) gObjAddMsgSendDelay(lpObj, 15, lpTargetObj->m_Index, 100, 0);
				}
			}

			if (CHECK_Dinorant(lpObj)) {
				lpObj->Life -= 1.0f;

				if (lpObj->Life < 0.0f) lpObj->Life = 0.0f;
				else itemIncPercent += 0.15; //AttackDamage = AttackDamage * 115 / 100;

				GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
			}

			// Dinorant Normal Attack
			if (skill == 0 && lpObj->pInventory[lpObj->m_btInvenPetPos].m_Type == ITEMGET(13, 3)) itemIncPercent += 0.3;	//AttackDamage = AttackDamage * 130 / 100;
			if (CHECK_Dinorant(lpTargetObj)) {
				CItem* Dinorant = &lpTargetObj->pInventory[lpTargetObj->m_btInvenPetPos];
				int dinorantdecdamage = 10 + Dinorant->IsDinorantReduceAttackDamaege();

				lpObj->Life -= 1.0f;

				if (lpObj->Life < 0.0f) lpObj->Life = 0.0f;
				else itemDecPercent -= dinorantdecdamage / 100.00; //AttackDamage = AttackDamage * dinorantdecdamage / 100;

				GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
			}

			if (CHECK_DarkHorse(lpTargetObj)) {
				CItem* Darkhorse = &lpTargetObj->pInventory[lpTargetObj->m_btInvenPetPos];
				int decdamage = ((Darkhorse->m_PetItem_Level + 30) / 2);

				lpTargetObj->Life -= 1.0f;

				if (lpTargetObj->Life < 0.0f) lpTargetObj->Life = 0.0f;
				else itemDecPercent -= decdamage / 100.00; //AttackDamage = AttackDamage * decdamage / 100;

				GSProtocol.PROTOCOL_ReFillSend(lpTargetObj->m_Index, lpTargetObj->Life, 0xFF, 0, lpTargetObj->iShield);
			}

			if (CHECK_Fenrir(lpObj)) {
				int iIncPercent = 0;

				iIncPercent += lpObj->pInventory[lpObj->m_btInvenPetPos].IsFenrirIncLastAttackDamage();
				iIncPercent += lpObj->pInventory[lpObj->m_btInvenPetPos].IsFenrirIllusion();

				if (iIncPercent > 0) itemIncPercent += iIncPercent / 100.00;	//AttackDamage += AttackDamage * iIncPercent / 100;
			}

			if (CHECK_Fenrir(lpTargetObj)) {
				int iDecPercent = 0;

				iDecPercent += lpTargetObj->pInventory[lpTargetObj->m_btInvenPetPos].IsFenrirDecLastAttackDamage();
				iDecPercent += lpTargetObj->pInventory[lpTargetObj->m_btInvenPetPos].IsFenrirIllusionDecDmg();

				if (iDecPercent > 0) itemDecPercent -= iDecPercent / 100.00;	//AttackDamage -= AttackDamage * iDecPercent / 100;
			}

			if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_TEMPLE_PROTECTION)) {
				AttackDamage = 0;
				GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
				return FALSE;
			}

			if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_FOCUS)) {
				int Value = 0;

				BUFF_GetBuffEffectValue(lpObj, BUFFTYPE_FOCUS, NULL, &Value);
				lethalBonus += Value;
			}

			// Elemental Seal Debuffs
			if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_FIRE) && skillAttr == R_FIRE) {
				int value = 0;

				BUFF_GetBuffEffectValue(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_FIRE, &value, NULL);
				buffIncPercent += value / 100.00;		//AttackDamage += AttackDamage * value / 100;
			}

			if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_WATER) && skillAttr == R_LIGHTNING) {
				int value = 0;

				BUFF_GetBuffEffectValue(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_WATER, &value, NULL);
				buffIncPercent += value / 100.00;		//AttackDamage += AttackDamage * value / 100;
			}

			// Improved Earthen Prison Debuff
			if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_EARTHEN_PRISON) && skillAttr == R_EARTH) {
				int value = 0;

				BUFF_GetBuffEffectValue(lpTargetObj, DEBUFFTYPE_EARTHEN_PRISON, &value, NULL);
				buffIncPercent += value / 100.00;		//AttackDamage += AttackDamage * value / 100;
			}

			if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_ETERNAL_ARROW)) buffIncPercent += lpObj->m_SkillInfo.ML_EternalArrow / 100.00;	//AttackDamage += AttackDamage * lpObj->m_SkillInfo.ML_EternalArrow / 100;

			if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_ARCA_FIRETOWER)) {
				int iArcaEffect = 0;

				BUFF_GetBuffEffectValue(lpObj, BUFFTYPE_ARCA_FIRETOWER, &iArcaEffect, 0);
				AttackDamage += iArcaEffect;
			}

			if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_ARCA_WINDTOWER)) {
				int iArcaEffect = 0;

				BUFF_GetBuffEffectValue(lpObj, BUFFTYPE_ARCA_WINDTOWER, &iArcaEffect, NULL);
				AttackDamage += iArcaEffect;
			}

			if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_ARCA_DARKNESSTOWER)) {
				int iArcaEffect = 0;

				BUFF_GetBuffEffectValue(lpTargetObj, BUFFTYPE_ARCA_DARKNESSTOWER, &iArcaEffect, NULL);
				buffDecPercent -= iArcaEffect / 100.00;
			}

			if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) {
				if (CC_MAP_RANGE(lpObj->MapNumber) && CC_MAP_RANGE(lpTargetObj->MapNumber)) AttackDamage = AttackDamage * 50 / 100;
				if (lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL && lpTargetObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) AttackDamage = AttackDamage * 50 / 100;
			}

			if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE) {
				if (g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) {
					if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) {
						if (lpObj->MapNumber == MAP_INDEX_CASTLESIEGE && lpTargetObj->MapNumber == MAP_INDEX_CASTLESIEGE) {
							if (lpObj->m_btCsJoinSide == lpTargetObj->m_btCsJoinSide) {
								if (g_CastleSiege.m_bCastleSiegeFriendlyFire == false) AttackDamage = 0;
								else AttackDamage = AttackDamage * g_CastleSiege.CastleSiegeSelfDmgReductionPercent / 100;
							} else AttackDamage = AttackDamage * g_CastleSiege.CastleSiegeDmgReductionPercent / 100;
						}
					}
				}
			}

			if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_MONSTER) {
				if (lpTargetObj->Class == 283) {
					if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_BLESS_POTION)) buffIncPercent += 0.20; //AttackDamage += (AttackDamage * 20) / 100;
					else if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_SOUL_POTION));
					else {
						if (lpObj->m_iAccumulatedDamage > 100) {
							gObjWeaponDurDownInCastle(lpObj, lpTargetObj, 1);
							lpObj->m_iAccumulatedDamage = 0;
						} else lpObj->m_iAccumulatedDamage += AttackDamage;

						AttackDamage = AttackDamage * 0.05;
					}
				}

				if (lpTargetObj->Class == 277) {
					if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_BLESS_POTION)) buffIncPercent += 0.20; //AttackDamage += (AttackDamage * 20) / 100;
					else if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_SOUL_POTION));
					else {
						if (lpObj->m_iAccumulatedDamage > 100) {
							gObjWeaponDurDownInCastle(lpObj, lpTargetObj, 1);
							lpObj->m_iAccumulatedDamage = 0;
						} else lpObj->m_iAccumulatedDamage += AttackDamage;

						AttackDamage = AttackDamage * 0.05;
					}
				}
			}

			if (bIsOnDuel == TRUE) AttackDamage = AttackDamage * g_NewPVP.m_iDuelDamageReduction / 100;
			if (g_GensSystem.IsMapBattleZone(lpObj->MapNumber) && g_GensSystem.IsMapBattleZone(lpTargetObj->MapNumber)) {
				if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) {
					if (g_GensSystem.IsPkEnable(lpObj, lpTargetObj) == true) AttackDamage = AttackDamage * g_GensSystem.GetDamageReduction() / 100;
				}
			}
			
			if (AttackDamage < 0) AttackDamage = 0;
			if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE) {
				if (g_CastleSiege.GetCastleState() != CASTLESIEGE_STATE_STARTSIEGE) {
					if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_MONSTER) {
						if (lpObj->MapNumber == MAP_INDEX_CASTLESIEGE && lpTargetObj->MapNumber == MAP_INDEX_CASTLESIEGE) {
							if (lpTargetObj->Class == 277 || lpTargetObj->Class == 283) return FALSE;
						}
					}
				}
			}

			// Shield Expertise Mastery
			if (lpTargetObj->pInventory[0].GetDetailItemType() == ITEM_SHIELD || lpTargetObj->pInventory[1].GetDetailItemType() == ITEM_SHIELD) {
				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 402) >= 10 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 440) >= 10 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 532) >= 10) {
					if (rand() % 10 == TRUE) masteryDecPercent -= 0.25;		//AttackDamage *= 0.75;
				}
			}

			if (lpTargetObj->m_SkillNumber == 18) {
				int value = 0;

				if (AttackDamage > 1) gObjUseSkill.m_Lua.Generic_Call("Defend", ">i", &value);

				globalDecPercent -= value / 100.00;
			}

			if (MsgDamage == 2) {
				if (iFenrirAttackDmg > 0) AttackDamage = iFenrirAttackDmg + lpMagic->m_DamageMax - targetdefense;

				globalIncPercent += 0.2;
				AttackDamage += lpObj->CharacterData->ANCIENT_ExcellentDamage;
			}

			AttackDamage *= charIncPercent * charDecPercent * masteryIncPercent * masteryDecPercent * wingsIncPercent * wingsDecPercent * itemIncPercent * itemDecPercent * buffIncPercent * buffDecPercent * globalIncPercent * globalDecPercent;

			if (bCombo) {
				int iComboDamage = 0;

				gObjUseSkill.m_Lua.Generic_Call("ComboSkillDamage", "iiii>i", Strength, Dexterity, Vitality, Energy, &iComboDamage);

				if (lpObj->CharacterData->Mastery.ML_DK_SkilledFighter > 0.0) iComboDamage += iComboDamage * lpObj->CharacterData->Mastery.ML_DK_SkilledFighter / 100.0;

				AttackDamage += iComboDamage;
				comboSkill = skill;
				skill = 59;
				MsgDamage |= 0x80;
			}

			if (lpObj->Type == OBJ_USER) {
				float fRate;
				int Resist = 0;

				if (lpObj->Class == CLASS_RAGEFIGHTER) fRate = lpObj->CharacterData->ANCIENT_LethalStrikeChance + lpObj->CharacterData->Mastery.ML_DK_SpearExpertise + lpObj->CharacterData->Mastery.ML_RF_LethalPunches + lpObj->CharacterData->Mastery.ML_RF_LethalBlows;
				else fRate = lpObj->CharacterData->ANCIENT_LethalStrikeChance + lpObj->CharacterData->Mastery.ML_DK_SpearExpertise + lpObj->CharacterData->Mastery.ML_GENERAL_LethalBlows;
				if (CHECK_Wings(lpObj)) {
					if (lpObj->pInventory[7].m_Type == ITEMGET(12, 267)) fRate += 4.0;
				}

				fRate += lethalBonus;

				if (lpTargetObj->Type == OBJ_USER) Resist = lpTargetObj->CharacterData->C_LethalStrikeResistance;
				if (!CHARACTER_PvPResistanceProc(Resist)) {
					if (CHARACTER_DebuffProc(fRate)) {
						AttackDamage *= 2;
						MsgDamage |= 0x40;
					}
				}
			}

			// Mana Absorbtion Buffs
				// Ring of Magic Implicit
			int manaDmg = NULL;

			if (lpTargetObj->Type == OBJ_USER) {
				CItem* RightRing = &lpTargetObj->pInventory[10];
				CItem* LeftRing = &lpTargetObj->pInventory[11];

				if (RightRing->IsItem() && RightRing->m_Type == ITEMGET(13, 24)) {
					manaDmg += AttackDamage * 0.05;
					ManaChange = TRUE;
				}

				if (LeftRing->IsItem() && LeftRing->m_Type == ITEMGET(13, 24)) {
					manaDmg += AttackDamage * 0.05;
					ManaChange = TRUE;
				}
			}

			// Mana Shield
			if (lpTargetObj->m_SkillInfo.ManaShieldAbsorbPercent > 0) {
				manaDmg += AttackDamage * lpTargetObj->m_SkillInfo.ManaShieldAbsorbPercent / 100;
				ManaChange = TRUE;
			}

			if (ManaChange) {
				if (manaDmg < lpTargetObj->Mana) {
					AttackDamage -= manaDmg;
					lpTargetObj->Mana -= manaDmg;
				} else {
					manaDmg = lpTargetObj->Mana;
					lpTargetObj->Mana = 0;
					AttackDamage -= manaDmg;
				}
			}

			if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_MONSTER) AttackDamage = AttackDamage * g_ConfigRead.calc.PvMDamageRate[lpObj->Class] / 100;
			if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) {
				AttackDamage = AttackDamage * g_ConfigRead.calc.PvPDamageRate[lpObj->Class][lpTargetObj->Class] / 100;

				if (skill == 265 || skill == 564 || skill == 566) {
					int nSuccessRate = 0, nDecreaseSDRate = 0;

					gObjUseSkill.m_Lua.Generic_Call("DragonFist_ShieldPenetration", "i>ii", Energy, &nSuccessRate, &nDecreaseSDRate);

					if (rand() % 100 < nSuccessRate) {
						bDragonKickSDAttackSuccess = TRUE;

						int iDragonKickSDDamage = nDecreaseSDRate * lpTargetObj->iShield / 100;

						lpTargetObj->iShield -= iDragonKickSDDamage;
						lpTargetObj->Life -= AttackDamage;
						iTotalShieldDamage += iDragonKickSDDamage;
					} else {
						iTempShieldDamage = this->ATTACK_CalcShieldDamage(lpObj, lpTargetObj, AttackDamage);
						lpTargetObj->iShield -= iTempShieldDamage;
						lpTargetObj->Life -= AttackDamage - iTempShieldDamage;
						iTotalShieldDamage += iTempShieldDamage;
					}
				} else {
					iTempShieldDamage = this->ATTACK_CalcShieldDamage(lpObj, lpTargetObj, AttackDamage);
					lpTargetObj->iShield -= iTempShieldDamage;
					lpTargetObj->Life -= AttackDamage - iTempShieldDamage;
					iTotalShieldDamage += iTempShieldDamage;
				}

				if (lpTargetObj->Life < 0.0f) lpTargetObj->Life = 0.0f;
			} else {
				if (lpObj->m_RecallMon >= 0 && lpCallObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) {
					if (AttackDamage > lpTargetObj->iShield) {
						iTempShieldDamage = AttackDamage;
						iTempShieldDamage -= lpTargetObj->iShield;
						lpTargetObj->iShield = 0;
						lpTargetObj->Life -= iTempShieldDamage;
						iTotalShieldDamage += iTempShieldDamage;
					} else {
						lpTargetObj->iShield -= AttackDamage;
						iTotalShieldDamage += AttackDamage;
					}
				} else lpTargetObj->Life -= AttackDamage;

				if (lpTargetObj->Life < 0.0f) lpTargetObj->Life = 0.0f;
			}
		}
	}

	lpCallObj = lpTargetObj;

	if (lpTargetObj->Type == OBJ_MONSTER) {
		gObjAddMsgSendDelay(lpTargetObj, 0, lpObj->m_Index, 100, 0);
		lpTargetObj->LastAttackerID = lpObj->m_Index;

		if (lpTargetObj->m_iCurrentAI) lpTargetObj->m_Agro->IncAgro(lpObj->m_Index, AttackDamage / 50);
		if (lpTargetObj->m_RecallMon >= 0) lpCallObj = &gObj[lpTargetObj->m_RecallMon];
	}

	BOOL selfdefense = FALSE;

	if (AttackDamage >= 1) {
		if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) {
			if (gObjDuelCheck(lpObj, lpTargetObj)) selfdefense = FALSE;
			else if (lpObj->CharacterData->RegisterdLMS == 1 && lpTargetObj->CharacterData->RegisterdLMS == 1) {
				if (lpObj->MapNumber == g_LastManStanding.m_Cfg.iPVPMap) {
					if (g_LastManStanding.m_Rooms[lpObj->CharacterData->RegisteredLMSRoom].bState == 3) selfdefense = FALSE;
				}
			} else if (CC_MAP_RANGE(lpObj->MapNumber) || CC_MAP_RANGE(lpTargetObj->MapNumber)) selfdefense = FALSE;
			else if (lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL || lpTargetObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) selfdefense = FALSE;
			else if (IT_MAP_RANGE(lpObj->MapNumber) || IT_MAP_RANGE(lpTargetObj->MapNumber)) selfdefense = FALSE;
			else if (g_GensSystem.IsMapBattleZone(lpObj->MapNumber) && g_GensSystem.IsMapBattleZone(lpTargetObj->MapNumber)) {
				selfdefense = FALSE;

				if (byReflect == false) lpObj->CharacterData->m_dwGensHitTeleportDelay = GetTickCount();
			} else if (g_ArcaBattle.IsArcaBattleServer() == TRUE) selfdefense = FALSE;
			else selfdefense = TRUE;

			if (gObjGetRelationShip(lpObj, lpTargetObj) == 2) selfdefense = FALSE;
			if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE) {
				if (g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) {
					if (lpObj->m_btCsJoinSide > 0) selfdefense = FALSE;
				}
			}
		} else if (lpTargetObj->Type == OBJ_MONSTER && lpObj->Type == OBJ_USER) {
				if (lpTargetObj->m_RecallMon >= 0) {
					if (!IT_MAP_RANGE(lpObj->MapNumber)) selfdefense = TRUE;
				}

				if (lpTargetObj->Class >= 678 && lpTargetObj->Class == 680) selfdefense = FALSE;
			}

		if (lpTargetObj->Type == OBJ_USER) {
			gObjArmorRandomDurDown(lpTargetObj, lpObj);

			if (skill == 333 || ((skill == 416 || skill == 490) && g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, skill) == 20)) {
				float fDurDownRate;

				if (skill == 333) fDurDownRate = g_MasterLevelSkillTreeSystem.ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
				else fDurDownRate = 10;

				if (rand() % 100 < fDurDownRate) {
					if (g_ConfigRead.server.GetServerType() != SERVER_CASTLE || (lpObj->m_btCsJoinSide == 0 || lpObj->m_btCsJoinSide != lpTargetObj->m_btCsJoinSide)) {
						int item_num[5];

						item_num[0] = 2;
						item_num[1] = 3;
						item_num[2] = 4;
						item_num[3] = 5;
						item_num[4] = 6;

						int iEquipmentPos = rand() % 5;
						CItem* lpEquipment = &lpTargetObj->pInventory[item_num[iEquipmentPos]];

						if (lpEquipment && lpEquipment->IsItem()) {
							float iDurDecValue = 1.0;

							if (lpTargetObj->Class == CLASS_RAGEFIGHTER && lpTargetObj->CharacterData->Mastery.ML_RF_Efficiency > 0.0) iDurDecValue += lpTargetObj->CharacterData->Mastery.ML_RF_EfficiencyLevel / 5.0;
							else if (lpTargetObj->CharacterData->Mastery.ML_GENERAL_Efficiency > 0.0) iDurDecValue += lpTargetObj->CharacterData->Mastery.ML_GENERAL_EfficiencyLevel / 5.0;

							int damagemin = lpEquipment->m_BaseDurability / 100.0 * (10 - iDurDecValue);
							int iDurEquipment = lpEquipment->m_Durability - damagemin;

							for (int iBuffIndex = 0; iBuffIndex < lpTargetObj->m_BuffEffectCount; iBuffIndex++) {
								if (lpTargetObj->m_BuffEffectList[iBuffIndex].BuffIndex == BUFFTYPE_CHARM_PROTECTITEM) {
									iDurEquipment = lpEquipment->m_Durability;
									break;
								}
							}

							if (g_LuckyItemManager.IsLuckyItemEquipment(lpEquipment->m_Type)) iDurEquipment = lpEquipment->m_Durability;

							lpEquipment->m_Durability = (float)iDurEquipment;

							if (lpEquipment->m_Durability < 0.0f) lpEquipment->m_Durability = 0.0f;

							GSProtocol.GCItemDurSend(lpTargetObj->m_Index, item_num[iEquipmentPos], (BYTE)lpEquipment->m_Durability, 0);
						}
					}
				}
			}
		}

		if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_ICE_ARROW)) {
			BUFF_ChangeBuffTime(lpTargetObj, DEBUFFTYPE_ICE_ARROW, -1);

			if (!BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_ICE_ARROW)) {
				BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_ICE_ARROW);
				GSProtocol.PROTOCOL_MagicCancelSend(lpTargetObj, 51);
				GSProtocol.PROTOCOL_MagicCancelSend(lpTargetObj, 424);
			}
		}

		if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_SLEEP)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_SLEEP);
	}

	if (selfdefense == TRUE && bDamageReflect == FALSE) {
		if (!gObjTargetGuildWarCheck(lpObj, lpCallObj)) {
			if (lpCallObj->PartyNumber >= 0) {
				int number = 0;
				int partynum = lpCallObj->PartyNumber;

				if ((gParty.GetPKPartyPenalty(partynum)) < 5) CHARACTER_CheckSelfDefense(lpObj, lpCallObj->m_Index);
			} else CHARACTER_CheckSelfDefense(lpObj, lpCallObj->m_Index);

			CHARACTER_CheckSelfDefense(lpObj, lpCallObj->m_Index);
		}
	}

	if (lpTargetObj->Class == 275) {
		if (lpTargetObj->m_iMonsterBattleDelay <= 0) {
			if ((rand() % 15) < 1) {
				gObjAddMsgSendDelay(lpTargetObj, 4, lpObj->m_Index, 100, 0);
				lpTargetObj->m_iMonsterBattleDelay = 10;
				GSProtocol.PROTOCOL_ActionSend(lpTargetObj, 126, lpTargetObj->m_Index, lpObj->m_Index);
			}
		}
	}

	if (lpTargetObj->Class == 131 || BC_STATUE_RANGE(lpTargetObj->Class - 132)) {
		gObjAddMsgSendDelay(lpTargetObj, 4, lpObj->m_Index, 100, 0);
		CHARACTER_Immobilize(lpTargetObj->m_Index, lpTargetObj->X, lpTargetObj->Y);
	} else if (AttackDamage >= 5) {	// To make strong hit
		if (lpTargetObj->Type == OBJ_MONSTER) {
			if ((rand() % 26) == 0) gObjAddMsgSendDelay(lpTargetObj, 4, lpObj->m_Index, 100, 0);
		} else if ((rand() % 4) == 0) {
			if (!MOUNT_Uniria(lpTargetObj)) MSBFlag = 1;
		}
	}

	if (magicsend) GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, skill, lpTargetObj->m_Index, skillSuccess);
	if (bCombo) GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, DK_COMBO, lpTargetObj->m_Index, skillSuccess);

	if (lpObj->Type == OBJ_USER) {
		if (lpObj->m_Change == 9) GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, 3, lpTargetObj->m_Index, 1);
	}

	if (lpObj->Class == CLASS_ELF && lpObj->Level == 1 && AttackDamage > 10) g_Log.Add("error-Level1 : [%s][%s] Str:%d %d %d %d %d %d %d", lpObj->AccountID, lpObj->Name, Strength, lpObj->m_AttackDamageMinRight, lpObj->m_AttackDamageMaxRight, lpObj->m_AttackDamageMinLeft, lpObj->m_AttackDamageMaxLeft, lpObj->m_AttackDamageMax, lpObj->m_AttackDamageMin);

	lpObj->m_Rest = 0;

	if (lpObj->Class >= 504 && lpObj->Class <= 511 && BUFF_TargetHasBuff(lpObj, BUFFTYPE_AMPLIFY_DAMAGE) == TRUE) AttackDamage *= 2;
	if (lpTargetObj->Class >= 504 && lpTargetObj->Class <= 511 && BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_AMPLIFY_DAMAGE) == TRUE) AttackDamage /= 2;
	if (AttackDamage > 0) {
		int refdmg = lpTargetObj->EXC_ReflectDamageTaken;

		if (refdmg > g_ConfigRead.calc.ReflectDamage) refdmg = g_ConfigRead.calc.ReflectDamage;

		int atd_reflect = AttackDamage * refdmg / 100;

		if (atd_reflect > 0) gObjAddMsgSendDelay(lpTargetObj, 10, lpObj->m_Index, 10, atd_reflect);
		if (lpTargetObj->Type == OBJ_USER && lpTargetObj->CharacterData->m_WingExcOption.iWingOpReturnEnemyDamage != 0) {
			if (rand() % 100 < lpTargetObj->CharacterData->m_WingExcOption.iWingOpReturnEnemyDamage) {
				if (lpObj->Type == OBJ_MONSTER) gObjAddMsgSendDelay(lpTargetObj, 12, lpObj->m_Index, 10, lpObj->m_AttackDamageMax);
				else if (lpObj->Type == OBJ_USER) gObjAddMsgSendDelay(lpTargetObj, 12, lpObj->m_Index, 10, AttackDamage);
			}
		}

		if (lpTargetObj->Type == OBJ_USER) {
			float fRevengeRate = 0.0;

			if (lpTargetObj->Class == CLASS_RAGEFIGHTER) fRevengeRate = lpTargetObj->CharacterData->Mastery.ML_RF_Vengeance;
			else fRevengeRate = lpTargetObj->CharacterData->Mastery.ML_GENERAL_Vengeance;
			if (fRevengeRate > 0.0 && rand() % 100 <= fRevengeRate) {
				if (lpObj->Type == OBJ_MONSTER) gObjAddMsgSendDelay(lpTargetObj, 12, lpObj->m_Index, 10, lpObj->m_AttackDamageMax);
				else if (lpObj->Type == OBJ_USER) gObjAddMsgSendDelay(lpTargetObj, 12, lpObj->m_Index, 10, AttackDamage);
				return TRUE;
			}
		}

		if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_CIRCLE_SHIELD) || BUFF_TargetHasBuff(lpObj, BUFFTYPE_CIRCLE_SHIELD_STR) || BUFF_TargetHasBuff(lpObj, BUFFTYPE_CIRCLE_SHIELD_MAS)) {
			if (rand() % 100 < lpObj->m_SkillInfo.fCircleShieldRate) {
				int iDecreaseBP = BUFF_GetTotalBuffEffectValue(lpObj, EFFECTTYPE_AG_OPPONENT_DECREASE);

				lpTargetObj->Stamina -= iDecreaseBP;

				if (lpTargetObj->Stamina < 0) lpTargetObj->Stamina = 0;
			}
		}

		/*if (lpObj->Type == OBJ_USER) {
			float fTripleRate = 0.0;	//MsgDamage |= 0x100;

			if (lpObj->Class == CLASS_RAGEFIGHTER) fTripleRate = lpObj->CharacterData->Mastery.iMpsIncTripleDamageRate_Monk;
			else fTripleRate = lpObj->CharacterData->Mastery.iMpsIncTripleDamageRate;
		}*/

		if (g_ShieldSystemOn == FALSE) {
			if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) {
				if (CC_MAP_RANGE(lpObj->MapNumber) && CC_MAP_RANGE(lpTargetObj->MapNumber)) AttackDamage = AttackDamage * 50 / 100;
			}
		}

		if (RFAttack) {
			if (lpObj->Class == CLASS_RAGEFIGHTER) {
				if (skill == 261 || skill == 263 || skill == 552 || skill == 555 || skill == 559 || skill == 563) {
					if (RFAttack % 2) MsgDamage |= 0x10;
					else MsgDamage |= 0x20;
				} else {
					if (RFAttack % 4) MsgDamage |= 0x10;
					else MsgDamage |= 0x20;
				}
			} else if (lpObj->Class == CLASS_GROWLANCER) {
				if (skill == 276 || skill == 274 || skill == 277) {
					if (RFAttack % 2) MsgDamage |= 0x10;
					else MsgDamage |= 0x20;
				}
			}
		}

		// Life & Mana Leech (NEW)
		float LifeLeechMultiplier = 1;
		int ManaRec = 0;
		int StamRec = 0;
		int LifeLeech = 0;
		int ManaLeech = 0;
		int LifeRec = 0;

		if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_IMPROVED_BLOOD_LUST) && g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 567) >= 10) LifeLeechMultiplier += 0.25;
		if ((g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 334) == 20 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 600) == 20) && CHARACTER_DebuffProc(5)) LifeRec += (lpTargetObj->MaxLife + lpTargetObj->AddLife) * 0.1;
		if ((lpTargetObj->Life + LifeRec) >= (lpTargetObj->AddLife + lpTargetObj->MaxLife)) lpTargetObj->Life = lpTargetObj->AddLife + lpTargetObj->MaxLife;
		else lpTargetObj->Life += LifeRec;
		if (LifeRec > 0) GSProtocol.PROTOCOL_ReFillSend(lpTargetObj->m_Index, lpTargetObj->Life, 0xFF, 0, lpTargetObj->iShield);
		if ((g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 338) == 20 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 601) == 20) && CHARACTER_DebuffProc(5)) ManaRec += (lpObj->MaxMana + lpObj->AddMana) * 0.1;
		if ((g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 341) == 20 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 602) == 20) && CHARACTER_DebuffProc(5)) StamRec += (lpObj->MaxStamina + lpObj->AddStamina) * 0.1;
		if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_ARCANE_FOCUS) && g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 383) == 20) ManaLeech += AttackDamage * 0.01;
		if (COMPANION_Demon(lpObj)) LifeLeech += AttackDamage * 0.005;
		else if (COMPANION_GuardianSpirit(lpObj)) ManaLeech += AttackDamage * 0.01;
		else if (lpObj->pInventory[8].IsItem() == TRUE && lpObj->pInventory[8].m_Type == ITEMGET(13, 123) && lpObj->pInventory[8].m_Durability > 0.0) LifeLeech += AttackDamage * 0.0033;

			// Wings Implicit
		if (lpObj->pInventory[7].IsItem() && lpObj->pInventory[7].m_IsValidItem == true) {
			CItem* Wings = &lpObj->pInventory[7];

			if (Wings->m_Durability > 0.0f) {
				switch (Wings->m_Type) {
					case ITEMGET(12, 2):			LifeLeech += AttackDamage * 0.0025;						break;					// Devil Wings
					case ITEMGET(12, 1):			ManaLeech += AttackDamage * 0.005;						break;					// Angel Wings
					case ITEMGET(12, 5):			LifeLeech += AttackDamage * 0.005;						break;					// Dragon Wings
					case ITEMGET(12, 4):			ManaLeech += AttackDamage * 0.01;						break;					// Archangel Wings
					case ITEMGET(12, 6):			LifeLeech += AttackDamage * 0.0025;												// Dark Wings
													ManaLeech += AttackDamage * 0.005;						break;
					case ITEMGET(12, 49):			StamRec += 3;											break;					// Warrior's Cloak
					case ITEMGET(12, 36):			LifeLeech += AttackDamage * 0.0075;						break;					// Storm Wings
					case ITEMGET(12, 37):			ManaLeech += AttackDamage * 0.015;						break;					// Eternal Wings
					case ITEMGET(12, 39):			LifeLeech += AttackDamage * 0.00375;											// Ruined Wings
													ManaLeech += AttackDamage * 0.0075;						break;
					case ITEMGET(12, 50):			StamRec += 5;											break;					// Warlord's Cloak
					case ITEMGET(12, 262):			StamRec += 4;											break;					// Cloak of Death
					case ITEMGET(12, 263):			LifeLeech += AttackDamage * 0.005;												// Wings of Chaos
													ManaLeech += AttackDamage * 0.005;						break;
					case ITEMGET(12, 264):			LifeLeech += AttackDamage * 0.0025;												// Wings of Magic
													ManaLeech += AttackDamage * 0.005;						break;
				}
			}
		}

		if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_VOID_AFFINITY)) {
			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 472) == 20) ManaLeech += AttackDamage * 0.02;
		}

			// Fist Mastery
		CItem* Right = &lpObj->pInventory[0];
		CItem* Left = &lpObj->pInventory[1];

		if (Right->GetDetailItemType() == ITEM_RAGEFIGHTER_WEAPON || Left->GetDetailItemType() == ITEM_RAGEFIGHTER_WEAPON) {
			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 568) == 20) LifeLeech += AttackDamage * 0.0033;
		}

			// Mastery Skill Bonuses
		switch (skill) {
			case 487:				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, skill) == 20) LifeLeech += AttackDamage * 0.0025;															break;		// Haunting Spirits			
			case 482:				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, skill) >= 10) ManaLeech += AttackDamage * 0.005;															break;		// Improved Power Slash	
			case 342:				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, skill) == 20 && lpTargetObj->Life <= 0) LifeRec += (lpObj->MaxLife + lpObj->AddLife) * 0.1;					break;		// Death Blow
			case 346:				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, skill) >= 10) LifeLeech += AttackDamage * 0.1;																break;		// Improved Blade Storm
			case 414:				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, skill) == 20) LifeLeech += AttackDamage * 0.002;															break;		// Improved Multishot
			case 518:				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, skill) == 20) manaBurn += 0.1;																				break;		// Improved Multishot
		}

		LifeLeech *= LifeLeechMultiplier;
		manaBurn *= AttackDamage;

		if ((lpObj->Life + LifeLeech) >= (lpObj->AddLife + lpObj->MaxLife)) lpObj->Life = lpObj->AddLife + lpObj->MaxLife;
		else lpObj->Life += LifeLeech;
		if ((lpObj->Mana + ManaLeech + ManaRec) >= (lpObj->AddMana + lpObj->MaxMana)) lpObj->Mana = lpObj->AddMana + lpObj->MaxMana;
		else lpObj->Mana += ManaLeech + ManaRec;
		if ((lpTargetObj->Mana - manaBurn) >= 0) lpTargetObj->Mana -= manaBurn;
		else lpTargetObj->Mana = 0;
		if ((lpObj->Stamina + StamRec) >= (lpObj->AddStamina + lpObj->MaxStamina)) lpObj->Stamina = lpObj->AddStamina + lpObj->MaxStamina;
		else lpObj->Stamina += StamRec;

		// Elemental Resistance Check
		if (!bDamageReflect) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, skill, skillLevel, AttackDamage, comboSkill);
		if (g_ShieldSystemOn == TRUE && !bDragonKickSDAttackSuccess) AttackDamage -= iTotalShieldDamage;

		GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, lpObj->Mana, 255, 0, lpObj->Stamina);
		gObjLifeCheck(lpTargetObj, lpObj, AttackDamage, 0, MSBFlag, MsgDamage, skill, iTotalShieldDamage, 0);

		if (iTotalShieldDamage > 0) g_Log.Add("[PvP System] Victim:[%s][%s], Attacker:[%s][%s] - SD[%d] HP[%f] -> SD[%d] HP[%f]", lpTargetObj->AccountID, lpTargetObj->Name, lpObj->AccountID, lpObj->Name, lpTargetObj->iShield + iTotalShieldDamage, lpTargetObj->Life + AttackDamage, lpTargetObj->iShield, lpTargetObj->Life);
	} else {
		if (lpObj->Type != OBJ_MONSTER || lpObj->m_iPentagramMainAttribute <= 0) GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, MsgDamage, 0);
	}

	if (byPentagramAttack == TRUE && bDamageReflect == FALSE) this->ATTACK_ElementalSeal(lpObj, lpTargetObj, lpMagic, MSBFlag, MsgDamage, AttackDamage, iTargetDefense);
	if (lpObj->Life <= 0.0f && lpObj->Type == OBJ_USER) {
		if (lpObj->m_CheckLifeTime <= 0) {
			lpObj->lpAttackObj = lpTargetObj;

			if (lpTargetObj->Type == OBJ_USER) lpObj->m_bAttackerKilled = true;
			else lpObj->m_bAttackerKilled = false;

			lpObj->m_CheckLifeTime = 3;
		}
	}

	if (manaBurn || ManaChange) GSProtocol.PROTOCOL_ManaSend(lpTargetObj->m_Index, lpTargetObj->Mana, 255, 0, lpTargetObj->Stamina);
	if (lpMagic) gObjUseSkill.ML_BurningEchoes(lpObj, lpTargetObj, lpMagic, AttackDamage);
	if (g_ConfigRead.antihack.EnabledAntiRefTimeCheck && MSBFlag) {
		if (lpTargetObj->Type == OBJ_USER) lpTargetObj->CharacterData->m_dwMSBFlagAttackDisable = GetTickCount();
	}

	return TRUE;
}
BOOL CObjAttack::ATTACK_ElementalSeal(LPOBJ lpObj, LPOBJ lpTargetObj, CMagicInf* lpMagic, BYTE MSBFlag, int MsgDamage, int AttackDamage, int iTargetDefense) {
	bool bPentagramEquip = false;
	int PentagramAttackDamage = 0;
	char PentagramDamageType1;
	char PentagramDamageType2;

	if (lpObj->Type == OBJ_USER && g_PentagramSystem.IsPentagramItem(lpObj->pInventory[236].m_Type) && lpObj->pInventory[236].m_IsValidItem) {
		bPentagramEquip = TRUE;
		PentagramAttackDamage = this->ATTACK_CalcElementalDamage(lpObj, lpTargetObj, &PentagramDamageType1, &PentagramDamageType2, AttackDamage, iTargetDefense);
	} else if (lpObj->Type == OBJ_MONSTER && lpObj->m_iPentagramMainAttribute > 0) {
		bPentagramEquip = TRUE;
		PentagramAttackDamage = this->ATTACK_CalcElementalDamage(lpObj, lpTargetObj, &PentagramDamageType1, &PentagramDamageType2, AttackDamage, iTargetDefense);
	}

	if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE) {
		if (g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)	{
			if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) {
				if (lpObj->MapNumber == MAP_INDEX_CASTLESIEGE && lpTargetObj->MapNumber == MAP_INDEX_CASTLESIEGE) {
					if (lpObj->m_btCsJoinSide == lpTargetObj->m_btCsJoinSide) {
						if (g_CastleSiege.m_bCastleSiegeFriendlyFire == false) PentagramAttackDamage = 0;
					}
				}
			}
		}
	}

	if (PentagramAttackDamage > 0) {
		if (lpObj->Type == OBJ_USER) {
			// Runestone of Radiance (Rank 5)
			if (lpObj->CharacterData->m_PentagramOptions.m_iOnyx_5RankHalfValueSkillRate > 0) {
				if (rand() % 100 < lpObj->CharacterData->m_PentagramOptions.m_iOnyx_5RankHalfValueSkillRate) {
					int iDuration = 0;
					int iEffect = 0;

					switch (PentagramDamageType1) {
						case EL_FIRE:
							gObjUseSkill.m_Lua.Generic_Call("ElementalSealDebuffDuration", "ii>ii", BUFFTYPE_ELEMENTALSEAL_FIRE, (lpObj->Level + lpObj->CharacterData->MasterLevel), &iEffect, &iDuration);
							BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_FIRE, 0, iEffect, 0, 0, iDuration);
						break;

						case EL_WATER:
							gObjUseSkill.m_Lua.Generic_Call("ElementalSealDebuffDuration", "ii>ii", BUFFTYPE_ELEMENTALSEAL_WATER, (lpObj->Level + lpObj->CharacterData->MasterLevel), &iEffect, &iDuration);
							BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_WATER, 0, iEffect, 0, 0, iDuration);
						break;

						case EL_EARTH: 
							gObjUseSkill.m_Lua.Generic_Call("ElementalSealDebuffDuration", "ii>ii", BUFFTYPE_ELEMENTALSEAL_EARTH, (lpObj->Level + lpObj->CharacterData->MasterLevel), &iEffect, &iDuration);
							BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_EARTH, 0, iEffect, 0, 0, iDuration);
						break;

						case EL_WIND: {
							gObjUseSkill.m_Lua.Generic_Call("ElementalSealDebuffDuration", "ii>ii", BUFFTYPE_ELEMENTALSEAL_WIND, (lpObj->Level + lpObj->CharacterData->MasterLevel), &iEffect, &iDuration);
							BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_WIND, EFFECTTYPE_DECREASE_ATTACK_SPEED_PERCENT, iEffect, 0, 0, iDuration);
							gObjCalCharacter.CHARACTER_Calc(lpTargetObj->m_Index);
						}
						break;

						case EL_DARKNESS: 
							gObjUseSkill.m_Lua.Generic_Call("ElementalSealDebuffDuration", "ii>ii", BUFFTYPE_ELEMENTALSEAL_DARKNESS, (lpObj->Level + lpObj->CharacterData->MasterLevel), &iEffect, &iDuration);
							BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_DARKNESS, 0, iEffect, 0, 0, iDuration);
						break;
					}
				}
			}
		}

		if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_FIRE) && PentagramDamageType1 == EL_FIRE) {
			int value = 0;

			BUFF_GetBuffEffectValue(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_FIRE, &value, NULL);
			PentagramAttackDamage += PentagramAttackDamage * value / 100;
		}

		// Mastery Debuffs
		if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_EARTHEN_PRISON) && PentagramDamageType1 == EL_EARTH) {
			int value = 0;

			BUFF_GetBuffEffectValue(lpTargetObj, DEBUFFTYPE_EARTHEN_PRISON, &value, NULL);
			PentagramAttackDamage += PentagramAttackDamage * value / 100;
		}
	}

	LPOBJ lpCallObj = &gObj[lpObj->m_RecallMon];

	if (bPentagramEquip) {
		if (PentagramAttackDamage <= 0) {
			if (PentagramDamageType2) GSProtocol.PROTOCOL_ElementalDamageSend(lpObj->m_Index, lpTargetObj->m_Index, NULL, PentagramDamageType2);
			else GSProtocol.PROTOCOL_ElementalDamageSend(lpObj->m_Index, lpTargetObj->m_Index, NULL, PentagramDamageType1);
		} else {
			if ((lpObj->Type == OBJ_USER || (lpObj->m_RecallMon != -1 && lpCallObj->Type == OBJ_USER)) && lpTargetObj->Type == OBJ_USER) {
				if (g_ConfigRead.calc.ElementalDamageAffectSDOnly == false) lpTargetObj->Life -= PentagramAttackDamage;					
				else {
					if (lpTargetObj->iShield > 0) {
						if (lpTargetObj->iShield < PentagramAttackDamage) {
							PentagramAttackDamage -= lpTargetObj->iShield;
							lpTargetObj->iShield = 0;
							lpTargetObj->Life -= PentagramAttackDamage;
						} else lpTargetObj->iShield -= PentagramAttackDamage;
					} else lpTargetObj->Life -= PentagramAttackDamage;
				}
			} else lpTargetObj->Life -= PentagramAttackDamage;

			if (lpTargetObj->Life < 0.0) lpTargetObj->Life = 0.0;
			if (lpTargetObj->Type == OBJ_USER) GSProtocol.PROTOCOL_ReFillSend(lpTargetObj->m_Index, lpTargetObj->Life, 0xFF, 0, lpTargetObj->iShield);
			if (PentagramDamageType2) gObjLifeCheck(lpTargetObj, lpObj, PentagramAttackDamage, 0, MSBFlag, PentagramDamageType2, 0, 0, TRUE);
			else gObjLifeCheck(lpTargetObj, lpObj, PentagramAttackDamage, 0, MSBFlag, PentagramDamageType1, 0, 0, TRUE);
		}
	}

	return TRUE;
}

// Companions Check
BOOL CHECK_Dinorant(LPOBJ lpObj) {
	if (lpObj->Type != OBJ_USER) return FALSE;
	if (lpObj->m_btInvenPetPos == 0 || lpObj->m_wInvenPet == (WORD) - 1) return FALSE;

	CItem * Dinorant = & lpObj->pInventory[lpObj->m_btInvenPetPos];

	if (Dinorant->m_Type == ITEMGET(13, 3)) {
		if (Dinorant->m_Durability > 0.0f) return TRUE;
	}

	return FALSE;
}
BOOL CHECK_DarkHorse(LPOBJ lpObj) {
	if (lpObj->Type != OBJ_USER) return FALSE;
	if (lpObj->m_btInvenPetPos == 0 || lpObj->m_wInvenPet == (WORD) - 1) return FALSE;

	CItem * Horse = & lpObj->pInventory[lpObj->m_btInvenPetPos];

	if (Horse->m_Type == ITEMGET(13, 4)) {
		if (Horse->m_Durability > 0.0f) return TRUE;
	}

	return FALSE;
}
BOOL CHECK_Fenrir(LPOBJ lpObj) {
	if (lpObj->Type != OBJ_USER) return FALSE;
	if (lpObj->m_btInvenPetPos == 0 || lpObj->m_wInvenPet == (WORD) - 1) return FALSE;

	CItem * lpFenrir = & lpObj->pInventory[lpObj->m_btInvenPetPos];

	if (lpFenrir->m_Type == ITEMGET(13, 37) && lpFenrir->m_Durability > 0.0f) return TRUE;

	return FALSE;
}

// Damage Calculations
BOOL CObjAttack::ATTACK_CalcSummonerBuff(LPOBJ lpObj, int* iAttackBerserkerMin, int* iAttackBerserkerMax, int nBuffType) {
	gObjUseSkill.m_Lua.Generic_Call("AmplifyDamage_AttackDamage", "ii>ii", (lpObj->CharacterData->Strength + lpObj->AddStrength), (lpObj->CharacterData->Dexterity + lpObj->AddDexterity), iAttackBerserkerMin, iAttackBerserkerMax);

	int iValue = 0;

	BUFF_GetBuffEffectValue(lpObj, nBuffType, &iValue, 0);
	*iAttackBerserkerMin = iValue * *iAttackBerserkerMin / 100;
	*iAttackBerserkerMax = iValue * *iAttackBerserkerMax / 100;

	if (lpObj->CharacterData->Mastery.ML_SUM_VoidAffinity > 0.0) {
		*iAttackBerserkerMin += *iAttackBerserkerMin * lpObj->CharacterData->Mastery.ML_SUM_VoidAffinity / 100.0;
		*iAttackBerserkerMax += *iAttackBerserkerMax * lpObj->CharacterData->Mastery.ML_SUM_VoidAffinity / 100.0;
	}

	return TRUE;
}
int CObjAttack::ATTACK_CalcPhysicalDamage(LPOBJ lpObj, LPOBJ lpTargetObj, int targetDefense, CMagicInf* lpMagic, float& charIncPercent, float& buffDecPercent, int& effect) {
	int ad = 0;
	int gap;
	int SkillRightMaxDamage = 0;
	int SkillRightMinDamage = 0;
	int SkillLeftMaxDamage = 0;
	int SkillLeftMinDamage = 0;
	CItem* Right = &lpObj->pInventory[0];
	CItem* Left = &lpObj->pInventory[1];
	BOOL IsDualWield = FALSE;

	if (lpObj->Type == OBJ_MONSTER || lpObj->Type == OBJ_NPC) {
		int AttackMin = 0;

		gap = lpObj->m_AttackDamageMax - lpObj->m_AttackDamageMin;

		if (gap < 0) {
			g_Log.Add("Monster Class [%d] Damage is 0, check files configuration", lpObj->Class);
			return FALSE;
		}

		AttackMin = lpObj->m_AttackDamageMin + (rand() % (gap + 1));
		lpObj->m_AttackDamageRight = AttackMin;
		lpObj->m_AttackDamageLeft = AttackMin;
	} else {
		if (lpObj->Class == CLASS_KNIGHT || lpObj->Class == CLASS_MAGICGLADIATOR || lpObj->Class == CLASS_RAGEFIGHTER) {
			if (Right->m_Type >= ITEMGET(0, 0) && Right->m_Type < ITEMGET(4, 0) && Left->m_Type >= ITEMGET(0, 0) && Left->m_Type < ITEMGET(4, 0)) {
				if (Right->m_IsValidItem && Left->m_IsValidItem) IsDualWield = TRUE;
			}
		}

		if (lpMagic) {
			int SkillAttr = MagicDamageC.MAGIC_GetSkillAttr(lpMagic->m_Skill);

			ad += lpObj->CharacterData->ANCIENT_SkillDamage;
			ad += lpObj->CharacterData->JewelOfHarmonyEffect.SkillDamage;

			if (SkillAttr != -1) {
				if ((lpObj->Authority & 0x20) != 0x20 || (lpObj->pInventory[10].m_Type != ITEMGET(13, 42) && lpObj->pInventory[11].m_Type != ITEMGET(13, 42))) ad += ((BYTE)lpObj->C_Resistance[SkillAttr] + (BYTE)lpObj->C_ResistanceBonus[SkillAttr]) * 5;
				else ad += 255;
			}
			/*if ((lpMagic->m_Skill == 66 || lpMagic->m_Skill == 509) && lpObj->SkillLongSpearChange) {
				SkillRightMaxDamage = DefMagicInf[66].m_DamageMax;// #error ???
				SkillRightMinDamage = DefMagicInf[66].m_DamageMin;// #error ???
			} else {*/
			SkillRightMinDamage = lpMagic->m_DamageMin;
			SkillRightMaxDamage = lpMagic->m_DamageMax;
			//}
			SkillRightMinDamage += g_SkillSpec.CalcStatBonus(lpObj, lpMagic->m_Skill);
			SkillRightMaxDamage += g_SkillSpec.CalcStatBonus(lpObj, lpMagic->m_Skill);

			if (g_MasterLevelSkillTreeSystem.CheckMasterLevelSkill(lpMagic->m_Skill) == true) {
				float fDamage = g_MasterLevelSkillTreeSystem.GetSkillAttackDamage(lpObj, lpMagic->m_Skill);

				SkillRightMaxDamage += fDamage;
				SkillRightMinDamage += fDamage;
			}

			if (IsDualWield == FALSE) {
				SkillLeftMaxDamage = SkillRightMaxDamage;
				SkillLeftMinDamage = SkillRightMinDamage;
			}

			if (lpObj->Class == CLASS_DARKLORD) {
				int skillDamage = (lpObj->CharacterData->Strength + lpObj->AddStrength) / 25 + (lpObj->CharacterData->Energy + lpObj->AddEnergy) / 50;

				if (lpMagic->m_Skill == 65 || lpMagic->m_Skill == 519) {
					int iPartyCount = this->GetPartyMemberCount(lpObj);

					skillDamage = (lpObj->Leadership + lpObj->AddLeadership) / 10 + (iPartyCount * 50);
				} else if (lpMagic->m_Skill == 62 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(lpMagic->m_Skill) == 512 || lpMagic->m_Skill == 516) {
					if (lpObj->pInventory[lpObj->m_btInvenPetPos].m_Type == ITEMGET(13, 4)) {	// Dark Horse
						skillDamage = (lpObj->CharacterData->Strength + lpObj->AddStrength) / 10 + (lpObj->Leadership + lpObj->AddLeadership) / 5 + lpObj->pInventory[lpObj->m_btInvenPetPos].m_PetItem_Level * 10;
						skillDamage = skillDamage * g_ConfigRead.calc.DarkHorseDamageMultiplier / 100.0f;
					}
				} else if (lpObj->Class == CLASS_DARKLORD && (lpMagic->m_Skill == 238 || lpMagic->m_Skill == 523)) skillDamage = (lpObj->CharacterData->Strength + lpObj->AddStrength) / 30 + (lpObj->CharacterData->Energy + lpObj->AddEnergy) / 55;

				SkillRightMaxDamage += skillDamage;
				SkillRightMinDamage += skillDamage;
				SkillLeftMaxDamage += skillDamage;
				SkillLeftMinDamage += skillDamage;
			}
		}

		int iAttackBerserkerMin = 0;
		int iAttackBerserkerMax = 0;

		if (lpObj->Type == OBJ_USER && BUFF_TargetHasBuff(lpObj, BUFFTYPE_AMPLIFY_DAMAGE)) this->ATTACK_CalcSummonerBuff(lpObj, &iAttackBerserkerMin, &iAttackBerserkerMax, BUFFTYPE_AMPLIFY_DAMAGE);
		else if (lpObj->Type == OBJ_USER && BUFF_TargetHasBuff(lpObj, BUFFTYPE_IMPROVED_AMPLIFY_DAMAGE)) this->ATTACK_CalcSummonerBuff(lpObj, &iAttackBerserkerMin, &iAttackBerserkerMax, BUFFTYPE_IMPROVED_AMPLIFY_DAMAGE);
		else if (lpObj->Type == OBJ_USER && BUFF_TargetHasBuff(lpObj, BUFFTYPE_DIMENSIONAL_AFFINITY)) this->ATTACK_CalcSummonerBuff(lpObj, &iAttackBerserkerMin, &iAttackBerserkerMax, BUFFTYPE_DIMENSIONAL_AFFINITY);
		else if (lpObj->Type == OBJ_USER && BUFF_TargetHasBuff(lpObj, BUFFTYPE_VOID_AFFINITY)) this->ATTACK_CalcSummonerBuff(lpObj, &iAttackBerserkerMin, &iAttackBerserkerMax, BUFFTYPE_VOID_AFFINITY);

		__try {
			gap = (lpObj->m_AttackDamageMaxRight + SkillRightMaxDamage + iAttackBerserkerMax) - (lpObj->m_AttackDamageMinRight + SkillRightMinDamage + iAttackBerserkerMin);	// #formula
			lpObj->m_AttackDamageRight = (lpObj->m_AttackDamageMinRight + SkillRightMinDamage + iAttackBerserkerMin) + (rand() % (gap + 1));
		} __except (gap = 1, 1) {}

		__try {
			gap = (lpObj->m_AttackDamageMaxLeft + SkillLeftMaxDamage + iAttackBerserkerMax) - (lpObj->m_AttackDamageMinLeft + SkillLeftMinDamage + iAttackBerserkerMin);	// #formula
			lpObj->m_AttackDamageLeft = (lpObj->m_AttackDamageMinLeft + SkillLeftMinDamage + iAttackBerserkerMin) + (rand() % (gap + 1));
		} __except (gap = 1, 1) {}

		int CriticalRate = lpObj->C_CriticalStrikeChance;
		int ExcellentRate = lpObj->C_ExcellentStrikeChance;
		int Resistance = 0;

		if (lpTargetObj->Type == OBJ_USER) Resistance = lpTargetObj->CharacterData->C_CriticalStrikeResistance;
		if (CriticalRate > 0) {
			if (!CHARACTER_PvPResistanceProc(Resistance)) {
				if (CHARACTER_DebuffProc(CriticalRate)) effect = 3;
			}
		}

		if (lpTargetObj->Type == OBJ_USER) Resistance = lpTargetObj->CharacterData->C_ExcellentStrikeResistance;
		if (ExcellentRate > 0) {
			if (!CHARACTER_PvPResistanceProc(Resistance)) {
				if (CHARACTER_DebuffProc(ExcellentRate)) effect = 2;
			}
		}

		if (effect == 2 || effect == 3) {
			lpObj->m_AttackDamageRight = lpObj->m_AttackDamageMaxRight + SkillRightMaxDamage + iAttackBerserkerMax;
			lpObj->m_AttackDamageLeft = lpObj->m_AttackDamageMaxLeft + SkillLeftMaxDamage + iAttackBerserkerMax;
		}
	}

	if (IsDualWield) {
		ad += lpObj->m_AttackDamageRight + lpObj->m_AttackDamageLeft;
	} else if ((Left->m_Type >= ITEMGET(4, 0) && Left->m_Type < ITEMGET(4, 7)) || Left->m_Type == ITEMGET(4, 20) || Left->m_Type == ITEMGET(4, 21) || Left->m_Type == ITEMGET(4, 22) || Left->m_Type == ITEMGET(4, 23) || Left->m_Type == ITEMGET(4, 24)) {
		ad += lpObj->m_AttackDamageLeft;

		if (lpObj->CharacterData->Mastery.ML_ELF_BowMastery > 0.0) ad += lpObj->CharacterData->Mastery.ML_ELF_BowMastery;
	} else if ((Right->m_Type >= ITEMGET(4, 8) && Right->m_Type < ITEMGET(4, 15)) || (Right->m_Type >= ITEMGET(4, 16) && Right->m_Type < ITEMGET(5, 0))) {
		ad += lpObj->m_AttackDamageRight;

		if (lpObj->CharacterData->Mastery.ML_ELF_CrossbowMastery > 0.0) ad += lpObj->CharacterData->Mastery.ML_ELF_CrossbowMastery;
	} else if (Right->GetDetailItemType() == ITEM_TWOHAND_SWORD && Right->GetDetailItemType() == ITEM_TWOHAND_SWORD) {
		ad += lpObj->m_AttackDamageRight;

		if (lpObj->CharacterData->Mastery.ML_DK_TwoHandedSwordMastery > 0.0) ad += lpObj->CharacterData->Mastery.ML_DK_TwoHandedSwordMastery;
	} else if (Right->m_Type >= ITEMGET(0, 0) && Right->m_Type < ITEMGET(4, 0)) ad += lpObj->m_AttackDamageRight;
	else if (Right->m_Type >= ITEMGET(5, 0) && Right->m_Type < ITEMGET(6, 0)) ad += lpObj->m_AttackDamageRight;
	else ad += lpObj->m_AttackDamageLeft;
	if (lpObj->Type == OBJ_USER && lpObj->CharacterData->ANCIENT_TwoHandedWeaponDamage) charIncPercent += lpObj->CharacterData->ANCIENT_TwoHandedWeaponDamage / 100.00;
	if (lpObj->Type == OBJ_USER && IsDualWield && lpObj->CharacterData->ANCIENT_DualWieldDamage > 0) charIncPercent += lpObj->CharacterData->ANCIENT_DualWieldDamage / 100.00;
	if (lpObj->m_SkillInfo.RemedyOfLoveEffect) ad += 15;
	if (lpObj->m_MonsterSkillElementInfo.m_iSkillElementAttackTime > 0) {
		ad += lpObj->m_MonsterSkillElementInfo.m_iSkillElementAttack;

		if (ad < 0) ad = 0;
	}

	int iValue = BUFF_GetTotalBuffEffectValue(lpObj, EFFECTTYPE_DECREASE_DAMAGE);

	buffDecPercent -= iValue / 100.00;

	float nAddAttackDamageLeft = 0.0;
	float nAddAttackDamageRight = 0.0;

	if (lpObj->Type == OBJ_USER) {
		switch (Left->GetDetailItemType()) {
			case ITEM_ONEHAND_SWORD:					if (lpObj->CharacterData->Mastery.ML_DK_OneHandedSwordMastery > 0.0) nAddAttackDamageLeft += lpObj->CharacterData->Mastery.ML_DK_OneHandedSwordMastery;						break;
			case ITEM_MACE:								if (lpObj->CharacterData->Mastery.ML_DK_MaceMastery > 0.0) nAddAttackDamageLeft += lpObj->CharacterData->Mastery.ML_DK_MaceMastery;											break;
			case ITEM_SPEAR:							if (lpObj->CharacterData->Mastery.ML_DK_SpearMastery > 0.0) nAddAttackDamageLeft += lpObj->CharacterData->Mastery.ML_DK_SpearMastery;										break;
			case ITEM_DARKLORD_SCEPTER:					if (lpObj->CharacterData->Mastery.ML_DL_ScepterMastery > 0.0) nAddAttackDamageLeft += lpObj->CharacterData->Mastery.ML_DL_ScepterMastery;									break;
			case ITEM_RAGEFIGHTER_WEAPON:				if (lpObj->CharacterData->Mastery.ML_RF_FistMastery > 0.0) nAddAttackDamageLeft += lpObj->CharacterData->Mastery.ML_RF_FistMastery;											break;
		}

		switch (Right->GetDetailItemType()) {
			case ITEM_ONEHAND_SWORD:					if (lpObj->CharacterData->Mastery.ML_DK_OneHandedSwordMastery > 0.0) nAddAttackDamageRight += lpObj->CharacterData->Mastery.ML_DK_OneHandedSwordMastery;						break;
			case ITEM_MACE:								if (lpObj->CharacterData->Mastery.ML_DK_MaceMastery > 0.0)	nAddAttackDamageRight += lpObj->CharacterData->Mastery.ML_DK_MaceMastery;											break;
			case ITEM_SPEAR:							if (lpObj->CharacterData->Mastery.ML_DK_SpearMastery > 0.0) nAddAttackDamageRight += lpObj->CharacterData->Mastery.ML_DK_SpearMastery;										break;
			case ITEM_DARKLORD_SCEPTER: 				if (lpObj->CharacterData->Mastery.ML_DL_ScepterMastery > 0.0) nAddAttackDamageRight += lpObj->CharacterData->Mastery.ML_DL_ScepterMastery;									break;
			case ITEM_RAGEFIGHTER_WEAPON: 				if (lpObj->CharacterData->Mastery.ML_RF_FistMastery > 0.0) nAddAttackDamageRight += lpObj->CharacterData->Mastery.ML_RF_FistMastery;											break;
		}
	}

	if (IsDualWield) ad += (nAddAttackDamageRight + nAddAttackDamageLeft) * 0.5;
	else ad += nAddAttackDamageRight + nAddAttackDamageLeft;

	ad += BUFF_GetTotalBuffEffectValue(lpObj, EFFECTTYPE_INCREASE_DAMAGE);
	ad -= targetDefense;

	if (ad < 0) ad = 0;

	return ad;
}
int CObjAttack::ATTACK_CalcMagicDamage(LPOBJ lpObj, LPOBJ lpTargetObj, int targetDefense, CMagicInf* lpMagic, float& itemIncPercent, int& effect) {
	if (lpObj->Type != OBJ_USER) return FALSE;

	int SkillAttr = MagicDamageC.MAGIC_GetSkillAttr(lpMagic->m_Skill);
	int SkillHellFire2CountDamageTable[13] = {120, 200, 350, 500, 660, 725, 825, 925, 1050, 1200, 1380, 1490, 1600};
	int CountDamage;
	double fCurseDamageMin = 0;
	double fCurseDamageMax = 0;
	double fMagicDamageMin = 0;
	double fMagicDamageMax = 0;
	int damagemin = 0;
	int damagemax = 0;
	int ad;

	if (g_MasterLevelSkillTreeSystem.CheckMasterLevelSkill(lpMagic->m_Skill)) {
		float fDamage = g_MasterLevelSkillTreeSystem.GetSkillAttackDamage(lpObj, lpMagic->m_Skill);

		damagemin += fDamage;
		damagemax += fDamage;
	}

	if (lpMagic->m_Skill == 40) {
		if (lpObj->CharacterData->SkillHellFire2Count >= 0) {
			if (lpObj->CharacterData->SkillHellFire2Count > 12) CountDamage = 0;
			else CountDamage = SkillHellFire2CountDamageTable[lpObj->CharacterData->SkillHellFire2Count];

			ad = (lpObj->CharacterData->Energy + lpObj->AddEnergy) / 8 + CountDamage;
			damagemin += ad + lpObj->m_MagicDamageMin;
			damagemax += ad + lpObj->m_MagicDamageMax;
			goto calculate_magic_damage;
		}
	} else if (lpMagic->m_Skill == 392) {
		if (lpObj->CharacterData->SkillStrengthenHellFire2Count >= 0) {
			if (lpObj->CharacterData->SkillStrengthenHellFire2Count > 12) CountDamage = 0;
			else CountDamage = SkillHellFire2CountDamageTable[lpObj->CharacterData->SkillStrengthenHellFire2Count];

			ad = (lpObj->CharacterData->Energy + lpObj->AddEnergy) / 8 + CountDamage;
			damagemin += ad + lpObj->m_MagicDamageMin;
			damagemax += ad + lpObj->m_MagicDamageMax;
			goto calculate_magic_damage;
		}
	} else {
		switch (lpMagic->m_Skill) {
			case 219:
			case 220:
			case 221:
			case 222:
			case 223:
			case 224:
			case 225:
			case 459:
			case 460:
				if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_AMPLIFY_DAMAGE) || BUFF_TargetHasBuff(lpObj, BUFFTYPE_IMPROVED_AMPLIFY_DAMAGE) || BUFF_TargetHasBuff(lpObj, BUFFTYPE_DIMENSIONAL_AFFINITY) || BUFF_TargetHasBuff(lpObj, BUFFTYPE_VOID_AFFINITY)) {
					int iValue = BUFF_GetTotalBuffEffectValue(lpObj, EFFECTTYPE_INCREASE_AMPLIFY_DAMAGE);

					if (lpObj->CharacterData->Mastery.ML_SUM_ImprovedAmplifyDamage > 0.0) iValue += lpObj->CharacterData->Mastery.ML_SUM_ImprovedAmplifyDamage;
					if (lpObj->CharacterData->Mastery.ML_SUM_VoidAffinity > 0.0) iValue += lpObj->CharacterData->Mastery.ML_SUM_VoidAffinity;

					gObjUseSkill.m_Lua.Generic_Call("AmplifyDamage_CurseDamage", "ii>dd", (lpObj->CharacterData->Energy + lpObj->AddEnergy), iValue, &fCurseDamageMin, &fCurseDamageMax);
				}

				damagemin += lpObj->m_CurseDamageMin + fCurseDamageMin + lpMagic->m_DamageMin + g_SkillSpec.CalcStatBonus(lpObj, lpMagic->m_Skill) + BUFF_GetTotalBuffEffectValue(lpObj, EFFECTTYPE_CURSE_DAMAGE);
				damagemax += lpObj->m_CurseDamageMax + fCurseDamageMax + lpMagic->m_DamageMax + g_SkillSpec.CalcStatBonus(lpObj, lpMagic->m_Skill) + BUFF_GetTotalBuffEffectValue(lpObj, EFFECTTYPE_CURSE_DAMAGE);

				if (lpObj->pInventory[1].IsItem() && (lpObj->pInventory[1].m_Type >= ITEMGET(5, 0) && lpObj->pInventory[1].m_Type < ITEMGET(6, 0)) && lpObj->pInventory[1].m_IsValidItem) {
					int damage = lpObj->pInventory[1].m_CurseSpell / 2 + lpObj->pInventory[1].m_Level * 2;

					damage -= (WORD)(lpObj->pInventory[1].m_CurrentDurabilityState * damage);
					itemIncPercent += damage / 100.00;
				}
			break;

			default:
				if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_AMPLIFY_DAMAGE) || BUFF_TargetHasBuff(lpObj, BUFFTYPE_IMPROVED_AMPLIFY_DAMAGE) || BUFF_TargetHasBuff(lpObj, BUFFTYPE_DIMENSIONAL_AFFINITY) || BUFF_TargetHasBuff(lpObj, BUFFTYPE_VOID_AFFINITY)) {
					int iValue = BUFF_GetTotalBuffEffectValue(lpObj, EFFECTTYPE_INCREASE_AMPLIFY_DAMAGE);

					if (lpObj->CharacterData->Mastery.ML_SUM_DimensionalAffinity > 0.0) iValue += lpObj->CharacterData->Mastery.ML_SUM_DimensionalAffinity;

					gObjUseSkill.m_Lua.Generic_Call("AmplifyDamage_MagicDamage", "ii>dd", (lpObj->CharacterData->Energy + lpObj->AddEnergy), iValue, &fMagicDamageMin, &fMagicDamageMax);
				}

				damagemin += (int)((float)(lpMagic->m_DamageMin + lpObj->m_MagicDamageMin) + fMagicDamageMin + g_SkillSpec.CalcStatBonus(lpObj, lpMagic->m_Skill) + BUFF_GetTotalBuffEffectValue(lpObj, EFFECTTYPE_INCREASE_SPELL_DAMAGE));
				damagemax += (int)((float)(lpMagic->m_DamageMax + lpObj->m_MagicDamageMax) + fMagicDamageMax + g_SkillSpec.CalcStatBonus(lpObj, lpMagic->m_Skill) + BUFF_GetTotalBuffEffectValue(lpObj, EFFECTTYPE_INCREASE_SPELL_DAMAGE));

				calculate_magic_damage:
				if (lpObj->pInventory[0].IsItem() && (lpObj->pInventory[0].m_Type >= ITEMGET(5, 0) && lpObj->pInventory[0].m_Type < ITEMGET(6, 0)) && lpObj->pInventory[0].m_IsValidItem) {
					int damage = lpObj->pInventory[0].m_Magic / 2 + lpObj->pInventory[0].m_Level * 2;

					damage -= (WORD)(lpObj->pInventory[0].m_CurrentDurabilityState * damage);
					itemIncPercent += damage / 100.00;
				}
			break;
		}
	}

	int CriticalRate = lpObj->C_CriticalStrikeChance;
	int ExcellentRate = lpObj->C_ExcellentStrikeChance;
	int Resistance = 0;
	int gap = damagemax - damagemin;

	__try {
		ad = damagemin + (rand() % (gap + 1));
	} __except (gap = 1, 1) {}

	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_ARCANE_FOCUS)) CriticalRate += lpObj->CharacterData->Mastery.ML_DW_ArcaneFocus;
	if (lpTargetObj->Type == OBJ_USER) Resistance = lpTargetObj->CharacterData->C_CriticalStrikeResistance;
	if (CriticalRate > 0) {
		if (!CHARACTER_PvPResistanceProc(Resistance)) {
			if (CHARACTER_DebuffProc(CriticalRate)) {
				effect = 3;
				ad = damagemax;
			}
		}
	}

	if (lpTargetObj->Type == OBJ_USER) Resistance = lpTargetObj->CharacterData->C_ExcellentStrikeResistance;
	if (ExcellentRate > 0) {
		if (!CHARACTER_PvPResistanceProc(Resistance)) {
			if (CHARACTER_DebuffProc(ExcellentRate)) {
				effect = 2;
				ad = damagemax;
			}
		}
	}

	if (lpObj->m_SkillInfo.RemedyOfLoveEffect) ad += 10;
	if (SkillAttr != -1) {
		if ((lpObj->Authority & 0x20) != 0x20 || (lpObj->pInventory[10].m_Type != ITEMGET(13, 42) && lpObj->pInventory[11].m_Type != ITEMGET(13, 42))) ad += ((BYTE)lpObj->C_Resistance[SkillAttr] + (BYTE)lpObj->C_ResistanceBonus[SkillAttr]) * 5;
		else ad += 255;
	}

	ad += lpObj->CharacterData->ANCIENT_SkillDamage;
	ad += lpObj->CharacterData->JewelOfHarmonyEffect.SkillDamage;
	ad += BUFF_GetTotalBuffEffectValue(lpObj, EFFECTTYPE_INCREASE_DAMAGE);
	ad -= targetDefense;

	if (ad < 0) ad = 0;

	return ad;
}
int CObjAttack::ATTACK_CalcShieldDamage(LPOBJ lpObj, LPOBJ lpTargetObj, int iAttackDamage) {
	int iShieldDamage = 0;

	if (g_ShieldSystemOn == FALSE) return FALSE;
	if (iAttackDamage <= 0)	return FALSE;
	if (lpObj->Type != OBJ_USER) return FALSE;
	if (lpTargetObj->Type != OBJ_USER) return FALSE;

	int iReduceLife = 0;
	int iReduceShield = 0;
	int iReduceLifeForEffect = 0; 
	bool bReduceShieldGage = 0;
	int iDamageDevideToSDRate = g_iDamageDevideToSDRate;

	iDamageDevideToSDRate -= lpObj->CharacterData->JewelOfHarmonyEffect.ShieldBypassRate;
	
	if (iDamageDevideToSDRate < 0) iDamageDevideToSDRate = 0;
	if (iDamageDevideToSDRate > 100) iDamageDevideToSDRate = 100;
	if (lpObj->CharacterData->JewelOfHarmonyEffect.ShieldPenetrationChance > 0) {
		int iAccessoryEffect = lpTargetObj->CharacterData->C_ShieldPenetrationResistance;
		int iIgnoreSDRate = lpObj->CharacterData->JewelOfHarmonyEffect.ShieldPenetrationChance;

		if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_FOCUS)) {
			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 575) == 20) iIgnoreSDRate += 1;
		}

		if (!CHARACTER_PvPResistanceProc(iAccessoryEffect)) {
			if (CHARACTER_DebuffProc(iIgnoreSDRate)) iDamageDevideToSDRate = 0;
		}
	}

	if ((lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) && (lpObj->CharacterData->JewelOfHarmonyEffect.ShieldBypassRate || lpTargetObj->CharacterData->JewelOfHarmonyEffect.ReduceShieldDamageTaken || lpObj->CharacterData->JewelOfHarmonyEffect.ShieldPenetrationChance)) g_Log.Add("[JewelOfHarmony][PvP System] Attacker:[%s][%s]-SD Decrease[%d] SD Ignore[%d] Defender:[%s][%s] SD Increase Option[%d] - SD Rate[%d]", lpObj->AccountID, lpObj->Name, lpObj->CharacterData->JewelOfHarmonyEffect.ShieldBypassRate, lpObj->CharacterData->JewelOfHarmonyEffect.ShieldPenetrationChance, lpTargetObj->AccountID, lpTargetObj->Name, lpTargetObj->CharacterData->JewelOfHarmonyEffect.ReduceShieldDamageTaken, iDamageDevideToSDRate);

	iReduceShield = iAttackDamage * iDamageDevideToSDRate / 100;
	iReduceLife = iAttackDamage - iReduceShield;
	iReduceShield -= iReduceShield * lpTargetObj->CharacterData->JewelOfHarmonyEffect.ReduceShieldDamageTaken / 100;

	if ((lpTargetObj->iShield - iReduceShield) < 0) {
		iReduceLife += iReduceShield - lpTargetObj->iShield;
		iReduceShield = lpTargetObj->iShield;

		if (lpTargetObj->iShield > 0) bReduceShieldGage = true;
	}

	iReduceLifeForEffect = (lpTargetObj->MaxLife + lpTargetObj->AddLife) * 0.2f;

	if (bReduceShieldGage == true && iReduceLife > iReduceLifeForEffect) {
		if (!CC_MAP_RANGE(lpTargetObj->MapNumber) && lpTargetObj->MapNumber != MAP_INDEX_CHAOSCASTLE_SURVIVAL) GSProtocol.PROTOCOL_SendEffectInfo(lpTargetObj->m_Index, 17);
	}

	return iReduceShield;
}
int CObjAttack::ATTACK_CalcElementalDamage(LPOBJ lpObj, LPOBJ lpTargetObj, char* DamageType1, char* DamageType2, int iAttackDamage, int iTargetDefense) {
	int distance = CHARACTER_CalcDistance(lpObj, lpTargetObj);
	int Strength = 0;
	int Dexterity = 0;
	int Vitality = 0;
	int Energy = 0;
	int Command = 0;
	int PentagramMinDamage = 0;
	int PentagramMaxDamage = 0;
	int PentagramDefense = 0;
	int BaseDamage = 0;
	int BaseDefense = 0;
	int MinDamage = 0;
	int MaxDamage = 0;
	int AddDamage = 0;
	int AddDefense = 0;
	int TotalDamage = 0;
	int TotalDefense = 0;
	int TotalDamageWithRelationShip = 0;
	int TotalDefenseWithRelationShip = 0;
	int AttackRelationShip = 0;
	int DefenseRelationShip = 0;
	int AttackSuccessRate = 0;
	int DefenseSuccessRate = 0;
	int AttackerAttribute = lpObj->m_iPentagramMainAttribute;
	int DefenserAttribute = lpTargetObj->m_iPentagramMainAttribute;
	bool bExcellentDamageOption = false;
	bool bCriticalDamageOption = false;
	bool bDoubleDamageOption = false;

	switch (AttackerAttribute) {
		case EL_FIRE:			*DamageType1 = 1;			break;
		case EL_WATER:			*DamageType1 = 2;			break;
		case EL_EARTH:			*DamageType1 = 3;			break;
		case EL_WIND:			*DamageType1 = 4;			break;
		case EL_DARKNESS:		*DamageType1 = 5;			break;
		default:				*DamageType1 = 0;			break;
	}

	*DamageType2 = 0;

	if (lpObj->Type == OBJ_USER) {
		// Runestone of Radiance (Rank 2)
		Strength = (lpObj->CharacterData->Strength + lpObj->AddStrength) + ((lpObj->CharacterData->Strength + lpObj->AddStrength) * lpObj->CharacterData->m_PentagramOptions.m_iOnyx_2RankAddStrength / 100);
		Dexterity = (lpObj->CharacterData->Dexterity + lpObj->AddDexterity) + ((lpObj->CharacterData->Dexterity + lpObj->AddDexterity) * lpObj->CharacterData->m_PentagramOptions.m_iOnyx_2RankAddDexterity / 100);
		Vitality = (lpObj->CharacterData->Vitality + lpObj->AddVitality) + ((lpObj->CharacterData->Vitality + lpObj->AddVitality) * lpObj->CharacterData->m_PentagramOptions.m_iOnyx_2RankAddVitality / 100);
		Energy = (lpObj->CharacterData->Energy + lpObj->AddEnergy) + ((lpObj->CharacterData->Energy + lpObj->AddEnergy) * lpObj->CharacterData->m_PentagramOptions.m_iOnyx_2RankAddEnergy / 100);
		Command = lpObj->Leadership + lpObj->AddLeadership;

		if (g_PentagramSystem.IsPentagramItem(&lpObj->pInventory[236]) == true) {
			PentagramMinDamage = lpObj->pInventory[236].m_DamageMin;
			PentagramMaxDamage = lpObj->pInventory[236].m_DamageMax;
		}

		int iExcelDamageRate = 0;
		int iDoubleDamageRate = 0;

		// Runestone of Radiance (Rank 4)
		if (lpObj->CharacterData->m_PentagramOptions.m_iOnyx_4RankOptionNum == 1) {
			if (lpTargetObj->Type == OBJ_USER) iDoubleDamageRate = lpObj->CharacterData->m_PentagramOptions.m_iOnyx_4RankAddDoubleDamageRate;
		} else if (lpObj->CharacterData->m_PentagramOptions.m_iOnyx_4RankOptionNum == 2) {
			if (lpTargetObj->Type == OBJ_MONSTER) iDoubleDamageRate = lpObj->CharacterData->m_PentagramOptions.m_iOnyx_4RankAddDoubleDamageRate;
		}

		if (rand() % 100 < iDoubleDamageRate) bDoubleDamageOption = TRUE;
		// Runestone of Radiance (Rank 1)
		if (lpObj->CharacterData->m_PentagramOptions.m_iOnyx_1RankAddExcellentDamageRate > 0) iExcelDamageRate = lpObj->CharacterData->m_PentagramOptions.m_iOnyx_1RankAddExcellentDamageRate;
		if (rand() % 100 < iExcelDamageRate) bExcellentDamageOption = TRUE;
		if (bExcellentDamageOption == FALSE) {
			int iCriDamageRate = 0;

			// Runestone of Anger (Rank 5)
			if (lpObj->CharacterData->m_PentagramOptions.m_iRuby_5RankOptionNum == 1) {
				if (lpTargetObj->Type == OBJ_USER) iCriDamageRate = lpObj->CharacterData->m_PentagramOptions.m_iRuby_5RankCriticalDamageRate;
			} else if (lpObj->CharacterData->m_PentagramOptions.m_iRuby_5RankOptionNum == 2) {
				if (lpTargetObj->Type == OBJ_MONSTER) iCriDamageRate = lpObj->CharacterData->m_PentagramOptions.m_iRuby_5RankCriticalDamageRate;
			}
			// Elemental Seal Bonus (+10% Seal Critical Strike Chance)
			if (lpObj->CharacterData->m_PentagramOptions.m_isAddCriPentaDamage == true) iCriDamageRate += 10;
			if (rand() % 100 < iCriDamageRate) bCriticalDamageOption = TRUE;
		}

		// Elemental Seal Bonus (+10% Seal Damage)
		if (lpObj->CharacterData->m_PentagramOptions.m_isAddPentaAttack == true) {
			PentagramMinDamage *= 1.1;
			PentagramMaxDamage *= 1.1;
		}

		switch (lpObj->Class) {
			case CLASS_WIZARD:				this->m_Lua.Generic_Call("WizardElementalDamageCalc", "iiiiii>ii", Strength, Dexterity, Vitality, Energy, PentagramMinDamage, PentagramMaxDamage, &MinDamage, &MaxDamage);					break;
			case CLASS_KNIGHT:				this->m_Lua.Generic_Call("KnightElementalDamageCalc", "iiiiii>ii", Strength, Dexterity, Vitality, Energy, PentagramMinDamage, PentagramMaxDamage, &MinDamage, &MaxDamage);					break;
			case CLASS_ELF:					this->m_Lua.Generic_Call("ElfElementalDamageCalc", "iiiiii>ii", Strength, Dexterity, Vitality, Energy, PentagramMinDamage, PentagramMaxDamage, &MinDamage, &MaxDamage);						break;
			case CLASS_MAGICGLADIATOR:				this->m_Lua.Generic_Call("GladiatorElementalDamageCalc", "iiiiii>ii", Strength, Dexterity, Vitality, Energy, PentagramMinDamage, PentagramMaxDamage, &MinDamage, &MaxDamage);				break;
			case CLASS_DARKLORD:			this->m_Lua.Generic_Call("LordElementalDamageCalc", "iiiiiii>ii", Strength, Dexterity, Vitality, Energy, Command, PentagramMinDamage, PentagramMaxDamage, &MinDamage, &MaxDamage);			break;
			case CLASS_SUMMONER:			this->m_Lua.Generic_Call("SummonerElementalDamageCalc", "iiiiii>ii", Strength, Dexterity, Vitality, Energy, PentagramMinDamage, PentagramMaxDamage, &MinDamage, &MaxDamage);				break;
			case CLASS_RAGEFIGHTER:			this->m_Lua.Generic_Call("RageFighterElementalDamageCalc", "iiiiii>ii", Strength, Dexterity, Vitality, Energy, PentagramMinDamage, PentagramMaxDamage, &MinDamage, &MaxDamage);				break;
			case CLASS_GROWLANCER:			this->m_Lua.Generic_Call("GrowLancerElementalDamageCalc", "iiiiii>ii", Strength, Dexterity, Vitality, Energy, PentagramMinDamage, PentagramMaxDamage, &MinDamage, &MaxDamage);				break;
		}

		// Runestone of Anger (Rank 1)
		AddDamage = lpObj->CharacterData->m_PentagramOptions.m_iRuby_1RankAddDamage;

		// Runestone of Anger (Rank 3)
		if (lpObj->CharacterData->m_PentagramOptions.m_iRuby_3RankOptionNum == 1 && lpTargetObj->Type == OBJ_USER) AddDamage += lpObj->CharacterData->m_PentagramOptions.m_iRuby_3RankAddDamage;
		else if (lpObj->CharacterData->m_PentagramOptions.m_iRuby_3RankOptionNum == 2 && lpTargetObj->Type == OBJ_MONSTER) AddDamage += lpObj->CharacterData->m_PentagramOptions.m_iRuby_3RankAddDamage;
		// Runestone of Anger (Rank 4)
		if (lpObj->CharacterData->m_PentagramOptions.m_iRuby_4RankOptionNum == 1) {
			if (distance > 2) AddDamage += lpObj->CharacterData->m_PentagramOptions.m_iRuby_4RankAddDamage;
		} else if (lpObj->CharacterData->m_PentagramOptions.m_iRuby_4RankOptionNum == 2) {
			if (distance < 3) AddDamage += lpObj->CharacterData->m_PentagramOptions.m_iRuby_4RankAddDamage;
		}

		// Runestone of Wrath (Rank 3)
		if (lpObj->CharacterData->m_PentagramOptions.m_iEme_3RankOptionNum == 1 && lpTargetObj->Type == OBJ_USER) AddDamage += lpObj->CharacterData->m_PentagramOptions.m_iEme_3RankAddDamage;
		else if (lpObj->CharacterData->m_PentagramOptions.m_iEme_3RankOptionNum == 2 && lpTargetObj->Type == OBJ_MONSTER) AddDamage += lpObj->CharacterData->m_PentagramOptions.m_iEme_3RankAddDamage;
		// Runestone of Wrath (Rank 4)
		if (lpObj->CharacterData->m_PentagramOptions.m_iEme_4RankOptionNum == 1) {
			if (distance > 2) AddDamage += lpObj->CharacterData->m_PentagramOptions.m_iEme_4RankAddDamage;
		} else if (lpObj->CharacterData->m_PentagramOptions.m_iEme_4RankOptionNum == 2) {
			if (distance < 3) AddDamage += lpObj->CharacterData->m_PentagramOptions.m_iEme_4RankAddDamage;
		}
	} else if (lpObj->Type == OBJ_MONSTER) {
		MinDamage = lpObj->m_iPentagramAttackMin;
		MaxDamage = lpObj->m_iPentagramAttackMax;
	}

	if (bCriticalDamageOption == TRUE || bExcellentDamageOption == TRUE) MinDamage = MaxDamage;

	int RandDamage = 0;

	if (lpObj->m_iPentagramDamageOrigin == 1) {
		if (MinDamage <= MaxDamage) RandDamage = MaxDamage - MinDamage;
		else RandDamage = MinDamage - MaxDamage;

		BaseDamage = MinDamage + (rand() % (RandDamage + 1));
	} else {
		if (lpObj->m_iPentagramDamageMax == 1) BaseDamage = MaxDamage;
		else if (lpObj->m_iPentagramDamageMin == 1) BaseDamage = MinDamage;
	}

	TotalDamage = BaseDamage + AddDamage;

	// Runestone of Wrath (Rank 5)
	if (lpObj->Type == OBJ_USER) {
		if (lpObj->CharacterData->m_PentagramOptions.m_iEme_5RankOptionNum == 1 && lpTargetObj->Type == OBJ_USER) TotalDamage += TotalDamage * lpObj->CharacterData->m_PentagramOptions.m_iEme_5RankAddDamageRate / 100;
		else if (lpObj->CharacterData->m_PentagramOptions.m_iEme_5RankOptionNum == 2 && lpTargetObj->Type == OBJ_MONSTER) TotalDamage += TotalDamage * lpObj->CharacterData->m_PentagramOptions.m_iEme_5RankAddDamageRate / 100;
	}

	if (TotalDamage > 0 && lpObj->Type == OBJ_USER) {
		if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_ARCA_FIRETOWER) == TRUE && AttackerAttribute == EL_FIRE) TotalDamage += 5 * TotalDamage / 100;
		else if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_ARCA_WATERTOWER) == TRUE && AttackerAttribute == EL_WATER) TotalDamage += 5 * TotalDamage / 100;
		else if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_ARCA_EARTHTOWER) == TRUE && AttackerAttribute == EL_EARTH) TotalDamage += 5 * TotalDamage / 100;
		else if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_ARCA_WINDTOWER) == TRUE && AttackerAttribute == EL_WIND) TotalDamage += 5 * TotalDamage / 100;
		else if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_ARCA_DARKNESSTOWER) == TRUE && AttackerAttribute == EL_DARKNESS) TotalDamage += 5 * TotalDamage / 100;
	}

	// Elemental Seal Bonus (+5% More Elemental Damage)
	if (lpObj->Type == OBJ_USER && lpObj->CharacterData->m_PentagramOptions.m_isChangePentaAttack == true) TotalDamage *= 1.05;
	if (lpTargetObj->Type == OBJ_USER) {
		if (g_PentagramSystem.IsPentagramItem(&lpTargetObj->pInventory[236]) == true) PentagramDefense = lpTargetObj->pInventory[236].Defense;
		// Elemental Seal Bonus (+10% Seal Defense)
		if (lpTargetObj->CharacterData->m_PentagramOptions.m_isAddPentaDefense == true) PentagramDefense *= 1.1;
	} else if (lpTargetObj->Type == OBJ_MONSTER) PentagramDefense = lpTargetObj->m_iPentagramDefense;

	if (lpTargetObj->Type == OBJ_USER) {
		this->m_Lua.Generic_Call("ElementalDefenseCalc", "ii>i", lpTargetObj->Class, lpTargetObj->CharacterData->Dexterity, &BaseDefense);
		BaseDefense += PentagramDefense;
	} else BaseDefense = PentagramDefense;

	if (lpTargetObj->Type == OBJ_USER && g_PentagramSystem.IsPentagramItem(&lpTargetObj->pInventory[236]) == true) {
		// Runestone of Blessing (Rank 1)
		AddDefense = lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_1RankAddDefense;

		// Runestone of Blessing (Rank 3)
		if (lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_3RankOptionNum == 1 && lpObj->Type == OBJ_USER) AddDefense += lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_3RankAddDefense;
		else if (lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_3RankOptionNum == 2 && lpObj->Type == OBJ_MONSTER) AddDefense += lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_3RankAddDefense;
		// Runestone of Blessing (Rank 4)
		if (lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_4RankOptionNum == 1) {
			if (distance > 2) AddDefense += lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_4RankAddDefense;
		} else if (lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_4RankOptionNum == 2) {
			if (distance < 3) AddDefense += lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_4RankAddDefense;
		}

		// Runestone of Divinity (Rank 3)
		if (lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_3RankOptionNum == 1 && lpObj->Type == OBJ_USER) AddDefense += lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_3RankAddDefense;
		else if (lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_3RankOptionNum == 2 && lpObj->Type == OBJ_MONSTER) AddDefense += lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_3RankAddDefense;
		// Runestone of Divinity (Rank 4)
		if (lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_4RankOptionNum == 1) {
			if (distance > 2) AddDefense += lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_4RankAddDefense;
		} else if (lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_4RankOptionNum == 2) {
			if (distance < 3) AddDefense += lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_4RankAddDefense;
		}

		// Elemental Seal Bonus (Gain 5% Defense As Extra Elemental Defense)
		if (lpTargetObj->CharacterData->m_PentagramOptions.m_isChangePentaDefense == true) AddDefense += iTargetDefense * 0.05;
	}

	TotalDefense = BaseDefense + AddDefense;

	if (TotalDefense > 0 && lpTargetObj->Type == OBJ_USER) {
		if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_ARCA_FIRETOWER) == TRUE && DefenserAttribute == EL_FIRE) TotalDefense += 5 * TotalDefense / 100;
		else if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_ARCA_WATERTOWER) == TRUE && DefenserAttribute == EL_WATER) TotalDefense += 5 * TotalDefense / 100;
		else if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_ARCA_EARTHTOWER) == TRUE && DefenserAttribute == EL_EARTH) TotalDefense += 5 * TotalDefense / 100;
		else if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_ARCA_WINDTOWER) == TRUE && DefenserAttribute == EL_WIND) TotalDefense += 5 * TotalDefense / 100;
		else if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_ARCA_DARKNESSTOWER) == TRUE && DefenserAttribute == EL_DARKNESS) TotalDefense += 5 * TotalDefense / 100;
	}

	// Runestone of Divinity (Rank 5)
	if (lpTargetObj->Type == OBJ_USER) {
		if (lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_5RankOptionNum == 1 && lpObj->Type == OBJ_USER) TotalDamage -= lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_5RankDamageAbsorbRate * TotalDamage / 100;
		else if (lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_5RankOptionNum == 2 && lpObj->Type == OBJ_MONSTER) TotalDamage -= lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_5RankDamageAbsorbRate * TotalDamage / 100;
	}

	// Attacker Relationships
	switch (AttackerAttribute) {
		// Fire vs. X
		case EL_FIRE:
			switch (DefenserAttribute) {
				case EL_FIRE:				AttackRelationShip = 100;				break;
				case EL_WATER:				AttackRelationShip = 80;				break;
				case EL_EARTH:				AttackRelationShip = 90;				break;
				case EL_WIND:				AttackRelationShip = 110;				break;
				case EL_DARKNESS:			AttackRelationShip = 120;				break;
				default:					AttackRelationShip = 120;				break;
			}
		break;

		// Water vs. X
		case EL_WATER:
			switch (DefenserAttribute) {
				case EL_FIRE:				AttackRelationShip = 120;				break;
				case EL_WATER:				AttackRelationShip = 100;				break;
				case EL_EARTH:				AttackRelationShip = 80;				break;
				case EL_WIND:				AttackRelationShip = 90;				break;
				case EL_DARKNESS:			AttackRelationShip = 110;				break;
				default:					AttackRelationShip = 120;				break;
			}
		break;
		
		// Earth vs. X
		case EL_EARTH:
			switch (DefenserAttribute) {
				case EL_FIRE:				AttackRelationShip = 110;				break;
				case EL_WATER:				AttackRelationShip = 120;				break;
				case EL_EARTH:				AttackRelationShip = 100;				break;
				case EL_WIND:				AttackRelationShip = 80;				break;
				case EL_DARKNESS:			AttackRelationShip = 90;				break;
				default:					AttackRelationShip = 120;				break;
			}
		break;

		// Wind vs. X
		case EL_WIND:
			switch (DefenserAttribute) {
				case EL_FIRE:				AttackRelationShip = 90;				break;
				case EL_WATER:				AttackRelationShip = 110;				break;
				case EL_EARTH:				AttackRelationShip = 120;				break;
				case EL_WIND:				AttackRelationShip = 100;				break;
				case EL_DARKNESS:			AttackRelationShip = 80;				break;
				default:					AttackRelationShip = 120;				break;
			}
		break;

		// Darkness vs. X
		case EL_DARKNESS:
			switch (DefenserAttribute) {
				case EL_FIRE:				AttackRelationShip = 100;				break;
				case EL_WATER:				AttackRelationShip = 80;				break;
				case EL_EARTH:				AttackRelationShip = 90;				break;
				case EL_WIND:				AttackRelationShip = 110;				break;
				case EL_DARKNESS:			AttackRelationShip = 120;				break;
				default:					AttackRelationShip = 120;				break;
			}
		break;

		// Everything Else
		default:
		return FALSE;
	}

	if (lpObj->Type == OBJ_USER) {
		// Runestone of Anger (Rank 2)
		switch (lpObj->CharacterData->m_PentagramOptions.m_iRuby_2RankOptionNum)	{
			case 1:				if (DefenserAttribute == EL_FIRE) AttackRelationShip += lpObj->CharacterData->m_PentagramOptions.m_iRuby_2RankAddAttackRelationshipRate;					break;
			case 2:				if (DefenserAttribute == EL_WATER) AttackRelationShip += lpObj->CharacterData->m_PentagramOptions.m_iRuby_2RankAddAttackRelationshipRate;					break;
			case 3:				if (DefenserAttribute == EL_EARTH) AttackRelationShip += lpObj->CharacterData->m_PentagramOptions.m_iRuby_2RankAddAttackRelationshipRate;					break;
			case 4:				if (DefenserAttribute == EL_WIND) AttackRelationShip += lpObj->CharacterData->m_PentagramOptions.m_iRuby_2RankAddAttackRelationshipRate;					break;
			case 5:				if (DefenserAttribute == EL_DARKNESS) AttackRelationShip += lpObj->CharacterData->m_PentagramOptions.m_iRuby_2RankAddAttackRelationshipRate;				break;
		}
	}

	TotalDamageWithRelationShip = AttackRelationShip * TotalDamage / 100;

	BOOL StrongerElement = FALSE;

	// Defender Relationships
	switch (DefenserAttribute) {
		// X vs. Fire
		case EL_FIRE: {
			switch (AttackerAttribute) {
				case EL_FIRE:					DefenseRelationShip = 100;														break;
				case EL_WATER:					StrongerElement = TRUE;					DefenseRelationShip = 80;				break;
				case EL_EARTH:					StrongerElement = TRUE;					DefenseRelationShip = 90;				break;
				case EL_WIND:					DefenseRelationShip = 110;														break;
				case EL_DARKNESS:				DefenseRelationShip = 120;														break;
				default:						return FALSE;
			}
		}
		break;

		// X vs. Water
		case EL_WATER: {
			switch (AttackerAttribute) {
				case EL_FIRE:					DefenseRelationShip = 120;														break;
				case EL_WATER:					DefenseRelationShip = 100;														break;
				case EL_EARTH:					StrongerElement = TRUE;					DefenseRelationShip = 80;				break;
				case EL_WIND:					StrongerElement = TRUE;					DefenseRelationShip = 90;				break;
				case EL_DARKNESS:				DefenseRelationShip = 110;														break;
				default:						return FALSE;
			}
		}
		break;

		// X vs. Earth
		case EL_EARTH: {
			switch (AttackerAttribute) {
				case EL_FIRE:					DefenseRelationShip = 110;														break;
				case EL_WATER:					DefenseRelationShip = 120;														break;
				case EL_EARTH:					StrongerElement = TRUE;					DefenseRelationShip = 80;				break;
				case EL_WIND:					DefenseRelationShip = 100;														break;
				case EL_DARKNESS:				StrongerElement = TRUE;					DefenseRelationShip = 90;				break;
				default:						return FALSE;
			}
		}
		break;

		// X vs. Wind
		case EL_WIND: {
			switch (AttackerAttribute) {
				case EL_FIRE:					StrongerElement = TRUE;					DefenseRelationShip = 90;				break;
				case EL_WATER:					DefenseRelationShip = 110;														break;
				case EL_EARTH:					DefenseRelationShip = 120;														break;
				case EL_WIND:					DefenseRelationShip = 100;														break;
				case EL_DARKNESS:				StrongerElement = TRUE;					DefenseRelationShip = 80;				break;
				default:						return FALSE;
			}
		}
		break;

		// X vs. Darkness
		case EL_DARKNESS: {
			switch (AttackerAttribute) {
				case EL_FIRE:					StrongerElement = TRUE;					DefenseRelationShip = 80;				break;
				case EL_WATER:					StrongerElement = TRUE;					DefenseRelationShip = 90;				break;
				case EL_EARTH:					DefenseRelationShip = 110;														break;
				case EL_WIND:					DefenseRelationShip = 120;														break;
				case EL_DARKNESS:				DefenseRelationShip = 100;														break;
				default:						return FALSE;
			}
		}
		break;

		// Everything Else
		default: 
			if (AttackerAttribute < 0 || AttackerAttribute > 5)	return FALSE;
			
			DefenseRelationShip = 80;
		break;
	}

	if (lpTargetObj->Type == OBJ_USER && lpObj->Type == OBJ_USER) {
		// Runestone of Blessing (Rank 2)
		switch (lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_2RankOptionNum) {
			case 1:				if (AttackerAttribute == EL_FIRE) DefenseRelationShip += lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_2RankAddDefenseRelationshipRate;					break;
			case 2:				if (AttackerAttribute == EL_WATER) DefenseRelationShip += lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_2RankAddDefenseRelationshipRate;					break;
			case 3:				if (AttackerAttribute == EL_EARTH) DefenseRelationShip += lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_2RankAddDefenseRelationshipRate;					break;
			case 4:				if (AttackerAttribute == EL_WIND) DefenseRelationShip += lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_2RankAddDefenseRelationshipRate;					break;
			case 5:				if (AttackerAttribute == EL_DARKNESS) DefenseRelationShip += lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_2RankAddDefenseRelationshipRate;				break;
		}
	}

	TotalDefenseWithRelationShip = DefenseRelationShip * TotalDefense / 100;

	if (TotalDamageWithRelationShip - TotalDefenseWithRelationShip > 0 && TotalDamageWithRelationShip > 0)	{
		if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) TotalDamageWithRelationShip = TotalDamageWithRelationShip * g_ConfigRead.calc.ElementalPvPDamageRate[lpObj->Class][lpTargetObj->Class] / 100;
		else if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_MONSTER) TotalDamageWithRelationShip = TotalDamageWithRelationShip * g_ConfigRead.calc.ElementalPvMDamageRate[lpObj->Class] / 100;

		TotalDamageWithRelationShip -= TotalDefenseWithRelationShip;

		if (bExcellentDamageOption == FALSE && bCriticalDamageOption == TRUE) *DamageType2 = 7;
		if (bExcellentDamageOption == TRUE)	{
			TotalDamageWithRelationShip *= 1.2;
			*DamageType2 = 6;
		}

		if (bDoubleDamageOption == TRUE) {
			TotalDamageWithRelationShip *= 2;
			*DamageType2 = 6;
		}

		// Runestone of Blessing (Rank 5)
		if (lpTargetObj->Type == OBJ_USER) {
			if (lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_5RankOptionNum == 1 && lpObj->Type == OBJ_USER) TotalDamageWithRelationShip -= lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_5RankMinusTargetDamageRate * TotalDamageWithRelationShip / 100;
			else if (lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_5RankOptionNum == 2 && lpObj->Type == OBJ_MONSTER) TotalDamageWithRelationShip -= lpTargetObj->CharacterData->m_PentagramOptions.m_iSapph_5RankMinusTargetDamageRate * TotalDamageWithRelationShip / 100;
		}
	} else {
		if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) TotalDamageWithRelationShip = (lpObj->Level + lpObj->CharacterData->MasterLevel) / ((rand() % 31) + 70);
		else if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_MONSTER) {
			if (lpObj->Level - lpTargetObj->Level > 10) TotalDamageWithRelationShip = (lpObj->Level + lpObj->CharacterData->MasterLevel - lpTargetObj->Level) / ((rand() % 31) + 70);
			else TotalDamageWithRelationShip = 0;
		} else if (lpObj->Type == OBJ_MONSTER && lpTargetObj->Type == OBJ_USER) TotalDamageWithRelationShip = 0;
	}

	// Elemental Seal Bonus (+5% Resistance To Stronger Elements)
	if (lpTargetObj->Type == OBJ_USER && lpTargetObj->CharacterData->m_PentagramOptions.m_isAddResistByStrongRelationShip == true && (rand() % 100) < 5) {
		switch (AttackerAttribute) {
			case EL_FIRE:			if (DefenserAttribute == EL_WIND || DefenserAttribute == EL_DARKNESS) return FALSE;												break;
			case EL_WATER:			if (DefenserAttribute == EL_DARKNESS || DefenserAttribute == EL_FIRE) return FALSE;												break;
			case EL_EARTH:			if (DefenserAttribute == EL_FIRE || DefenserAttribute == EL_WATER) return FALSE;												break;
			case EL_WIND:			if (DefenserAttribute == EL_WATER || DefenserAttribute == EL_EARTH)	return FALSE;												break;
			default:				if (AttackerAttribute == EL_DARKNESS && (DefenserAttribute == EL_EARTH || DefenserAttribute == EL_WIND)) return FALSE;			break;
		}
	}

	// Elemental Seal Bonus (+5%% To Elemental Resistance)
	if (lpTargetObj->Type == OBJ_USER && lpTargetObj->CharacterData->m_PentagramOptions.m_isAddResistByPentaAttack == true && (rand() % 100) < 5) return FALSE;
	if (lpObj->Type == OBJ_USER) {
		// Elemental Hit Rating
		switch (lpObj->Class) {
			case CLASS_KNIGHT:			AttackSuccessRate = lpObj->CharacterData->Dexterity * 4.5 + 3 * (lpObj->Level + lpObj->CharacterData->MasterLevel);				break;
			case CLASS_WIZARD:			AttackSuccessRate = lpObj->CharacterData->Dexterity * 4 + 3 * (lpObj->Level + lpObj->CharacterData->MasterLevel);				break;
			case CLASS_ELF:				AttackSuccessRate = lpObj->CharacterData->Dexterity * 0.6 + 3 * (lpObj->Level + lpObj->CharacterData->MasterLevel);				break;
			case CLASS_SUMMONER:		AttackSuccessRate = lpObj->CharacterData->Dexterity * 3.5 + 3 * (lpObj->Level + lpObj->CharacterData->MasterLevel);				break;
			case CLASS_MAGICGLADIATOR:	AttackSuccessRate = lpObj->CharacterData->Dexterity * 3.5 + 3 * (lpObj->Level + lpObj->CharacterData->MasterLevel);				break;
			case CLASS_DARKLORD:		AttackSuccessRate = lpObj->CharacterData->Dexterity * 4 + 3 * (lpObj->Level + lpObj->CharacterData->MasterLevel);				break;
			case CLASS_RAGEFIGHTER:		AttackSuccessRate = lpObj->CharacterData->Dexterity * 3.6 + 3 * (lpObj->Level + lpObj->CharacterData->MasterLevel);				break;
			default:					AttackSuccessRate = lpObj->CharacterData->Dexterity * 2.25 + 3 * (lpObj->Level + lpObj->CharacterData->MasterLevel);			break;
		}

		// Runestone of Wrath (Rank 1)
		AttackSuccessRate += AttackSuccessRate * lpObj->CharacterData->m_PentagramOptions.m_iEme_1RankAddAttackRate;

		// Runestone of Wrath (Rank 2)
		switch (lpObj->CharacterData->m_PentagramOptions.m_iEme_2RankOptionNum) {
			case 1:				if (DefenserAttribute == EL_FIRE) AttackSuccessRate += AttackSuccessRate * lpObj->CharacterData->m_PentagramOptions.m_iEme_2RankAddAttackRelationshipRate;					break;
			case 2:				if (DefenserAttribute == EL_WATER) AttackSuccessRate += AttackSuccessRate * lpObj->CharacterData->m_PentagramOptions.m_iEme_2RankAddAttackRelationshipRate;					break;
			case 3:				if (DefenserAttribute == EL_EARTH) AttackSuccessRate += AttackSuccessRate * lpObj->CharacterData->m_PentagramOptions.m_iEme_2RankAddAttackRelationshipRate;					break;
			case 4:				if (DefenserAttribute == EL_WIND) AttackSuccessRate += AttackSuccessRate * lpObj->CharacterData->m_PentagramOptions.m_iEme_2RankAddAttackRelationshipRate;					break;
			case 5:				if (DefenserAttribute == EL_DARKNESS) AttackSuccessRate += AttackSuccessRate * lpObj->CharacterData->m_PentagramOptions.m_iEme_2RankAddAttackRelationshipRate;				break;
		}
	} else AttackSuccessRate = lpObj->m_iPentagramAttackRating;

	// Elemental Evasion Rating
	if (lpTargetObj->Type == OBJ_USER) {
		switch (lpTargetObj->Class)	{
			case CLASS_KNIGHT:			DefenseSuccessRate = lpTargetObj->CharacterData->Dexterity / 2 + 2 * (lpTargetObj->Level + lpTargetObj->CharacterData->MasterLevel);				break;
			case CLASS_WIZARD:			DefenseSuccessRate = lpTargetObj->CharacterData->Dexterity / 4 + 2 * (lpTargetObj->Level + lpTargetObj->CharacterData->MasterLevel);				break;
			case CLASS_ELF:				DefenseSuccessRate = lpTargetObj->CharacterData->Dexterity / 4 + 2 * (lpTargetObj->Level + lpTargetObj->CharacterData->MasterLevel);				break;
			case CLASS_SUMMONER:		DefenseSuccessRate = lpTargetObj->CharacterData->Dexterity / 2 + 2 * (lpTargetObj->Level + lpTargetObj->CharacterData->MasterLevel);				break;
			case CLASS_MAGICGLADIATOR:	DefenseSuccessRate = lpTargetObj->CharacterData->Dexterity / 4 + 2 * (lpTargetObj->Level + lpTargetObj->CharacterData->MasterLevel);				break;
			case CLASS_DARKLORD:		DefenseSuccessRate = lpTargetObj->CharacterData->Dexterity / 2 + 2 * (lpTargetObj->Level + lpTargetObj->CharacterData->MasterLevel);				break;
			case CLASS_RAGEFIGHTER:		DefenseSuccessRate = lpTargetObj->CharacterData->Dexterity / 5 + 1.5 * (lpTargetObj->Level + lpTargetObj->CharacterData->MasterLevel);				break;
			default:					DefenseSuccessRate = lpTargetObj->CharacterData->Dexterity / 3 + 2 * (lpTargetObj->Level + lpTargetObj->CharacterData->MasterLevel);				break;
		}

		// Runestone of Divinity (Rank 1)
		DefenseSuccessRate += DefenseSuccessRate * lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_1RankAddDefenseSuccessRate / 100;

		// Runestone of Divinity (Rank 2)
		switch (lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_2RankOptionNum) {
			case 1:				if (AttackerAttribute == EL_FIRE) DefenseSuccessRate += DefenseSuccessRate * lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_2RankAddDefenseRelationshipRate;						break;
			case 2:				if (AttackerAttribute == EL_WATER) DefenseSuccessRate += DefenseSuccessRate * lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_2RankAddDefenseRelationshipRate;					break;
			case 3:				if (AttackerAttribute == EL_EARTH) DefenseSuccessRate += DefenseSuccessRate * lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_2RankAddDefenseRelationshipRate;					break;
			case 4:				if (AttackerAttribute == EL_WIND) DefenseSuccessRate += DefenseSuccessRate * lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_2RankAddDefenseRelationshipRate;						break;
			case 5:				if (AttackerAttribute == EL_DARKNESS) DefenseSuccessRate += DefenseSuccessRate * lpTargetObj->CharacterData->m_PentagramOptions.m_iTopa_2RankAddDefenseRelationshipRate;					break;
		}
	} else DefenseSuccessRate = lpTargetObj->m_iPentagramDefenseRating;

	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_ELEMENTALSEAL_DARKNESS)) {
		int darkValue = 0;

		BUFF_GetBuffEffectValue(lpObj, BUFFTYPE_ELEMENTALSEAL_DARKNESS, &darkValue, 0);

		if (darkValue > 0) AttackSuccessRate -= AttackSuccessRate * darkValue / 100.0;
	}

	if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_EARTH)) {
		int earthValue = 0;

		BUFF_GetBuffEffectValue(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_EARTH, &earthValue, 0);

		if (earthValue > 0) DefenseSuccessRate -= DefenseSuccessRate * earthValue / 100.0;
	}

	BOOL bAllMiss = FALSE;

	// Result
	if (lpObj->PartyNumber < 0 || lpTargetObj->PartyNumber < 0 || lpObj->PartyNumber != lpTargetObj->PartyNumber) {
		if (AttackSuccessRate < DefenseSuccessRate || AttackSuccessRate == 0) bAllMiss = TRUE;
		if (bAllMiss) return FALSE;
		else if (rand() % AttackSuccessRate >= DefenseSuccessRate) return TotalDamageWithRelationShip;
		else return FALSE;
	}

	return FALSE;
}