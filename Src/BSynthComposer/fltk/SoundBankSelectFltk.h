#ifndef SOUNDBANKSELECT_H
#define SOUNDBANKSELECT_H

class SoundBankSelect : public 	Fl_Window
{
protected:
	int doneSet;
	Fl_Button *okBtn;
	Fl_Button *canBtn;
	Fl_Choice *fileList;
	Fl_Hold_Browser *bankList;
	Fl_Hold_Browser *presetList;

	void SwitchFile();
	void SwitchBank();
	SoundBank *GetFile();
	SBInstr *GetPreset();
	int GetBankNum();
	int GetPresetNum();

public:
	bsString fileID;
	bsString preName;
	bsInt16 bnkNum;
	bsInt16 preNum;

	SoundBankSelect();
	~SoundBankSelect();

	int DoModal();
	void OnOK();
	void OnCancel();
	void OnFileChange();
	void OnBankChange();
};

#endif
