//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "Stdafx.h"
#include "resource.h"
#include "SoundBankSelect.h"

int SelectSoundBankPreset(SFPlayerInstr *instr)
{
	SoundBankSelect dlg;
	float value;
	instr->GetParam(16, &value);
	dlg.bnkNum = (bsInt16) value;
	instr->GetParam(17, &value);
	dlg.insNum = (bsInt16) value;

	dlg.fileID = instr->GetSoundFile();
	dlg.insName = instr->GetInstrName();

	if (dlg.DoModal() == IDOK)
	{
		instr->SetSoundFile(dlg.fileID);
		instr->SetInstrName(dlg.insName);
		instr->SetParam(16, (float) dlg.bnkNum);
		instr->SetParam(17, (float) dlg.insNum);
		return 1;
	}
	return 0;
}

int SelectSoundBankPreset(GMManager *gm)
{
	SoundBankSelect dlg;

	float val;
	dlg.fileID = gm->GetSoundFile();
	gm->GetParam(18, &val);
	dlg.bnkNum = (bsInt16) val;
	gm->GetParam(19, &val);
	dlg.insNum = (bsInt16) val;

	if (dlg.DoModal() == IDOK)
	{
		gm->SetSoundFile(dlg.fileID);
		gm->SetParam(18, (float) dlg.bnkNum);
		gm->SetParam(19, (float) dlg.insNum);
		return 1;
	}
	return 0;
}

int SelectSoundBankPreset(bsString& file, MIDIControl *instr, int chnl)
{
	SoundBankSelect dlg;

	dlg.fileID = file;
	dlg.bnkNum = instr->GetBank(chnl);
	dlg.insNum = instr->GetPatch(chnl);

	if (dlg.DoModal() == IDOK)
	{
		file = dlg.fileID;
		instr->SetBank(chnl, dlg.bnkNum);
		instr->SetPatch(chnl, dlg.insNum);
		return 1;
	}
	return 0;
}

/////////////////////////////////////////////////////////

LRESULT SoundBankSelect::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();

	fileList = GetDlgItem(IDC_SB_FILE);
	bankList = GetDlgItem(IDC_SB_BANK);
	instrList = GetDlgItem(IDC_SB_PRESET);

	int bnkSel = 0;
	SoundBank *sb = SoundBank::SoundBankList.next;
	while (sb)
	{
		int index = fileList.AddString(sb->name);
		fileList.SetItemDataPtr(index, (void*)sb);
		if (sb->name.Compare(fileID) == 0)
			bnkSel = index;
		sb = sb->next;
	}

	fileList.SetCurSel(bnkSel);
	SwitchFile();

	return 1;
}

LRESULT SoundBankSelect::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SoundBank *bnk = GetFile();
	if (bnk)
	{
		fileID = bnk->name;
		bnkNum = GetBankNum();
		SBInstr *pre = GetInstr();
		if (pre)
		{
			insNum = pre->instrNdx;
			insName = pre->instrName;
		}
	}

	EndDialog(IDOK);
	return 0;
}

LRESULT SoundBankSelect::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT SoundBankSelect::OnFileChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bnkNum = 0;
	insNum = 0;
	SwitchFile();
	return 0;
}

LRESULT SoundBankSelect::OnBankChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	insNum = 0;
	SwitchBank();
	return 0;
}


void SoundBankSelect::SwitchFile()
{
	bankList.ResetContent();
	int sel = fileList.GetCurSel();
	if (sel != CB_ERR)
	{
		int bnkSel = 0;
		SoundBank *sb = (SoundBank *) fileList.GetItemDataPtr(sel);
		for (int n = 0; n < 129; n++)
		{
			if (sb->instrBank[n])
			{
				char num[40];
				sprintf_s(num, 40, "%d", n);
				int index = bankList.AddString(num);
				bankList.SetItemData(index, n);
				if (n == bnkNum)
					bnkSel = index;
			}
		}
		bankList.SetCurSel(bnkSel);
		SwitchBank();
	}
	else
		instrList.ResetContent();
}

void SoundBankSelect::SwitchBank()
{
	instrList.ResetContent();

	int insSel = 0;
	SoundBank *sb = GetFile();
	if (sb)
	{
		int bank = GetBankNum();
		if (bank >= 0)
		{
			SBInstr *ins;
			for (int n = 0; n < 128; n++)
			{
				if ((ins = sb->GetInstr(bank, n)) != 0)
				{
					int index = instrList.AddString(ins->instrName);
					instrList.SetItemDataPtr(index, ins);
					if (n == insNum)
						insSel = n;
				}
			}
			instrList.SetCurSel(insSel);
		}
	}
}

SoundBank *SoundBankSelect::GetFile()
{
	int sel = fileList.GetCurSel();
	if (sel != CB_ERR)
		return (SoundBank *) fileList.GetItemDataPtr(sel);
	return 0;
}

int SoundBankSelect::GetBankNum()
{
	int sel = bankList.GetCurSel();
	if (sel != LB_ERR)
		return (int) bankList.GetItemData(sel);
	return -1;
}

SBInstr *SoundBankSelect::GetInstr()
{
	int sel = instrList.GetCurSel();
	if (sel != LB_ERR)
		return (SBInstr *) instrList.GetItemDataPtr(sel);
	return 0;
}

int SoundBankSelect::GetInstrNum()
{
	SBInstr *ins = GetInstr();
	if (ins)
		return ins->instrNdx;
	return -1;
}

