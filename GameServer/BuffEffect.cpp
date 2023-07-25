// BuffEffect.cpp: implementation of the CBuffEffect class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "User.h"
#include "BuffEffect.h"
#include "BuffEffectSlot.h"
#include "Prodef.H"
#include "Winutil.h"
#include "Protocol.h"
#include "GameMain.h"
#include "PeriodItemEx.h"
#include "TLog.h"
#include "ArcaBattle.h"
#include "configread.h"
#include "ObjUseSkill.h"

CBuffEffect	g_BuffEffect;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBuffEffect::CBuffEffect() {}
CBuffEffect::~CBuffEffect() {}

void CBuffEffect::BUFFEFFECT_SetEffect(LPOBJECTSTRUCT lpObj, BYTE EffectType, int EffectValue) {
	if (lpObj == NULL || EffectType < EFFECTTYPE_NONE) return;
	if (lpObj->Connected < PLAYER_PLAYING) return;
	switch (EffectType) {
		case EFFECTTYPE_LIFE_REGENERATION:
			lpObj->LifeRegeneration += EffectValue;
		break;

		case EFFECTTYPE_WATER_DEBUFF:
			lpObj->AddMana -= EffectValue;

			if (lpObj->Mana > (float)(lpObj->AddMana) + (float)(lpObj->MaxMana)) {
				lpObj->Mana = (float)(lpObj->AddMana) + (float)(lpObj->MaxMana);
				GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, lpObj->Mana, 0xFF, 0, lpObj->Stamina);
			}

			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, (lpObj->MaxMana + lpObj->AddMana), 0xFE, 0, (lpObj->MaxStamina + lpObj->AddStamina));
		break;

		case EFFECTTYPE_LIGHTNING_DEBUFF:
			lpObj->Defense -= EffectValue;
			lpObj->m_MagicDefense -= EffectValue;
			lpObj->m_iPentagramDefense -= EffectValue;
		break;

		case EFFECTTYPE_EARTH_DEBUFF:
			lpObj->AddLife -= EffectValue;

			if (lpObj->Life > (float)(lpObj->AddLife) + (float)(lpObj->MaxLife)) {
				lpObj->Life = (float)(lpObj->AddLife) + (float)(lpObj->MaxLife);
				GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
			}

			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->AddLife + lpObj->MaxLife, 0xFE, 0, lpObj->iAddShield + lpObj->iMaxShield);
		break;

		case EFFECTTYPE_INCREASE_ATTACKSPEED:
			lpObj->AttackSpeed += EffectValue;
			lpObj->MagicSpeed += EffectValue;
		break;

		case EFFECTTYPE_INCREASE_DEFENSE:
			lpObj->Defense += EffectValue;
			lpObj->m_MagicDefense += EffectValue;
		break;

		case EFFECTTYPE_INCREASE_LIFE:
			lpObj->AddLife += EffectValue;	
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->AddLife+lpObj->MaxLife, 0xFE, 0, lpObj->iAddShield+lpObj->iMaxShield);
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
		break;

		case EFFECTTYPE_INCREASE_MANA:
			lpObj->AddMana += EffectValue;	
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, (lpObj->MaxMana + lpObj->AddMana), 0xFE, 0, (lpObj->MaxStamina+lpObj->AddStamina));
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, lpObj->Mana, 0xFF, 0, lpObj->Stamina);
		break;

		case EFFECTTYPE_INCREASE_STRENGTH:
			lpObj->AddStrength += EffectValue;
		break;

		case EFFECTTYPE_INCREASE_DEXTERITY:
			EnterCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
			lpObj->CharacterData->AgilityCheckDelay = GetTickCount();
			lpObj->AddDexterity += EffectValue;
			LeaveCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
		break;

		case EFFECTTYPE_INCREASE_VITALITY:
			lpObj->AddVitality += EffectValue;
		break;

		case EFFECTTYPE_INCREASE_ENERGY:
			lpObj->AddEnergy += EffectValue;
		break;

		case EFFECTTYPE_INCREASE_COMMAND:
			lpObj->AddLeadership += EffectValue;
		break;

		case EFFECTTYPE_INCREASE_PHYSICAL_DAMAGE:
			lpObj->m_AttackDamageMaxLeft += EffectValue;
			lpObj->m_AttackDamageMinLeft += EffectValue;
			lpObj->m_AttackDamageMaxRight += EffectValue;
			lpObj->m_AttackDamageMinRight += EffectValue;
		break;

		case EFFECTTYPE_INCREASE_SPELL_DAMAGE:
			lpObj->m_MagicDamageMin += EffectValue;
			lpObj->m_MagicDamageMax += EffectValue;
		break;

		case EFFECTTYPE_MANA_SHIELD:
			lpObj->m_SkillInfo.ManaShieldAbsorbPercent = EffectValue;
		break;

		case EFFECTTYPE_IMPROVE_MAGIC_DEFENSE:
			lpObj->m_MagicDefense += EffectValue;
		break;

		case EFFECTTYPE_INCREASE_DAMAGE_REFLECT:
			lpObj->EXC_ReflectDamageTaken += EffectValue;
		break;

		case EFFECTTYPE_REDUCE_ATTACK_RATE:
			lpObj->HitRating -= EffectValue;
		break;

		case EFFECTTYPE_UNUSED:
			lpObj->m_SkillInfo.SoulBarrierManaRate = EffectValue;
		break;

		case EFFECTTYPE_INCREASE_AMPLIFY_DAMAGE:
			lpObj->AddMana += (int)( (((float)(EffectValue) * (float)(lpObj->MaxMana))) / 100.0f);		
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, (lpObj->MaxMana + lpObj->AddMana), 0xFE, 0, (lpObj->MaxStamina+lpObj->AddStamina));
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, lpObj->Mana, 0xFF, 0, lpObj->Stamina);
		break;

		case EFFECTTYPE_DECREASE_AMPLIFY_DAMAGE: {
			float fAddLife = 0;
			float fPer = (((float)(40.0f) - (float)(EffectValue)) / 100.0f);

			fPer = (fPer > 0.1f) ? fPer : 0.1f;
			fAddLife = fPer * lpObj->MaxLife;
			lpObj->AddLife -= (int)((float)(fAddLife));
			lpObj->Life = ((float)(lpObj->Life) < ((float)(lpObj->AddLife) + (float)(lpObj->MaxLife))) ? ((float)(lpObj->Life)) : (((float)(lpObj->AddLife) + (float)(lpObj->MaxLife)));
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->AddLife + lpObj->MaxLife, 0xFE, 0, lpObj->iAddShield + lpObj->iMaxShield);
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
		}
		break;

		case EFFECTTYPE_INCREASE_MINIMUM_SPELL_DAMAGE:
			lpObj->m_MagicDamageMin += EffectValue;
		break;

		case EFFECTTYPE_INCREASE_MAXIMUM_SPELL_DAMAGE:
			lpObj->m_MagicDamageMax += EffectValue;
		break;

		case EFFECTTYPE_POWER_UP:
			lpObj->m_AttackDamageMaxLeft += EffectValue;
			lpObj->m_AttackDamageMinLeft += EffectValue;
			lpObj->m_AttackDamageMaxRight += EffectValue;
			lpObj->m_AttackDamageMinRight += EffectValue;
			lpObj->m_MagicDamageMin += EffectValue;
			lpObj->m_MagicDamageMax += EffectValue;
			lpObj->m_CurseDamageMin += EffectValue;
			lpObj->m_CurseDamageMax += EffectValue;
		break;

		case EFFECTTYPE_GUARD_UP:
			lpObj->Defense += EffectValue;
		break;

		case EFFECTTYPE_AG_UP:
			lpObj->AddStamina += EffectValue * (lpObj->Level + lpObj->CharacterData->MasterLevel);
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, (WORD)(lpObj->MaxMana + lpObj->AddMana), 0xFE, 0, (WORD)(lpObj->MaxStamina+lpObj->AddStamina));
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, (WORD)lpObj->Mana, 0xFF, 0, (WORD)lpObj->Stamina);
		break;

		case EFFECTTYPE_SD_UP:
			lpObj->iAddShield += EffectValue * (lpObj->Level + lpObj->CharacterData->MasterLevel);
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->AddLife+lpObj->MaxLife, 0xFE, 0, lpObj->iAddShield+lpObj->iMaxShield);
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
		break;

		case EFFECTTYPE_SD_UP_VALUE:
			lpObj->iAddShield += EffectValue;
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->AddLife+lpObj->MaxLife, 0xFE, 0, lpObj->iAddShield+lpObj->iMaxShield);
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
		break;

		case EFFECTTYPE_AG_UP_VALUE:
			lpObj->AddStamina += EffectValue;
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, (WORD)(lpObj->MaxMana + lpObj->AddMana), 0xFE, 0, (WORD)(lpObj->MaxStamina+lpObj->AddStamina));
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, (WORD)lpObj->Mana, 0xFF, 0, (WORD)lpObj->Stamina);
		break;

		case EFFECTTYPE_BLIND:
			lpObj->m_bBlind = true;
		break;

		case EFFECTTYPE_WRATH_INC_DAMAGE:
			lpObj->m_AttackDamageMinLeft += lpObj->m_AttackDamageMinLeft * EffectValue / 100;
			lpObj->m_AttackDamageMaxLeft += lpObj->m_AttackDamageMaxLeft * EffectValue / 100;
			lpObj->m_AttackDamageMinRight += lpObj->m_AttackDamageMinRight * EffectValue / 100;
			lpObj->m_AttackDamageMaxRight += lpObj->m_AttackDamageMaxRight * EffectValue / 100;
		break;
	}
}

void CBuffEffect::BUFFEFFECT_ClearEffect(LPOBJECTSTRUCT lpObj, BYTE EffectType, int EffectValue) {
	if (lpObj == NULL || EffectType < EFFECTTYPE_NONE) return;
	if (lpObj->Connected < PLAYER_PLAYING) return;
	switch (EffectType) {
		case EFFECTTYPE_LIFE_REGENERATION:
			lpObj->LifeRegeneration -= EffectValue;
		break;

		case EFFECTTYPE_WATER_DEBUFF:
			lpObj->AddMana += EffectValue;
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, (lpObj->MaxMana + lpObj->AddMana), 0xFE, 0, (lpObj->MaxStamina + lpObj->AddStamina));
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, lpObj->Mana, 0xFF, 0, lpObj->Stamina);
		break;

		case EFFECTTYPE_LIGHTNING_DEBUFF:
			lpObj->Defense += EffectValue;
			lpObj->m_MagicDefense += EffectValue;
			lpObj->m_iPentagramDefense += EffectValue;
		break;

		case EFFECTTYPE_EARTH_DEBUFF:
			lpObj->AddLife += EffectValue;
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->AddLife + lpObj->MaxLife, 0xFE, 0, lpObj->iAddShield + lpObj->iMaxShield);
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
		break;

		case EFFECTTYPE_INCREASE_ATTACKSPEED:
			lpObj->AttackSpeed -= EffectValue;
			lpObj->MagicSpeed -= EffectValue;
		break;

		case EFFECTTYPE_INCREASE_DEFENSE:
			lpObj->Defense -= EffectValue;
			lpObj->m_MagicDefense -= EffectValue;
		break;

		case EFFECTTYPE_INCREASE_LIFE:
			lpObj->AddLife -= EffectValue;
			if (lpObj->AddLife <= 0.0) lpObj->AddLife = 0.0;
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->AddLife+lpObj->MaxLife, 0xFE, 0, lpObj->iAddShield+lpObj->iMaxShield);
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
		break;

		case EFFECTTYPE_INCREASE_MANA:
			lpObj->AddMana -= EffectValue;
			if (lpObj->AddMana <= 0.0) lpObj->AddMana = 0.0;
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, (WORD)(lpObj->MaxMana + lpObj->AddMana), 0xFE, 0, (WORD)(lpObj->MaxStamina+lpObj->AddStamina));
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, (WORD)lpObj->Mana, 0xFF, 0, (WORD)lpObj->Stamina);
		break;

		case EFFECTTYPE_INCREASE_STRENGTH:
			lpObj->AddStrength -= EffectValue;
		break;

		case EFFECTTYPE_INCREASE_DEXTERITY:
			EnterCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
			lpObj->CharacterData->AgilityCheckDelay = GetTickCount();
			lpObj->AddDexterity -= EffectValue;
			LeaveCriticalSection(&lpObj->CharacterData->AgiCheckCriti);
		break;

		case EFFECTTYPE_INCREASE_VITALITY:
			lpObj->AddVitality -= EffectValue;
		break;

		case EFFECTTYPE_INCREASE_ENERGY:
			lpObj->AddEnergy -= EffectValue;
		break;

		case EFFECTTYPE_INCREASE_COMMAND:
			lpObj->AddLeadership -= EffectValue;
		break;

		case EFFECTTYPE_INCREASE_PHYSICAL_DAMAGE:
			lpObj->m_AttackDamageMaxLeft -= EffectValue;
			lpObj->m_AttackDamageMinLeft -= EffectValue;
			lpObj->m_AttackDamageMaxRight -= EffectValue;
			lpObj->m_AttackDamageMinRight -= EffectValue;
		break;

		case EFFECTTYPE_INCREASE_SPELL_DAMAGE:
			lpObj->m_MagicDamageMin -= EffectValue;
			lpObj->m_MagicDamageMax -= EffectValue;
		break;

		case EFFECTTYPE_MANA_SHIELD:
			lpObj->m_SkillInfo.ManaShieldAbsorbPercent -= EffectValue;
		break;

		case EFFECTTYPE_IMPROVE_MAGIC_DEFENSE:
			lpObj->m_MagicDefense -= EffectValue;
		break;

		case EFFECTTYPE_INCREASE_DAMAGE_REFLECT:
			lpObj->EXC_ReflectDamageTaken -= EffectValue;
		break;

		case EFFECTTYPE_REDUCE_ATTACK_RATE:
			lpObj->HitRating += EffectValue;
		break;

		case EFFECTTYPE_UNUSED:
			lpObj->m_SkillInfo.SoulBarrierManaRate = 0;
		break;

		case EFFECTTYPE_INCREASE_AMPLIFY_DAMAGE:
			lpObj->AddMana -= (int)((((float)(EffectValue) * (float)(lpObj->MaxMana))) / 100.0f);

			if (lpObj->AddMana <= 0.0) lpObj->AddMana = 0.0;

			lpObj->Mana = ((float)(lpObj->Mana) < ((float)(lpObj->AddMana) + (float)(lpObj->MaxMana))) ? ((float)(lpObj->Mana)) : (((float)(lpObj->AddMana) + (float)(lpObj->MaxMana)));		
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, (WORD)(lpObj->MaxMana + lpObj->AddMana), 0xFE, 0, (WORD)(lpObj->MaxStamina+lpObj->AddStamina));
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, (WORD)lpObj->Mana, 0xFF, 0, (WORD)lpObj->Stamina);
		break;

		case EFFECTTYPE_DECREASE_AMPLIFY_DAMAGE: {
			float fAddLife = 0;
			float fPer = (((float)(40.0f) - (float)(EffectValue)) / 100.0f);

			fPer = (fPer > 0.1f) ? fPer : 0.1f;
			fAddLife = fPer * lpObj->MaxLife;
			lpObj->AddLife += (int)(fAddLife);
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->AddLife+lpObj->MaxLife, 0xFE, 0, lpObj->iAddShield+lpObj->iMaxShield);
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
		}
		break;

		case EFFECTTYPE_INCREASE_MINIMUM_SPELL_DAMAGE:
			lpObj->m_MagicDamageMin -= EffectValue;
		break;

		case EFFECTTYPE_INCREASE_MAXIMUM_SPELL_DAMAGE:
			lpObj->m_MagicDamageMax -= EffectValue;
			lpObj->CharacterData->Mastery.ML_DW_ArcaneFocus = 0.0;
			lpObj->CharacterData->Mastery.ML_ExcellentStrike = 0;
		break;

		case EFFECTTYPE_POWER_UP:
			lpObj->m_AttackDamageMaxLeft -= EffectValue;
			lpObj->m_AttackDamageMinLeft -= EffectValue;
			lpObj->m_AttackDamageMaxRight -= EffectValue;
			lpObj->m_AttackDamageMinRight -= EffectValue;
			lpObj->m_MagicDamageMin -= EffectValue;
			lpObj->m_MagicDamageMax -= EffectValue;
			lpObj->m_CurseDamageMin -= EffectValue;
			lpObj->m_CurseDamageMax -= EffectValue;
		break;

		case EFFECTTYPE_GUARD_UP:
			lpObj->Defense -= EffectValue;
		break;

		case EFFECTTYPE_AG_UP:
			lpObj->AddStamina -= EffectValue * (lpObj->Level + lpObj->CharacterData->MasterLevel);
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, (WORD)(lpObj->MaxMana + lpObj->AddMana), 0xFE, 0, (WORD)(lpObj->MaxStamina+lpObj->AddStamina));
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, (WORD)lpObj->Mana, 0xFF, 0, (WORD)lpObj->Stamina);
		break;

		case EFFECTTYPE_SD_UP:
			lpObj->iAddShield -= EffectValue * (lpObj->Level + lpObj->CharacterData->MasterLevel);
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->AddLife+lpObj->MaxLife, 0xFE, 0, lpObj->iAddShield+lpObj->iMaxShield);
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
		break;

		case EFFECTTYPE_SD_UP_VALUE:
			lpObj->iAddShield -= EffectValue;
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->AddLife+lpObj->MaxLife, 0xFE, 0, lpObj->iAddShield+lpObj->iMaxShield);
			GSProtocol.PROTOCOL_ReFillSend(lpObj->m_Index, lpObj->Life, 0xFF, 0, lpObj->iShield);
		break;

		case EFFECTTYPE_AG_UP_VALUE:
			lpObj->AddStamina -= EffectValue;
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, (WORD)(lpObj->MaxMana + lpObj->AddMana), 0xFE, 0, (WORD)(lpObj->MaxStamina+lpObj->AddStamina));
			GSProtocol.PROTOCOL_ManaSend(lpObj->m_Index, (WORD)lpObj->Mana, 0xFF, 0, (WORD)lpObj->Stamina);
		break;

		case EFFECTTYPE_INCREASE_ALLSTATS: {
			PMSG_USE_STAT_FRUIT pMsg;

			PHeadSetB((LPBYTE)&pMsg, 0x2C, sizeof(pMsg));
			pMsg.result = 0x12;
			pMsg.btStatValue = EffectValue;
			pMsg.btFruitType = 0x07;
			IOCP.DataSend(lpObj->m_Index, (LPBYTE)&pMsg, pMsg.h.size);
		}
		break;

		case EFFECTTYPE_BLIND:
			lpObj->m_bBlind = false;
		break;

		case EFFECTTYPE_WRATH_INC_DAMAGE:
			lpObj->m_AttackDamageMinLeft -= lpObj->m_AttackDamageMinLeft * EffectValue / 100;
			lpObj->m_AttackDamageMaxLeft -= lpObj->m_AttackDamageMaxLeft * EffectValue / 100;
			lpObj->m_AttackDamageMinRight -= lpObj->m_AttackDamageMinRight * EffectValue / 100;
			lpObj->m_AttackDamageMaxRight -= lpObj->m_AttackDamageMaxRight * EffectValue / 100;
		break;
	}
}

void CBuffEffect::BUFFEFFECT_SetDOT(class OBJECTSTRUCT* lpObj, BYTE EffectType, int EffectValue) {
	if (EffectType <= EFFECTTYPE_NONE) return;
	switch (EffectType) {
		case EFFECTTYPE_DOT_GENERAL:				BUFFEFFECT_DamageOverTime(lpObj, EffectValue);					break;
		case EFFECTTYPE_DOT_POISON:					BUFFEFFECT_Poison(lpObj, EffectValue);							break;
		case EFFECTTYPE_DOT_VAMPIRIC:				BUFFEFFECT_Vampiric(lpObj, EffectValue);						break;
		case EFFECTTYPE_DOT_IMPROVED_POISON:		BUFFEFFECT_ImprovedPoison(lpObj, EffectValue);					break;
		case EFFECTTYPE_DOT_ROT:					BUFFEFFECT_Rot(lpObj, EffectValue);								break;
	}
}

void CBuffEffect::BUFFEFFECT_DamageOverTime(class OBJECTSTRUCT* lpObj, int Damage) {
	if (lpObj->Live == 0 || Damage <= 0) return;

	int DecreaseHealthPoint = 0;
	int DecreaseShieldPoint = 0;

	DecreaseHealthPoint = Damage;
	DecreaseShieldPoint = DecreaseHealthPoint;

	if (lpObj->lpAttackObj != NULL && lpObj->Type == OBJ_USER && lpObj->m_bOffLevel == FALSE && lpObj->lpAttackObj->Type == OBJ_USER) {
		if (lpObj->iShield - DecreaseShieldPoint >= 0) {
			lpObj->iShield -= DecreaseShieldPoint;
			DecreaseHealthPoint = 0;
		} else {
			DecreaseHealthPoint = DecreaseShieldPoint - lpObj->iShield;
			DecreaseShieldPoint = lpObj->iShield;
			lpObj->iShield = 0;
			lpObj->Life -= DecreaseHealthPoint;
		}
	} else {
		DecreaseShieldPoint = 0;
		lpObj->Life -= DecreaseHealthPoint;
	}

	if (lpObj->Life < 0.0f) lpObj->Life = 0.0f;
	if (lpObj->lpAttackObj != NULL) gObjLifeCheck(lpObj, lpObj->lpAttackObj, DecreaseHealthPoint, 3, 0, 0, 0, DecreaseShieldPoint, 0);
}

void CBuffEffect::BUFFEFFECT_Poison(LPOBJECTSTRUCT lpObj, BYTE PoisonRate) {
	if (lpObj->Live == 0) return;

	int DecreaseHealthPoint = 0;
	int DecreaseShiledPoint = 0;

	if (lpObj->Type == OBJ_MONSTER) DecreaseHealthPoint = (((int)PoisonRate * lpObj->Life) / 100);
	else DecreaseHealthPoint = (((int)(PoisonRate) * (lpObj->AddLife + lpObj->MaxLife)) / 100);

	DecreaseShiledPoint = DecreaseHealthPoint;

	if (lpObj->lpAttackObj != NULL && lpObj->Type == OBJ_USER && lpObj->m_bOffLevel == FALSE && lpObj->lpAttackObj->Type == OBJ_USER) {
		if (lpObj->iShield - DecreaseShiledPoint >= 0) {
			lpObj->iShield -= DecreaseShiledPoint;
			DecreaseHealthPoint = 0;
		} else {
			DecreaseHealthPoint = DecreaseShiledPoint - lpObj->iShield;
			DecreaseShiledPoint = lpObj->iShield;
			lpObj->Life -= DecreaseHealthPoint;
			lpObj->iShield = 0;
		}
	} else {
		DecreaseShiledPoint = 0;
		lpObj->Life -= DecreaseHealthPoint;
	}
	
	if (lpObj->Life < 0.0f) lpObj->Life = 0.0f;
	if (lpObj->lpAttackObj != NULL) gObjLifeCheck(lpObj, lpObj->lpAttackObj, DecreaseHealthPoint, 2, 0, 0, 1, DecreaseShiledPoint, 0);
}
void CBuffEffect::BUFFEFFECT_ImprovedPoison(LPOBJECTSTRUCT lpObj, BYTE PoisonRate) {
	if (lpObj->Live == 0) return;

	int DecreaseHealthPoint = 0;

	if (lpObj->Type == OBJ_MONSTER) DecreaseHealthPoint = (((int)PoisonRate * lpObj->Life) / 100);
	else DecreaseHealthPoint = (((int)(PoisonRate) * (lpObj->AddLife + lpObj->MaxLife)) / 100);

	int DecreaseShiledPoint = DecreaseHealthPoint;

	if (lpObj->lpAttackObj != NULL && lpObj->Type == OBJ_USER && lpObj->m_bOffLevel == FALSE && lpObj->lpAttackObj->Type == OBJ_USER) {
		int DecreaseManaPoint = DecreaseHealthPoint;

		if (lpObj->iShield - DecreaseShiledPoint >= 0) {
			lpObj->iShield -= DecreaseShiledPoint;
			DecreaseHealthPoint = 0;
		} else {
			DecreaseHealthPoint = DecreaseShiledPoint - lpObj->iShield;
			DecreaseShiledPoint = lpObj->iShield;
			lpObj->iShield = 0;
			lpObj->Life -= DecreaseHealthPoint;
		}

		if (lpObj->Mana - DecreaseManaPoint >= 0) {
			lpObj->Mana -= DecreaseManaPoint;
			lpObj->lpAttackObj->Mana += DecreaseManaPoint;
		} else {
			DecreaseManaPoint = lpObj->Mana;
			lpObj->Mana = 0;
			lpObj->lpAttackObj->Mana += DecreaseManaPoint;
		}

		if (lpObj->lpAttackObj->Mana > (lpObj->lpAttackObj->MaxMana + lpObj->lpAttackObj->AddMana)) lpObj->lpAttackObj->Mana = lpObj->lpAttackObj->MaxMana + lpObj->lpAttackObj->AddMana;
	} else {
		DecreaseShiledPoint = 0;
		lpObj->Life -= DecreaseHealthPoint;
	}

	if (lpObj->Life < 0.0f) lpObj->Life = 0.0f;
	if (lpObj->lpAttackObj != NULL) {
		gObjLifeCheck(lpObj, lpObj->lpAttackObj, DecreaseHealthPoint, 2, 0, 0, 1, DecreaseShiledPoint, 0);
		GSProtocol.PROTOCOL_ManaSend(lpObj->lpAttackObj->m_Index, lpObj->lpAttackObj->Mana, 255, 0, lpObj->lpAttackObj->Stamina);
	}
}
void CBuffEffect::BUFFEFFECT_Rot(LPOBJECTSTRUCT lpObj, int PoisonRate) {
	if (lpObj->Live == 0) return;

	int DecreaseHealthPoint = 0;
	int DecreaseShiledPoint = 0;

	if (lpObj->Type == OBJ_MONSTER) DecreaseHealthPoint = (((int)PoisonRate * lpObj->Life) / 100);
	else DecreaseHealthPoint = (((int)(PoisonRate) * (lpObj->AddLife + lpObj->MaxLife)) / 100);

	DecreaseShiledPoint = DecreaseHealthPoint;

	if (lpObj->lpAttackObj != NULL && lpObj->Type == OBJ_USER && lpObj->m_bOffLevel == FALSE && lpObj->lpAttackObj->Type == OBJ_USER) {
		if (lpObj->iShield - DecreaseShiledPoint > 0) {
			lpObj->iShield -= DecreaseShiledPoint;
			DecreaseHealthPoint = 0;
		} else {
			DecreaseHealthPoint = DecreaseShiledPoint - lpObj->iShield;
			DecreaseShiledPoint = lpObj->iShield;
			lpObj->Life -= DecreaseHealthPoint;
			lpObj->iShield = 0;
		}
	} else {
		DecreaseShiledPoint = 0;
		lpObj->Life -= DecreaseHealthPoint;
	}

	if (lpObj->Life < 0.0f) lpObj->Life = 0.0f;
	if (lpObj->lpAttackObj != NULL) gObjLifeCheck(lpObj, lpObj->lpAttackObj, DecreaseHealthPoint, 2, 0, 0, 1, DecreaseShiledPoint, 0);
	if (lpObj->Type == OBJ_USER) {
		float fDurDownRate = 10;

		gObjArmorRandomDurDown(lpObj, lpObj->lpAttackObj);

		if (rand() % 100 < fDurDownRate) {
			if (g_ConfigRead.server.GetServerType() != SERVER_CASTLE || (lpObj->lpAttackObj->m_btCsJoinSide == 0 || lpObj->lpAttackObj->m_btCsJoinSide != lpObj->m_btCsJoinSide)) {
				int item_num[5];

				item_num[0] = 2;
				item_num[1] = 3;
				item_num[2] = 4;
				item_num[3] = 5;
				item_num[4] = 6;

				int iEquipmentPos = rand() % 5;
				CItem* lpEquipment = &lpObj->pInventory[item_num[iEquipmentPos]];

				if (lpEquipment && lpEquipment->IsItem()) {
					float iDurDecValue = PoisonRate / 10;
					int damagemin = lpEquipment->m_BaseDurability / 100.0 * (10 - iDurDecValue);
					int iDurEquipment = lpEquipment->m_Durability - damagemin;

					for (int iBuffIndex = 0; iBuffIndex < lpObj->m_BuffEffectCount; iBuffIndex++) {
						if (lpObj->m_BuffEffectList[iBuffIndex].BuffIndex == BUFFTYPE_CHARM_PROTECTITEM) {
							iDurEquipment = lpEquipment->m_Durability;
							break;
						}
					}

					lpEquipment->m_Durability = (float)iDurEquipment;

					if (lpEquipment->m_Durability < 0.0f) lpEquipment->m_Durability = 0.0f;

					GSProtocol.GCItemDurSend(lpObj->m_Index, item_num[iEquipmentPos], (BYTE)lpEquipment->m_Durability, 0);
				}
			}
		}
	}
}
void CBuffEffect::BUFFEFFECT_Vampiric(LPOBJECTSTRUCT lpObj, int Damage) {
	if (lpObj->Live == 0) return;

	lpObj->Life -= Damage;

	if (lpObj->Life < 0.0f) lpObj->Life = 0.0f;
	if (lpObj->lpAttackObj != NULL) gObjLifeCheck(lpObj, lpObj->lpAttackObj, Damage, 3, 0, 0, 0, 0, 0);
	if (BUFF_TargetHasBuff(lpObj, DEBUFFTYPE_SIPHON_LIFE) == true) {
		int value1 = 0, value2 = 0;

		BUFF_GetBuffEffectValue(lpObj, DEBUFFTYPE_SIPHON_LIFE, &value1, &value2);

		if (ObjectMaxRange(value2) == false) return;

		LPOBJ Caster = &gObj[value2];

		if ((Caster->AddLife + Caster->MaxLife) >= (Caster->Life + Damage)) Caster->Life += Damage;
		else Caster->Life = Caster->AddLife + Caster->MaxLife;

		GSProtocol.PROTOCOL_ReFillSend(Caster->m_Index, Caster->Life, 0xFF, 0, Caster->iShield);
	}
}

void CBuffEffect::SetPrevEffect(LPOBJECTSTRUCT lpObj) {
	if (lpObj == NULL) return;

	int BuffCount = 0;

	for(int i = 0; i < MAX_BUFFEFFECT; i++)	{
		if (lpObj->m_BuffEffectList[i].BuffIndex == BUFFTYPE_NONE) continue;

		switch(lpObj->m_BuffEffectList[i].EffectType1) {
			case EFFECTTYPE_EARTH_DEBUFF:
			case EFFECTTYPE_INCREASE_LIFE:
			case EFFECTTYPE_INCREASE_MANA:
			case EFFECTTYPE_INCREASE_STRENGTH:
			case EFFECTTYPE_INCREASE_DEXTERITY:
			case EFFECTTYPE_INCREASE_VITALITY:
			case EFFECTTYPE_INCREASE_ENERGY:
			case EFFECTTYPE_INCREASE_COMMAND:
			case EFFECTTYPE_INCREASE_DAMAGE_REFLECT:
			case EFFECTTYPE_SD_UP:
			case EFFECTTYPE_AG_UP:
			case EFFECTTYPE_SD_UP_VALUE:
			case EFFECTTYPE_AG_UP_VALUE:
				BuffCount++;
				BUFFEFFECT_SetEffect(lpObj, lpObj->m_BuffEffectList[i].EffectType1, lpObj->m_BuffEffectList[i].EffectValue1);
			break;

			default:
			break;
		}

		switch(lpObj->m_BuffEffectList[i].EffectType2) {
			case EFFECTTYPE_EARTH_DEBUFF:
			case EFFECTTYPE_INCREASE_LIFE:
			case EFFECTTYPE_INCREASE_MANA:
			case EFFECTTYPE_INCREASE_STRENGTH:
			case EFFECTTYPE_INCREASE_DEXTERITY:
			case EFFECTTYPE_INCREASE_VITALITY:
			case EFFECTTYPE_INCREASE_ENERGY:
			case EFFECTTYPE_INCREASE_COMMAND:
			case EFFECTTYPE_INCREASE_DAMAGE_REFLECT:
			case EFFECTTYPE_SD_UP:
			case EFFECTTYPE_AG_UP:
			case EFFECTTYPE_SD_UP_VALUE:
			case EFFECTTYPE_AG_UP_VALUE:
				BuffCount++;
				BUFFEFFECT_SetEffect(lpObj, lpObj->m_BuffEffectList[i].EffectType2, lpObj->m_BuffEffectList[i].EffectValue2);
			break;

			default:
			break;
		}
	}
}

void CBuffEffect::SetNextEffect(LPOBJECTSTRUCT lpObj) {
	if (lpObj == NULL) return;

	int BuffCount = 0;

	for(int i = 0; i < MAX_BUFFEFFECT; i++) {
		if (lpObj->m_BuffEffectList[i].BuffIndex == BUFFTYPE_NONE) continue;

		switch(lpObj->m_BuffEffectList[i].EffectType1) {
			case EFFECTTYPE_EARTH_DEBUFF:
			case EFFECTTYPE_INCREASE_LIFE:
			case EFFECTTYPE_INCREASE_MANA:
			case EFFECTTYPE_INCREASE_STRENGTH:
			case EFFECTTYPE_INCREASE_DEXTERITY:
			case EFFECTTYPE_INCREASE_VITALITY:
			case EFFECTTYPE_INCREASE_ENERGY:
			case EFFECTTYPE_INCREASE_COMMAND:
			case EFFECTTYPE_INCREASE_DAMAGE_REFLECT:
			case EFFECTTYPE_SD_UP:
			case EFFECTTYPE_AG_UP:
			case EFFECTTYPE_SD_UP_VALUE:
			case EFFECTTYPE_AG_UP_VALUE:
			break;

			default:
				BuffCount++;
				BUFFEFFECT_SetEffect(lpObj, lpObj->m_BuffEffectList[i].EffectType1, lpObj->m_BuffEffectList[i].EffectValue1);
			break;
		}

		switch(lpObj->m_BuffEffectList[i].EffectType2) {
			case EFFECTTYPE_EARTH_DEBUFF:
			case EFFECTTYPE_INCREASE_LIFE:
			case EFFECTTYPE_INCREASE_MANA:
			case EFFECTTYPE_INCREASE_STRENGTH:
			case EFFECTTYPE_INCREASE_DEXTERITY:
			case EFFECTTYPE_INCREASE_VITALITY:
			case EFFECTTYPE_INCREASE_ENERGY:
			case EFFECTTYPE_INCREASE_COMMAND:
			case EFFECTTYPE_INCREASE_DAMAGE_REFLECT:
			case EFFECTTYPE_SD_UP:
			case EFFECTTYPE_AG_UP:
			case EFFECTTYPE_SD_UP_VALUE:
			case EFFECTTYPE_AG_UP_VALUE:
			break;

			default:
				BuffCount++;
				BUFFEFFECT_SetEffect(lpObj, lpObj->m_BuffEffectList[i].EffectType2, lpObj->m_BuffEffectList[i].EffectValue2);
			break;
		}
	}
}

void CBuffEffect::ClearPrevEffect(LPOBJECTSTRUCT lpObj) {
	if (lpObj == NULL) return;

	int BuffCount = 0;

	for(int i = 0; i < MAX_BUFFEFFECT; i++)	{
		if (lpObj->m_BuffEffectList[i].BuffIndex == BUFFTYPE_NONE) continue;

		switch(lpObj->m_BuffEffectList[i].EffectType1) {
			case EFFECTTYPE_EARTH_DEBUFF:
			case EFFECTTYPE_INCREASE_LIFE:
			case EFFECTTYPE_INCREASE_MANA:
			case EFFECTTYPE_INCREASE_STRENGTH:
			case EFFECTTYPE_INCREASE_DEXTERITY:
			case EFFECTTYPE_INCREASE_VITALITY:
			case EFFECTTYPE_INCREASE_ENERGY:
			case EFFECTTYPE_INCREASE_COMMAND:
			case EFFECTTYPE_INCREASE_DAMAGE_REFLECT:
			case EFFECTTYPE_SD_UP:
			case EFFECTTYPE_AG_UP:
			case EFFECTTYPE_SD_UP_VALUE:
			case EFFECTTYPE_AG_UP_VALUE:
				BuffCount++;
				BUFFEFFECT_ClearEffect(lpObj, lpObj->m_BuffEffectList[i].EffectType1, lpObj->m_BuffEffectList[i].EffectValue1);
			break;

			default:
			break;
		}

		switch(lpObj->m_BuffEffectList[i].EffectType2) {
			case EFFECTTYPE_EARTH_DEBUFF:
			case EFFECTTYPE_INCREASE_LIFE:
			case EFFECTTYPE_INCREASE_MANA:
			case EFFECTTYPE_INCREASE_STRENGTH:
			case EFFECTTYPE_INCREASE_DEXTERITY:
			case EFFECTTYPE_INCREASE_VITALITY:
			case EFFECTTYPE_INCREASE_ENERGY:
			case EFFECTTYPE_INCREASE_COMMAND:
			case EFFECTTYPE_INCREASE_DAMAGE_REFLECT:
			case EFFECTTYPE_SD_UP:
			case EFFECTTYPE_AG_UP:
			case EFFECTTYPE_SD_UP_VALUE:
			case EFFECTTYPE_AG_UP_VALUE:
				BuffCount++;
				BUFFEFFECT_ClearEffect(lpObj, lpObj->m_BuffEffectList[i].EffectType2, lpObj->m_BuffEffectList[i].EffectValue2);
			break;

			default:
			break;
		}
	}
}

struct PMSG_REQ_GUILD_PERIODBUFF_INSERT {
	PBMSG_HEAD2 head;
	char szGuildName[MAX_GUILD_LEN+1];
	WORD wBuffIndex;
	char btEffectType1;
	char btEffectType2;
	DWORD dwDuration;
	time_t lExpireDate;
};

void CBuffEffect::RequestGuildPeriodBuffInsert(char *szGuildName, _tagPeriodBuffInfo *lpBuffInfo) {
	PMSG_REQ_GUILD_PERIODBUFF_INSERT pMsg;

	memcpy(pMsg.szGuildName, szGuildName, MAX_GUILD_LEN+1);
	pMsg.wBuffIndex = lpBuffInfo->wBuffIndex;
	pMsg.btEffectType1 = lpBuffInfo->btEffectType1;
	pMsg.btEffectType2 = lpBuffInfo->btEffectType2;
	pMsg.dwDuration = lpBuffInfo->lDuration;
	pMsg.lExpireDate = g_PeriodItemEx.GetExpireDate(lpBuffInfo->lDuration);
	PHeadSubSetB((LPBYTE)&pMsg, 0x53, 1, sizeof(pMsg)); 
	wsExDbCli.DataSend((char*)&pMsg, pMsg.head.size);  
	g_Log.Add("[PeriodBuff][Insert] Request Insert Guild PeriodBuff. GuildName : %s, BuffIndex : %d, Duration : %d, lExpireDate : %d", szGuildName, lpBuffInfo->wBuffIndex, lpBuffInfo->lDuration, pMsg.lExpireDate);
}

struct PMSG_REQ_GUILD_PERIODBUFF_DELETE {
	PBMSG_HEAD2 head;
	char btGuildCnt;
	WORD wBuffIndex[5];
};

void CBuffEffect::RequestGuildPeriodBuffDelete(WORD *wBuffIndex, char btGuildCnt) {
	PMSG_REQ_GUILD_PERIODBUFF_DELETE pMsg;

	for (int i = 0; i < btGuildCnt; ++i) pMsg.wBuffIndex[i] = wBuffIndex[i];

	pMsg.btGuildCnt = btGuildCnt;
	PHeadSubSetB((LPBYTE)&pMsg, 0x53, 2, sizeof(pMsg));
	wsExDbCli.DataSend((char*)&pMsg, pMsg.head.size);
	g_Log.Add("[PeriodBuff][Delete] Request All Delete Guild PeriodBuff");
}

struct PMSG_REQ_PERIODBUFF_DELETE {
	PBMSG_HEAD2 head;
	WORD wUserIndex;
	WORD wBuffIndex;
	char szCharacterName[11];
};

void CBuffEffect::RequestPeriodBuffDelete(OBJECTSTRUCT *lpObj, WORD wBuffIndex) {
	PMSG_REQ_PERIODBUFF_DELETE pMsg;

	pMsg.wUserIndex = lpObj->m_Index;
	pMsg.wBuffIndex = wBuffIndex;
	memcpy(pMsg.szCharacterName, lpObj->Name, MAX_ACCOUNT_LEN+1);
	PHeadSubSetB((LPBYTE)&pMsg, 0xE4, 2, sizeof(pMsg));  
	wsDataCli.DataSend((char*)&pMsg, pMsg.head.size);
	g_Log.Add("[PeriodBuff][Delete] Request Delete PeriodBuff. User Id : %s(%d), Name : %s, BuffIndex : %d", lpObj->AccountID, lpObj->DBNumber, lpObj->Name, wBuffIndex);
}

void CBuffEffect::RequestPeriodBuffDelete(char *szName, WORD wBuffIndex) {
	PMSG_REQ_PERIODBUFF_DELETE pMsg;

	pMsg.wUserIndex = 0;
	pMsg.wBuffIndex = wBuffIndex;
	memcpy(pMsg.szCharacterName, szName, MAX_ACCOUNT_LEN + 1);
	PHeadSubSetB((LPBYTE)&pMsg, 0xE4, 2, sizeof(pMsg));
	wsDataCli.DataSend((char*)&pMsg, pMsg.head.size);
	g_Log.Add("[PeriodBuff][Delete] Request Delete PeriodBuff.Name : %s, BuffIndex : %d", szName, wBuffIndex);
}

struct PMSG_REQ_PERIODBUFF_INSERT {
	PBMSG_HEAD2 head;
	WORD wUserIndex;
	char szCharacterName[MAX_ACCOUNT_LEN+1];
	WORD wBuffIndex;
	char btEffectType1;
	char btEffectType2;
	DWORD dwDuration;
	time_t lExpireDate;
};

void CBuffEffect::RequestPeriodBuffInsert(OBJECTSTRUCT *lpObj, _tagPeriodBuffInfo *lpBuffInfo) {
	PMSG_REQ_PERIODBUFF_INSERT pMsg; 

	pMsg.wUserIndex = lpObj->m_Index;
	memcpy(pMsg.szCharacterName, lpObj->Name, MAX_ACCOUNT_LEN+1);
	pMsg.wBuffIndex = lpBuffInfo->wBuffIndex;
	pMsg.btEffectType1 = lpBuffInfo->btEffectType1;
	pMsg.btEffectType2 = lpBuffInfo->btEffectType2;
	pMsg.dwDuration = lpBuffInfo->lDuration;
	pMsg.lExpireDate = g_PeriodItemEx.GetExpireDate(lpBuffInfo->lDuration);
	PHeadSubSetB((LPBYTE)&pMsg, 0xE4, 1, sizeof(pMsg));
	wsDataCli.DataSend((char*)&pMsg, pMsg.head.size);
	g_Log.Add("[PeriodBuff][Insert] Request Insert PeriodBuff. User Id : %s(%d), Name : %s, BuffIndex : %d, Duration %d, lExpireDate%d", lpObj->AccountID, lpObj->DBNumber, lpObj->Name, lpBuffInfo->wBuffIndex, lpBuffInfo->lDuration, pMsg.lExpireDate);
}

struct PMSG_REQ_PERIODBUFF_SELECT {
	PBMSG_HEAD2 head;
	WORD wUserIndex;
	char szCharacterName[MAX_ACCOUNT_LEN+1];
};

void CBuffEffect::RequestPeriodBuffSelect(OBJECTSTRUCT *lpObj) {
	PMSG_REQ_PERIODBUFF_SELECT pMsg;

	pMsg.wUserIndex = lpObj->m_Index;
	memcpy(pMsg.szCharacterName, lpObj->Name, MAX_ACCOUNT_LEN+1);
	PHeadSubSetB((LPBYTE)&pMsg, 0xE4, 3, sizeof(pMsg));
	wsDataCli.DataSend((char*)&pMsg, pMsg.head.size);
}

void CBuffEffect::DGPeriodItemExSelect(PMSG_ANS_PERIODBUFF_SELECT *lpMsg) {
	if (!ObjectMaxRange(lpMsg->wUserIndex))	return;

	LPOBJ lpObj = &gObj[lpMsg->wUserIndex];

	if (lpObj->Connected < PLAYER_LOGGED) return;
	if (lpObj->Type != OBJ_USER) return;
	if (g_ArcaBattle.IsArcaBattleServer() == FALSE && lpMsg->wBuffIndex == 184) return;

	LPPERIOD_BUFF_EFFECT_INFO lpPeriBuff = g_BuffEffectSlot.GetPeriodBuffInfo(lpMsg->wBuffIndex);

	if (!lpPeriBuff) return;

	time_t lLeftDate = g_PeriodItemEx.GetLeftDate(lpMsg->lExpireDate);

	if (gObjAddPeriodBuffEffect(lpObj, lpPeriBuff, lLeftDate) == FALSE) g_Log.Add("[PeriodBuff][Error][Select] Answer Select PeriodBuff. User Id : %s(%d), Name : %s, BuffIndex : %d Type1 : %d Type2 : %d ExpireDate : %d ResultCode : %d", lpObj->AccountID, lpObj->DBNumber, lpObj->Name, lpMsg->wBuffIndex, lpMsg->btEffectType1, lpMsg->btEffectType2, lpMsg->lExpireDate, lpMsg->btResultCode);
	else g_Log.Add("[PeriodBuff][Select] Answer Select PeriodBuff. User Id : %s(%d), Name : %s, BuffIndex : %d Type1 : %d Type2 : %d ExpireDate : %d ResultCode : %d", lpObj->AccountID, lpObj->DBNumber, lpObj->Name, lpMsg->wBuffIndex, lpMsg->btEffectType1, lpMsg->btEffectType2, lpMsg->lExpireDate, lpMsg->btResultCode);
}