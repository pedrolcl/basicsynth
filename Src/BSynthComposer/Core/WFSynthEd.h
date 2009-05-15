#ifndef WFSYNTH_ED_H
#define WFSYNTH_ED_H

class WFSynthEdit : public SynthEdit
{
protected:

public:
	WFSynthEdit();
	~WFSynthEdit();

	void SaveValues();
	void ValueChanged(SynthWidget *wdg);
};
#endif
