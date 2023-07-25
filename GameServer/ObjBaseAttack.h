#ifndef OBJBASEATTACK_H
#define OBJBASEATTACK_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "user.h"


class CObjBaseAttack
{

public:

	CObjBaseAttack();
	virtual ~CObjBaseAttack();

	int ATTACK_DecreaseArrow(LPOBJ lpObj);
	int CheckAttackArea(LPOBJ lpObj, LPOBJ lpTargetObj);
	int PkCheck(LPOBJ lpObj, LPOBJ lpTargetObj);
	void ATTACK_SkillDebuffProc(LPOBJ lpObj, LPOBJ lpTargetObj, int skill, int skillLevel, int AttackDamage, int cSkill);
	int ATTACK_MissCheckPvP(LPOBJ lpObj, LPOBJ lpTargetObj, int skill, int skillSuccess, int magicsend, BOOL& bAllMiss, BYTE RFAttack);
	int ATTACK_CalcTargetDefense(LPOBJ lpObj, LPOBJ lpTargetObj, int& MsgDamage, int& iOriginTargetDefense, int skill);
	int GetPartyMemberCount(LPOBJ lpObj);
	void ATTACK_GetMonsterAttackEffect(LPOBJ lpObj, int& skillAttr, int& Type, int& Duration);

	virtual int ATTACK_MissCheck(LPOBJ lpObj, LPOBJ lpTargetObj, int skill, int skillSuccess, int magicsend, BOOL& bAllMiss, BYTE RFAttack);	// 4


};

#endif