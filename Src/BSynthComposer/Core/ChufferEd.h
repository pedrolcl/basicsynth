#ifndef _CHUFFER_EDIT_H
#define _CHUFFER_EDIT_H

class ChufferEdit : public SynthEdit
{
protected:
	void SetFRange(SynthWidget *wdg, int draw = 0);
	void SetQRange(SynthWidget *wdg, int draw = 0);

public:
	ChufferEdit();
	~ChufferEdit();

	void GetParams();
	void SaveValues();
	void ValueChanged(SynthWidget *wdg);
};

#endif
