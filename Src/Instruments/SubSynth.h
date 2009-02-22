//////////////////////////////////////////////////////////////////////
// BasicSynth Subtractive synthesis instrument
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#if !defined(_SUBSYNTHINSTR_H_)
#define _SUBSYNTHINSTR_H_

#include "LFO.h"
#include "PitchBend.h"

class SubFilt
{
protected:
	EnvGenADSR *egFilt;
	AmpValue res;
	AmpValue gain;
	bsInt32 coefRate;
	bsInt32 coefCount;
public:
	SubFilt()
	{
		egFilt = 0;
		res = 0;
		gain = 0;
		coefRate = 0;
		coefCount = 0;
	}
	virtual ~SubFilt() { }
	virtual AmpValue Sample(AmpValue in) { return in; }
	virtual void Init(EnvGenADSR *eg, AmpValue g, AmpValue r)
	{
		egFilt = eg;
		gain = g;
		res = r;
	}
	virtual void Reset(float ) { }
	virtual void Copy(SubFilt *tp) { coefRate = tp->coefRate; }
	virtual void SetCalcRate(float f)
	{
		coefRate = (bsInt32) (f * 0.001 * synthParams.sampleRate);
	}
};

class SubFiltLP : public SubFilt
{
private:
	FilterLP filt;
public:
	virtual AmpValue Sample(AmpValue in)
	{
		AmpValue f = egFilt->Gen();
		if (--coefCount <= 0)
		{
			filt.Init(f, gain);
			coefCount = coefRate;
		}
		return filt.Sample(in);
	}
	virtual void Reset(float initPhs)
	{
		if (initPhs >= 0)
			filt.Init(egFilt->GetStart(), gain);
		coefCount = coefRate;
		filt.Reset(initPhs);
	}
	virtual void Copy(SubFilt *tp)
	{
		SubFilt::Copy(tp);
		filt.Copy(&((SubFiltLP*)tp)->filt);
	}
};

class SubFiltHP : public SubFilt
{
private:
	FilterHP filt;
public:
	virtual AmpValue Sample(AmpValue in)
	{
		AmpValue f = egFilt->Gen();
		if (--coefCount <= 0)
		{
			filt.Init(f, gain);
			coefCount = coefRate;
		}
		return filt.Sample(in);
	}
	virtual void Reset(float initPhs)
	{
		if (initPhs >= 0)
			filt.Init(egFilt->GetStart(), gain);
		coefCount = coefRate;
		filt.Reset(initPhs);
	}
	virtual void Copy(SubFilt *tp)
	{
		SubFilt::Copy(tp);
		filt.Copy(&((SubFiltHP*)tp)->filt);
	}
};

class SubFiltBP : public SubFilt
{
private:
	FilterBP filt;
public:
	virtual AmpValue Sample(AmpValue in)
	{
		AmpValue f = egFilt->Gen();
		if (--coefCount <= 0)
		{
			filt.Init(f, gain, res);
			coefCount = coefRate;
		}
		return filt.Sample(in);
	}
	virtual void Reset(float initPhs)
	{
		if (initPhs >= 0)
			filt.Init(egFilt->GetStart(), gain, res);
		coefCount = coefRate;
		filt.Reset(initPhs);
	}
	virtual void Copy(SubFilt *tp)
	{
		SubFilt::Copy(tp);
		filt.Copy(&((SubFiltBP*)tp)->filt);
	}
};

class SubFiltRES : public SubFilt
{
private:
	Reson filt;
public:
	virtual AmpValue Sample(AmpValue in)
	{
		AmpValue f = egFilt->Gen();
		if (--coefCount <= 0)
		{
			filt.Init(f, res);
			coefCount = coefRate;
		}
		return filt.Sample(in);
	}
	virtual void Reset(float initPhs)
	{
		if (initPhs >= 0)
			filt.InitRes(egFilt->GetStart(), gain, res);
		coefCount = coefRate;
		filt.Reset(initPhs);
	}
	virtual void Copy(SubFilt *tp)
	{
		SubFilt::Copy(tp);
		filt.Copy(&((SubFiltRES*)tp)->filt);
	}
};

class SubSynth  : public Instrument
{
private:
#ifdef USE_OSCILI
	GenWaveI osc;
#else
	GenWaveWT osc;
#endif
	GenNoise nz;
	EnvGenADSR   envSig;
	SubFilt *filt;
	EnvGenADSR   envFlt;
	LFO lfoGen;
	PitchBend pbGen;
	PitchBendWT pbWT;
	AmpValue vol;
	AmpValue sigMix;
	AmpValue nzMix;
	AmpValue fltGain;
	AmpValue fltRes; // only if Reson filter
	short fltType; // 0=LP, 1=HP, 2=BP, 3=RES
	float coefRate;
	int chnl;
	int nzOn;
	int pbOn;
	InstrManager *im;

public:
	SubSynth();
	SubSynth(SubSynth *tp);
	virtual ~SubSynth();
	static Instrument *SubSynthFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *SubSynthEventFactory(Opaque tmplt);
	static bsInt16    MapParamID(const char *name);

	void Copy(SubSynth *tp);
	void CreateFilter();
	virtual void Start(SeqEvent *evt);
	virtual void Param(SeqEvent *evt);
	virtual void Stop();
	virtual void Tick();
	virtual int  IsFinished();
	virtual void Destroy();

	int Load(XmlSynthElem *parent);
	int Save(XmlSynthElem *parent);
	int GetParams(VarParamEvent *params);
	int SetParams(VarParamEvent *params);
	int SetParam(bsInt16 id, float val);
};

#endif
