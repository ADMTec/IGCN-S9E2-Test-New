#ifndef OBJATTACK_H
#define OBJATTACK_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MagicInf.h"
#include "ObjBaseAttack.h"

class CObjAttack : public CObjBaseAttack
{

public:

	CObjAttack();
	virtual ~CObjAttack();

	void Init();

	BOOL ATTACK_Normal(LPOBJ lpObj, LPOBJ lpTargetObj, CMagicInf* lpMagic,  int magicsend, unsigned char MSBFlag, int AttackDamage, BOOL bCombo, BYTE RFAttack, BYTE byReflect, BYTE byPentagramAttack);
	BOOL ATTACK_ElementalSeal(LPOBJ lpObj, LPOBJ lpTargetObj, CMagicInf* lpMagic, BYTE MSBFlag, int MsgDamage, int AttackDamage, int iTargetDefense);
	int ATTACK_CalcPhysicalDamage(LPOBJ lpObj, LPOBJ lpTargetObj, int targetDefense, CMagicInf* lpMagic, float& charIncPercent, float& buffDecPercent, int& effect);
	int ATTACK_CalcMagicDamage(LPOBJ lpObj, LPOBJ lpTargetObj, int targetDefense, CMagicInf* lpMagic, float& itemIncPercent, int& effect);
	int ATTACK_CalcShieldDamage(LPOBJ lpObj, LPOBJ lpTargetObj, int iAttackDamage);
	int ATTACK_CalcSummonerBuff(LPOBJ lpObj, int *iAttackBerserkerMin, int *iAttackBerserkerMax, int nBuffType);
	int ATTACK_CalcElementalDamage(LPOBJ lpObj, LPOBJ lpTargetObj, char* DamageType1, char* DamageType2, int iAttackDamage, int iTargetDefense);

private:
	MULua m_Lua;

};

BOOL CHECK_Dinorant(LPOBJ lpObj);
BOOL CHECK_DarkHorse(LPOBJ lpObj);
BOOL CHECK_Fenrir(LPOBJ lpObj);

extern CObjAttack gclassObjAttack;

#endif