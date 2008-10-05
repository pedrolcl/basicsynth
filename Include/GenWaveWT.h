///////////////////////////////////////////////////////////////
//
// BasicSynth - GenWaveWT
//
// Various wavetable waveform generators
//
// GenWaveWT - basic wavetable generator, non-interpolating
// GenWaveI - wavetable generator with interpolation
// GenWave32 - wavetable generator w/ fast 32-bit fixed point index
// GenWave64 - wavetable generator w/ fast 64-bit fixed point index
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////////
#ifndef _GENWAVET_H_
#define _GENWAVET_H_

#include "WaveTable.h"
#include "GenWave.h"

// Generic wavetable based generator (oscillator)
class GenWaveWT : public GenWave
{
public:
	AmpValue *waveTable;
	int   wtIndex;

	GenWaveWT()
	{
		wtIndex = WT_SIN;
		waveTable = wtSet.wavSet[wtIndex];
	}

	virtual void Reset(float initPhs = 0)
	{
		indexIncr = (PhsAccum) frq * synthParams.frqTI;
		while (indexIncr >= synthParams.ftableLength)
			indexIncr -= synthParams.ftableLength;
		while (indexIncr < 0)
			indexIncr += synthParams.ftableLength;
		if (initPhs >= 0)
		{
			index = initPhs * synthParams.radTI;
			while (index >= synthParams.ftableLength)
				index -= synthParams.ftableLength;
		}
	}

	virtual void Modulate(FrqValue d)
	{
		indexIncr = (PhsAccum)(frq + d) * synthParams.frqTI;
		while (indexIncr < 0)
			indexIncr += synthParams.ftableLength;
		PhsAccum maxInc = synthParams.ftableLength / 2; 
		if (indexIncr >= maxInc)
			indexIncr = maxInc;
	}

	virtual void PhaseMod(PhsAccum phs)
	{
		PhaseModWT(phs * synthParams.radTI);
	}

	// Here, phase offset is in fraction of table length.
	// This can be used by instruments that know the oscil
	// is a WT type and adjust the modulator amplitude
	// accordingly, i.e. set the LFO/EG amp to the tableLength/2 range,
	// and thus avoid the extra calculations on each sample.
	virtual void PhaseModWT(PhsAccum phs)
	{
		index += phs;
		if (index >= synthParams.ftableLength)
		{
			do
				index -= synthParams.ftableLength;
			while (index >= synthParams.ftableLength);
		}
		else if (index < 0)
		{
			do
				index += synthParams.ftableLength;
			while (index < 0);
		}
	}

	inline void SetWavetable(int wti)
	{
		if (wti >= 0 && wti < wtSet.wavTblMax)
		{
			waveTable = wtSet.wavSet[wtIndex = wti];
			if (waveTable == NULL)
				waveTable = wtSet.wavSet[WT_SIN];
		}
	}

	inline int GetWavetable()
	{
		return wtIndex;
	}

	void InitWT(FrqValue f, int wti)
	{
		SetWavetable(wti);
		SetFrequency(f);
		Reset();
	}

	// Fo WT
	virtual void Init(int n, float *v)
	{
		if (n > 0)
		{
			SetFrequency(FrqValue(v[0]));
			if (n > 1)
				SetWavetable((int)v[1]);
		}
		Reset();
	}

	// Generate a sample with wavetable lookup
	// Note: it's OK to round-up index since tables have guard point.
	virtual AmpValue Gen()
	{
		AmpValue v = waveTable[(int)(index+0.5)];
		if ((index += indexIncr) >= synthParams.ftableLength)
			index -= synthParams.ftableLength;
		return v;
	}
};

// Wavetable generator with simple linear interpolation.
// This significantly improves the quality of signals 
// when shorter wave tables (< 4096) are used.
// With longer wavetables, the benefit is a little less.
class GenWaveI : public GenWaveWT
{
public:
	virtual AmpValue Gen()
	{
		// fract = index - floor(index);
		int intIndex = (int) index;
		PhsAccum fract = index - (PhsAccum) intIndex;
		AmpValue v1 = waveTable[intIndex];
		AmpValue v2 = waveTable[intIndex+1];
		AmpValue value = v1 + ((v2 - v1) * (AmpValue)fract);
		if ((index += indexIncr) >= synthParams.ftableLength)
			index -= synthParams.ftableLength;
		return value;
	}
};

// Fast wavetable generator using Q16.16 index
// limit wavetable size to <= 16k entries
class GenWave32 : public GenWaveWT
{
private:
	bsInt32 i32Index;
	bsInt32 i32IndexIncr;
	bsInt32 i32IndexMask;

public:
	GenWave32()
	{
		i32Index = 0;
		i32IndexIncr = 0;
		i32IndexMask = (synthParams.itableLength << 16) - 1;
	}

	inline void CalcPhase()
	{
		i32IndexIncr = (bsInt32) (indexIncr * 65536.0);
	}

	virtual void Modulate(FrqValue d)
	{
		GenWaveWT::Modulate(d);
		CalcPhase();
	}

	// N.B. - abs(phs) should NEVER be > tableLength/2
	virtual void PhaseModWT(PhsAccum phs)
	{
		if (phs >= synthParams.ftableLength)
			phs -= synthParams.ftableLength;
		else if (phs < 0)
			phs += synthParams.ftableLength;
		i32Index += (bsInt32) (phs * 65536.0) & i32IndexMask;
	}

	virtual void Reset(float initPhs = 0)
	{
		GenWaveWT::Reset(initPhs);
		CalcPhase();
		if (initPhs >= 0)
			i32Index = (bsInt32) (index * 65536,0);
	}

	virtual AmpValue Gen()
	{
		AmpValue v = waveTable[(i32Index + 0x8000) >> 16];
		i32Index = (i32Index + i32IndexIncr) & i32IndexMask;
		return v; 
	}
};
#endif

