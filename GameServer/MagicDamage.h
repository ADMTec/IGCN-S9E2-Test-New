#ifndef MAGICDAMAGE_H
#define MAGICDAMAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_SKILL			800
#define MAX_REQ_SKILL_STATUS	3

#include "classdef.h"

enum eSkillType {
	DW_POISON = 0x1,
	DW_METEOR = 0x2,
	DW_LIGHTNING = 0x3,
	DW_FIREBALL = 0x4,
	DW_FLAME = 0x5,
	DW_TELEPORT = 0x6,
	DW_ICE = 0x7,
	DW_TWISTER = 0x8,
	DW_EVILSPIRITS = 0x9,
	DW_HELLFIRE = 0xa,
	DW_POWERWAVE = 0xb,
	DW_AQUABEAM = 0xc,
	DW_BLAST = 0xd,
	DW_INFERNO = 0xe,
	DW_TELEPORTALLY = 0xf,
	DW_MANASHIELD = 0x10,
	DW_ENERGYBALL = 0x11,
	SHIELD_BLOCK = 0x12,
	WEAPON_FALLINGSLASH = 0x13,
	WEAPON_LUNGE = 0x14,
	WEAPON_UPPERCUT = 0x15,
	WEAPON_CYCLONESLASH = 0x16,
	WEAPON_FORWARDSLASH = 0x17,
	WEAPON_SKILL_CROSSBOW = 0x18,
	WEAPON_SKILL_BOW = 0x19,
	ELF_HEAL = 0x1a,
	ELF_ENHANCE = 0x1b,
	ELF_EMPOWER = 0x1c,
	ELF_SUMMON_GOBLIN = 0x1e,
	ELF_SUMMON_STONEGOLEM = 0x1f,
	ELF_SUMMON_ASSASSIN = 0x20,
	ELF_SUMMON_GIANTYETI = 0x21,
	ELF_SUMMON_DARKKNIGHT = 0x22,
	ELF_SUMMON_BALI = 0x23,
	ELF_SUMMON_SOLDIER = 0x24,
	DK_TWISTINGSLASH = 0x29,
	DK_RAGEFULBLOW = 0x2a,
	DK_DEATHSTAB = 0x2b,
	MOUNT_IMPALE = 0x2f,
	DK_LIFESWELL = 0x30,
	MOUNT_SCORCHINGBREATH = 0x31,
	ELF_ICEARROW = 0x33,
	ELF_PENETRATION = 0x34,
	MG_FIRESLASH = 0x37,
	MG_POWERSLASH = 0x38,
	WEAPON_SKILL_CROSSBOW2 = 0x36,
	DW_DECAY = 0x26,
	DW_ICESTORM = 0x27,
	DW_NOVA = 0x28,
	DW_NOVA_START = 0x3a,
	AT_SKILL_IMPROVE_AG_REFILL = 0x35,
	AT_SKILL_DEVILFIRE = 0x32,
	DK_COMBO = 0x3b,
	DL_FORCE = 0x3c,
	DL_FIREBURST = 0x3d,
	DL_EARTHQUAKE = 0x3e,
	DL_SUMMON = 0x3f,
	DL_CONCENTRATION = 0x40,
	DL_SPARK = 0x41,
	AT_SKILL_LONGSPEAR = 0x42,
	DK_SUNWAVESLASH = 0x2c,
	DW_SOULGLAIVES = 0x2d,
	ELF_STARFALL = 0x2e,
	MG_SPIRALSLASH = 0x39,
	MG_MANARAYS = 0x49,
	DL_PILLARSOFFIRE = 0x4a,
	SIEGE_BRAND_OF_SKILL = 0x4b,
	SIEGE_STUN = 0x43,
	SIEGE_REMOVESTUN = 0x44,
	SIEGE_SWELLMANA = 0x45,
	SIEGE_INVISIBILITY = 0x46,
	SIEGE_REMOVEINVISIBILITY = 0x47,
	SIEGE_ABOLISHMAGIC = 0x48,
	FENRIR_SKILL = 0x4c,
	ELF_INFINITYARROW = 0x4d,
	DL_FIRESCREAM = 0x4e,
	AT_SKILL_EXPLOSION = 0x4f,
	KUNDUN_SUMMON = 0xc8,
	AT_SKILL_IMMUNE_TO_MAGIC = 0xc9,
	AT_SKILL_IMMUNE_TO_HARM = 0xca,
	AT_SKILL_ILLUSION_SHIELD = 0xd2,
	AT_SKILL_ILLUSION_PARALYZE = 0xd3,
	AT_SKILL_ILLUSION_TELEPORT = 0xd4,
	AT_SKILL_ILLUSION_ATTACK = 0xd5,
	SUM_DRAIN_LIFE = 0xd6,
	SUM_CHAINLIGHTNING = 0xd7,
	SUM_MAGICMIRROR = 0xd9,
	SUM_AMPLIFYDAMAGE = 0xda,
	SUM_SLEEP = 0xdb,
	SUM_ENFEEBLE = 0xdd,
	SUM_WEAKNESS = 0xde,
	SUM_SPIRIT_OF_FLAMES = 0xdf,
	SUM_SPIRIT_OF_ANGUISH = 0xe0,
	SUM_SPIRIT_OF_TERROR = 0xe1,
	SUM_LIGHTNINGBLAST = 0xe6,
	DK_GROUNDSLAM = 0xe8,
	DW_ARCANEPOWER = 0xe9,
	ELF_SHIELDRECOVERY = 0xea,
	ELF_FIVESHOT = 0xeb,
	MG_FLAMESTRIKE = 0xec,
	MG_THUNDERSTORM = 0xed,
	DL_DARKNESS = 0xee,
	MONSTER_SUICIDE = 0xef,
	RF_FISTFLURRY = 0x104,
	RF_BEASTUPPERCUT = 0x105,
	RF_CHAINDRIVE = 0x106,
	RF_DARKSIDE = 0x107,
	RF_MOLTENSTRIKE = 0x108,
	RF_DRAGONFIST = 0x109,
	RF_FOCUS = 0x10a,
	RF_TOUGHNESS = 0x10b,
	RF_LIGHTNINGREFLEXES = 0x10c,
	RF_CHARGE = 0x10d,
	RF_PHOENIXSHOT = 0x10e,
	AT_SKILL_SPIN_STEP = 0x10f,
	AT_SKILL_CIRCLE_SHIELD = 0x110,
	AT_SKILL_OBSIDIAN = 0x111,
	AT_SKILL_MAGIC_PIN = 0x112,
	AT_SKILL_CLASH = 0x113,
	AT_SKILL_HARSH_STRIKE = 0x114,
	AT_SKILL_SHINING_PEAK = 0x115,
	AT_SKILL_WRATH = 0x116,
	AT_SKILL_BRECHE = 0x117,
	AT_SKILL_GL_EXPLOSION = 0x118,
	AT_SKILL_MAGIC_PIN_EXPLOSION = 0x119,
	MONSTER_LORDSILVESTER_SUMMON = 0x26e
};

struct MASTER_SKILL_INFO {
	int SkillID;
	int Rank;
	int Group;
	int MasterPoint;
	int SkillType;
	int Effect;
};

class CMagicDamage {

private:

	int Get(int skill);
	void SetEx(int iSkill, int iSkillUseType, int iSkillBrand, int iKillCount, int* pReqStatus);
	void Set(char* name, int skill, int damage,  int rlevel,  int mana,  int bp,  int dis, int Attr,  int iSkillType, unsigned char* ReqClass, int rEnergy, int rLeadership, int rdelay, BYTE byItemSkill, BYTE byItemDamage, BYTE byBufIndex);
	void SetMasterLevelSkillInfo(int iSkill, int iSkillRank, int iSkillGroup, int iRequireMLPoint, int iHP, int iSD, WORD wKeepTime, int iRequireStr, int iRequireDex);

public:

	CMagicDamage();
	virtual ~CMagicDamage();

	void Init();
	void LogSkillList(char* filename);
	int SkillGet(int skill);
	int MAGIC_GetManaUsage(int skill);
	int MAGIC_GetRequireEnergy(int skill);
	int SkillGetRequireLevel(int skill);
	int MAGIC_GetRequireClass(int Cclass, int ChangeUP, int skill);
	int MAGIC_GetStamUsage(int skill);
	int MAGIC_GetSkillAttr(int skill);
	int MAGIC_GetRequireLeadership(int skill);
	int GetDelayTime(int skill);
	int GetSkillType(int iSkill);
	int CheckStatus(int iSkill, int iGuildStatus);
	int CheckBrandOfSkill(int iSkill);
	int CheckKillCount(int iSkill, int iKillCount);
	int GetSkillDistance(int skill);
	int GetRequireMLPoint(int skill);
	int GetRequireStrength(int skill);
	int GetRequireDexterity(int skill);
	WORD GetBrandOfSkill(int iSkill);
	WORD GetNormalBrandOfSkill(int iSkill);
	BYTE IsItemAttachedSkill(int iSkill);
	int GetSkillUseType(int iSkill);
	int GetSkillLevel(int iSkill);
	int GetSkillGroup(int iSkill);
	int GetSkillRank(int iSkill);
	int GetSkillKeepTime(int iSkill);
	char m_Name[MAX_SKILL][50];	// 404

private:
	int m_Damage[MAX_SKILL];	// 4
	unsigned char m_rLevel[MAX_SKILL];	// 3604
	unsigned short m_Mana[MAX_SKILL];	// 3704
	unsigned char m_Distance[MAX_SKILL];	// 3904
	int m_RequireEnergy[MAX_SKILL];	// 3A04
	unsigned char m_RequireClass[MAX_SKILL][MAX_TYPE_PLAYER+1];	// 3E04
	int m_RequireLevel[MAX_SKILL];	// 4304
	int m_RequireLeadership[MAX_SKILL];	// 4704
	int m_Delay[MAX_SKILL];	// 4B04
	WORD m_BrainPower[MAX_SKILL];	// 4F04
	int m_Attr[MAX_SKILL];	// 5104
	int m_iSkillType[MAX_SKILL];	// 5504
	int m_iSkillUseType[MAX_SKILL];	// 5904
	int m_iSkillBrand[MAX_SKILL];	// 5D04
	int m_iKillCount[MAX_SKILL];	// 6104
	int m_iRequireStatus[MAX_SKILL][MAX_REQ_SKILL_STATUS];	// 6504
	WORD m_iSkillRank[MAX_SKILL];
	WORD m_iSkillGroup[MAX_SKILL];
	WORD m_nRequirePoint[MAX_SKILL];
	WORD m_iHP[MAX_SKILL];
	WORD m_iSD[MAX_SKILL];
	WORD m_wKeepTime[MAX_SKILL];
	WORD m_iRequireStr[MAX_SKILL];
	WORD m_iRequireDex[MAX_SKILL];
	BYTE m_byItemAttachedSkill[MAX_SKILL];
	BYTE m_byIsDamage[MAX_SKILL];
	BYTE m_byBufIndex[MAX_SKILL];

	enum {
		SKILL_TYPE_NONE = 0xff,
		SKILL_TYPE_PHYSICS = 0x0,
		SKILL_TYPE_MAGIC = 0x1,
		SKILL_TYPE_PASSIVE = 0x2,
		SKILL_TYPE_MASTER_PASSIVE = 0x3,
		SKILL_TYPE_MASTER_MAGIC = 0x4,
	};

};

extern CMagicDamage MagicDamageC;

#endif