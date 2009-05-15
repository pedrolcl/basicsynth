#ifndef TONESYNTH_ED_H
#define TONESYNTH_ED_H

class ToneSynthEdit : public SynthEdit
{
public:
	ToneSynthEdit();
	~ToneSynthEdit();

	void SaveValues();
	void ValueChanged(SynthWidget *wdg);
};

class ToneFMSynthEdit : public ToneSynthEdit
{
public:
	ToneFMSynthEdit();
};

#endif
