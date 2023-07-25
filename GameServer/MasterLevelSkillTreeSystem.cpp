#include "stdafx.h"
#include "MasterLevelSkillTreeSystem.h"
#include "MasterSkillSystem.h"
#include "TLog.h"
#include "GameMain.h"
#include "configread.h"
#include "ObjCalCharacter.h"
#include "BuffEffectSlot.h"
#include "BuffEffect.h"
#include "SkillAdditionInfo.h"
#include "ChaosCastle.h"
#include "ObjUseSkill.h"
#include "SkillHitBox.h"
#include "zzzmathlib.h"
#include "GensSystem.h"
#include "ObjAttack.h"
#include "ArcaBattle.h"
#include "winutil.h"

CMasterLevelSkillTreeSystem g_MasterLevelSkillTreeSystem;

MLST_Value_Table::MLST_Value_Table() {
	this->fValueTable = NULL;
}
MLST_Value_Table::~MLST_Value_Table() {
	if (this->fValueTable != NULL) {
		delete[] this->fValueTable;
		this->fValueTable = NULL;
	}
}
CMasterLevelSkillTreeSystem::CMasterLevelSkillTreeSystem(void):m_Lua(true) {
	this->m_MLSValueTable = NULL;
}
CMasterLevelSkillTreeSystem::~CMasterLevelSkillTreeSystem(void) {
	if (this->m_MLSValueTable != NULL) {
		delete [] this->m_MLSValueTable;
		this->m_MLSValueTable = NULL;
	}
}

// General
void CMasterLevelSkillTreeSystem::ML_Init() {
	memset(&this->m_MLSTable, 0x00, sizeof(this->m_MLSTable));

	if (this->m_MLSValueTable != NULL) {
		delete [] this->m_MLSValueTable;
		this->m_MLSValueTable = NULL;
	}

	this->m_MLSValueTable = new MLST_Value_Table[MAX_MASTER_SKILL_TYPE];

	for (int i = 0; i < MAX_MASTER_SKILL_TYPE; i++) this->m_MLSValueTable[i].Init(this->GetMaxMasterSkillLevel());

	lua_State* pLuaState = this->m_Lua.GetLua();

	if (pLuaState == NULL) {
		g_Log.Add("[MasterLevelSkillTreeSystem] - Error - [%s] [%d]", __FILE__, __LINE__);
		return;
	}

	this->m_Lua.DoFile(g_ConfigRead.GetPath("\\Scripts\\Skills\\MasterSkillPoint.lua"));
	this->m_Lua.DoFile(g_ConfigRead.GetPath("\\Scripts\\Skills\\MasterSkillCalc.lua"));
	this->m_Lua.Generic_Call("MLS_ValueInit", ">");
}
void CMasterLevelSkillTreeSystem::AddToValueTable(int iSkillValueID, const char * szFunctionName, int iSkillValueType) {
	if (iSkillValueID < 0 || iSkillValueID > MAX_MASTER_SKILL_TYPE) {
		g_Log.Add("[MasterLevelSkillTreeSystem] - Error - Wrong iSkillValueID [%d] - [%s] [%d]", iSkillValueID, __FILE__, __LINE__);
		return;
	}

	this->m_MLSValueTable[iSkillValueID].iValueType = iSkillValueType;
	this->m_MLSValueTable[iSkillValueID].fValueTable[0] = 0.0;
	double fValue = 0.0;

	for (int i = 1; i <= this->GetMaxMasterSkillLevel(); i++) {
		this->m_Lua.Generic_Call(szFunctionName, "i>d", i, &fValue);
		this->m_MLSValueTable[iSkillValueID].fValueTable[i] = this->RoundValue(fValue, 2);
	}
}
void CMasterLevelSkillTreeSystem::ML_Load(const char *lpszFileName) {
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(lpszFileName);

	if (res.status != pugi::status_ok) {
		g_Log.MsgBox("[MasterLevelSkillTreeSystem] - Can't Load %s (%s)", lpszFileName, res.description());
		return;
	}

	int iIndex = -1;
	int iTempIndex, iRank, iPos, iPoint, iMaxPoint, iParentSkill1, iParentSkill2, iSkillID;

	pugi::xml_node main = file.child("MasterSkillTree");

	this->m_wMaxMasterSkillLevel = main.attribute("MaxSkillLevel").as_int(20); // private custom !!!
	this->ML_Init();

	for (pugi::xml_node class_node = main.child("Class"); class_node; class_node = class_node.next_sibling()) {
		int iClass = class_node.attribute("ID").as_int();

		for (pugi::xml_node tree = class_node.child("Tree"); tree; tree = tree.next_sibling()) {
			int iTreeType = tree.attribute("Type").as_int();

			for (pugi::xml_node skill = tree.child("Skill"); skill; skill = skill.next_sibling()) {
				iIndex = skill.attribute("Index").as_int();
				iTempIndex = iIndex % 36 - 1;
				iRank = iTempIndex / 4;
				iPos = iTempIndex % 4;

				iPoint = skill.attribute("ReqMinPoint").as_int();
				iMaxPoint = skill.attribute("MaxPoint").as_int();
				iParentSkill1 = skill.attribute("ParentSkill1").as_int();
				iParentSkill2 = skill.attribute("ParentSkill2").as_int();
				iSkillID = skill.attribute("MagicNumber").as_int();

				for (int i = 0; i < MAX_TYPE_PLAYER; i++) {
					if (CHARACTER_CLASS_TABLE[i][0] & iClass) {
						this->m_MLSTable[CHARACTER_CLASS_TABLE[i][1]].iAbility[iTreeType].iElement[iRank][iPos].m_dwIndex = iIndex;
						this->m_MLSTable[CHARACTER_CLASS_TABLE[i][1]].iAbility[iTreeType].iElement[iRank][iPos].m_byConsumePoint = iPoint;
						this->m_MLSTable[CHARACTER_CLASS_TABLE[i][1]].iAbility[iTreeType].iElement[iRank][iPos].m_byMaxPoint = iMaxPoint;
						this->m_MLSTable[CHARACTER_CLASS_TABLE[i][1]].iAbility[iTreeType].iElement[iRank][iPos].m_dwParentSkill1 = iParentSkill1;
						this->m_MLSTable[CHARACTER_CLASS_TABLE[i][1]].iAbility[iTreeType].iElement[iRank][iPos].m_dwParentSkill2 = iParentSkill2;
						this->m_MLSTable[CHARACTER_CLASS_TABLE[i][1]].iAbility[iTreeType].iElement[iRank][iPos].m_dwSkillID = iSkillID;
					}
				}
			}
		}
	}

	g_Log.Add("[MasterLevelSkillTreeSystem] - %s file is Loaded", lpszFileName);
}
void CMasterLevelSkillTreeSystem::ML_InitPassiveSkillData(LPOBJ lpObj) {
	if (lpObj->Type != OBJ_USER) return;

	lpObj->CharacterData->Mastery.Clear();
}
void CMasterLevelSkillTreeSystem::ML_CalcPassiveSkillData(LPOBJ lpObj, int iMLSkill, int iSkillLevel) {
	if (lpObj->Type != OBJ_USER) return;
	if (lpObj->CharacterData->Strength < MagicDamageC.GetRequireStrength(iMLSkill)) return;
	if (lpObj->CharacterData->Dexterity < MagicDamageC.GetRequireDexterity(iMLSkill)) return;

	float iSkillValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(iMLSkill), iSkillLevel);

	switch (this->GetBaseMasterLevelSkill(iMLSkill)) {
		case 300:				lpObj->CharacterData->Mastery.ML_GENERAL_Efficiency = iSkillValue;
								lpObj->CharacterData->Mastery.ML_GENERAL_EfficiencyLevel = iSkillLevel;					break;
		case 301:				lpObj->CharacterData->Mastery.ML_GENERAL_GladiatorsResolve = iSkillValue;				break;
		case 302:				lpObj->CharacterData->Mastery.ML_GENERAL_GladiatorSpirit = iSkillValue;					break;
		case 303:				lpObj->CharacterData->Mastery.ML_GENERAL_MentalQuickness = iSkillValue;					break;
		case 304:				lpObj->CharacterData->Mastery.ML_GENERAL_Immunity = iSkillValue / 10;					break;
		case 305:				lpObj->CharacterData->Mastery.ML_GENERAL_Jeweller = iSkillValue;						break;
		case 306:				lpObj->CharacterData->Mastery.ML_GENERAL_GladiatorsVigor = iSkillValue;					break;
		case 307:				lpObj->CharacterData->Mastery.ML_GENERAL_Regeneration = iSkillValue;					break;
		case 308:				lpObj->CharacterData->Mastery.ML_GENERAL_Resistivity = iSkillValue / 10;				break;
		case 309:				lpObj->CharacterData->Mastery.ML_GENERAL_PhysicalResistance = iSkillValue;				break;
		case 310:				lpObj->CharacterData->Mastery.ML_GENERAL_Endurance = iSkillValue;						break;
		case 311:				lpObj->CharacterData->Mastery.ML_GENERAL_ColdAffinity = iSkillValue / 10;				break;
		case 312:				lpObj->CharacterData->Mastery.ML_GENERAL_Caretaker = iSkillValue;						break;
		case 313:				lpObj->CharacterData->Mastery.ML_GENERAL_SuddenMovements = iSkillValue;					break;
		case 314:				lpObj->CharacterData->Mastery.iMpsImmuneRate = iSkillValue;								break;
		case 315:				lpObj->CharacterData->Mastery.ML_GENERAL_Valor = iSkillValue;							break;
		case 316:				lpObj->CharacterData->Mastery.ML_GENERAL_Vengeance = iSkillValue;						break;
		case 317:				lpObj->CharacterData->Mastery.ML_GENERAL_Intelligence = iSkillValue;					break;
		case 318:				lpObj->CharacterData->Mastery.ML_GENERAL_Resilience = iSkillValue;						break;
		case 319:				lpObj->CharacterData->Mastery.ML_GENERAL_Quickness = iSkillValue;						break;
		case 320:				lpObj->CharacterData->Mastery.ML_GENERAL_Brawn = iSkillValue;							break;
		case 325:				lpObj->CharacterData->Mastery.ML_GENERAL_Accuracy = iSkillValue;						break;
		case 334:				lpObj->CharacterData->Mastery.ML_GENERAL_Vigor = iSkillValue;							break;
		case 338:				lpObj->CharacterData->Mastery.ML_GENERAL_RisingSpirit = iSkillValue;					break;
		case 341:				lpObj->CharacterData->Mastery.ML_GENERAL_Vitality = iSkillValue;						break;
		case 345:				lpObj->CharacterData->Mastery.ML_DK_SkilledFighter = iSkillValue;						break;
		case 347:				lpObj->CharacterData->Mastery.ML_GENERAL_Precision = iSkillValue;						break;
		case 348:				lpObj->CharacterData->Mastery.ML_DK_TwoHandedSwordMastery = iSkillValue;				break;
		case 349:				lpObj->CharacterData->Mastery.ML_DK_OneHandedSwordMastery = iSkillValue;				break;
		case 350:				lpObj->CharacterData->Mastery.ML_DK_MaceMastery = iSkillValue;							break;
		case 351:				lpObj->CharacterData->Mastery.ML_DK_SpearMastery = iSkillValue;							break;
		case 352:				lpObj->CharacterData->Mastery.ML_DK_TwoHandedSwordExpertise = iSkillValue;				break;
		case 353:				lpObj->CharacterData->Mastery.ML_DK_OneHandedSwordExpertise = iSkillValue;				break;
		case 354:				lpObj->CharacterData->Mastery.ML_DK_MaceExpertise = iSkillValue;						break;
		case 355:				lpObj->CharacterData->Mastery.ML_DK_SpearExpertise = iSkillValue;						break;
		case 357:				lpObj->CharacterData->Mastery.ML_GENERAL_ArcaneEfficiency = iSkillValue;				break;
		case 358:				lpObj->CharacterData->Mastery.ML_GENERAL_Leech = iSkillValue;							break;
		case 359:				lpObj->CharacterData->Mastery.ML_GENERAL_Slayer = iSkillValue;							break;
		case 361:				lpObj->CharacterData->Mastery.ML_DK_Stability = iSkillValue;							break;
		case 362:				lpObj->CharacterData->Mastery.ML_GENERAL_Vanquisher = iSkillValue;						break;
		case 364:				lpObj->CharacterData->Mastery.ML_DK_Recklessness = iSkillValue;							break;
		case 366:				lpObj->CharacterData->Mastery.ML_GENERAL_CriticalBlows = iSkillValue;					break;
		case 367:				lpObj->CharacterData->Mastery.ML_GENERAL_ArcaneRecovery = iSkillValue;					break;
		case 368:				lpObj->CharacterData->Mastery.ML_GENERAL_LastStand = iSkillValue;						break;
		case 369:				lpObj->CharacterData->Mastery.ML_GENERAL_ExcellentBlows = iSkillValue;					break;
		case 370:				lpObj->CharacterData->Mastery.ML_GENERAL_LethalBlows = iSkillValue;						break;
		case 371:				lpObj->CharacterData->Mastery.ML_GENERAL_FatalBlows = iSkillValue;						break;
		case 372:				lpObj->CharacterData->Mastery.ML_GENERAL_Overpower = iSkillValue;						break;
		case 373:				lpObj->CharacterData->Mastery.iMpsIncTripleDamageRate = iSkillValue;					break;
		case 397:				lpObj->CharacterData->Mastery.ML_DW_OneHandedStaffMastery = iSkillValue;				break;
		case 398:				lpObj->CharacterData->Mastery.ML_DW_TwoHandedStaffMastery = iSkillValue;				break;
		case 399:				lpObj->CharacterData->Mastery.ML_GENERAL_ShieldMastery = iSkillValue;					break;
		case 400:				lpObj->CharacterData->Mastery.ML_DW_OneHandedStaffExpertise = iSkillValue;				break;
		case 401:				lpObj->CharacterData->Mastery.ML_DW_TwoHandedStaffExpertise = iSkillValue;				break;
		case 402:				lpObj->CharacterData->Mastery.ML_GENERAL_ShieldExpertise = iSkillValue;					break;
		case 405:				lpObj->CharacterData->Mastery.ML_GENERAL_ArcaneSubtlety = iSkillValue;					break;
		case 407:				lpObj->CharacterData->Mastery.ML_GENERAL_ArcaneCombustion = iSkillValue;				break;
		case 415:				lpObj->CharacterData->Mastery.ML_ELF_MinionLife = iSkillValue;							break;
		case 419:				lpObj->CharacterData->Mastery.ML_ELF_MinionDefensive = iSkillValue;						break;
		case 421:				lpObj->CharacterData->Mastery.ML_ELF_WeaponMastery = iSkillValue;						break;
		case 428:				lpObj->CharacterData->Mastery.ML_ELF_MinionOffensive = iSkillValue;						break;
		case 435:				lpObj->CharacterData->Mastery.ML_ELF_BowMastery = iSkillValue;							break;
		case 436:				lpObj->CharacterData->Mastery.ML_ELF_CrossbowMastery = iSkillValue;						break;
		case 438:				lpObj->CharacterData->Mastery.ML_ELF_BowExpertise = iSkillValue;						break;
		case 439:				lpObj->CharacterData->Mastery.ML_ELF_CrossbowExpertise = iSkillValue;					break;
		case 442:				lpObj->CharacterData->Mastery.ML_ELF_Stability = iSkillValue;							break;
		case 443:				lpObj->CharacterData->Mastery.ML_ELF_Recklessness = iSkillValue;						break;
		case 448:				lpObj->CharacterData->Mastery.ML_SUM_ImprovedSahamutt = iSkillValue;					break;
		case 449:				lpObj->CharacterData->Mastery.ML_SUM_ImprovedNeil = iSkillValue;						break;
		case 450:				lpObj->CharacterData->Mastery.ML_SUM_ImprovedPhantasm = iSkillValue;					break;
		case 451:				lpObj->CharacterData->Mastery.ML_SUM_SpiritOfFlames = iSkillValue;						break;
		case 452:				lpObj->CharacterData->Mastery.ML_SUM_SpiritOfAnguish = iSkillValue;						break;
		case 453:				lpObj->CharacterData->Mastery.ML_SUM_SpiritOfTerror = iSkillValue;						break;
		case 457:				lpObj->CharacterData->Mastery.ML_SUM_SpellMastery = iSkillValue;						break;
		case 465:				lpObj->CharacterData->Mastery.ML_SUM_StaffMastery = iSkillValue;						break;
		case 466:				lpObj->CharacterData->Mastery.ML_SUM_CurseMastery = iSkillValue;						break;
		case 467:				lpObj->CharacterData->Mastery.ML_SUM_StaffExpertise = iSkillValue;						break;
		case 468:				lpObj->CharacterData->Mastery.ML_SUM_CurseExpertise = iSkillValue;						break;
		case 471:				lpObj->CharacterData->Mastery.ML_SUM_VoidReach = iSkillValue;							break;
		case 473:				lpObj->CharacterData->Mastery.ML_SUM_ArcaneFlows = iSkillValue;							break;
		case 506:				lpObj->CharacterData->Mastery.ML_DL_Leadership = iSkillValue;							break;
		case 526:				lpObj->CharacterData->Mastery.ML_DL_ImprovedDarkCrow = iSkillValue;						break;
		case 527:				lpObj->CharacterData->Mastery.ML_DL_ScepterMastery = iSkillValue;						break;
		case 528:				lpObj->CharacterData->Mastery.ML_DL_ShieldMastery = iSkillValue;						break;
		case 529:				lpObj->CharacterData->Mastery.ML_DL_Commander = iSkillValue;							break;
		case 530:				lpObj->CharacterData->Mastery.ML_DL_SharpTalons = iSkillValue;							break;
		case 531:				lpObj->CharacterData->Mastery.ML_DL_ScepterExpertise = iSkillValue;						break;
		case 532:				lpObj->CharacterData->Mastery.ML_DL_ShieldExpertise = iSkillValue;						break;
		case 533:				lpObj->CharacterData->Mastery.ML_DL_DelusionalGrandeur = iSkillValue;					break;
		case 535:				lpObj->CharacterData->Mastery.ML_DL_CompanionFortitude = iSkillValue;					break;
		case 536:				lpObj->CharacterData->Mastery.ML_DL_SwiftCrow = iSkillValue;							break;
		case 538:				lpObj->CharacterData->Mastery.ML_DL_LethalCrow = iSkillValue;							break;
		case 539:				lpObj->CharacterData->Mastery.ML_DL_Eyepecker = iSkillValue;							break;
		case 568:				lpObj->CharacterData->Mastery.ML_RF_FistMastery = iSkillValue;							break;
		case 571:				lpObj->CharacterData->Mastery.ML_RF_LethalPunches = iSkillValue;						break;
		case 574:				lpObj->CharacterData->Mastery.iMpsMonkAddVitalToDefense = iSkillValue;					break;
		case 575:				lpObj->CharacterData->Mastery.ML_RF_ImprovedFocus = iSkillValue;						break;
		case 577:				lpObj->CharacterData->Mastery.ML_RF_MasteredFocus = iSkillValue;						break;
		case 578:				lpObj->CharacterData->Mastery.ML_RF_Efficiency = iSkillValue;
								lpObj->CharacterData->Mastery.ML_RF_EfficiencyLevel = iSkillLevel;						break;
		case 579:				lpObj->CharacterData->Mastery.ML_RF_GladiatorsResolve = iSkillValue;					break;
		case 580:				lpObj->CharacterData->Mastery.ML_RF_GladiatorSpirit = iSkillValue;						break;
		case 581:				lpObj->CharacterData->Mastery.ML_RF_MentalQuickness = iSkillValue;						break;
		case 582:				lpObj->CharacterData->Mastery.ML_RF_Immunity = iSkillValue / 10;						break;
		case 583:				lpObj->CharacterData->Mastery.ML_RF_Jeweller = iSkillValue;								break;
		case 584:				lpObj->CharacterData->Mastery.ML_RF_GladiatorsVigor = iSkillValue;						break;
		case 585:				lpObj->CharacterData->Mastery.ML_RF_Regeneration = iSkillValue;							break;
		case 586:				lpObj->CharacterData->Mastery.ML_RF_Resistivity = iSkillValue / 10;						break;
		case 587:				lpObj->CharacterData->Mastery.ML_RF_PhysicalResistance = iSkillValue;					break;
		case 588:				lpObj->CharacterData->Mastery.ML_RF_Endurance = iSkillValue;							break;
		case 589:				lpObj->CharacterData->Mastery.ML_RF_ColdAffinity = iSkillValue / 10;					break;
		case 590:				lpObj->CharacterData->Mastery.ML_RF_Caretaker = iSkillValue;							break;
		case 591:				lpObj->CharacterData->Mastery.ML_RF_SuddenMovements = iSkillValue;						break;
		case 592:				lpObj->CharacterData->Mastery.iMpsImmuneRate_Monk = iSkillValue;						break;
		case 593:				lpObj->CharacterData->Mastery.ML_RF_Valor = iSkillValue;								break;
		case 594:				lpObj->CharacterData->Mastery.ML_RF_Vengeance = iSkillValue;							break;
		case 595:				lpObj->CharacterData->Mastery.ML_RF_Intelligence = iSkillValue;							break;
		case 596:				lpObj->CharacterData->Mastery.ML_RF_Resilience = iSkillValue;							break;
		case 597:				lpObj->CharacterData->Mastery.ML_RF_Quickness = iSkillValue;							break;
		case 598:				lpObj->CharacterData->Mastery.ML_RF_Brawn = iSkillValue;								break;
		case 599:				lpObj->CharacterData->Mastery.ML_RF_Accuracy = iSkillValue;								break;
		case 600:				lpObj->CharacterData->Mastery.ML_RF_Vigor = iSkillValue;								break;
		case 601:				lpObj->CharacterData->Mastery.ML_RF_RisingSpirit = iSkillValue;							break;
		case 602:				lpObj->CharacterData->Mastery.ML_RF_Vitality = iSkillValue;								break;
		case 603:				lpObj->CharacterData->Mastery.ML_RF_Precision = iSkillValue;							break;
		case 604:				lpObj->CharacterData->Mastery.ML_RF_ArcaneEfficiency = iSkillValue;						break;
		case 605:				lpObj->CharacterData->Mastery.ML_RF_Leech = iSkillValue;								break;
		case 606:				lpObj->CharacterData->Mastery.ML_RF_Slayer = iSkillValue;								break;
		case 607:				lpObj->CharacterData->Mastery.ML_RF_Stability = iSkillValue;							break;
		case 608:				lpObj->CharacterData->Mastery.ML_RF_Vanquisher = iSkillValue;							break;
		case 609:				lpObj->CharacterData->Mastery.ML_RF_Recklessness = iSkillValue;							break;
		case 610:				lpObj->CharacterData->Mastery.ML_RF_CriticalBlows = iSkillValue;						break;
		case 611:				lpObj->CharacterData->Mastery.ML_RF_ArcaneRecovery = iSkillValue;						break;
		case 612:				lpObj->CharacterData->Mastery.ML_RF_LastStand = iSkillValue;							break;
		case 613:				lpObj->CharacterData->Mastery.ML_RF_ExcellentBlows = iSkillValue;						break;
		case 614:				lpObj->CharacterData->Mastery.ML_RF_LethalBlows = iSkillValue;							break;
		case 615:				lpObj->CharacterData->Mastery.ML_RF_FatalBlows = iSkillValue;							break;
		case 616:				lpObj->CharacterData->Mastery.ML_RF_Overpower = iSkillValue;							break;
		case 617:				lpObj->CharacterData->Mastery.iMpsIncTripleDamageRate_Monk = iSkillValue;				break;

		case 386:
		case 488:				lpObj->CharacterData->Mastery.ML_MG_MagicMastery = iSkillValue;							break;
		case 335:
		case 485:
		case 513:
		case 557:				lpObj->CharacterData->Mastery.ML_GENERAL_WeaponMastery = iSkillValue;					break;
		case 534:				lpObj->CharacterData->Mastery.ML_DL_RazorBeak = iSkillValue;							break;
		case 321:
		case 374:
		case 409:
		case 445:
		case 475:
		case 504:
		case 548:				lpObj->CharacterData->Mastery.iMpsAddWingDamageBlock = iSkillValue;						break;
		case 322:
		case 375:
		case 410:
		case 446:
		case 476:
		case 505:
		case 549:				lpObj->CharacterData->Mastery.ML_GENERAL_WingsDefense = iSkillValue;					break;
		case 324:
		case 377:
		case 412:
		case 447:
		case 478:
		case 507:
		case 550:				lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage = iSkillValue;						break;
		default:																										break;
	}

	gObjCalCharacter.CHARACTER_Calc(lpObj->m_Index);
}
int CMasterLevelSkillTreeSystem::CheckSkillCondition(LPOBJ lpObj, int iMLSkill, int iSkillLevel) {
	if (gObjIsConnected(lpObj) == FALSE) {
		GSProtocol.GCResultSend(lpObj->m_Index, 0x51, 0x03);
		return 2;
	}

	if (iMLSkill < 0 || iMLSkill > MAX_SKILL-1) {
		g_Log.Add("error: %s %d", __FILE__, __LINE__);
		return 3;
	}

	if (this->ML_IsMasterCharacter(lpObj) == false) return 2;
	if (this->CheckMasterLevelSkill(iMLSkill) == false) return 4;
	if (this->GetMaxPointOfMasterSkill(lpObj->Class, iMLSkill) < iSkillLevel) return 4;

	int iReqMLPoint = this->GetRequireMLPoint(lpObj->Class, iMLSkill);

	if (iReqMLPoint < 0) return 4;
	if (lpObj->CharacterData->MasterPoint < iReqMLPoint) return 4;
	if (iSkillLevel != 1) return 1;
	if (this->CheckCompleteBrandOfSkill(lpObj, iMLSkill) == false) return 4;
	if (this->CheckPreviousRankSkill(lpObj, iMLSkill, 1) == false) return 4;

	return 1;
}
BOOL CMasterLevelSkillTreeSystem::CheckCompleteBrandOfSkill(LPOBJ lpObj, int iMLSkill) {
	if (MagicDamageC.GetSkillUseType(iMLSkill) == 4) {
		int iBrandOfSkill = MagicDamageC.GetBrandOfSkill(iMLSkill);
		BOOL iCompleteSkill = FALSE;

		if (iBrandOfSkill > 0) {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == iBrandOfSkill) {
					iCompleteSkill = TRUE;
					break;
				}
			}
		}

		if (iCompleteSkill == FALSE) return FALSE;
	}

	std::pair<int,int>iBrandOfSkill = std::pair<int, int>(this->GetParentSkill1(lpObj->Class, iMLSkill), this->GetParentSkill2(lpObj->Class, iMLSkill));

	if (iBrandOfSkill.first <= 0) return TRUE;

	BOOL iCompleteSkill1 = FALSE;
	BOOL iCompleteSkill2 = FALSE;

	if (iBrandOfSkill.first > 0) {
		for (int i = 0; i < MAX_MAGIC; i++) {
			if (lpObj->Magic[i].IsMagic() == TRUE && lpObj->Magic[i].m_Skill == iBrandOfSkill.first && lpObj->Magic[i].m_Level >= 10) {
				iCompleteSkill1 = TRUE;
				break;
			}
		}
	}

	if (iBrandOfSkill.second > 0) {
		for (int i = 0; i < MAX_MAGIC; i++) {
			if (lpObj->Magic[i].IsMagic() == TRUE && lpObj->Magic[i].m_Skill == iBrandOfSkill.second && lpObj->Magic[i].m_Level >= 10) {
				iCompleteSkill2 = TRUE;
				break;
			}
		}
	}

	if ((iBrandOfSkill.first + iBrandOfSkill.second) == iBrandOfSkill.first) {
		if (iCompleteSkill1 == TRUE) return TRUE;
	} else if ((iBrandOfSkill.first + iBrandOfSkill.second) == iBrandOfSkill.second) {
		if (iCompleteSkill2 == TRUE) return TRUE;
	} else if (iCompleteSkill1 == TRUE && iCompleteSkill2 == TRUE) return TRUE;

	return FALSE;
}
BOOL CMasterLevelSkillTreeSystem::CheckMasterLevelSkill(int iMLSkill) {
	int iSkillUseType = MagicDamageC.GetSkillUseType(iMLSkill);

	return iSkillUseType == 3 || iSkillUseType == 4;
}
BOOL CMasterLevelSkillTreeSystem::IsExistMLSBasedSkill(LPOBJ lpObj, int iSkill) {
	for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
		if (lpObj->Magic[iCount].IsMagic() == TRUE && this->CheckMasterLevelSkill(lpObj->Magic[iCount].m_Skill) == true && iSkill == MagicDamageC.GetBrandOfSkill(lpObj->Magic[iCount].m_Skill)) return TRUE;
	}

	return FALSE;
}
int CMasterLevelSkillTreeSystem::GetBaseMasterLevelSkill(int iMLSkill) {
	if (iMLSkill < 0 || iMLSkill > MAX_SKILL - 1) return -1;
	if (this->CheckMasterLevelSkill(iMLSkill) == true) return iMLSkill;
	return -1;
}
int CMasterLevelSkillTreeSystem::gObjMagicAdd_MLS(OBJECTSTRUCT *lpObj, int iMLSkill, int iSkillLevel) {
	if (g_EnergyCheckOff == TRUE) {
		int reqeng = MagicDamageC.MAGIC_GetRequireEnergy(iMLSkill);

		if (reqeng < 0) {
			g_Log.Add("error-L2: Skill energy not enough");
			return -1;
		}
		if (lpObj->Class == CLASS_KNIGHT) reqeng -= 10;
		if (lpObj->Class == CLASS_ELF) reqeng -= 5;
		if (lpObj->Class == CLASS_DARKLORD) reqeng -= 5;
		if ((lpObj->CharacterData->Energy + lpObj->AddEnergy) < reqeng) {
			g_Log.Add("error-L2: Skill energy not enough");
			return -1;
		}
	}

	if (MagicDamageC.MAGIC_GetRequireClass(lpObj->Class,lpObj->CharacterData->ChangeUP, iMLSkill) < 1 && lpObj->Type == OBJ_USER && lpObj->CharacterData->ISBOT == false) {
		g_Log.Add("error-L2: His class is not allowed to use the skill [ID: %d]", iMLSkill);
		return -1;
	}

	for (int n = 0; n < MAX_MAGIC; n++) {
		if (lpObj->Magic[n].IsMagic() == TRUE && lpObj->Magic[n].m_Skill == iMLSkill && lpObj->Magic[n].m_Level == iSkillLevel) {
			g_Log.Add("Same skill already exists %d %d", lpObj->Magic[n].m_Skill,iMLSkill);
			return -1;
		}
	}

	if (MagicDamageC.GetSkillUseType(iMLSkill) == 3) {
		if (iSkillLevel > 1) {
			for (int n = 0; n < MAX_MAGIC; n++) {
				if (lpObj->Magic[n].IsMagic() == TRUE && lpObj->Magic[n].m_Skill == iMLSkill && lpObj->Magic[n].m_Level == iSkillLevel-1) {
					if (lpObj->Magic[n].UpdateMasterSkill(iMLSkill, iSkillLevel) < 0) return -1;
					else return n;
				}
			}
		}
	} else if (MagicDamageC.GetSkillUseType(iMLSkill) == 4) {
		int iBrand = MagicDamageC.GetBrandOfSkill(iMLSkill);

		for (int n = 0; n < MAX_MAGIC; n++) {
			if (lpObj->Magic[n].IsMagic() == TRUE) {
				if (lpObj->Magic[n].m_Skill == iBrand) lpObj->Magic[n].m_Pass = 1;
				if (lpObj->Magic[n].m_Skill == iMLSkill) {
					if (lpObj->Magic[n].UpdateMasterSkill(iMLSkill, iSkillLevel) < 0) return -1;
					else return n;
				}
			}
		}
	} else return -1;

	for (int n = 0; n < MAX_MAGIC; n++) {
		if (lpObj->Magic[n].IsMagic() == FALSE) {
			if (lpObj->Magic[n].Set(iMLSkill, iSkillLevel) >= 0) {
				lpObj->MagicCount++;

				if (MagicDamageC.GetSkillUseType(iMLSkill) == 4 && !MagicDamageC.GetBrandOfSkill(iMLSkill)) GSProtocol.GCMagicListOneSend(lpObj->m_Index, n, lpObj->Magic[n].m_Skill, lpObj->Magic[n].m_Level, 0, 0);
				return n;
			} else {
				g_Log.Add("Same magic already exists %s %d",__FILE__,__LINE__);
				return -1;
			}
		}
	}

	g_Log.Add("No space to add magic %s %d",__FILE__,__LINE__);
	return -1;
}
void CMasterLevelSkillTreeSystem::CGReqGetMasterLevelSkillTree(int aIndex) {
	if (gObjIsConnected(aIndex) == FALSE) {
		GSProtocol.GCSendDisableReconnect(aIndex);
		//IOCP.CloseClient(aIndex); // lol
		return;
	}

	BYTE sendbuf[3072];
	PMSG_MASTERLEVELSKILLTREE_COUNT pCountMsg;
	PMSG_MASTERLEVELSKILLTREE pElementMsg;
	LPOBJ lpObj = &gObj[aIndex];

	PHeadSubSetW((LPBYTE)&pCountMsg, 0xF3, 0x53, sizeof(pCountMsg));
	memset(sendbuf, 0x00, sizeof(sendbuf));

	int nCount = 0;
	int lOfs = 12;

	for (int i = 0; i < MAX_MAGIC; i++) {
		if (lpObj->Magic[i].IsMagic() == TRUE && this->CheckMasterLevelSkill(lpObj->Magic[i].m_Skill) == true) {
			pElementMsg.btMasterSkillUIIndex = this->GetMasterSkillUIIndex(lpObj->Class, lpObj->Magic[i].m_Skill);
			pElementMsg.btMasterSkillLevel = lpObj->Magic[i].m_Level;

			if (this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[i].m_Skill), lpObj->Magic[i].m_Level) <= 0.0) pElementMsg.fMasterSkillCurValue = -1.0;
			else pElementMsg.fMasterSkillCurValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[i].m_Skill), lpObj->Magic[i].m_Level);
			if (this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[i].m_Skill), lpObj->Magic[i].m_Level + 1) <= 0.0) pElementMsg.fMasterSkillNextValue = -1.0;
			else pElementMsg.fMasterSkillNextValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[i].m_Skill), lpObj->Magic[i].m_Level + 1);

			nCount++;
			memcpy(&sendbuf[lOfs], &pElementMsg, sizeof(pElementMsg));
			lOfs += sizeof(pElementMsg);
		}
	}
	
	pCountMsg.count = nCount;
	pCountMsg.h.sizeH = HIBYTE(lOfs);
	pCountMsg.h.sizeL = LOBYTE(lOfs);
	memcpy(sendbuf, &pCountMsg, sizeof(pCountMsg));
	IOCP.DataSend(aIndex, sendbuf, lOfs);
}
void CMasterLevelSkillTreeSystem::CGReqGetMasterLevelSkill(PMSG_REQ_MASTERLEVEL_SKILL *lpMsg, int aIndex) {
	if (!gObjIsConnected(aIndex)) {
		GSProtocol.GCSendDisableReconnect(aIndex);
		//IOCP.CloseClient(aIndex);
		return;
	}

	if (gObj[aIndex].Type != OBJ_USER) return;

	PMSG_ANS_MASTERLEVELSKILLTREE pMsg;
	LPOBJ lpObj = &gObj[aIndex];

	PHeadSubSetB((LPBYTE)&pMsg, 0xF3, 0x52, sizeof(pMsg));
	pMsg.btResult = 0;
	pMsg.wMasterLevelPoint = lpObj->CharacterData->MasterPoint;
	pMsg.dwMasterSkillIndex = -1;
	pMsg.dwMasterSkillLevel = 0;

	if (lpObj->m_IfState.use && lpObj->m_IfState.type == 1) {
		g_Log.Add("[MasterSkill] [%s][%s] Fail(Use Trade Interface) - Add Magic List", lpObj->AccountID, lpObj->Name);
		pMsg.btResult = 6;
		IOCP.DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
		return;
	}

	int iCurSkillLevel = this->ML_CheckNodeLevel(lpObj, lpMsg->iMasterLevelSkill);
	int iSkillLevel = this->GetRequireMLPoint(lpObj->Class, lpMsg->iMasterLevelSkill) + iCurSkillLevel;
	int iResult = this->CheckSkillCondition(lpObj, lpMsg->iMasterLevelSkill, iSkillLevel);

	if (iResult != TRUE) {
		g_Log.Add("[MasterSkill] [%s][%s] Fail - Result:%d, Skill:%d Level:%d MLPoint:%d", lpObj->AccountID, lpObj->Name, iResult, lpMsg->iMasterLevelSkill, iSkillLevel, lpObj->CharacterData->MasterPoint);
		pMsg.btResult = iResult;
	} else {
		if (this->gObjMagicAdd_MLS(lpObj, lpMsg->iMasterLevelSkill, iSkillLevel) < 0) g_Log.Add("[MasterSkill] [%s][%s] Fail - Add Magic List, Skill:%d Level:%d", lpObj->AccountID, lpObj->Name, lpMsg->iMasterLevelSkill, iSkillLevel);
		else {
			int iReqMLPoint = this->GetRequireMLPoint(lpObj->Class, lpMsg->iMasterLevelSkill);

			if (iReqMLPoint < 0) return;

			lpObj->CharacterData->MasterPoint -= iReqMLPoint;
			lpObj->CharacterData->MasterPointUsed += iReqMLPoint;
			pMsg.btResult = TRUE;
			pMsg.wMasterLevelPoint = lpObj->CharacterData->MasterPoint;
			pMsg.btMasterSkillUIIndex = this->GetMasterSkillUIIndex(lpObj->Class, lpMsg->iMasterLevelSkill);
			pMsg.dwMasterSkillIndex = lpMsg->iMasterLevelSkill;
			pMsg.dwMasterSkillLevel = iSkillLevel;

			if (this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMsg->iMasterLevelSkill), iSkillLevel) <= 0.0) pMsg.fMasterSkillCurValue = -1.0;
			else pMsg.fMasterSkillCurValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMsg->iMasterLevelSkill), iSkillLevel);
			if (this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMsg->iMasterLevelSkill), iSkillLevel + 1) <= 0.0) pMsg.fMasterSkillNextValue = -1.0;
			else pMsg.fMasterSkillNextValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMsg->iMasterLevelSkill), iSkillLevel + 1);

			this->ML_CalcPassiveSkillData(lpObj, lpMsg->iMasterLevelSkill, iSkillLevel);
			g_Log.Add("[MasterSkill] [%s][%s] Success - Add Magic List, Skill:%d Level:%d MLPoint:%d", lpObj->AccountID, lpObj->Name, lpMsg->iMasterLevelSkill, iSkillLevel, lpObj->CharacterData->MasterPoint);
		}
	}

	IOCP.DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
}
BOOL CMasterLevelSkillTreeSystem::RunningSkill_MLS(OBJECTSTRUCT *lpObj, int aTargetIndex, CMagicInf *lpMagic, BOOL bCombo, BYTE x, BYTE y, BYTE aTargetPos) {
	if (!lpObj || !lpMagic) return FALSE;
	if (this->m_Lua.GetLua() == NULL) return FALSE;

	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	switch (lpMagic->m_Skill) {
		case 403:				this->ML_DW_ImprovedManaShield(lpObj, aTargetIndex, lpMagic);																					break;
		case 413:				this->ML_ELF_ImprovedHeal(lpObj, aTargetIndex, lpMagic);																						break;
		case 330:
		case 332:
		case 481:				this->ML_DK_ImprovedTwistingSlash(lpObj, lpMagic, aTargetIndex, bCombo);																		break;
		case 360:				this->ML_DK_Innervation(lpObj, lpMagic);																										break;
		case 356:				this->ML_DK_ImprovedLifeSwell(lpObj, lpMagic);																									break;
		case 512:
		case 516:				this->ML_DL_ImprovedEarthquake(lpObj, aTargetIndex, lpMagic);																					break;
		case 508:
		case 514:				this->ML_DL_ImprovedFireburst(lpObj, aTargetIndex, lpMagic);																					break;
		case 391:				this->ML_DW_ImprovedIceStorm(lpObj->m_Index, lpMagic, x, y, aTargetIndex);																		break;
		case 336:				this->ML_DK_ImprovedDeathStab(lpObj->m_Index, aTargetIndex, lpMagic, bCombo);																	break;
		case 331:
		case 333:				this->ML_DK_ImprovedRagefulBlow(lpObj->m_Index, lpMagic, x, y, aTargetIndex, bCombo);															break;
		case 417:				this->ML_ELF_ImprovedEnhance(lpObj->m_Index, aTargetIndex, lpMagic);																			break;
		case 420:				this->ML_ELF_ImprovedEmpower(lpObj->m_Index, aTargetIndex, lpMagic);																			break;
		case 482:				this->ML_MG_ImprovedPowerSlash(lpObj->m_Index, lpMagic, x, y, aTargetPos, aTargetIndex);														break;
		case 454:				this->ML_SUM_ImprovedSleep(lpObj->m_Index, aTargetIndex, lpMagic);																				break;
		case 455:				this->ML_SUM_ImprovedChainLightning(lpObj->m_Index, aTargetIndex, lpMagic);																		break;
		case 456:				this->ML_SUM_ImprovedLightningBlast(lpObj->m_Index, aTargetIndex, lpMagic);																		break;
		case 458:				this->ML_SUM_ImprovedDrainLife(lpObj->m_Index, aTargetIndex, lpMagic);																			break;
		case 326:				this->ML_DK_TornadoSlash(lpObj, lpMagic, aTargetIndex, bCombo);																					break;
		case 479:				this->ML_MG_TornadoSlash(lpObj, lpMagic, aTargetIndex, 0);																						break;
		case 327:				this->ML_DK_SteppingSlash(lpObj, lpMagic, aTargetIndex, bCombo);																				break;
		case 328:				this->ML_DK_LeapingSlash(lpObj, lpMagic, aTargetIndex, bCombo);																					break;
		case 329:				this->ML_DK_Stab(lpObj, lpMagic, aTargetIndex, bCombo);																							break;
		case 337:				this->ML_DK_ImprovedGroundslam(lpObj, lpMagic, x, y, aTargetIndex, bCombo);																		break;
		case 380:				this->ML_DW_ImprovedArcanePower(lpObj, lpMagic, aTargetIndex);																					break;
		case 383:				this->ML_DW_ArcaneFocus(lpObj, lpMagic, aTargetIndex);																							break;
		case 379:
		case 480:				this->ML_DW_ImprovedLightning(lpObj, lpMagic, aTargetIndex);																					break;
		case 384:				this->ML_DW_ImprovedPoison(lpObj, lpMagic, aTargetIndex);																						break;
		case 387:				this->ML_DW_Rot(lpObj->m_Index, lpMagic, x, y, aTargetIndex);																					break;
		case 389:
		case 489:				this->ML_DW_ImprovedIce(lpObj, lpMagic, aTargetIndex);																							break;
		case 404:				this->ML_DW_ManaBarrier(lpObj, lpMagic, aTargetIndex);																							break;
		case 422:				this->ML_ELF_Berserk(lpObj->m_Index, aTargetIndex, lpMagic);																					break;
		case 423:				this->ML_ELF_SpiritWard(lpObj->m_Index, aTargetIndex, lpMagic);																					break;
		case 441:				this->ML_ELF_EternalArrow(lpObj->m_Index, aTargetIndex, lpMagic);																				break;
		case 469:				this->ML_SUM_ImprovedAmplifyDamage(lpObj->m_Index, lpMagic);																					break;
		case 470:				this->ML_SUM_DimensionalAffinity(lpObj->m_Index, lpMagic);																						break;
		case 472:				this->ML_SUM_VoidAffinity(lpObj->m_Index, lpMagic);																								break;
		case 511:				this->ML_DL_ImprovedConcentration(lpObj->m_Index, lpMagic);																						break;
		case 515:				this->ML_DL_EnhancedConcentration(lpObj->m_Index, lpMagic);																						break;
		case 517:				this->ML_DL_CriticalConcentration(lpObj->m_Index, lpMagic);																						break;
		case 509:				this->ML_DL_ImprovedForce(lpObj->m_Index, aTargetIndex, lpMagic);																				break;
		case 424:				gObjAttack(lpObj, lpTargetObj, lpMagic, 1, 0, 0, 0, 0, 0);																						break;
		case 551:
		case 552:
		case 554:
		case 555:
		case 558:
		case 562:				this->ML_RF_ImprovedWeaponSkill(lpObj->m_Index, lpMagic, aTargetIndex);																			break;
		case 559:
		case 563:				gObjUseSkill.RF_DarkSide(lpObj->m_Index, aTargetIndex, lpMagic);																				break;
		case 560:
		case 561:				this->ML_RF_ImprovedMoltenStrike(lpObj->m_Index, lpMagic, gObj[aTargetIndex].X, gObj[aTargetIndex].Y, aTargetIndex, 3, TRUE, 0);				break;
		case 569:				this->ML_RF_ImprovedLightningReflexes(lpObj->m_Index, lpMagic);																					break;
		case 572:				this->ML_RF_MasteredReflexes(lpObj->m_Index, lpMagic);																							break;
		case 573:				this->ML_RF_ImprovedToughness(lpObj->m_Index, lpMagic);																							break;
		case 575:
		case 577:				this->ML_RF_ImprovedFocus(lpObj->m_Index, lpMagic);																								break;
		case 344:
		case 346:				this->ML_DK_BladeStorm(lpObj, lpMagic, x, y, aTargetIndex, bCombo);																				break;
		case 495:
		case 497:				this->ML_DW_EarthenPrison(lpObj, aTargetIndex, lpMagic);																						break;
		case 427:
		case 434:				this->ML_ELF_PoisonArrow(lpObj, lpMagic, aTargetIndex, bCombo);																					break;
		case 425:				this->ML_ELF_Cure(lpObj, lpMagic, aTargetIndex);																								break;
		case 432:				this->ML_ELF_SummonSatyr(lpObj->m_Index, 597, lpObj->X - 1, lpObj->Y + 1);																		break;
		case 426:
		case 429:				this->ML_ELF_PrayerOfHealing(lpObj->m_Index, aTargetIndex, lpMagic);																			break;
		case 430:				this->ML_ELF_Bless(lpObj, aTargetIndex, lpMagic);																								break;
		case 433:				this->ML_ELF_ImprovedBless(lpObj, aTargetIndex, lpMagic);																						break;
		case 461:
		case 463:				this->ML_SUM_Blind(lpObj, aTargetIndex, lpMagic);																								break;
		case 521:				this->ML_DL_Stoneskin(lpObj, aTargetIndex, lpMagic);																							break;
		case 565:				this->ML_RF_Bloodlust(lpObj, aTargetIndex, lpMagic);																							break;
		case 339:				this->ML_DK_PuncturingStab(lpObj->m_Index, aTargetIndex, lpMagic, bCombo);																		break;
		case 342:				this->ML_DK_DeathBlow(lpObj->m_Index, aTargetIndex, lpMagic, bCombo);																			break;
		case 340:				this->ML_DK_EarthShake(lpObj, lpMagic, x, y, aTargetIndex, bCombo);																				break;
		case 343:				this->ML_DK_ArcticBlow(lpObj, lpMagic, x, y, aTargetIndex, bCombo);																				break;
		case 393:				this->ML_DW_Blizzard(lpObj->m_Index, lpMagic, x, y, aTargetIndex);																				break;
		case 392:				this->ML_DW_Supernova(lpObj->m_Index, aTargetIndex, lpMagic);																					break;
		case 390:
		case 394:				gObjAttack(lpObj, lpTargetObj, lpMagic, 1, 0, 0, 0, 0, 0);																						break;
		case 411:
		case 431:				this->ML_ELF_ImprovedBarrage(lpObj->m_Index, lpMagic, aTargetPos, aTargetIndex);																break;
		case 363:				this->ML_DK_Invigoration(lpObj, lpMagic);																										break;			
		case 406:				this->ML_DW_SoulBarrier(lpObj, lpMagic, aTargetIndex);																							break;
		case 459:				this->ML_SUM_ImprovedEnfeeble(lpObj->m_Index, lpMagic, x, y, aTargetIndex);																		break;
		case 460:				this->ML_SUM_ImprovedWeakness(lpObj->m_Index, lpMagic, x, y, aTargetIndex);																		break;
		case 462:				this->ML_SUM_ImprovedDrainLife(lpObj->m_Index, aTargetIndex, lpMagic);																			break;
		case 491:				this->ML_MG_Freeze(lpObj, lpMagic, aTargetIndex);																								break;
		case 492:
		case 494:				this->ML_MG_ImprovedFlameStrike(lpObj->m_Index, lpMagic, aTargetPos, aTargetIndex);																break;
		case 496:				this->ML_MG_ImprovedThunderStorm(lpObj->m_Index, lpMagic, x, y, aTargetIndex);																	break;
		case 519:				this->ML_DL_ImprovedSpark(lpObj->m_Index, lpMagic, x, y, aTargetPos, aTargetIndex);																break;
		case 522:				this->ML_DL_ExcellentConcentration(lpObj->m_Index, lpMagic);																					break;
		case 523:				this->ML_DL_ImprovedDarkness(lpObj->m_Index, lpMagic, aTargetPos, aTargetIndex);																break;
		case 524:				this->ML_DL_ImprovedStoneskin(lpObj, aTargetIndex, lpMagic);																					break;
		case 564:
		case 566:				gObjAttack(lpObj, lpTargetObj, lpMagic, 1, 0, 0, 0, 0, 0);																						break;
		case 567:				this->ML_RF_ImprovedBloodlust(lpObj, aTargetIndex, lpMagic);																					break;
		default:																																								break;
	}

	return TRUE;
}
int CMasterLevelSkillTreeSystem::GetRequireMLPoint(int iClass, int iSkill) {
	if (iClass < 0 || iClass >= MAX_TYPE_PLAYER) {
		g_Log.Add("[MasterLevelSkillTreeSystem] GetRequireMLPoint - Invalid Class Code  : %d", iClass);
		return -1;
	}

	if (iSkill < 0 || iSkill > 700) {
		g_Log.Add("[MasterLevelSkillTreeSystem] GetRequireMLPoint - Invalid Skill Code  : %d", iSkill);
		return -1;
	}

	for (int iTreeKind = 0; iTreeKind < MAX_TREE_TYPE; iTreeKind++) {
		for (int iRank = 0; iRank < MAX_MASTER_SKILL_RANK; iRank++) {
			for (int iPos = 0; iPos < MAX_SKILL_POS; iPos++) {
				if (this->m_MLSTable[iClass].iAbility[iTreeKind].iElement[iRank][iPos].m_dwSkillID == iSkill) return this->m_MLSTable[iClass].iAbility[iTreeKind].iElement[iRank][iPos].m_byConsumePoint;
			}
		}
	}

	return -1;
}
int CMasterLevelSkillTreeSystem::GetMaxPointOfMasterSkill(int iClass, int iSkill) {
	if (iClass < 0 || iClass >= MAX_TYPE_PLAYER) {
		g_Log.Add("[MasterLevelSkillTreeSystem] GetMaxPointOfMasterSkill - Invalid Class Code  : %d", iClass);
		return -1;
	}

	if (iSkill < 0 || iSkill > 700) {
		g_Log.Add("[MasterLevelSkillTreeSystem] GetMaxPointOfMasterSkill - Invalid Skill Code  : %d", iSkill);
		return -1;
	}

	for (int iTreeKind = 0; iTreeKind < MAX_TREE_TYPE; iTreeKind++) {
		for (int iRank = 0; iRank < MAX_MASTER_SKILL_RANK; iRank++) {
			for (int iPos = 0; iPos < MAX_SKILL_POS; iPos++) {
				if (this->m_MLSTable[iClass].iAbility[iTreeKind].iElement[iRank][iPos].m_dwSkillID == iSkill) return this->m_MLSTable[iClass].iAbility[iTreeKind].iElement[iRank][iPos].m_byMaxPoint;
			}
		}
	}

	return -1;
}
float CMasterLevelSkillTreeSystem::ML_GetNodeValue(int iType, int iPoint) {
	if (iPoint < 0 || iPoint > this->GetMaxMasterSkillLevel() + 1) {
		g_Log.Add("[MasterLevelSkillTreeSystem] ML_GetNodeValue - Invalid Point : %d", iPoint);
		return 0.0;
	}

	if (iType < 0 || iType > MAX_MASTER_SKILL_TYPE) {
		g_Log.Add("[MasterLevelSkillTreeSystem] ML_GetNodeValue - Invalid Type : %d", iType);
		return 0.0;
	}

	if (iPoint == this->GetMaxMasterSkillLevel() + 1 || iPoint == 0) return 0.0;
	return this->m_MLSValueTable[iType].fValueTable[iPoint];
}
float CMasterLevelSkillTreeSystem::GetMasterSkillManaIncRate(int iPoint) {
	if (iPoint < 1 || iPoint > this->GetMaxMasterSkillLevel()) {
		g_Log.Add("[MasterLevelSkillTreeSystem] GetMasterSkillManaIncRate - Invalid Point : %d", iPoint);
		return -1.0;
	}

	for (int i = 0; i < MAX_MASTER_SKILL_TYPE; i++) {
		if (this->m_MLSValueTable[i].iValueType == MLST_VALUE_MANA_INC) return this->m_MLSValueTable[i].fValueTable[iPoint];
	}

	return -1.0;
}
float CMasterLevelSkillTreeSystem::GetBrandOfMasterSkillValue(LPOBJ lpObj, int iSkill, int iStep) {
	int nStep = iStep;
	int nPrevSkill = iSkill;
	int nSkill = iSkill;

	while (nStep && nPrevSkill) {
		WORD nBrandOfSkill = MagicDamageC.GetBrandOfSkill(nSkill);
		nPrevSkill = nBrandOfSkill;
		nSkill = nBrandOfSkill;
		nStep--;
	}

	if (this->CheckMasterLevelSkill(nSkill)) {
		for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
			if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == nSkill) return this->ML_GetNodeValue(MagicDamageC.SkillGet(nSkill), lpObj->Magic[iCount].m_Level);
		}
	}

	return 0.0;
}
int CMasterLevelSkillTreeSystem::CheckPreviousRankSkill(LPOBJ lpObj, int iSkill, int iLevel) {
	if (iSkill < 0 || iSkill > 700) {
		g_Log.Add("[MasterLevelSkillTreeSystem] CheckPreviousRankSkill - Invalid Skill Code  : %d", iSkill);
		return -1;
	}

	int nTreeKind = -1;
	int nRank = MagicDamageC.GetSkillRank(iSkill) - 1;

	if (nRank < 1) return 1;

	int iClass = lpObj->Class;

	for (int iTreeKind = 0; iTreeKind < MAX_TREE_TYPE; iTreeKind++) {
		for (int iRank = 0; iRank < MAX_MASTER_SKILL_RANK; iRank++) {
			for (int iPos = 0; iPos < MAX_SKILL_POS; iPos++) {
				if (this->m_MLSTable[iClass].iAbility[iTreeKind].iElement[iRank][iPos].m_dwSkillID == iSkill) {
					nTreeKind = iTreeKind;
					break;
				}
			}
		}
	}

	int nPoint = 0;
	nRank--;

	for (int iPos = 0; iPos < MAX_SKILL_POS; iPos++) {
		for (int i = 0; i < MAX_MAGIC; i++) {
			if (lpObj->Magic[i].IsMagic() == TRUE && this->CheckMasterLevelSkill(lpObj->Magic[i].m_Skill) == true) {
				if (lpObj->Magic[i].m_Skill == this->m_MLSTable[iClass].iAbility[nTreeKind].iElement[nRank][iPos].m_dwSkillID) nPoint |= lpObj->Magic[i].m_Level;
			}
		}
	}

	return nPoint >= 10;
}
int CMasterLevelSkillTreeSystem::ML_CheckNodeLevel(LPOBJ lpObj, int iSkill) {
	for (int i = 0; i < MAX_MAGIC; i++) {
		if (lpObj->Magic[i].IsMagic() == TRUE && lpObj->Magic[i].m_Skill == iSkill) return lpObj->Magic[i].m_Level;
	}

	return FALSE;
}
int CMasterLevelSkillTreeSystem::GetMasterSkillUIIndex(int iClass, int iSkill) {
	if (iClass < 0 || iClass >= MAX_TYPE_PLAYER) {
		g_Log.Add("[MasterLevelSkillTreeSystem] GetMasterSkillUIIndex - Invalid Class Code  : %d", iClass);
		return -1;
	}

	if (iSkill < 0 || iSkill > 700) {
		g_Log.Add("[MasterLevelSkillTreeSystem] GetMasterSkillUIIndex - Invalid Skill Code  : %d", iSkill);
		return -1;
	}

	for (int iTreeKind = 0; iTreeKind < MAX_TREE_TYPE; iTreeKind++) {
		for (int iRank = 0; iRank < MAX_MASTER_SKILL_RANK; iRank++) {
			for (int iPos = 0; iPos < MAX_SKILL_POS; iPos++) {
				if (this->m_MLSTable[iClass].iAbility[iTreeKind].iElement[iRank][iPos].m_dwSkillID == iSkill) return this->m_MLSTable[iClass].iAbility[iTreeKind].iElement[iRank][iPos].m_dwIndex;
			}
		}
	}

	return -1;
}
int CMasterLevelSkillTreeSystem::GetParentSkill1(int iClass, int iSkill) {
	for (int iTreeKind = 0; iTreeKind < MAX_TREE_TYPE; iTreeKind++) {
		for (int iRank = 0; iRank < MAX_MASTER_SKILL_RANK; iRank++) {
			for (int iPos = 0; iPos < MAX_SKILL_POS; iPos++) {
				if (this->m_MLSTable[iClass].iAbility[iTreeKind].iElement[iRank][iPos].m_dwSkillID == iSkill) return this->m_MLSTable[iClass].iAbility[iTreeKind].iElement[iRank][iPos].m_dwParentSkill1;
			}
		}
	}

	return FALSE;
}
int CMasterLevelSkillTreeSystem::GetParentSkill2(int iClass, int iSkill) {
	for (int iTreeKind = 0; iTreeKind < MAX_TREE_TYPE; iTreeKind++) {
		for (int iRank = 0; iRank < MAX_MASTER_SKILL_RANK; iRank++) {
			for (int iPos = 0; iPos < MAX_SKILL_POS; iPos++) {
				if (this->m_MLSTable[iClass].iAbility[iTreeKind].iElement[iRank][iPos].m_dwSkillID == iSkill) return this->m_MLSTable[iClass].iAbility[iTreeKind].iElement[iRank][iPos].m_dwParentSkill2;
			}
		}
	}

	return FALSE;
}
void CMasterLevelSkillTreeSystem::SetItemMLPassiveSkill(LPOBJ lpObj, int iType) {
	switch (iType) {
		case ITEM_TWOHAND_SWORD: {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 348) {
					lpObj->CharacterData->Mastery.ML_DK_TwoHandedSwordMastery = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					break;
				}
			}

			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 352) {
					lpObj->CharacterData->Mastery.ML_DK_TwoHandedSwordExpertise = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEM_ONEHAND_SWORD: {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 349) {
					lpObj->CharacterData->Mastery.ML_DK_OneHandedSwordMastery = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					break;
				}
			}

			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 353) {
					lpObj->CharacterData->Mastery.ML_DK_OneHandedSwordExpertise = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEM_MACE: {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 350) {
					lpObj->CharacterData->Mastery.ML_DK_MaceMastery = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					break;
				}
			}

			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 354) {
					lpObj->CharacterData->Mastery.ML_DK_MaceExpertise = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEM_SPEAR: {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 351) {
					lpObj->CharacterData->Mastery.ML_DK_SpearMastery = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					break;
				}
			}

			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 355) {
					lpObj->CharacterData->Mastery.ML_DK_SpearExpertise = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEM_ONEHAND_STAFF: {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 397) {
					lpObj->CharacterData->Mastery.ML_DW_OneHandedStaffMastery = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					break;
				}
			}

			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 400) {
					lpObj->CharacterData->Mastery.ML_DW_OneHandedStaffExpertise = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEM_TWOHAND_STAFF: {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 398) {
					lpObj->CharacterData->Mastery.ML_DW_TwoHandedStaffMastery = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					break;
				}
			}

			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 401) {
					lpObj->CharacterData->Mastery.ML_DW_TwoHandedStaffExpertise = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEM_SHIELD: {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE) {
					if (lpObj->Magic[iCount].m_Skill == 399) {
						lpObj->CharacterData->Mastery.ML_GENERAL_ShieldMastery = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
						break;
					}

					if (lpObj->Magic[iCount].m_Skill == 437) {
						lpObj->CharacterData->Mastery.ML_ELF_ShieldMastery = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
						break;
					}


					if (lpObj->Magic[iCount].m_Skill == 528) {
						lpObj->CharacterData->Mastery.ML_DL_ShieldMastery = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
						break;
					}
				}
			}

			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE) {
					if (lpObj->Magic[iCount].m_Skill == 402) {
						lpObj->CharacterData->Mastery.ML_GENERAL_ShieldExpertise = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
						return;
					}

					if (lpObj->Magic[iCount].m_Skill == 440) {
						lpObj->CharacterData->Mastery.ML_ELF_ShieldExpertise = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
						return;
					}

					if (lpObj->Magic[iCount].m_Skill == 532) {
						lpObj->CharacterData->Mastery.ML_DL_ShieldExpertise = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
						return;
					}
				}
			}
		}
		break;

		case ITEM_BOW: {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 435) {
					lpObj->CharacterData->Mastery.ML_ELF_BowMastery = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					break;
				}
			}

			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 438) {
					lpObj->CharacterData->Mastery.ML_ELF_BowExpertise = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEM_CROSSBOW: {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 436) {
					lpObj->CharacterData->Mastery.ML_ELF_CrossbowMastery = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					break;
				}
			}

			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 439) {
					lpObj->CharacterData->Mastery.ML_ELF_CrossbowExpertise = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEM_SUMMONER_STAFF: {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 465) {
					lpObj->CharacterData->Mastery.ML_SUM_StaffMastery = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					break;
				}
			}

			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 467) {
					lpObj->CharacterData->Mastery.ML_SUM_StaffExpertise = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEM_SUMMONER_BOOK: {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 466)
				{
					lpObj->CharacterData->Mastery.ML_SUM_CurseMastery = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					break;
				}
			}

			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 468)
				{
					lpObj->CharacterData->Mastery.ML_SUM_CurseExpertise = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEM_DARKLORD_SCEPTER: {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 527) {
					lpObj->CharacterData->Mastery.ML_DL_ScepterMastery = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					break;
				}
			}

			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 529) {
					lpObj->CharacterData->Mastery.ML_DL_Commander = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					break;
				}
			}

			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 531) {
					lpObj->CharacterData->Mastery.ML_DL_ScepterExpertise = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					break;
				}
			}

			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 533) {
					lpObj->CharacterData->Mastery.ML_DL_DelusionalGrandeur = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEM_RAGEFIGHTER_WEAPON: {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 568) {
					lpObj->CharacterData->Mastery.ML_RF_FistMastery = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					break;
				}
			}

			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 571) {
					lpObj->CharacterData->Mastery.ML_RF_LethalPunches = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		default:
		return;
	}
}
void CMasterLevelSkillTreeSystem::SetPetItemMLPassiveSkill(LPOBJ lpObj, int iType) {
	if (iType == ITEMGET(13, 4)) {
		for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
			if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 510) {
				lpObj->CharacterData->Mastery.iMpsIncDarkHorseDefense = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
				return;
			}
		}
	}
}
void CMasterLevelSkillTreeSystem::SetWingMLPassiveSkill(LPOBJ lpObj, int iType) {
	switch (iType) {
		case ITEMGET(12, 36): {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 324) {
					lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEMGET(12, 37): {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 377) {
					lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEMGET(12, 38): {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 412) {
					lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEMGET(12, 39): {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 478) {
					lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEMGET(12, 40): {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 507) {
					lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEMGET(12, 43): {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 447) {
					lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		case ITEMGET(12, 50): {
			for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
				if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == 550) {
					lpObj->CharacterData->Mastery.ML_GENERAL_WingsDamage = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpObj->Magic[iCount].m_Skill), lpObj->Magic[iCount].m_Level);
					return;
				}
			}
		}
		break;

		default:
		return;
	}
}
int CMasterLevelSkillTreeSystem::ML_GetNodeLevel(LPOBJ lpObj, int iSkill) {
	for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
		if (lpObj->Magic[iCount].IsMagic() == TRUE && lpObj->Magic[iCount].m_Skill == iSkill) return lpObj->Magic[iCount].m_Level;
	}

	return -1;
}
int CMasterLevelSkillTreeSystem::GetScriptClassCode(int iClass) {
	if (iClass < 0 || iClass >= MAX_TYPE_PLAYER) {
		g_Log.Add("[MasterLevelSkillTreeSystem] GetScriptClassCode - Invalid Class Code  : %d", iClass);
		return -1;
	}

	for (int i = 0; i < MAX_TYPE_PLAYER; i++) {
		if (CHARACTER_CLASS_TABLE[i][1] == iClass) return CHARACTER_CLASS_TABLE[i][0];
	}

	return -1;
}
float CMasterLevelSkillTreeSystem::RoundValue(long double dbValue, int iPos) {
	char szPos[16];
	char szTemp[256];

	sprintf(szPos, "%%.%d", iPos);
	strcat(szPos, "f");
	sprintf(szTemp, szPos, dbValue);
	return atof(szTemp);
}
BYTE CMasterLevelSkillTreeSystem::ResetMasterSkill(int aIndex, int nTreeType) {
	LPOBJ lpObj = &gObj[aIndex];

	if (!lpObj) return -1;
	if (nTreeType < 0 || nTreeType > 3) return -1;

	int iClass = lpObj->Class;

	for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
		if (lpObj->Magic[iCount].IsMagic() == TRUE && this->CheckMasterLevelSkill(lpObj->Magic[iCount].m_Skill) && (nTreeType == 0 || this->CheckSkillInTree(iClass, lpObj->Magic[iCount].m_Skill, nTreeType - 1))) {
			if (MagicDamageC.GetSkillUseType(lpObj->Magic[iCount].m_Skill) == 3) {
				lpObj->CharacterData->MasterPoint += lpObj->Magic[iCount].m_Level;
				lpObj->Magic[iCount].Clear();
			} else {
				int iBrandOfSkill = MagicDamageC.GetBrandOfSkill(lpObj->Magic[iCount].m_Skill);

				if (iBrandOfSkill > 0) {
					if (this->CheckMasterLevelSkill(iBrandOfSkill)) {
						lpObj->CharacterData->MasterPoint += lpObj->Magic[iCount].m_Level;
						lpObj->Magic[iCount].Clear();
					} else {
						CItem* lpLeftItem = &lpObj->pInventory[1];
						CItem* lpRightItem = &lpObj->pInventory[0];

						if (lpLeftItem->m_Special[0] == iBrandOfSkill) {
							lpObj->CharacterData->MasterPoint += lpObj->Magic[iCount].m_Level;
							lpObj->Magic[iCount].UpdateMasterSkill(iBrandOfSkill, lpLeftItem->m_Level);
						} else if (lpRightItem->m_Special[0] == iBrandOfSkill) {
							lpObj->CharacterData->MasterPoint += lpObj->Magic[iCount].m_Level;
							lpObj->Magic[iCount].UpdateMasterSkill(iBrandOfSkill, lpRightItem->m_Level);
						} else {
							lpObj->CharacterData->MasterPoint += lpObj->Magic[iCount].m_Level;
							lpObj->Magic[iCount].UpdateMasterSkill(iBrandOfSkill, 0);
						}
					}
				} else {
					lpObj->CharacterData->MasterPoint += lpObj->Magic[iCount].m_Level;
					lpObj->Magic[iCount].Clear();
				}
			}
		}
	}

	g_Log.Add("[%s][%s] Master Skill Reset (TreeType: %d)", lpObj->AccountID, lpObj->Name, nTreeType);
	gObjCloseSet(lpObj->m_Index, 1);
	return FALSE;
}
BYTE CMasterLevelSkillTreeSystem::CheckMasterSkillPoint(int aIndex, int nTreeType) {
	LPOBJ lpObj = &gObj[aIndex];
	int nSkillCnt = 0;

	if (!lpObj) return -1;
	if (nTreeType < 0 || nTreeType > MAX_TREE_TYPE) return -1;

	int iClass = lpObj->Class;

	if (nTreeType == 0) return 1;
	for (int n = 0; n < MAX_MAGIC; n++) {
		if (lpObj->Magic[n].IsMagic() == TRUE && this->CheckMasterLevelSkill(lpObj->Magic[n].m_Skill) == true && nTreeType > 0) {
			if (this->CheckSkillInTree(iClass, lpObj->Magic[n].m_Skill, nTreeType - 1)) nSkillCnt++;
		}
	}

	return nSkillCnt > 0;
}
BYTE CMasterLevelSkillTreeSystem::CheckSkillInTree(int iClass, int iSkill, int iTreeKind) {
	for (int iRank = 0; iRank < MAX_MASTER_SKILL_RANK; iRank++) {
		for (int iPos = 0; iPos < MAX_SKILL_POS; iPos++) {
			if (this->m_MLSTable[iClass].iAbility[iTreeKind].iElement[iRank][iPos].m_dwSkillID == iSkill) return true;
		}
	}

	return false;
}
float CMasterLevelSkillTreeSystem::GetSkillAttackDamage(LPOBJ lpObj, int iSkill) {
	if (!lpObj) return 0.0;
	if (!this->CheckMasterLevelSkill(iSkill)) return 0.0;

	int nGroup = MagicDamageC.GetSkillGroup(iSkill);
	WORD nBrandOfSkill = iSkill;
	WORD iBrandOfSkill = 0;
	int nCnt = 0;
	BOOL bReturnAddDamage = FALSE;
	int iValueType;

	while (this->CheckMasterLevelSkill(nBrandOfSkill)) {
		iBrandOfSkill = nBrandOfSkill;

		if (MagicDamageC.GetSkillGroup(nBrandOfSkill) != nGroup) {
			iBrandOfSkill = 0;
			break;
		}

		iValueType = MagicDamageC.SkillGet(nBrandOfSkill);

		if (this->m_MLSValueTable[iValueType].iValueType == MLST_VALUE_DAMAGE && nBrandOfSkill != 520) {
			bReturnAddDamage = TRUE;
			break;
		}

		nBrandOfSkill = MagicDamageC.GetBrandOfSkill(nBrandOfSkill);
		nCnt++;

		if (nCnt > 2) {
			g_Log.Add("[GetSkillAttackDamage] fail!!! - %d", iSkill);
			iBrandOfSkill = 0;
			break;
		}
	}

	if (iBrandOfSkill > 0) {
		if (bReturnAddDamage == TRUE) {
			for (int n = 0; n < MAX_MAGIC; n++) {
				if (lpObj->Magic[n].IsMagic() == TRUE && lpObj->Magic[n].m_Skill == iBrandOfSkill) return this->ML_GetNodeValue(iValueType, lpObj->Magic[n].m_Level);
			}
		}
	}

	return 0.0;
}
int CMasterLevelSkillTreeSystem::ML_SKILL_CalcDistance(int x, int y, int x1, int y1) {
	if (x == x1 && y == y1) return FALSE;

	float tx = x - x1;
	float ty = y - y1;

	return sqrt(tx * tx + ty * ty);
}
int CMasterLevelSkillTreeSystem::MLS_GetAngle(int x, int y, int tx, int ty) {
	double diffX = x - tx;
	double diffY = y - ty;
	double rad = atan2(diffY, diffX);
	int angle = rad * 180 / 3.141592741012573 + 90;

	if (angle < 0) angle += 360;
	return angle;
}
void CMasterLevelSkillTreeSystem::MLS_SkillFrustrum(int aIndex, float fangle, float tx, float ty) {
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
void CMasterLevelSkillTreeSystem::MLS_SkillFrustrum3(int aIndex, BYTE bAngle, float tx1, float ty1, float tx2, float ty2) {
	LPOBJECTSTRUCT lpObj = (LPOBJECTSTRUCT)&gObj[aIndex];
	vec3_t p[4];
	Vector(-tx1, ty1, 0.f, p[0]);
	Vector(tx1, ty1, 0.f, p[1]);
	Vector(tx2, ty2, 0.f, p[2]);
	Vector(-tx2, ty2, 0.f, p[3]);
	vec3_t Angle;
	Vector(0.f, 0.f, bAngle * 360 / (BYTE)255, Angle);
	float Matrix[3][4];
	vec3_t vFrustrum[4];

	AngleMatrix(Angle, Matrix);

	for (int i = 0; i < 4; i++) {
		VectorRotate(p[i], Matrix, vFrustrum[i]);
		lpObj->fSkillFrustrumX[i] = (int)vFrustrum[i][0] + lpObj->X;
		lpObj->fSkillFrustrumY[i] = (int)vFrustrum[i][1] + lpObj->Y;
	}
}
bool CMasterLevelSkillTreeSystem::MLS_WeaponSkillDel(LPOBJ lpObj, WORD aSkill, BYTE Level) {
	for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
		if (lpObj->Magic[iCount].IsMagic() == TRUE) {
			if (this->CheckMasterLevelSkill(lpObj->Magic[iCount].m_Skill)) {
				int iBaseMLS = this->GetBaseMasterLevelSkill(lpObj->Magic[iCount].m_Skill);

				if (iBaseMLS > 0) {
					int BrandSkill = MagicDamageC.GetNormalBrandOfSkill(iBaseMLS);

					if (aSkill == BrandSkill) {
						if (MagicDamageC.IsItemAttachedSkill(BrandSkill) && this->IsExistMLSBasedSkill(lpObj, iBaseMLS) == false) {
							GSProtocol.GCMagicListOneDelSend(lpObj->m_Index, iCount, lpObj->Magic[iCount].m_Skill, lpObj->Magic[iCount].m_Level, 0, 0);
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}
bool CMasterLevelSkillTreeSystem::MLS_WeaponSkillAdd(LPOBJ lpObj, WORD aSkill, BYTE Level) {
	for (int iCount = 0; iCount < MAX_MAGIC; iCount++) {
		if (lpObj->Magic[iCount].IsMagic() == TRUE) {
			if (this->CheckMasterLevelSkill(lpObj->Magic[iCount].m_Skill)) {
				int iBaseMLS = this->GetBaseMasterLevelSkill(lpObj->Magic[iCount].m_Skill);

				if (iBaseMLS > 0) {
					if (aSkill == MagicDamageC.GetNormalBrandOfSkill(iBaseMLS) && !this->IsExistMLSBasedSkill(lpObj, iBaseMLS)) {
						GSProtocol.GCMagicListOneSend(lpObj->m_Index, iCount, lpObj->Magic[iCount].m_Skill, lpObj->Magic[iCount].m_Level, 0, 0);
						return true;
					}
				}
			}
		}
	}

	return false;
}
BOOL CMasterLevelSkillTreeSystem::CheckUsableWeaponSkill(LPOBJ lpObj, int nCount) {
	if (!this->CheckMasterLevelSkill(lpObj->Magic[nCount].m_Skill)) return TRUE;

	int iBaseMLS = this->GetBaseMasterLevelSkill(lpObj->Magic[nCount].m_Skill);

	if (iBaseMLS <= 0) return TRUE;
	if (MagicDamageC.GetSkillUseType(iBaseMLS) != 4) return TRUE;

	int BrandOfSkill = MagicDamageC.GetNormalBrandOfSkill(iBaseMLS);

	if (MagicDamageC.IsItemAttachedSkill(lpObj->Magic[nCount].m_Skill) == false) return TRUE;
	if (lpObj->pInventory[1].IsItem() == TRUE && lpObj->pInventory[1].m_Special[0] == BrandOfSkill) return TRUE;
	if (lpObj->pInventory[0].IsItem() == TRUE && lpObj->pInventory[0].m_Special[0] == BrandOfSkill) return TRUE;
	return FALSE;
}

// Dark Knight
void CMasterLevelSkillTreeSystem::ML_DK_TornadoSlash(LPOBJ lpObj, CMagicInf* lpMagic, int aTargetIndex, BOOL isCombo) {
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (gObjIsConnected(lpObj->m_Index) == FALSE && gObjIsConnected(lpTargetObj->m_Index) == FALSE) return;
	if (lpObj->Type != OBJ_USER || lpObj->Class != CLASS_KNIGHT) return;

	gObjAttack(lpObj, lpTargetObj, lpMagic, 0, 1, 0, isCombo, 0, 0);
}
void CMasterLevelSkillTreeSystem::ML_DK_SteppingSlash(LPOBJ lpObj, CMagicInf* lpMagic, int aTargetIndex, BOOL isCombo) {
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (gObjIsConnected(lpObj->m_Index) == FALSE && gObjIsConnected(lpTargetObj->m_Index) == FALSE) return;
	if (lpObj->Type != OBJ_USER || lpObj->Class != CLASS_KNIGHT) return;

	gObjAttack(lpObj, lpTargetObj, lpMagic, 0, 1, 0, isCombo, 0, 0);
}
void CMasterLevelSkillTreeSystem::ML_DK_LeapingSlash(LPOBJ lpObj, CMagicInf* lpMagic, int aTargetIndex, BOOL isCombo) {
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (gObjIsConnected(lpObj->m_Index) == FALSE && gObjIsConnected(lpTargetObj->m_Index) == FALSE) return;
	if (lpObj->Type != OBJ_USER || lpObj->Class != CLASS_KNIGHT) return;

	gObjAttack(lpObj, lpTargetObj, lpMagic, 0, 1, 0, isCombo, 0, 0);
}
void CMasterLevelSkillTreeSystem::ML_DK_Stab(LPOBJ lpObj, CMagicInf* lpMagic, int aTargetIndex, BOOL isCombo) {
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (gObjIsConnected(lpObj->m_Index) == FALSE && gObjIsConnected(lpTargetObj->m_Index) == FALSE) return;
	if (lpObj->Type != OBJ_USER || lpObj->Class != CLASS_KNIGHT) return;

	gObjAttack(lpObj, lpTargetObj, lpMagic, 0, 1, 0, isCombo, 0, 0);
}
void CMasterLevelSkillTreeSystem::ML_DK_ImprovedTwistingSlash(LPOBJ lpObj, CMagicInf* lpMagic, int aTargetIndex, BOOL isCombo) {
	int count = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int tObjNum;
	int range = MagicDamageC.GetSkillDistance(lpMagic->m_Skill);
	BOOL EnableAttack;

	if (this->ML_GetNodeLevel(lpObj, 330) == 20 || this->ML_GetNodeLevel(lpObj, 481) == 20) range += 1;
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
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack == TRUE) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) <= range) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, FALSE, TRUE, FALSE, isCombo, FALSE, FALSE);
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}
void CMasterLevelSkillTreeSystem::ML_DK_ImprovedLifeSwell(LPOBJ lpObj, CMagicInf* lpMagic) {
	if (lpObj->Type != OBJ_USER && lpObj->m_RecallMon == -1) return;
	if (lpObj->Class != CLASS_KNIGHT && lpObj->Class != CLASS_MAGICGLADIATOR) return;

	int number;
	int partynum = 0;
	int totallevel = 0;
	int partycount;
	int dis;
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
		if (viewplayer == 2) viewpercent = 3;
		else if (viewplayer == 3) viewpercent = 6;
		else if (viewplayer == 4) viewpercent = 9;
		else viewpercent = 12;
	}

	double fAddValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
	double faddLifepower = 0.0;
	int iLifeTime = 0;

	this->m_Lua.Generic_Call("ImprovedLifeSwell", "iii>di", lpObj->CharacterData->Vitality + lpObj->AddVitality, lpObj->CharacterData->Energy + lpObj->AddEnergy, viewpercent, &faddLifepower, &iLifeTime);
	faddLifepower += fAddValue;

	if (partynum == -1) {
		if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_HP_INC)) BUFF_RemoveBuffEffect(lpObj, BUFFTYPE_HP_INC);

		BUFF_AddBuffEffect(lpObj, BUFFTYPE_HP_INC, EFFECTTYPE_INCREASE_LIFE, ((lpObj->MaxLife + lpObj->AddLife) * faddLifepower / 100.0), 0, 0, iLifeTime);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, TRUE);
	} else {
		for (int i = 0; i < MAX_USER_IN_PARTY; i++) {
			if (ApplyPartyIndex[i] != -1) {
				if (BUFF_TargetHasBuff(&gObj[ApplyPartyIndex[i]], BUFFTYPE_HP_INC)) BUFF_RemoveBuffEffect(&gObj[ApplyPartyIndex[i]], BUFFTYPE_HP_INC);

				BUFF_AddBuffEffect(&gObj[ApplyPartyIndex[i]], BUFFTYPE_HP_INC, EFFECTTYPE_INCREASE_LIFE, ((gObj[ApplyPartyIndex[i]].MaxLife + gObj[ApplyPartyIndex[i]].AddLife) * faddLifepower / 100.0), 0, 0, iLifeTime);
				GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, ApplyPartyIndex[i], TRUE);
			}
		}
	}
}
void CMasterLevelSkillTreeSystem::ML_DK_Innervation(LPOBJ lpObj, CMagicInf* lpMagic) {
	if (lpObj->Type != OBJ_USER && lpObj->m_RecallMon == -1) return;
	if (lpObj->Class != CLASS_KNIGHT && lpObj->Class != CLASS_MAGICGLADIATOR) return;

	int number;
	int partynum = 0;
	int totallevel = 0;
	int partycount;
	int dis;
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
		if (viewplayer == 2) viewpercent = 4;
		else if (viewplayer == 3) viewpercent = 8;
		else if (viewplayer == 4) viewpercent = 12;
		else viewpercent = 16;
	}

	double faddLifepower = 0.0;
	int iLifeTime = 0;
	int iPreSkill = this->GetParentSkill1(lpObj->Class, lpMagic->m_Skill);

	if (iPreSkill <= 0) this->m_Lua.Generic_Call("Innervation", "iii>di", lpObj->CharacterData->Vitality + lpObj->AddVitality, lpObj->CharacterData->Energy + lpObj->AddEnergy, viewpercent, &faddLifepower, &iLifeTime);
	else {
		int iPreLevel = this->ML_GetNodeLevel(lpObj, iPreSkill);

		if (iPreLevel > 0) {
			this->m_Lua.Generic_Call("Innervation", "iii>di", lpObj->CharacterData->Vitality + lpObj->AddVitality, lpObj->CharacterData->Energy + lpObj->AddEnergy, viewpercent, &faddLifepower, &iLifeTime);
			faddLifepower += this->ML_GetNodeValue(MagicDamageC.SkillGet(iPreSkill), iPreLevel);
		}
	}

	double faddManapower = 0.0;

	if (lpMagic->m_Skill == 360) faddManapower = viewpercent + this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
	if (partynum == -1) {
		if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_HP_INC_STR)) BUFF_RemoveBuffEffect(lpObj, BUFFTYPE_HP_INC_STR);

		BUFF_AddBuffEffect(lpObj, BUFFTYPE_HP_INC_STR, EFFECTTYPE_INCREASE_LIFE, ((lpObj->MaxLife + lpObj->AddLife) * faddLifepower / 100.0), EFFECTTYPE_INCREASE_MANA, ((lpObj->MaxMana + lpObj->AddMana) * faddManapower / 100.0), iLifeTime);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, TRUE);
	} else {
		for (int i = 0; i < MAX_USER_IN_PARTY; i++) {
			if (ApplyPartyIndex[i] != -1) {
				if (BUFF_TargetHasBuff(&gObj[ApplyPartyIndex[i]], BUFFTYPE_HP_INC_STR)) BUFF_RemoveBuffEffect(&gObj[ApplyPartyIndex[i]], BUFFTYPE_HP_INC_STR);

				BUFF_AddBuffEffect(&gObj[ApplyPartyIndex[i]], BUFFTYPE_HP_INC_STR, EFFECTTYPE_INCREASE_LIFE, ((gObj[ApplyPartyIndex[i]].MaxLife + gObj[ApplyPartyIndex[i]].AddLife) * faddLifepower / 100.0), EFFECTTYPE_INCREASE_MANA, ((gObj[ApplyPartyIndex[i]].MaxMana + gObj[ApplyPartyIndex[i]].AddMana) * faddManapower / 100.0), iLifeTime);
				GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, ApplyPartyIndex[i], TRUE);
			}
		}
	}
}
void CMasterLevelSkillTreeSystem::ML_DK_Invigoration(LPOBJ lpObj, CMagicInf* lpMagic) {
	if (lpObj->Type != OBJ_USER && lpObj->m_RecallMon == -1) return;
	if (lpObj->Class != CLASS_KNIGHT && lpObj->Class != CLASS_MAGICGLADIATOR) return;

	int number;
	int partynum = 0;
	int totallevel = 0;
	int partycount;
	int dis;
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
		if (viewplayer == 2) viewpercent = 5;
		else if (viewplayer == 3) viewpercent = 10;
		else if (viewplayer == 4) viewpercent = 15;
		else viewpercent = 20;
	}

	double faddLifepower = 0.0;
	double faddManapower = 0.0;
	int iLifeTime = 0;
	int iPreSkill = this->GetParentSkill1(lpObj->Class, lpMagic->m_Skill);

	if (iPreSkill <= 0) faddManapower = viewpercent;
	else {
		int iPrePreSkill = this->GetParentSkill1(lpObj->Class, iPreSkill);

		if (iPrePreSkill <= 0) this->m_Lua.Generic_Call("Invigoration", "iii>di", lpObj->CharacterData->Vitality + lpObj->AddVitality, lpObj->CharacterData->Energy + lpObj->AddEnergy, viewpercent, &faddLifepower, &iLifeTime);
		else {
			int iPrePreLevel = this->ML_GetNodeLevel(lpObj, iPrePreSkill);

			if (iPrePreLevel > 0) {
				this->m_Lua.Generic_Call("Invigoration", "iii>di", lpObj->CharacterData->Vitality + lpObj->AddVitality, lpObj->CharacterData->Energy + lpObj->AddEnergy, viewpercent, &faddLifepower, &iLifeTime);
				faddLifepower += this->ML_GetNodeValue(MagicDamageC.SkillGet(iPrePreSkill), iPrePreLevel);
			} else this->m_Lua.Generic_Call("Invigoration", "iii>di", lpObj->CharacterData->Vitality + lpObj->AddVitality, lpObj->CharacterData->Energy + lpObj->AddEnergy, viewpercent, &faddLifepower, &iLifeTime);
		}

		int iPreLevel = this->ML_GetNodeLevel(lpObj, iPreSkill);

		if (iPreLevel > 0) faddManapower = viewpercent + this->ML_GetNodeValue(MagicDamageC.SkillGet(iPreSkill), iPreLevel);
	}

	if (partynum == -1) {
		if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_HP_INC_STR)) BUFF_RemoveBuffEffect(lpObj, BUFFTYPE_HP_INC_STR);

		lpObj->CharacterData->Mastery.ML_DK_Innervation = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
		BUFF_AddBuffEffect(lpObj, BUFFTYPE_HP_INC_STR, EFFECTTYPE_INCREASE_LIFE, ((lpObj->MaxLife + lpObj->AddLife) * faddLifepower / 100.0), EFFECTTYPE_INCREASE_MANA, ((lpObj->MaxMana + lpObj->AddMana) * faddManapower / 100.0), iLifeTime);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, TRUE);
	} else {
		for (int i = 0; i < MAX_USER_IN_PARTY; i++) {
			if (ApplyPartyIndex[i] != -1) {
				if (BUFF_TargetHasBuff(&gObj[ApplyPartyIndex[i]], BUFFTYPE_HP_INC_STR)) BUFF_RemoveBuffEffect(&gObj[ApplyPartyIndex[i]], BUFFTYPE_HP_INC_STR);

				gObj[ApplyPartyIndex[i]].CharacterData->Mastery.ML_DK_Innervation = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
				BUFF_AddBuffEffect(&gObj[ApplyPartyIndex[i]], BUFFTYPE_HP_INC_STR, EFFECTTYPE_INCREASE_LIFE, ((gObj[ApplyPartyIndex[i]].MaxLife + gObj[ApplyPartyIndex[i]].AddLife) * faddLifepower / 100.0), EFFECTTYPE_INCREASE_MANA, ((gObj[ApplyPartyIndex[i]].MaxMana + gObj[ApplyPartyIndex[i]].AddMana) * faddManapower / 100.0), iLifeTime);
				GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, ApplyPartyIndex[i], TRUE);
			}
		}
	}
}
void CMasterLevelSkillTreeSystem::ML_DK_ImprovedDeathStab(int aIndex, int aTargetIndex, CMagicInf* lpMagic, BOOL isCombo) {
	LPOBJ lpObj = &gObj[aIndex];
	float fangle = this->MLS_GetAngle(gObj[aIndex].X, gObj[aIndex].Y, gObj[aTargetIndex].X, gObj[aTargetIndex].Y);
	float fDistance = MagicDamageC.GetSkillDistance(lpMagic->m_Skill) + 1.0;

	this->MLS_SkillFrustrum(aIndex, fangle, 1.5, fDistance);

	int count = 0;
	int loopcount = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int tObjNum;
	BOOL EnableAttack;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);

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
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack == TRUE) {
					BOOL attackcheck = FALSE;

					if (loopcount) {
						if ((rand() % 3) == 0) attackcheck = TRUE;
					} else attackcheck = TRUE;
					if (attackcheck == TRUE) {
						if (SkillTestFrustrum(gObj[tObjNum].X, gObj[tObjNum].Y, aIndex)) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 0, TRUE, 0, isCombo, 0, 0);
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
}
void CMasterLevelSkillTreeSystem::ML_DK_PuncturingStab(int aIndex, int aTargetIndex, CMagicInf* lpMagic, BOOL isCombo) {
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	float fangle = this->MLS_GetAngle(gObj[aIndex].X, gObj[aIndex].Y, gObj[aTargetIndex].X, gObj[aTargetIndex].Y);
	float fDistance = MagicDamageC.GetSkillDistance(lpMagic->m_Skill) + 1.0;

	this->MLS_SkillFrustrum(aIndex, fangle, 1.5, fDistance);

	int count = 0;
	int loopcount = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int tObjNum;
	BOOL EnableAttack;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);

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
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (g_GensSystem.IsMapBattleZone(lpObj->MapNumber) == TRUE) EnableAttack = TRUE;
				if (gclassObjAttack.PkCheck(lpObj, lpTargetObj) == FALSE) EnableAttack = FALSE;
				if (EnableAttack == TRUE) {
					BOOL attackcheck = FALSE;

					if (loopcount) {
						if ((rand() % 3) == 0) attackcheck = TRUE;
					} else attackcheck = TRUE;

					if (attackcheck == TRUE) {
						if (SkillTestFrustrum(gObj[tObjNum].X, gObj[tObjNum].Y, aIndex)) {
							gObj[tObjNum].lpAttackObj = lpObj;
							gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 0, 1, 0, isCombo, 0, 0);
						}
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
}
void CMasterLevelSkillTreeSystem::ML_DK_DeathBlow(int aIndex, int aTargetIndex, CMagicInf* lpMagic, BOOL isCombo) {
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	float fangle = this->MLS_GetAngle(gObj[aIndex].X, gObj[aIndex].Y, gObj[aTargetIndex].X, gObj[aTargetIndex].Y);
	float fDistance = MagicDamageC.GetSkillDistance(lpMagic->m_Skill) + 1.0;

	this->MLS_SkillFrustrum(aIndex, fangle, 1.5, fDistance);

	int count = 0;
	int loopcount = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int tObjNum;
	BOOL EnableAttack;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);

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
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (g_GensSystem.IsMapBattleZone(lpObj->MapNumber) == TRUE) EnableAttack = TRUE;
				if (gclassObjAttack.PkCheck(lpObj, lpTargetObj) == FALSE) EnableAttack = FALSE;
				if (EnableAttack == TRUE) {
					BOOL attackcheck = FALSE;

					if (loopcount) {
						if ((rand() % 3) == 0) attackcheck = TRUE;
					} else attackcheck = TRUE;

					if (attackcheck == TRUE) {
						if (SkillTestFrustrum(gObj[tObjNum].X, gObj[tObjNum].Y, aIndex)) {
							gObj[tObjNum].lpAttackObj = lpObj;
							gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 0, 1, 0, isCombo, 0, 0);
						}
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
}
void CMasterLevelSkillTreeSystem::ML_DK_ImprovedRagefulBlow(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex, BOOL isCombo) {
	LPOBJ lpObj = &gObj[aIndex];
	int count = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int tObjNum;
	BOOL EnableAttack;

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
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack == TRUE) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) <= MagicDamageC.GetSkillDistance(lpMagic->m_Skill)) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 500, lpMagic->m_Skill, isCombo);
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}
void CMasterLevelSkillTreeSystem::ML_DK_ImprovedGroundslam(LPOBJ lpObj, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex, int isCombo) {
	int count = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int nHitCount = 0;
	BOOL bAttack = FALSE;
	BOOL EnableAttack;
	int tObjNum;

	if (this->ML_SKILL_CalcDistance(lpObj->X, lpObj->Y, x, y) <= MagicDamageC.GetSkillDistance(lpMagic->m_Skill)) {
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
						if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
						if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
					}

					if (EnableAttack == TRUE) {
						if (gObj[tObjNum].X >= x - 3 && gObj[tObjNum].X <= x + 3 && gObj[tObjNum].Y >= y - 3 && gObj[tObjNum].Y <= y + 3) {
							gObj[tObjNum].lpAttackObj = lpObj;
							nHitCount++;

							if (nHitCount <= 4) bAttack = TRUE;
							else if (rand() % 2) bAttack = TRUE;
							if (nHitCount > 8) bAttack = FALSE;
							if (bAttack == TRUE) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 500, lpMagic->m_Skill, 0);
						}
					}
				}
			}

			count++;

			if (count > MAX_VIEWPORT - 1) break;
		}
	}
}
void CMasterLevelSkillTreeSystem::ML_DK_EarthShake(OBJECTSTRUCT* lpObj, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex, BOOL isCombo) {
	int count = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int nHitCount = 0;
	BOOL bAttack = FALSE;
	BOOL EnableAttack;
	int tObjNum;

	if (this->ML_SKILL_CalcDistance(lpObj->X, lpObj->Y, x, y) <= MagicDamageC.GetSkillDistance(lpMagic->m_Skill)) {
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
						if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
						if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
					}

					if (EnableAttack == TRUE) {
						if (gObj[tObjNum].X >= x - 3 && gObj[tObjNum].X <= x + 3 && gObj[tObjNum].Y >= y - 3 && gObj[tObjNum].Y <= y + 3) {
							gObj[tObjNum].lpAttackObj = lpObj;
							nHitCount++;

							if (nHitCount <= 4) bAttack = TRUE;
							else if (rand() % 2) bAttack = TRUE;
							if (nHitCount > 8) bAttack = FALSE;
							if (bAttack == TRUE) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 500, lpMagic->m_Skill, 0);
						}
					}
				}
			}

			count++;

			if (count > MAX_VIEWPORT - 1) break;
		}
	}
}
void CMasterLevelSkillTreeSystem::ML_DK_ArcticBlow(OBJECTSTRUCT* lpObj, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex, BOOL isCombo) {
	int count = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int nHitCount = 0;
	BOOL bAttack = FALSE;
	BOOL EnableAttack;
	int tObjNum;

	if (this->ML_SKILL_CalcDistance(lpObj->X, lpObj->Y, x, y) <= MagicDamageC.GetSkillDistance(lpMagic->m_Skill)) {
		while (true) {
			if (lpObj->VpPlayer2[count].state != 0) {
				tObjNum = lpObj->VpPlayer2[count].number;

				if (tObjNum >= 0) {
					EnableAttack = FALSE;

					if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
					else if (tObjNum == aTargetIndex || DuelIndex == tObjNum)EnableAttack = TRUE;
					else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
					else {
						int CallMonIndex = gObj[tObjNum].m_Index;

						if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
						if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
						if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
					}

					if (EnableAttack == TRUE) {
						if (gObj[tObjNum].X >= x - 3 && gObj[tObjNum].X <= x + 3 && gObj[tObjNum].Y >= y - 3 && gObj[tObjNum].Y <= y + 3) {
							gObj[tObjNum].lpAttackObj = lpObj;
							nHitCount++;

							if (nHitCount <= 4) bAttack = TRUE;
							else if (rand() % 2) bAttack = TRUE;
							if (nHitCount > 8) bAttack = FALSE;
							if (bAttack == TRUE) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 500, lpMagic->m_Skill, 0);
						}
					}
				}
			}

			count++;

			if (count > MAX_VIEWPORT - 1) break;
		}
	}
}
void CMasterLevelSkillTreeSystem::ML_DK_BladeStorm(LPOBJ lpObj, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex, BOOL isCombo) {
	int DuelIndex = lpObj->m_iDuelUser;
	int nHitCount = 0;
	BOOL bAttack = FALSE;
	int tObjNum;
	int count = 0;
	BOOL EnableAttack = FALSE;
	int MLbonus = FALSE;

	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 346) == 20) MLbonus = TRUE;
	if (this->ML_SKILL_CalcDistance(lpObj->X, lpObj->Y, x, y) <= MagicDamageC.GetSkillDistance(lpMagic->m_Skill) + MLbonus) {
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
						if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
						if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
					}

					if (EnableAttack == TRUE) {
						if (gObj[tObjNum].X >= x - 2 && gObj[tObjNum].X <= x + 2 && gObj[tObjNum].Y >= y - 2 && gObj[tObjNum].Y <= y + 2) {
							gObj[tObjNum].lpAttackObj = lpObj;
							nHitCount++;

							if (nHitCount <= 4) bAttack = TRUE;
							else if (rand() % 2) bAttack = TRUE;
							if (nHitCount > 8) bAttack = FALSE;
							if (bAttack == TRUE) {
								gObjAddAttackProcMsgSendDelay(lpObj, 60, tObjNum, 1000, lpMagic->m_Skill, isCombo);
								gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 1500, lpMagic->m_Skill, 0);
							}
						}
					}
				}
			}

			count++;

			if (count > MAX_VIEWPORT - 1) break;
		}
	}
}

// Dark Wizard
void CMasterLevelSkillTreeSystem::ML_DW_ImprovedManaShield(LPOBJ lpObj, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpObj->Type != OBJ_USER) return;
	if (lpObj->m_RecallMon != -1) return;
	if (lpObj->Class != CLASS_WIZARD && lpObj->Class != CLASS_MAGICGLADIATOR) return;
	if (lpObj->PartyNumber != lpTargetObj->PartyNumber) return;

	double fEffectValue = 0.0;
	int iSkillTime = 0;

	this->m_Lua.Generic_Call("ImprovedManaShield", "ii>di", lpObj->CharacterData->Dexterity + lpObj->AddDexterity, lpObj->AddEnergy + lpObj->CharacterData->Energy, &fEffectValue, &iSkillTime);	

	float fSkillValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);

	fSkillValue += fEffectValue;
	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_MANA_SHIELD, EFFECTTYPE_MANA_SHIELD, fSkillValue, NULL, NULL, iSkillTime);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_DW_ManaBarrier(LPOBJ lpObj, CMagicInf* lpMagic, int aTargetIndex) {
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpObj->Type != OBJ_USER) return;
	if (lpObj->m_RecallMon != -1) return;
	if (lpObj->Class != CLASS_WIZARD && lpObj->Class != CLASS_MAGICGLADIATOR) return;
	if (lpObj->PartyNumber != lpTargetObj->PartyNumber) return;

	double fDefenseValue = 0.0;
	int iSkillTime = 0;
	int iPreSkill = this->GetParentSkill1(lpObj->Class, lpMagic->m_Skill);

	if (iPreSkill <= 0) this->m_Lua.Generic_Call("ManaBarrier", "ii>di", lpObj->AddDexterity + lpObj->CharacterData->Dexterity, lpObj->AddEnergy + lpObj->CharacterData->Energy, &fDefenseValue, &iSkillTime);
	else {
		int iPreLevel = this->ML_GetNodeLevel(lpObj, iPreSkill);

		if (iPreLevel > 0) {
			this->m_Lua.Generic_Call("ManaBarrier", "ii>di", lpObj->AddDexterity + lpObj->CharacterData->Dexterity, lpObj->AddEnergy + lpObj->CharacterData->Energy, &fDefenseValue, &iSkillTime);
			fDefenseValue += this->ML_GetNodeValue(MagicDamageC.SkillGet(iPreSkill), iPreLevel);
		}
	}

	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_MANA_SHIELD, EFFECTTYPE_MANA_SHIELD, fDefenseValue, NULL, NULL, iSkillTime + this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level));
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpTargetObj->m_Index, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_DW_SoulBarrier(LPOBJ lpObj, CMagicInf* lpMagic, int aTargetIndex) {
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpObj->Type != OBJ_USER) return;
	if (lpTargetObj->Type != OBJ_USER) return;
	if (lpObj->m_RecallMon != -1) return;
	if (lpObj->Class != CLASS_WIZARD && lpObj->Class != CLASS_MAGICGLADIATOR) return;
	if (lpObj->PartyNumber != lpTargetObj->PartyNumber) return;

	double fDefenseValue = 0.0;
	double fSkillValidTime = 0.0;

	this->m_Lua.Generic_Call("SoulBarrier", "ii>dd", lpObj->AddDexterity + lpObj->CharacterData->Dexterity, lpObj->AddEnergy + lpObj->CharacterData->Energy, &fDefenseValue, &fSkillValidTime);
	fDefenseValue += this->GetBrandOfMasterSkillValue(lpObj, lpMagic->m_Skill, 2);
	fSkillValidTime += this->GetBrandOfMasterSkillValue(lpObj, lpMagic->m_Skill, 1);
	lpTargetObj->CharacterData->Mastery.ML_DW_ImprovedManaShield = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_IMPROVED_MANA_SHIELD, EFFECTTYPE_MANA_SHIELD, fDefenseValue, NULL, NULL, fSkillValidTime);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_DW_ImprovedLightning(OBJECTSTRUCT* lpObj, CMagicInf* lpMagic, int aTargetIndex) {
	if (lpObj->Type != OBJ_USER) return;

	gObjAttack(lpObj, &gObj[aTargetIndex], lpMagic, TRUE, 0, 0, 0, 0, 0);
}
void CMasterLevelSkillTreeSystem::ML_DW_ImprovedPoison(OBJECTSTRUCT* lpObj, CMagicInf* lpMagic, int aTargetIndex) {
	if (lpObj->Type != OBJ_USER) return;

	gObjAttack(lpObj, &gObj[aTargetIndex], lpMagic, TRUE, 0, 0, 0, 0, 0);
}
void CMasterLevelSkillTreeSystem::ML_DW_ImprovedIce(OBJECTSTRUCT* lpObj, CMagicInf* lpMagic, int aTargetIndex) {
	if (lpObj->Type != OBJ_USER) return;

	gObjAttack(lpObj, &gObj[aTargetIndex], lpMagic, TRUE, 0, 0, 0, 0, 0);
}
void CMasterLevelSkillTreeSystem::ML_DW_ImprovedFlame(OBJECTSTRUCT* lpObj, CMagicInf* lpMagic, int aTargetIndex) {
	if (lpObj->Type != OBJ_USER) return;

	gObjAttack(lpObj, &gObj[aTargetIndex], lpMagic, TRUE, 0, 0, 0, 0, 0);
}
void CMasterLevelSkillTreeSystem::ML_DW_ImprovedHellfire(OBJECTSTRUCT* lpObj, CMagicInf* lpMagic) {
	int count = 0;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, TRUE);

	while (true) {
		if (lpObj->VpPlayer2[count].state) {
			if (lpObj->VpPlayer2[count].type == OBJ_MONSTER) {
				int tObjNum = lpObj->VpPlayer2[count].number;

				if (tObjNum >= 0) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) < 4) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 200, lpMagic->m_Skill, 0);
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}
void CMasterLevelSkillTreeSystem::ML_DW_ArcaneBlast(LPOBJ lpObj, CMagicInf* lpMagic, int aTargetIndex) {
	if (lpObj->Type = OBJ_USER) return;

	gObjAttack(lpObj, &gObj[aTargetIndex], lpMagic, TRUE, 0, 0, 0, 0, 0);
}
void CMasterLevelSkillTreeSystem::ML_DW_SearingFlames(LPOBJ lpObj, CMagicInf* lpMagic) {
	return;
}
void CMasterLevelSkillTreeSystem::ML_DW_Rot(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJ_USER) return;

	int count = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int tObjNum;
	BOOL EnableAttack;

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
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack == TRUE) {
					if (this->ML_SKILL_CalcDistance(x, y, gObj[tObjNum].X, gObj[tObjNum].Y) < 3) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, rand() % 500, lpMagic->m_Skill, 0);
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}
void CMasterLevelSkillTreeSystem::ML_DW_ImprovedIceStorm(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int count = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int tObjNum;
	BOOL EnableAttack;

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
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack == TRUE) {
					if (this->ML_SKILL_CalcDistance(x, y, gObj[tObjNum].X, gObj[tObjNum].Y) < 3) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 1, 0, 0, 0, 0);
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}
void CMasterLevelSkillTreeSystem::ML_DW_Blizzard(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int count = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int tObjNum;
	int rangeBonus = FALSE;
	BOOL EnableAttack;

	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, lpMagic->m_Skill) >= 10) rangeBonus = TRUE;
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
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack == TRUE) {
					if (this->ML_SKILL_CalcDistance(x, y, gObj[tObjNum].X, gObj[tObjNum].Y) < 3 + rangeBonus) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 1, 0, 0, 0, 0);
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}
void CMasterLevelSkillTreeSystem::ML_DW_Supernova(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (aTargetIndex == 395) {
		this->ML_DW_SupernovaStart(aIndex, lpMagic);
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->CharacterData->SkillStrengthenHellFire2State == 0) {
		g_Log.Add("[%s][%s] [MLS] Nova skill didn't cast", lpObj->AccountID, lpObj->Name); //HermeX Fix
		return;
	}

	lpObj->CharacterData->SkillStrengthenHellFire2State = 0;
	lpObj->CharacterData->SkillStrengthenHellFire2Time = 0;

	int count = 0;
	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	int DuelIndex = lpObj->m_iDuelUser;
	BOOL EnableAttack = FALSE;
	int tObjNum;

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = 0;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = 1;
				else if (aTargetIndex == tObjNum || DuelIndex == tObjNum) EnableAttack = 1;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = 1;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack != 0) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) <= MagicDamageC.GetSkillDistance(lpMagic->m_Skill)) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 600, lpMagic->m_Skill, 0);
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_DW_SupernovaStart(int aIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJ_USER) return;
	if (lpObj->CharacterData->SkillStrengthenHellFire2State != 0) {
		g_Log.Add("[%s][%s] [MLS] Nova skill already casted", lpObj->AccountID, lpObj->Name);
		return;
	}

	lpObj->CharacterData->SkillStrengthenHellFire2Time = GetTickCount();
	lpObj->CharacterData->SkillStrengthenHellFire2State = 1;
	lpObj->CharacterData->SkillStrengthenHellFire2Count = 0;
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, 395, aIndex, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_DW_ImprovedArcanePower(LPOBJ lpObj, CMagicInf* lpMagic, int aTargetIndex) {
	if (lpObj->Type != OBJ_USER) return;
	if (lpObj->Class != CLASS_WIZARD) return;

	BUFF_AddBuffEffect(lpObj, BUFFTYPE_IMPROVED_ARCANE_POWER, EFFECTTYPE_INCREASE_MINIMUM_SPELL_DAMAGE, ((lpObj->CharacterData->Energy + lpObj->AddEnergy) / 6.0 * 0.20), EFFECTTYPE_INCREASE_MAXIMUM_SPELL_DAMAGE, (lpObj->m_MagicDamageMax / 100.0 * this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level)), 1800);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_DW_ArcaneFocus(LPOBJ lpObj, CMagicInf* lpMagic, int aTargetIndex) {
	if (lpObj->Type != OBJ_USER) return;
	if (lpObj->Class != CLASS_WIZARD) return;

	lpObj->CharacterData->Mastery.ML_DW_ArcaneFocus = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
	lpObj->CharacterData->Mastery.ML_ExcellentStrike = 5;
	BUFF_AddBuffEffect(lpObj, BUFFTYPE_ARCANE_FOCUS, EFFECTTYPE_INCREASE_MINIMUM_SPELL_DAMAGE, ((lpObj->CharacterData->Energy + lpObj->AddEnergy) / 6.0 * 0.20), EFFECTTYPE_INCREASE_MAXIMUM_SPELL_DAMAGE, (lpObj->m_MagicDamageMax / 100.0 * this->GetBrandOfMasterSkillValue(lpObj, lpMagic->m_Skill, 1)), 1800);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, TRUE);

	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 383) >= 10) gObjCalCharacter.CHARACTER_Calc(lpObj->m_Index);
}
void CMasterLevelSkillTreeSystem::ML_DW_EarthenPrison(LPOBJ lpObj, int aTargetIndex, CMagicInf* lpMagic) {
	int nHitCount = 0;
	BOOL bAttack = FALSE;
	int DuelIndex = lpObj->m_iDuelUser;
	int count = 0;
	int tObjNum;
	BOOL EnableAttack = FALSE;

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (gObj[tObjNum].Class >= 100 && gObj[tObjNum].Class <= 110) EnableAttack = FALSE;
				if (gObj[tObjNum].Class == 689) EnableAttack = FALSE;
				else if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack == TRUE) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) <= MagicDamageC.GetSkillDistance(lpMagic->m_Skill)) {
						bAttack = TRUE;
						nHitCount++;

						if (nHitCount > 15) bAttack = FALSE;
						if (bAttack == TRUE) {
							gObjAddAttackProcMsgSendDelay(lpObj, 60, tObjNum, 500, lpMagic->m_Skill, 0);
							gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 800, lpMagic->m_Skill, 0);
						}
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}

// Elf
void CMasterLevelSkillTreeSystem::ML_ELF_ImprovedHeal(LPOBJ lpObj, int aTargetIndex, CMagicInf* lpMagic) {
	BOOL isRecast = FALSE;
	recast:
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpTargetObj->Type != OBJ_USER && lpTargetObj->m_RecallMon == -1) return;
	if (lpObj->Class != CLASS_ELF) return;
	if ((CC_MAP_RANGE(lpObj->MapNumber) || lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) && lpObj->m_Index != lpTargetObj->m_Index) return;

	double fAddLife = 0.0;
	this->m_Lua.Generic_Call("ImprovedHeal", "i>d", lpObj->CharacterData->Energy + lpObj->AddEnergy, &fAddLife);
	float fSkillValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);

	if (fSkillValue > 0.0) fAddLife = ((fAddLife * fSkillValue) / 100.0) + fAddLife;
	if (isRecast) fAddLife /= 2;

	lpTargetObj->Life += fAddLife;

	if ((lpTargetObj->MaxLife + lpTargetObj->AddLife) < lpTargetObj->Life) lpTargetObj->Life = lpTargetObj->MaxLife + lpTargetObj->AddLife;
	if (lpTargetObj->Type == OBJ_USER) {
		GSProtocol.PROTOCOL_ReFillSend(lpTargetObj->m_Index, lpTargetObj->Life, 0xFF, 0, lpTargetObj->iShield);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);

		for (int n = 0; n < MAX_VIEWPORT; n++) {
			if (lpObj->VpPlayer2[n].type == OBJ_MONSTER && lpObj->VpPlayer2[n].state != 0) {
				LPOBJ lpMonster = &gObj[lpObj->VpPlayer2[n].number];

				if (lpMonster->m_iCurrentAI != 0) lpMonster->m_Agro->IncAgro(lpObj->m_Index,(lpObj->CharacterData->Energy / 5) / 40);
			}
		}
	} else if (lpTargetObj->m_RecallMon >= 0) {
		GSProtocol.GCRecallMonLife(lpTargetObj->m_RecallMon, lpTargetObj->MaxLife, lpTargetObj->Life);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
	}

	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, lpMagic->m_Skill) >= 10 && CHARACTER_DebuffProc(5)) BUFF_RemoveDebuffEffect(lpTargetObj);
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, lpMagic->m_Skill) == 20 && CHARACTER_DebuffProc(10)) {
		isRecast = TRUE;
		goto recast;
	}
}
void CMasterLevelSkillTreeSystem::ML_ELF_ImprovedEnhance(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpTargetObj->Type != OBJ_USER && lpTargetObj->m_RecallMon == -1) return;
	if (lpObj->Class != CLASS_ELF) return;
	if ((CC_MAP_RANGE(lpObj->MapNumber) || lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) && lpObj->m_Index != lpTargetObj->m_Index) return;

	double skill_defense = 0.0;
	double skill_time = 0.0;

	this->m_Lua.Generic_Call("ImprovedEnhance", "i>dd", lpObj->CharacterData->Energy + lpObj->AddEnergy, &skill_defense, &skill_time);
	skill_defense += skill_defense * this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level) / 100.0;

	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, lpMagic->m_Skill) == 20) BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_ENHANCE, EFFECTTYPE_INCREASE_DEFENSE, skill_defense, EFFECTTYPE_INCREASE_STUN_RESISTANCE, 0, skill_time);
	else BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_ENHANCE, EFFECTTYPE_INCREASE_DEFENSE, skill_defense, 0, 0, skill_time);

	for (int n = 0; n < MAX_VIEWPORT; n++) {
		if (lpObj->VpPlayer2[n].type == OBJ_MONSTER && lpObj->VpPlayer2[n].state != 0) {
			LPOBJ lpMonster = &gObj[lpObj->VpPlayer2[n].number];

			if (lpMonster->m_iCurrentAI != 0) lpMonster->m_Agro->IncAgro(lpObj->m_Index, int(skill_defense) / 10);
		}
	}

	gObjCalCharacter.CHARACTER_Calc(aTargetIndex);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_ELF_SpiritWard(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpTargetObj->Type != OBJ_USER && lpTargetObj->m_RecallMon == -1) return;
	if (lpObj->Class != CLASS_ELF) return;
	if ((CC_MAP_RANGE(lpObj->MapNumber) || lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) && lpObj->m_Index != lpTargetObj->m_Index) return;

	float aMLSkillValue = 0.0;
	int iPreSkill = this->GetParentSkill1(lpObj->Class, lpMagic->m_Skill);

	if (iPreSkill > 0) {
		int iPreLevel = this->ML_GetNodeLevel(lpObj, iPreSkill);

		if (iPreLevel > 0) aMLSkillValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(iPreSkill), iPreLevel);
	}

	int effectIndex = 0, effectValue = 0;
	float fValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
	double skill_defense = 0.0;
	double skill_time = 0.0;

	this->m_Lua.Generic_Call("SpiritWard", "i>dd", lpObj->CharacterData->Energy + lpObj->AddEnergy, &skill_defense, &skill_time);
	skill_defense += skill_defense * (aMLSkillValue + fValue) / 100.0;
	skill_time += fValue;

	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 417) == 20) effectIndex = EFFECTTYPE_INCREASE_STUN_RESISTANCE;
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, lpMagic->m_Skill) == 20) effectValue = 5;

	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_ENHANCE, EFFECTTYPE_INCREASE_DEFENSE, skill_defense, effectIndex, effectValue, skill_time);

	for (int n = 0; n < MAX_VIEWPORT; n++) {
		if (lpObj->VpPlayer2[n].type == OBJ_MONSTER && lpObj->VpPlayer2[n].state != 0) {
			LPOBJ lpMonster = &gObj[lpObj->VpPlayer2[n].number];

			if (lpMonster->m_iCurrentAI != 0) lpMonster->m_Agro->IncAgro(lpObj->m_Index, int(skill_defense) / 10);
		}
	}

	gObjCalCharacter.CHARACTER_Calc(aTargetIndex);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_ELF_ImprovedEmpower(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpTargetObj->Type != OBJ_USER && lpTargetObj->m_RecallMon == -1) return;
	if (lpObj->Class != CLASS_ELF) return;
	if ((CC_MAP_RANGE(lpObj->MapNumber) || lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) && lpObj->m_Index != lpTargetObj->m_Index) return;

	int effectIndex = 0;
	double skill_attack = 0.0;
	double skill_time = 0.0;

	this->m_Lua.Generic_Call("ImprovedEmpower", "i>dd", lpObj->CharacterData->Energy + lpObj->AddEnergy, &skill_attack, &skill_time);
	skill_attack += skill_attack * this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level) / 100.0;

	if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_AMPLIFY_DAMAGE)) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	} else if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_IMPROVED_AMPLIFY_DAMAGE)) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	} else if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_DIMENSIONAL_AFFINITY)) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	} else if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_VOID_AFFINITY)) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	}

	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, lpMagic->m_Skill) == 20) effectIndex = EFFECTTYPE_INCREASE_ATTACK_SPEED_PERCENT;

	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_EMPOWER, EFFECTTYPE_INCREASE_DAMAGE, skill_attack, effectIndex, 0, skill_time);

	for (int n = 0; n < MAX_VIEWPORT; n++) {
		if (lpObj->VpPlayer2[n].type == OBJ_MONSTER && lpObj->VpPlayer2[n].state != 0) {
			LPOBJ lpMonster = &gObj[lpObj->VpPlayer2[n].number];

			if (lpMonster->m_iCurrentAI != 0) lpMonster->m_Agro->IncAgro(lpObj->m_Index, int(skill_attack) / 10);
		}
	}

	gObjCalCharacter.CHARACTER_Calc(aTargetIndex);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_ELF_Berserk(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpTargetObj->Type != OBJ_USER && lpTargetObj->m_RecallMon == -1) return;
	if (lpObj->Class != CLASS_ELF) return;
	if ((CC_MAP_RANGE(lpObj->MapNumber) || lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) && lpObj->m_Index != lpTargetObj->m_Index) return;

	float aMLSkillValue = 0.0;
	int iPreSkill = this->GetParentSkill1(lpObj->Class, lpMagic->m_Skill);

	if (iPreSkill > 0) {
		int iPreLevel = this->ML_GetNodeLevel(lpObj, iPreSkill);

		if (iPreLevel > 0) aMLSkillValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(iPreSkill), iPreLevel);
	}

	int effectIndex = 0, effectValue = 0;
	float fValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
	double skill_attack = 0.0;
	double skill_time = 0.0;

	this->m_Lua.Generic_Call("Berserk", "i>dd", lpObj->CharacterData->Energy + lpObj->AddEnergy, &skill_attack, &skill_time);
	skill_attack += skill_attack * (aMLSkillValue + fValue) / 100.0;

	if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_AMPLIFY_DAMAGE)) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	} else if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_IMPROVED_AMPLIFY_DAMAGE)) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	} else if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_DIMENSIONAL_AFFINITY)) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	} else if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_VOID_AFFINITY)) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	}

	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, lpMagic->m_Skill) == 20) effectValue = 3;
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 420) == 20) effectIndex = EFFECTTYPE_INCREASE_ATTACK_SPEED_PERCENT;

	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_EMPOWER, EFFECTTYPE_INCREASE_DAMAGE, skill_attack, effectIndex, effectValue, skill_time + fValue);

	for (int n = 0; n < MAX_VIEWPORT; n++) {
		if (lpObj->VpPlayer2[n].type == OBJ_MONSTER && lpObj->VpPlayer2[n].state != 0) {
			LPOBJ lpMonster = &gObj[lpObj->VpPlayer2[n].number];

			if (lpMonster->m_iCurrentAI != 0) lpMonster->m_Agro->IncAgro(lpObj->m_Index, int(skill_attack) / 10);
		}
	}

	gObjCalCharacter.CHARACTER_Calc(aTargetIndex);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_ELF_Cure(LPOBJ lpObj, CMagicInf* lpMagic, int aTargetIndex) {
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (gObjIsConnected(lpObj->m_Index) == FALSE && gObjIsConnected(lpTargetObj->m_Index) == FALSE) return;
	if (lpObj->Type != OBJ_USER) return;

	BUFF_RemoveDebuffEffect(lpTargetObj);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_ELF_PrayerOfHealing(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return;
	if (ObjectMaxRange(aTargetIndex) == false) return;

	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpObj->Type != OBJ_USER) return;
	if (lpTargetObj->Type != OBJ_USER) return;

	int count = 0;
	int nChainTarget[3] = { -1, -1, -1 };
	int partynum = lpObj->PartyNumber;

	if (!CHARACTER_CheckSkillDistance(aIndex, aTargetIndex, lpMagic->m_Skill)) return;
	if (partynum > -1 && partynum != lpTargetObj->PartyNumber) {
		g_Log.Add("[party healing] mismatch party number. target : %d, (%d - %d)", aTargetIndex, lpObj->PartyNumber, lpTargetObj->PartyNumber);
		return;
	}

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
	nChainTarget[0] = aTargetIndex;
	count++;

	if (partynum > -1) {
		int partycount = gParty.m_PartyS[partynum].Count;
		BOOL EnableHealing = FALSE;

		for (int nIndex = 0; nIndex < MAX_USER_IN_PARTY; nIndex++) {
			int number = gParty.m_PartyS[partynum].Number[nIndex];

			if (number >= 0) {
				if (aIndex == number) continue;
				if (aTargetIndex == number) continue;

				LPOBJ lpPartyObj = &gObj[number];

				if (lpPartyObj) {
					if (lpPartyObj->X >= lpTargetObj->X - 3 && lpPartyObj->X <= lpTargetObj->X + 3 && lpPartyObj->Y >= lpTargetObj->Y - 3 && lpPartyObj->Y <= lpTargetObj->Y + 3) {
						EnableHealing = TRUE;
						nChainTarget[1] = number;
						count++;
						break;
					}
				}
			}
		}

		if (EnableHealing == TRUE) {
			for (int nIndex = 0; nIndex < MAX_USER_IN_PARTY; nIndex++) {
				int number = gParty.m_PartyS[partynum].Number[nIndex];

				if (number >= 0) {
					if (nChainTarget[1] == number) continue;
					if (aIndex == number) continue;
					if (aTargetIndex == number) continue;

					LPOBJ lpPartyObj = &gObj[number];

					if (lpPartyObj) {
						if (lpPartyObj->X >= lpTargetObj->X - 3 && lpPartyObj->X <= lpTargetObj->X + 3 && lpPartyObj->Y >= lpTargetObj->Y - 3 && lpPartyObj->Y <= lpTargetObj->Y + 3) {
							EnableHealing = TRUE;
							nChainTarget[2] = number;
							count++;
							break;
						}
					}
				}
			}
		}
	}

	if (count >= 1) {
		PMSG_CHAIN_MAGIC pMsg;
		PMSG_CHAIN_MAGIC_OBJECT pMagicObj;
		char SendByte[256];

		pMsg.MagicNumberH = HIBYTE(lpMagic->m_Skill);
		pMsg.MagicNumberL = LOBYTE(lpMagic->m_Skill);
		pMsg.wUserIndex = aIndex;
		pMsg.nCount = count;

		int nOffset = 10;

		for (int i = 0; i < count; i++) {
			pMagicObj.nTargetIndex = nChainTarget[i];
			memcpy(&SendByte[nOffset], &pMagicObj, 2);
			nOffset += 2;
		}

		PHeadSubSetB((LPBYTE)&pMsg, 0xBF, 0x18, nOffset);
		memcpy(&SendByte, &pMsg, sizeof(pMsg));

		if (lpObj->Type == OBJ_USER) IOCP.DataSend(aIndex, (LPBYTE)&SendByte, nOffset);

		GSProtocol.MsgSendV2(lpObj, (LPBYTE)&SendByte, nOffset);
	}

	int nHP = 0;
	int nApplyHP = 0;

	this->m_Lua.Generic_Call("PrayerOfHealing", "i>i", lpObj->CharacterData->Energy + lpObj->AddEnergy, &nHP);

	for (int n = 0; n < count; n++) {
		if (ObjectMaxRange(nChainTarget[n]) == false) continue;

		nApplyHP = nHP - (nHP * n * 0.2);

		if (lpMagic->m_Skill == 429) nApplyHP += nApplyHP * this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level) / 100.0;

		gObjAddMsgSendDelay(&gObj[nChainTarget[n]], 16, nChainTarget[n], 200 * n + 200, nApplyHP);
	}
}
void CMasterLevelSkillTreeSystem::ML_ELF_Bless(LPOBJ lpObj, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aTargetIndex) == false) return;

	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpTargetObj->Type != OBJ_USER) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	}

	if (BUFF_TargetHasBuff(lpTargetObj, BUFFTYPE_BLESS)) BUFF_RemoveBuffEffect(lpTargetObj, BUFFTYPE_BLESS);

	int iEffectValue = 0;

	this->m_Lua.Generic_Call("Bless", "i>i", lpObj->CharacterData->Energy + lpObj->AddEnergy, &iEffectValue);

	// protection for exceeding stats over maximum allowed
	int Strength = lpTargetObj->CharacterData->Strength + lpTargetObj->AddStrength;
	int Dexterity = lpTargetObj->CharacterData->Dexterity + lpTargetObj->AddDexterity;
	int Vitality = lpTargetObj->CharacterData->Vitality + lpTargetObj->AddVitality;
	int Energy = lpTargetObj->CharacterData->Energy + lpTargetObj->AddEnergy;
	int Leadership = lpTargetObj->Leadership + lpTargetObj->AddLeadership;

	if ((Strength + iEffectValue) > g_MaxStrength) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	}

	if ((Dexterity + iEffectValue) > g_MaxAgility) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	}

	if ((Vitality + iEffectValue) > g_MaxVitality) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	}

	if ((Energy + iEffectValue) > g_MaxEnergy) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	}

	if ((Leadership + iEffectValue) > g_MaxCommand) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	}

	EnterCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
	lpTargetObj->CharacterData->AgilityCheckDelay = GetTickCount();
	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_BLESS, EFFECTTYPE_INCREASE_ALLSTATS, iEffectValue, 0, 0, MagicDamageC.GetSkillKeepTime(lpMagic->m_Skill));

	PMSG_USE_STAT_FRUIT pMsg;

	PHeadSetB((LPBYTE)&pMsg, 0x2C, sizeof(pMsg));
	pMsg.result = 17;
	pMsg.btStatValue = iEffectValue;
	pMsg.btFruitType = 7;
	IOCP.DataSend(aTargetIndex, (LPBYTE)&pMsg, pMsg.h.size);
	LeaveCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
	gObjCalCharacter.CHARACTER_Calc(aTargetIndex);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_ELF_ImprovedBless(LPOBJ lpObj, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aTargetIndex) == false) return;

	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpTargetObj->Type != OBJ_USER) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	}

	double fValue = 0.0;
	float durationBonus = 1;
	float effectBonus = 1;

	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, lpMagic->m_Skill) >= 10) durationBonus += 0.3;
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, lpMagic->m_Skill) == 20) effectBonus += 0.15;

	this->m_Lua.Generic_Call("Bless", "i>d", lpObj->CharacterData->Energy + lpObj->AddEnergy, &fValue);
	fValue += this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
	fValue *= effectBonus;

	// protection for exceeding stats over maximum allowed
	int Strength = lpTargetObj->CharacterData->Strength + lpTargetObj->AddStrength;
	int Dexterity = lpTargetObj->CharacterData->Dexterity + lpTargetObj->AddDexterity;
	int Vitality = lpTargetObj->CharacterData->Vitality + lpTargetObj->AddVitality;
	int Energy = lpTargetObj->CharacterData->Energy + lpTargetObj->AddEnergy;
	int Leadership = lpTargetObj->Leadership + lpTargetObj->AddLeadership;

	if ((Strength + fValue) > g_MaxStrength) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	}

	if ((Dexterity + fValue) > g_MaxAgility) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	}

	if ((Vitality + fValue) > g_MaxVitality) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	}

	if ((Energy + fValue) > g_MaxEnergy) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	}

	if ((Leadership + fValue) > g_MaxCommand) {
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, FALSE);
		return;
	}

	EnterCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
	lpTargetObj->CharacterData->AgilityCheckDelay = GetTickCount();
	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_BLESS, EFFECTTYPE_INCREASE_ALLSTATS, fValue, 0, 0, MagicDamageC.GetSkillKeepTime(lpMagic->m_Skill) * durationBonus);
	PMSG_USE_STAT_FRUIT pMsg;
	PHeadSetB((LPBYTE)&pMsg, 0x2C, sizeof(pMsg));
	pMsg.result = 17;
	pMsg.btStatValue = fValue;
	pMsg.btFruitType = 7;
	IOCP.DataSend(aTargetIndex, (LPBYTE)&pMsg, pMsg.h.size);
	LeaveCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
	gObjCalCharacter.CHARACTER_Calc(aTargetIndex);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_ELF_ImprovedPenetration(OBJECTSTRUCT* lpObj, CMagicInf* lpMagic, int aTargetIndex) {
	if (lpObj->Type != OBJ_USER) return;

	gObjAttack(lpObj, &gObj[aTargetIndex], lpMagic, TRUE, 0, 0, 0, 0, 0);
}
void CMasterLevelSkillTreeSystem::ML_ELF_ImprovedBarrage(int aIndex, CMagicInf* lpMagic, BYTE bAngle, int aTargetIndex) {
	LPOBJECTSTRUCT lpObj = (LPOBJECTSTRUCT)&gObj[aIndex];
	this->MLS_SkillFrustrum3(aIndex, bAngle, 6.0f, 7.0f, 1.0f, 0);
	vec3_t p;
	Vector(0.f, 6.0f, 0.f, p);
	vec3_t Angle[4];
	float Matrix[3][4];
	vec3_t vLine[5];
	Vector(0.f, 0.f, 40.0f, Angle[0]);
	Vector(0.f, 0.f, 20.0f, Angle[1]);
	Vector(0.f, 0.f, 340.0f, Angle[2]);
	Vector(0.f, 0.f, 320.0f, Angle[3]);
	AngleMatrix(Angle[0], Matrix);
	VectorRotate(p, Matrix, vLine[0]);
	AngleMatrix(Angle[1], Matrix);
	VectorRotate(p, Matrix, vLine[1]);
	Vector(0.f, 6.0f, 0.f, vLine[2]);
	AngleMatrix(Angle[2], Matrix);
	VectorRotate(p, Matrix, vLine[3]);
	AngleMatrix(Angle[3], Matrix);
	VectorRotate(p, Matrix, vLine[4]);
	Vector(0.f, 0.f, (bAngle * 360 / (BYTE)255), Angle[0]);
	AngleMatrix((float*)Angle, Matrix);
	int vSkillLineX[5];
	int vSkillLineY[5];
	vec3_t vFrustrum[5];

	for (int i = 0; i < 5; i++) {
		VectorRotate(vLine[i], Matrix, vFrustrum[i]);
		vSkillLineX[i] = (int)vFrustrum[i][0] + lpObj->X;
		vSkillLineY[i] = (int)vFrustrum[i][1] + lpObj->Y;
	}

	int tObjNum;
	int count = 0;
	int HitCount = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	BOOL EnableAttack;

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
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack) {
					int nResult[5];
					for (int i = 0; i < 5; i++) {
						nResult[i] = (((int)vSkillLineX[i] - lpObj->X) * (gObj[tObjNum].Y - lpObj->Y)) - (((int)vSkillLineY[i] - lpObj->Y) * (gObj[tObjNum].X - lpObj->X));
						if (SkillTestFrustrum(gObj[tObjNum].X, gObj[tObjNum].Y, aIndex)) {
							if (nResult[i] > -5 && nResult[i] < 5) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 0, 0, FALSE, 0, 0);
						}
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}
void CMasterLevelSkillTreeSystem::ML_ELF_SummonSatyr(int aIndex, int MonsterType, int x, int y) {
	if (gObj[aIndex].Type != OBJ_USER) return;
	if (gObj[aIndex].MapNumber == MAP_INDEX_ICARUS) return;
	if (CC_MAP_RANGE(gObj[aIndex].MapNumber) || gObj[aIndex].MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) return;
	if (gObj[aIndex].m_RecallMon >= 0) {
		GSProtocol.GCRecallMonLife(aIndex, 60, 0);
		gObjMonsterCallKill(aIndex);
		return;
	}

	int result = gObjAddCallMon();

	if (result >= 0) {
		gObj[result].X = x;
		gObj[result].Y = y;
		gObj[result].MTX = x;
		gObj[result].MTY = y;
		gObj[result].Dir = 2;
		gObj[result].MapNumber = gObj[aIndex].MapNumber;
		gObjSetMonster(result, MonsterType);
		gObj[result].m_Attribute = 100;
		gObj[result].TargetNumber = -1;
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
			gObj[result].m_iPentagramDefense += gObj[result].m_iPentagramDefense * gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionDefensive / 100.0;
			gObj[result].m_iPentagramDefenseRating += gObj[result].m_iPentagramDefenseRating * gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionDefensive / 100.0;
		}

		if (gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionOffensive > 0.0) {
			gObj[result].m_AttackDamageMin += gObj[result].m_AttackDamageMin * gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionOffensive / 100.0;
			gObj[result].m_AttackDamageMax += gObj[result].m_AttackDamageMax * gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionOffensive / 100.0;
			gObj[result].HitRating += gObj[result].HitRating * gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionOffensive / 100.0;
			gObj[result].m_iPentagramAttackMin = gObj[result].m_iPentagramAttackMin * gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionOffensive / 100.0;
			gObj[result].m_iPentagramAttackMax = gObj[result].m_iPentagramAttackMax * gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionOffensive / 100.0;
			gObj[result].m_iPentagramAttackRating = gObj[result].m_iPentagramAttackRating * gObj[aIndex].CharacterData->Mastery.ML_ELF_MinionOffensive / 100.0;
		}

		GSProtocol.GCRecallMonLife(gObj[result].m_RecallMon, gObj[result].MaxLife, gObj[result].Life);
	}
}
void CMasterLevelSkillTreeSystem::ML_ELF_EternalArrow(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	int effectIndex = 0;

	if (lpObj->Type != OBJ_USER) return;
	if (lpObj->Class != CLASS_ELF) return;
	if (lpObj->CharacterData->ChangeUP == FALSE) return;
	if (lpObj->m_Index != aTargetIndex) return;
	if (BUFF_TargetHasBuff(lpObj, BUFFTYPE_ETERNAL_ARROW)) return;
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, lpMagic->m_Skill) >= 10) effectIndex = EFFECTTYPE_INCREASE_ATTACK_SPEED_PERCENT;

	lpObj->m_SkillInfo.ML_EternalArrow = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
	BUFF_AddBuffEffect(lpObj, BUFFTYPE_ETERNAL_ARROW, effectIndex, 0, 0, 0, -10);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, TRUE);
	g_Log.Add("[%s][%s] Use Infinity Arrow Skill (Time:%d)(Character Level : %d)(ChangeUp: %d)", lpObj->AccountID, lpObj->Name, g_SkillAdditionInfo.GetInfinityArrowSkillTime(), lpObj->Level, lpObj->CharacterData->ChangeUP);
	gObjCalCharacter.CHARACTER_Calc(aIndex);
}
void CMasterLevelSkillTreeSystem::ML_ELF_PoisonArrow(LPOBJ lpObj, CMagicInf* lpMagic, int aTargetIndex, BOOL isCombo) {
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (gObjIsConnected(lpObj->m_Index) == FALSE && gObjIsConnected(lpTargetObj->m_Index) == FALSE) return;
	if (lpObj->Type != OBJ_USER) return;

	gObjAttack(lpObj, lpTargetObj, lpMagic, TRUE, 0, 0, isCombo, 0, 0);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
}

// Summoner
void CMasterLevelSkillTreeSystem::ML_SUM_ImprovedAmplifyDamage(int aIndex, CMagicInf* lpMagic) {
	if (!ObjectMaxRange(aIndex)) return;

	LPOBJ lpObj = &gObj[aIndex];
	int iDuration = 0;
	int iEffectUPValue = 0;
	int iEffectDownValue = 0;

	if (lpObj->Type == OBJ_USER) {
		this->m_Lua.Generic_Call("SummonerBerserker_Level1", "i>iii", lpObj->CharacterData->Energy + lpObj->AddEnergy, &iEffectUPValue, &iEffectDownValue, &iDuration);
		lpObj->CharacterData->Mastery.ML_SUM_ImprovedAmplifyDamage = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
	} else {
		iDuration = 600000;
		iEffectUPValue = 0;
		iEffectDownValue = 0;
	}

	BUFF_AddBuffEffect(lpObj, BUFFTYPE_IMPROVED_AMPLIFY_DAMAGE, EFFECTTYPE_INCREASE_AMPLIFY_DAMAGE, iEffectUPValue, EFFECTTYPE_DECREASE_AMPLIFY_DAMAGE, iEffectDownValue, iDuration);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aIndex, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_SUM_DimensionalAffinity(int aIndex, CMagicInf* lpMagic) {
	if (!ObjectMaxRange(aIndex)) return;

	LPOBJ lpObj = &gObj[aIndex];
	int iDuration = 0;
	int iEffectUPValue = 0;
	int iEffectDownValue = 0;

	lua_Reader();

	if (lpObj->Type == OBJ_USER) {
		this->m_Lua.Generic_Call("SummonerBerserker_Level2", "i>iii", lpObj->CharacterData->Energy + lpObj->AddEnergy, &iEffectUPValue, &iEffectDownValue, &iDuration);
		lpObj->CharacterData->Mastery.ML_SUM_ImprovedAmplifyDamage = this->GetBrandOfMasterSkillValue(lpObj, lpMagic->m_Skill, 1);
		lpObj->CharacterData->Mastery.ML_SUM_DimensionalAffinity = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
	} else {
		iDuration = 600000;
		iEffectUPValue = 0;
		iEffectDownValue = 0;
	}

	BUFF_AddBuffEffect(lpObj, BUFFTYPE_DIMENSIONAL_AFFINITY, EFFECTTYPE_INCREASE_AMPLIFY_DAMAGE, iEffectUPValue, EFFECTTYPE_DECREASE_AMPLIFY_DAMAGE, iEffectDownValue, iDuration);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aIndex, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_SUM_VoidAffinity(int aIndex, CMagicInf* lpMagic) {
	if (!ObjectMaxRange(aIndex)) return;

	LPOBJ lpObj = &gObj[aIndex];
	int iDuration = 0;
	int iEffectUPValue = 0;
	int iEffectDownValue = 0;

	if (lpObj->Type == OBJ_USER) {
		this->m_Lua.Generic_Call("SummonerBerserker_Level3", "i>iii", lpObj->CharacterData->Energy + lpObj->AddEnergy, &iEffectUPValue, &iEffectDownValue, &iDuration);
		lpObj->CharacterData->Mastery.ML_SUM_ImprovedAmplifyDamage = this->GetBrandOfMasterSkillValue(lpObj, lpMagic->m_Skill, 2);
		lpObj->CharacterData->Mastery.ML_SUM_DimensionalAffinity = this->GetBrandOfMasterSkillValue(lpObj, lpMagic->m_Skill, 1);
		lpObj->CharacterData->Mastery.ML_SUM_VoidAffinity = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
	} else {
		iDuration = 600000;
		iEffectUPValue = 0;
		iEffectDownValue = 0;
	}

	BUFF_AddBuffEffect(lpObj, BUFFTYPE_VOID_AFFINITY, EFFECTTYPE_INCREASE_AMPLIFY_DAMAGE, iEffectUPValue, EFFECTTYPE_DECREASE_AMPLIFY_DAMAGE, iEffectDownValue, iDuration);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aIndex, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_SUM_ImprovedDrainLife(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aTargetIndex) == false) return;

	LPOBJ lpObj = &gObj[aIndex];
	int tObjNum;
	int count = 0;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 1);

	int DuelIndex = lpObj->m_iDuelUser;
	BOOL EnableAttack;

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = 0;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = 1;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = 1;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = 1;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack != 0) {
					if (tObjNum == aTargetIndex) {
						if (CHARACTER_CheckSkillDistance(aIndex, aTargetIndex, lpMagic->m_Skill)) {
							gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 700, lpMagic->m_Skill, 0);
							break;
						}
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}
void CMasterLevelSkillTreeSystem::ML_SUM_ImprovedChainLightning(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aIndex) == false) return;
	if (ObjectMaxRange(aTargetIndex) == false) return;

	LPOBJECTSTRUCT lpObj = (LPOBJECTSTRUCT)&gObj[aIndex];
	LPOBJECTSTRUCT lpTargetObj = (LPOBJECTSTRUCT)&gObj[aTargetIndex];
	int tObjNum;
	int count = 0;
	int tnum = 0;
	int nChainTarget[10] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	int totalTargets = 0;
	int DuelIndex;
	BOOL EnableAttack;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, 1);

	DuelIndex = lpObj->m_iDuelUser;

	if (!CHARACTER_CheckSkillDistance(aIndex, aTargetIndex, lpMagic->m_Skill)) return;

	nChainTarget[0] = aTargetIndex;
	totalTargets++;

	while (totalTargets < 10) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0 && ((gObj[tObjNum].Class < 100 || gObj[tObjNum].Class > 110) || gObj[tObjNum].Class != 689)) EnableAttack = TRUE;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack) {
					if (gObj[nChainTarget[totalTargets - 1]].m_Index != gObj[tObjNum].m_Index) {
						if (CHARACTER_CalcDistance(&gObj[nChainTarget[totalTargets - 1]], &gObj[tObjNum]) < 5) {
							nChainTarget[totalTargets] = tObjNum;
							totalTargets++;
						}
					} else {
						if (totalTargets > 1) {
							nChainTarget[totalTargets] = nChainTarget[totalTargets - 2];
							totalTargets++;
						}
					}					
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}

	int nOffSet = 0;
	PMSG_CHAIN_MAGIC pMsg;

	pMsg.MagicNumberH = HIBYTE(lpMagic->m_Skill);
	pMsg.MagicNumberL = LOBYTE(lpMagic->m_Skill);
	pMsg.wUserIndex = aIndex;
	pMsg.nCount = totalTargets;
	nOffSet += sizeof(pMsg);

	BYTE SendByte[256];
	PMSG_CHAIN_MAGIC_OBJECT	pMagicObj;

	for (int n = 0; n < totalTargets; n++) {
		pMagicObj.nTargetIndex = nChainTarget[n];
		memcpy(&SendByte[nOffSet], &pMagicObj, sizeof(PMSG_CHAIN_MAGIC_OBJECT));
		nOffSet += sizeof(PMSG_CHAIN_MAGIC_OBJECT);
	}

	PHeadSubSetB((LPBYTE)&pMsg, 0xBF, 0x0A, nOffSet);
	memcpy(SendByte, &pMsg, sizeof(pMsg));

	if (lpObj->Type == OBJ_USER) IOCP.DataSend(lpObj->m_Index, SendByte, pMsg.h.size);

	GSProtocol.MsgSendV2(lpObj, SendByte, pMsg.h.size);

	for (int n = 0; n < totalTargets; n++) gObjAddAttackProcMsgSendDelay(lpObj, 50, nChainTarget[n], (n * 15) + 200, lpMagic->m_Skill, n + 1);
}
void CMasterLevelSkillTreeSystem::ML_SUM_ImprovedSleep(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	int result = 0;
	int tObjNum;
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
							break;
						}
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}
void CMasterLevelSkillTreeSystem::ML_SUM_ImprovedEnfeeble(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int count = 0;
	int HitCount = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int tObjNum;
	BOOL EnableAttack;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (DuelIndex == tObjNum) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
				else if (CC_MAP_RANGE(lpObj->MapNumber) == TRUE || lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) EnableAttack = TRUE;
				else if (IT_MAP_RANGE(lpObj->MapNumber) == TRUE) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) < 5) {
						gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 700, lpMagic->m_Skill, 0);
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
void CMasterLevelSkillTreeSystem::ML_SUM_ImprovedWeakness(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int count = 0;
	int HitCount = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int tObjNum;
	BOOL EnableAttack;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = FALSE;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
				else if (DuelIndex == tObjNum) EnableAttack = TRUE;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
				else if (CC_MAP_RANGE(lpObj->MapNumber) == TRUE || lpObj->MapNumber == MAP_INDEX_CHAOSCASTLE_SURVIVAL) EnableAttack = TRUE;
				else if (IT_MAP_RANGE(lpObj->MapNumber) == TRUE) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) < 5) {
						gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 700, lpMagic->m_Skill, 0);
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
void CMasterLevelSkillTreeSystem::ML_SUM_ImprovedLightningBlast(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	BOOL EnableAttack;

	for (int n = 0; n < MAX_VIEWPORT; n++) {
		EnableAttack = FALSE;

		if (lpObj->VpPlayer2[n].state <= 0) continue;

		int index = lpObj->VpPlayer2[n].number;

		if (gObj[index].Type == OBJ_MONSTER && gObj[index].m_RecallMon < 0) EnableAttack = TRUE;
		if (gObj[aIndex].Type == OBJ_MONSTER && gObj[index].Type == OBJ_USER) EnableAttack = TRUE;
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
void CMasterLevelSkillTreeSystem::ML_SUM_Blind(LPOBJ lpObj, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aTargetIndex) == false) return;

	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	float fValue = 0.0;

	if (lpMagic->m_Skill == 463) fValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
	if (BUFF_TargetHasBuff(lpTargetObj, DEBUFFTYPE_BLIND)) BUFF_RemoveBuffEffect(lpTargetObj, DEBUFFTYPE_BLIND);

	BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_BLIND, EFFECTTYPE_BLIND, fValue, 0, 0, MagicDamageC.GetSkillKeepTime(lpMagic->m_Skill));
	gObjViewportListCreate(lpTargetObj->m_Index);
	gObjAttack(lpObj, lpTargetObj, lpMagic, 1, 1, 0, 0, 0, 0);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
}

// Magic Gladiator
void CMasterLevelSkillTreeSystem::ML_MG_TornadoSlash(LPOBJ lpObj, CMagicInf *lpMagic, int aTargetIndex, BOOL isCombo) {
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (gObjIsConnected(lpObj->m_Index) == FALSE && gObjIsConnected(lpTargetObj->m_Index) == FALSE) return;
	if (lpObj->Type != OBJ_USER || lpObj->Class != CLASS_MAGICGLADIATOR) return;

	gObjAttack(lpObj, lpTargetObj, lpMagic, 0, 1, 0, isCombo, 0, 0);
}
void CMasterLevelSkillTreeSystem::ML_MG_ImprovedPowerSlash(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, BYTE Targetangle, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int iSkillDis = MagicDamageC.GetSkillDistance(lpMagic->m_Skill);

	this->MLS_SkillFrustrum(aIndex, (360 * Targetangle / 255), iSkillDis + 1.0, iSkillDis + 1.0);

	int count = 0;
	int HitCount = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int tObjNum;
	BOOL EnableAttack;
	BOOL bAttack;

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
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack == TRUE) {
					if (SkillTestFrustrum(gObj[tObjNum].X, gObj[tObjNum].Y, aIndex) == TRUE) {
						if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) <= iSkillDis) {
							bAttack = FALSE;

							if (HitCount > 10) bAttack = FALSE;
							if (HitCount < 5) bAttack = TRUE;
							else bAttack = rand() % 2;
							if (bAttack == TRUE) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, TRUE, TRUE, 0, 0, 0, 0);

							HitCount++;
						}
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}
void CMasterLevelSkillTreeSystem::ML_MG_ImprovedFireSlash(int aIndex, int aTargetIndex) {
	int SuccessRate = 50;
	int Value = 15;
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpObj->Type == OBJ_USER && lpObj->Class != CLASS_MAGICGLADIATOR) return;
	if (!ObjectMaxRange(aTargetIndex)) return;
	if (CHARACTER_DebuffProc(SuccessRate)) {
		BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_FIRE_SLASH, EFFECTTYPE_DECREASE_DEFENSE, Value, 0, 0, 10);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, 490, aTargetIndex, TRUE);
	}
}
void CMasterLevelSkillTreeSystem::ML_MG_SearingSlash(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpObj->Type != OBJ_USER) return;
	if (lpObj->Class != CLASS_MAGICGLADIATOR) return;
	if (ObjectMaxRange(aTargetIndex) == false) return;

	float fDownValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
	int SuccessRate = 50;
	int Value = 15;
	int bonus = 0;

	Value += fDownValue;

	if (this->ML_GetNodeLevel(lpObj, lpMagic->m_Skill) == 20) bonus = 25;
	if (CHARACTER_DebuffProc(SuccessRate)) {
		BUFF_AddBuffEffect(lpTargetObj, DEBUFFTYPE_SEARING_SLASH, EFFECTTYPE_DECREASE_DEFENSE, Value, 0, bonus, 10);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
	}
}
void CMasterLevelSkillTreeSystem::ML_MG_ImprovedFlameStrike(int aIndex, CMagicInf* lpMagic, BYTE bAngle, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	this->MLS_SkillFrustrum3(aIndex, bAngle, 2.0, 4.0, 5.0, 0.0);
	int count = 0;
	int HitCount = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int tObjNum;
	BOOL EnableAttack = FALSE;
	BOOL bAttack = FALSE;

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
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack == TRUE) {
					if (SkillTestFrustrum(gObj[tObjNum].X, gObj[tObjNum].Y, aIndex) && CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) <= MagicDamageC.GetSkillDistance(lpMagic->m_Skill)) {
						bAttack = FALSE;

						if (HitCount < 8) bAttack = TRUE;
						else if (rand() % 2) bAttack = TRUE;
						if (bAttack) {
							gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 0, 0, 0, 0, 0);
							gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 0, 0, 0, 0, 0);
						}

						HitCount++;
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}
void CMasterLevelSkillTreeSystem::ML_MG_Freeze(LPOBJ lpObj, CMagicInf* lpMagic, int aTargetIndex) {
	if (lpObj->Type != OBJ_USER) return;

	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	float fValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);

	gObjAttack(lpObj, lpTargetObj, lpMagic, 1, 0, 0, 0, 0, 0);
}
void CMasterLevelSkillTreeSystem::ML_MG_ImprovedThunderStorm(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int count = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int nHitCount = 0;
	BOOL bAttack = FALSE;
	int tObjNum;
	BOOL EnableAttack = FALSE;
	int nMinCount = 15;
	int nMaxCount = 20;

	if (this->ML_SKILL_CalcDistance(lpObj->X, lpObj->Y, x, y) <= MagicDamageC.GetSkillDistance(lpMagic->m_Skill)) {
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
						if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
						if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
					}

					if (EnableAttack == TRUE) {
						if (gObj[tObjNum].X >= x - 6 && gObj[tObjNum].X <= x + 6 && gObj[tObjNum].Y >= y - 6 && gObj[tObjNum].Y <= y + 6) {
							gObj[tObjNum].lpAttackObj = lpObj;
							nHitCount++;

							if (nHitCount <= nMinCount) bAttack = TRUE;
							else bAttack = rand() % 2;
							if (nHitCount > nMaxCount) bAttack = FALSE;
							if (bAttack == TRUE) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 400, lpMagic->m_Skill, 0);
						}
					}
				}
			}

			count++;

			if (count > MAX_VIEWPORT - 1) break;
		}
	}
}

// Dark Lord
void CMasterLevelSkillTreeSystem::ML_DL_ImprovedEarthquake(LPOBJ lpObj, int aTargetIndex, CMagicInf* lpMagic) {
	if (lpObj->Type != OBJ_USER) return;
	if (CHECK_DarkHorse(lpObj) == FALSE) return;
	
	int count = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int tObjNum;
	BOOL EnableAttack;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, TRUE);

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
					if (gObjTargetGuildWarCheck(lpObj,&gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack == TRUE) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) < 5) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 500, lpMagic->m_Skill, 0);
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT-1) break;
	}
}
void CMasterLevelSkillTreeSystem::ML_DL_ImprovedForce(int aIndex, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	int count = 0;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);

	if (lpObj->SkillLongSpearChange == false) {
		gObjAttack(lpObj, &gObj[aTargetIndex], lpMagic, 0, 1, 0, 0, 0, 0);
		return;
	}

	int DuelIndex = lpObj->m_iDuelUser;
	int iangle = this->MLS_GetAngle(gObj[aIndex].X, gObj[aIndex].Y, gObj[aTargetIndex].X, gObj[aTargetIndex].Y);
	int tObjNum;
	BOOL EnableAttack;

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
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;					
				}

				if (EnableAttack == TRUE) {
					if (SkillSpearHitBox.HitCheck(iangle, gObj[aIndex].X, gObj[aIndex].Y, gObj[tObjNum].X, gObj[tObjNum].Y) == TRUE) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 0, 1, 0, 0, 0, 0);
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}
void CMasterLevelSkillTreeSystem::ML_DL_ImprovedFireburst(LPOBJ lpObj, int aTargetIndex, CMagicInf* lpMagic) {
	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	int count = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int tObjNum;
	BOOL EnableAttack;

	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
	gObjAttack(lpObj, lpTargetObj, lpMagic, 0, TRUE, 0, 0, 0, 0);

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
					if (gObjTargetGuildWarCheck(lpObj,&gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack == TRUE) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[tObjNum]) < 3) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, (rand()*17)%300 + 500, lpMagic->m_Skill, 0);
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT-1) break;
	}
}
void CMasterLevelSkillTreeSystem::ML_DL_ImprovedFireScream(int aIndex, int aTargetIndex, CMagicInf *lpMagic) {
	if(g_SkillAdditionInfo.GetFireScreamSkill() == false) return;

	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (gObjIsConnected(lpObj->m_Index) == false && gObjIsConnected(lpTargetObj->m_Index) == false) return;
	if (lpObj->Type != OBJ_USER) return;
	if (lpObj->Class != CLASS_DARKLORD) return;

	gObjAttack(lpObj,lpTargetObj,lpMagic,0,0,0,0,0,0);
}
void CMasterLevelSkillTreeSystem::ML_DL_ImprovedConcentration(int aIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	int number;
	int ApplyPartyIndex[MAX_USER_IN_PARTY];
	LPOBJ lpPartyObj;

	memset(ApplyPartyIndex, -1, sizeof(ApplyPartyIndex));

	int partynum = lpObj->PartyNumber;

	if (partynum != -1) {
		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			number = gParty.m_PartyS[partynum].Number[n];

			if (number >= 0) {
				lpPartyObj = &gObj[number];

				if (lpObj->MapNumber == lpPartyObj->MapNumber) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[number]) < 10) ApplyPartyIndex[n] = lpPartyObj->m_Index;
				}
			}
		}
	}

	int addcriticaldamagevalue = 0;
	int SkillTime = 0;

	this->m_Lua.Generic_Call("DarkLordCriticalDamage_Level1", "ii>ii", lpObj->Leadership + lpObj->AddLeadership, lpObj->CharacterData->Energy + lpObj->AddEnergy, &addcriticaldamagevalue, &SkillTime);
	addcriticaldamagevalue += this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);

	if (partynum == -1) {
		BUFF_AddBuffEffect(lpObj, BUFFTYPE_CONCENTRATION, EFFECTTYPE_INCREASE_CRITICAL_DAMAGE, addcriticaldamagevalue, 0, 0, SkillTime);
		MsgOutput(aIndex, Lang.GetText(0, 134), SkillTime);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, TRUE);
	} else {
		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			if (ApplyPartyIndex[n] != -1) {
				lpPartyObj = &gObj[ApplyPartyIndex[n]];
				BUFF_AddBuffEffect(lpPartyObj, BUFFTYPE_CONCENTRATION, EFFECTTYPE_INCREASE_CRITICAL_DAMAGE, addcriticaldamagevalue, 0, 0, SkillTime);
				MsgOutput(ApplyPartyIndex[n], Lang.GetText(0, 134), SkillTime);
				GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpPartyObj->m_Index, TRUE);
			}
		}
	}
}
void CMasterLevelSkillTreeSystem::ML_DL_EnhancedConcentration(int aIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	int number;
	int ApplyPartyIndex[MAX_USER_IN_PARTY];
	LPOBJ lpPartyObj;
	memset(ApplyPartyIndex, -1, sizeof(ApplyPartyIndex));

	int partynum = lpObj->PartyNumber;

	if (partynum != -1) {
		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			number = gParty.m_PartyS[partynum].Number[n];

			if (number >= 0) {
				lpPartyObj = &gObj[number];

				if (lpObj->MapNumber == lpPartyObj->MapNumber) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[number]) < 10) ApplyPartyIndex[n] = lpPartyObj->m_Index;
				}
			}
		}
	}

	int addcriticaldamagevalue = 0;
	int SkillTime = 0;

	this->m_Lua.Generic_Call("DarkLordCriticalDamage_Level2", "ii>ii", lpObj->Leadership + lpObj->AddLeadership, lpObj->CharacterData->Energy + lpObj->AddEnergy, &addcriticaldamagevalue, &SkillTime);
	addcriticaldamagevalue += this->GetBrandOfMasterSkillValue(lpObj, lpMagic->m_Skill, 1);
	SkillTime += this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);

	if (partynum == -1) {
		BUFF_AddBuffEffect(lpObj, BUFFTYPE_CONCENTRATION, EFFECTTYPE_INCREASE_CRITICAL_DAMAGE, addcriticaldamagevalue, 0, 0, SkillTime);
		MsgOutput(aIndex, Lang.GetText(0, 134), SkillTime);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, TRUE);
	} else {
		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			if (ApplyPartyIndex[n] != -1) {
				lpPartyObj = &gObj[ApplyPartyIndex[n]];
				BUFF_AddBuffEffect(lpPartyObj, BUFFTYPE_CONCENTRATION, EFFECTTYPE_INCREASE_CRITICAL_DAMAGE, addcriticaldamagevalue, 0, 0, SkillTime);
				MsgOutput(ApplyPartyIndex[n], Lang.GetText(0, 134), SkillTime);
				GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpPartyObj->m_Index, TRUE);
			}
		}
	}
}
void CMasterLevelSkillTreeSystem::ML_DL_CriticalConcentration(int aIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	int number;
	int ApplyPartyIndex[MAX_USER_IN_PARTY];
	LPOBJ lpPartyObj;

	memset(ApplyPartyIndex, -1, sizeof(ApplyPartyIndex));

	int partynum = lpObj->PartyNumber;

	if (partynum != -1) {
		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			number = gParty.m_PartyS[partynum].Number[n];

			if (number >= 0) {
				lpPartyObj = &gObj[number];

				if (lpObj->MapNumber == lpPartyObj->MapNumber) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[number]) < 10) ApplyPartyIndex[n] = lpPartyObj->m_Index;
				}
			}
		}
	}

	int addcriticaldamagevalue = 0;
	int SkillTime = 0;

	this->m_Lua.Generic_Call("DarkLordCriticalDamage_Level3", "ii>ii", lpObj->Leadership + lpObj->AddLeadership, lpObj->CharacterData->Energy + lpObj->AddEnergy, &addcriticaldamagevalue, &SkillTime);
	addcriticaldamagevalue += this->GetBrandOfMasterSkillValue(lpObj, lpMagic->m_Skill, 2);
	SkillTime += this->GetBrandOfMasterSkillValue(lpObj, lpMagic->m_Skill, 1);

	float fRate = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);

	if (partynum == -1) {
		BUFF_AddBuffEffect(lpObj, BUFFTYPE_CRITICAL_CONCENTRATION, EFFECTTYPE_INCREASE_CRITICAL_DAMAGE, addcriticaldamagevalue, EFFECTTYPE_INCREASE_CRITICAL_STRIKE_CHANCE, fRate, SkillTime);
		MsgOutput(aIndex, Lang.GetText(0, 134), SkillTime);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, TRUE);
	} else {
		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			if (ApplyPartyIndex[n] != -1) {
				lpPartyObj = &gObj[ApplyPartyIndex[n]];
				BUFF_AddBuffEffect(lpPartyObj, BUFFTYPE_CRITICAL_CONCENTRATION, EFFECTTYPE_INCREASE_CRITICAL_DAMAGE, addcriticaldamagevalue, EFFECTTYPE_INCREASE_CRITICAL_STRIKE_CHANCE, fRate, SkillTime);
				MsgOutput(ApplyPartyIndex[n], Lang.GetText(0, 134), SkillTime);
				GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpPartyObj->m_Index, TRUE);
			}
		}
	}
}
void CMasterLevelSkillTreeSystem::ML_DL_ExcellentConcentration(int aIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];
	int number;
	int ApplyPartyIndex[MAX_USER_IN_PARTY];
	LPOBJ lpPartyObj;

	memset(ApplyPartyIndex, -1, sizeof(ApplyPartyIndex));

	int partynum = lpObj->PartyNumber;

	if (partynum != -1) {
		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			number = gParty.m_PartyS[partynum].Number[n];

			if (number >= 0) {
				lpPartyObj = &gObj[number];

				if (lpObj->MapNumber == lpPartyObj->MapNumber) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[number]) < 10) ApplyPartyIndex[n] = lpPartyObj->m_Index;
				}
			}
		}
	}

	int addcriticaldamagevalue = 0;
	int SkillTime = 0;

	this->m_Lua.Generic_Call("DarkLordCriticalDamage_Level4", "ii>ii", lpObj->Leadership + lpObj->AddLeadership, lpObj->CharacterData->Energy + lpObj->AddEnergy, &addcriticaldamagevalue, &SkillTime);
	addcriticaldamagevalue += this->GetBrandOfMasterSkillValue(lpObj, lpMagic->m_Skill, 3);
	SkillTime += this->GetBrandOfMasterSkillValue(lpObj, lpMagic->m_Skill, 2);

	float fRate = this->GetBrandOfMasterSkillValue(lpObj, lpMagic->m_Skill, 1);

	if (partynum == -1) {
		lpObj->CharacterData->Mastery.iMpsIncExcellentDamageRate_Darklord = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
		BUFF_AddBuffEffect(lpObj, BUFFTYPE_EXCELLENT_CONCENTRATION, EFFECTTYPE_INCREASE_CRITICAL_DAMAGE, addcriticaldamagevalue, EFFECTTYPE_INCREASE_CRITICAL_STRIKE_CHANCE, fRate, SkillTime);
		MsgOutput(aIndex, Lang.GetText(0, 134), SkillTime);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpObj->m_Index, TRUE);
	} else {
		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			if (ApplyPartyIndex[n] != -1) {
				lpPartyObj = &gObj[ApplyPartyIndex[n]];
				lpPartyObj->CharacterData->Mastery.iMpsIncExcellentDamageRate_Darklord = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
				BUFF_AddBuffEffect(lpPartyObj, BUFFTYPE_EXCELLENT_CONCENTRATION, EFFECTTYPE_INCREASE_CRITICAL_DAMAGE, addcriticaldamagevalue, EFFECTTYPE_INCREASE_CRITICAL_STRIKE_CHANCE, fRate, SkillTime);
				MsgOutput(ApplyPartyIndex[n], Lang.GetText(0, 134), SkillTime);
				GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, lpPartyObj->m_Index, TRUE);
			}
		}
	}
}
void CMasterLevelSkillTreeSystem::ML_DL_ImprovedSpark(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, BYTE TargetPos, int aTargetIndex) {
	LPOBJ lpObj = &gObj[aIndex];
	int TargePosx = lpObj->X - (8 - (TargetPos & 15));
	int TargePosy = lpObj->Y - (8 - ((TargetPos & 240) >> 4));
	int tObjNum;
	int count = 0;
	int DuelIndex = lpObj->m_iDuelUser;
	int iangle = this->MLS_GetAngle(lpObj->X, lpObj->Y, TargePosx, TargePosy);
	int delaytime;
	int Hit = 0;
	int EnableAttack;

	while (true) {
		if (lpObj->VpPlayer2[count].state != 0) {
			tObjNum = lpObj->VpPlayer2[count].number;

			if (tObjNum >= 0) {
				EnableAttack = 0;

				if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = 1;
				else if (tObjNum == aTargetIndex || DuelIndex == tObjNum) EnableAttack = 1;
				else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
				else {
					int CallMonIndex = gObj[tObjNum].m_Index;

					if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == 1) EnableAttack = 1;
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
void CMasterLevelSkillTreeSystem::ML_DL_ImprovedDarkness(int aIndex, CMagicInf* lpMagic, BYTE bAngle, int aTargetIndex) {
	LPOBJECTSTRUCT lpObj = (LPOBJECTSTRUCT)&gObj[aIndex];
	LPOBJECTSTRUCT lpTargetObj = (LPOBJECTSTRUCT)&gObj[aTargetIndex];
	this->MLS_SkillFrustrum3(aIndex, bAngle, 1.5f, 6.0f, 1.5f, 0);
	int tObjNum;
	int count = 0;
	int HitCount = 0;
	BOOL bAttack;
	int DuelIndex = lpObj->m_iDuelUser;
	BOOL EnableAttack;

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
					if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
					if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
				}

				if (EnableAttack) {
					if (SkillTestFrustrum(gObj[tObjNum].X, gObj[tObjNum].Y, aIndex)) {
						bAttack = FALSE;

						if (HitCount >= 8) {
							if (rand() % 2) bAttack = TRUE;
						} else bAttack = TRUE;

						if (bAttack) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 200, lpMagic->m_Skill, 0);
						if (ObjectMaxRange(aTargetIndex) == true && tObjNum == aTargetIndex) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, 300, lpMagic->m_Skill, 0);

						HitCount++;
					}
				}
			}
		}

		count++;

		if (count > MAX_VIEWPORT - 1) break;
	}
}
void CMasterLevelSkillTreeSystem::ML_DL_Stoneskin(LPOBJ lpObj, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aTargetIndex) == false) return;

	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	float fValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);

	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_STONESKIN, EFFECTTYPE_INCREASE_LIFE, fValue, EFFECTTYPE_INCREASE_DEFENSE, fValue, MagicDamageC.GetSkillKeepTime(lpMagic->m_Skill));
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_DL_ImprovedStoneskin(LPOBJ lpObj, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aTargetIndex) == false) return;

	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	float fPrevValue = 0.0;
	int iPreSkill = this->GetParentSkill1(lpObj->Class, lpMagic->m_Skill);
	float BonusDuration = 1;

	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, lpMagic->m_Skill) == 20) BonusDuration += 0.5;			// Improved Stoneskin
	if (iPreSkill > 0) {
		int iPreLevel = this->ML_GetNodeLevel(lpObj, iPreSkill);

		if (iPreLevel > 0) fPrevValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(iPreSkill), iPreLevel);
	}

	fPrevValue += this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_IMPROVED_STONESKIN, EFFECTTYPE_INCREASE_LIFE, fPrevValue, EFFECTTYPE_INCREASE_DEFENSE, fPrevValue, MagicDamageC.GetSkillKeepTime(lpMagic->m_Skill) * BonusDuration);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
}

// Rage Fighter
void CMasterLevelSkillTreeSystem::ML_RF_ImprovedLightningReflexes(int aIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJ_USER && lpObj->m_RecallMon == -1) return;
	if (lpObj->Class != CLASS_RAGEFIGHTER) return;

	int number;
	int ApplyPartyIndex[MAX_USER_IN_PARTY];
	LPOBJ lpPartyObj;

	memset(ApplyPartyIndex, -1, sizeof(ApplyPartyIndex));

	int partynum = lpObj->PartyNumber;

	if (partynum != -1) {
		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			number = gParty.m_PartyS[partynum].Number[n];

			if (number >= 0) {
				lpPartyObj = &gObj[number];

				if (lpObj->MapNumber == lpPartyObj->MapNumber) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[number]) < 10) ApplyPartyIndex[n] = lpPartyObj->m_Index;
				}
			}
		}
	}

	int nEffectValue = (lpObj->CharacterData->Energy + lpObj->AddEnergy) / 50.0 + 10.0;
	float bonusDuration = 1;

	if (nEffectValue > 100) nEffectValue = 100;
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, lpMagic->m_Skill) == 20) bonusDuration += 0.2;

	nEffectValue += this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);

	if (partynum == -1) {
		BUFF_AddBuffEffect(lpObj, BUFFTYPE_IMPROVED_LIGHTNING_REFLEXES, NULL, nEffectValue, 0, 0, ((lpObj->CharacterData->Energy + lpObj->AddEnergy) / 5 + 60) * bonusDuration);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aIndex, TRUE);
	} else {
		for (int i = 0; i < MAX_USER_IN_PARTY; i++) {
			if (ApplyPartyIndex[i] != -1) {
				BUFF_AddBuffEffect(&gObj[ApplyPartyIndex[i]], BUFFTYPE_IMPROVED_LIGHTNING_REFLEXES, NULL, nEffectValue, 0, 0, ((lpObj->CharacterData->Energy + lpObj->AddEnergy) / 5 + 60) * bonusDuration);
				GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, ApplyPartyIndex[i], TRUE);
			}
		}
	}
}
void CMasterLevelSkillTreeSystem::ML_RF_MasteredReflexes(int aIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJ_USER && lpObj->m_RecallMon == -1) return;
	if (lpObj->Class != CLASS_RAGEFIGHTER) return;

	int number;
	int ApplyPartyIndex[MAX_USER_IN_PARTY];
	LPOBJ lpPartyObj;

	memset(ApplyPartyIndex, -1, sizeof(ApplyPartyIndex));

	int partynum = lpObj->PartyNumber;

	if (partynum != -1) {
		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			number = gParty.m_PartyS[partynum].Number[n];

			if (number >= 0) {
				lpPartyObj = &gObj[number];

				if (lpObj->MapNumber == lpPartyObj->MapNumber) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[number]) < 10) ApplyPartyIndex[n] = lpPartyObj->m_Index;
				}
			}
		}
	}

	int nEffectValue = (lpObj->CharacterData->Energy + lpObj->AddEnergy) / 50.0 + 10.0;

	if (nEffectValue > 100) nEffectValue = 100;

	nEffectValue += this->GetBrandOfMasterSkillValue(lpObj, lpMagic->m_Skill, 1);
	
	float fDefenseValue = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
	float bonusDuration = 1;
	int EffectIndex = EFFECTTYPE_NONE;

	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 569) == 20) bonusDuration += 0.2;
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, lpMagic->m_Skill) >= 10) EffectIndex = EFFECTTYPE_MASTERED_REFLEXES_RESIST;
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, lpMagic->m_Skill) == 20) EffectIndex = EFFECTTYPE_MASTERED_REFLEXES_MAX;

	if (partynum == -1) {
		BUFF_AddBuffEffect(lpObj, BUFFTYPE_IMPROVED_LIGHTNING_REFLEXES, EffectIndex, nEffectValue, EFFECTTYPE_INCREASE_DEFENSE, fDefenseValue, ((lpObj->CharacterData->Energy + lpObj->AddEnergy) / 5 + 60) * bonusDuration);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aIndex, TRUE);
		gObjCalCharacter.CHARACTER_Calc(lpObj->m_Index);
	} else {
		for (int i = 0; i < MAX_USER_IN_PARTY; i++) {
			if (ApplyPartyIndex[i] != -1) {
				BUFF_AddBuffEffect(&gObj[ApplyPartyIndex[i]], BUFFTYPE_IMPROVED_LIGHTNING_REFLEXES, EffectIndex, nEffectValue, EFFECTTYPE_INCREASE_DEFENSE, fDefenseValue, ((lpObj->CharacterData->Energy + lpObj->AddEnergy) / 5 + 60) * bonusDuration);
				GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, ApplyPartyIndex[i], TRUE);
				gObjCalCharacter.CHARACTER_Calc(ApplyPartyIndex[i]);
			}
		}
	}
}
void CMasterLevelSkillTreeSystem::ML_RF_ImprovedToughness(int aIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJ_USER && lpObj->m_RecallMon == -1) return;
	if (lpObj->Class != CLASS_RAGEFIGHTER) return;

	int number;
	int ApplyPartyIndex[MAX_USER_IN_PARTY];
	LPOBJ lpPartyObj;

	memset(ApplyPartyIndex, -1, sizeof(ApplyPartyIndex));

	int partynum = lpObj->PartyNumber;

	if (partynum != -1) {
		for (int n = 0; n < MAX_USER_IN_PARTY; n++) {
			number = gParty.m_PartyS[partynum].Number[n];

			if (number >= 0) {
				lpPartyObj = &gObj[number];

				if (lpObj->MapNumber == lpPartyObj->MapNumber) {
					if (CHARACTER_CalcDistance(lpObj, &gObj[number]) < 10) ApplyPartyIndex[n] = lpPartyObj->m_Index;
				}
			}
		}
	}

	int nEffectValue = ((lpObj->CharacterData->Energy + lpObj->AddEnergy) - 132) / 10.0 + 30.0;
	int EffectIndex = 0;
	int value = 0;

	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, lpMagic->m_Skill) >= 10) EffectIndex = EFFECTTYPE_IMPROVED_TOUGHNESS;
	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, lpMagic->m_Skill) == 20) value = 10;

	nEffectValue += this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);

	if (partynum == -1) {
		BUFF_AddBuffEffect(lpObj, BUFFTYPE_IMPROVED_TOUGHNESS, EFFECTTYPE_INCREASE_VITALITY_RF, nEffectValue, EffectIndex, value, lpObj->CharacterData->Energy / 5 + 60);
		GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aIndex, TRUE);
		gObjCalCharacter.CHARACTER_Calc(lpObj->m_Index);
	} else {
		for (int i = 0; i < MAX_USER_IN_PARTY; i++) {
			if (ApplyPartyIndex[i] != -1) {
				BUFF_AddBuffEffect(&gObj[ApplyPartyIndex[i]], BUFFTYPE_IMPROVED_TOUGHNESS, EFFECTTYPE_INCREASE_VITALITY_RF, nEffectValue, EffectIndex, value, lpObj->CharacterData->Energy / 5 + 60);
				GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, ApplyPartyIndex[i], TRUE);
				gObjCalCharacter.CHARACTER_Calc(ApplyPartyIndex[i]);
			}
		}
	}
}
void CMasterLevelSkillTreeSystem::ML_RF_ImprovedFocus(int aIndex, CMagicInf* lpMagic) {
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJ_USER) return;
	if (lpObj->Class != CLASS_RAGEFIGHTER) return;
	if (lpMagic == NULL) return;

	int nEffectValue = (lpObj->CharacterData->Energy + lpObj->AddEnergy) / 400.0 + 3.0;
	float bonusDuration = 1;

	if (g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, 577) == 20) bonusDuration += 0.3;
	if (lpObj->CharacterData->Mastery.ML_RF_ImprovedFocus > 0.0) nEffectValue += lpObj->CharacterData->Mastery.ML_RF_ImprovedFocus;
	if (nEffectValue > 15) nEffectValue = 15;

	BUFF_AddBuffEffect(lpObj, BUFFTYPE_FOCUS, EFFECTTYPE_IGNORE_OPPONENT_DEFENSE, nEffectValue, EFFECTTYPE_INCREASE_LETHAL_STRIKE_CHANCE, lpObj->CharacterData->Mastery.ML_RF_MasteredFocus, ((lpObj->CharacterData->Energy + lpObj->AddEnergy) / 5.00 + 60.00) * bonusDuration);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aIndex, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_RF_ImprovedWeaponSkill(int aIndex, CMagicInf* lpMagic, int aTargetIndex) {
	if (ObjectMaxRange(aTargetIndex) == FALSE) {
		g_Log.Add("[InvalidTargetIndex][MLS_SkillMonkBarrageJustOneTarget] Index :%d , AccountID : %s", aIndex, gObj[aIndex].AccountID);
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[aTargetIndex];
	BOOL EnableAttack = FALSE;
	int count = 0;
	int tObjNum;
	int HitCount;
	int nDistance = MagicDamageC.GetSkillDistance(lpMagic->m_Skill);
	int MLSkill = g_MasterLevelSkillTreeSystem.GetBaseMasterLevelSkill(lpMagic->m_Skill);

	if (lpMagic->m_Skill == 558 && g_MasterLevelSkillTreeSystem.ML_GetNodeLevel(lpObj, MLSkill) == 20) nDistance += 1;
	if (lpMagic->m_Skill == 563) {
		int iPoint = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);

		nDistance += iPoint / 10;
	}

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
		else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpTargetObj->MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
		else if (gclassObjAttack.PkCheck(lpObj, lpTargetObj) == TRUE) EnableAttack = TRUE;
	} else if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) {
		int CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
	
		if (gObjTargetGuildWarCheck(lpObj, &gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
		if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
	} else if (lpTargetObj->Type == OBJ_MONSTER || gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
	else if (tObjNum == lpObj->m_iDuelUser) EnableAttack = TRUE;

	if (EnableAttack == TRUE && this->ML_SKILL_CalcDistance(gObj[aTargetIndex].X, gObj[aTargetIndex].Y, gObj[tObjNum].X, gObj[tObjNum].Y) < nDistance) {
		switch (lpMagic->m_Skill) {
			case 551:		HitCount = 4;		break;
			case 554:		HitCount = 4;		break;
			case 552:		HitCount = 2;		break;
			case 555:		HitCount = 2;		break;
			case 558:
			case 562:		HitCount = 8;		break;
			case 559:
			case 563:		HitCount = 4;		break;
			default:		HitCount = 0;		break;
		}

		for (int i = 1; i <= HitCount; i++) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 1, 0, 0, i, 0);
	}
}
void CMasterLevelSkillTreeSystem::ML_RF_ImprovedMoltenStrike(int aIndex, CMagicInf *lpMagic, BYTE x, BYTE y, int aTargetIndex, int aDistance, BOOL bPVP, int nDelay) {
	LPOBJ lpObj = &gObj[aIndex];
	int DuelIndex = lpObj->m_iDuelUser;
	int nHitCount = 0;
	BOOL bAttack = FALSE;
	int nMinCount = 4;
	int nMaxCount = 8;
	BOOL EnableAttack = FALSE;
	int tObjNum;
	int count = 0;

	if (this->ML_SKILL_CalcDistance(lpObj->X, lpObj->Y, x, y) <= MagicDamageC.GetSkillDistance(lpMagic->m_Skill)) {
		while (true) {
			if (lpObj->VpPlayer2[count].state != 0) {
				tObjNum = lpObj->VpPlayer2[count].number;

				if (tObjNum >= 0) {
					EnableAttack = FALSE;

					if (lpObj->VpPlayer2[count].type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon < 0) EnableAttack = TRUE;
					else if ((tObjNum == aTargetIndex && bPVP == TRUE) || DuelIndex == tObjNum) EnableAttack = TRUE;
					else if (g_ConfigRead.server.GetServerType() == SERVER_CASTLE && lpObj->VpPlayer2[count].type != OBJ_NPC && gObj[tObjNum].MapNumber == MAP_INDEX_CASTLESIEGE && g_CastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE) EnableAttack = TRUE;
					else if (lpObj->VpPlayer2[count].type == OBJ_USER && tObjNum == aTargetIndex && bPVP == TRUE) {
						if (g_GensSystem.IsMapBattleZone(lpObj->MapNumber) == TRUE) EnableAttack = TRUE;
						else if (gclassObjAttack.PkCheck(lpObj, &gObj[aTargetIndex]) == TRUE) EnableAttack = TRUE;
					} else {
						int CallMonIndex = gObj[tObjNum].m_Index;

						if (gObj[tObjNum].Type == OBJ_MONSTER && gObj[tObjNum].m_RecallMon >= 0) CallMonIndex = gObj[gObj[tObjNum].m_RecallMon].m_Index;
						if (gObjTargetGuildWarCheck(lpObj,&gObj[CallMonIndex]) == TRUE) EnableAttack = TRUE;
						if (gObj[tObjNum].Class >= 678 && gObj[tObjNum].Class <= 680) EnableAttack = TRUE;
					}

					if (EnableAttack == TRUE) {
						if (gObj[tObjNum].X >= x - aDistance && gObj[tObjNum].X <= x + aDistance && gObj[tObjNum].Y >= y - aDistance && gObj[tObjNum].Y <= y + aDistance) {
							gObj[tObjNum].lpAttackObj = lpObj;
							nHitCount++;

							if (nHitCount <= nMinCount) bAttack = TRUE;
							else if (nHitCount > nMaxCount) bAttack = FALSE;
							else bAttack = rand() % 2;
							if (bAttack == TRUE) {
								if (nDelay) gObjAddAttackProcMsgSendDelay(lpObj, 50, tObjNum, nDelay, lpMagic->m_Skill, 0);
								else if (lpMagic->m_Skill != 560 && lpMagic->m_Skill != 561) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 0, 0, 0, 0, 0);
								else {
									for (int i = 1; i <= 4; i++) {
										if (i == 1) gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 1, 1, 0, 0, i, 0);
										else gObjAttack(lpObj, &gObj[tObjNum], lpMagic, 0, 1, 0, 0, i, 0);
									}
								}
							}
						}
					}
				}
			}

			count++;

			if (count > MAX_VIEWPORT-1) break;
		}
	}
}
void CMasterLevelSkillTreeSystem::ML_RF_Bloodlust(LPOBJ lpObj, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aTargetIndex) == false) return;

	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_BLOOD_LUST, 0, 0, 0, 0, MagicDamageC.GetSkillKeepTime(lpMagic->m_Skill));
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
}
void CMasterLevelSkillTreeSystem::ML_RF_ImprovedBloodlust(LPOBJ lpObj, int aTargetIndex, CMagicInf* lpMagic) {
	if (ObjectMaxRange(aTargetIndex) == false) return;

	LPOBJ lpTargetObj = &gObj[aTargetIndex];

	if (lpTargetObj->Type != OBJ_USER) return;

	BUFF_AddBuffEffect(lpTargetObj, BUFFTYPE_IMPROVED_BLOOD_LUST, 0, 0, 0, 0, MagicDamageC.GetSkillKeepTime(lpMagic->m_Skill));
	lpTargetObj->CharacterData->Mastery.ML_RF_ImprovedBloodlust = this->ML_GetNodeValue(MagicDamageC.SkillGet(lpMagic->m_Skill), lpMagic->m_Level);
	GSProtocol.PROTOCOL_MagicAttackNumberSend(lpObj, lpMagic->m_Skill, aTargetIndex, TRUE);
}