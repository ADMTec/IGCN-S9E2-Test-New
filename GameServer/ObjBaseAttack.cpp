#include "stdafx.h"
#include "ObjBaseAttack.h"
#include "GameMain.h"
#include "TLog.h"
#include "DevilSquare.h"
#include "BloodCastle.h"
#include "ChaosCastle.h"
#include "IllusionTempleEvent_Renewal.h"
#include "BuffEffectSlot.h"
#include "BuffEffect.h"
#include "LastManStanding.h"
#include "ObjUseSkill.h"
#include "ObjCalCharacter.h"
#include "configread.h"
#include "MapAttribute.h"
#include "MasterLevelSkillTreeSystem.h"
#include "CrywolfAltar.h"
#include "CrywolfStatue.h"
#include "ChaosCastleFinal.h"
#include "MuunSystem.h"

//Review by HermeX
//#include "GameServer.h"

CObjBaseAttack::CObjBaseAttack() {
	return;
}
CObjBaseAttack::~CObjBaseAttack() {
	return;
}
BOOL CObjBaseAttack::CheckAttackArea(LPOBJ lpObj, LPOBJ lpTargetObj) {
	if (lpTargetObj->MapNumber == g_LastManStanding.m_Cfg.iPVPMap) return TRUE;
	if (lpTargetObj->Type == OBJ_USER || lpTargetObj->m_RecallMon >= 0) {
		BYTE attr = MapC[lpTargetObj->MapNumber].GetAttr(lpTargetObj->X, lpTargetObj->Y);

		if ((attr & 1) == 1) return FALSE;
	}

	if (lpObj->Type == OBJ_USER) {
		int iRet = gObjCheckAttackArea(lpObj->m_Index, lpTargetObj->m_Index);

		if (iRet != 0) {
			g_Log.Add("[%s][%s] Try Attack In Not Attack Area (%s,%d,%d) errortype = %d", lpObj->AccountID, lpObj->Name, Lang.GetMap(0, 0 + lpObj->MapNumber), lpObj->X, lpObj->Y, iRet);

			if (bIsIgnorePacketSpeedHackDetect != FALSE) return FALSE;
		}
	}

	return TRUE;
}
BOOL CObjBaseAttack::PkCheck(LPOBJ lpObj, LPOBJ lpTargetObj) {
	if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) {
		if (lpTargetObj->Level <= 5 || lpObj->Level <= 5) return FALSE;
		if (gObjGetRelationShip(lpObj, lpTargetObj) == 2) { // Rivals
			if (g_ConfigRead.server.IsNonPvP() == FALSE) return TRUE;
		}

		if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE) {
			if (lpObj->MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) return TRUE;
		}

		if (g_ArcaBattle.IsArcaBattleServer() == TRUE) return TRUE;
		if (g_ConfigRead.antihack.EnableAttackBlockInSafeZone == TRUE) {
			BYTE btAttr1 = MapC[lpObj->MapNumber].GetAttr(lpObj->X, lpObj->Y);
			BYTE btAttr2 = MapC[lpObj->MapNumber].GetAttr(lpTargetObj->X, lpTargetObj->Y);

			if ((btAttr1 & 1) == 1 || (btAttr2 & 1) == 1) return FALSE;
		}

		if (lpObj->CharacterData->lpGuild != NULL && lpTargetObj->CharacterData->lpGuild != NULL) {
			if (lpObj->CharacterData->lpGuild->WarState == 1 && lpTargetObj->CharacterData->lpGuild->WarState == 1) {
				if (lpObj->CharacterData->lpGuild->Number == lpTargetObj->CharacterData->lpGuild->Number) return FALSE;
			}
		}

		if (DG_MAP_RANGE(lpObj->MapNumber) || IMPERIAL_MAP_RANGE(lpObj->MapNumber)) return false;
		if (IT_MAP_RANGE(lpObj->MapNumber) && IT_MAP_RANGE(lpTargetObj->MapNumber)) return true;
		if (lpObj->MapNumber == MAP_INDEX_DEVILSQUARE_FINAL) return false;
		if (lpObj->MapNumber == g_LastManStanding.m_Cfg.iPVPMap) {
			if (lpObj->CharacterData->RegisterdLMS == TRUE || lpTargetObj->CharacterData->RegisterdLMS == TRUE) {
				if (g_LastManStanding.m_Rooms[lpObj->CharacterData->RegisteredLMSRoom].bState != 3) return false;
			}
		}

		if (gObjTargetGuildWarCheck(lpObj, lpTargetObj) == FALSE && lpTargetObj->CharacterData->lpGuild != NULL && lpTargetObj->CharacterData->lpGuild->WarState != 0) {
			if (lpTargetObj->CharacterData->lpGuild->WarType == 1) {
				if (lpTargetObj->MapNumber != 6) {
					if (!g_ConfigRead.server.IsNonPvP()) return TRUE;
				}
			}

			if (CC_MAP_RANGE(lpTargetObj->MapNumber) == FALSE) return FALSE;
			if (lpTargetObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) return FALSE;
		} else if (CC_MAP_RANGE(lpObj->MapNumber) != FALSE) {
			if (g_ChaosCastle.GetCurrentState(g_ChaosCastle.GetChaosCastleIndex(lpObj->MapNumber)) != 2) return FALSE;
		} else if (lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) {
			if (g_ChaosCastleFinal.GetCurrentState() != 2) return FALSE;
		} else if (g_ConfigRead.server.IsNonPvP() != FALSE) return FALSE;
		if (g_MapAttr.GetPvPState(lpObj->MapNumber) == 1) return FALSE;
		if (DS_MAP_RANGE(lpObj->MapNumber)) return FALSE;
		if (BC_MAP_RANGE(lpObj->MapNumber)) return FALSE;
		if (gObjDuelCheck(lpObj, lpTargetObj) == FALSE) {
			if (gObjDuelCheck(lpTargetObj)) return FALSE;
		}
	}

	return true;
}
BOOL CObjBaseAttack::ATTACK_MissCheck(LPOBJ lpObj, LPOBJ lpTargetObj, int skill, int skillSuccess, int magicsend, BOOL& bAllMiss, BYTE RFAttack) {
	int iAttackRate = 0;

	__try {
		if (lpObj->Type == OBJ_USER && g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 554) >= 10 && CHARACTER_DebuffProc(20)) return TRUE;		// Fist Frenzy

		iAttackRate = 0;

		int iDefenseRate = lpTargetObj->EvasionRating;
		BYTE MSBDamage = 0;	// MonsterSetBase Damage

		if (IT_MAP_RANGE(lpTargetObj->MapNumber) && lpTargetObj->Type == OBJ_USER) {
			if (g_IT_Event.GetIllusionTempleState(lpTargetObj->MapNumber) == 2) {
				if (g_IT_Event.CheckSkillProdection(lpTargetObj->m_nITR_Index, lpTargetObj->MapNumber) == TRUE) {
					GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
					return FALSE;
				}

				if (lpTargetObj->PartyNumber == lpObj->PartyNumber) {
					GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
					return FALSE;
				}
			}
		}

		if (lpObj->Type == OBJ_USER) {	// Miss for Users
			iAttackRate = lpObj->CharacterData->HitRating;
			iAttackRate += lpObj->CharacterData->ANCIENT_HitRating;
		} else iAttackRate = lpObj->HitRating; // Miss for Monsters

		if (lpObj->Type == OBJ_USER) {
			if (lpObj->Class == CLASS_RAGEFIGHTER) iAttackRate += lpObj->CharacterData->Mastery.ML_RF_Accuracy;
			else iAttackRate += lpObj->CharacterData->Mastery.ML_GENERAL_Accuracy;
		}

		if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_FOCUS)) {
			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 577) >= 10) iAttackRate *= 1.1;
		}

		if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_EARTH)) {
			int earthValue = 0;

			BUFF_GetBuffEffectValue(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_EARTH, &earthValue, 0);

			if (earthValue > 0) iDefenseRate -= iDefenseRate * earthValue / 100.0;
		}

		if (BUFF_TargetHasBuff(lpObj, DEBUFFTYPE_BLIND)) {
			int nDecRate = 0;

			BUFF_GetBuffEffectValue(lpObj, DEBUFFTYPE_BLIND, &nDecRate, 0);

			if (nDecRate > 0) iAttackRate -= iAttackRate * nDecRate / 100.0;
		}

		if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_ELEMENTALSEAL_DARKNESS)) {
			int darkValue = 0;

			BUFF_GetBuffEffectValue(lpObj, BUFFTYPE_ELEMENTALSEAL_DARKNESS, &darkValue, 0);

			if (darkValue > 0) iAttackRate -= iAttackRate * darkValue / 100.0;
		}

		if (BUFF_TargetHasBuff(lpObj, DEBUFFTYPE_IMPROVED_DARKNESS)) {
			int darkValue = 0;

			BUFF_GetBuffEffectValue(lpObj, DEBUFFTYPE_IMPROVED_DARKNESS, &darkValue, 0);

			if (darkValue > 0) iAttackRate -= iAttackRate * darkValue / 100.0;
		}

		if (iAttackRate < iDefenseRate) bAllMiss = TRUE;
		if (bAllMiss != FALSE) {
			if ((rand() % 100) >= 5) {
				if (RFAttack) {
					if (lpObj->Class == CLASS_RAGEFIGHTER) {
						if (skill == 261 || skill == 263 || skill == 552 || skill == 555 || skill == 559 || skill == 563) {
							if (RFAttack % 2) MSBDamage |= 0x10;
							else MSBDamage |= 0x20;
						} else {
							if (RFAttack % 4) MSBDamage |= 0x10;
							else MSBDamage |= 0x20;
						}
					} else if (lpObj->Class == CLASS_GROWLANCER) {
						if (skill == 276 || skill == 274 || skill == 277) {
							if (RFAttack % 2) MSBDamage |= 0x10;
							else MSBDamage |= 0x20;
						}
					}
				}

				GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, MSBDamage, 0);

				if (magicsend) GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, skill, lpTargetObj->m_Index, skillSuccess);
				return FALSE;
			}
		} else {
			if ((rand() % iAttackRate) < iDefenseRate) {
				if (RFAttack) {
					if (lpObj->Class == CLASS_RAGEFIGHTER) {
						if (skill == 261 || skill == 263 || skill == 552 || skill == 555 || skill == 559 || skill == 563) {
							if (RFAttack % 2) MSBDamage |= 0x10;
							else MSBDamage |= 0x20;
						} else {
							if (RFAttack % 4) MSBDamage |= 0x10;
							else MSBDamage |= 0x20;
						}
					} else if (lpObj->Class == CLASS_GROWLANCER) {
						if (skill == 276 || skill == 274 || skill == 277) {
							if (RFAttack % 2) MSBDamage |= 0x10;
							else MSBDamage |= 0x20;
						}
					}
				}

				GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, MSBDamage, 0);

				if (magicsend) GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, skill, lpTargetObj->m_Index, skillSuccess);
				return FALSE;
			}
		}
	}
	__except (iAttackRate = 1, 1) {
		g_Log.Add("error2: %s's level is 0", lpObj->Name);
		return FALSE;
	}

	return TRUE;
}
BOOL CObjBaseAttack::ATTACK_MissCheckPvP(LPOBJ lpObj, LPOBJ lpTargetObj, int skill, int skillSuccess, int magicsend, BOOL& bAllMiss, BYTE RFAttack) {
	if (lpObj == NULL) return FALSE;
	if (lpTargetObj == NULL) return FALSE;
	if (lpTargetObj->Type != OBJ_USER || lpObj->Type != OBJ_USER) return FALSE;
	if (lpObj->Type == OBJ_USER && g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 554) >= 10 && CHARACTER_DebuffProc(20)) return TRUE;		// Fist Frenzy

	double iAttackRate = 0;
	double iDefenseRate = 0;
	int AttackLevel = lpObj->Level + lpObj->CharacterData->MasterLevel;
	int DefenseLevel = lpTargetObj->Level + lpTargetObj->CharacterData->MasterLevel;
	int iAttackSuccessRate = 0;

	if (IT_MAP_RANGE(lpTargetObj->MapNumber) && lpTargetObj->Type == OBJ_USER) {
		if (g_IT_Event.GetIllusionTempleState(lpTargetObj->MapNumber) == 2) {
			if (g_IT_Event.CheckSkillProdection(lpTargetObj->m_nITR_Index, lpTargetObj->MapNumber) == TRUE) {
				GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
				return FALSE;
			}

			if (lpTargetObj->PartyNumber == lpObj->PartyNumber) {
				GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, 0, 0);
				return FALSE;
			}
		}
	}

	iAttackRate = lpObj->CharacterData->HitRatingPvP;
	iDefenseRate = lpTargetObj->CharacterData->EvasionRatingPvP;

	if (iAttackRate <= 0.0f || iDefenseRate <= 0.0f || AttackLevel <= 0 || lpTargetObj == 0) return FALSE;

	iAttackRate += lpObj->CharacterData->PvPEquipmentMods.HitRatingPvP;
	iDefenseRate += lpTargetObj->CharacterData->PvPEquipmentMods.EvasionRatingPvP;
	iAttackRate += lpObj->CharacterData->JewelOfHarmonyEffect.HitRatingPvP;
	iDefenseRate += lpTargetObj->CharacterData->JewelOfHarmonyEffect.EvasionRatingPvP;

	if (lpObj->Class == CLASS_RAGEFIGHTER) {
		iAttackRate += lpObj->CharacterData->Mastery.ML_RF_Precision;
		iDefenseRate += lpTargetObj->CharacterData->Mastery.ML_RF_GladiatorsResolve;
	} else {
		iAttackRate += lpObj->CharacterData->Mastery.ML_GENERAL_Precision;
		iDefenseRate += lpTargetObj->CharacterData->Mastery.ML_GENERAL_GladiatorsResolve;
	}

	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_FOCUS)) {
		if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 577) >= 10) iAttackRate *= 1.1;
	}

	if (BUFF_TargetHasBuff(lpObj, DEBUFFTYPE_BLIND)) {
		int nDecRate = 0;

		BUFF_GetBuffEffectValue(lpObj, DEBUFFTYPE_BLIND, &nDecRate, 0);

		if (nDecRate > 0) iAttackRate -= iAttackRate * nDecRate / 100.0;
	}

	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_ELEMENTALSEAL_DARKNESS)) {
		int darkValue = 0;

		BUFF_GetBuffEffectValue(lpObj, BUFFTYPE_ELEMENTALSEAL_DARKNESS, &darkValue, 0);

		if (darkValue > 0) iAttackRate -= iAttackRate * darkValue / 100.0;
	}

	if (BUFF_TargetHasBuff(lpObj, DEBUFFTYPE_IMPROVED_DARKNESS)) {
		int darkValue = 0;

		BUFF_GetBuffEffectValue(lpObj, DEBUFFTYPE_IMPROVED_DARKNESS, &darkValue, 0);

		if (darkValue > 0) iAttackRate -= iAttackRate * darkValue / 100.0;
	}

	if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_EARTH)) {
		int earthValue = 0;

		BUFF_GetBuffEffectValue(lpTargetObj, BUFFTYPE_ELEMENTALSEAL_EARTH, &earthValue, 0);

		if (earthValue > 0) iDefenseRate -= iDefenseRate * earthValue / 100.0;
	}

	float iExpressionA = (iAttackRate * 10000.0f) / (iAttackRate + iDefenseRate);	// #formula
	float iExpressionB = (AttackLevel * 10000.0f) / (AttackLevel + DefenseLevel);	// #formula

	iExpressionA /= 10000.0f;
	iExpressionB /= 10000.0f;
	iAttackSuccessRate = 100.0f * iExpressionA * g_fSuccessAttackRateOption * iExpressionB;

	if ((DefenseLevel - AttackLevel) >= 100) iAttackSuccessRate -= 5;
	else if ((DefenseLevel - AttackLevel) >= 200) iAttackSuccessRate -= 10;
	else if ((DefenseLevel - AttackLevel) >= 300) iAttackSuccessRate -= 15;

	DWORD dwRate = rand() % 100;

	if (dwRate > iAttackSuccessRate) {
		BYTE MsgDamage = 0;

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

		if (magicsend) GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, skill, lpTargetObj->m_Index, skillSuccess);

		GSProtocol.PROTOCOL_DamageSend(lpObj->m_Index, lpTargetObj->m_Index, 0, 0, MsgDamage, 0);

		if (g_bShieldComboMissOptionOn == TRUE) {
			if (lpObj->CharacterData->comboSkill.ProgressIndex >= 0) {
				g_Log.Add("[Shield] ComboSkill Cancel! [%s][%s]", lpObj->AccountID, lpObj->Name);
				lpObj->CharacterData->comboSkill.Init();
			}
		}

		return FALSE;
	}

	return TRUE;
}
void CObjBaseAttack::ATTACK_SkillDebuffProc(LPOBJ lpObj, LPOBJ lpTargetObj, int skill, int skillLevel, int AttackDamage, int cSkill) {
	if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE) {
		if (lpTargetObj->Type != OBJ_USER) {
			if (lpTargetObj->Class == 277 || lpTargetObj->Class == 283 || lpTargetObj->Class == 288 || lpTargetObj->Class == 278 || lpTargetObj->Class == 215 || lpTargetObj->Class == 216 || lpTargetObj->Class == 217 || lpTargetObj->Class == 218 || lpTargetObj->Class == 219) return;
			if (CRYWOLF_ALTAR_CLASS_RANGE(lpTargetObj->Class) != FALSE || CRYWOLF_STATUE_CHECK(lpTargetObj->Class) != FALSE) return;
		}
	}

	if (lpTargetObj->Type == OBJ_USER) {
		if (lpTargetObj->Class == CLASS_RAGEFIGHTER) {
			if (lpTargetObj->CharacterData->Mastery.iMpsImmuneRate_Monk > 0.0 && lpTargetObj->CharacterData->Mastery.iMpsImmuneRate_Monk >= (rand() % 100) && !BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_INVISIBLE2)) {
				GSProtocol.PROTOCOL_MagicAttackNumberSend(lpTargetObj, 323, lpTargetObj->m_Index, 1);
				BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_INVISIBLE2, 0, 0, 0, 0, 5);
			}
		} else {
			if (lpTargetObj->CharacterData->Mastery.iMpsImmuneRate > 0.0 && lpTargetObj->CharacterData->Mastery.iMpsImmuneRate >= (rand() % 100) && !BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_INVISIBLE2)) {
				GSProtocol.PROTOCOL_MagicAttackNumberSend(lpTargetObj, 323, lpTargetObj->m_Index, 1);
				BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_INVISIBLE2, 0, 0, 0, 0, 5);
			}
		}
	}

	int BuffEffectType = 0;										// Additional Buff Effect Type
	int BuffEffectValue = 0;									// Additional Buff Effect Value
	int Distance = 0;											// Knockback Distance
	int Duration[6] = {0, 0, 0, 0, 0, 0};						// 0 = Elemental Ailment; 1 = Skill Debuff; 2 = Skill Debuff; 3 = Skill Debuff; 4 = Skill Debuff; 5 = Item/Buff Effect;
	int Type[6] = {-1, -1, -1, -1, -1, -1};						// 0 = Elemental Ailment; 1 = Skill Debuff; 2 = Skill Debuff; 3 = Skill Debuff; 4 = Skill Debuff; 5 = Item/Buff Effect;
	int Value[6] = {0, 0, 0, 0, 0, 0};							// 0 = Elemental Ailment; 1 = Skill Debuff; 2 = Skill Debuff; 3 = Skill Debuff; 4 = Skill Debuff; 5 = Item/Buff Effect;
	float Rate[6] = {20, 0, 0, 0, 0, 0};						// 0 = Elemental Ailment; 1 = Skill Debuff; 2 = Skill Debuff; 3 = Skill Debuff; 4 = Skill Debuff; 5 = Item/Buff Effect;
	float durationBonus = 1;									// Elemental Ailments Duration Bonus
	bool IsStack = false;										// Burn Stacking
	bool IsIgnore = false;										// Ignore Resist
	BK_Combo:
	int skillAttr = MagicDamageC.MAGIC_GetSkillAttr(skill);		// Skill Elemental Attribute

	if (lpObj->Type == OBJ_MONSTER) ATTACK_GetMonsterAttackEffect(lpObj, skillAttr, Type[1], Duration[1]);
	// Skill Debuffs
		// Normal Skills
	switch (skill) {
		case MONSTER_SUICIDE:		CHARACTER_KnockbackNormal(lpTargetObj, lpObj);																																		break;		// DG Self-Explosion
		case WEAPON_FALLINGSLASH:
		case WEAPON_LUNGE:
		case WEAPON_UPPERCUT:
		case WEAPON_CYCLONESLASH:
		case WEAPON_FORWARDSLASH:
		case RF_PHOENIXSHOT:
		case 326:
		case 327:
		case 328:
		case 329:
		case 479:					gObjAddMsgSendDelay(lpTargetObj, 2, lpObj->m_Index, 150, 0);																														break;		// Weapon Skills

		case DW_NOVA:
		case DL_EARTHQUAKE:
		case 392:
		case 512:
		case 516:					Type[1] = KNOCKBACK, Rate[1] = 100, Distance = 4;																																	break;		// Nova & Earthquake
		case 79:					if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 520) >= 10) Type[1] = KNOCKBACK, Rate[1] = 25, Distance = 2;																break;		// Burning Echoes Explosion
		case SUM_SPIRIT_OF_FLAMES:	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 451) >= 10) Type[1] = KNOCKBACK, Rate[1] = 30, Distance = 3;
									if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 451) == 20)	IsIgnore = true;																								break;		// Sahamutt
		case SUM_SPIRIT_OF_ANGUISH:	Type[1] = BLEED, Value[1] = AttackDamage * 0.2, Rate[1] = 100, Duration[1] = 10;
									Type[2] = STUN, Rate[2] = lpObj->CharacterData->Mastery.ML_SUM_SpiritOfAnguish, Duration[2] = 4;
									if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 452) == 20) IsIgnore = true;																								break;		// Neil
		case SUM_SPIRIT_OF_TERROR:	Type[1] = FREEZE, Rate[1] = lpObj->CharacterData->Mastery.ML_SUM_SpiritOfTerror, Duration[1] = 2;
									if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 453) >= 10) Type[2] = FROSTBITE, Value[2] = (lpObj->CharacterData->Energy + lpObj->AddEnergy) / 5, Rate[2] = 20, Duration[2] = 15;
									if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 453) == 20)	IsIgnore = true;																								break;		// Phantasm
		case DL_DARKNESS:			if (lpObj->Class == 561) Type[1] = STUN, Rate[1] = 25, Duration[1] = 5;																												break;		// Darkness (Medusa)
		case ELF_ICEARROW:			Type[0] = FREEZE, Duration[0] = gObjUseSkill.m_SkillData.IceArrowTime;																												break;		// Ice Arrow
		case DK_COMBO:				skill = cSkill;
									goto BK_Combo;																																										break;		// DK Combo

		// Mastery Skills
		default: {
			if (g_MasterLevelSkillTreeSystem.CheckMasterLevelSkill(skill)) {
				int MLSkill = g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skill);
				float PrevValue = 0.0;
				double MLSkillValue = g_MasterLevelSkillTreeSystem.ML_GetNodeValue(MagicDamageC.SkillGet(skill), skillLevel);

				if (g_MasterLevelSkillTreeSystem.GetParentSkill1(lpObj->Class, skill) > 0) {
					int PrevLevel = g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, g_MasterLevelSkillTreeSystem.GetParentSkill1(lpObj->Class, skill));

					if (PrevLevel > 0) PrevValue = g_MasterLevelSkillTreeSystem.ML_GetNodeValue(MagicDamageC.SkillGet(g_MasterLevelSkillTreeSystem.GetParentSkill1(lpObj->Class, skill)), PrevLevel);
				}

				switch (MLSkill) {
					// Blade Master
					case 326:
					case 479:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) skillAttr = R_WIND;																													break;	// Tornado Slash
					case 328:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) Type[1] = STUN, Rate[1] = 10, Duration[1] = 2;																						break;	// Leaping Slash
					case 332:			Type[1] = KNOCKBACK, Rate[1] = MLSkillValue, Distance = 2;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) Distance += 1;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) Rate[1] *= 2;																														break;	// Whirlwind Slash
					case 333:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_FLAME, DEFAULT, AttackDamage, FALSE);													break;	// Crushing Blow
					case 337:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_METEOR, DEFAULT, NULL, FALSE);														break;	// Improved Groundslam
					case 339:			Type[1] = BLEED, Value[1] = (lpObj->CharacterData->Strength + lpObj->AddStrength) / 5, Rate[1] = MLSkillValue, Duration[1] = 10;																			break;	// Puncturing Stab
					case 340:			Type[1] = FREEZE, Rate[1] = MLSkillValue, Duration[1] = 3;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 337) == 20) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_METEOR, DEFAULT, NULL, FALSE);
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) Duration[1] += 1;																													break;	// Earthshake
					case 342:			Type[1] = BLEED, Value[1] = (lpObj->CharacterData->Strength + lpObj->AddStrength) / 5, Rate[1] = PrevValue, Duration[1] = 10, Type[2] = STUN, Rate[2] = MLSkillValue, Duration[2] = 2;						break;	// Death Blow
					case 343:			Type[1] = FREEZE, Rate[1] = PrevValue, Duration[1] = 3;
										if (CHARACTER_DebuffProc(MLSkillValue)) BuffEffectType = EFFECTTYPE_DECREASE_ATTACK_SPEED_PERCENT, BuffEffectValue = 30;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 337) == 20) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_METEOR, DEFAULT, NULL, FALSE);
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 340) == 20) Duration[1] += 1;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10)	Type[2] = FROSTBITE, Value[2] = (lpObj->CharacterData->Energy + lpObj->AddEnergy) / 6, Rate[2] = 20, Duration[2] = 15;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20)	durationBonus += 0.2;																												break;	// Arctic Blow
					case 345:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) Type[3] = KNOCKBACK, Rate[3] = 100, Distance = 3;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) Type[4] = STUN, Rate[4] = 10, Duration[4] = 3;
										skill = cSkill;
										goto BK_Combo;																																																break;	// Skilled Fighter
					
					// Grand Master
					case 381:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_METEOR, DEFAULT, NULL, FALSE);														break;	// Searing Flames
					case 382:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) Rate[0] += 10;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20 && BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_SHOCK)) Type[1] = STUN, Rate[1] = 10, Duration[1] = 3;									break;	// Arcane Blast
					case 385:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_ICE, DEFAULT, NULL, FALSE);															break;	// Haunting Spirits
					case 391:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) BuffEffectType = EFFECTTYPE_DECREASE_ATTACK_SPEED_PERCENT, BuffEffectValue = 25;													break;	// Improved Ice Storm
					case 393:			Type[1] = FREEZE, Rate[1] = MLSkillValue, Duration[1] = 3;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 391) == 20) BuffEffectType = EFFECTTYPE_DECREASE_ATTACK_SPEED_PERCENT, BuffEffectValue = 25;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) {
											ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_TWISTER, 0, 0, FALSE);
											ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_POWERWAVE, 0, 0, FALSE);
										}																																																			break;	// Blizzard
					case 394:			Type[1] = STUN, Rate[1] = MLSkillValue, Duration[1] = 3;																																					break;	// Comet
					case 495:			Type[1] = DEBUFFTYPE_EARTHEN_PRISON, Rate[1] = 100, Duration[1] = 5;																																		break;	// Earthen Prison
					case 497:			Type[1] = DEBUFFTYPE_EARTHEN_PRISON, Value[1] = 15, Rate[1] = 100, Duration[1] = 5;																															break;	// Improved Earthen Prison
					
					// High Elf
					case 411:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_METEOR, DEFAULT, NULL, FALSE);														break;	// Improved Barrage
					case 424:			Type[0] = FREEZE, Duration[0] = gObjUseSkill.m_SkillData.IceArrowTime;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) Duration[0] += 2;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) Type[1] = FROSTBITE, Value[1] = (lpObj->CharacterData->Energy + lpObj->AddEnergy) / 4, Rate[1] = 20, Duration[1] = 15;				break;	// Improved Ice Arrow
					case 431:			Type[1] = STUN, Rate[1] = MLSkillValue, Duration[1] = 2;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_TWISTER, DEFAULT, NULL, FALSE);
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20)	Duration[1] += 1;																													break;	// Empowered Barrage

					// Dimension Master
					case 456:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) Type[1] = STUN, Rate[1] = 5, Duration[1] = 2;																						break;	// Improved Lightning Blast

					// Duel Master
					case 480:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) BuffEffectType = EFFECTTYPE_LIGHTNING_MASTERY_DEBUFF;																				break;	// Lightning Mastery
					case 484:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) Rate[0] += 10;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20 && BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_SHOCK)) Type[1] = STUN, Rate[1] = 10, Duration[1] = 3;									break;	// Arcane Blast
					case 482:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_POWERWAVE, DEFAULT, NULL, FALSE);														break;	// Improved Power Slash
					case 486:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_METEOR, DEFAULT, NULL, FALSE);						 								break;	// Searing Flames
					case 489:																																																								// Improved Ice
					case 491:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 489) == 20) BuffEffectType = EFFECTTYPE_DECREASE_ATTACK_SPEED_PERCENT, BuffEffectValue = 20;
										if (skill == 491) Type[1] = FREEZE, Rate[1] = MLSkillValue, Duration[1] = 3;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 491) >= 10) Rate[1] += 5;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 491) == 20) Duration[1] += 1;																														break;	// Freeze
					case 494:			Type[1] = KNOCKBACK, Rate[1] = MLSkillValue, Distance = 2;																																					break;	// Scorching Strike
					case 496:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) Type[1] = STUN, Rate[1] = 5, Duration[1] = 2;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) {
											ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_POWERWAVE, DEFAULT, NULL, FALSE);
											ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_TWISTER, DEFAULT, NULL, FALSE);
										}																																																			break;	// Improved Thunder Storm

					// Lord Emperor
					case 514:			Type[1] = STUN, Rate[1] = MLSkillValue, Duration[1] = 2;																																					break;	// Fireblast
					case 512:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) Distance += 1;																														break;	// Improved Earthquake
					case 516:			Type[2] = STUN, Rate[2] = MLSkillValue, Duration[2] = 5;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 512) == 20) Distance += 1;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) Rate[2] += 10;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20 && BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_TRAUMA)) Duration[2] *= 2;																break;	// Warstomp

					// Fist Master
					case 551:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) Type[1] = STUN, Rate[1] = 1, Duration[1] = 2;																						break;	// Improved Fist Flurry
					case 552:			if (CHARACTER_DebuffProc(1) && g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_BLACKOUT, EFFECTTYPE_BLIND, 0, 0, 0, 3);						break;	// Improved Beast Uppercut
					case 554:			Type[1] = STUN, Rate[1] = MLSkillValue + 1, Duration[1] = 2;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) Duration[1] += 1;																													break;	// Fist Frenzy
					case 555: {
										int tempDur = (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) ? 4 : 3;
										if (CHARACTER_DebuffProc(MLSkillValue + 1) && g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_BLACKOUT, EFFECTTYPE_BLIND, 20, 0, 0, tempDur);
					}																																																								break;	// Savage Flurry
					case 559:																																																								// Improved Dark Side
					case 563:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 559) == 20) skillAttr = R_COLD;																														break;	// Dark Reach
					case 561:			ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_FLAME, MLSkillValue, AttackDamage, FALSE);																													break;	// Scorching Blow
					case 562:			Type[2] = FROSTBITE, Value[2] = (lpObj->CharacterData->Energy + lpObj->AddEnergy) / 6, Rate[2] = MLSkillValue, Duration[2] = 15;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) Type[1] = FREEZE, Rate[1] = MLSkillValue, Duration[1] = 2;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) Duration[1] += 1;																													break;	// Freezing Strikes
					case 564:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_METEOR, DEFAULT, NULL, FALSE);														break;	// Improved Dragon Fist
					case 566:			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 564) == 20) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_METEOR, DEFAULT, NULL, FALSE);
										Type[1] = STUN, Rate[1] = MLSkillValue, Duration[1] = 5;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) Duration[1] += 2;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) Rate[1] += 10;																														break;	// Dragon Strike
				}
			}
		}
	}

	if (lpObj->Type == OBJ_USER) {
		// Items
		CItem* Right = &lpObj->pInventory[0];
		CItem* Left = &lpObj->pInventory[1];

		if (lpObj->CharacterData->ANCIENT_ElementalAilmentsDuration > 0) durationBonus += lpObj->CharacterData->ANCIENT_ElementalAilmentsDuration / 100.00;												// Ancient Set Bonus
		if (Right->GetDetailItemType() == ITEM_MACE || Left->GetDetailItemType() == ITEM_MACE) {																										// Mace Expertise DK
			if (lpObj->CharacterData->Mastery.ML_DK_MaceExpertise > 0.0) {
				Type[5] = STUN;
				Rate[5] = lpObj->CharacterData->Mastery.ML_DK_MaceExpertise;
				Duration[5] = 2;
			}
		}

		if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 455) == 20 && CHARACTER_DebuffProc(50) && Left->GetDetailItemType() == ITEM_SUMMONER_BOOK) {										// Improved Chain Lightning
			switch (Left->m_Type) {
				case ITEMGET(5, 21):			skillAttr = R_FIRE;				break;		// Book of Flames
				case ITEMGET(5, 22):			skillAttr = R_EARTH;			break;		// Book of Anguish
				case ITEMGET(5, 23):			skillAttr = R_COLD;				break;		// Book of Terror
			}
		}

		if (((Right->m_IsValidItem != false && Right->IsItem()) || (Left->m_IsValidItem != false && Left->IsItem())) && skillLevel != DEFAULT) {													// Weapon Elemental Attributes
			if (Right->m_Type == ITEMGET(2, 4) || Left->m_Type == ITEMGET(2, 4)) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_ICE, DEFAULT, NULL, FALSE);												// Crystal Mace
			if (Right->m_Type == ITEMGET(2, 5) || Left->m_Type == ITEMGET(2, 5)) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_ICE, DEFAULT, NULL, FALSE);												// Crystal Sword
			if (Right->m_Type == ITEMGET(0, 14) || Left->m_Type == ITEMGET(0, 14)) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_LIGHTNING, DEFAULT, NULL, FALSE);										// Lightning Sword
			if (Right->m_Type == ITEMGET(0, 18) || Left->m_Type == ITEMGET(0, 18)) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_LIGHTNING, DEFAULT, NULL, FALSE);										// Thunder Blade
			if (Right->m_Type == ITEMGET(2, 20) || Left->m_Type == ITEMGET(2, 20)) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_METEOR, DEFAULT, NULL, FALSE);											// Steel Crusher
			if (Right->m_Type == ITEMGET(0, 26) || Left->m_Type == ITEMGET(0, 26)) ATTACK_SkillDebuffProc(lpObj, lpTargetObj, DW_FLAME, DEFAULT, AttackDamage, FALSE);									// Fire Blade
		}

		// Masteries
		if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 387) >= 10) durationBonus += 0.3;																										// Rot DW
		if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 389) == 20) Type[2] = FROSTBITE, Value[2] = (lpObj->CharacterData->Energy + lpObj->AddEnergy) / 5,  Rate[2] = 20, Duration[2] = 15;		// Ice Mastery DW
		if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 378) == 20 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 483) == 20) IsStack = true;											// Fire Mastery DW/MG
	}

	// Buff Effects
	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_EMPOWER)) {
		int value = 0;

		BUFF_GetBuffEffectValue(lpObj, BUFFTYPE_EMPOWER, NULL, &value);
		Type[5] = STUN;
		Rate[5] += value;
		Duration[5] = 2;
	}

	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_BLOOD_LUST)) {
		Type[3] = BLEED;
		Value[3] = ((int)lpObj->CharacterData->Strength + lpObj->AddStrength + (int)lpObj->CharacterData->Vitality + lpObj->AddVitality) / 8;
		Rate[3] = 10;
		Duration[3] = 10;
	}

	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_IMPROVED_BLOOD_LUST)) {
		int value = 0;

		if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 567) == 20) value = 125;

		Type[3] = BLEED;
		Value[3] = ((int)lpObj->CharacterData->Strength + lpObj->AddStrength + (int)lpObj->CharacterData->Vitality + lpObj->AddVitality) / 8.00 * (lpObj->CharacterData->Mastery.ML_RF_ImprovedBloodlust + value) / 100.00;
		Rate[3] = 10 + lpObj->CharacterData->Mastery.ML_RF_ImprovedBloodlust;
		Duration[3] = 10;
	}

	// Knockback/Stun/Bleed
	for (int x = 0; x <= 5; x++) {
		int iAccessoryEffect = 0;

		switch (Type[x]) {
			case KNOCKBACK:							if (CHARACTER_DebuffProc(Rate[x])) CHARACTER_KnockbackDistance(lpTargetObj, lpObj, Distance);							break;
			case STUN:								if (lpTargetObj->Type == OBJ_USER) iAccessoryEffect = lpTargetObj->CharacterData->C_StunResistance;
				if (lpObj->Type == OBJ_USER && lpObj->CharacterData->ANCIENT_StunDuration > 0) Duration[x] += lpObj->CharacterData->ANCIENT_StunDuration;
				if (!CHARACTER_PvPResistanceProc(iAccessoryEffect)) {
					if (CHARACTER_DebuffProc(Rate[x])) {
						if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 524) >= 10 && BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_IMPROVED_STONESKIN)) break;
						else {
							if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_STUN)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_STUN);

							BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_STUN, 0, 0, 0, 0, Duration[x]);
							CHARACTER_Immobilize(lpTargetObj->m_Index, lpTargetObj->X, lpTargetObj->Y);

							if (skill == 382 || skill == 484) {
								BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_SHOCK);
								return;
							}
						}
					}
				}																																							break;
			case BLEED:								if (!BUFF_IsStrongerBuff(lpTargetObj, DEBUFFTYPE_HEMORRHAGE, Value[x], 0)) {
				if (CHARACTER_DebuffProc(Rate[x])) {
					if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 524) >= 10 && BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_IMPROVED_STONESKIN)) break;
					else {
						lpTargetObj->lpAttackObj = lpObj;

						if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_HEMORRHAGE)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_HEMORRHAGE);

						BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_HEMORRHAGE, EFFECTTYPE_DOT_GENERAL, Value[x], 0, 0, Duration[x]);
					}
				}
			}																																								break;
		}
	}

	// Elemental Attribute Debuffs
	switch (skillAttr) {
		case R_COLD:
			if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_BURN) && (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 311) == 20 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 589) == 20)) break;
			if (IsIgnore) goto ignore_cold;
			if (!CHARACTER_ResistanceProc(lpTargetObj, R_COLD)) {
				ignore_cold:
				for (int x = 0; x <= 5; x++) {
					if (x > 0 && Type[x] == DEFAULT) continue;
					if (Type[x] == FREEZE && (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 311) >= 10 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 589) >= 10)) Rate[x] -= 3;
					if (CHARACTER_DebuffProc(Rate[x])) {
						lpTargetObj->lpAttackObj = lpObj;

						switch (skill) {
							case 0:					break;
							default:
								switch (Type[x]) {
									case DEFAULT:
										Duration[x] = 10 * durationBonus;
										lpTargetObj->DelayLevel = 1;
										lpTargetObj->m_SkillNumber = skill;

										if (!BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_FROSTBITE)) {
											if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_CHILL)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_CHILL);
											if (skill == 232 || skill == 262 || skill == 337 || skill == 340 || skill == 343 || skill == 558 || skill == 562) lpTargetObj->DelayActionTime = 1000;
											else lpTargetObj->DelayActionTime = 800;

											BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_CHILL, EFFECTTYPE_REDUCE_MOVE_SPEED, 0, BuffEffectType, BuffEffectValue, Duration[x]);

											if (BuffEffectValue > 0) gObjCalCharacter.CHARACTER_Calc(lpTargetObj->m_Index);
										}
									break;

									case FROSTBITE:
										Duration[x] *= durationBonus;
										lpTargetObj->lpAttackObj = lpObj;
										lpTargetObj->m_SkillNumber = skill;

										if ((BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_CHILL) || BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_FREEZE)) && !BUFF_IsStrongerBuff(lpTargetObj, DEBUFFTYPE_FROSTBITE, Value[x], 0)) {
											if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_CHILL)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_CHILL);
											if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_FROSTBITE)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_FROSTBITE);

											BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_FROSTBITE, EFFECTTYPE_DOT_GENERAL, Value[x], BuffEffectType, BuffEffectValue, Duration[x]);

											if (BuffEffectValue > 0) gObjCalCharacter.CHARACTER_Calc(lpTargetObj->m_Index);
										}
									break;

									case FREEZE:
										Duration[x] *= durationBonus;

										if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_ICE_ARROW)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_ICE_ARROW);
										if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_FREEZE)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_FREEZE);
										if (skill == 51 || skill == 424) BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_ICE_ARROW, 0, 0, 0, 0, Duration[x]);
										else BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_FREEZE, 0, 0, 0, 0, Duration[x]);

										lpTargetObj->PathCount = 0;
										lpTargetObj->PathStartEnd = 0;
										CHARACTER_Immobilize(lpTargetObj->m_Index, lpTargetObj->X, lpTargetObj->Y);
									break;
								}
							break;
						}
					}
				}
			}
		break;

		case R_POISON:
			if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_SOUL_SATURATION) && (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 304) == 20 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 582) == 20)) break;
			if (!CHARACTER_ResistanceProc(lpTargetObj, R_POISON)) {
				if (CHARACTER_DebuffProc(Rate[0])) {
					Value[0] = 4;
					Duration[0] = 30 * durationBonus;
					lpTargetObj->lpAttackObj = lpObj;
					lpTargetObj->m_SkillNumber = skill;

					if (skill == 38 || skill == 387 || skill == 427 || skill == 434) Value[0] = 5;
					if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_POISON)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_POISON);
					switch (skill) {
						// Improved Poison
						case 384:				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, skill) == 20) BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_POISON, EFFECTTYPE_DOT_IMPROVED_POISON, Value[0], 0, 0, Duration[0]);
												else BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_POISON, EFFECTTYPE_DOT_POISON, Value[0], 0, 0, Duration[0]);																																	break;
						// Rot
						case 387:				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, skill) == 20) BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_POISON, EFFECTTYPE_DOT_ROT, Value[0], 0, 0, Duration[0]);
												else BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_POISON, EFFECTTYPE_DOT_POISON, Value[0], 0, 0, Duration[0]);																																	break;
						// Corrosive Arrow
						case 434: {
							double MLSkillValue = g_MasterLevelSkillTreeSystem.ML_GetNodeValue(MagicDamageC.SkillGet(skill), skillLevel);

							Value[0] += Value[0] * MLSkillValue / 100.00;
							if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, skill) >= 10) BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_POISON, EFFECTTYPE_DOT_ROT, Value[0], 0, 0, Duration[0]);
							if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, skill) == 20) BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_POISON, EFFECTTYPE_DOT_ROT, Value[0], EFFECTTYPE_DECREASE_DEFENSE, lpTargetObj->Defense * 35 / 100, Duration[0]);
						}																																																																		break;
						// Everything Else
						default:				BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_POISON, EFFECTTYPE_DOT_POISON, Value[0], 0, 0, Duration[0]);																																			break;
					}
				}
			}
		break;

		case R_LIGHTNING:
			if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_TRAUMA) && (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 308) == 20 || g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpTargetObj, 586) == 20)) break;
			if (!CHARACTER_ResistanceProc(lpTargetObj, R_LIGHTNING)) {
				if (CHARACTER_DebuffProc(Rate[0])) {
					if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_SHOCK)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_SHOCK);

					Value[0] = lpTargetObj->Defense * 25 / 100;
					Duration[0] = 10 * durationBonus;
					BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_SHOCK, EFFECTTYPE_LIGHTNING_DEBUFF, Value[0], BuffEffectType, 0, Duration[0]);
					BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_LIGHTNINGSHOCK_VISUAL, 0, 0, 0, 0, 1);
				}
			}
		break;

		case R_FIRE:
			if (AttackDamage > 0) {
				if (IsIgnore) goto ignore_fire;
				if (!CHARACTER_ResistanceProc(lpTargetObj, R_FIRE)) {
					if (skill == DW_FLAME && skillLevel > 0) Rate[0] = skillLevel;

					ignore_fire:
					Value[0] = AttackDamage * 0.2;
					Duration[0] = 5 * durationBonus;

					switch (skill) {
						case 79:						if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 520) == 20) Value[0] *= 1.5;													break;		// Burning Echoes Explosion
						case SUM_SPIRIT_OF_FLAMES:		Value[0] = (AttackDamage * 0.4) * (1 + lpObj->CharacterData->Mastery.ML_SUM_SpiritOfFlames / 100.00), Rate[0] = 100;
														if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 448) == 20 && CHARACTER_DebuffProc(15)) Value[0] *= 2;							break;		// Spirit of Flames
						case 388:						if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, skill) == 20 && CHARACTER_DebuffProc(20)) Value[0] *= 2;						break;		// Improved Hellfire
						case 392:						if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, skill) >= 10) Rate[0] += 40;													break;		// Supernova
						case 492:
						case 494:						if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 492) == 20 && CHARACTER_DebuffProc(10)) Value[0] *= 2;
														if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 494) >= 10) Rate[0] += 20;
														if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 494) == 20) Value[0] *= 1.25;													break;		// Scorching Strike
						case 508:
						case 514:						if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 508) == 20) Rate[0] += 10;														break;		// Improved Fireburst
						case 561:						if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, skill) >= 10) Rate[0] += 10;	
														if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, skill) == 20) Value[0] *= 1.35;													break;		// Scorching Blow
					}

					if (lpObj->Type == OBJ_USER) {
						if (lpObj->CharacterData->ANCIENT_BurnChance > 0) Rate[0] += lpObj->CharacterData->ANCIENT_BurnChance;
						if (lpObj->CharacterData->ANCIENT_BurnDamage > 0) {
							float bonus = lpObj->CharacterData->ANCIENT_BurnDamage / 100.00;
 
							Value[0] += Value[0] * bonus;
						}
					}

					if (CHARACTER_DebuffProc(Rate[0])) {
						if (IsStack) {
							int Stack = 0;
							int bonus = 0;

							BUFF_GetBuffEffectValue(lpTargetObj, DEBUFFTYPE_BURN, &bonus, &Stack);

							if (Stack < 3) {
								Value[0] += bonus;
								Stack++;
								lpTargetObj->lpAttackObj = lpObj;
								lpTargetObj->m_SkillNumber = skill;

								if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_BURN)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_BURN);

								BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_BURN, EFFECTTYPE_DOT_GENERAL, Value[0], 0, Stack, Duration[0]);
								BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_FLAMESTRKE_VISUAL, 0, 0, 0, 0, 1);
							}
						} else {
							if (!BUFF_IsStrongerBuff(lpTargetObj, DEBUFFTYPE_BURN, Value[0], 0)) {
								lpTargetObj->lpAttackObj = lpObj;
								lpTargetObj->m_SkillNumber = skill;

								if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_BURN)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_BURN);

								BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_BURN, EFFECTTYPE_DOT_GENERAL, Value[0], 0, 0, Duration[0]);
								BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_FLAMESTRKE_VISUAL, 0, 0, 0, 0, 1);
							}
						}
					}
				}
			}
		break;

		case R_EARTH:
			if (IsIgnore) goto ignore_earth;
			if (!CHARACTER_ResistanceProc(lpTargetObj, R_EARTH)) {
				ignore_earth:
				for (int x = 0; x <= 5; x++) {
					if (x > 0 && Type[x] == DEFAULT) continue;
					if (CHARACTER_DebuffProc(Rate[x])) {
						switch (Type[x]) {
							case DEBUFFTYPE_EARTHEN_PRISON:
								if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_EARTHEN_PRISON)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_EARTHEN_PRISON);

								Duration[x] *= durationBonus;
								BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_EARTHEN_PRISON, 0, Value[x], 0, 0, Duration[x]);
								lpTargetObj->PathCount = 0;
								lpTargetObj->PathStartEnd = 0;
								CHARACTER_Immobilize(lpTargetObj->m_Index, lpTargetObj->X, lpTargetObj->Y);
							break;

							case DEFAULT:
								if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_TRAUMA)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_TRAUMA);

								Value[x] = (lpTargetObj->MaxLife + lpTargetObj->AddLife) * 20 / 100;
								Duration[x] = 10 * durationBonus;
								BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_TRAUMA, EFFECTTYPE_EARTH_DEBUFF, Value[x], 0, 0, Duration[x]);
							break;
						}
					}
				}
			}
		break;

		case R_WIND:
			if (!CHARACTER_ResistanceProc(lpTargetObj, R_WIND)) {
				Rate[0] = 50;
				
				if (CHARACTER_DebuffProc(Rate[0])) CHARACTER_KnockbackNormal(lpTargetObj, lpObj);
			}
		break;

		case R_WATER:
			if (!CHARACTER_ResistanceProc(lpTargetObj, R_WATER)) {
				if (CHARACTER_DebuffProc(Rate[0])) {
					if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_SOUL_SATURATION)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_SOUL_SATURATION);

					Value[0] = (lpTargetObj->MaxMana + lpTargetObj->AddMana) * 30 / 100;
					Duration[0] = 10 * durationBonus;
					BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_SOUL_SATURATION, EFFECTTYPE_WATER_DEBUFF, Value[0], 0, 0, Duration[0]);
				}
			}
		break;
	}
}
void CObjBaseAttack::ATTACK_GetMonsterAttackEffect(LPOBJ lpObj, int& skillAttr, int& Type, int& Duration) {
	switch (lpObj->Class) {
		// Cold
		case 22:
		case 52:
		case 70:
		case 81:
		case 358:
		case 458:
		case 558:				skillAttr = R_COLD;					break;
		case 454:				skillAttr = R_COLD;
								Type = FREEZE;
								Duration = 3;						break;

		// Poison
		case 3:
		case 8:
		case 12:
		case 18:
		case 27:
		case 30:
		case 39:
		case 51:
		case 60:
		case 149:
		case 179:
		case 187:
		case 195:
		case 265:
		case 273:
		case 293:
		case 303:
		case 308:
		case 335:
		case 359:
		case 421:
		case 446:
		case 448:
		case 457:
		case 498:
		case 500:
		case 532:
		case 557:
		case 560:
		case 564:
		case 569:
		case 570:
		case 573:
		case 574:				skillAttr = R_POISON;				break;

		// Lightning
		case 9:
		case 48:
		case 69:
		case 80:
		case 106:
		case 135:
		case 292:
		case 447:				skillAttr = R_LIGHTNING;			break;

		// Fire
		case 2:
		case 35:
		case 37:
		case 38:
		case 43:
		case 53:
		case 54:
		case 57:
		case 58:
		case 59:
		case 61:
		case 63:
		case 66:
		case 67:
		case 75:
		case 77:
		case 83:
		case 89:
		case 95:
		case 102:
		case 112:
		case 118:
		case 124:
		case 130:
		case 143:
		case 151:
		case 291:
		case 433:
		case 444:
		case 449:
		case 489:
		case 490:
		case 491:
		case 494:				skillAttr = R_FIRE;					break;

		// Earth
		case 6:
		case 32:
		case 34:
		case 103:
		case 422:
		case 455:
		case 562:
		case 575:
		case 576:				skillAttr = R_EARTH;				break;

		// Wind
		case 11:
		case 71:
		case 72:
		case 74:
		case 150:
		case 301:
		case 496:				skillAttr = R_WIND;					break;

		// Water
		case 13:
		case 25:
		case 46:
		case 47:
		case 49:
		case 144:
		case 147:
		case 174:
		case 177:
		case 182:
		case 185:
		case 190:
		case 193:
		case 260:
		case 263:
		case 268:
		case 271:
		case 305:
		case 331:
		case 334:				skillAttr = R_WATER;				break;
	}
}
BOOL CObjBaseAttack::ATTACK_DecreaseArrow(LPOBJ lpObj) {
	CItem * Right = &lpObj->pInventory[0];
	CItem * Left  = &lpObj->pInventory[1];

	if ((Right->m_Type >= ITEMGET(4,8) && Right->m_Type <= ITEMGET(4,14)) || Right->m_Type == ITEMGET(4,16) || Right->m_Type == ITEMGET(4,18) || Right->m_Type == ITEMGET(4,19) || Right->m_Type == ITEMGET(4,20) || Right->m_Type == ITEMGET(4,21) || Right->m_Type == ITEMGET(4,22) || Right->m_Type == ITEMGET(4,23) || Right->m_Type == ITEMGET(4,24)) {
		if (Left->m_Type == ITEMGET(4,7)) {
			if (Left->m_Durability < 1.0f) return FALSE;

			Left->m_Durability -= 1.0f;
			GSProtocol.GCItemDurSend(lpObj->m_Index, 1, Left->m_Durability, 0);

			if (Left->m_Durability < 1.0f) {
				lpObj->pInventory[1].Clear();
				GSProtocol.PROTOCOL_InventoryItemDeleteSend(lpObj->m_Index, 1, 0);
			}
		} else return FALSE;
	} else if ((Left->m_Type >= ITEMGET(4,0) && Left->m_Type <= ITEMGET(4,6)) || Left->m_Type == ITEMGET(4,17) || Left->m_Type == ITEMGET(4,20) || Left->m_Type == ITEMGET(4,21) || Left->m_Type == ITEMGET(4,22) || Left->m_Type == ITEMGET(4,23) || Left->m_Type == ITEMGET(4,24)) {
		if (Right->m_Type == ITEMGET(4,15)) {
			if (Right->m_Durability < 1.0f) return FALSE;

			Right->m_Durability -= 1.0f;
			GSProtocol.GCItemDurSend(lpObj->m_Index, 0, Right->m_Durability, 0);

			if (Right->m_Durability < 1.0f) {
				lpObj->pInventory[0].Clear();
				GSProtocol.PROTOCOL_InventoryItemDeleteSend(lpObj->m_Index, 0, 0);
			}
		} else return FALSE;
	}

	return TRUE;
}
int CObjBaseAttack::ATTACK_CalcTargetDefense(LPOBJ lpObj, LPOBJ lpTargetObj, int& MsgDamage, int& iOriginTargetDefense, int skill) {
	int decdef = BUFF_GetTotalBuffEffectValue(lpTargetObj, EFFECTTYPE_DECREASE_DEFENSE);
	float fValue = (float)BUFF_GetTotalBuffEffectValue(lpTargetObj, EFFECTTYPE_DECREASE_AMPLIFY_DAMAGE);
	int iCurseValue;
	int nMuunItemEffectValue = 0;
	int iAccessoryEffect = 0;
	int addBonus = 0;
	float percentBonus = 100;
	float multiBonus = 100;
	float fatalChance = 0;

	multiBonus -= decdef;

	switch (skill) {
		case SUM_SPIRIT_OF_ANGUISH:		if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 449) == 20) fatalChance += 10;																	break;		// Spirit of Anguish
		default: {
			int MLSkill = g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(skill);

			switch (MLSkill) {
				case 329:				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) percentBonus -= 10;																	break;		// Stab
				case 330:				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) fatalChance += 5;																	break;		// Improved Rageful Blow
				case 333:				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 330) == 20) fatalChance += 5;
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20 && BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_BURN)) percentBonus -= 20;				break;		// Crushing Blow
				case 336:				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) percentBonus -= 15;																	break;		// Improved Death Stab
				case 339:																																														// Puncturing Stab
				case 342:				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 336) == 20) percentBonus -= 15;																					
										if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20 && CHARACTER_DebuffProc(10)) percentBonus -= 35;										break;		// Death Blow
				case 416:				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) percentBonus -= 20;																	break;		// Improved Penetration
				case 456:				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) fatalChance += 10;																	break;		// Improved Lightning Blast
				case 509:				if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) >= 10) fatalChance += 10;																	break;		// Improved Force Wave
			}
		}
		break;
	}

	if (lpObj->Type == OBJ_USER) {
		fatalChance += lpObj->CharacterData->ANCIENT_FatalStrikeChance;

		if (lpObj->pInventory[7].IsItem() != FALSE) fatalChance += lpObj->CharacterData->m_WingExcOption.iWingOpIgnoreEnemyDefense;
		if (lpObj->Class == CLASS_RAGEFIGHTER) {
			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 571) == 10) fatalChance += 1;		// Lethal Punches
			if (lpObj->CharacterData->Mastery.ML_RF_FatalBlows > 0.0) fatalChance += lpObj->CharacterData->Mastery.ML_RF_FatalBlows;
		} else if (lpObj->CharacterData->Mastery.ML_GENERAL_FatalBlows > 0.0) fatalChance += lpObj->CharacterData->Mastery.ML_GENERAL_FatalBlows;
	}

	if (lpTargetObj->Type == OBJ_USER) iAccessoryEffect = lpTargetObj->CharacterData->C_FatalStrikeResistance;
	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_FOCUS)) {
		int Value = 0;

		BUFF_GetBuffEffectValue(lpObj, BUFFTYPE_FOCUS, &Value, NULL);
		fatalChance += Value;
	}

	if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER) addBonus += lpTargetObj->CharacterData->PvPEquipmentMods.Defense;
	if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_WEAKNESS)) {
		iCurseValue = 0;
		BUFF_GetBuffEffectValue(lpTargetObj, DEBUFFTYPE_WEAKNESS, &iCurseValue, 0);
		multiBonus -= iCurseValue;
	}

	if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_IMPROVED_WEAKNESS)) {
		iCurseValue = 0;
		BUFF_GetBuffEffectValue(lpTargetObj, DEBUFFTYPE_IMPROVED_WEAKNESS, &iCurseValue, NULL);
		multiBonus -= iCurseValue;
	}

	if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_AMPLIFY_DAMAGE)) {		
		float fPercent = (40 - fValue) / 100;

		fPercent = (0.1f > fPercent) ? 0.1f : fPercent;
		fPercent *= 100;
		multiBonus -= fPercent;
	}

	if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_IMPROVED_AMPLIFY_DAMAGE) || BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_DIMENSIONAL_AFFINITY) || BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_VOID_AFFINITY)) {
		float fPercent = (40 - (fValue + lpTargetObj->CharacterData->Mastery.ML_SUM_ImprovedAmplifyDamage)) / 100;

		fPercent = (0.1f > fPercent) ? 0.1f : fPercent;
		fPercent *= 100;
		multiBonus -= fPercent;
	}

	if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_ARCA_WATERTOWER)) {
		int nEffectValue = 0;

		BUFF_GetBuffEffectValue(lpTargetObj, BUFFTYPE_ARCA_WATERTOWER, &nEffectValue, 0);
		addBonus += nEffectValue;
	}

	if (g_CMuunSystem.GetMuunItemValueOfOptType(lpTargetObj, MUUN_INC_DEFENSE, &nMuunItemEffectValue, 0) == TRUE) addBonus += nMuunItemEffectValue;

	int targetdefense = lpTargetObj->Defense;

	if (lpTargetObj->m_MonsterSkillElementInfo.m_iSkillElementDefenseTime > 0) {
		targetdefense += lpTargetObj->m_MonsterSkillElementInfo.m_iSkillElementDefense;

		if (targetdefense < 0) targetdefense = 0;
	}

	percentBonus /= 100;	
	multiBonus /= 100;
	targetdefense += addBonus;
	targetdefense *= percentBonus;
	targetdefense *= multiBonus;
	iOriginTargetDefense = targetdefense;
	
	if (fatalChance > 0) {
		if (CHARACTER_PvPResistanceProc(iAccessoryEffect) == FALSE) {
			if (CHARACTER_DebuffProc(fatalChance) == TRUE) {
				targetdefense = 0;
				MsgDamage = 1;
			}
		}
	}

	return targetdefense;
}
int CObjBaseAttack::GetPartyMemberCount(LPOBJ lpObj) {
	LPOBJ lpPartyObj;
	int partynum = lpObj->PartyNumber;

	if (ObjectMaxRange(partynum) == FALSE) return FALSE;

	int partycount = gParty.m_PartyS[partynum].Count;
	int retcount = 0;

	for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
		int memberindex = gParty.m_PartyS[partynum].Number[n];

		if (memberindex >= 0) {
			lpPartyObj = &gObj[memberindex];

			if (lpObj->MapNumber == lpPartyObj->MapNumber) {
				int dis = CHARACTER_CalcDistance(lpObj, &gObj[memberindex]);
				
				if (dis < MAX_PARTY_DISTANCE_EFFECT) retcount++;
			}
		}
	}

	return retcount;
}