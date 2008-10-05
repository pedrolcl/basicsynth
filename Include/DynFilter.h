///////////////////////////////////////////////////////////
// BasicSynth - DynFilter
//
// This is a combination Filter/Envelope Generators that 
// is optimized for classic subtractive synthesis where
// the filter cutoff frequency is constantly varied with
// an envelope generator. 
// The tangent is calculated with a lookup from a sin wave table
// to save processing time. This is slightly less accurate, but
// for a constantly changing cutoff frequency, there should be
// no audible difference. Coefficents are only calculated when
// the table index value changes, saving additional time.
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////
#ifndef _DYNFILTER_H_
#define _DYNFILTER_H_

//#include <BiQuad.h>

class DynFilterLP : public BiQuadFilter
{
private:
	EnvGenADSR env;
	FrqValue frqTI; // convert fc to table index, 0 - PI
	int cosOffs; // offset of cosine values in sin wave table
	int lastNdx;
	AmpValue *sinTable;

public:
	DynFilterLP()
	{
		sinTable = wtSet.wavSet[WT_SIN];
		// this is for PI / sampleRate conversion...
		frqTI = (synthParams.ftableLength / 2) / synthParams.sampleRate;
		cosOffs = synthParams.itableLength / 4;
		lastNdx = 0;
	}

	AmpValue Sample(AmpValue in)
	{
		int tndx = (int) (env.Gen() + 0.5);
		if (tndx < 0 || tndx >= cosOffs)
			return in; // filter off or out of range
		if (tndx != lastNdx)
		{
			lastNdx = tndx;
			// c = 1 / tan((PI / synthParams.sampleRate) * cutoff);
			double c = (double) sinTable[tndx+cosOffs] / (double) sinTable[tndx];
			double c2 = c * c;
			double csqr2 = sqr2 * c;
			double oned = 1.0 / (c2 + csqr2 + 1.0);

			ampIn0 = oned;
			ampIn1 = oned + oned;
			ampIn2 = oned;
			ampOut1 = (2.0 * (1.0 - c2)) * oned;
			ampOut2 = (c2 - csqr2 + 1.0) * oned;
		}
		return BiQuadFilter::Sample(in);
	}

	void SetStart(AmpValue val)  { env.SetStart(val); }
	void SetAtkRt(FrqValue val)  { env.SetAtkRt(val); }
	void SetAtkLvl(AmpValue val) { env.SetAtkLvl(val); }
	void SetDecRt(FrqValue val)  { env.SetDecRt(val); }
	void SetSusLvl(AmpValue val) { env.SetSusLvl(val); }
	void SetRelRt(FrqValue val)  { env.SetRelRt(val); }
	void SetRelLvl(AmpValue val) { env.SetRelLvl(val); }
	void SetType(EGSegType ty)   { env.SetType(ty); }

	AmpValue GetStart() { return env.GetStart(); }
	FrqValue GetAtkRt() { return env.GetAtkRt(); }
	AmpValue GetAtkLvl(){ return env.GetAtkLvl(); }
	FrqValue GetDecRt() { return env.GetDecRt(); }
	AmpValue GetSusLvl(){ return env.GetSusLvl(); }
	FrqValue GetRelRt() { return env.GetRelRt(); }
	AmpValue GetRelLvl(){ return env.GetRelLvl(); }
	EGSegType GetType() { return env.GetType(); }

	void InitFilter(AmpValue st, FrqValue ar, AmpValue al, FrqValue dr, 
	                AmpValue sl, FrqValue rr, AmpValue rl, EGSegType t = linSeg, AmpValue fg = 1.0)
	{
		BiQuadFilter::Init(0, fg);
		env.InitADSR(st*frqTI, ar, al*frqTI, dr, sl*frqTI, rr, rl*frqTI, t);
		Reset(0);
	}

	void Copy(DynFilterLP *fp)
	{
		BiQuadFilter::Copy(fp);
		env.Copy(&fp->env);
	}

	void GetEnvDef(EnvDef *e)
	{
		env.GetEnvDef(e);
	}

	void SetEnvDef(EnvDef *e)
	{
		env.GetEnvDef(e);
	}

	void Reset(float initPhs)
	{
		env.Reset(initPhs);
	}

	void Release()
	{
		env.Release();
	}
};
#endif
