#ifndef OBJUSESKILL_H
#define OBJUSESKILL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MagicInf.h"
#include "user.h"
#include "MuLua.h"

struct SkillConfig {
	// Default
	int SternTime;
	int SwellManaTime;
	int CloakingTime;
	int IceArrowTime;

	bool EnableWizardSkillWhileTeleport;
	bool EnableSiegeOnAllMaps;

	// Elemental Ailments System
	int IceArrowSuccessRate;
	int IceSpellSuccessRate;

	int PoisonDamageLifePercent;
	int PoisonDebuffTime;
	int PoisonSuccessRate;

	int LightningDefenseReducePercent;
	int LightningDebuffTime;
	int LightningSuccessRate;

	int BurnDamagePercent;
	int BurnDebuffTime;
	int BurnSuccessRate;

	int EarthMaxLifeReducePercent;
	int EarthDebuffTime;
	int EarthSuccessRate;

	int WindSuccessRate;

	int WaterMaxManaReducePercent;
	int WaterDebuffTime;
	int WaterSuccessRate;
};

class CObjUseSkill {
	public:
		CObjUseSkill();
		virtual ~CObjUseSkill();
		friend class CObjAttack;

		bool LoadSkillInfo();

		int SpeedHackCheck(int aIndex);
		int EnableSkill(WORD Skill);
		int GetUseMana(int aIndex, CMagicInf* lpMagic);
		int ELF_InfinityArrowManaUsage(LPOBJ lpObj, CMagicInf* lpMagic);
		int GetUseStamina(int aIndex, CMagicInf* lpMagic);
		void UseSkill(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		void UseSkill(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, BYTE dir, BYTE TargetPos, int aTargetIndex);
		int RunningSkill(int aIndex, int aTargetIndex, CMagicInf* lpMagic, BOOL bCombo);
		void MG_FireSlash(int aIndex, int aTargetIndex, int skill_level);
		void DK_LifeSwell(int aIndex, CMagicInf * lpMagic);
		void DW_ManaShield(int aIndex, int aTargetIndex, CMagicInf * lpMagic);
		void ELF_Enhance(int aIndex, int aTargetIndex, CMagicInf * lpMagic);
		void ELF_Empower(int aIndex, int aTargetIndex, CMagicInf * lpMagic);
		int SkillChangeUse(int aIndex);
		void ELF_Heal(int aIndex, int aTargetIndex, CMagicInf * lpMagic);
		int ELF_Summon(int aIndex, int MonsterType, int x, int y);
		int DW_Decay(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, BYTE dir, BYTE TargetPos, int aTargetIndex);
		int MEDUSA_Decay(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex);
		int DW_IceStorm(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, BYTE dir, BYTE TargetPos, int aTargetIndex);
		int DW_NovaStart(int aIndex, CMagicInf* lpMagic);
		int DW_Nova(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		void DK_RagefulBlow(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, BYTE dir, BYTE TargetPos, int aTargetIndex, BOOL isCombo);
		void DK_TwistingSlash(int aIndex, CMagicInf* lpMagic, int aTargetIndex, BOOL isCombo);
		int DK_DeathStab(int aIndex, int aTargetIndex, CMagicInf* lpMagic, BOOL isCombo);
		void MG_PowerSlash(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, BYTE dir, BYTE Targetangle, int aTargetIndex);
		void SkillFrustrum(int aIndex, float fangle, float tx, float ty);
		int GetTargetLinePath(int sx, int sy, int tx, int ty,  int* PathX,int* PathY, int distance);
		int GetAngle(int x, int y, int tx,  int ty);
		int SKILL_CalcDistance(int x, int y, int x1, int y1);
		void SkillFrustrum2(int aIndex, float fangle, float tx, float ty);
		void SkillFrustrum3(int aIndex, float fangle, float tx, float ty, float f1, float f2);
		int SkillAreaCheck(int angel, int x, int y,  int tx, int ty,  int dis,  int arc);
		int DL_Force(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int DL_FireBurst(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int DL_Earthquake(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int DL_SummonCheck(int aIndex, int skill_level);
		void DL_Summon(int aIndex, int skill_level);
		void DL_Concentration(int aIndex, int skill_level, CMagicInf* lpMagic);
		void DL_Spark(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, BYTE dir, BYTE TargetPos, int aTargetIndex);
		int KUNDUN_Summon(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int SkillImmuneToMagic(int aIndex, CMagicInf* lpMagic);
		int SkillImmuneToHarm(int aIndex, CMagicInf* lpMagic);
		int DK_SunwaveSlash(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int DW_SoulGlaives(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int ELF_Starfall(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int MG_SpiralSlash(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int MG_ManaRays(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int DL_PillarsOfFire(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int SIEGE_SkillBrandOfSkill(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int SIEGE_SkillRemoveStun(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int SIEGE_SkillAddMana(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int SIEGE_SkillCloaking(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int SIEGE_SkillRemoveCloaking(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int SIEGE_SkillRemoveMagic(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int SIEGE_RemoveCloakingEffect(int aIndex);
		int SIEGE_SkillStun(int aIndex, int aTargetIndex, CMagicInf* lpMagic, BYTE btX, BYTE btY, BYTE btDir, BYTE btTargetPos);
		int RemoveAllCharacterInvalidMagicAndSkillState(LPOBJ lpObj);
		int ELF_RemoveBuff(LPOBJ lpObj, int iSkillNumber);
		int FENRIR_PlasmaStorm(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int ELF_InfinityArrow(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int DL_FireScream(int aIndex, int aTargetIndex, CMagicInf* lpMagic);
		int DL_FireScreamExplosion(LPOBJ lpObj, LPOBJ lpTargetObj, int iAttackDamage, int iAddDamage);
		void ML_BurningEchoes(LPOBJ lpObj, LPOBJ lpTargetObj, CMagicInf* lpMagic, int iAttackDamage);
		int SUM_SpiritOfFlames(int aIndex, CMagicInf * lpMagic, BYTE x, BYTE y, int aTargetIndex);
		int SUM_Sleep(int aIndex, int aTargetIndex, CMagicInf * lpMagic);
		int SUM_DrainLife(int aIndex, CMagicInf * lpMagic, int aTargetIndex);
		int SUM_MagicMirror(int aIndex, CMagicInf * lpMagic, int aTargetIndex);
		int SUM_SpiritOfAnguish(int aIndex, CMagicInf * lpMagic, BYTE x, BYTE y, int aTargetIndex);
		int SUM_Weakness(int aIndex, CMagicInf * lpMagic, BYTE x, BYTE y, int aTargetIndex);
		int SUM_Enfeeble(int aIndex, CMagicInf * lpMagic, BYTE x, BYTE y, int aTargetIndex);
		int SUM_ChainLighting(int aIndex, CMagicInf * lpMagic, int aTargetIndex);
		int DK_Groundslam(int aIndex, CMagicInf * lpMagic, BYTE x, BYTE y, int aTargetIndex, BOOL bCombo);
		int SKILL_AreaAttack(int aIndex, CMagicInf * lpMagic, BYTE x, BYTE y, int aTargetIndex, int Distance, int MultiAttack, int AttackDelay);
		void SUM_LightningBlast(int aIndex, CMagicInf * lpMagic, int aTargetIndex);
		int DW_ArcanePower(int aIndex, int aTargetIndex, CMagicInf * lpMagic);
		int ELF_ShieldRecovery(int aIndex, CMagicInf * lpMagic, int aTargetIndex);
		int SUM_AmplifyDamage(int aIndex, CMagicInf * lpMagic, int aTargetIndex);
		int MG_ThunderStorm(int aIndex, CMagicInf * lpMagic, BYTE x, BYTE y, int aTargetIndex);
		int ELF_FiveShot(int aIndex, CMagicInf * lpMagic, BYTE TargetPos, int aTargetIndex);
		int DL_Darkness(int aIndex, CMagicInf * lpMagic, BYTE TargetPos, int aTargetIndex);
		int MG_FlameStrike(int aIndex, CMagicInf * lpMagic, BYTE TargetPos, int aTargetIndex);
		void SUM_SpiritOfTerror(int aIndex, CMagicInf * lpMagic, BYTE x, BYTE y, int aTargetIndex, int aMsgSubCode, BOOL bCombo);
		int RF_Focus(int aIndex, CMagicInf * lpMagic);
		int RF_BuffParty(int aIndex, CMagicInf * lpMagic);
		int RF_WeaponSkill(int aIndex, CMagicInf *lpMagic, int aTargetIndex);
		int RF_Charge(int aIndex, CMagicInf *lpMagic, int aTargetIndex);
		int RF_DarkSideGetTargetIndex(int aIndex, int aTargetIndex, CMagicInf* lpMagic, WORD *Target);
		void RF_DarkSide(int aindex, int aTargetIndex, CMagicInf * lpMagic);
		int RF_PhoenixShot(int aIndex, CMagicInf * lpMagic, BYTE x, BYTE y, int aTargetIndex);
		int EvilMonsterSkill(int aIndex, int aTargetIndex, CMagicInf *lpMagic);
		void SplashDamage(LPOBJ lpObj, LPOBJ lpTargetObj, int nSkill, int nAttackDamage, int nDistance, int nDamageRate);
		void SILVESTER_Summon(int aIndex, int iTargetIndex, CMagicInf *lpMagic);

		void SkillSpinStep(int aIndex, int iTargetIndex, CMagicInf *lpMagic);
		void SkillGrowLancerAttackOneTarget(int aIndex, int aTargetIndex, CMagicInf *lpMagic);
		int SkillBreche(int aIndex, CMagicInf* lpMagic, BYTE x, BYTE y, int aTargetIndex);
		int SkillShiningPeak(int aIndex, CMagicInf *lpMagic, int aTargetIndex);
		int SkillBuffGrowLancer(int aIndex, CMagicInf *lpMagic, int aTargetIndex);
		int SkillBuffPartyGrowLancer(int aIndex, CMagicInf *lpMagic, int aTargetIndex);

	SkillConfig	m_SkillData;
	MULua m_Lua;
};

int DL_SparkDrain (LPOBJ lpObj);
extern CObjUseSkill gObjUseSkill;
#endif