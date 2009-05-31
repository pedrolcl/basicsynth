///////////////////////////////////////////////////////////////
//
// BasicSynth - GenWaveX
//
/// @file GenWaveX.h Various complex spectrum waveform generators using wavetables.
//
// GenWaveSum - sum of waveforms, typically for additive synth, but other uses as well.
// GenWaveFM - frequency modulation generator, one modulator
// GenWaveAM - amplitude modulation generator
// GenWaveRM - ring modulation generator
// GenWaveNZ - pitched noise
// GenWaveBuzz - pulse generator
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////////
/// @addtogroup grpOscil
//@{
#ifndef _GENWAVEX_H_
#define _GENWAVEX_H_

/// Incremental calculation of sum of waves. Slower than precalculating
/// a sum of sinusoids, but bandwidth limited and also useful for doubling, 
/// chorus effects, etc.
class GenWaveSum : public GenWaveWT
{
private:
	/// Structure to hold information for one partial.
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

	/// Initialize from an array of values.
	/// The array contains {Fo, WT, G, N, (part,amp)*n}
	/// where Fo is the oscillator frequency,
	/// WT the wavetable index,
	/// G the gibbs correction flag,
	/// N the number of partials,
	/// followed by N sets of partial number and amplitude pairs.
	/// @param n number of values
	/// @param v array of values
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

	/// Initialize partials.
	/// @param n number of partials
	/// @param m partial number
	/// @param a relative amplitude values
	/// @param g apply correction for gibbs effect
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

	/// @copydoc GenWave::Reset
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

	/// Allocate space for partials.
	/// This is called automatically from Init and InitParts.
	/// If partials are set individually using SetPart this
	/// must be called first.
	/// @param n number of partials
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

	/// Set a partial.
	/// @param n partial number index (0 based)
	/// @param mul partial number (1 based)
	/// @param amp amplitude
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

	/// Calculate amplitude and phase increments.
	/// This is called during a Reset to calculate
	/// the phase increment for each partial and
	/// a scaling factor for normalization. Partials
	/// that exceed the Nyquist limit are eliminated.
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

	/// @copydoc GenWave::Gen
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


/// FM (PM) Generator.
/// Any WT class has a modulator input. This special class has the
/// modulator oscillator built in for convienence.
/// The Modulate() method also works, and can be used for LFO.
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

	/// Initialize the oscillator.
	/// The values are {Fc, WT, M, I} with
	/// Fc the carrier frequency, WT the wavetable,
	/// M the modulation multiple (c:m) and I
	/// the index of modulation.
	/// @param n number of values
	/// @param v array of values
	virtual void Init(int n, float *v)
	{
		if (n > 3)
			InitFM(FrqValue(v[0]), FrqValue(v[2]), AmpValue(v[3]), (int) v[1]);
	}

	/// Initialize the oscillator from arguments.
	/// @param frequency carrier oscillator frequency
	/// @param mult modulator oscillator frequency as a c:m value
	/// @param mi index of modulation (I=dF/Fm)
	/// @param wtIndex wavetable index
	virtual void InitFM(FrqValue frequency, FrqValue mult, AmpValue mi, int wtIndex)
	{
		indexOfMod = PhsAccum(mi);
		modMult = mult;
		GenWaveWT::InitWT(frequency, wtIndex);
	}

	/// Calculate the modulator amplitude from index of modulation.
	/// This is invoked internally whenever the index of modulation
	/// is changed and does not normally need to be called directly.
	inline void CalcModAmp()
	{
		//modAmp = synthParams.frqTI * indexOfMod * frq * modMult;
		modAmp = indexOfMod * modIncr;
	}

	/// Set the index of modulation
	void SetModIndex(AmpValue iom)
	{
		indexOfMod = (PhsAccum)iom;
		CalcModAmp();
	}

	/// Get the index of modulation
	AmpValue GetModIndex()
	{ 
		return indexOfMod; 
	}

	/// Set the modulation frequency multiplier (c:m)
	void SetModMultiple(FrqValue m)
	{
		modMult = m;
	}

	/// Get the modulation frequency multiplier
	FrqValue GetModMultiple()
	{
		return modMult;
	}

	/// @copydoc GenWave::Reset
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

	/// @copydoc GenWave::Gen
	virtual AmpValue Gen()
	{
		while (index >= synthParams.ftableLength)
			index -= synthParams.ftableLength;
		while (index < 0)
			index += synthParams.ftableLength;
		while (modIndex >= synthParams.ftableLength)
			modIndex -= synthParams.ftableLength;

		AmpValue valCar = waveTable[(int)(index+0.5)];
		AmpValue valMod = waveTable[(int)(modIndex+0.5)];
		index += indexIncr + (PhsAccum(valMod) * modAmp);
		modIndex += modIncr;
		
		return valCar;
	}
};

/// Amplitude modulation (AM) Generator (2-quadrant multiply)
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

	/// Initialize the oscillator.
	/// The array of values contains {Fc, WT, Fm, Am}
	/// where Fc is the carrier frequency, WT the wavetable, 
	/// Fm is the modulator frequency and Am is the modulator
	/// amplitude.
	/// @param n number of values (4)
	/// @param v array of values
	virtual void Init(int n, float *v)
	{
		if (n > 3)
			InitAM(FrqValue(v[0]), FrqValue(v[2]), FrqValue(v[3]), (int) v[1]);
	}

	/// Initialize the oscillator from arguments.
	/// @param frequency carrier frequency
	/// @param mfrq modulator frequency
	/// @param mamp modulator amplitude.
	/// @param wtIndex the wavetable index
	virtual void InitAM(FrqValue frequency, FrqValue mfrq, AmpValue mamp, int wtIndex)
	{
		modAmp = mamp;
		modFrq = mfrq;
		modScale = 1.0 / (1.0 + modAmp);
		GenWaveWT::InitWT(frequency, wtIndex);
	}

	/// @copydoc GenWave::Reset
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

	/// @copydoc GenWave::Gen
	virtual AmpValue Gen()
	{
		if (index >= synthParams.ftableLength)
		{
			do
				index -= synthParams.ftableLength;
			while (index >= synthParams.ftableLength);
		}
		if (index < 0)
		{
			do
				index += synthParams.ftableLength;
			while (index < 0);
		}
		while (modIndex >= synthParams.ftableLength)
			modIndex -= synthParams.ftableLength;

		AmpValue valMod = 1.0 + (modAmp * waveTable[(int)(modIndex+0.5)]);
		AmpValue v = waveTable[(int)(index+0.5)] * valMod * modScale;
		index += indexIncr;
		modIndex += modIncr;
		return v;
	}
};

/// Ring modulation (RM) generator (i.e. 4-quadrant multiply)
class GenWaveRM : public GenWaveAM
{
public:
	/// @copydoc GenWave::Gen
	virtual AmpValue Gen()
	{
		if (index >= synthParams.ftableLength)
		{
			do
				index -= synthParams.ftableLength;
			while (index >= synthParams.ftableLength);
		}
		if (index < 0)
		{
			do
				index += synthParams.ftableLength;
			while (index < 0);
		}
		while (modIndex >= synthParams.ftableLength)
			modIndex -= synthParams.ftableLength;

		AmpValue v = waveTable[(int)(index+0.5)] * modAmp * waveTable[(int)(modIndex+0.5)];
		index += indexIncr;
		modIndex += modIncr;
		return v;
	}
};

///////////////////////////////////////////////////////////
/// Pitched noise. Ring modulation of a sine wave and noise
/// See: Computer Music, Dodge&Jerse, chapter 4.11b
/// @sa GenNoise
///////////////////////////////////////////////////////////
class GenWaveNZ : public GenUnit
{
private:
	GenWaveWT osc;
	GenNoiseI nz;

public:
	/// Initialize the oscillator.
	/// The array of values contains {Fo WT Fn}
	/// where Fo is the oscillator frequency,
	/// WT is the oscillator wavetable,
	/// and Fn is the noise hold frequency.
	/// @param n number of values
	/// @param v array of values
	virtual void Init(int n, float *v)
	{
		if (n > 2)
			InitNZ(FrqValue(v[0]), FrqValue(v[2]), (int) v[1]);
	}

	/// Initialize the oscillator from arguments.
	/// @param frequency oscillator frequency
	/// @param nzfrq noise hold frequency
	/// @param wtIndex oscillator wavetable index
	virtual void InitNZ(FrqValue frequency, FrqValue nzfrq, int wtIndex)
	{
		osc.InitWT(frequency, wtIndex);
		nz.Init(1, &nzfrq);
	}

	/// @copydoc GenWave::Reset
	virtual void Reset(float initPhs = 0)
	{
		osc.Reset(initPhs);
		nz.Reset(initPhs);
	}

	/// @copydoc GenWave::Sample
	virtual AmpValue Sample(AmpValue in)
	{
		return Gen() * in;
	}

	/// @copydoc GenWave::Gen
	virtual AmpValue Gen()
	{
		return osc.Gen() * nz.Gen();
	}
};

///////////////////////////////////////////////////////////
/// @brief Pulse wave generator.
/// @details Buzz uses a closed form of the Fourier series:
/// @code
///         A       sin((2N+1)*PI*fo*t)
/// f(t) = ---- * ((---------------------) - 1)
///         2N        sin(PI*fo*t)
/// ============================================
///         A      (2N+1)*cos((2N+1)*PI*fo*t)
/// f(t) = ---- * ((--------------------------) - 1)
///         2N            cos(PI*fo*t)
/// @endcode
/// This produces a bandwidth limited pulse wave with peak amplitude A.
/// In this implementation, A is always 1.
/// The number of harmonics (N) is a settable parameter. 
/// A higher number of harmonics produces a narrower pulse.
/// When the denominator sin(x) is 0, then
/// we can't use the first form, but we can use the cos() form.
/// However, we know that when sin(x) = 0, then cos(x) = 1. 
/// In addition, sin(x) = 0 only when the phase is some multiple of PI.
/// Since phase of the numerator is an integer
/// multiple of the phase of the denominator, we know
/// the phase must be 0 or some multiple of PI also. Thus:
/// @code
/// f(t) = A/2N * (((2N+1)*cos(0)/cos(0)) - 1)
/// f(t) = A/2N * (((2N+1)*1/1) - 1)
/// f(t) = A/2N * 2N
/// f(t) = A = 1
/// @endcode
/// See: Computer Music, Dodge&Jerse, chapter 5.3a
/// and http://www.cs.sfu.ca/~tamaras/
/// When a wavetable is used, we need to use interpolation
/// to avoid clicks and pops caused by round-off errros.
/// @sa GenWave
///////////////////////////////////////////////////////////
class GenWaveBuzz : public GenWave
{
private:
	bsInt32 numHarm;
	PhsAccum index2;
	PhsAccum indexIncr2;
	AmpValue ampScale;
	PhsAccum num2p1;

	/// Calcuate the phase increment.
	/// @param f frequency
	void CalcIncr(FrqValue f)
	{
		num2p1 = PhsAccum((2 * numHarm) + 1);
		indexIncr = synthParams.frqTI * f * 0.5;
		indexIncr2 = indexIncr * num2p1;
	}

	/// Get sin(ndx) using interpolation of a table.
	AmpValue SinWT(PhsAccum ndx)
	{
		int intIndex = (int) ndx; // floor(ndx)
		PhsAccum fract = ndx - (PhsAccum) intIndex;
		AmpValue v1 = wtSet.wavSin[intIndex];
		AmpValue v2 = wtSet.wavSin[intIndex+1];
		return v1 + ((v2 - v1) * (AmpValue)fract);
	}

public:
	GenWaveBuzz()
	{
		numHarm = 1;
		ampScale = 0.5;
		index2 = 0.0;
		indexIncr2 = 0.0;
	}

	/// @copydoc GenWave::Init
	void Init(int n, float *v)
	{
		if (n > 1)
			InitBuzz(v[0], (int)v[1]);
	}

	/// Set the number of harmonics.
	/// The higher n, the narowwer the pulse.
	/// @param n number of harmonics (0 < n < (SR/2Fo))
	void SetHarmonics(bsInt32 n)
	{
		numHarm = n;
	}

	/// Initialize the Buzz generator
	/// @param f frequency
	/// @param n number of harmonics (0 = maximum)
	void InitBuzz(FrqValue f, bsInt32 n)
	{
		SetFrequency(f);
		SetHarmonics(n);
		Reset();
	}

	/// @copydoc GenWave::Reset
	void Reset(float initPhs = 0.0)
	{
		bsInt32 maxN = (bsInt32) floor(synthParams.sampleRate / (2.0 * frq));
		if (numHarm > maxN)
			numHarm = maxN;
		ampScale = 1.0 / AmpValue(2 * numHarm);
		CalcIncr(frq);
		if (initPhs == 0.0)
		{
			index = 0.0;
			index2 = 0.0;
		}
		else
			index2 = index * num2p1;
	}

	/// @copydoc GenWave::Modulate
	void Modulate(FrqValue d)
	{
		CalcIncr(frq+d);
	}

	/// @copydoc GenWave::Gen
	AmpValue Gen()
	{
		AmpValue out;
		AmpValue div;
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
		if (index2 >= synthParams.ftableLength)
		{
			do
				index2 -= synthParams.ftableLength;
			while (index2 >= synthParams.ftableLength);
		}
		else if (index2 < 0)
		{
			do
				index2 += synthParams.ftableLength;
			while (index2 < 0);
		}
		div = SinWT(index);
		if (div != 0)
			out = ampScale * ((SinWT(index2) / div) - 1.0);
		else
			out = 1.0;
		index += indexIncr;
		index2 += indexIncr2;
		return out;
	}
};

//@}
#endif

