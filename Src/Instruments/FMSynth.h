// FMSynth.h: interface for the FMSynth class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_FMSYNTH_H_)
#define _FMSYNTH_H_

#include "LFO.h"

#define ALG_STACK 1
#define ALG_STACK2 2
#define ALG_WYE 3
#define ALG_DELTA 4

class FMSynth : public Instrument
{
private:
	GenWaveWT  carOsc;
	EnvGenADSR carEG;
	EnvDef     carEnvDef;
	FrqValue   carMult;
	AmpValue   fmMix;
	AmpValue   fmDly;
	long algorithm;

	GenWaveWT  m1Osc;
	EnvGenADSR m1EG;
	EnvDef     m1EnvDef;
	FrqValue   m1Mult;

	GenWaveWT  m2Osc;
	EnvGenADSR m2EG;
	EnvDef     m2EnvDef;
	FrqValue   m2Mult;

	GenWaveNoise nz;
	EnvGenADSR   nzEG;
	EnvDef       nzEnvDef;
	AmpValue     nzMix;
	AmpValue     nzDly;
	int nzOn;

	FilterLP   filt;
	EnvGenADSR filtEG;
	EnvDef     filtEnvDef;
	AmpValue   filtGain;
	
	AllPassDelay apd;
	FrqValue     dlyTim;
	FrqValue     dlyDec;
	AmpValue     dlyMix;
	long dlySamps;
	int dlyOn;
	LFO lfoGen;

	int chnl;
	FrqValue frq;
	AmpValue vol;
	AmpValue maxPhs;

	InstrManager *im;

	AmpValue CalcPhaseMod(AmpValue amp, FrqValue mult);
	void LoadEG(XmlSynthElem *elem, EnvDef& eg);
	XmlSynthElem *SaveEG(XmlSynthElem *parent, char *tag, EnvDef& eg);

	void SetParams(VarParamEvent *evt);

public:
	FMSynth();
	virtual ~FMSynth();
	static Instrument *FMSynthFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *FMSynthEventFactory(Opaque tmplt);
	void Copy(FMSynth *tp);
	void Start(SeqEvent *evt);
	void Param(SeqEvent *evt);
	void Stop();
	void Tick();
	int  IsFinished();
	void Destroy();

	int Load(XmlSynthElem *parent);
	int Save(XmlSynthElem *parent);
};

#endif
