#include "stdafx.h"
#include "ObjUseSkill.h"
#include "ObjCalCharacter.h"
#include "GameMain.h"
#include "ChaosCastle.h"
#include "user.h"
#include "zzzmathlib.h"
#include "SkillHitBox.h"
#include "MoveCommand.h"
#include "SkillAdditionInfo.h"
#include "BuffEffect.h"
#include "BuffEffectSlot.h"
#include "IllusionTempleEvent_Renewal.h"
#include "TLog.h"
#include "Raklion.h"
#include "RaklionBattleUserMng.h"
#include "CastleSiegeSync.h"
#include "CastleSiege.h"
#include "winutil.h"
#include "MasterLevelSkillTreeSystem.h"
#include "GensSystem.h"
#include "configread.h"
#include "NewPVP.h"
#include "DoppelGanger.h"
#include "ObjBaseAttack.h"
#include "ObjAttack.h"
#include "SkillUseTime.h"
#include "IniReader.h"
#include "MineSystem.h"
#include "PentagramSystem.h"
#include "BotSystem.h"
#include "ArcaBattle.h"
#include "MapAttribute.h"
#include "SkillSafeZoneUse.h"

// todo: mastering skills for summoner
// Duration master skills not tested

CObjUseSkill gObjUseSkill;

CObjUseSkill::CObjUseSkill() : m_Lua(true) {
	return;
}
CObjUseSkill::~CObjUseSkill() {
	return;
}
bool CObjUseSkill::LoadSkillInfo() {
	CIniReader ReadSkill(g_ConfigRead.GetPath("\\Skills\\IGC_SkillSettings.ini"));
	this->m_SkillData.EnableWizardSkillWhileTeleport = ReadSkill.ReadInt("SkillInfo", "CanDarkWizardUseSkillsWhileTeleport", 0);
	this->m_SkillData.SternTime	= ReadSkill.ReadInt("SkillInfo", "SternTime", 10);
	this->m_SkillData.SwellManaTime	= ReadSkill.ReadInt("SkillInfo", "SwellManaTime", 60);
	this->m_SkillData.CloakingTime = ReadSkill.ReadInt("SkillInfo", "CloakingTime", 60);
	this->m_SkillData.IceArrowTime = ReadSkill.ReadInt("SkillInfo", "IceArrowTime", 7);
	this->m_SkillData.EnableSiegeOnAllMaps = ReadSkill.ReadInt("SiegeSkills", "EnableOnAllMaps", 1);
	this->m_Lua.DoFile(g_ConfigRead.GetPath("\\Scripts\\Skills\\RegularSkillCalc.lua"));
	return true;
}

// General
BOOL CObjUseSkill::EnableSkill(WORD Skill) {
	if (g_MasterLevelSkillTreeSystem.CheckMasterLevelSkill(Skill)) {
		int iBaseMLS = g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(Skill);

		if (iBaseMLS == 403 || iBaseMLS == 388 || iBaseMLS == 330 || iBaseMLS == 413 || iBaseMLS == 414 || iBaseMLS == 481 || iBaseMLS == 356 || iBaseMLS == 382 || iBaseMLS == 512 || iBaseMLS == 508
			|| iBaseMLS == 385 || iBaseMLS == 391 || iBaseMLS == 336 || iBaseMLS == 331 || iBaseMLS == 417 || iBaseMLS == 420 || iBaseMLS == 490 || iBaseMLS == 482 || iBaseMLS == 518 || iBaseMLS == 487
			|| iBaseMLS == 454 || iBaseMLS == 455 || iBaseMLS == 456 || iBaseMLS == 458 || iBaseMLS == 326 || iBaseMLS == 327 || iBaseMLS == 328 || iBaseMLS == 329 || iBaseMLS == 332 || iBaseMLS == 333
			|| iBaseMLS == 337 || iBaseMLS == 339 || iBaseMLS == 340 || iBaseMLS == 342 || iBaseMLS == 343 || iBaseMLS == 344 || iBaseMLS == 346 || iBaseMLS == 360 || iBaseMLS == 363 || iBaseMLS == 378
			|| iBaseMLS == 379 || iBaseMLS == 380 || iBaseMLS == 381 || iBaseMLS == 383 || iBaseMLS == 384 || iBaseMLS == 386 || iBaseMLS == 387 || iBaseMLS == 389 || iBaseMLS == 392 || iBaseMLS == 393
			|| iBaseMLS == 404 || iBaseMLS == 406 || iBaseMLS == 415 || iBaseMLS == 416 || iBaseMLS == 418 || iBaseMLS == 419 || iBaseMLS == 425 || iBaseMLS == 426 || iBaseMLS == 427 || iBaseMLS == 428
			|| iBaseMLS == 429 || iBaseMLS == 430 || iBaseMLS == 432 || iBaseMLS == 433 || iBaseMLS == 441 || iBaseMLS == 459 || iBaseMLS == 460 || iBaseMLS == 461 || iBaseMLS == 462 || iBaseMLS == 466
			|| iBaseMLS == 468 || iBaseMLS == 469 || iBaseMLS == 470 || iBaseMLS == 472 || iBaseMLS == 479 || iBaseMLS == 480 || iBaseMLS == 483 || iBaseMLS == 484 || iBaseMLS == 486 || iBaseMLS == 488
			|| iBaseMLS == 489 || iBaseMLS == 492 || iBaseMLS == 493 || iBaseMLS == 494 || iBaseMLS == 495 || iBaseMLS == 496 || iBaseMLS == 497 || iBaseMLS == 509 || iBaseMLS == 510 || iBaseMLS == 514
			|| iBaseMLS == 515 || iBaseMLS == 516 || iBaseMLS == 517 || iBaseMLS == 519 || iBaseMLS == 520 || iBaseMLS == 523 || iBaseMLS == 539 || iBaseMLS == 551 || iBaseMLS == 552 || iBaseMLS == 554
			|| iBaseMLS == 555 || iBaseMLS == 556 || iBaseMLS == 558 || iBaseMLS == 559 || iBaseMLS == 560 || iBaseMLS == 561 || iBaseMLS == 562 || iBaseMLS == 563 || iBaseMLS == 564 || iBaseMLS == 565
			|| iBaseMLS == 566 || iBaseMLS == 567 || iBaseMLS == 575 || iBaseMLS == 577 || iBaseMLS == 411 || iBaseMLS == 424 || iBaseMLS == 431 || iBaseMLS == 463 || iBaseMLS == 491 || iBaseMLS == 522
			|| iBaseMLS == 521 || iBaseMLS == 524) return true;
	}
	if (Skill != DW_TWISTER && Skill != DW_EVILSPIRITS && Skill != DW_FLAME && Skill != DW_HELLFIRE && Skill != DW_BLAST && Skill != DW_INFERNO && Skill != WEAPON_SKILL_CROSSBOW
		&& Skill != AT_SKILL_DEVILFIRE && Skill != DW_AQUABEAM && Skill != DK_TWISTINGSLASH && Skill != MG_POWERSLASH && Skill != MOUNT_IMPALE && Skill != DK_LIFESWELL
		&& Skill != DK_DEATHSTAB && Skill != MOUNT_SCORCHINGBREATH && Skill != ELF_ICEARROW && Skill != ELF_PENETRATION && Skill != MG_FIRESLASH && Skill != DW_MANASHIELD
		&& Skill != DK_RAGEFULBLOW && Skill != DW_DECAY && Skill != DW_ICESTORM && Skill != DW_NOVA && Skill != DL_SPARK && Skill != DL_EARTHQUAKE
		&& Skill != SIEGE_BRAND_OF_SKILL && Skill != SIEGE_STUN && Skill != SIEGE_REMOVESTUN && Skill != SIEGE_SWELLMANA && Skill != SIEGE_INVISIBILITY && Skill != SIEGE_REMOVEINVISIBILITY
		&& Skill != SIEGE_ABOLISHMAGIC && Skill != FENRIR_SKILL && Skill != ELF_INFINITYARROW && Skill != DL_FIRESCREAM && Skill != SUM_SPIRIT_OF_FLAMES && Skill != SUM_SPIRIT_OF_ANGUISH
		&& Skill != SUM_SPIRIT_OF_TERROR && Skill != SUM_LIGHTNINGBLAST && Skill != SUM_SLEEP && Skill != SUM_DRAIN_LIFE && Skill != SUM_MAGICMIRROR && Skill != SUM_WEAKNESS
		&& Skill != SUM_ENFEEBLE && Skill != SUM_CHAINLIGHTNING && Skill != DK_GROUNDSLAM && Skill != DW_ARCANEPOWER && Skill != ELF_SHIELDRECOVERY && Skill != MG_FLAMESTRIKE
		&& Skill != MG_THUNDERSTORM && Skill != DL_DARKNESS && Skill != MONSTER_SUICIDE && Skill != ELF_FIVESHOT && Skill != SUM_AMPLIFYDAMAGE && Skill != RF_FISTFLURRY
		&& Skill != RF_FOCUS && Skill != RF_TOUGHNESS && Skill != RF_LIGHTNINGREFLEXES && Skill != RF_DARKSIDE && Skill != RF_CHAINDRIVE && Skill != RF_DRAGONFIST
		&& Skill != RF_MOLTENSTRIKE && Skill != RF_PHOENIXSHOT && Skill != AT_SKILL_SPIN_STEP && Skill != AT_SKILL_CIRCLE_SHIELD && Skill != AT_SKILL_OBSIDIAN && Skill != AT_SKILL_MAGIC_PIN
		&& Skill != AT_SKILL_CLASH && Skill != AT_SKILL_HARSH_STRIKE && Skill != AT_SKILL_SHINING_PEAK && Skill != AT_SKILL_WRATH && Skill != AT_SKILL_BRECHE && Skill != MONSTER_LORDSILVESTER_SUMMON) return false;
	return true;
}
BOOL CObjUseSkill::RemoveAllCharacterInvalidMagicAndSkillState(LPOBJ lpObj) {
	if (lpObj->Type != OBJ_USER) return false;

	return true;
}
void CObjUseSkill::UseSkill(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, BYTE dir, BYTE TargetPos, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int bCombo = FALSE;

	if (gComboAttack.CheckCombo(aIndex, lpMagic->m_Skill)) bCombo = TRUE;
	if (g_SkillUseTime.CheckSkillTime(lpObj, lpMagic->m_Skill) == false) return;
	if (g_ConfigRead.antihack.EnableAttackBlockInSafeZone == TRUE) {
		BYTE btAttr = MapC[lpObj->MapNumber].GetAttr(lpObj->X, lpObj->Y);

		if ((btAttr & 1) == 1) {
			if (g_SkillSafeZone.CanUseSkill(lpObj, lpMagic) == false) return;
		}
	}

	if (lpObj->Type == OBJ_USER) {
		if (lpObj->CharacterData->SkillDelay.Check(lpMagic->m_Skill) == FALSE) return;
		if (lpObj->CharacterData->SkillHellFire2State && lpMagic->m_Skill != DW_NOVA) {
			CMagicInf* lpMagic = PLAYER_GetMagicSearch(lpObj, DW_NOVA);

			if (lpMagic != 0) gObjUseSkill.RunningSkill(lpObj->m_Index, 0, lpMagic, 0);
			return;
		} else if (lpObj->CharacterData->SkillStrengthenHellFire2State && lpMagic->m_Skill != 392) {
			CMagicInf* lpMagic = PLAYER_GetMagicSearch(lpObj, 392);

			if (lpMagic != 0) gObjUseSkill.RunningSkill(lpObj->m_Index, 0, lpMagic, 0);
			return;
		}
	}

	int usemana = this->GetUseMana(aIndex, lpMagic);

	if (lpMagic->m_Skill == DL_EARTHQUAKE) usemana = gObj[aIndex].Mana;
	if (usemana >= 0) {
		int usebp = PLAYER_MagicStamUse(&gObj[aIndex], lpMagic);

		if (usebp >= 0) {
			lpObj->UseMagicNumber = lpMagic->m_Skill;
			lpObj->UseMagicTime = GetTickCount();
			lpObj->UseMagicCount = 0;
			lpObj->Mana = usemana;
			lpObj->Stamina = usebp;
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, lpObj->Mana, 0xFF, 0, lpObj->Stamina);

			PMSG_DURATION_MAGIC_SEND pMsg;

			PHeadSetBE((PBYTE)&pMsg, 0x1E, sizeof(pMsg));
			pMsg.MagicNumberH = SET_NUMBERH(lpMagic->m_Skill);
			pMsg.MagicNumberL = SET_NUMBERL(lpMagic->m_Skill);
			pMsg.X = x;
			pMsg.Y = y;
			pMsg.Dir = dir;
			pMsg.NumberH = SET_NUMBERH(aIndex);
			pMsg.NumberL = SET_NUMBERL(aIndex);

			if (lpObj->Type == OBJ_USER) IOCP.DataSend(lpObj->m_Index, (PBYTE)&pMsg, pMsg.h.size);
			if (CC_MAP_RANGE(lpObj->MapNumber) || lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) {
				if (g_MasterLevelSkillTreeSystem.CheckMasterLevelSkill(lpMagic->m_Skill) != FALSE) { //Season3 add-on
					int GetSkillID = g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(lpMagic->m_Skill);

					switch (GetSkillID) {
						case 356:	//1.01.00
						case 360:
						case 363:
						case 403:
						case 404:
						case 406:
						case 413:
						case 417:
						case 420:
						break;

						default:	GSProtocol.MsgSendV2(lpObj, (LPBYTE)&pMsg, pMsg.h.size);
					}
				} else {
					switch (lpMagic->m_Skill) {
						case DW_MANASHIELD:
						case ELF_HEAL:
						case ELF_ENHANCE:
						case ELF_EMPOWER:
						case DK_LIFESWELL:
						break;

						default:	GSProtocol.MsgSendV2(lpObj, (PBYTE)&pMsg, pMsg.h.size);
					}
				}
			} else GSProtocol.MsgSendV2(lpObj, (PBYTE)&pMsg, pMsg.h.size);
		}

		if (g_MasterLevelSkillTreeSystem.CheckMasterLevelSkill(lpMagic->m_Skill) == true) {
			g_MasterLevelSkillTreeSystem.RunningSkill_MLS(lpObj, aTargetIndex, lpMagic, bCombo, x, y, TargetPos);
			return;
		}

		switch (lpMagic->m_Skill) {
			case DK_TWISTINGSLASH:							this->DK_TwistingSlash(aIndex, lpMagic, aTargetIndex, bCombo);																			break;
			case DK_RAGEFULBLOW:							this->DK_RagefulBlow(aIndex, lpMagic, x, y, dir, TargetPos, aTargetIndex, bCombo);														break;
			case DK_GROUNDSLAM:								this->DK_Groundslam(aIndex, lpMagic, x, y, aTargetIndex, bCombo);																		break;
			case DW_ICESTORM:								this->DW_IceStorm(aIndex, lpMagic, x, y, dir, TargetPos, aTargetIndex);																	break;
			case DW_DECAY:									if (lpObj->Class == 561 && lpMagic->m_Skill == DW_DECAY) this->MEDUSA_Decay(aIndex, lpMagic, x, y, aTargetIndex);
															else this->DW_Decay(aIndex, lpMagic, x, y, dir, TargetPos, aTargetIndex);																break;
			case ELF_FIVESHOT:								this->ELF_FiveShot(aIndex, lpMagic, TargetPos, aTargetIndex);																			break;
			case SUM_SPIRIT_OF_FLAMES:						this->SUM_SpiritOfFlames(aIndex, lpMagic, x, y, aTargetIndex);																			break;
			case SUM_SPIRIT_OF_ANGUISH:						this->SUM_SpiritOfAnguish(aIndex, lpMagic, x, y, aTargetIndex);																			break;
			case SUM_SPIRIT_OF_TERROR:						this->SUM_SpiritOfTerror(aIndex, lpMagic, x, y, aTargetIndex, 59, 0);																	break;
			case SUM_DRAIN_LIFE:							this->SUM_DrainLife(aIndex, lpMagic, aTargetIndex);																						break;
			case SUM_CHAINLIGHTNING:						this->SUM_ChainLighting(aIndex, lpMagic, aTargetIndex);																					break;
			case SUM_LIGHTNINGBLAST:						this->SUM_LightningBlast(aIndex, lpMagic, aTargetIndex);																				break;
			case SUM_ENFEEBLE:								this->SUM_Enfeeble(aIndex, lpMagic, x, y, aTargetIndex);																				break;
			case SUM_WEAKNESS:								this->SUM_Weakness(aIndex, lpMagic, x, y, aTargetIndex);																				break;
			case MG_POWERSLASH:								this->MG_PowerSlash(aIndex, lpMagic, x, y, dir, TargetPos, aTargetIndex);																break;
			case MG_THUNDERSTORM:							this->MG_ThunderStorm(aIndex, lpMagic, x, y, aTargetIndex);																				break;
			case MG_FLAMESTRIKE:							this->MG_FlameStrike(aIndex, lpMagic, TargetPos, aTargetIndex);																			break;
			case DL_EARTHQUAKE:								this->DL_Earthquake(lpObj->m_Index, aTargetIndex, lpMagic);																				break;
			case DL_SPARK:									this->DL_Spark(aIndex, lpMagic, x, y, dir, TargetPos, aTargetIndex);																	break;
			case DL_DARKNESS:								this->DL_Darkness(aIndex, lpMagic, TargetPos, aTargetIndex);																			break;
			case RF_PHOENIXSHOT:							if (ObjectMaxRange(aTargetIndex) == FALSE) {
																g_Log.Add("[InvalidTargetIndex][CObjUseSkill.UseSkill][AT_SKILL_PHOENIXSHOT] Index :%d , AccountID : %s ", aIndex, gObj[aIndex].AccountID);
																return;
															}

															this->SKILL_AreaAttack(aIndex, lpMagic, gObj[aTargetIndex].X, gObj[aTargetIndex].Y, aTargetIndex, 2, 1, 0);								break;
			case RF_MOLTENSTRIKE:							if (ObjectMaxRange(aTargetIndex) == FALSE) {
																g_Log.Add("[InvalidTargetIndex][CObjUseSkill.UseSkill][AT_SKILL_DRAGON_ROAR] Index :%d , AccountID : %s ", aIndex, gObj[aIndex].AccountID);
																return;
															}

															this->SKILL_AreaAttack(aIndex, lpMagic, gObj[aTargetIndex].X, gObj[aTargetIndex].Y, aTargetIndex, 3, 1, 0);								break;
			case RF_DARKSIDE:								this->RF_DarkSide(aIndex, aTargetIndex, lpMagic);																						break;
			case FENRIR_SKILL:								this->FENRIR_PlasmaStorm(aIndex, aTargetIndex, lpMagic);																				break;
			case SIEGE_STUN:								this->SIEGE_SkillStun(aIndex, aTargetIndex, lpMagic, x, y, dir, TargetPos);																break;
			case AT_SKILL_BRECHE:							this->SkillBreche(aIndex, lpMagic, x, y, aTargetIndex);																					break;
			case MONSTER_SUICIDE:							g_DoppelGanger.SelfExplosion(aIndex, lpMagic, aTargetIndex);																			break;
			case MONSTER_LORDSILVESTER_SUMMON:				this->SILVESTER_Summon(aIndex, aTargetIndex, lpMagic);																					break;
		}
	}
}
void CObjUseSkill::UseSkill(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	BOOL bCombo = FALSE;

	if (g_SkillUseTime.CheckSkillTime(lpObj, lpMagic->m_Skill) == false) return;
	if (g_ConfigRead.antihack.EnableAttackBlockInSafeZone == TRUE) {
		BYTE btAttr = MapC[lpObj->MapNumber].GetAttr(lpObj->X, lpObj->Y);

		if ((btAttr & 1) == 1) {
			if (g_SkillSafeZone.CanUseSkill(lpObj, lpMagic) == false) return;
		}
	}

	if (gComboAttack.CheckCombo(aIndex, lpMagic->m_Skill)) bCombo = TRUE;
	if (lpObj->Type == OBJ_USER) {
		if (lpObj->CharacterData->SkillDelay.Check(lpMagic->m_Skill) == FALSE) return;
		if (lpObj->CharacterData->SkillHellFire2State && lpMagic->m_Skill != DW_NOVA) {
			CMagicInf* lpMagic = PLAYER_GetMagicSearch(lpObj, DW_NOVA);

			if (lpMagic != 0) gObjUseSkill.RunningSkill(lpObj->m_Index, 0, lpMagic, 0);
			return;
		} else if (lpObj->CharacterData->SkillStrengthenHellFire2State && lpMagic->m_Skill != 392) {
			CMagicInf* lpMagic = PLAYER_GetMagicSearch(lpObj, 392);

			if (lpMagic != 0) gObjUseSkill.RunningSkill(lpObj->m_Index, 0, lpMagic, 0);
			return;
		}
	}

	if (lpMagic->m_Skill == DW_NOVA && aTargetIndex != 58) {
		this->RunningSkill(aIndex, aTargetIndex, lpMagic, bCombo);
		lpObj->UseMagicNumber = lpMagic->m_Skill;
		lpObj->UseMagicTime = GetTickCount();
		lpObj->UseMagicCount = 0;
		return;
	}

	if (lpMagic->m_Skill == 392 && aTargetIndex != 395) {
		this->RunningSkill(aIndex, aTargetIndex, lpMagic, bCombo);
		lpObj->UseMagicNumber = lpMagic->m_Skill;
		lpObj->UseMagicTime = GetTickCount();
		lpObj->UseMagicCount = 0;
		return;
	}

	if (lpMagic->m_Skill == RF_DARKSIDE || lpMagic->m_Skill == 559 || lpMagic->m_Skill == 563) {
		if (lpObj->Type == OBJ_USER && lpObj->CharacterData->DarkSideCount > 1) {
			this->RunningSkill(aIndex, aTargetIndex, lpMagic, bCombo);
			lpObj->UseMagicNumber = lpMagic->m_Skill;
			lpObj->UseMagicTime = GetTickCount();
			lpObj->UseMagicCount = 0;
			return;
		}
	}

	int usemana = this->GetUseMana(aIndex, lpMagic);

	if (usemana >= 0) {
		int usebp = this->GetUseStamina(aIndex, lpMagic);

		if (usebp >= 0) {
			if (this->RunningSkill(aIndex, aTargetIndex, lpMagic, bCombo) == 1) {
				lpObj->UseMagicNumber = lpMagic->m_Skill;
				lpObj->UseMagicTime = GetTickCount();
				lpObj->UseMagicCount = 0;
				lpObj->Mana = usemana;
				lpObj->Stamina = usebp;
				GSProtocol.PROTOCOL_ManaSend(aIndex, lpObj->Mana, 0xFF, 0, lpObj->Stamina);
			}
		}
	}
}
BOOL CObjUseSkill::RunningSkill(int aIndex, int aTargetIndex, CMagicInf* lpMagic, BOOL bCombo) {
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	WORD MagicNumber = lpMagic->m_Skill;

	lpObj->m_SkillNumber = MagicNumber;

	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_INVISABLE) == TRUE && (gObj[aIndex].Authority & 0x20) != 0x20) this->SIEGE_RemoveCloakingEffect(lpObj->m_Index);
	if (g_MasterLevelSkillTreeSystem.CheckMasterLevelSkill(lpMagic->m_Skill)) return g_MasterLevelSkillTreeSystem.RunningSkill_MLS(lpObj, aTargetIndex, lpMagic, bCombo, 0, 0, 0);
	if (lpObj->Type == OBJ_USER && lpObj->CharacterData->DarkSideCount > 0) {
		if (lpMagic->m_Skill != 263 && lpMagic->m_Skill != 559 && lpMagic->m_Skill != 563) {
			for (int i = 0; i < 5; i++) lpObj->CharacterData->DarkSideTarget[i] = 10000;

			lpObj->CharacterData->DarkSideCount = 0;
			return FALSE;
		}

		BOOL bAttack = FALSE;

		for (int j = 0; j < 5; j++) {
			if (lpObj->CharacterData->DarkSideTarget[j] == aTargetIndex && aIndex != aTargetIndex) {
				lpObj->CharacterData->DarkSideTarget[j] = 10000;
				--lpObj->CharacterData->DarkSideCount;
				bAttack = TRUE;
				break;
			}
		}

		if (bAttack == FALSE) {
			for (int i = 0; i < 5; i++) lpObj->CharacterData->DarkSideTarget[i] = 10000;

			lpObj->CharacterData->DarkSideCount = 0;
			return FALSE;
		}
	} else if (lpObj->Type == OBJ_USER && lpObj->CharacterData->DarkSideCount == 0) {
		if (lpMagic->m_Skill == 263 || lpMagic->m_Skill == 559 || lpMagic->m_Skill == 563) return TRUE;
	}


	switch (MagicNumber) {
		case SHIELD_BLOCK:
			if (lpObj->pInventory[1].IsItem() == 1) {
				if (lpObj->pInventory[1].m_Type >= ITEMGET(6, 4) && lpObj->pInventory[1].m_Type < ITEMGET(7, 0)) {
					if (lpObj->pInventory[1].m_Option1 != 0) {
						lpObj->m_SkillTime = GetTickCount() + 4000;
						GSProtocol.PROTOCOL_ActionSend(lpObj, 18, lpObj->m_Index, aTargetIndex);
					}
				}
			}
		break;

		case DW_POISON:
		case DW_METEOR:
		case DW_LIGHTNING:
		case DW_FIREBALL:
		case DW_FLAME:
		case DW_ICE:
		case DW_ENERGYBALL:
		case WEAPON_FALLINGSLASH:
		case WEAPON_LUNGE:
		case WEAPON_UPPERCUT:
		case WEAPON_CYCLONESLASH:
		case WEAPON_FORWARDSLASH:
		case WEAPON_SKILL_CROSSBOW:
		case WEAPON_SKILL_BOW:
		case DK_TWISTINGSLASH:
		case DK_RAGEFULBLOW:
		case MOUNT_IMPALE:
		case ELF_PENETRATION:
		case MG_POWERSLASH:					gObjAttack(lpObj, lpTargetObj, lpMagic, TRUE, 0, 0, 0, 0, 0);			break;

		case ELF_HEAL:						this->ELF_Heal(aIndex, aTargetIndex, lpMagic);							break;
		case DW_MANASHIELD:					this->DW_ManaShield(aIndex, aTargetIndex, lpMagic);						break;
		case DK_LIFESWELL:					this->DK_LifeSwell(aIndex, lpMagic);									break;
		case ELF_ENHANCE:					this->ELF_Enhance(aIndex, aTargetIndex, lpMagic);						break;
		case ELF_EMPOWER:					this->ELF_Empower(aIndex, aTargetIndex, lpMagic);						break;
		case SUM_AMPLIFYDAMAGE:				this->SUM_AmplifyDamage(aIndex, lpMagic, aTargetIndex);					break;

		case ELF_SUMMON_GOBLIN:				return this->ELF_Summon(lpObj->m_Index, 26, lpObj->X - 1, lpObj->Y + 1);
		case ELF_SUMMON_STONEGOLEM:			return this->ELF_Summon(lpObj->m_Index, 32, lpObj->X - 1, lpObj->Y + 1);
		case ELF_SUMMON_ASSASSIN:			return this->ELF_Summon(lpObj->m_Index, 21, lpObj->X - 1, lpObj->Y + 1);
		case ELF_SUMMON_GIANTYETI:			return this->ELF_Summon(lpObj->m_Index, 20, lpObj->X - 1, lpObj->Y + 1);
		case ELF_SUMMON_DARKKNIGHT:			return this->ELF_Summon(lpObj->m_Index, 10, lpObj->X - 1, lpObj->Y + 1);
		case ELF_SUMMON_BALI:				return this->ELF_Summon(lpObj->m_Index, 150, lpObj->X - 1, lpObj->Y + 1);
		case ELF_SUMMON_SOLDIER:			return this->ELF_Summon(lpObj->m_Index, 151, lpObj->X - 1, lpObj->Y + 1);

		case DW_NOVA:						return this->DW_Nova(lpObj->m_Index, aTargetIndex, lpMagic);
		case DK_DEATHSTAB:					return this->DK_DeathStab(lpObj->m_Index, aTargetIndex, lpMagic, bCombo);

		case DL_FORCE:						return this->DL_Force(lpObj->m_Index, aTargetIndex, lpMagic);
		case DL_FIREBURST:					return this->DL_FireBurst(lpObj->m_Index, aTargetIndex, lpMagic);
		case DL_EARTHQUAKE:					return this->DL_Earthquake(lpObj->m_Index, aTargetIndex, lpMagic);
		case DL_SUMMON:						this->DL_Summon(aIndex, lpMagic->m_Level);								break;
		case DL_CONCENTRATION:				this->DL_Concentration(aIndex, lpMagic->m_Level, lpMagic);				break;

		case MG_FIRESLASH:					this->MG_FireSlash(lpObj->m_Index, aTargetIndex, 1);					break;

		case KUNDUN_SUMMON:					this->KUNDUN_Summon(lpObj->m_Index, aTargetIndex, lpMagic);				break;
		case AT_SKILL_IMMUNE_TO_MAGIC:		this->SkillImmuneToMagic(lpObj->m_Index, lpMagic);						break;
		case AT_SKILL_IMMUNE_TO_HARM:		this->SkillImmuneToHarm(lpObj->m_Index, lpMagic);						break;
		case DK_SUNWAVESLASH:				this->DK_SunwaveSlash(lpObj->m_Index, aTargetIndex, lpMagic);			break;
		case DW_SOULGLAIVES:				this->DW_SoulGlaives(lpObj->m_Index, aTargetIndex, lpMagic);			break;
		case ELF_STARFALL:					this->ELF_Starfall(lpObj->m_Index, aTargetIndex, lpMagic);				break;
		case MG_SPIRALSLASH:				this->MG_SpiralSlash(lpObj->m_Index, aTargetIndex, lpMagic);			break;
		case MG_MANARAYS:					this->MG_ManaRays(lpObj->m_Index, aTargetIndex, lpMagic);				break;
		case DL_PILLARSOFFIRE:				this->DL_PillarsOfFire(lpObj->m_Index, aTargetIndex, lpMagic);			break;

		case SIEGE_BRAND_OF_SKILL:			this->SIEGE_SkillBrandOfSkill(aIndex, aTargetIndex, lpMagic);			break;
		case SIEGE_REMOVESTUN:				this->SIEGE_SkillRemoveStun(aIndex, aTargetIndex, lpMagic);				break;
		case SIEGE_SWELLMANA:				this->SIEGE_SkillAddMana(aIndex, aTargetIndex, lpMagic);				break;
		case SIEGE_INVISIBILITY:			this->SIEGE_SkillCloaking(aIndex, aTargetIndex, lpMagic);				break;
		case SIEGE_REMOVEINVISIBILITY:		this->SIEGE_SkillRemoveCloaking(aIndex, aTargetIndex, lpMagic);			break;
		case SIEGE_ABOLISHMAGIC:			this->SIEGE_SkillRemoveMagic(aIndex, aTargetIndex, lpMagic);			break;

		case ELF_INFINITYARROW:				this->ELF_InfinityArrow(aIndex, aTargetIndex, lpMagic);					break;

		case SUM_SLEEP:						this->SUM_Sleep(aIndex, aTargetIndex, lpMagic);							break;
		case SUM_MAGICMIRROR:				this->SUM_MagicMirror(aIndex, lpMagic, aTargetIndex);					break;
		case DW_ARCANEPOWER:				this->DW_ArcanePower(aIndex, aTargetIndex, lpMagic);					break;
		case ELF_SHIELDRECOVERY:			this->ELF_ShieldRecovery(aIndex, lpMagic, aTargetIndex);				break;

		case RF_FISTFLURRY:
		case RF_BEASTUPPERCUT:
		case RF_CHAINDRIVE:					this->RF_WeaponSkill(aIndex, lpMagic, aTargetIndex);					break;
		case RF_FOCUS:						this->RF_Focus(aIndex, lpMagic);										break;
		case RF_TOUGHNESS:
		case RF_LIGHTNINGREFLEXES:			this->RF_BuffParty(aIndex, lpMagic);									break;

		// LANCER (UNUSED AT THE MOMENT)
		case AT_SKILL_SPIN_STEP:			this->SkillSpinStep(aIndex, aTargetIndex, lpMagic);						break;

		case AT_SKILL_HARSH_STRIKE:
		case AT_SKILL_MAGIC_PIN:			this->SkillGrowLancerAttackOneTarget(aIndex, aTargetIndex, lpMagic);	break;

		case AT_SKILL_SHINING_PEAK:			this->SkillShiningPeak(aIndex, lpMagic, aTargetIndex);					break;
		case AT_SKILL_OBSIDIAN:				this->SkillBuffPartyGrowLancer(aIndex, lpMagic, aTargetIndex);			break;

		case AT_SKILL_WRATH:
		case AT_SKILL_CIRCLE_SHIELD:		this->SkillBuffGrowLancer(aIndex, lpMagic, aTargetIndex);				break;

		default:							gObjAttack(lpObj, lpTargetObj, lpMagic, 1, 0, 0, 0, 0, 0);				break;
	}

	return true;
}
int CObjUseSkill::GetUseMana(int aIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	float mana = lpObj->Mana;

	if (lpObj->Type != OBJ_USER) return FALSE;
	if (lpMagic == NULL) return FALSE;

	float ManaCost;
	float CostReduction;

	if (g_MasterLevelSkillTreeSystem.CheckMasterLevelSkill(lpMagic->m_Skill) == true) {
		ManaCost = MagicDamageC.MAGIC_GetManaUsage(lpMagic->m_Skill);
		ManaCost += ManaCost * g_MasterLevelSkillTreeSystem.GetMasterSkillManaIncRate(lpMagic->m_Level) / 100.0;
		ManaCost += this->ELF_InfinityArrowManaUsage(lpObj, lpMagic);
	} else {
		ManaCost = MagicDamageC.MAGIC_GetManaUsage(lpMagic->m_Skill);
		ManaCost += this->ELF_InfinityArrowManaUsage(lpObj, lpMagic);
	}

	if (lpObj->Class == CLASS_RAGEFIGHTER) CostReduction = ManaCost * lpObj->CharacterData->Mastery.ML_RF_ArcaneEfficiency / 100.0;
	else CostReduction = ManaCost * lpObj->CharacterData->Mastery.ML_GENERAL_ArcaneEfficiency / 100.0;	
	if (lpObj->Class == CLASS_WIZARD && g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 386) == 20) CostReduction += ManaCost * 0.1;		// Arcane Brilliance Mastery

	// Pendant of Ability Cost Reduce (10%)
	CItem Amulet = lpObj->pInventory[9];

	if (Amulet.IsItem() == TRUE && Amulet.m_Type == ITEMGET(13, 28)) CostReduction += ManaCost * 0.1;

	ManaCost -= CostReduction;

	if (ManaCost < 0) ManaCost = 0;
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 357) == 20 && rand() % 10 == TRUE)	ManaCost = 0;		// Arcane Efficiency

	mana -= ManaCost;

	if (mana < 0.0f) return -1;
	return mana;
}
int CObjUseSkill::GetUseStamina(int aIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	int stam = lpObj->Stamina;

	if (lpObj->Type != OBJ_USER) return FALSE;
	if (lpMagic == 0) return FALSE;

	int StaminaCost = MagicDamageC.MAGIC_GetStamUsage(lpMagic->m_Skill);
	int CostReduction;

	if (lpObj->CharacterData->m_btAGReduceRate > 0) CostReduction = StaminaCost * lpObj->CharacterData->m_btAGReduceRate / 100;			// Socket Bonus
	if (lpObj->Class == CLASS_WIZARD && g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 386) == 20) CostReduction += StaminaCost * 0.1;			// Arcane Brilliance Mastery
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 357) >= 10) CostReduction += StaminaCost * 0.15;		// Arcane Efficiency

	// Pendant of Ability Cost Reduce (10%)
	CItem Amulet = lpObj->pInventory[9];

	if (Amulet.IsItem() == TRUE && Amulet.m_Type == ITEMGET(13, 28)) CostReduction += StaminaCost * 0.1;

	StaminaCost -= CostReduction;

	if (StaminaCost < 0) StaminaCost = 0;
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 357) == 20 && rand() % 10 == TRUE)	StaminaCost = 0;		// Arcane Efficiency

	stam -= StaminaCost;

	if (stam < 0) return -1;
	return stam;
}
int CObjUseSkill::GetTargetLinePath(int sx, int sy, int tx, int ty, int * PathX, int * PathY, int distance) {
	int x,y;
	int delta_x,delta_y;
	int half;
	int error = 0;
	int incx = 1,incy = 1;
	int dis;
	int count;

	if (sx > tx)	{
		incx = -1;
		delta_x = sx - tx;
	} else delta_x = tx - sx;
	if (sy > ty) {
		incy = -1;
		delta_y = sy - ty;
	} else delta_y = ty - sy;

	dis = sqrt((float)(delta_x * delta_x + delta_y * delta_y));
	dis = distance - dis;
	count = 0;
	PathX[count] = sx;
	PathY[count] = sy;
	count++;

	if (delta_x > delta_y) {
		y = sy;
		half = delta_x / 2;

		if (incx > 0) {
			for (x = (sx+1); x <= (tx + dis); x++) {
				error += delta_y;

				if (error > half) {
					y += incy;
					error -= delta_x;
				}

				PathX[count] = x;
				PathY[count] = y;
				count++;
			}
		} else {
			for (x = (sx-1); x >= (tx - dis);x--) {
				error += delta_y;

				if (error > half) {
					y += incy;
					error -= delta_x;
				}

				PathX[count] = x;
				PathY[count] = y;
				count++;
			}
		}
	} else {
		x = sx;
		half = delta_y / 2;

		if (incy > 0) {
			for (y = (sy+1); y <= (ty + dis);y++) {
				error += delta_x;

				if (error > half) {
					x += incx;
					error -= delta_y;
				}

				PathX[count] = x;
				PathY[count] = y;
				count++;
			}
		} else {
			for (y = (sy-1);y >= (ty - dis);y--) {
				error += delta_x;

				if (error > half) {
					x += incx;
					error -= delta_y;
				}

				PathX[count] = x;
				PathY[count] = y;
				count++;
			}
		}
	}

	return count;
}
void CObjUseSkill::SkillFrustrum(int aIndex, float fangle, float tx, float ty) {
	LPOBJ lpObj = &gObj[aIndex];
	vec3_t p[4];
	Vector(-tx, ty, 0.f, p[0]);
	Vector(tx, ty, 0.f, p[1]);
	Vector(1.0f, 0.f, 0.f, p[2]);
	Vector(-1.0f, 0.f, 0.f, p[3]);
	vec3_t Angle;
	Vector(0.f, 0.f, fangle, Angle);
	float Matrix[3][4];
	vec3_t vFrustrum[4];

	AngleMatrix(Angle, Matrix);

	for (int i = 0; i < 4; i++) {
		VectorRotate(p[i], Matrix, vFrustrum[i]);
		lpObj->fSkillFrustrumX[i] = (int)vFrustrum[i][0] + lpObj->X;
		lpObj->fSkillFrustrumY[i] = (int)vFrustrum[i][1] + lpObj->Y;
	}
}
void CObjUseSkill::SkillFrustrum2(int aIndex, float fangle, float tx, float ty) {
	LPOBJ lpObj = &gObj[aIndex];
	vec3_t p[4];
	Vector(0.f, ty, 0.f, p[0]);
	Vector(0.f, ty, 0.f, p[1]);
	Vector(tx, 1.0f, 0.f, p[2]);
	Vector(-tx, 1.0f, 0.f, p[3]);
	vec3_t Angle;
	Vector(0.f, 0.f, fangle, Angle);
	float Matrix[3][4];
	vec3_t vFrustrum[4];

	AngleMatrix(Angle, Matrix);

	for (int i = 0; i < 4; i++) {
		VectorRotate(p[i], Matrix, vFrustrum[i]);
		lpObj->fSkillFrustrumX[i] = (int)vFrustrum[i][0] + lpObj->X;
		lpObj->fSkillFrustrumY[i] = (int)vFrustrum[i][1] + lpObj->Y;
	}
}
void CObjUseSkill::SkillFrustrum3(int aIndex, float fangle, float tx1, float ty1, float tx2, float ty2) {
	LPOBJ lpObj = &gObj[aIndex];
	vec3_t p[4];
	Vector(-tx1, ty1, 0.f, p[0]);
	Vector(tx1, ty1, 0.f, p[1]);
	Vector(tx2, ty2, 0.f, p[2]);
	Vector(-tx2, ty2, 0.f, p[3]);
	vec3_t Angle;
	Vector(0.f, 0.f, fangle * 360 / (BYTE)255, Angle);
	float Matrix[3][4];
	vec3_t vFrustrum[4];

	AngleMatrix(Angle, Matrix);

	for (int i = 0; i < 4; i++) {
		VectorRotate(p[i], Matrix, vFrustrum[i]);
		lpObj->fSkillFrustrumX[i] = (int)vFrustrum[i][0] + lpObj->X;
		lpObj->fSkillFrustrumY[i] = (int)vFrustrum[i][1] + lpObj->Y;
	}
}
int CObjUseSkill::GetAngle(int x, int y, int tx, int ty) {
	double diffX = x - tx;
	double diffY = y - ty;
	double rad = atan2(diffY, diffX);
	int angle = rad * 180 / 3.141592741012573 + 90;

	if (angle < 0) angle += 360;
	return angle;
}
int CObjUseSkill::SKILL_CalcDistance(int x, int y, int x1, int y1) {
	if (x == x1 && y == y1) return FALSE;

	float tx = x - x1;
	float ty = y - y1;

	return sqrt(tx * tx + ty * ty);
}
BOOL CObjUseSkill::SkillAreaCheck(int angel, int x, int y, int tx, int ty, int dis, int arc) {
	int minangle = angel - arc;
	int maxangle = angel + arc;

	if (this->SKILL_CalcDistance(x, y, tx, ty) > dis) return false;

	int targetangle = this->GetAngle(x, y, tx, ty);

	if (minangle < 0) {
		if (targetangle > maxangle) {
			if (targetangle > minangle + 360) return true;
			else return false;
		} else if (maxangle > targetangle) return false;
	} else if (maxangle > 360) {
		if (targetangle < minangle) {
			if (targetangle < maxangle - 360) return true;
			else return false;
		} else if (maxangle < targetangle) return false;
	} else if (minangle > targetangle) return false;
	else if (maxangle < targetangle) return false;

	return true;
}
int CObjUseSkill::SKILL_AreaAttack(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex, int Distance, int MultiAttack, int AttackDelay) {
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	int HitCount = 4;
	int MaxHitCount = 8;
	int CurrHitCount = 0;
	int count = 0;
	int tObjNum;
	int EnableAttack;
	int DuelIndex = lpObj->m_iDuelUser;

	if (lpMagic->m_Skill == MG_THUNDERSTORM) {
		HitCount = 10;
		MaxHitCount = 15;
	}

	int iSkillDistance = MagicDamageC.GetSkillDistance(lpMagic->m_Skill);

	if (this->SKILL_CalcDistance(x, y, lpObj->X, lpObj->Y) <= iSkillDistance) {
		while (true) {
			if (lpObj->VpPlayer2[count].state != 0) {
				tObjNum = lpObj->VpPlayer2[count].number;

				if (tObjNum >= 0) {
					EnableAttack = FALSE;

					if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
					if ((tObjNum == aTargetIndex && MultiAttack == TRUE) || DuelIndex == tObjNum) EnableAttack = TRUE;
					else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && gObj[tObjNum].Type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
					if (lpObj->VpPlayer2[count].type == OBJ_NPC || lpObj->MapNumber != MAP_INDEX_CASTLESIEGE || g_CastleSiegeSync.GetCastleState() != 7) {
						int CallMonIndex = gObj[tObjNum].m_Index;

						if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
						if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
						if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
					}

					if (gObj[tObjNum].Live == FALSE) EnableAttack = FALSE;
					if (EnableAttack == TRUE) {
						if (gObj[tObjNum].X >= x - Distance) {
							if (gObj[tObjNum].X <= x + Distance) {
								if (gObj[tObjNum].Y >= y - Distance) {
									if (gObj[tObjNum].Y <= y + Distance) {
										gObj[tObjNum].lpAttackObj = lpObj;
										++CurrHitCount;

										BOOL Attack = FALSE;

										if (CurrHitCount <= HitCount) Attack = TRUE;
										else Attack = rand() % 2;
										if (CurrHitCount > MaxHitCount) Attack = FALSE;
										if (Attack) {
											if (AttackDelay) {
												if (lpMagic->m_Skill == MG_THUNDERSTORM && lpObj->Class == 561 && !BUFF_TargetHasBuff(&gObj[tObjNum], DEBUFFTYPE_CHILL)) {
													gObj[tObjNum].DelayActionTime = 800;
													gObj[tObjNum].DelayLevel = 1;
													gObj[tObjNum].lpAttackObj = lpObj;
													BUFF_AddBuffEffect(&gObj[tObjNum], DEBUFFTYPE_CHILL, 0, 0, 0, 0, 3);
												}

												gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, AttackDelay, lpMagic->m_Skill, 0);
											} else {
												if (lpMagic->m_Skill != RF_MOLTENSTRIKE && lpMagic->m_Skill != RF_PHOENIXSHOT && lpMagic->m_Skill != 560) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 0, 0, 0, 0, 0);
												else {
													for (int i = 1; i <= 4; i++) {
														if (i == 1) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 1, 0, 0, 1, 0);
														else gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 0, 1, 0, 0, i, 0);
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}

			++count;

			if (count > MAX_VIEWPORT - 1) break;
		}
	}

	return TRUE;
}
BOOL CObjUseSkill::SpeedHackCheck(int aIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int iTimeCalc = GetTickCount() - lpObj->m_LastAttackTime;

	if (lpObj->Type != OBJ_USER) return TRUE;
	if (iTimeCalc < lpObj->m_DetectSpeedHackTime) {
		lpObj->m_DetectCount++;
		lpObj->m_SumLastAttackTime += iTimeCalc;

		if (lpObj->m_DetectCount > gHackCheckCount)	{
			lpObj->m_DetectedHackKickCount++;

			if (gIsKickDetecHackCountLimit != 0 && lpObj->m_DetectedHackKickCount > gDetectedHackKickCount)	{
				g_Log.Add("[%s][%s] Kick DetecHackCountLimit Over User (%d)", lpObj->AccountID, lpObj->Name, lpObj->m_DetectedHackKickCount);
				GSProtocol.GCSendDisableReconnect(aIndex);
				//IOCP.CloseClient(aIndex);
				return false;
			}

			lpObj->m_SpeedHackPenalty = gSpeedHackPenalty;

			if (gIsKickDetecHackCountLimit != 0) g_Log.Add("[%s][%s] Attack Speed Is Wrong MagicSkill (%d)(%d) Penalty %d", lpObj->AccountID, lpObj->Name, lpObj->m_DetectSpeedHackTime, lpObj->m_SumLastAttackTime / lpObj->m_DetectCount, lpObj->m_SpeedHackPenalty);
		}
	} else {
		lpObj->m_SumLastAttackTime = 0;
		lpObj->m_DetectCount = 0;
	}

	lpObj->m_LastAttackTime = GetTickCount();

	if (bIsIgnorePacketSpeedHackDetect != 0 && lpObj->m_SpeedHackPenalty > 0) {
		lpObj->m_SpeedHackPenalty--;
		g_Log.Add("[%s][%s] Apply Attack Speed Penalty (%d left)", lpObj->AccountID, lpObj->Name, lpObj->m_SpeedHackPenalty);
		return false;
	}

	return true;
}

// Castle Siege
BOOL CObjUseSkill::SIEGE_SkillBrandOfSkill(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return false;

	LPOBJ lpObj = &gObj[aIndex];
	int tObjNum;

	if (lpObj->MapNumber != MAP_INDEX_CASTLESIEGE && lpObj->MapNumber != MAP_INDEX_CASTLEHUNTZONE && this->m_SkillData.EnableSiegeOnAllMaps == false) return false;

	int count = 0;

	BUFF_AddBuffEffect(lpObj, BUFFTYPE_DARKLORD_CASTLE_SKILL, 0, 0, 0, 0, 10);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, SIEGE_BRAND_OF_SKILL, lpObj->m_Index, 1);

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (ObjectMaxRange(tObjNum)) {
				if (lpObj->VpPlayer2[count].type == OBJ_USER) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) < 5) {
						BUFF_AddBuffEffect(&gObj[tObjNum], BUFFTYPE_DARKLORD_CASTLE_SKILL, 0, 0, 0, 0, 10);
						GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, SIEGE_BRAND_OF_SKILL, tObjNum, 1);
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	return true;
}
BOOL CObjUseSkill::SIEGE_SkillStun(int aIndex, int aTargetIndex, CMagicInf* lpMagic, BYTE btX, BYTE btY, BYTE btDir, BYTE btTargetPos) {
	if (ObjectMaxRange(aIndex) == false) return false;

	LPOBJ lpObj = &gObj[aIndex];
	int TargePosx = lpObj->X - (8 - (btTargetPos & 15));
	int TargePosy = lpObj->Y - (8 - ((btTargetPos & 240) >> 4));
	int iangle = this->GetAngle(lpObj->X, lpObj->Y, TargePosx, TargePosy);

	if (lpObj->MapNumber != MAP_INDEX_CASTLESIEGE && lpObj->MapNumber != MAP_INDEX_CASTLEHUNTZONE && this->m_SkillData.EnableSiegeOnAllMaps == false) return false;

	int tObjNum;
	int count = 0;

	gObjSetKillCount(aIndex, 0);

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			if (lpObj->VpPlayer2[count].type == OBJ_USER || lpObj->VpPlayer2[count].type == OBJ_MONSTER) {
				tObjNum = lpObj->VpPlayer2[count].number;

				if (ObjectMaxRange(tObjNum)) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) <= 4) {
						if (SkillElectricSparkHitBox.HitCheck(iangle, lpObj->X, lpObj->Y, gObj[tObjNum].X, gObj[tObjNum].Y)) {
							BUFF_AddBuffEffect(&gObj[tObjNum], DEBUFFTYPE_STUN, 0, 0, 0, 0, 10);
							GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, SIEGE_STUN, tObjNum, 1);
							CHARACTER_Immobilize(tObjNum, gObj[tObjNum].X, gObj[tObjNum].Y);
						}
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	return true;
}
BOOL CObjUseSkill::SIEGE_SkillRemoveStun(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return false;

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->MapNumber != MAP_INDEX_CASTLESIEGE && lpObj->MapNumber != MAP_INDEX_CASTLEHUNTZONE && this->m_SkillData.EnableSiegeOnAllMaps == false) return false;

	int tObjNum;
	int count = 0;

	gObjSetKillCount(aIndex, 0);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, SIEGE_REMOVESTUN, lpObj->m_Index, 1);

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (ObjectMaxRange(tObjNum)) {
				if (lpObj->VpPlayer2[count].type == OBJ_USER) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) < 6) {
						BUFF_RemoveBuffEffect(&gObj[tObjNum], DEBUFFTYPE_STUN);
						GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, SIEGE_REMOVESTUN, tObjNum, 1);
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	return true;
}
BOOL CObjUseSkill::SIEGE_SkillAddMana(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return false;

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->MapNumber != MAP_INDEX_CASTLESIEGE && lpObj->MapNumber != MAP_INDEX_CASTLEHUNTZONE && this->m_SkillData.EnableSiegeOnAllMaps == false) return false;

	int tObjNum;
	int count = 0;

	gObjSetKillCount(aIndex, 0);

	int iSwelledMana = short(lpObj->MaxMana + lpObj->AddMana) * 50 / 100;

	BUFF_AddBuffEffect(lpObj, BUFFTYPE_MANA_INC, EFFECTTYPE_INCREASE_MANA, iSwelledMana, 0, 0, 60);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, SIEGE_SWELLMANA, lpObj->m_Index, 1);

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (ObjectMaxRange(tObjNum) && lpObj->VpPlayer2[count].type == OBJ_USER) {
				if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) < 5) {
					if (lpObj->CharacterData->GuildNumber > 0 && gObj[tObjNum].CharacterData->GuildNumber > 0) {
						if (lpObj->CharacterData->GuildNumber != gObj[tObjNum].CharacterData->GuildNumber) {
							count++;
							continue;
						}

						iSwelledMana = short(gObj[tObjNum].MaxMana + gObj[tObjNum].AddMana) * 50 / 100;
						BUFF_AddBuffEffect(&gObj[tObjNum], BUFFTYPE_MANA_INC, EFFECTTYPE_INCREASE_MANA, iSwelledMana, 0, 0, 60);
						GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, SIEGE_SWELLMANA, tObjNum, 1);
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	return true;
}
BOOL CObjUseSkill::SIEGE_SkillCloaking(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return false;
	if (ObjectMaxRange(aTargetIndex) == false) return false;

	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpObj->MapNumber != MAP_INDEX_CASTLESIEGE && lpObj->MapNumber != MAP_INDEX_CASTLEHUNTZONE && this->m_SkillData.EnableSiegeOnAllMaps == false) return false;
	if (lpTargetObj->Type != OBJ_USER) {
		g_Log.Add("[ANTI-HACK][Cloaking Skill] [%s][%s] Invalid Targeting  TargetType [%d] TargetClass [%d]", lpObj->AccountID, lpObj->Name, lpTargetObj->Type, lpTargetObj->Class);
		return false;
	}

	gObjSetKillCount(aIndex, 0);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, SIEGE_INVISIBILITY, lpObj->m_Index, 1);
	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_INVISABLE, 0, 0, 0, 0, 60);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, SIEGE_INVISIBILITY, aTargetIndex, 1);
	return true;
}
BOOL CObjUseSkill::SIEGE_SkillRemoveCloaking(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return false;

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->MapNumber != MAP_INDEX_CASTLESIEGE && lpObj->MapNumber != MAP_INDEX_CASTLEHUNTZONE && this->m_SkillData.EnableSiegeOnAllMaps == false) return false;
	if ((lpObj->Authority & 0x20) == 0x20 && BUFF_TargetHasBuff(lpObj, BUFFTYPE_INVISABLE) == TRUE) return false;

	int tObjNum;
	int count = 0;

	gObjSetKillCount(aIndex, 0);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, SIEGE_REMOVEINVISIBILITY, lpObj->m_Index, 1);

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (ObjectMaxRange(tObjNum) && lpObj->VpPlayer2[count].type == OBJ_USER) {
				if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) < 5) {
					BUFF_RemoveBuffEffect(&gObj[tObjNum], BUFFTYPE_INVISABLE);
					GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, SIEGE_REMOVEINVISIBILITY, tObjNum, 1);
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	return true;
}
BOOL CObjUseSkill::SIEGE_SkillRemoveMagic(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return false;

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->MapNumber != MAP_INDEX_CASTLESIEGE && lpObj->MapNumber != MAP_INDEX_CASTLEHUNTZONE && this->m_SkillData.EnableSiegeOnAllMaps == false) return false;

	int tObjNum;
	int count = 0;

	gObjSetKillCount(aIndex, 0);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, SIEGE_ABOLISHMAGIC, lpObj->m_Index, 1);

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (ObjectMaxRange(tObjNum) && lpObj->VpPlayer2[count].type == OBJ_USER) {
				if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) < 5) {
					this->RemoveAllCharacterInvalidMagicAndSkillState(&gObj[tObjNum]);
					GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, SIEGE_ABOLISHMAGIC, tObjNum, 1);
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	return true;
}
BOOL CObjUseSkill::SIEGE_RemoveCloakingEffect(int aIndex) {
	if (ObjectMaxRange(aIndex) == false) return false;

	BUFF_RemoveBuffEffect(&gObj[aIndex], BUFFTYPE_INVISABLE);
	return true;
}

// Items
BOOL CObjUseSkill::SkillChangeUse(int aIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int skill_level = -1;

	if (lpObj->Type != OBJ_USER) return false;
	if (lpObj->pInventory[10].IsItem() == 1 && lpObj->pInventory[10].m_Type == ITEMGET(13, 10)) {
		if (lpObj->pInventory[10].m_Durability < 1.0f) skill_level = -1;
		else skill_level = lpObj->pInventory[10].m_Level;
	}

	if (lpObj->pInventory[11].IsItem() == 1 && skill_level == -1 && lpObj->pInventory[11].m_Type == ITEMGET(13, 10)) {
		if (lpObj->pInventory[11].m_Durability < 1.0f) skill_level = -1;
		else skill_level = lpObj->pInventory[11].m_Level;
	}

	if (skill_level > 7) return false;
	if (lpObj->pInventory[10].IsItem() == 1 && lpObj->pInventory[10].m_Type == ITEMGET(13, 39)) {
		if (lpObj->pInventory[10].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 372;
	}

	if (lpObj->pInventory[11].IsItem() == 1 && skill_level == -1 && lpObj->pInventory[11].m_Type == ITEMGET(13, 39)) {
		if (lpObj->pInventory[11].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 372;
	}

	if (lpObj->pInventory[10].IsItem() == 1 && lpObj->pInventory[10].m_Type == ITEMGET(13, 40)) {
		if (lpObj->pInventory[10].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 373;
	}

	if (lpObj->pInventory[11].IsItem() == 1 && skill_level == -1 && lpObj->pInventory[11].m_Type == ITEMGET(13, 40)) {
		if (lpObj->pInventory[11].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 373;
	}

	if (lpObj->pInventory[10].IsItem() == 1 && lpObj->pInventory[10].m_Type == ITEMGET(13, 41)) {
		if (lpObj->pInventory[10].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 374;
	}

	if (lpObj->pInventory[11].IsItem() == 1 && skill_level == -1 && lpObj->pInventory[11].m_Type == ITEMGET(13, 41)) {
		if (lpObj->pInventory[11].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 374;
	}

	if (lpObj->pInventory[10].IsItem() == 1 && lpObj->pInventory[10].m_Type == ITEMGET(13, 42)) {
		if (lpObj->pInventory[10].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 378;
	}

	if (lpObj->pInventory[11].IsItem() == 1 && skill_level == -1 && lpObj->pInventory[11].m_Type == ITEMGET(13, 42)) {
		if (lpObj->pInventory[11].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 378;
	}

	if (lpObj->pInventory[10].IsItem() == 1 && lpObj->pInventory[10].m_Type == ITEMGET(13, 68)) {
		if (lpObj->pInventory[10].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 477;
	}

	if (lpObj->pInventory[11].IsItem() == 1 && lpObj->pInventory[11].m_Type == ITEMGET(13, 68)) {
		if (lpObj->pInventory[11].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 477;
	}

	if (lpObj->pInventory[10].IsItem() == 1 && lpObj->pInventory[10].m_Type == ITEMGET(13, 76)) {
		if (lpObj->pInventory[10].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 503;
	}

	if (lpObj->pInventory[11].IsItem() == 1 && lpObj->pInventory[11].m_Type == ITEMGET(13, 76)) {
		if (lpObj->pInventory[11].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 503;
	}

	if (lpObj->pInventory[10].IsItem() == 1 && lpObj->pInventory[10].m_Type == ITEMGET(13, 77)) {
		if (lpObj->pInventory[10].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 616;
	}

	if (lpObj->pInventory[11].IsItem() == 1 && lpObj->pInventory[11].m_Type == ITEMGET(13, 77)) {
		if (lpObj->pInventory[11].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 616;
	}

	if (lpObj->pInventory[10].IsItem() == 1 && lpObj->pInventory[10].m_Type == ITEMGET(13, 78)) {
		if (lpObj->pInventory[10].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 617;
	}

	if (lpObj->pInventory[11].IsItem() == 1 && lpObj->pInventory[11].m_Type == ITEMGET(13, 78)) {
		if (lpObj->pInventory[11].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 617;
	}

	if (lpObj->pInventory[10].IsItem() == 1 && lpObj->pInventory[10].m_Type == ITEMGET(13, 122)) {
		if (lpObj->pInventory[10].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 548;
	}

	if (lpObj->pInventory[11].IsItem() == 1 && lpObj->pInventory[11].m_Type == ITEMGET(13, 122)) {
		if (lpObj->pInventory[11].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 548;
	}

	if (lpObj->pInventory[10].IsItem() == 1 && lpObj->pInventory[10].m_Type == ITEMGET(13, 163)) {
		if (lpObj->pInventory[10].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 625;
	}

	if (lpObj->pInventory[11].IsItem() == 1 && lpObj->pInventory[11].m_Type == ITEMGET(13, 163)) {
		if (lpObj->pInventory[11].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 625;
	}

	if (lpObj->pInventory[10].IsItem() == 1 && lpObj->pInventory[10].m_Type == ITEMGET(13, 164)) {
		if (lpObj->pInventory[10].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 626;
	}

	if (lpObj->pInventory[11].IsItem() == 1 && lpObj->pInventory[11].m_Type == ITEMGET(13, 164)) {
		if (lpObj->pInventory[11].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 626;
	}

	if (lpObj->pInventory[10].IsItem() == 1 && lpObj->pInventory[10].m_Type == ITEMGET(13, 165)) {
		if (lpObj->pInventory[10].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 642;
	}

	if (lpObj->pInventory[11].IsItem() == 1 && lpObj->pInventory[11].m_Type == ITEMGET(13, 165)) {
		if (lpObj->pInventory[11].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 642;
	}

	if (lpObj->pInventory[11].IsItem() == 1 && lpObj->pInventory[11].m_Type == ITEMGET(13, 166)) {
		if (lpObj->pInventory[11].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 646;
	}

	if (lpObj->pInventory[10].IsItem() == 1 && lpObj->pInventory[10].m_Type == ITEMGET(13, 268)) {
		if (lpObj->pInventory[10].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 706;
	}

	if (lpObj->pInventory[11].IsItem() == 1 && lpObj->pInventory[11].m_Type == ITEMGET(13, 268)) {
		if (lpObj->pInventory[11].m_Durability < 1.0f) skill_level = -1;
		else skill_level = 706;
	}

	if (IT_MAP_RANGE(lpObj->MapNumber)) {
		if (g_IT_Event.GetIllusionTempleState(lpObj->MapNumber)) {
			BYTE team = g_IT_Event.GetUserTeam(lpObj->MapNumber, lpObj->m_Index);

			if (team == 0) skill_level = 404;
			else if (team == 1) skill_level = 405;
		}
	}

	int change = -1;

	switch (skill_level) {
		case 0:		change = 2;				break;
		case 1:		change = 7;				break;
		case 2:		change = 14;			break;
		case 3:		change = 8;				break;
		case 4:		change = 9;				break;
		case 5:		change = 41;			break;
		case 372:	change = skill_level;	break;
		case 373:	change = skill_level;	break;
		case 374:	change = skill_level;	break;
		case 378:	change = skill_level;	break;
		case 404:
		case 405:	change = skill_level;	break;
		case 477:	change = skill_level;	break;
		case 503:	change = skill_level;	break;
		case 548:	change = skill_level;	break;
		case 616:	change = skill_level;	break;
		case 617:	change = skill_level;	break;
		case 642:	change = skill_level;	break;
		case 626:	change = skill_level;	break;
		case 625:	change = skill_level;	break;
		case 646:	change = skill_level;	break;
		case 706:	change = skill_level;	break;
		default:	change = -1;			break;
	}

	if (lpObj->m_Change >= 0 && lpObj->m_Change == change) return false;
	if (lpObj->m_Change == 41 && change != 41) {
		lpObj->AddLife = 0;
		GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
		GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->AddLife + lpObj->MaxLife, 0xFE, 0, lpObj->iAddShield + lpObj->iMaxShield);
	}

	if (lpObj->m_Change == 372 && change != 372) {
		lpObj->AddLife = 0;
		gObjCalCharacter.CHARACTER_Calc(lpObj->m_Index);
		GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
		GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->AddLife + lpObj->MaxLife, 0xFE, 0, lpObj->iAddShield + lpObj->iMaxShield);
	}

	if (lpObj->m_Change == 503 || lpObj->m_Change == 548 || lpObj->m_Change == 616 || lpObj->m_Change == 617) gObjCalCharacter.CHARACTER_Calc(lpObj->m_Index);

	lpObj->m_Change = change;
	gObjViewportListProtocolCreate(lpObj);

	if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE) g_CastleSiege.NotifySelfCsJoinSide(lpObj->m_Index);

	PMSG_KILLCOUNT pMsg = { 0 };

	PHeadSubSetB((LPBYTE)&pMsg, 0xB8, 0x01, sizeof(pMsg));
	pMsg.btKillCount = lpObj->m_btKillCount;
	IOCP.DataSend(aIndex, (PBYTE)&pMsg, sizeof(pMsg));
	return true;
}
int CObjUseSkill::FENRIR_PlasmaStorm(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	int tObjNum;
	int count = 0;
	int HitCount = 0;
	int bAttack;
	int DuelIndex = lpObj->m_iDuelUser;
	int EnableAttack;

	if (lpObj->Level < 300) return false;
	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if ((gObj[tObjNum].Class < 100 || gObj[tObjNum].Class > 110) && gObj[tObjNum].Class != 689) {
					if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
					else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
					else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
					else {
						int CallMonIndex = gObj[tObjNum].m_Index;

						if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
						if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
						if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
					}
				}

				if (EnableAttack != 0) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) < 6) {
						bAttack = 0;

						if (HitCount >= 5) bAttack = 0;
						else bAttack = 1;
						if (tObjNum == aTargetIndex) bAttack = 1;
						if (bAttack != 0) {
							gObjAddAttackProcMsgSendDelay(lpObj, 53, tObjNum, 300, lpMagic->m_Skill, 0);
							HitCount++;
						}
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	return true;
}

// Monsters
int CObjUseSkill::MEDUSA_Decay(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int tObjNum;
	int count = 0;
	int EnableAttack;
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_USER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				if (EnableAttack == TRUE) {
					if (this->SKILL_CalcDistance(x, y, gObj[tObjNum].X, gObj[tObjNum].Y) < 8) {
						if (aIndex != tObjNum) {
							if (CHARACTER_CalcDistance(lpTargetObj, &gObj[tObjNum]) <= 3) {
								int delay = rand() % 500;

								gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, delay, lpMagic->m_Skill, 0);

								if (BUFF_TargetHasBuff(&gObj[tObjNum], DEBUFFTYPE_POISON) == FALSE) {
									if (!CHARACTER_ResistanceProc(&gObj[tObjNum], 1)) {
										gObj[tObjNum].lpAttackObj = lpObj;
										BUFF_AddBuffEffect(&gObj[tObjNum], DEBUFFTYPE_POISON, EFFECTTYPE_DOT_POISON, 3, 0, 0, 20);
									}
								}
							}
						}
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	return true;
}
BOOL CObjUseSkill::KUNDUN_Summon(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return false;

	int iSummonCount = 1;
	int iSummonMonterType = 0;
	BYTE bDoSummon = 1;
	BYTE bSummonSuccess = 0;
	BYTE cSummonX = 0;
	BYTE cSummonY = 0;
	LPOBJ lpObj = &gObj[aIndex];

	switch (lpObj->Class) {
		case 161:
			iSummonMonterType = rand() % 2 + 147;
			bDoSummon = (rand() % 15) ? false : true;
		break;

		case 181:
			iSummonMonterType = rand() % 2 + 177;
			bDoSummon = (rand() % 15) ? false : true;
		break;

		case 189:
			iSummonMonterType = rand() % 2 + 185;
			bDoSummon = (rand() % 15) ? false : true;
		break;

		case 197:
			iSummonMonterType = rand() % 2 + 193;
			bDoSummon = (rand() % 15) ? false : true;
		break;

		case 267:
			iSummonMonterType = rand() % 2 + 263;
			bDoSummon = (rand() % 15) ? false : true;
		break;

		case 275:
			iSummonMonterType = rand() % 2 + 271;
			bDoSummon = (rand() % 15) ? false : true;
		break;
	}

	if (bDoSummon == false) return false;
	for (int iSC = 0; iSC < iSummonCount; iSC++) {
		BYTE cSX = lpObj->X;
		BYTE cSY = lpObj->Y;

		if (gObjGetRandomFreeLocation(lpObj->MapNumber, (BYTE&)cSX, (BYTE&)cSY, 2, 2, 30) == 1) {
			int result = gObjAddMonster(lpObj->MapNumber);

			if (result >= 0) {
				gObj[result].m_PosNum = (WORD)-1;
				gObj[result].X = cSX;
				gObj[result].Y = cSY;
				gObj[result].MapNumber = lpObj->MapNumber;
				gObj[result].TX = gObj[result].X;
				gObj[result].TY = gObj[result].Y;
				gObj[result].m_OldX = gObj[result].X;
				gObj[result].m_OldY = gObj[result].Y;
				gObj[result].Dir = 1;
				gObj[result].StartX = gObj[result].X;
				gObj[result].StartY = gObj[result].Y;
				gObjSetMonster(result, iSummonMonterType);
				gObj[result].m_Attribute = 60;
				gObj[result].MaxRegenTime = 0;
				gObj[result].Dir = rand() % 8;
				bSummonSuccess = 1;
				cSummonX = gObj[result].StartX;
				cSummonY = gObj[result].StartY;
			}
		}
	}

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, KUNDUN_SUMMON, (((cSummonX & 0xFF) & 0xFF) | ((cSummonY & 0xFF) & 0xFF) << 8) & 0xFFFF, bSummonSuccess);
	return bSummonSuccess;
}
void CObjUseSkill::SILVESTER_Summon(int aIndex, int iTargetIndex, CMagicInf* lpMagic) {
	if (!ObjectMaxRange(aIndex)) return;

	int nCallMonster[3];
	nCallMonster[0] = 678;
	nCallMonster[1] = 679;
	nCallMonster[2] = 680;
	LPOBJ lpObj = &gObj[aIndex];

	for (int iSC = 0; iSC < 3; iSC++) {
		BYTE cSX = lpObj->X;
		BYTE cSY = lpObj->Y;

		if (gObjGetRandomFreeLocation(lpObj->MapNumber, cSX, cSY, 5, 5, 30) == TRUE) {
			int result = gObjAddMonster(lpObj->MapNumber);
			int nMonsterIndex = nCallMonster[iSC];
			LPMONSTER_ATTRIBUTE lpm = gMAttr.GetAttr(nMonsterIndex);

			if (result >= 0) {
				gObj[result].m_PosNum = -1;
				gObj[result].X = cSX;
				gObj[result].Y = cSY;
				gObj[result].MapNumber = lpObj->MapNumber;
				gObj[result].TX = gObj[result].X;
				gObj[result].TY = gObj[result].Y;
				gObj[result].m_OldX = gObj[result].X;
				gObj[result].m_OldY = gObj[result].Y;
				gObj[result].Dir = 1;
				gObj[result].StartX = gObj[result].X;
				gObj[result].StartY = gObj[result].Y;
				gObjSetMonster(result, nMonsterIndex);
				gObj[result].m_Attribute = 100;
				gObj[result].MaxRegenTime = 0;
				gObj[result].Dir = rand() % 8;
				gObj[result].m_RecallMon = aIndex;
				lpObj->m_nRecallMonIndex[iSC] = result;

				if (lpm->m_PentagramMainAttribute == 6) {
					switch (rand() % 5) {
						case 0:
							gObj[result].m_iPentagramMainAttribute = EL_FIRE;
						break;

						case 1:
							gObj[result].m_iPentagramMainAttribute = EL_WATER;
						break;

						case 2:
							gObj[result].m_iPentagramMainAttribute = EL_EARTH;
						break;

						case 3:
							gObj[result].m_iPentagramMainAttribute = EL_WIND;
						break;

						case 4:
							gObj[result].m_iPentagramMainAttribute = EL_DARKNESS;
						break;
					}
				} else if (lpm->m_PentagramMainAttribute > 0) gObj[result].m_iPentagramMainAttribute = lpm->m_PentagramMainAttribute;

				gObj[result].m_iPentagramAttributePattern = lpm->m_PentagramAttributePattern;
				gObj[result].m_iPentagramDefense = lpm->m_PentagramDefense;
				gObj[result].m_iPentagramAttackMin = lpm->m_PentagramAttackMin;
				gObj[result].m_iPentagramAttackMax = lpm->m_PentagramAttackMax;
				gObj[result].m_iPentagramAttackRating = lpm->m_PentagramAttackRating;
				gObj[result].m_iPentagramDefenseRating = lpm->m_PentagramDefenseRating;
				g_Log.Add("[LORDSILVESTER]Create %s MAP(%d)/X(%d)/Y(%d)", gObj[result].Name, gObj[result].MapNumber, gObj[result].X, gObj[result].Y);
			}
		}
	}
}
BOOL CObjUseSkill::SkillImmuneToMagic(int aIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return false;

	LPOBJ lpObj = &gObj[aIndex];

	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_MONSTER_MAGIC_IMMUNE) == 0 && BUFF_TargetHasBuff(lpObj, BUFFTYPE_MONSTER_MELEE_IMMUNE) == 0) {
		BUFF_AddBuffEffect(lpObj, BUFFTYPE_MONSTER_MAGIC_IMMUNE, 0, 0, 0, 0, 10);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, AT_SKILL_IMMUNE_TO_MAGIC, lpObj->m_Index, 1);
		return true;
	}

	return false;
}
BOOL CObjUseSkill::SkillImmuneToHarm(int aIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return false;

	LPOBJ lpObj = &gObj[aIndex];

	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_MONSTER_MAGIC_IMMUNE) == 0 && BUFF_TargetHasBuff(lpObj, BUFFTYPE_MONSTER_MELEE_IMMUNE) == 0) {
		BUFF_AddBuffEffect(lpObj, BUFFTYPE_MONSTER_MELEE_IMMUNE, 0, 0, 0, 0, 10);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, AT_SKILL_IMMUNE_TO_HARM, lpObj->m_Index, 1);
		return true;
	}

	return false;
}
void CObjUseSkill::SplashDamage(LPOBJ lpObj, LPOBJ lpTargetObj, int nSkill, int nAttackDamage, int nDistance, int nDamageRate) {
	if (gObjIsConnected(lpObj) && gObjIsConnected(lpTargetObj)) {
		if (nAttackDamage > 0) {
			int iCount = 0;
			BOOL bEnableAttack = FALSE;

			while (true) {
				if (lpObj->VpPlayer2[iCount].state) {
					int iTarObjNum = lpObj->VpPlayer2[iCount].number;

					if (iTarObjNum >= 0) {
						bEnableAttack = FALSE;

						if (lpObj->VpPlayer2[iTarObjNum].type == OBJ_MONSTER && gObj[iTarObjNum].m_RecallMon < 0) bEnableAttack = TRUE;
						if (lpTargetObj->m_cChaosCastleIndex != -1 && lpTargetObj->m_cChaosCastleIndex == gObj[iTarObjNum].m_cChaosCastleIndex) bEnableAttack = TRUE;
						if (lpObj->Type == OBJ_USER && lpTargetObj->Type == OBJ_USER && gObj[iTarObjNum].Type != OBJ_NPC) bEnableAttack = TRUE;
						if (gObj[iTarObjNum].Class >= 678 && gObj[iTarObjNum].Class <= 680) bEnableAttack = TRUE;
						if (bEnableAttack == TRUE) {
							if (lpObj->m_Index != iTarObjNum) {
								if (CHARACTER_CalcDistance(lpTargetObj, &gObj[iTarObjNum]) <= nDistance) gObjAddMsgSendDelay(lpObj, 58, iTarObjNum, 300, nDamageRate * nAttackDamage / 100);
							}
						}
					}
				}

				iCount++;

				if (iCount > MAX_VIEWPORT - 1) break;
			}
		}
	}
}
int CObjUseSkill::EvilMonsterSkill(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	int tObjNum;
	int count = 0;
	int attackcount = 0;
	int EnableAttack = FALSE;

	while (true) {
		tObjNum = lpObj->VpPlayer2[count].number;
		EnableAttack = FALSE;

		if (tObjNum >= 0 && lpObj->VpPlayer2[count].type == OBJ_USER) {
			EnableAttack = TRUE;

			if (gObj[tObjNum].TargetNumber >= 0) {
				if (tObjNum != aTargetIndex)	EnableAttack = FALSE;
			}

			if (EnableAttack == TRUE && CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) < 6) {
				BOOL Attack = attackcount < 5;

				if (tObjNum == aTargetIndex) Attack = TRUE;
				if (Attack) {
					gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 300, lpMagic->m_Skill, 0);
					attackcount++;
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	return TRUE;
}

// Dark Knight
BOOL CObjUseSkill::DK_SunwaveSlash(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return false;

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->MapNumber != MAP_INDEX_CASTLESIEGE && lpObj->MapNumber != MAP_INDEX_CASTLEHUNTZONE && this->m_SkillData.EnableSiegeOnAllMaps == false) return false;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 1);
	gObjAttack(lpObj, &gObj[aTargetIndex], lpMagic, 0, 1, 0, 0, 0, 0);
	return true;
}
void CObjUseSkill::DK_TwistingSlash(int aIndex, CMagicInf* lpMagic, int aTargetIndex, BOOL isCombo) {
	LPOBJ lpObj = &gObj[aIndex];
	int tObjNum;
	int count = 0;
	int HitCount = 0;
	int bAttack;
	int DuelIndex = lpObj->m_iDuelUser;
	int EnableAttack;

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack != 0) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) <= MagicDamageC.GetSkillDistance(lpMagic->m_Skill)) {
						bAttack = 0;

						if (HitCount > 15) bAttack = 0;
						if (HitCount >= 10) {
							if (rand() % 2) bAttack = 1;
						} else bAttack = 1;
						if (bAttack != 0) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 0, 1, 0, isCombo, 0, 0);
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}
void CObjUseSkill::DK_LifeSwell(int aIndex, CMagicInf* lpMagic) {
	int skillSuccess = true;
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJ_USER && lpObj->m_RecallMon == -1) return;
	if ((lpObj->Type == OBJ_USER) && (lpObj->Class != CLASS_KNIGHT && lpObj->Class != CLASS_MAGICGLADIATOR && lpObj->CharacterData->ISBOT == false)) return;
	if (lpObj->Level < 120) {
		MsgOutput(aIndex, Lang.GetText(0, 503));
		return;
	}

	int number;
	int partynum = 0;
	int totallevel = 0;
	int partycount;
	int dis;
	float skillbonus = 0;
	int ApplyPartyIndex[MAX_USER_IN_PARTY];
	int viewplayer = 0;
	int viewpercent = 0;
	LPOBJ lpPartyObj;

	memset(ApplyPartyIndex, -1, sizeof(ApplyPartyIndex));
	partynum = lpObj->PartyNumber;

	if (partynum != -1) {
		partycount = gParty.m_PartyS[partynum].Count;

		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			number = gParty.m_PartyS[partynum].Number[n];

			if (number >= 0) {
				lpPartyObj = &gObj[number];

				if (lpObj->MapNumber == lpPartyObj->MapNumber) {
					dis = CHARACTER_CalcDistance(lpObj, &gObj[number]);

					if (dis < 10) {
						ApplyPartyIndex[n] = lpPartyObj->m_Index;
						viewplayer++;
					}
				}
			}
		}
	}

	if (viewplayer > 1) {
		if (viewplayer == 2) viewpercent = 2;
		else if (viewplayer == 3) viewpercent = 5;
		else if (viewplayer == 4) viewpercent = 8;
		else viewpercent = 10;
	}

	double iaddLifepower = 0.0;
	int iLifeTime = 0;
	int iaddlife = 0;

	this->m_Lua.Generic_Call("LifeSwell", "iii>di", (lpObj->CharacterData->Vitality + lpObj->AddVitality), (lpObj->CharacterData->Energy + lpObj->AddEnergy), viewpercent, &iaddLifepower, &iLifeTime);

	if (lpObj->Type == OBJ_USER && partynum == -1 && lpObj->CharacterData->ISBOT == false) {
		if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_HP_INC)) BUFF_RemoveBuffEffect(lpObj, BUFFTYPE_HP_INC);

		iaddlife = ((int(lpObj->MaxLife) + int(lpObj->AddLife)) * iaddLifepower / 100);

		if (iaddlife > 3000000) iaddlife = 3000000;

		BUFF_AddBuffEffect(lpObj, BUFFTYPE_HP_INC, EFFECTTYPE_INCREASE_LIFE, iaddlife, 0, 0, iLifeTime);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, skillSuccess);
		gObjCalCharacter.CHARACTER_Calc(lpObj->m_Index);
	} else if (lpObj->Type == OBJ_USER && lpObj->CharacterData->ISBOT == true) {
		iaddlife = ((int(lpObj->MaxLife) + int(lpObj->AddLife)) * iaddLifepower / 100);

		if (iaddlife > 3000000) iaddlife = 3000000;

		iLifeTime = g_BotSystem.GetSkillTime(lpObj->m_Index, lpMagic->m_Skill);
		BUFF_AddBuffEffect(&gObj[lpObj->BuffPlayerIndex], BUFFTYPE_HP_INC, EFFECTTYPE_INCREASE_LIFE, iaddlife, 0, 0, iLifeTime);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, skillSuccess);
		gObjCalCharacter.CHARACTER_Calc(lpObj->BuffPlayerIndex);
	} else {
		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			if (ApplyPartyIndex[n] != -1) {
				lpPartyObj = &gObj[ApplyPartyIndex[n]];

				if (BUFF_TargetHasBuff(lpPartyObj, BUFFTYPE_HP_INC)) BUFF_RemoveBuffEffect(lpPartyObj, BUFFTYPE_HP_INC);

				iaddlife = (lpPartyObj->MaxLife + lpPartyObj->AddLife) * iaddLifepower / 100;
				BUFF_AddBuffEffect(lpPartyObj, BUFFTYPE_HP_INC, EFFECTTYPE_INCREASE_LIFE, iaddlife, 0, 0, iLifeTime);
				GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpPartyObj->m_Index, skillSuccess);
				gObjCalCharacter.CHARACTER_Calc(ApplyPartyIndex[n]);
			}
		}
	}
}
BOOL CObjUseSkill::DK_DeathStab(int aIndex, int aTargetIndex, CMagicInf* lpMagic, BOOL isCombo) {
	LPOBJ lpObj = &gObj[aIndex];
	int StartDis = 1;
	int tObjNum;
	float fangle = this->GetAngle(lpObj->X, lpObj->Y, gObj[aTargetIndex].X, gObj[aTargetIndex].Y);

	this->SkillFrustrum(aIndex, fangle, 1.5f, 3.0f);

	int count = 0;
	int loopcount = 0;
	int attackcheck;
	int EnableAttack;
	int DuelIndex = lpObj->m_iDuelUser;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 1);

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack != 0) {
					attackcheck = 0;

					if (loopcount == 0) attackcheck = 1;
					else if (rand() % 3 == 0) attackcheck = 1;
					if (attackcheck != 0) {
						if (::SkillTestFrustrum(gObj[tObjNum].X, gObj[tObjNum].Y, aIndex)) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 0, 1, 0, isCombo, 0, 0);
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) {
			loopcount++;
			count = 0;

			if (loopcount >= 3) break;
		}
	}

	return true;
}
void CObjUseSkill::DK_RagefulBlow(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, BYTE dir, BYTE TargetPos, int aTargetIndex, BOOL isCombo) {
	LPOBJ lpObj = &gObj[aIndex];
	int tObjNum;
	int count = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int HitCount = 0;
	int bAttack = 0;
	int EnableAttack;

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack != 0) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) <= MagicDamageC.GetSkillDistance(lpMagic->m_Skill)) {
						bAttack = 0;

						if (HitCount > 15) bAttack = 0;
						if (HitCount >= 10) {
							if (rand() % 2) bAttack = 1;
						} else bAttack = 1;
						if (bAttack != 0) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 500, lpMagic->m_Skill, isCombo);
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}
int CObjUseSkill::DK_Groundslam(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex, BOOL bCombo) {
	if (ObjectMaxRange(aIndex) == 0) return FALSE;

	LPOBJ lpObj = &gObj[aIndex];
	BOOL EnableAttack;

	for (int n = 0; n < MAX_VIEWPORT; n++) {
		if (lpObj->VpPlayer2[n].state <= 0) continue;

		EnableAttack = FALSE;

		int index = lpObj->VpPlayer2[n].number;

		if (gObj[index].Type == OBJ_MONSTER && gObj[index].m_RecallMon < 0) EnableAttack = TRUE;
		else if (aTargetIndex == index || index == lpObj->m_iDuelUser) EnableAttack = TRUE;
		else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && gObj[index].Type != OBJ_NPC && gObj[index].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;

		int summon = aIndex;

		if (gObj[aIndex].Type == OBJ_MONSTER && gObj[aIndex].m_RecallMon >= 0) summon = gObj[aIndex].m_RecallMon;
		if (gObjTargetGuildWarCheck(&gObj[summon], lpObj) != 0) EnableAttack = TRUE;
		if (this->SKILL_CalcDistance(x, y, gObj[index].X, gObj[index].Y) < 3 && EnableAttack) {
			gObjAddAttackProcMsgSendDelay(lpObj, 50, index, 400, lpMagic->m_Skill, bCombo);

			if (bCombo != 0) GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, 59, aIndex, 1);
		}
	}

	return TRUE;
}

// Dark Wizard
BOOL CObjUseSkill::DW_SoulGlaives(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return false;

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->MapNumber != MAP_INDEX_CASTLESIEGE && lpObj->MapNumber != MAP_INDEX_CASTLEHUNTZONE && this->m_SkillData.EnableSiegeOnAllMaps == false) return false;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 1);

	int iDelay = 500;

	gObjAddAttackProcMsgSendDelay(lpObj, 50, aTargetIndex, iDelay, lpMagic->m_Skill, 0);
	gObjAddAttackProcMsgSendDelay(lpObj, 50, aTargetIndex, iDelay + 200, lpMagic->m_Skill, 0);
	return true;
}
void CObjUseSkill::DW_ManaShield(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	int skillSuccess = true;
	double skillInc = 0.0;
	int skillTime = 0;
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpObj->Type != OBJ_USER && lpObj->m_RecallMon == -1) return;
	if (lpObj->Class != CLASS_WIZARD && lpObj->Class != CLASS_MAGICGLADIATOR && lpObj->CharacterData->ISBOT == false) return;
	if (lpObj->PartyNumber != lpTargetObj->PartyNumber && lpObj->CharacterData->ISBOT == false) return;

	this->m_Lua.Generic_Call("ManaShield", "ii>di", (lpObj->CharacterData->Dexterity + lpObj->AddDexterity), (lpObj->CharacterData->Energy + lpObj->AddEnergy), &skillInc, &skillTime);

	if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_MANA_SHIELD) == true) BUFF_RemoveBuffEffect(lpTargetObj, BUFFTYPE_MANA_SHIELD);
	if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_IMPROVED_MANA_SHIELD) == true) BUFF_RemoveBuffEffect(lpTargetObj, BUFFTYPE_IMPROVED_MANA_SHIELD);

	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_MANA_SHIELD, EFFECTTYPE_MANA_SHIELD, skillInc, EFFECTTYPE_UNUSED, skillInc, skillTime);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpTargetObj->m_Index, skillSuccess);
}
int CObjUseSkill::DW_IceStorm(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, BYTE dir, BYTE TargetPos, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int tObjNum;
	int count = 0;
	int FirstHit = 0;
	int HitCount = 0;
	int bAttack;
	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	int DuelIndex = lpObj->m_iDuelUser;
	int EnableAttack;

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack != FALSE) {
					if (this->SKILL_CalcDistance(x, y, gObj[tObjNum].X, gObj[tObjNum].Y) < 3/*MagicDamageC.GetSkillDistance(lpMagic->m_Skill)*/) {
						bAttack = FALSE;

						if (HitCount > 10) bAttack = FALSE;
						else bAttack = TRUE;
						if (bAttack != FALSE) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 1, 0, 0, 0, 0);
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	return true;
}
int CObjUseSkill::DW_Decay(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, BYTE dir, BYTE TargetPos, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int tObjNum;
	int count = 0;
	int FirstHit = 0;
	int HitCount = 0;
	int bAttack;
	int DuelIndex = lpObj->m_iDuelUser;
	int EnableAttack;
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack != 0) {
					if (this->SKILL_CalcDistance(x, y, gObj[tObjNum].X, gObj[tObjNum].Y) < 3) {
						bAttack = 0;

						if (HitCount > 10) bAttack = 0;
						if (HitCount >= 5) {
							if (rand() % 2) bAttack = 1;
						} else bAttack = 1;
						if (bAttack != 0) {
							int delay = rand() % 500;
							gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, delay, lpMagic->m_Skill, 0);
						}
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	return true;
}
BOOL CObjUseSkill::DW_Nova(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (aTargetIndex == 58) return this->DW_NovaStart(aIndex, lpMagic);

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJ_USER) return false;

	int tObjNum;

	if (lpObj->CharacterData->SkillHellFire2State == 0) {
		g_Log.Add("[%s][%s] Nova skill didn't cast", lpObj->AccountID, lpObj->Name); //HermeX Fix
		return false;
	}

	int EnableAttack;
	int DuelIndex = lpObj->m_iDuelUser;
	int count = 0;

	lpObj->CharacterData->SkillHellFire2State = 0;
	lpObj->CharacterData->SkillHellFire2Time = 0;

	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (aTargetIndex == tObjNum || DuelIndex == tObjNum) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack != FALSE) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) <= MagicDamageC.GetSkillDistance(lpMagic->m_Skill)) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 600, lpMagic->m_Skill, 0);
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, DW_NOVA, lpObj->m_Index, 1);
	return true;
}
BOOL CObjUseSkill::DW_NovaStart(int aIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJ_USER) return false;
	if (lpObj->CharacterData->SkillHellFire2State != 0) {
		g_Log.Add("[%s][%s] Nova skill already casted", lpObj->AccountID, lpObj->Name); // HermeX Fix
		return false;
	}

	lpObj->CharacterData->SkillHellFire2Time = GetTickCount();
	lpObj->CharacterData->SkillHellFire2State = 1;
	lpObj->CharacterData->SkillHellFire2Count = 0;
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, DW_NOVA_START, aIndex, 1);
	return true;
}
int CObjUseSkill::DW_ArcanePower(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (gObj[aIndex].CharacterData->ISBOT == true) BUFF_AddBuffEffect(&gObj[gObj[aIndex].BuffPlayerIndex], BUFFTYPE_MAGIC_POWER_INC, EFFECTTYPE_INCREASE_MINIMUM_SPELL_DAMAGE, 20, 0, 0, g_BotSystem.GetSkillTime(aIndex, lpMagic->m_Skill));
	else {
		LPOBJ lpObj = &gObj[aIndex];
		int iEffect = (gObj[aIndex].CharacterData->Energy + gObj[aIndex].AddEnergy) / 6 * 0.20;

		BUFF_AddBuffEffect(lpObj, BUFFTYPE_MAGIC_POWER_INC, EFFECTTYPE_INCREASE_MINIMUM_SPELL_DAMAGE, iEffect, 0, 0, 1800);
	}

	GSProtocol.PROTOCOL_MagicAttackNumberSend(&gObj[aIndex], lpMagic->m_Skill, aIndex, 1);
	return TRUE;
}

// Elf
BOOL CObjUseSkill::ELF_Starfall(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return false;

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->MapNumber != MAP_INDEX_CASTLESIEGE && lpObj->MapNumber != MAP_INDEX_CASTLEHUNTZONE && this->m_SkillData.EnableSiegeOnAllMaps == false) return false;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 1);
	gObjAddAttackProcMsgSendDelay(lpObj, 50, aTargetIndex, 800, lpMagic->m_Skill, 0);
	return true;
}
void CObjUseSkill::ELF_Heal(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	int skillSuccess = true;
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpTargetObj->Type != OBJ_USER && lpTargetObj->m_RecallMon == -1) return;
	if (lpObj->Class != CLASS_ELF && lpObj->CharacterData->ISBOT == false) return;
	if ((CC_MAP_RANGE(lpObj->MapNumber) || lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) && lpObj->m_Index != lpTargetObj->m_Index) return;

	int addlife = 0;

	this->m_Lua.Generic_Call("Heal", "i>i", lpObj->CharacterData->Energy + lpObj->AddEnergy, &addlife);
	lpTargetObj->Life += addlife;

	if ((lpTargetObj->MaxLife + lpTargetObj->AddLife) < lpTargetObj->Life) lpTargetObj->Life = lpTargetObj->MaxLife + lpTargetObj->AddLife;
	if (lpTargetObj->Type == OBJ_USER) {
		GSProtocol.PROTOCOL_ReFillSend(lpTargetObj->m_Index, lpTargetObj->Life, 0xFF, 0, lpTargetObj->iShield);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, ELF_HEAL, lpTargetObj->m_Index, skillSuccess);

		for (int n = 0; n < MAX_VIEWPORT; n++) {
			if (lpObj->VpPlayer2[n].type == OBJ_MONSTER && lpObj->VpPlayer2[n].state != 0) {
				LPOBJ lpMonster = &gObj[lpObj->VpPlayer2[n].number];

				if (lpMonster->m_iCurrentAI != 0) lpMonster->m_Agro->IncAgro(lpObj->m_Index, (lpObj->CharacterData->Energy / 5) / 40);
			}
		}
	} else if (lpTargetObj->m_RecallMon >= 0) {
		GSProtocol.GCRecallMonLife(lpTargetObj->m_RecallMon, lpTargetObj->MaxLife, lpTargetObj->Life);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, ELF_HEAL, lpTargetObj->m_Index, skillSuccess);
	}
}
void CObjUseSkill::ELF_Enhance(int aIndex, int aTargetIndex, CMagicInf * lpMagic) {
	int skillSuccess = true;
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpTargetObj->Type != OBJ_USER && lpTargetObj->m_RecallMon == -1) return;
	if (lpObj->Class != CLASS_ELF && lpObj->CharacterData->ISBOT == false) return;
	if ((CC_MAP_RANGE(lpObj->MapNumber) || lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) && lpObj->m_Index != lpTargetObj->m_Index) return;

	double skill_defense = 0.0;
	double skill_time = 0.0;

	this->m_Lua.Generic_Call("Enhance", "i>dd", lpObj->CharacterData->Energy + lpObj->AddEnergy, &skill_defense, &skill_time);

	if (lpObj->CharacterData->ISBOT == true) skill_time = g_BotSystem.GetSkillTime(aIndex, lpMagic->m_Skill);

	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_ENHANCE, EFFECTTYPE_INCREASE_DEFENSE, skill_defense, 0, 0, skill_time);

	for (int n = 0; n < MAX_VIEWPORT; n++) {
		if (lpObj->VpPlayer2[n].type == OBJ_MONSTER && lpObj->VpPlayer2[n].state != 0) {
			LPOBJ lpMonster = &gObj[lpObj->VpPlayer2[n].number];

			if (lpMonster->m_iCurrentAI != 0) lpMonster->m_Agro->IncAgro(lpObj->m_Index,int(skill_defense) / 10);
		}
	}

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, ELF_ENHANCE, lpTargetObj->m_Index, skillSuccess);
}
void CObjUseSkill::ELF_Empower(int aIndex, int aTargetIndex, CMagicInf * lpMagic) {
	int skillSuccess = true;
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpTargetObj->Type != OBJ_USER && lpTargetObj->m_RecallMon == -1) return;
	if (lpObj->Class != CLASS_ELF && lpObj->CharacterData->ISBOT == false) return;
	if ((CC_MAP_RANGE(lpObj->MapNumber) || lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) && lpObj->m_Index != lpTargetObj->m_Index) return;

	double skill_attack = 0.0;
	double skill_time = 0.0;

	this->m_Lua.Generic_Call("Empower", "i>dd", lpObj->CharacterData->Energy + lpObj->AddEnergy, &skill_attack, &skill_time);

	if (lpObj->CharacterData->ISBOT == true) skill_time = g_BotSystem.GetSkillTime(aIndex, lpMagic->m_Skill);
	if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_AMPLIFY_DAMAGE) == TRUE) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, ELF_EMPOWER, lpTargetObj->m_Index, FALSE);
		return;
	} else if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_IMPROVED_AMPLIFY_DAMAGE) == TRUE) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, ELF_EMPOWER, lpTargetObj->m_Index, FALSE);
		return;
	} else if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_DIMENSIONAL_AFFINITY) == TRUE) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, ELF_EMPOWER, lpTargetObj->m_Index, FALSE);
		return;
	} else if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_VOID_AFFINITY) == TRUE) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, ELF_EMPOWER, lpTargetObj->m_Index, FALSE);
		return;
	}

	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_EMPOWER, EFFECTTYPE_INCREASE_DAMAGE, skill_attack, 0, 0, skill_time);

	for (int n = 0; n < MAX_VIEWPORT; n++) {
		if (lpObj->VpPlayer2[n].type == OBJ_MONSTER && lpObj->VpPlayer2[n].state != 0) {
			LPOBJ lpMonster = &gObj[lpObj->VpPlayer2[n].number];

			if (lpMonster->m_iCurrentAI != 0) lpMonster->m_Agro->IncAgro(lpObj->m_Index,int(skill_attack)/10);
		}
	}

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, ELF_EMPOWER, lpTargetObj->m_Index, skillSuccess);
}
int CObjUseSkill::ELF_RemoveBuff(LPOBJ lpObj, int iSkillNumber) {
	if (lpObj->Type != OBJ_USER) return false;

	int BuffNum = 0;

	if (iSkillNumber == ELF_EMPOWER) BuffNum = 1;
	else if (iSkillNumber == ELF_ENHANCE) BuffNum = 2;

	BUFF_RemoveBuffEffect(lpObj, BuffNum);
	return true;
}
int CObjUseSkill::ELF_Summon(int aIndex, int MonsterType, int x, int y) {
	if (gObj[aIndex].Type != OBJ_USER) return false;
	if (gObj[aIndex].MapNumber == MAP_INDEX_ICARUS) return false;
	if (CC_MAP_RANGE(gObj[aIndex].MapNumber) || gObj[aIndex].MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) return false;
	if (gObj[aIndex].m_RecallMon >= 0) {
		GSProtocol.GCRecallMonLife(aIndex, 60, 0);
		gObjMonsterCallKill(aIndex);
		return false;
	}

	int result = gObjAddCallMon();

	if (result >= 0)	{
		gObj[result].X = x;
		gObj[result].Y = y;
		gObj[result].MTX = x;
		gObj[result].MTY = y;
		gObj[result].Dir = 2;
		gObj[result].MapNumber = gObj[aIndex].MapNumber;
		gObjSetMonster(result, MonsterType);
		gObj[result].m_Attribute = 100;
		gObj[result].TargetNumber = (WORD) - 1;
		gObj[result].m_ActState.Emotion = 0;
		gObj[result].m_ActState.Attack = 0;
		gObj[result].m_ActState.EmotionCount = 0;
		gObj[result].PathCount = 0;
		gObj[result].m_MoveRange = 15;
		gObj[result].m_RecallMon = aIndex;
		gObj[aIndex].m_RecallMon = result;

		if (gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionLife > 0.0) {
			gObj[result].Life += gObj[result].Life * gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionLife / 100.0;
			gObj[result].MaxLife += gObj[result].MaxLife * gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionLife / 100.0;
		}

		if (gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionDefensive > 0.0) {
			gObj[result].Defense += gObj[result].Defense * gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionDefensive / 100.0;
			gObj[result].EvasionRating += gObj[result].EvasionRating * gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionDefensive / 100.0;
		}

		if (gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionOffensive > 0.0) {
			gObj[result].m_AttackDamageMin += gObj[result].m_AttackDamageMin * gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionOffensive / 100.0;
			gObj[result].m_AttackDamageMax += gObj[result].m_AttackDamageMax * gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionOffensive / 100.0;
			gObj[result].HitRating += gObj[result].HitRating * gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionOffensive / 100.0;
		}

		GSProtocol.GCRecallMonLife(gObj[result].m_RecallMon, gObj[result].MaxLife, gObj[result].Life);
		return true;
	}

	return false;
}
int CObjUseSkill::ELF_InfinityArrow(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJ_USER) return false;
	if (lpObj->Class != CLASS_ELF || lpObj->CharacterData->ChangeUP == 0) return false;
	if (lpObj->m_Index != aTargetIndex) return false;

	int Time = g_SkillAdditionInfo.GetInfinityArrowSkillTime();

	BUFF_AddBuffEffect(lpObj, BUFFTYPE_INFINITY_ARROW, 0, 0, 0, 0, -10);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, 1);
	g_Log.Add("[%s][%s] Use Infinity Arrow Skill (Time:%d)(Character Level : %d)(ChangeUp: %d)", lpObj->AccountID, lpObj->Name, Time, lpObj->Level, lpObj->CharacterData->ChangeUP);
	return true;
}
int CObjUseSkill::ELF_InfinityArrowManaUsage(LPOBJ lpObj, CMagicInf* lpMagic) {
	int iRetValue = 0;

	if (lpObj->Class == CLASS_ELF && lpObj->Type == OBJ_USER && lpObj->CharacterData->ChangeUP != FALSE && (BUFF_TargetHasBuff(lpObj, BUFFTYPE_INFINITY_ARROW) == TRUE || BUFF_TargetHasBuff(lpObj, BUFFTYPE_ETERNAL_ARROW) == TRUE)) {
		if (lpMagic->m_Skill == WEAPON_SKILL_CROSSBOW || lpMagic->m_Skill == WEAPON_SKILL_BOW || lpMagic->m_Skill == ELF_ICEARROW || lpMagic->m_Skill == ELF_PENETRATION || lpMagic->m_Skill == WEAPON_SKILL_CROSSBOW2 || g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(lpMagic->m_Skill) == 414 || lpMagic->m_Skill == 416 || lpMagic->m_Skill == 418 || lpMagic->m_Skill == 424) {
			int iArrowLevel = 0;
			CItem* Right = &lpObj->pInventory[0];
			CItem* Left = &lpObj->pInventory[1];

			if (Right->m_Type >= ITEMGET(4, 8) && Right->m_Type <= ITEMGET(4, 14) || Right->m_Type == ITEMGET(4, 16) || Right->m_Type == ITEMGET(4, 18) || Right->m_Type == ITEMGET(4, 19) || Right->m_Type == ITEMGET(4, 26)) {
				if (Left->m_Type == ITEMGET(4, 7)) iArrowLevel = Left->m_Level;
			} else if (Left->m_Type >= ITEMGET(4, 0) && Left->m_Type <= ITEMGET(4, 6) || Left->m_Type == ITEMGET(4, 17) || Left->m_Type == ITEMGET(4, 20) || Left->m_Type == ITEMGET(4, 21) || Left->m_Type == ITEMGET(4, 22) || Left->m_Type == ITEMGET(4, 23) || Left->m_Type == ITEMGET(4, 24) || Left->m_Type == ITEMGET(4, 25)) {
				if (Right->m_Type == ITEMGET(4, 15)) iArrowLevel = Right->m_Level;
			}
			if (iArrowLevel == 0) iRetValue = g_SkillAdditionInfo.GetInfinityArrowMPConsumptionPlus0();
			else if (iArrowLevel == 1) iRetValue = g_SkillAdditionInfo.GetInfinityArrowMPConsumptionPlus1();
			else if (iArrowLevel == 2) iRetValue = g_SkillAdditionInfo.GetInfinityArrowMPConsumptionPlus2();
			else if (iArrowLevel == 3) iRetValue = g_SkillAdditionInfo.GetInfinityArrowMPConsumptionPlus3();
			if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 441) == 20) iRetValue /= 2;
		}
	}

	return iRetValue;
}
int CObjUseSkill::ELF_FiveShot(int aIndex, CMagicInf* lpMagic, BYTE TargetPos, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];

	this->SkillFrustrum3(aIndex, TargetPos, 6.0f, 7.0f, 1.0f, 0); //S4 New Skill Frustrum OK :)

	vec3_t Angle; //Good
	Vector(0.f, 6.0f, 0.f, Angle); //3,4,5 Good
	vec3_t p[4]; //Good size OK
	Vector(0.f, 0.f, 40.0f, p[0]); //Good
	Vector(0.f, 0.f, 20.0f, p[1]); //Good
	Vector(0.f, 0.f, 340.0f, p[2]); //Good
	Vector(0.f, 0.f, 320.0f, p[3]); //Good
	float Matrix[3][4]; //Good
	vec3_t vFrustrum[5]; //Good

	AngleMatrix(p[0], Matrix); //Good
	VectorRotate(Angle, Matrix, vFrustrum[0]); //loc44 Good	
	AngleMatrix(p[1], Matrix); //Good
	VectorRotate(Angle, Matrix, vFrustrum[1]); //loc41 Good

	Vector(0.f, 6.0f, 0.f, vFrustrum[2]);//36,37,38

	AngleMatrix(p[2], Matrix); //Good
	VectorRotate(Angle, Matrix, vFrustrum[3]); //loc35 Good
	AngleMatrix(p[3], Matrix); //Good
	VectorRotate(Angle, Matrix, vFrustrum[4]); //loc32 Good

	Vector(0.f, 0.f, (TargetPos * 360 / (BYTE)255), p[0]); //15, 16, 17 Good

	AngleMatrix((float*)p, Matrix); //Good

	int Value1[5];
	int Value2[5];
	vec3_t vFrustrum2[5];

	for (int i = 0; i < 5; i++) { //loc70 Good
		VectorRotate(vFrustrum[i], Matrix, vFrustrum2[i]);
		Value1[i] = (int)vFrustrum2[i][0] + lpObj->X;
		Value2[i] = (int)vFrustrum2[i][1] + lpObj->Y;
	}

	int tObjNum; //loc71
	int count = 0; //loc72
	int loc73 = 0; //loc73
	int DuelIndex = lpObj->m_iDuelUser; //loc74
	BOOL bEnableAttack; //loc75

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				bEnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) bEnableAttack = TRUE;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) bEnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) bEnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) bEnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) bEnableAttack = TRUE;
				}

				if (bEnableAttack != FALSE) {
					int Value3[5];

					for (int j = 0; j < 5; j++) {
						Value3[j] = (((int)Value1[j] - lpObj->X) * (gObj[tObjNum].Y - lpObj->Y)) - (((int)Value2[j] - lpObj->Y) * (gObj[tObjNum].X - lpObj->X));

						if (SkillTestFrustrum(gObj[tObjNum].X, gObj[tObjNum].Y, aIndex)) {
							if (Value3[j] > -5 && Value3[j] < 5) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 0, 0, 0, 0, 0);
						}
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	return TRUE;
}
int CObjUseSkill::ELF_ShieldRecovery(int aIndex, CMagicInf* lpMagic, int aTargetIndex) {
	int skillSuccess = true;
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	int skill_level = lpMagic->m_Level;

	if (lpTargetObj->Type != OBJ_USER && lpTargetObj->m_RecallMon == -1) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 0);
		return FALSE;
	}

	if (lpObj->Class != CLASS_ELF) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 0);
		return FALSE;
	}

	if ((CC_MAP_RANGE(lpObj->MapNumber) || lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) && lpObj->m_Index != lpTargetObj->m_Index) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 0);
		return FALSE;
	}

	if (gObjDuelCheck(lpTargetObj) != 0) lpTargetObj = lpObj;
	if (lpTargetObj->iShield < lpTargetObj->iAddShield + lpTargetObj->iMaxShield) {
		double skill_shield = 0.0;

		this->m_Lua.Generic_Call("ShieldRecovery", "ii>d", (int)(lpObj->CharacterData->Energy + lpObj->AddEnergy), (int)lpObj->Level, &skill_shield);
		lpTargetObj->iShield += skill_shield;

		if (lpTargetObj->iShield > lpTargetObj->iAddShield + lpTargetObj->iMaxShield) lpTargetObj->iShield = lpTargetObj->iAddShield + lpTargetObj->iMaxShield;
		if (lpTargetObj->Type == OBJ_USER) {
			GSProtocol.PROTOCOL_ReFillSend(lpTargetObj->m_Index, lpTargetObj->Life, 0xFF, 0, lpTargetObj->iShield);
			GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, ELF_SHIELDRECOVERY, lpTargetObj->m_Index, 1);

			for (int n = 0; n < MAX_VIEWPORT; n++) {
				if (lpObj->VpPlayer2[n].type == OBJ_MONSTER && lpObj->VpPlayer2[n].state != 0) {
					LPOBJ lpMonster = &gObj[lpObj->VpPlayer2[n].number];

					if (lpMonster->m_iCurrentAI != 0) lpMonster->m_Agro->IncAgro(lpObj->m_Index, lpObj->CharacterData->Energy / 5 / 40);
				}
			}
		}
	} else GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, ELF_SHIELDRECOVERY, lpTargetObj->m_Index, 0);
	return TRUE;
}

// Summoner
int CObjUseSkill::SUM_SpiritOfFlames(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex) {
	int count = 0;
	int tObjNum;
	int EnableAttack;
	int DuelIndex = gObj[aIndex].m_iDuelUser;
	LPOBJ lpObj = &gObj[aIndex];

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, SUM_SPIRIT_OF_FLAMES, aTargetIndex, 1);

	int Distance = MagicDamageC.GetSkillDistance(lpMagic->m_Skill);

	if (this->SKILL_CalcDistance(lpObj->X, lpObj->Y, lpObj->X, lpObj->Y) <= Distance) {
		while (true) {
			if (lpObj->VpPlayer2[count].state != 0) {
				tObjNum = lpObj->VpPlayer2[count].number;

				if (tObjNum >= 0) {
					EnableAttack = FALSE;

					if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
					else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
					else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && gObj[tObjNum].Type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
					else {
						int CallMonIndex = gObj[tObjNum].m_Index;

						if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
						if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
					}

					if (EnableAttack) {
						if (gObj[tObjNum].X >= x - 2) {
							if (gObj[tObjNum].X <= x + 2) {
								if (gObj[tObjNum].Y >= y - 2) {
									if (gObj[tObjNum].Y <= y + 2) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 1000, lpMagic->m_Skill, 0);
								}
							}
						}
					}
				}
			}

			count++;

			if (count > MAX_VIEWPORT - 1) break;
		}
	}

	return TRUE;
}
int CObjUseSkill::SUM_SpiritOfAnguish(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex) {
	if (!ObjectMaxRange(aTargetIndex)) return TRUE;

	int count = 0;
	int tObjNum;
	int EnableAttack;
	int DuelIndex = gObj[aIndex].m_iDuelUser;
	LPOBJ lpObj = &gObj[aIndex];

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, SUM_SPIRIT_OF_ANGUISH, aTargetIndex, 1);

	int Distance = MagicDamageC.GetSkillDistance(lpMagic->m_Skill);

	if (this->SKILL_CalcDistance(lpObj->X, lpObj->Y, lpObj->X, lpObj->Y) <= Distance) {
		while (true) {
			if (lpObj->VpPlayer2[count].state != 0) {
				tObjNum = lpObj->VpPlayer2[count].number;

				if (tObjNum >= 0) {
					EnableAttack = FALSE;

					if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
					else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
					else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && gObj[tObjNum].Type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
					else {
						int CallMonIndex = gObj[tObjNum].m_Index;

						if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
						if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
					}

					if (EnableAttack) {
						if (gObj[tObjNum].X >= x - 2) {
							if (gObj[tObjNum].X <= x + 2) {
								if (gObj[tObjNum].Y >= y - 2) {
									if (gObj[tObjNum].Y <= y + 2) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 1000, lpMagic->m_Skill, 0);
								}
							}
						}
					}
				}
			}

			count++;

			if (count > MAX_VIEWPORT - 1) break;
		}
	}

	return TRUE;
}
void CObjUseSkill::SUM_SpiritOfTerror(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex, int aMsgSubCode, BOOL bCombo) {
	if (aMsgSubCode == 59) GSProtocol.PROTOCOL_MagicAttackNumberSend(&gObj[aIndex], lpMagic->m_Skill, aTargetIndex, 1);
	if (lpMagic->m_Skill == SUM_SPIRIT_OF_TERROR) this->SKILL_AreaAttack(aIndex, lpMagic, x, y, aTargetIndex, 3, 1, 0);
	if (aMsgSubCode >= 59 && aMsgSubCode <= 64) gObjAddMsgSendDelay(&gObj[aIndex], 59, aTargetIndex, 750, (MAKELONG(MAKEWORD(x, y), aMsgSubCode)));
}
int CObjUseSkill::SUM_DrainLife(int aIndex, CMagicInf* lpMagic, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 1);
	gObjAddAttackProcMsgSendDelay(lpObj, 50, aTargetIndex, 700, lpMagic->m_Skill, 0);
	return true;
}
int CObjUseSkill::SUM_ChainLighting(int aIndex, CMagicInf* lpMagic, int aTargetIndex) {
	if (!ObjectMaxRange(aIndex)) return FALSE;
	if (!ObjectMaxRange(aTargetIndex)) return FALSE;

	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	BOOL EnableAttack = FALSE;
	int count = 0;
	int DuelIndex = gObj[aIndex].m_iDuelUser;
	int nChainTarget[3] = {-1, -1, -1};

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 1);

	if (!CHARACTER_CheckSkillDistance(aIndex, aTargetIndex, lpMagic->m_Skill)) return FALSE;

	nChainTarget[0] = aTargetIndex;

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			int tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && gObj[tObjNum].Type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack == TRUE) {
					if (gObj[tObjNum].X >= gObj[aTargetIndex].X - 1 && gObj[tObjNum].X <= gObj[aTargetIndex].X + 1 && gObj[tObjNum].Y >= gObj[aTargetIndex].Y - 1 && gObj[tObjNum].Y <= gObj[aTargetIndex].Y + 1) {
						nChainTarget[1] = tObjNum;

						if (nChainTarget[2] != -1) break;
					} else if (gObj[tObjNum].X >= gObj[aTargetIndex].X - 2 && gObj[tObjNum].X <= gObj[aTargetIndex].X + 2 && gObj[tObjNum].Y >= gObj[aTargetIndex].Y - 2 && gObj[tObjNum].Y <= gObj[aTargetIndex].Y + 2) {
						nChainTarget[2] = tObjNum;

						if (nChainTarget[1] != -1) break;
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	if (nChainTarget[2] == -1) nChainTarget[2] = nChainTarget[1];

	if (nChainTarget[1] == -1) {
		nChainTarget[1] = nChainTarget[0];
		nChainTarget[2] = nChainTarget[0];
	}

	PMSG_CHAIN_MAGIC pMsg;

	pMsg.MagicNumberH = HIBYTE(lpMagic->m_Skill);
	pMsg.MagicNumberL = LOBYTE(lpMagic->m_Skill);
	pMsg.wUserIndex = aIndex;
	pMsg.nCount = 3;

	char SendByte[256];
	int nOffset = sizeof(PMSG_CHAIN_MAGIC);
	PMSG_CHAIN_MAGIC_OBJECT pMagicObj;

	for (int i = 0; i < 3; i++) {
		pMagicObj.nTargetIndex = nChainTarget[i];
		memcpy(&SendByte[nOffset], &pMagicObj, sizeof(PMSG_CHAIN_MAGIC_OBJECT));
		nOffset += sizeof(PMSG_CHAIN_MAGIC_OBJECT);
	}

	PHeadSubSetB((LPBYTE)&pMsg, 0xBF, 0x0A, nOffset);
	memcpy(&SendByte, &pMsg, sizeof(pMsg));

	if (lpObj->Type == OBJ_USER) IOCP.DataSend(aIndex, (LPBYTE)SendByte, nOffset);

	GSProtocol.MsgSendV2(lpObj, (LPBYTE)SendByte, nOffset);

	for (int i = 0; i < 3; i++) gObjAddAttackProcMsgSendDelay(lpObj, 50, nChainTarget[i], 200 * i + 200, lpMagic->m_Skill, i + 1);
	return TRUE;
}
int CObjUseSkill::SUM_Sleep(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	int result = 0;
	int tObjNum;
	int angle = this->GetAngle(lpObj->X, lpObj->Y, gObj[aTargetIndex].X, gObj[aTargetIndex].Y);
	int count = 0;
	int DuelIndex;
	BOOL bEnableAttack;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 1);
	DuelIndex = lpObj->m_iDuelUser;

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				bEnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) bEnableAttack = TRUE;
				else if (tObjNum == aTargetIndex && DuelIndex == tObjNum) bEnableAttack = TRUE;
				else if (CC_MAP_RANGE(lpObj->MapNumber) != FALSE || IT_MAP_RANGE(lpObj->MapNumber) != FALSE || lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) bEnableAttack = TRUE;
				else if (g_ArcaBattle.IsArcaBattleServer() == TRUE) bEnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) bEnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) bEnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) bEnableAttack = TRUE;
				}

				if (bEnableAttack != FALSE) {
					if (tObjNum == aTargetIndex) {
						if (CHARACTER_CheckSkillDistance(aIndex, aTargetIndex, lpMagic->m_Skill)) {
							gObjAttack(lpObj, &gObj[aTargetIndex], lpMagic, 1, 0, 0, 0, 0, 0);
							result = 1;
							break;
						}
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	return result;
}
int CObjUseSkill::SUM_MagicMirror(int aIndex, CMagicInf* lpMagic, int aTargetIndex) {
	if (!ObjectMaxRange(aIndex)) return FALSE;

	LPOBJ lpObj = &gObj[aIndex];
	int Index = aIndex;

	if (ObjectMaxRange(aTargetIndex)) Index = aTargetIndex;

	int RefDmg = 0;
	int Time = 0;

	if (gObj[aIndex].CharacterData->ISBOT == true) Time = g_BotSystem.GetSkillTime(aIndex, lpMagic->m_Skill);

	this->m_Lua.Generic_Call("MagicMirror", "i>ii", (lpObj->CharacterData->Energy + lpObj->AddEnergy), &RefDmg, &Time);
	BUFF_AddBuffEffect(&gObj[Index], BUFFTYPE_DAMAGE_REFLECT, EFFECTTYPE_INCREASE_DAMAGE_REFLECT, RefDmg, 0, 0, Time);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(&gObj[aIndex], lpMagic->m_Skill, Index, 1);
	return TRUE;
}
int CObjUseSkill::SUM_Enfeeble(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int EnableAttack;
	int count = 0;
	int tObjNum;
	int attacked = 0;
	int DuelIndex = lpObj->m_iDuelUser;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 1);

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			EnableAttack = FALSE;
			tObjNum = lpObj->VpPlayer2[count].number;

			if (lpObj->VpPlayer2[count].type != OBJ_MONSTER || gObj[tObjNum].m_RecallMon > 0) {
				if (tObjNum == DuelIndex) EnableAttack = TRUE;
				else if (CC_MAP_RANGE(lpObj->MapNumber) || lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) EnableAttack = TRUE;
				else if (IT_MAP_RANGE(lpObj->MapNumber)) EnableAttack = TRUE;
				else if (g_ArcaBattle.IsArcaBattleServer() == TRUE) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && gObj[tObjNum].Type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) {
					if (g_CastleSiege.m_bCastleSiegeFriendlyFire == false && lpObj->m_btCsJoinSide == gObj[tObjNum].m_btCsJoinSide) EnableAttack = FALSE;
					else EnableAttack = TRUE;
				}
			} else if (gObj[tObjNum].Type == OBJ_MONSTER) {
				if (gObj[tObjNum].m_RecallMon >= 0) {
					if (gObjTargetGuildWarCheck(lpObj, &gObj[tObjNum])) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				} else EnableAttack = TRUE;
			}

			if (EnableAttack) {
				if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) <= 6) {
					gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 700, lpMagic->m_Skill, 0);
					++attacked;

					if (attacked >= 5) break;
				}
			}
		}

		++count;

		if (count >= MAX_VIEWPORT - 1)	break;
	}

	return TRUE;
}
int CObjUseSkill::SUM_Weakness(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int EnableAttack;
	int count = 0;
	int tObjNum;
	int attacked = 0;
	int DuelIndex = lpObj->m_iDuelUser;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 1);

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			EnableAttack = FALSE;
			tObjNum = lpObj->VpPlayer2[count].number;

			if (lpObj->VpPlayer2[count].type != OBJ_MONSTER || gObj[tObjNum].m_RecallMon > 0) {
				if (tObjNum == DuelIndex) EnableAttack = TRUE;
				else if (CC_MAP_RANGE(lpObj->MapNumber) || lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) EnableAttack = TRUE;
				else if (IT_MAP_RANGE(lpObj->MapNumber)) EnableAttack = TRUE;
				else if (g_ArcaBattle.IsArcaBattleServer() == TRUE) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && gObj[tObjNum].Type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) {
					if (g_CastleSiege.m_bCastleSiegeFriendlyFire == false && lpObj->m_btCsJoinSide == gObj[tObjNum].m_btCsJoinSide) EnableAttack = FALSE;
					else EnableAttack = TRUE;
				}
			} else if (gObj[tObjNum].Type == OBJ_MONSTER) {
				if (gObj[tObjNum].m_RecallMon >= 0) {
					if (gObjTargetGuildWarCheck(lpObj, &gObj[tObjNum])) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				} else EnableAttack = TRUE;
			}

			if (EnableAttack) {
				if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) <= 6) {
					gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 700, lpMagic->m_Skill, 0);
					++attacked;

					if (attacked >= 5) break;
				}
			}
		}

		++count;

		if (count >= MAX_VIEWPORT - 1)	break;
	}

	return TRUE;
}
int CObjUseSkill::SUM_AmplifyDamage(int aIndex, CMagicInf* lpMagic, int aTargetIndex) {
	if (!ObjectMaxRange(aIndex)) return FALSE;

	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = lpObj;

	if (lpObj->Type == OBJ_USER && lpObj->Class != CLASS_SUMMONER && lpObj->CharacterData->ISBOT == false) return FALSE;
	if (lpObj->Type == OBJ_USER && lpObj->CharacterData->ISBOT == true && lpObj->BuffPlayerIndex != -1) lpTargetObj = &gObj[lpObj->BuffPlayerIndex];

	int EffectUPValue = 0;
	int EffectDownValue = 0;
	int Time = 0;

	if (lpObj->Type == OBJ_USER) this->m_Lua.Generic_Call("AmplifyDamage", "i>iii", (lpObj->CharacterData->Energy + lpObj->AddEnergy), &EffectUPValue, &EffectDownValue, &Time);
	if (lpObj->Type == OBJ_USER && lpObj->CharacterData->ISBOT == true) Time = g_BotSystem.GetSkillTime(aIndex, lpMagic->m_Skill);
	if (lpObj->Type != OBJ_USER) {
		Time = 600000;
		EffectUPValue = 0;
		EffectDownValue = 0;
	}

	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_AMPLIFY_DAMAGE, EFFECTTYPE_INCREASE_AMPLIFY_DAMAGE, EffectUPValue, EFFECTTYPE_DECREASE_AMPLIFY_DAMAGE, EffectDownValue, Time);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpTargetObj->m_Index, 1);

	return TRUE;
}
void CObjUseSkill::SUM_LightningBlast(int aIndex, CMagicInf* lpMagic, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	BOOL EnableAttack;

	for (int n = 0; n < MAX_VIEWPORT; n++) {
		EnableAttack = FALSE;

		if (lpObj->VpPlayer2[n].state <= 0) continue;

		int index = lpObj->VpPlayer2[n].number;

		if (!ObjectMaxRange(index)) continue;
		if (gObj[index].Type == OBJ_MONSTER && gObj[index].m_RecallMon < 0) EnableAttack = TRUE;
		if (gObj[aIndex].Type == OBJ_MONSTER && gObj[index].Type == OBJ_USER) EnableAttack = TRUE; // Foreign Dimension Fix
		if (aTargetIndex == index || index == lpObj->m_iDuelUser) EnableAttack = TRUE;
		if (CC_MAP_RANGE(lpObj->MapNumber) || IT_MAP_RANGE(lpObj->MapNumber) || lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) EnableAttack = TRUE;
		else if (g_ArcaBattle.IsArcaBattleServer() == TRUE) EnableAttack = TRUE;
		else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE) {
			if (gObj[index].Type != OBJ_NPC && gObj[index].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
		}

		int summon = aIndex;

		if (gObj[aIndex].Type == OBJ_MONSTER && gObj[aIndex].m_RecallMon >= 0) summon = gObj[aIndex].m_RecallMon;
		if (gObjTargetGuildWarCheck(&gObj[summon], lpObj) != 0) EnableAttack = TRUE;
		if (gObj[index].Class >= 678 && gObj[index].Class <= 680) EnableAttack = TRUE;
		if (CHARACTER_CalcDistance(lpObj, &gObj[index]) <= 6 && EnableAttack == TRUE) gObjAddAttackProcMsgSendDelay(lpObj, 50, index, 300, lpMagic->m_Skill, 0);
	}
}

// Magic Gladiator
BOOL CObjUseSkill::MG_SpiralSlash(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return false;

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->MapNumber != MAP_INDEX_CASTLESIEGE && lpObj->MapNumber != MAP_INDEX_CASTLEHUNTZONE && this->m_SkillData.EnableSiegeOnAllMaps == false) return false;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 1);
	gObjAttack(lpObj, &gObj[aTargetIndex], lpMagic, 0, 1, 0, 0, 0, 0);
	gObjAddAttackProcMsgSendDelay(lpObj, 50, aTargetIndex, 100, lpMagic->m_Skill, 0);
	return true;
}
BOOL CObjUseSkill::MG_ManaRays(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return false;

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->MapNumber != MAP_INDEX_CASTLESIEGE && lpObj->MapNumber != MAP_INDEX_CASTLEHUNTZONE && this->m_SkillData.EnableSiegeOnAllMaps == false) return false;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 1);
	gObjAttack(lpObj, &gObj[aTargetIndex], lpMagic, 0, 1, 0, 0, 0, 0);
	return true;
}
void CObjUseSkill::MG_PowerSlash(int aIndex, CMagicInf * lpMagic, BYTE x, BYTE y, BYTE dir, BYTE targetangle, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int StartDis = 1;
	int tObjNum;

	::SkillFrustrum(targetangle,aIndex);

	int count = 0;
	int HitCount = 0;
	int bAttack;
	int DuelIndex = lpObj->m_iDuelUser;
	int EnableAttack;

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj,&gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack != 0) {
					if (SkillTestFrustrum(gObj[tObjNum].X,gObj[tObjNum].Y,aIndex)) {
						bAttack = 0;

						if (HitCount > 10) bAttack = 0;
						if (HitCount >= 5) {
							if (rand() % 2) bAttack = 1;
						} else bAttack = 1;
						if (bAttack != 0) gObjAttack(lpObj,&gObj[tObjNum],lpMagic,1,1,0,0,0,0);

						HitCount++;
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT -1) break;
	}
}
void CObjUseSkill::MG_FireSlash(int aIndex, int aTargetIndex, int skill_level) {
	int SuccessRate = 50;
	int Value = 15;
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpObj->Type == OBJ_USER && lpObj->Class != CLASS_MAGICGLADIATOR) return;
	if (!ObjectMaxRange(aTargetIndex)) return;
	if (CHARACTER_DebuffProc(SuccessRate) == TRUE) {
		BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_FIRE_SLASH, EFFECTTYPE_DECREASE_DEFENSE, Value, 0, 0, 10);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, MG_FIRESLASH, lpTargetObj->m_Index, TRUE);
	}
}
int CObjUseSkill::MG_FlameStrike(int aIndex, CMagicInf* lpMagic, BYTE TargetPos, int aTargetIndex) {
	int count = 0;
	int tObjNum;
	int EnableAttack = FALSE;
	int HitCount = 0;

	this->SkillFrustrum3(aIndex, TargetPos, 2.0, 4.0, 5.0, 0.0);

	LPOBJ lpObj = &gObj[aIndex];
	int DuelIndex = lpObj->m_iDuelUser;

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;
			EnableAttack = FALSE;

			if (tObjNum >= 0) {
				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;

				int CallMonIndex = gObj[tObjNum].m_Index;

				if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
				if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
				if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
			}

			if (EnableAttack) {
				if (SkillTestFrustrum(gObj[tObjNum].X, gObj[tObjNum].Y, aIndex) == TRUE && CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) <= MagicDamageC.GetSkillDistance(lpMagic->m_Skill)) {
					int Attack = 0;

					if (HitCount < 8) Attack = 1;
					else Attack = rand() % 2;
					if (Attack) {
						gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 0, 0, 0, 0, 0);
						gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 0, 0, 0, 0, 0);
					}
				}
			}
		}

		++count;

		if (count > MAX_VIEWPORT - 1) break;
	}

	return TRUE;
}
int CObjUseSkill::MG_ThunderStorm(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex) {
	return this->SKILL_AreaAttack(aIndex, lpMagic, x, y, aTargetIndex, 7, 1, 400);
}

// Dark Lord
BOOL CObjUseSkill::DL_PillarsOfFire(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return false;

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->MapNumber != MAP_INDEX_CASTLESIEGE && lpObj->MapNumber != MAP_INDEX_CASTLEHUNTZONE && this->m_SkillData.EnableSiegeOnAllMaps == false) return false;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 1);
	gObjAddAttackProcMsgSendDelay(lpObj, 50, aTargetIndex, 500, lpMagic->m_Skill, 0);
	return true;
}
BOOL CObjUseSkill::DL_Force(int aIndex, int aTargetIndex, CMagicInf * lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	int StartDis = 1;
	int tObjNum;
	int iangle = this->GetAngle(lpObj->X,lpObj->Y,gObj[aTargetIndex].X,gObj[aTargetIndex].Y);
	int count = 0;
	int loopcount = 0;
	int attackcheck;
	int EnableAttack;
	int DuelIndex = lpObj->m_iDuelUser;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj,lpMagic->m_Skill,aTargetIndex,1);

	if (lpObj->SkillLongSpearChange == 0) {
		gObjAttack(lpObj,&gObj[aTargetIndex],lpMagic,0,1,0,0,0,0);
		return true;
	}

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj,&gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack != 0) {
					attackcheck = 0;

					if (loopcount == 0) attackcheck = 1;
					else if (rand() % 3 == 0) attackcheck = 1;
					if (attackcheck != 0) {
						if (SkillSpearHitBox.HitCheck(iangle,lpObj->X,lpObj->Y,gObj[tObjNum].X,gObj[tObjNum].Y)) gObjAttack(lpObj,&gObj[tObjNum],lpMagic,0,1,0,0,0,0);
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT -1) break;
	}

	return true;
}
BOOL CObjUseSkill::DL_FireBurst(int aIndex, int aTargetIndex, CMagicInf * lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	int StartDis = 1;
	int tObjNum;
	int count = 0;
	int loopcount = 0;
	int attackcheck;
	int delaytime;
	int EnableAttack;
	int DuelIndex = lpObj->m_iDuelUser;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, DL_FIREBURST,aTargetIndex,1);
	gObjAttack(lpObj,&gObj[aTargetIndex],lpMagic,0,1,0,0,0,0);

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0 && aTargetIndex != tObjNum) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj,&gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack != 0) {
					attackcheck = 0;

					if (loopcount == 0) attackcheck = 1;
					else if (rand() % 3 == 0) attackcheck = 1;
					if (attackcheck != 0) {
						if (CHARACTER_CalcDistance(&gObj[aTargetIndex],&gObj[tObjNum]) < 3) {
							delaytime = (rand() * 17) % 300 + 500;
							gObjAddAttackProcMsgSendDelay(lpObj,50,tObjNum,delaytime,lpMagic->m_Skill,0);
						}
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT -1) break;
	}

	return true;
}
void CObjUseSkill::DL_Concentration(int aIndex, int skill_level, CMagicInf* lpMagic) {
	int skillSuccess = true;
	LPOBJ lpObj = &gObj[aIndex];
	int number;

	if (lpObj->Type != OBJ_USER && lpObj->m_RecallMon == -1) return;
	if (lpObj->Class != CLASS_DARKLORD && lpObj->CharacterData->ISBOT == false) return;

	int partynum = 0;
	int totallevel = 0;
	int partycount;
	int dis;
	int ApplyPartyIndex[MAX_USER_IN_PARTY];
	LPOBJ lpPartyObj;

	memset(ApplyPartyIndex, -1, sizeof(ApplyPartyIndex));
	partynum = lpObj->PartyNumber;

	if (partynum != -1) {
		partycount = gParty.m_PartyS[partynum].Count;

		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			number = gParty.m_PartyS[partynum].Number[n];

			if (number >= 0) {
				lpPartyObj = &gObj[number];

				if (lpObj->MapNumber == lpPartyObj->MapNumber) {
					dis = CHARACTER_CalcDistance(lpObj, &gObj[number]);
					if (dis < 10) ApplyPartyIndex[n] = lpPartyObj->m_Index;
				}
			}
		}
	}

	int addcriticaldamagevalue = 0;
	int SkillTime = 0;

	this->m_Lua.Generic_Call("Concentration", "ii>ii", (lpObj->Leadership + lpObj->AddLeadership), (lpObj->CharacterData->Energy + lpObj->AddEnergy), &addcriticaldamagevalue, &SkillTime);

	if (partynum == -1 && lpObj->CharacterData->ISBOT == false) {
		BUFF_AddBuffEffect(lpObj, BUFFTYPE_CONCENTRATION, EFFECTTYPE_INCREASE_CRITICAL_DAMAGE, addcriticaldamagevalue, 0, 0, SkillTime);
		MsgOutput(aIndex, Lang.GetText(0, 134), SkillTime);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, skillSuccess);
	} else if (lpObj->CharacterData->ISBOT == true) {
		SkillTime = g_BotSystem.GetSkillTime(lpObj->m_Index, lpMagic->m_Skill);
		BUFF_AddBuffEffect(&gObj[lpObj->BuffPlayerIndex], BUFFTYPE_CONCENTRATION, EFFECTTYPE_INCREASE_CRITICAL_DAMAGE, addcriticaldamagevalue, 0, 0, SkillTime);
		MsgOutput(lpObj->BuffPlayerIndex, Lang.GetText(0, 134), SkillTime);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, skillSuccess);
	} else {
		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			if (ApplyPartyIndex[n] != -1) {
				lpPartyObj = &gObj[ApplyPartyIndex[n]];
				BUFF_AddBuffEffect(lpPartyObj, BUFFTYPE_CONCENTRATION, EFFECTTYPE_INCREASE_CRITICAL_DAMAGE, addcriticaldamagevalue, 0, 0, SkillTime);
				MsgOutput(ApplyPartyIndex[n], Lang.GetText(0, 134), SkillTime);
				GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpPartyObj->m_Index, skillSuccess);
			}
		}
	}
}
BOOL CObjUseSkill::DL_FireScream(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (g_SkillAdditionInfo.GetFireScreamSkill() == false) return false;

	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (gObjIsConnected(lpObj->m_Index) == false && gObjIsConnected(lpTargetObj->m_Index) == false) return false;
	if (lpObj->Type != OBJ_USER) return false;
	if (lpObj->Class != CLASS_DARKLORD) return false;

	gObjAttack(lpObj, lpTargetObj, lpMagic, 0, 0, 0, 0, 0, 0);
	return true;
}
BOOL CObjUseSkill::DL_FireScreamExplosion(LPOBJ lpObj, LPOBJ lpTargetObj, int iAttackDamage, int iAddDamage) {
	if (gObjIsConnected(lpObj->m_Index) == false && gObjIsConnected(lpTargetObj->m_Index) == false) return false;
	if (rand() % 10000 > g_SkillAdditionInfo.GetFireScreamExplosionRate()) return false;

	int iExplosionDamage = iAttackDamage / 10;
	int iTarObjNum;

	if (iExplosionDamage < 1) return true;
	if (iAddDamage > 0) iExplosionDamage += iAddDamage;

	int iCount = 0;
	int bEnableAttack;

	while (true) {
		if (lpObj->VpPlayer2[iCount].state != 0) {
			iTarObjNum = lpObj->VpPlayer2[iCount].number;

			if (iTarObjNum >= 0) {
				bEnableAttack = FALSE;

				if (gObj[iTarObjNum].Type == OBJ_MONSTER && gObj[iTarObjNum].m_RecallMon < 0) bEnableAttack = TRUE;
				else if (lpTargetObj->Type == OBJ_USER && lpObj->Type == OBJ_USER && gObj[iTarObjNum].Type != OBJ_NPC) bEnableAttack = TRUE;
				else if (lpTargetObj->m_cChaosCastleIndex == gObj[iTarObjNum].m_cChaosCastleIndex && lpTargetObj->m_cChaosCastleIndex != -1) bEnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[iCount].type != OBJ_NPC && gObj[iTarObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) {
					if (lpObj->m_btCsJoinSide != gObj[iTarObjNum].m_btCsJoinSide && gObj[iTarObjNum].m_btCsJoinSide > 0) bEnableAttack = TRUE;
				}

				if (gObj[iTarObjNum].Class >= 678 && gObj[iTarObjNum].Class <= 680) bEnableAttack = TRUE;
				if (bEnableAttack != 0) {
					if (lpObj->m_Index != gObj[iTarObjNum].m_Index) {
						int iSkillDistance = CHARACTER_CalcDistance(lpTargetObj, &gObj[iTarObjNum]);

						if (iSkillDistance <= g_SkillAdditionInfo.GetFireScreamExplosionAttackDistance()) gObjAddMsgSendDelay(lpObj, 54, iTarObjNum, 100, iExplosionDamage);
					}
				}
			}
		}

		iCount++;

		if (iCount > MAX_VIEWPORT - 1) break;
	}

	return true;
}
void CObjUseSkill::ML_BurningEchoes(LPOBJ lpObj, LPOBJ lpTargetObj, CMagicInf* lpMagic, int iAttackDamage) {
	WORD bySkillNum = lpMagic->m_Skill;

	if (bySkillNum == DL_FIRESCREAM || bySkillNum == 518 || bySkillNum == 520) {
		int iAddDamage = 0;

		if (lpMagic->m_Skill == 520) iAddDamage = g_MasterLevelSkillTreeSystem.ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);

		gObjUseSkill.DL_FireScreamExplosion(lpObj, lpTargetObj, iAttackDamage, iAddDamage);
	}
}
void CObjUseSkill::DL_Spark(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, BYTE dir, BYTE TargetPos, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int TargePosx = lpObj->X - (8 - (TargetPos & 15));
	int TargePosy = lpObj->Y - (8 - ((TargetPos & 240) >> 4));
	int tObjNum;
	int count = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int iangle = this->GetAngle(lpObj->X, lpObj->Y, TargePosx, TargePosy);
	int delaytime;
	int Hit = 0;
	int EnableAttack;

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack != 0) {
					if (SkillElectricSparkHitBox.HitCheck(iangle, gObj[aIndex].X, gObj[aIndex].Y, gObj[tObjNum].X, gObj[tObjNum].Y)) {
						delaytime = 500;
						Hit = 1;
						gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, delaytime, lpMagic->m_Skill, 0);
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	if (Hit != 0) DL_SparkDrain(lpObj);
}
int DL_SparkDrain(LPOBJ lpObj) {
	LPOBJ lpPartyObj;
	int partynum = lpObj->PartyNumber;

	if (ObjectMaxRange(partynum) == false) return false;

	int partycount = gParty.m_PartyS[partynum].Count;
	int retcount = 0;

	for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
		int number = gParty.m_PartyS[partynum].Number[n];

		if (number >= 0) {
			lpPartyObj = &gObj[number];

			if (lpObj->MapNumber == lpPartyObj->MapNumber && lpObj->m_Index != number) {
				int dis = CHARACTER_CalcDistance(lpObj, &gObj[number]);

				if (dis < 10) {
					lpPartyObj = &gObj[number];

					int declife = lpPartyObj->Life * 20.0f / 100.0f;
					int decmana = lpPartyObj->Mana * 5.0f / 100.0f;

					lpPartyObj->Life = lpPartyObj->Life - declife;
					lpPartyObj->Mana = lpPartyObj->Mana - decmana;
					GSProtocol.PROTOCOL_ReFillSend(lpPartyObj->m_Index, lpPartyObj->Life, 0xFF, 0, lpPartyObj->iShield);
					GSProtocol.PROTOCOL_ManaSend(lpPartyObj->m_Index, lpPartyObj->Mana, 0xFF, 0, lpPartyObj->Stamina);
					retcount++;
				}
			}
		}
	}

	return retcount;
}
void CObjUseSkill::DL_Summon(int aIndex, int skill_level) {
	int skillSuccess = true;
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJ_USER && lpObj->m_RecallMon == -1) return;
	if (this->DL_SummonCheck(aIndex, skill_level) == 0) {
		GSProtocol.GCServerMsgStringSend(Lang.GetText(0, 132), aIndex, 1);
		return;
	}

	if (gMoveCommand.CheckMainToMove(lpObj) == 0) {
		GSProtocol.GCServerMsgStringSend(Lang.GetText(0, 141), lpObj->m_Index, 1);
		return;
	}

	if (gMoveCommand.CheckInterfaceToMove(lpObj) == 0) {
		GSProtocol.GCServerMsgStringSend(Lang.GetText(0, 133), lpObj->m_Index, 1);
		return;
	}

	if (lpObj->MapNumber == MAP_INDEX_HATCHERY && lpObj->Connected == PLAYER_PLAYING &&
		(g_Raklion.GetRaklionState() == RAKLION_STATE_CLOSE_DOOR || g_Raklion.GetRaklionState() == RAKLION_STATE_ALL_USER_DIE || g_Raklion.GetRaklionState() == RAKLION_STATE_NOTIFY_4 || g_Raklion.GetRaklionState() == RAKLION_STATE_END)) {
		GSProtocol.GCServerMsgStringSend(Lang.GetText(0, 299), aIndex, 1);
		return;
	}

	if (g_GensSystem.IsMapBattleZone(gObj[aIndex].MapNumber) == TRUE) {
		GSProtocol.GCServerMsgStringSend(Lang.GetText(0, 533), aIndex, 1);
		return;
	}

	if (IMPERIAL_MAP_RANGE(gObj[aIndex].MapNumber) == TRUE) {
		GSProtocol.GCServerMsgStringSend(Lang.GetText(0, 147), aIndex, 1);
		return;
	}

	int number;
	int partynum;
	int partycount;
	LPOBJ lpPartyObj;
	int recallcount;

	partynum = 0;
	partynum = lpObj->PartyNumber;
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, DL_SUMMON, lpObj->m_Index, skillSuccess);
	recallcount = 0;

	if (partynum != -1) {
		partycount = gParty.m_PartyS[partynum].Count;

		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			number = gParty.m_PartyS[partynum].Number[n];

			if (number >= 0) {
				lpPartyObj = &gObj[number];

				if (g_NewPVP.IsDuel(gObj[number])) {
					GSProtocol.GCServerMsgStringSendEx(aIndex, 1, Lang.GetText(0, 322), lpPartyObj->Name);
					continue;
				}

				if (lpPartyObj->m_Index != lpObj->m_Index) {
					if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) {
						if (lpPartyObj->m_btCsJoinSide != lpObj->m_btCsJoinSide) {
							GSProtocol.GCServerMsgStringSendEx(aIndex, 1, Lang.GetText(0, 511), lpPartyObj->Name);
							continue;
						}
					}

					int movelevel = gMoveCommand.GetMoveLevel(lpObj->MapNumber, lpObj->X, lpObj->Y, lpPartyObj->Class);
					int bCheckMainToMove = gMoveCommand.CheckMainToMove(lpPartyObj);
					int bCheckEquipmentToMove = gMoveCommand.CheckEquipmentToMove(lpPartyObj, lpObj->MapNumber);
					int bCheckInterfaceToMove = gMoveCommand.CheckInterfaceToMove(lpPartyObj);
					bool bCheckPk = g_ConfigRead.pk.CanSummonByDarkLord(lpPartyObj->m_PK_Level);
					int iVipLevel = g_MapAttr.CheckVIP(lpObj->MapNumber);

					if (lpPartyObj->Level >= movelevel && movelevel != -1 && bCheckMainToMove != false && bCheckEquipmentToMove != false && bCheckInterfaceToMove != false && bCheckPk != false && lpPartyObj->CharacterData->VipType >= iVipLevel) {
						int addx;
						int addy;
						int count = 50;
						int Find = 0;

						while (count--) {
							addx = lpObj->X + rand() % 9 - 4;
							addy = lpObj->Y + rand() % 9 - 4;

							if (gObjCheckTeleportArea(lpObj->m_Index, addx, addy)) {
								Find = 1;
								break;
							}
						}

						if (Find == 0) {
							addx = lpObj->X;
							addy = lpObj->Y;
						}

						lpPartyObj->m_SkillInfo.LordSummonTime = 7;
						lpPartyObj->m_SkillInfo.LordSummonMapNumber = lpObj->MapNumber;
						lpPartyObj->m_SkillInfo.LordSummonX = addx;
						lpPartyObj->m_SkillInfo.LordSummonY = addy;

						if (lpObj->MapNumber == MAP_INDEX_HATCHERY) g_RaklionBattleUserMng.AddUserData(lpPartyObj->m_Index);
						if (lpPartyObj->CharacterData->m_bIsMining) g_MineSystem.CheckMoveMapWhileMining(lpPartyObj->m_Index);

						GSProtocol.GCServerMsgStringSend(Lang.GetText(0, 143), lpPartyObj->m_Index, 1);
						recallcount++;
					}
					else GSProtocol.GCServerMsgStringSend(Lang.GetText(0, 144), lpPartyObj->m_Index, 1);
				}
			}
		}
	}

	if (recallcount != 0) {
		char msg[255];
		wsprintf(msg, Lang.GetText(0, 136), recallcount);
		GSProtocol.GCServerMsgStringSend(msg, lpObj->m_Index, 1);
	}
}
BOOL CObjUseSkill::DL_SummonCheck(int aIndex, int skill_level) {
	LPOBJ lpObj = &gObj[aIndex];
	int number;
	int partynum = 0;

	partynum = lpObj->PartyNumber;

	int partycount;
	LPOBJ lpPartyObj;
	int recallcount = 0;

	if (partynum != -1) {
		partycount = gParty.m_PartyS[partynum].Count;

		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			number = gParty.m_PartyS[partynum].Number[n];

			if (number >= 0) {
				lpPartyObj = &gObj[number];

				if (lpPartyObj->m_Index != lpObj->m_Index) {
					if (lpPartyObj->MapNumber == lpObj->MapNumber) {
						if (lpPartyObj->m_SkillInfo.LordSummonTime != 0) return false;
					}
				}
			}
		}
	}

	return true;
}
int CObjUseSkill::DL_Darkness(int aIndex, CMagicInf* lpMagic, BYTE TargetPos, int aTargetIndex) {
	int count = 0;
	int tObjNum;
	int EnableAttack = FALSE;
	int HitCount = 0;

	this->SkillFrustrum3(aIndex, TargetPos, 1.5, 6.0, 1.5, 0.0);

	LPOBJ lpObj = &gObj[aIndex];
	int DuelIndex = lpObj->m_iDuelUser;

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;
			EnableAttack = FALSE;
			if (tObjNum >= 0) {
				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
				else if (lpObj->VpPlayer2[count].type == OBJ_USER && lpObj->Class == 561) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}
			}

			if (EnableAttack) {
				if (SkillTestFrustrum(gObj[tObjNum].X, gObj[tObjNum].Y, aIndex) == TRUE) {
					int Attack = 0;

					if (HitCount < 8) Attack = 1;
					else Attack = rand() % 2;
					if (Attack) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 200, lpMagic->m_Skill, 0);
					if (ObjectMaxRange(aTargetIndex)) {
						if (tObjNum == aTargetIndex) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 300, lpMagic->m_Skill, 0);
					}
				}
			}
		}

		++count;

		if (count > MAX_VIEWPORT - 1) break;
	}

	return TRUE;
}
BOOL CObjUseSkill::DL_Earthquake(int aIndex, int aTargetIndex, CMagicInf * lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	int tObjNum;
	int count = 0;
	int HitCount = 0;
	int bAttack;
	int DuelIndex = lpObj->m_iDuelUser;
	int EnableAttack;

	if (lpObj->Type == OBJ_USER) {
		if (CHECK_DarkHorse(lpObj) == FALSE) return false;
	}

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj,lpMagic->m_Skill,lpObj->m_Index,1);

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if ((gObj[tObjNum].Class < 100 || gObj[tObjNum].Class > 110) && gObj[tObjNum].Class != 689) {
					if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
					else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
					else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) {
						if (g_CastleSiege.m_bCastleSiegeFriendlyFire == false && lpObj->m_btCsJoinSide == gObj[tObjNum].m_btCsJoinSide) EnableAttack = FALSE;
						else EnableAttack = TRUE;
					} else {
						int CallMonIndex = gObj[tObjNum].m_Index;
	
						if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
						if (gObjTargetGuildWarCheck(lpObj,&gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
						if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
					}
	
					if (EnableAttack != 0) {
						if (CHARACTER_CalcDistance(lpObj,&gObj[tObjNum]) < 5) {
							bAttack = 0;
	
							if (HitCount > 15) bAttack = 0;
							if (HitCount >= 10) {
								if (rand() % 2) bAttack = 1;
							} else bAttack = 1;
							if (bAttack != 0) gObjAddAttackProcMsgSendDelay(lpObj,50,tObjNum,500,lpMagic->m_Skill,0);
						}
					}
				}
			}
		}
	
		count++;

		if (count > MAX_VIEWPORT -1) break;		
	}

	return true;
}

// Rage Fighter
int CObjUseSkill::RF_Charge(int aIndex, CMagicInf* lpMagic, int aTargetIndex) {
	if (ObjectMaxRange(aIndex) == FALSE) return FALSE;

	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpObj->MapNumber == MAP_INDEX_CASTLESIEGE || lpObj->MapNumber == MAP_INDEX_CASTLEHUNTZONE || this->m_SkillData.EnableSiegeOnAllMaps == true) {
		if (lpObj->MapNumber == lpTargetObj->MapNumber) {
			GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
			gObjAttack(lpObj, lpTargetObj, lpMagic, 0, 1, 0, 0, 0, 0);
			return TRUE;
		}
	}

	return FALSE;
}
int CObjUseSkill::RF_WeaponSkill(int aIndex, CMagicInf* lpMagic, int aTargetIndex) {
	if (ObjectMaxRange(aTargetIndex) == FALSE) {
		g_Log.Add("[InvalidTargetIndex][SkillMonkBarrageJustOneTarget] Index :%d , AccountID : %s", aIndex, gObj[aIndex].AccountID);
		return FALSE;
	}

	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	BOOL EnableAttack = FALSE;
	int count = 0;
	int tObjNum;
	int HitCount = 0;
	int nDistance = MagicDamageC.GetSkillDistance(lpMagic->m_Skill);

	while (true) {
		if (lpObj->VpPlayer2[count].state == TRUE) {
			tObjNum = lpObj->VpPlayer2[count].number;
			if (tObjNum >= 0) {
				if (tObjNum == aTargetIndex) break;
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) return FALSE;
	}

	if (lpObj->VpPlayer2[count].type == OBJ_USER) {
		if (g_GensSystem.IsMapBattleZone(lpObj->MapNumber) == TRUE) EnableAttack = TRUE;
		else if (gclassObjAttack.PkCheck(lpObj, lpTargetObj) == TRUE) EnableAttack = TRUE;
		else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpTargetObj->MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
	} else if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) {
		int CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;

		if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
		if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
	} else if (lpTargetObj->Type == OBJ_MONSTER || gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
	else if (tObjNum == lpObj->m_iDuelUser) EnableAttack = TRUE;

	if (EnableAttack == TRUE && this->SKILL_CalcDistance(gObj[aTargetIndex].X, gObj[aTargetIndex].Y, gObj[tObjNum].X, gObj[tObjNum].Y) < nDistance) {
		switch (lpMagic->m_Skill) {
			case RF_FISTFLURRY:			HitCount = 4;		break;
			case RF_BEASTUPPERCUT:		HitCount = 2;		break;
			case RF_CHAINDRIVE:			HitCount = 8;		break;
			case RF_DARKSIDE:			HitCount = 3;		break;
			default:					HitCount = 0;		break;
		}

		for (int i = 1; i <= HitCount; i++) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 1, 0, 0, i, 0);
	}
}
int CObjUseSkill::RF_PhoenixShot(int aIndex, CMagicInf * lpMagic, BYTE x, BYTE y, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aIndex];
	BOOL EnableAttack;
	int attackcheck;
	int loopcount = 0;
	int delaytime = 0;

    for (int n = 0; n < MAX_VIEWPORT; n++) {
        if (lpObj->VpPlayer2[n].state <= 0) continue;

		EnableAttack = FALSE;

		int index = lpObj->VpPlayer2[n].number;

		if (gObj[index].Type == OBJ_MONSTER && gObj[index].m_RecallMon < 0) EnableAttack = TRUE;
		if (aTargetIndex == index || index == lpObj->m_iDuelUser) EnableAttack = TRUE;
		else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && gObj[index].Type != OBJ_NPC && gObj[index].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;

		int summon = aIndex;

		if (gObj[aIndex].Type == OBJ_MONSTER && gObj[aIndex].m_RecallMon >= 0) summon = gObj[aIndex].m_RecallMon;
		if (gObjTargetGuildWarCheck(&gObj[summon],lpObj) != FALSE) EnableAttack = TRUE;
		if (EnableAttack != FALSE) {
			attackcheck = FALSE;

			if (loopcount == 0) attackcheck = TRUE;
			else if (rand() % 3 == 0) attackcheck = TRUE;
			if (attackcheck != FALSE) {
				if (CHARACTER_CalcDistance(&gObj[aTargetIndex],&gObj[index]) < 3) {
					delaytime = (rand() * 17) % 300 + 300;
					gObjAddAttackProcMsgSendDelay(lpObj,53,index,delaytime,lpMagic->m_Skill,0);
					gObjAddAttackProcMsgSendDelay(lpObj,50,index,delaytime,lpMagic->m_Skill,0);
					gObjAddAttackProcMsgSendDelay(lpObj,50,index,delaytime + 100,lpMagic->m_Skill,0);
					gObjAddAttackProcMsgSendDelay(lpObj,50,index,delaytime + 100,lpMagic->m_Skill,0);
				}
			}
		}
	}

	return TRUE;
}
void CObjUseSkill::RF_DarkSide(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	int iTargetCount = 0;
	WORD wTargetList[5];

	for (int i = 0; i < 5; i++) wTargetList[i] = 10000;
	if (gObj[aIndex].CloseCount < 0 && lpMagic != NULL && this->RF_DarkSideGetTargetIndex(aIndex, aTargetIndex, lpMagic, wTargetList) == TRUE) {
		PMSG_RAGE_ATTACK_RANGE_ANS pMsg;

		PHeadSetBE((LPBYTE)&pMsg, 0x4B, sizeof(pMsg));
		memcpy(&pMsg.Target, wTargetList, sizeof(wTargetList));
		pMsg.MagicNumber = lpMagic->m_Skill;;
		gObj[aIndex].CharacterData->DarkSideCount = 0;

		for (int i = 0; i < 5; i++) {
			if (wTargetList[i] != 10000) {
				gObj[aIndex].CharacterData->DarkSideCount++;
				gObjAddAttackProcMsgSendDelay(&gObj[aIndex], 61, aIndex, 150 + (iTargetCount * 150), lpMagic->m_Skill, wTargetList[i]);
				iTargetCount++;
			}
		}

		memcpy(gObj[aIndex].CharacterData->DarkSideTarget, wTargetList, sizeof(wTargetList));
		IOCP.DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
	}
}
int CObjUseSkill::RF_DarkSideGetTargetIndex(int aIndex, int aTargetIndex, CMagicInf* lpMagic, WORD* Target) {
	LPOBJ lpObj = &gObj[aIndex];
	int count = 0;
	int HitCount = 0;
	int DuelUser = lpObj->m_iDuelUser;
	int nDistance = MagicDamageC.GetSkillDistance(lpMagic->m_Skill);

	if (lpMagic->m_Skill == 563) {
		int iPoint = g_MasterLevelSkillTreeSystem.ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);

		nDistance += iPoint / 10;
	}

	if (Target != NULL) {
		if (lpMagic->m_Skill == 263 || lpMagic->m_Skill == 559 || lpMagic->m_Skill == 563) {
			if (aTargetIndex) {
				while (true) {
					if (lpObj->VpPlayer2[count].state == 1) {
						int tObjNum = lpObj->VpPlayer2[count].number;

						if (tObjNum >= 0) {
							if (tObjNum != 10000 && lpObj->VpPlayer2[count].type == OBJ_USER && tObjNum == aTargetIndex) {
								if (this->SKILL_CalcDistance(lpObj->X, lpObj->Y, gObj[tObjNum].X, gObj[tObjNum].Y) < nDistance) {
									*Target = tObjNum;
									HitCount = 1;
									break;
								}
							}
						}
					}

					count++;

					if (count > MAX_VIEWPORT - 1) break;
				}
			}

			count = 0;

			while (true) {
				if (lpObj->VpPlayer2[count].state == 1) {
					int tObjNum = lpObj->VpPlayer2[count].number;

					if (tObjNum >= 0) {
						LPOBJ lpTargetObj = &gObj[tObjNum];
						BOOL EnableAttack = FALSE;
						int CallMonIndex = -1;

						if (lpTargetObj->Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
						else if (tObjNum == lpObj->m_iDuelUser) EnableAttack = TRUE;
						else if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) {
							CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;

							if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
							if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
						}

						if (EnableAttack == TRUE && gObjAttackQ(lpTargetObj) == FALSE) EnableAttack = FALSE;
						if (EnableAttack == TRUE) {
							if (this->SKILL_CalcDistance(lpObj->X, lpObj->Y, lpTargetObj->X, lpTargetObj->Y) < nDistance) {
								Target[HitCount] = tObjNum;
								HitCount++;

								if (HitCount >= 5) break;
							}
						}
					}
				}

				count++;

				if (count > MAX_VIEWPORT - 1) break;
			}
		}
	}

	return TRUE;
}
int CObjUseSkill::RF_BuffParty(int aIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == FALSE) return FALSE;

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJ_USER && lpObj->m_RecallMon == -1) return FALSE;
	if (lpObj->Class != CLASS_RAGEFIGHTER) return FALSE;

	int partynum = 0;
	int partyindex[5];

	memset(partyindex, -1, sizeof(partyindex));
	partynum = lpObj->PartyNumber;

	if (partynum != -1) {
		for (int i = 0; i < MAX_USER_IN_PARTY; i++) {
			int index = gParty.m_PartyS[partynum].Number[i];

			if (index >= 0 && index < g_ConfigRead.server.GetObjectMax()) {
				LPOBJ lpPartyObj = &gObj[index];

				if (lpPartyObj->MapNumber == lpObj->MapNumber) {
					if (CHARACTER_CalcDistance(lpObj, lpPartyObj) < 4) partyindex[i] = index;
				}
			}
		}
	}

	int skill_improve = 0;
	BYTE BuffIndex = BUFFTYPE_NONE;

	switch (lpMagic->m_Skill) {
		case RF_TOUGHNESS:
			BuffIndex = BUFFTYPE_TOUGHNESS;
		break;

		case RF_LIGHTNINGREFLEXES:
			BuffIndex = BUFFTYPE_LIGHTNING_REFLEXES;
		break;

		default:
		return FALSE;
	}

	if (lpMagic->m_Skill == RF_TOUGHNESS) skill_improve = (((lpObj->CharacterData->Energy + lpObj->AddEnergy) - 132) / 10.0 + 30.0);
	else if (lpMagic->m_Skill == RF_LIGHTNINGREFLEXES) {
		skill_improve = (((lpObj->CharacterData->Energy + lpObj->AddEnergy) - 80) / 10.0 + 10.0);

		if (skill_improve > 100) skill_improve = 100;
	} else return FALSE;

	if (partynum == -1) {
		if (lpMagic->m_Skill == RF_TOUGHNESS) {
			BUFF_AddBuffEffect(lpObj, BuffIndex, EFFECTTYPE_INCREASE_VITALITY_RF, skill_improve, 0, 0, lpObj->CharacterData->Energy / 5 + 60);
			GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aIndex, TRUE);
			gObjCalCharacter.CHARACTER_Calc(aIndex);
		} else if (lpMagic->m_Skill == RF_LIGHTNINGREFLEXES) {
			BUFF_AddBuffEffect(lpObj, BuffIndex, NULL, skill_improve, 0, 0, lpObj->CharacterData->Energy / 5 + 60);
			GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aIndex, TRUE);
		}
	} else {
		for (int j = 0; j < 5; j++) {
			if (partyindex[j] != -1) {
				LPOBJ lpPartyObj = &gObj[partyindex[j]];

				if (lpMagic->m_Skill == RF_TOUGHNESS) {
					BUFF_AddBuffEffect(lpPartyObj, BuffIndex, EFFECTTYPE_INCREASE_VITALITY_RF, skill_improve, 0, 0, lpObj->CharacterData->Energy / 5 + 60);
					GSProtocol.PROTOCOL_MagicAttackNumberSend(lpPartyObj, lpMagic->m_Skill, partyindex[j], TRUE);
					gObjCalCharacter.CHARACTER_Calc(partyindex[j]);
				} else {
					BUFF_AddBuffEffect(lpPartyObj, BuffIndex, NULL, skill_improve, 0, 0, lpObj->CharacterData->Energy / 5 + 60);
					GSProtocol.PROTOCOL_MagicAttackNumberSend(lpPartyObj, lpMagic->m_Skill, partyindex[j], TRUE);
				}
			}
		}
	}

	return TRUE;
}
int CObjUseSkill::RF_Focus(int aIndex, CMagicInf * lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];

	if (gObj[aIndex].Type != OBJ_USER) return FALSE;
	if (lpObj->Class != CLASS_RAGEFIGHTER) return FALSE;
	if (lpMagic == NULL) return FALSE;
	if (lpMagic->m_Skill != RF_FOCUS) return FALSE;

	int Ignore = (lpObj->CharacterData->Energy + lpObj->AddEnergy) / 400.0 + 3.0;

	if (Ignore > 15) Ignore = 15;

	BUFF_AddBuffEffect(lpObj, BUFFTYPE_FOCUS, EFFECTTYPE_IGNORE_OPPONENT_DEFENSE, Ignore, 0, 0, lpObj->CharacterData->Energy / 5 + 60);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, TRUE);
	return TRUE;
}

// LANCER (UNUSED AT THE MOMENT)
void CObjUseSkill::SkillSpinStep(int aIndex, int iTargetIndex, CMagicInf * lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[iTargetIndex];

	if (gObjIsConnected(lpObj->m_Index) == FALSE && gObjIsConnected(lpTargetObj->m_Index) == FALSE) return;
	if (lpObj->Type != OBJ_USER || lpObj->Class != CLASS_GROWLANCER) return;

	bool EnableAttack = false;
	int count = 0;
	int tObjNum;
	int iDuelIndex = lpObj->m_iDuelUser;

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (tObjNum == iTargetIndex || tObjNum == iDuelIndex) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack != 0) {
					if (CHARACTER_CalcDistance(lpTargetObj, &gObj[tObjNum]) <= MagicDamageC.GetSkillDistance(lpMagic->m_Skill)) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 0, 0, 0, 0, 0);
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}
void CObjUseSkill::SkillGrowLancerAttackOneTarget(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];

	if (ObjectMaxRange(aTargetIndex) == FALSE) {
		g_Log.Add("[InvalidTargetIndex][SkillGrowLancerAttackOneTarget] Index :%d , AccountID : %s", aIndex, gObj[aIndex].AccountID);
		return;
	}

	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	BOOL EnableAttack = FALSE;
	int count = 0;
	int tObjNum;
	int CallMonIndex;
	int HitCount = 0;
	int nDistance = MagicDamageC.GetSkillDistance(lpMagic->m_Skill);

	while (true) {
		if (lpObj->VpPlayer2[count].state == TRUE) {
			tObjNum = lpObj->VpPlayer2[count].number;
			if (tObjNum >= 0) {
				if (tObjNum == aTargetIndex) break;
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) return;
	}

	if (lpObj->VpPlayer2[count].type == OBJ_USER) {
		if (g_GensSystem.IsMapBattleZone(lpObj->MapNumber) == TRUE) EnableAttack = TRUE;
		if (gclassObjAttack.PkCheck(lpObj, lpTargetObj) == TRUE) EnableAttack = TRUE;
	} else if (lpTargetObj->Type == OBJ_MONSTER || gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
	else if (tObjNum == lpObj->m_iDuelUser) EnableAttack = TRUE;
	else if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) {
		CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;

		if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
		if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
	}

	int nHitCount = 0;

	if (lpMagic->m_Skill == 276) { // Harsh Strike
		nHitCount = 2; // 2 attacks
	} else if (lpMagic->m_Skill == 274) { // Magic Pin
		nHitCount = 3; // 3 attacks
	}

	if (EnableAttack == TRUE && this->SKILL_CalcDistance(gObj[aTargetIndex].X, gObj[aTargetIndex].Y, gObj[tObjNum].X, gObj[tObjNum].Y) < nDistance) {
		for (int i = 1; i < nHitCount; i++)	{
			if ((i % (nHitCount - 1)) == 1)	gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 1, 0, 0, i, 0);
			else gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 0, 1, 0, 0, i, 0);
		}

		if (lpMagic->m_Skill == 274) {
			CMagicInf * MagicPinExp = &DefMagicInf[AT_SKILL_MAGIC_PIN_EXPLOSION];
			gObjAttack(lpObj, &gObj[tObjNum], MagicPinExp, 1, 0, 0, 0, 0, 0);
		}
	}
}
int CObjUseSkill::SkillBreche(int aIndex, CMagicInf * lpMagic, BYTE x, BYTE y, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int tObjNum;
	int count = 0;
	int HitCount = 0;
	int bAttack;
	int DuelIndex = lpObj->m_iDuelUser;
	int EnableAttack;
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0)	{
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack != 0) {
					if (this->SKILL_CalcDistance(x, y, gObj[tObjNum].X, gObj[tObjNum].Y) < MagicDamageC.GetSkillDistance(lpMagic->m_Skill)) {
						bAttack = 0;

						if (HitCount > 4) bAttack = 0;
						else {
							bAttack = 1;
							HitCount++;
						}

						if (bAttack != 0) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 1, 0, 0, 0, 0);
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	return true;
}
int CObjUseSkill::SkillShiningPeak(int aIndex, CMagicInf * lpMagic, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];

	if (ObjectMaxRange(aTargetIndex) == FALSE) {
		g_Log.Add("[InvalidTargetIndex][SkillShiningPeak] Index :%d , AccountID : %s", aIndex, gObj[aIndex].AccountID);
		return FALSE;
	}

	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	BOOL EnableAttack = FALSE;
	int count = 0;
	int tObjNum;
	int CallMonIndex;
	int HitCount = 0;
	int nDistance = MagicDamageC.GetSkillDistance(lpMagic->m_Skill);

	while (true) {
		if (lpObj->VpPlayer2[count].state == TRUE) {
			tObjNum = lpObj->VpPlayer2[count].number;
			if (tObjNum >= 0) {
				if (tObjNum == aTargetIndex) break;
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) return FALSE;
	}

	if (lpObj->VpPlayer2[count].type == OBJ_USER) {
		if (g_GensSystem.IsMapBattleZone(lpObj->MapNumber) == TRUE) EnableAttack = TRUE;
		if (gclassObjAttack.PkCheck(lpObj, lpTargetObj) == TRUE) EnableAttack = TRUE;
	} else if (lpTargetObj->Type == OBJ_MONSTER || gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
	else if (tObjNum == lpObj->m_iDuelUser) EnableAttack = TRUE;
	else if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) {
		CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;

		if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = TRUE;
		if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
	}

	int nHitCount = 4;
	BYTE byPentagramAttack = TRUE;

	if (EnableAttack == TRUE && this->SKILL_CalcDistance(gObj[aTargetIndex].X, gObj[aTargetIndex].Y, gObj[tObjNum].X, gObj[tObjNum].Y) < nDistance) {
		int nHitCount = 4;

		for (int i = 1; i < nHitCount; i++, byPentagramAttack = FALSE) {
			if ((i % (nHitCount - 1)) == 1)	gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 1, 0, 0, i, 0, byPentagramAttack);
			else gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 0, 1, 0, 0, i, 0, byPentagramAttack);
		}
	}
}
int CObjUseSkill::SkillBuffGrowLancer(int aIndex, CMagicInf * lpMagic, int aTargetIndex) {
	if (ObjectMaxRange(aIndex) == FALSE) return FALSE;

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJ_USER && lpObj->m_RecallMon == -1) return FALSE;
	if (lpObj->Class != CLASS_GROWLANCER) return FALSE;

	int skill_improve = 0;
	int skill_improve2 = 0;
	int skill_time = 0;
	BYTE BuffIndex = BUFFTYPE_NONE;

	if (lpMagic->m_Skill == AT_SKILL_WRATH) {
		BuffIndex = BUFFTYPE_WRATH;
		this->m_Lua.Generic_Call("GrowLancerWrath", "iii>iii", lpObj->CharacterData->Strength + lpObj->AddStrength, lpObj->CharacterData->Dexterity + lpObj->AddDexterity, lpObj->CharacterData->Energy + lpObj->AddEnergy,	&skill_improve, &skill_improve2, &skill_time);
		BUFF_AddBuffEffect(lpObj, BuffIndex, EFFECTTYPE_WRATH_INC_DAMAGE, skill_improve, EFFECTTYPE_DECREASE_DEFENSE, skill_improve2, skill_time);
	} else if (lpMagic->m_Skill == AT_SKILL_CIRCLE_SHIELD) {
		BuffIndex = BUFFTYPE_CIRCLE_SHIELD;
		this->m_Lua.Generic_Call("GrowLancerCircleShield", "iii>iii", lpObj->CharacterData->Strength + lpObj->AddStrength, lpObj->CharacterData->Dexterity + lpObj->AddDexterity, lpObj->CharacterData->Energy + lpObj->AddEnergy,	&skill_improve, &skill_improve2, &skill_time);
		lpObj->m_SkillInfo.fCircleShieldRate = skill_improve2;
		BUFF_AddBuffEffect(lpObj, BuffIndex, EFFECTTYPE_AG_OPPONENT_DECREASE, skill_improve, 0, 0, skill_time);
	} else return FALSE;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aIndex, TRUE);
	return TRUE;
}
int CObjUseSkill::SkillBuffPartyGrowLancer(int aIndex, CMagicInf * lpMagic, int aTargetIndex) {
	if (ObjectMaxRange(aIndex) == FALSE) return FALSE;

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJ_USER && lpObj->m_RecallMon == -1) return FALSE;
	if (lpObj->Class != CLASS_GROWLANCER) return FALSE;

	int partynum = 0;
	int partyindex[5];

	memset(partyindex, -1, sizeof(partyindex));

	partynum = lpObj->PartyNumber;

	if (partynum != -1) {
		for (int i = 0; i < MAX_USER_IN_PARTY; i++)	{
			int index = gParty.m_PartyS[partynum].Number[i];

			if (index >= 0 && index < g_ConfigRead.server.GetObjectMax()) {
				LPOBJ lpPartyObj = &gObj[index];

				if (lpPartyObj->MapNumber == lpObj->MapNumber) {
					if (CHARACTER_CalcDistance(lpObj, lpPartyObj) < 4) partyindex[i] = index;
				}
			}
		}
	}

	int skill_improve = 0;
	int skill_time = 0;

	BYTE BuffIndex = BUFFTYPE_OBSIDIAN;

	gObjUseSkill.m_Lua.Generic_Call("GrowLancerObsidian", "iii>ii", (lpObj->CharacterData->Strength + lpObj->AddStrength), (lpObj->CharacterData->Dexterity + lpObj->AddDexterity), (lpObj->CharacterData->Energy + lpObj->AddEnergy), &skill_improve, &skill_time);

	if (partynum == -1)	{
		BUFF_AddBuffEffect(lpObj, BuffIndex, EFFECTTYPE_INCREASE_DAMAGE, skill_improve, 0, 0, skill_time);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aIndex, TRUE);
	} else {
		for (int j = 0; j < 5; j++)	{
			if (partyindex[j] != -1) {
				LPOBJ lpPartyObj = &gObj[partyindex[j]];

				BUFF_AddBuffEffect(lpPartyObj, BuffIndex, EFFECTTYPE_INCREASE_DAMAGE, skill_improve, 0, 0, skill_time);
				GSProtocol.PROTOCOL_MagicAttackNumberSend(lpPartyObj, lpMagic->m_Skill, partyindex[j], TRUE);
			}
		}
	}

	return TRUE;
}