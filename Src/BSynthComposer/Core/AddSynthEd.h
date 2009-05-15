#ifndef _ADDSYNTH_ED_H
#define _ADDSYNTH_ED_H

#define MAXADDPART 16
#define MAXADDSEG 7

class AddSynthEdit : public SynthEdit
{
private:
	int curPart;
	int numParts;
	AddSynth *as;
	SynthWidget *genWdg;
	SynthWidget *rtRange;
	GraphWidget *frqGraph;
	EnvelopeWidget *egWdg;

	void EnableParts();
	void EnableEnvSegs(int ns, int redraw);
	void SetRateRange(float rtVal);

public:
	AddSynthEdit();
	virtual ~AddSynthEdit();

	virtual void SetInstrument(InstrConfig *ip);
	virtual void GetParams();
	virtual void SaveValues();
	virtual void LoadValues();
	virtual void ValueChanged(SynthWidget *wdg);

};

#endif
