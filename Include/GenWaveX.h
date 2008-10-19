///////////////////////////////////////////////////////////////
//
// BasicSynth - GenWaveX
//
// Various complex spectrum waveform generators using wavetables
//
// GenWaveSum - sum of waveforms, typically for additive synth, but other uses as well.
// GenWaveFM - frequency modulation generator, one modulator
// GenWaveAM - amplitude modulation generator
// GenWaveRM - ring modulation generator
// GenWaveNZ - pitched noise
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////////
#ifndef _GENWAVEX_H_
#define _GENWAVEX_H_

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
		AmpValue sigma;
	} *parts;
	int   numPart;
	int   cntPart;
	PhsAccum maxMult;
	AmpValue scale;
	AmpValue sigK;
	AmpValue sigN;
	AmpValue sigTL;
	int gibbs;
public:
	GenWaveSum()
	{
		parts = NULL;
		numPart = 0;
		cntPart = 0;
		sigK = 0;
		scale = 1;
		gibbs = 0;
		maxMult = 0;
	}

	~GenWaveSum()
	{
		delete[] parts;
	}

	// Fo, WT, gibbs, n, {part,amp}*n
	virtual void Init(int n, float *v)
	{
		if (n > 3)
		{
			gibbs = (int) v[2];
			AllocParts((int)v[3]);
			int n2 = n - 4;
			float *v2 = v + 4;
			for (int i = 0; i < numPart && n2 >= 2; i++)
			{
				SetPart(i, v2[0], v2[1]);
				n2 -= 2;
				v2 += 2;
			}
		}
		// the base class will call Reset()
		GenWaveWT::Init(n, v);
	}

	void InitParts(int n, float *m, float *a, int g = 0)
	{
		maxMult = 0;
		gibbs = g;
		AllocParts(n);
		for (int i = 0; i < n; i++)
			SetPart(i, m[i], a[i]);
		Reset();
		CalcParts();
	}

	virtual void Reset(float initPhs = 0)
	{
		GenWaveWT::Reset(initPhs);
		CalcParts();
		if (initPhs >= 0)
		{
			SumPart *pp = parts;
			SumPart *pe = &parts[numPart];
			while (pp < pe)
			{
				pp->phase = 0;
				pp++;
			}
		}
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
		maxMult = 0;
		cntPart = 0;
		scale = 1;
	}

	void SetPart(int n, float mul, float amp)
	{
		SumPart *pp = &parts[n];
		pp->amp = AmpValue(amp);
		pp->sigma = pp->amp;
		pp->mul = PhsAccum(mul);
		pp->incr = 0;
		pp->phase = 0;
		if (pp->mul > maxMult)
			maxMult = pp->mul;
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

	virtual void PhaseModWT(PhsAccum phs)
	{
		SumPart *pp = parts;
		SumPart *pe = &parts[numPart];
		while (pp < pe)
		{
			pp->phase += phs * pp->mul;
			while (pp->phase >= synthParams.ftableLength)
				pp->phase -= synthParams.ftableLength;
			while (pp->phase < 0)
				pp->phase += synthParams.ftableLength;
			pp++;
		}
	}

	// NB - indexIncr is calculated in the base class Reset method.
	void CalcParts()
	{
		FrqValue tld2 = synthParams.ftableLength / 2;
		scale = 0;
		cntPart = 0;
		AmpValue sigK = PI / maxMult;
		AmpValue sigN;
		AmpValue sigTL = tld2/PI;
		SumPart *pp = parts;
		SumPart *pe = &parts[numPart];
		while (pp < pe)
		{
			pp->incr = indexIncr * pp->mul;
			if (pp->incr < tld2)
			{
				if (gibbs && pp->mul > 0)
				{
					sigN = sigK * pp->mul;
					pp->sigma = (wtSet.wavSin[(int)((sigN*sigTL)+0.5)] / sigN) * pp->amp;
				}
				else
					pp->sigma = pp->amp;
				scale += (AmpValue) fabs((double)pp->sigma);
				cntPart++;
			}
			else
				pp->sigma = 0;
			pp++;
		}
	}

	virtual AmpValue Gen()
	{
		if (cntPart < 1)
			return 0;

		AmpValue val = 0;
		SumPart *pp = parts;
		SumPart *pe = &parts[numPart];
		while (pp < pe)
		{
			val += waveTable[(int)(pp->phase+0.5)] * pp->sigma;
			pp->phase += pp->incr;
			if (pp->phase >= synthParams.ftableLength)
				pp->phase -= synthParams.ftableLength;
			pp++;
		}
		return val / scale;
	}
};


// FM (PM) Generator
// Any WT class has a modulator input. This special class has the
// modulator oscillator built in for convienence.
// The Modulate() method also works, and can be used for LFO.
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
	virtual void Init(int n, float *v)
	{
		if (n > 3)
			InitFM(FrqValue(v[0]), FrqValue(v[2]), AmpValue(v[3]), (int) v[1]);
	}

	virtual void InitFM(FrqValue frequency, FrqValue mult, AmpValue mi, int wtIndex)
	{
		indexOfMod = PhsAccum(mi);
		modMult = mult;
		GenWaveWT::InitWT(frequency, wtIndex);
	}

	inline void CalcModAmp()
	{
		//modAmp = synthParams.frqTI * indexOfMod * frq * modMult;
		modAmp = indexOfMod * modIncr;
	}

	void SetModIndex(AmpValue iom)
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
		index += indexIncr + (PhsAccum(valMod) * modAmp);
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
	virtual void Init(int n, float *v)
	{
		if (n > 3)
			InitAM(FrqValue(v[0]), FrqValue(v[2]), FrqValue(v[3]), (int) v[1]);
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
		modIncr = synthParams.frqTI * PhsAccum(modFrq);
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

///////////////////////////////////////////////////////////
// Pitched noise - ring modulation of a sine wave and noise
// See: Computer Music, Dodge&Jerse, chapter 4.11b
///////////////////////////////////////////////////////////
class GenWaveNZ : public GenUnit
{
private:
	GenWaveWT osc;
	GenNoiseI nz;

public:
	// Fo WT Fn
	virtual void Init(int n, float *v)
	{
		if (n > 2)
			InitNZ(FrqValue(v[0]), FrqValue(v[2]), (int) v[1]);
	}

	virtual void InitNZ(FrqValue frequency, FrqValue nzfrq, int wtIndex)
	{
		osc.InitWT(frequency, wtIndex);
		nz.Init(1, &nzfrq);
	}

	virtual void Reset(float initPhs = 0)
	{
		osc.Reset(initPhs);
		nz.Reset(initPhs);
	}

	virtual AmpValue Sample(AmpValue in)
	{
		return Gen() * in;
	}

	virtual AmpValue Gen()
	{
		return osc.Gen() * nz.Gen();
	}
};


#endif

