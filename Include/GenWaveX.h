///////////////////////////////////////////////////////////////
//
// BasicSynth - GenWaveWT
//
// Various complex spectrum waveform generators using wavetables
//
// GenWaveSum - sum of waveforms, typically for additive synth, but other uses as well.
// GenWaveFM - frequency modulation generator, one modulator
// GenWaveAM - amplitude modulation generator
// GenWaveRM - ring modulation generator
//
// Daniel R. Mitchell
///////////////////////////////////////////////////////////////
#ifndef _GENWAVEX_H_
#define _GENWAVEX_H_

#include "GenWaveWT.h"

// Incremental calculation of sum of waves - slower than precalc,
// but BW limited and also useful for doubling, chorus effects, etc.
class GenWaveSum : public GenWaveWT
{
private:
	struct SumPart
	{
		PhsAccum phase;
		PhsAccum incr;
		PhsAccum mul;
		AmpValue amp;
	} *parts;
	int   maxPart;
	int   numPart;
	AmpValue scale;
	AmpValue sigK;
	AmpValue sigN;
	AmpValue sigTL;
	int gibbs;
public:
	GenWaveSum()
	{
		parts = NULL;
		maxPart = 0;
		numPart = 0;
		sigK = 0;
		scale = 1;
		gibbs = 0;
	}

	~GenWaveSum()
	{
		delete[] parts;
	}

	// Fo, WT, n, {part,amp}*n, Gibbs
	void Init(int n, float *p)
	{
		if (n > 2)
		{
			AllocParts((int)p[2]);
			int n2 = n - 3;
			float *p2 = p + 3;
			for (int i = 0; i < numPart && n2 >= 2; i++)
			{
				SetPart(i, p2[0], p2[1]);
				n2 -= 2;
				p2 += 2;
			}
			if (n2 > 0)
				gibbs = (int) p2[0];
		}
		GenWaveWT::Init(n, p);
	}

	void InitParts(int n, float *m, float *a, int g = 0)
	{
		gibbs = g;
		AllocParts(n);
		for (int i = 0; i < n; i++)
			SetPart(i, m[i], a[i]);
		Reset();
		CalcParts();
	}

	void AllocParts(int n)
	{
		if (parts)
		{
			delete[] parts;
			parts = NULL;
		}
		parts = new SumPart[n];
		numPart = n;
		maxPart = 0;
		scale = 1;
		// Lanczos sigma factor:
		sigK = (AmpValue) (PI / numPart);
	}

	void SetPart(int n, float mul, float amp, float phs = 0)
	{
		SumPart *pp = &parts[n];
		pp->amp = (AmpValue)amp;
		pp->mul = (PhsAccum)mul;
		pp->incr = 0;
		pp->phase = phs;
		if (n > maxPart)
			maxPart = n;
	}

	void SetGibbs(int n)
	{
		gibbs = n;
	}

	virtual void Modulate(FrqValue d)
	{
		GenWaveWT::Modulate(d);
		CalcParts();
	}

	virtual void Reset(float initPhs = 0)
	{
		GenWaveWT::Reset(initPhs);
		CalcParts();
		for (int np = 0; np < numPart; np++)
			parts[np].phase = 0;
	}

	virtual void PhaseModWT(PhsAccum phs)
	{
		SumPart *pp = parts;
		for (int np = 0; np < numPart; np++)
		{
			pp->phase += phs * pp->mul;
			if (pp->phase >= synthParams.ftableLength)
			{
				do
					pp->phase -= synthParams.ftableLength;
				while (pp->phase >= synthParams.ftableLength);
			}
			else if (pp->phase < 0)
			{
				do
					pp->phase += synthParams.ftableLength;
				while (pp->phase < 0);
			}
			pp++;
		}
	}

	void CalcParts()
	{
		FrqValue tld2 = synthParams.ftableLength / 2;
		maxPart = 0;
		scale = 0;
		SumPart *pp = parts;
		for (int np = 0; np < numPart; np++)
		{
			scale += (AmpValue) fabs((double)pp->amp);
			pp->incr = indexIncr * pp->mul;
			if (pp->incr < tld2)
				maxPart++;
			pp++;
		}
		// Lanczos sigma factor:
		if (maxPart > 0)
			sigK = (AmpValue) (PI / maxPart);
		else
			sigK = 0;
		sigTL = tld2/PI;
	}

	virtual AmpValue Gen()
	{
		if (maxPart < 1)
			return 0;

		SumPart *pp = parts;
		AmpValue val = 0;
		AmpValue amp;
		AmpValue sigN = sigK;
		for (int n = 0; n < maxPart; n++)
		{
			if (gibbs && n > 0)
			{
				// use sin wavetable
				//amp = pp->amp * (sinv(sigN) / sigN);
				amp = pp->amp * (wtSet.wavSin[(int)((sigN*sigTL)+0.5)] / sigN);
				sigN += sigK;
			}
			else
				amp = pp->amp;
			val += waveTable[(int)pp->phase] * amp;
			pp->phase += pp->incr;
			if (pp->phase >= synthParams.ftableLength)
				pp->phase -= synthParams.ftableLength;
			pp++;
		}
		// hack around round-off errors if needed
		//if (val > scale)
		//	scale = val;
		return val / scale;
	}
};


// FM (PM) Generator
// Any WT class has a modulator input. This special class has the
// modulator oscillator built in for convienence.
// The Modulate() method still works, and can be used for LFO.
class GenWaveFM : public GenWaveWT
{
private:
	FrqValue modMult;
	PhsAccum modAmp;
	PhsAccum modIncr;
	PhsAccum modIndex;
	PhsAccum indexOfMod;

public:
	GenWaveFM()
	{
		modMult = 1.0;
		modAmp = 0.0;
		modIncr = 0.0;
		modIndex = 0.0;
		indexOfMod = 1;
	}

	// Fc, WT, H, I
	virtual void Init(int n, float *p)
	{
		if (n > 2)
		{
			modMult = (PhsAccum)p[2];
			if (n > 3)
				indexOfMod = (PhsAccum)p[3];
		}
		GenWave::Init(n, p);
	}

	virtual void InitFM(FrqValue frequency, FrqValue mult, AmpValue mi, int wtIndex)
	{
		indexOfMod = (PhsAccum)mi;
		modMult = mult;
		GenWaveWT::InitWT(frequency, wtIndex);
	}

	inline void CalcModAmp()
	{
		//modAmp = synthParams.frqTI * indexOfMod * frq * modMult;
		modAmp = indexOfMod * modIncr;
	}

	virtual void SetModIndex(AmpValue iom)
	{
		indexOfMod = (PhsAccum)iom;
		CalcModAmp();
	}

	AmpValue GetModIndex()
	{ 
		return indexOfMod; 
	}

	void SetModMultiple(FrqValue m)
	{
		modMult = m;
	}

	FrqValue GetModMultiple()
	{
		return modMult;
	}

	virtual void Reset(float initPhs = 0)
	{
		GenWaveWT::Reset(initPhs);
		if (initPhs >= 0)
			modIndex = initPhs * synthParams.radTI;
		modIncr = indexIncr * modMult;
		if (modIncr >= (synthParams.ftableLength/2))
			modIncr = synthParams.ftableLength/2;
		CalcModAmp();
	}

	virtual AmpValue Gen()
	{
		AmpValue valMod;
		AmpValue valCar;

		valCar = waveTable[(int)(index+0.5)];
		valMod = waveTable[(int)(modIndex+0.5)];
		index += indexIncr + ((PhsAccum)valMod * modAmp);
		if (index >= synthParams.ftableLength)
			index -= synthParams.ftableLength;
		else if (index < 0)
			index += synthParams.ftableLength;
		modIndex += modIncr;
		if (modIndex >= synthParams.ftableLength)
			modIndex -= synthParams.ftableLength;
		
		return valCar;
	}
};

// AM Generator (2-quadrant multiply)
class GenWaveAM : public GenWaveWT
{
protected:
	FrqValue modFrq;
	AmpValue modAmp;
	PhsAccum modIncr;
	PhsAccum modIndex;
	AmpValue modScale;

public:
	GenWaveAM()
	{
		modFrq = 0.0;
		modAmp = 0.0;
		modIncr = 0.0;
		modIndex = 0.0;
		modScale = 0.0;
	}

	// Fc, WT, Fm, Am
	virtual void Init(int n, float *p)
	{
		if (n > 2)
		{
			modFrq = (PhsAccum)p[2];
			if (n > 3)
				modAmp = (PhsAccum)p[3];
		}
		GenWave::Init(n, p);
	}

	virtual void InitAM(FrqValue frequency, FrqValue mfrq, AmpValue mamp, int wtIndex)
	{
		modAmp = mamp;
		modFrq = mfrq;
		modScale = 1.0 / (1.0 + modAmp);
		GenWaveWT::InitWT(frequency, wtIndex);
	}

	virtual void Reset(float initPhs = 0)
	{
		GenWaveWT::Reset(initPhs);
		modIncr = synthParams.frqTI * (PhsAccum) modFrq;
		if (initPhs >= 0)
		{
			modIndex = initPhs * synthParams.radTI;
			while (modIndex >= synthParams.ftableLength)
				modIndex -= synthParams.ftableLength;
		}
	}

	virtual AmpValue Gen()
	{
		AmpValue valMod = 1.0 + (modAmp * waveTable[(int)(modIndex+0.5)]);
		AmpValue v = waveTable[(int)(index+0.5)] * valMod * modScale;
		if ((index += indexIncr) >= synthParams.ftableLength)
			index -= synthParams.ftableLength;
		if ((modIndex += modIncr) >= synthParams.ftableLength)
			modIndex -= synthParams.ftableLength;
		return v;
	}
};

// Ring modulator (i.e. 4-quadrant multiply)
class GenWaveRM : public GenWaveAM
{
public:
	virtual AmpValue Gen()
	{
		AmpValue v = waveTable[(int)(index+0.5)] * modAmp * waveTable[(int)(modIndex+0.5)];
		if ((index += indexIncr) >= synthParams.ftableLength)
			index -= synthParams.ftableLength;
		if ((modIndex += modIncr) >= synthParams.ftableLength)
			modIndex -= synthParams.ftableLength;
		return v;
	}
};

#endif

