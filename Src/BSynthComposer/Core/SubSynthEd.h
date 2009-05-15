#ifndef SUBSYNTH_EDIT_H
#define SUBSYNTH_EDIT_H

class SubSynthEdit : public SynthEdit
{
private:
	void SetResonRange(int ft, int draw);

public:
	SubSynthEdit();
	~SubSynthEdit();

	void LoadValues();
	void ValueChanged(SynthWidget *wdg);
};

#endif
