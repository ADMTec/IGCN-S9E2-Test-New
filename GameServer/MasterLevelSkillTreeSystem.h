#ifndef MASTERLEVELSKILLTREESYSTEM_H
#define MASTERLEVELSKILLTREESYSTEM_H

#include "StdAfx.h"
#include "user.h"
#include "classdef.h"
#include "MasterSkillSystem.h"
#include "MuLua.h"
#include "ObjAttack.h"

#define MAX_SKILL_POS 4
#define MAX_TREE_TYPE 3
#define MAX_MASTER_SKILL_TYPE 450
#define MAX_MASTER_SKILL_RANK 9

struct MLST_element {
	int		m_dwIndex;
	BYTE	m_byConsumePoint;
	BYTE	m_byMaxPoint;
	DWORD	m_dwParentSkill1;
	DWORD	m_dwParentSkill2;
	DWORD	m_dwSkillID;
};

struct MLSTree {
	MLST_element		iElement[MAX_MASTER_SKILL_RANK][MAX_SKILL_POS];
};

struct MLST_table {
	BYTE		btClass;
	BYTE		btType;
	BYTE		btWidth;
	MLSTree		iAbility[MAX_TREE_TYPE];
};

enum MLST_Value_Types {
	MLST_VALUE_NORMAL = 0,
	MLST_VALUE_DAMAGE = 1,
	MLST_VALUE_MANA_INC = 2
};

struct MLST_Value_Table {
	MLST_Value_Table();
	~MLST_Value_Table();

	void Init(int iMaxSkillLevel) {
		if (this->fValueTable != NULL) {
			delete[] this->fValueTable;
			this->fValueTable = NULL;
		}

		this->fValueTable = new float[iMaxSkillLevel + 1];
		this->iValueType = MLST_VALUE_NORMAL;

		for (int i = 0; i <= iMaxSkillLevel; i++) this->fValueTable[i] = 0.0;
	}

	int iValueType;
	float * fValueTable;
};

static const int CHARACTER_CLASS_TABLE[MAX_TYPE_PLAYER][2] = {
	2,		CLASS_WIZARD,
	1,		CLASS_KNIGHT,
	4,		CLASS_ELF,
	16,		CLASS_MAGICGLADIATOR,
	8,		CLASS_SUMMONER,
	32,		CLASS_DARKLORD,
	64,		CLASS_RAGEFIGHTER,
	128,	CLASS_GROWLANCER
};

struct PMSG_MASTERLEVELSKILLTREE_COUNT {
	PWMSG_HEAD2	h;
	int count;				// 8
};

struct PMSG_MASTERLEVELSKILLTREE {
	BYTE btMasterSkillUIIndex;
	BYTE btMasterSkillLevel;
	float fMasterSkillCurValue;
	float fMasterSkillNextValue;
	BYTE btUnk;
};

struct PMSG_ANS_MASTERLEVELSKILLTREE {
	PBMSG_HEAD2	h;
	BYTE btResult;			// 4
	WORD wMasterLevelPoint;		// 6
	BYTE btMasterSkillUIIndex;	// 8
	int dwMasterSkillIndex;		// C
	int dwMasterSkillLevel;			// 10
	float fMasterSkillCurValue;			// 14
	float fMasterSkillNextValue;		// 18
};


#pragma once
class CMasterLevelSkillTreeSystem : public CMasterLevelSystem {
	public:
		CMasterLevelSkillTreeSystem(void);
		virtual ~CMasterLevelSkillTreeSystem(void);
		friend CObjAttack;
		friend void gObjCharZeroSet(int aIndex);

		void ML_Init();
		void AddToValueTable(int iSkillValueID, const char * szFunctionName, int iSkillValueType);
		void ML_Load(const char *lpszFileName);
		void ML_InitPassiveSkillData(OBJECTSTRUCT* lpObj);
		void ML_CalcPassiveSkillData(OBJECTSTRUCT* lpObj, int iMLSkill, int iSkillLevel);
		int CheckSkillCondition(OBJECTSTRUCT *lpObj, int iMLSkill, int iSkillLevel);
		int CheckCompleteBrandOfSkill(OBJECTSTRUCT *lpObj, int iMLSkill);
		int CheckMasterLevelSkill(int iMLSkill);
		int IsExistMLSBasedSkill(OBJECTSTRUCT *lpObj, int iSkill);
		int GetBaseMasterLevelSkill(int iMLSkill);
		int gObjMagicAdd_MLS(OBJECTSTRUCT *lpObj, int iMLSkill, int iSkillLevel);
		void CGReqGetMasterLevelSkillTree(int aIndex);
		void CGReqGetMasterLevelSkill(PMSG_REQ_MASTERLEVEL_SKILL *lpMsg, int aIndex);
		int RunningSkill_MLS(OBJECTSTRUCT *lpObj, int aTargetIndex, CMagicInf *lpMagic, BOOL bCombo, BYTE x, BYTE y, BYTE aTargetPos);
		int CheckUsableWeaponSkill(OBJECTSTRUCT *lpObj, int nCount);
		int GetRequireMLPoint(int iClass, int iSkill);
		int GetMaxPointOfMasterSkill(int iClass, int iSkill);
		float ML_GetNodeValue(int iType, int iPoint);
		float GetMasterSkillManaIncRate(int iPoint);
		float GetBrandOfMasterSkillValue(OBJECTSTRUCT *lpObj, int iSkill, int iStep);
		int CheckPreviousRankSkill(OBJECTSTRUCT *lpObj, int iSkill, int iLevel);
		int ML_CheckNodeLevel(OBJECTSTRUCT *lpObj, int iSkill);
		int GetMasterSkillUIIndex(int iClass, int iSkill);
		int GetParentSkill1(int iClass, int iSkill);
		int GetParentSkill2(int iClass, int iSkill);
		void SetItemMLPassiveSkill(OBJECTSTRUCT *lpObj, int iType);
		void SetPetItemMLPassiveSkill(OBJECTSTRUCT *lpObj, int iType);
		void SetWingMLPassiveSkill(OBJECTSTRUCT *lpObj, int iType);
		int ML_GetNodeLevel(OBJECTSTRUCT *lpObj, int iSkill);
		int GetScriptClassCode(int iClass);
		float RoundValue(long double dbValue, int iPos);
		BYTE ResetMasterSkill(int aIndex, int nTreeType);
		BYTE CheckMasterSkillPoint(int aIndex, int nTreeType);
		BYTE CheckSkillInTree(int iClass, int iSkill, int iTreeKind);
		float GetSkillAttackDamage(OBJECTSTRUCT *lpObj, int iSkill);
		int ML_SKILL_CalcDistance(int x, int y, int x1, int y1);
		int MLS_GetAngle(int x, int y, int tx, int ty);
		void MLS_SkillFrustrum(int aIndex, float fangle, float tx, float ty);
		bool MLS_WeaponSkillDel(OBJECTSTRUCT* lpObj, WORD aSkill, BYTE Level);
		bool MLS_WeaponSkillAdd(OBJECTSTRUCT* lpObj, WORD aSkill, BYTE Level);

		// Skills
		void ML_DW_ImprovedManaShield(OBJECTSTRUCT *lpObj, int aTargetIndex, CMagicInf *lpMagic);
		void ML_ELF_ImprovedHeal(OBJECTSTRUCT *lpObj, int aTargetIndex, CMagicInf *lpMagic);
		void ML_DK_ImprovedTwistingSlash(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex, BOOL isCombo);
		void ML_DK_TornadoSlash(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex, BOOL isCombo);
		void ML_MG_TornadoSlash(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex, BOOL isCombo);
		void ML_DK_SteppingSlash(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex, BOOL isCombo);
		void ML_DK_LeapingSlash(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex, BOOL isCombo);
		void ML_DK_Stab(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex, BOOL isCombo);
		void ML_DK_ImprovedGroundslam(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, BYTE x, BYTE y, int aTargetIndex, int isCombo);
		void ML_DK_ImprovedLifeSwell(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic);
		void ML_DK_Innervation(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic);
		void ML_DK_Invigoration(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic);
		void ML_DW_ImprovedArcanePower(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex);
		void ML_DW_ArcaneFocus(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex);
		void ML_DW_ImprovedFlame(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex);
		void ML_DW_ImprovedLightning(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex);
		void ML_DW_SearingFlames(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic);
		void ML_DW_ImprovedHellfire(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic);
		void ML_DW_ImprovedPoison(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex);
		void ML_DW_Rot(int aIndex, CMagicInf *lpMagic, BYTE x, BYTE y, int aTargetIndex);
		void ML_DW_ImprovedIce(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex);
		void ML_DW_ManaBarrier(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex);
		void ML_ELF_ImprovedPenetration(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex);
		void ML_ELF_EternalArrow(int aIndex, int aTargetIndex, CMagicInf *lpMagic);
		void ML_SUM_ImprovedAmplifyDamage(int aIndex, CMagicInf *lpMagic);
		void ML_SUM_DimensionalAffinity(int aIndex, CMagicInf *lpMagic);
		void ML_SUM_VoidAffinity(int aIndex, CMagicInf *lpMagic);
		void ML_DW_ArcaneBlast(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex);
		void ML_DL_ImprovedConcentration(int aIndex, CMagicInf *lpMagic);
		void ML_DL_EnhancedConcentration(int aIndex, CMagicInf *lpMagic);
		void ML_DL_CriticalConcentration(int aIndex, CMagicInf *lpMagic);
		void ML_DL_ImprovedEarthquake(OBJECTSTRUCT *lpObj, int aTargetIndex, CMagicInf *lpMagic);
		void ML_DL_ImprovedFireburst(OBJECTSTRUCT *lpObj, int aTargetIndex, CMagicInf *lpMagic);
		void ML_DL_ImprovedForce(int aIndex, int aTargetIndex, CMagicInf *lpMagic);
		void ML_DW_ImprovedIceStorm(int aIndex, CMagicInf *lpMagic, BYTE x, BYTE y, int aTargetIndex);
		void ML_DK_ImprovedDeathStab(int aIndex, int aTargetIndex, CMagicInf *lpMagic, BOOL isCombo);
		void ML_DK_ImprovedRagefulBlow(int aIndex, CMagicInf *lpMagic, BYTE x, BYTE y, int aTargetIndex, BOOL isCombo);
		void ML_ELF_ImprovedEnhance(int aIndex, int aTargetIndex, CMagicInf *lpMagic);
		void ML_ELF_ImprovedEmpower(int aIndex, int aTargetIndex, CMagicInf *lpMagic);
		void ML_ELF_SpiritWard(int aIndex, int aTargetIndex, CMagicInf *lpMagic);
		void ML_ELF_Berserk(int aIndex, int aTargetIndex, CMagicInf *lpMagic);
		void ML_MG_ImprovedFireSlash(int aIndex, int aTargetIndex);
		void ML_MG_ImprovedPowerSlash(int aIndex, CMagicInf *lpMagic, BYTE x, BYTE y, BYTE Targetangle, int aTargetIndex);
		void ML_DL_ImprovedFireScream(int aIndex, int aTargetIndex, CMagicInf *lpMagic);
		void ML_RF_ImprovedWeaponSkill(int aIndex, CMagicInf *lpMagic, int aTargetIndex);
		void ML_RF_ImprovedMoltenStrike(int aIndex, CMagicInf *lpMagic, BYTE x, BYTE y, int aTargetIndex, int aDistance, BOOL bPVP, int nDelay);
		void ML_RF_ImprovedLightningReflexes(int aIndex, CMagicInf *lpMagic);
		void ML_RF_MasteredReflexes(int aIndex, CMagicInf *lpMagic);
		void ML_RF_ImprovedToughness(int aIndex, CMagicInf *lpMagic);
		void ML_RF_ImprovedFocus(int aIndex, CMagicInf *lpMagic);
		void ML_DK_BladeStorm(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, BYTE x, BYTE y, int aTargetIndex, BOOL isCombo);
		void ML_DW_EarthenPrison(OBJECTSTRUCT *lpObj, int aTargetIndex, CMagicInf *lpMagic);
		void ML_ELF_PoisonArrow(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex, BOOL isCombo);
		void ML_ELF_Cure(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex);
		void ML_ELF_SummonSatyr(int aIndex, int MonsterType, int x, int y);
		void ML_ELF_PrayerOfHealing(int aIndex, int aTargetIndex, CMagicInf *lpMagic);
		void ML_ELF_Bless(OBJECTSTRUCT *lpObj, int aTargetIndex, CMagicInf *lpMagic);
		void ML_ELF_ImprovedBless(OBJECTSTRUCT *lpObj, int aTargetIndex, CMagicInf *lpMagic);
		void ML_SUM_Blind(OBJECTSTRUCT *lpObj, int aTargetIndex, CMagicInf *lpMagic);
		void ML_DL_Stoneskin(OBJECTSTRUCT *lpObj, int aTargetIndex, CMagicInf *lpMagic);
		void ML_DL_ImprovedStoneskin(OBJECTSTRUCT *lpObj, int aTargetIndex, CMagicInf *lpMagic);
		void ML_RF_Bloodlust(OBJECTSTRUCT *lpObj, int aTargetIndex, CMagicInf *lpMagic);
		void ML_RF_ImprovedBloodlust(OBJECTSTRUCT *lpObj, int aTargetIndex, CMagicInf *lpMagic);
		void ML_DK_PuncturingStab(int aIndex, int aTargetIndex, CMagicInf *lpMagic, BOOL isCombo);
		void ML_DK_DeathBlow(int aIndex, int aTargetIndex, CMagicInf *lpMagic, BOOL isCombo);
		void ML_DK_EarthShake(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, BYTE x, BYTE y, int aTargetIndex, BOOL isCombo);
		void ML_DK_ArcticBlow(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, BYTE x, BYTE y, int aTargetIndex, BOOL isCombo);
		void ML_DW_Blizzard(int aIndex, CMagicInf *lpMagic, BYTE x, BYTE y, int aTargetIndex);
		void ML_ELF_ImprovedBarrage(int aIndex, CMagicInf *lpMagic, BYTE bAngle, int aTargetIndex);
		void ML_DW_Supernova(int aIndex, int aTargetIndex, CMagicInf *lpMagic);
		void ML_DW_SupernovaStart(int aIndex, CMagicInf *lpMagic);
		void ML_DW_SoulBarrier(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex);
		void ML_SUM_ImprovedEnfeeble(int aIndex, CMagicInf *lpMagic, BYTE x, BYTE y, int aTargetIndex);
		void ML_SUM_ImprovedWeakness(int aIndex, CMagicInf *lpMagic, BYTE x, BYTE y, int aTargetIndex);
		void ML_MG_Freeze(OBJECTSTRUCT *lpObj, CMagicInf *lpMagic, int aTargetIndex);
		void ML_MG_ImprovedFlameStrike(int aIndex, CMagicInf *lpMagic, BYTE bAngle, int aTargetIndex);
		void ML_MG_ImprovedThunderStorm(int aIndex, CMagicInf *lpMagic, BYTE x, BYTE y, int aTargetIndex);
		void ML_MG_SearingSlash(int aIndex, int aTargetIndex, CMagicInf *lpMagic);
		void ML_DL_ExcellentConcentration(int aIndex, CMagicInf *lpMagic);
		void MLS_SkillFrustrum3(int aIndex, BYTE bAngle, float tx1, float ty1, float tx2, float ty2);
		void ML_DL_ImprovedSpark(int aIndex, CMagicInf *lpMagic, BYTE x, BYTE y, BYTE TargetPos, int aTargetIndex);
		void ML_DL_ImprovedDarkness(int aIndex, CMagicInf *lpMagic, BYTE bAngle, int aTargetIndex);
		void ML_SUM_ImprovedSleep(int aIndex, int aTargetIndex, CMagicInf *lpMagic);
		void ML_SUM_ImprovedChainLightning(int aIndex, int aTargetIndex, CMagicInf *lpMagic);
		void ML_SUM_ImprovedLightningBlast(int aIndex, int aTargetIndex, CMagicInf *lpMagic);
		void ML_SUM_ImprovedDrainLife(int aIndex, int aTargetIndex, CMagicInf *lpMagic);

		int GetMaxMasterSkillLevel() { 
			return this->m_wMaxMasterSkillLevel;
		}

	private:
		MLST_table			m_MLSTable[MAX_TYPE_PLAYER];
		MLST_Value_Table	* m_MLSValueTable; //[MAX_MASTER_SKILL_TYPE];
		MULua				m_Lua;
		WORD				m_wMaxMasterSkillLevel; // private custom
};

extern CMasterLevelSkillTreeSystem g_MasterLevelSkillTreeSystem;

#endif