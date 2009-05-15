#ifndef _FMSYNTH_EDIT_H
#define _FMSYNTH_EDIT_H

class FMSynthEdit : public SynthEdit
{
public:
	FMSynthEdit();
	~FMSynthEdit();
	
	void SaveValues();
	void ValueChanged(SynthWidget *wdg);
};

#endif
