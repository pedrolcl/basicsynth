/////////////////////////////////////////////////////////////////
/// SynthEdit provides a generic editor for instruments dervied
/// from InstrumentVP. For instruments where there is one widget
/// for each editable parameter, the derived class only needs
/// to implement code to setup knob/label pairs in SetInstrument
/// and handle specialized ValueChanged functions. More complex
/// instruments where only some values are displayed must
/// implement specialized code for LoadValues and ValueChanged.
/// (See MatSynthEd for an example.)
////////////////////////////////////////////////////////////////
#ifndef _SYNTHEDIT_H_
#define _SYNTHEDIT_H_

#include "WidgetForm.h"

class SynthEdit : public WidgetForm
{
protected:
	InstrConfig *instr;
	InstrumentVP  *tone;
	VarParamEvent *parms;
	VarParamEvent *save;
	int changed;

public:
	SynthEdit();
	virtual ~SynthEdit();
	virtual int IsChanged();
	virtual void SetInstrument(InstrConfig *ip);
	virtual void SetPair(int knobID, int lblID);
	virtual void ValueChanged(SynthWidget *wdg);
	virtual int SelectWavetable(int wt);
	virtual void SelectWavetable(SynthWidget *wdg, int draw = 1);

	virtual void Cancel();
	virtual void GetParams();
	virtual void SetParams();
	virtual void LoadValues();
	virtual void SaveValues();
	virtual void SaveValues(int *ips);
};

#endif
