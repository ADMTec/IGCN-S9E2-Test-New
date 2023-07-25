#ifndef TDURMAGICKEYCHECKER_H
#define TDURMAGICKEYCHECKER_H
#if _MSC_VER > 1000
#pragma once
#endif
#define MAX_DUR_MAGIC_KEY 60

class TDurMagicKeyChecker {
public:
	BOOL IsValidDurationTime(BYTE btMagicKey) {
		if (btMagicKey <= MAX_DUR_MAGIC_KEY && btMagicKey > 0) {
			if ((GetTickCount() - this->m_dwDurationTime[btMagicKey]) < 15000) return TRUE;
		}

		return FALSE;
	};

	BOOL IsValidCount(BYTE btMagicKey) {
		if (btMagicKey <= MAX_DUR_MAGIC_KEY && btMagicKey > 0) {
			if (this->m_btValidCount[btMagicKey] >= 0) return TRUE;
		}

		return FALSE;
	};

	void SetDurationTime(BYTE btMagicKey, DWORD dwCurrentTickCount) {
		if (btMagicKey <= MAX_DUR_MAGIC_KEY && btMagicKey > 0) {
			this->m_dwDurationTime[btMagicKey] = dwCurrentTickCount;
			this->m_btValidCount[btMagicKey] = 7;
		}
	};

	int GetValidDurationTime(BYTE btMagicKey) {
		if (btMagicKey <= MAX_DUR_MAGIC_KEY && btMagicKey > 0) return this->m_dwDurationTime[btMagicKey];

		return -77;
	};

	int GetValidCount(BYTE btMagicKey) {
		if (btMagicKey <= MAX_DUR_MAGIC_KEY && btMagicKey > 0) return this->m_btValidCount[btMagicKey];

		return -77;
	};

private:
	DWORD m_dwDurationTime[MAX_DUR_MAGIC_KEY];	// 0
	BYTE m_btValidCount[MAX_DUR_MAGIC_KEY];	// F0
};
#endif