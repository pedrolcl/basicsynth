///////////////////////////////////////////////////////////////
//
// BasicSynth - GenWaveWT
//
/// @file GenWaveWT.h Various wavetable waveform generators
//
/// - GenWaveWT - basic wavetable generator, non-interpolating
/// - GenWaveI - wavetable generator with interpolation
/// - GenWave32 - wavetable generator w/ fast 32-bit fixed point index
/// - GenWave64 - wavetable generator w/ fast 64-bit fixed point index
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////////
/// @addtogroup grpOscil
//@{
#ifndef _GENWAVET_H_
#define _GENWAVET_H_

#include "WaveTable.h"
#include "GenWave.h"

/// Generic wavetable based generator (oscillator).
/// The wave tables are stored in the global \ref wtSet object
/// and referenced by index number.
class GenWaveWT : public GenWave
{
public:
	AmpValue *waveTable;
	int   wtIndex;

	GenWaveWT()
	{
		wtIndex = WT_SIN;
		waveTable = 0; //wtSet.GetWavetable(wtIndex);
	}

	/// @copydoc GenWave::Reset()
	virtual void Reset(float initPhs = 0)
	{
		if (waveTable == 0)
			waveTable = wtSet.GetWavetable(WT_SIN);

		indexIncr = (PhsAccum) frq * synthParams.frqTI;
		if (initPhs > 0)
			index = (initPhs / twoPI) * synthParams.ftableLength;
	}

	/// @copydoc GenWave::Modulate()
	virtual void Modulate(FrqValue d)
	{
		indexIncr = (PhsAccum)(frq + d) * synthParams.frqTI;
		while (indexIncr < 0)
			indexIncr += synthParams.ftableLength;
		PhsAccum maxInc = synthParams.ftableLength / 2; 
		if (indexIncr >= maxInc)
			indexIncr = maxInc;
	}

	/// @copydoc GenWave::PhaseMod()
	virtual void PhaseMod(PhsAccum phs)
	{
		//PhaseModWT(phs * synthParams.radTI);
		index += phs * synthParams.radTI;
	}

	/// Modulate phase for wavetable.
	/// Similar to PhaseMod(), but here phase offset is in fraction of table length.
	/// This can be used by instruments that know the oscil
	/// is a WT type and adjust the modulator amplitude
	/// accordingly, i.e. set the LFO/EG amp to the tableLength/2 range,
	/// and thus avoid the extra calculations on each sample.
	/// Phase overflow is checked in Gen().
	/// @param phs wavetable index delta
	virtual void PhaseModWT(PhsAccum phs)
	{
		index += phs;
	}

	/// Set the wavetable. The wavetable index is used to
	/// get one of the wavetables allocated in wtSet. If
	/// the index is invalid, the SIN wave table is used.
	/// @param wti wavetable index
	inline void SetWavetable(int wti)
	{
		waveTable = wtSet.GetWavetable(wtSet.FindWavetable(wtIndex = wti));
	}

	/// Get the wavetable index
	/// @return wavetable index
	inline int GetWavetable()
	{
		return wtIndex;
	}

	/// Initialize the oscillator.
	/// @param f frequency in Hz
	/// @param wti wavetable index
	void InitWT(FrqValue f, int wti)
	{
		SetWavetable(wti);
		SetFrequency(f);
		Reset();
	}

	/// Initialize the oscillator. The values are passed in the v array
	/// with v[0] = frequency and v[1] = wavetable index.
	/// @param n number of values (2)
	/// @param v array of values
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

	/// @copydoc GenWave::Gen()
	virtual AmpValue Gen()
	{
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
		// Note: it's OK to round-up index since tables have guard point.
		int n = (int) (index + 0.5);
		index += indexIncr;
		return waveTable[n];
	}
};

/// Wavetable oscillator with simple linear interpolation.
/// This significantly improves the quality of signals 
/// when shorter wave tables (< 4096) are used.
/// With longer wavetables, the benefit is a little less.
class GenWaveI : public GenWaveWT
{
public:
	/// @copydoc GenWave::Gen()
	virtual AmpValue Gen()
	{
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
		// fract = index - floor(index);
		int intIndex = (int) index;
		PhsAccum fract = index - (PhsAccum) intIndex;
		index += indexIncr;
		AmpValue v1 = waveTable[intIndex];
		AmpValue v2 = waveTable[intIndex+1];
		return v1 + ((v2 - v1) * (AmpValue)fract);
	}
};

/// Fast wavetable generator. This oscillator used a fixed point (Q16.16)
/// phase accumulator for fast operation at the expense of slightly
/// less accurate phase increment values. Overall, the signal quality
/// is very good and indistinguisable from floating point versions
/// in most cases.
/// @note The index range limits wavetable size to <= 16k entries
class GenWave32 : public GenWaveWT
{
private:
	bsInt32 i32Index;
	bsInt32 i32IndexIncr;
	bsInt32 i32IndexMask;

	inline void CalcPhase()
	{
		i32IndexIncr = (bsInt32) (indexIncr * 65536.0);
	}

public:
	GenWave32()
	{
		i32Index = 0;
		i32IndexIncr = 0;
		i32IndexMask = (synthParams.itableLength << 16) - 1;
	}

	/// @copydoc GenWave::Modulate()
	virtual void Modulate(FrqValue d)
	{
		GenWaveWT::Modulate(d);
		CalcPhase();
	}

	// N.B. - abs(phs) should NEVER be > tableLength/2
	/// @copydoc GenWaveWT::PhaseModWT
	virtual void PhaseModWT(PhsAccum phs)
	{
		if (phs >= synthParams.ftableLength)
		{
			do
				phs -= synthParams.ftableLength;
			while (phs >= synthParams.ftableLength);
		}
		else if (phs < 0)
		{
			do
				phs += synthParams.ftableLength;
			while (phs < 0);
		}
		i32Index += (bsInt32) (phs * 65536.0) & i32IndexMask;
	}

	/// @copydoc GenWave::Reset()
	virtual void Reset(float initPhs = 0)
	{
		GenWaveWT::Reset(initPhs);
		CalcPhase();
		if (initPhs >= 0)
			i32Index = (bsInt32) (index * 65536.0);
	}

	/// @copydoc GenWave::Gen()
	virtual AmpValue Gen()
	{
		AmpValue v = waveTable[(i32Index + 0x8000) >> 16];
		i32Index = (i32Index + i32IndexIncr) & i32IndexMask;
		return v; 
	}
};

/// Specialized wavetable oscillator for sample playback.
/// Sampled systems typically use a wavetable containing
/// multiple periods. In addition, the wavetable is divided
/// into multiple segments: attack, loop, and optional release.
/// This oscillator implements generic looped wavetable playback
/// with pitch shifting. Interpolation is linear between adjacent
/// samples.
///
/// The oscillator can operate in one of three states:
/// 1. Scanning the wavetable up to the loop (attack)
/// 2. Looping between loopStart and loopEnd (sustain)
/// 3. Playing the wavetable through to the end.
///
/// The loopMode variable determines the interpretation
/// of the wavetable. Mode 1 indicates no looping. Mode 2 indicates
/// a loop through decay. Mode 3 indicates play to end during decay.
///
/// This oscillator supports 2-channel capability so that stereo
/// recordings are preserved when desired. If both left
/// and right wavetables are set, the values for each channel
/// are determined individually. The sum of the two channels is
/// returned by the Gen() function. After Gen() is called, the
/// GetLeft() and GetRight() accessor functions can be used to
/// retrieve the separate left and right values.
///
/// Values can be set directly using the SetWavetable and Init
/// functions. However, this class is normally used as a base class
/// to a sample file specific class that provides initialization from
/// data contained in a sample file.
class GenWaveWTLoop : public GenUnit
{
protected:
	AmpValue *wavetable;
	PhsAccum phase;
	PhsAccum phsIncr;
	PhsAccum period;
	PhsAccum loopStart;
	PhsAccum loopEnd;
	PhsAccum loopLen;
	PhsAccum tableEnd;
	FrqValue rateRatio;
	FrqValue frq;
	FrqValue recFrq;
	FrqValue piMult;
	int state;
	int loopMode;

public:
	GenWaveWTLoop()
	{
		// To save a little cpu time we skip initialization.
		// This is useless until you call InitWT() anyway...
		/*
		wavetable = wtSet.GetWavetable(WT_SIN);
		tableEnd = synthParams.ftableLength;
		period = tableEnd;
		loopStart = 0;
		loopEnd = tableEnd;
		loopLen = tableEnd;
		phase = 0.0;
		phsIncr = 1.0;
		frq = 440.0;
		state = 1;
		loopMode = 0;
		rateRatio = 1.0;
		recFrq = synthParams.sampleRate / tableEnd;
		piMult = 1.0 / recFrq;
		*/
		tableEnd = 0.0;
		phase = 0.0;
	}

	void SetFrequency(FrqValue f)
	{
		frq = f;
	}

	void SetWavetable(AmpValue *wt)
	{
		wavetable = wt;
	}

	/// Initialize from an array of values.
	/// The wavetable must be set directly with SetWavetable().
	void Init(int n, float *values)
	{
		if (n >= 7)
		{
			InitWTLoop(values[0], values[1], values[2], 
				(bsInt32) values[3], (bsInt32) values[4], 
				(bsInt32) values[5], (bsInt16) values[6], 
				wavetable);
		}
	}

	void InitWTLoop(FrqValue fo, FrqValue fr, FrqValue sr, 
		bsInt32 te, bsInt32 ls, bsInt32 le, bsInt16 lm, AmpValue *wt)
	{
		frq = fo;
		recFrq = fr;
		rateRatio = (FrqValue) sr / synthParams.sampleRate;
		piMult = rateRatio / recFrq; // pre-calculate for Modulate code
		period = sr / recFrq;
		tableEnd = (PhsAccum) te;
		loopStart = (PhsAccum) ls;
		loopEnd = (PhsAccum) le;
		loopLen = loopEnd - loopStart;
		loopMode = lm;
		if (loopMode == 0)
			state = 2;
		else
			state = 1;
		wavetable = wt;
		phase = 0.0;
		Reset(0);
	}

	void Reset(float initPhs)
	{
		if (initPhs >= 0)
			phase = initPhs;
		//phsIncr = rateRatio * frq / recFrq;
		phsIncr = frq * piMult;
	}

	/// Release is called to stop any more looping.
	/// The user of this object must determine when
	/// the release begins and call this method in
	/// the case where the wavetable as a separate
	/// release segment.
	void Release()
	{
		if (loopMode == 3)
			state = 2;
	}

	void Modulate(FrqValue d)
	{
		phsIncr = (frq + d) * piMult;
	}
	
	AmpValue Gen()
	{
		if (phase < 0)
			phase += period;
		if (state == 0) // attack - segment prior to loopStart
		{
			if (phase >= loopStart)
				state = 1;
		}
		else if (state == 1) // looping from loopStart to loopEnd
		{
			if (phase >= loopEnd)
				phase -= loopLen;
			else if (phase < loopStart)
				phase += loopLen;
		}
		if (phase >= tableEnd)
			return 0;
		// else (state == 2) // playing through to the end (release or no loop)

		// no interpolation is faster...
		//int ii = (int)(phase+0.5);
		//phase += phsIncr;
		//return wavetable[ii];

		// ...but interpolation sounds a little better:
		int ii = (int) phase;
		PhsAccum fr = phase - (PhsAccum) ii;
		phase += phsIncr;
		AmpValue v1 = wavetable[ii];
		AmpValue v2 = wavetable[ii+1];
		return v1 + ((v2 - v1) * fr);
	}

	int IsFinished()
	{
		return phase >= tableEnd;
	}
};

//@}
#endif

