///////////////////////////////////////////////////////////
// BasicSynth - SoundFont unit generators
//
/// @file SFGen.h SoundFont(R) unit generators
//
// These class derive from standard ugens but are initialized from SoundFont file data.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
/// @addtogroup grpSoundbank
//@{

#ifndef SFGEN_H
#define SFGEN_H

/// Oscillator that initializes directly from a SBZone.
class GenWaveSF : public GenWaveWTLoop
{
public:
	/// Init the wavetable oscillator.
	/// @param f frequency in Hz
	/// @param zone sample information.
	/// @param skipAttack when true, start at the loop section.
	void InitSF(FrqValue f, SBZone *zone, int skipAttack = 0)
	{
		if (zone && zone->sample)
		{
			InitWTLoop(f, zone->recFreq, zone->rate, zone->tableEnd, 
		           zone->loopStart, zone->loopEnd, 
		           zone->mode, zone->sample->sample);
			if (skipAttack && zone->mode == 1)
			{
				state = 1;
				Reset(zone->loopStart);
			}
		}
		else
		{
			InitWTLoop(f, f, synthParams.sampleRate, 0, 0, 0, 1, wtSet.GetWavetable(WT_SIN));
		}
	}
};

/// Envelope generator for sound founts.
/// A sound bank (SF2 or DLS) uses a six segment envelope --
/// delay, attack, hold, decay, sustain, release.
/// This EG differs from the typical BasicSynth EG in the following ways:
/// 1) Start, peak and end levels are normalized to 0, 1, 0.
/// 2) Attack, decay and release are constant-rate calculations.
/// 3) Attack level follows an exponential convex curve (n^2)
class EnvGenSF : public GenUnit
{
private:
	AmpValue curLevel;
	AmpValue susLevel;
	AmpValue atkIncr;
	AmpValue decIncr;
	AmpValue relIncr;
	bsInt32  delayCount;
	bsInt32  holdCount;
	int      segNdx;
public:
	EnvGenSF()
	{
		curLevel = 0;
		susLevel = 0;
		atkIncr = 1.0;
		decIncr = 1.0;
		relIncr = 1.0;
		delayCount = 0;
		holdCount = 0;
		segNdx = 0;
	}

	void Init(int n, float *v)
	{
		if (n >= 6)
		{
			SetDelay(v[0]);
			SetAttack(v[1]);
			SetHold(v[2]);
			SetDecay(v[3]);
			SetSustain(v[4]);
			SetRelease(v[5]);
			segNdx = 0;
		}
	}

	inline AmpValue GetCurLevel() { return curLevel; }
	inline void SetCurLevel(AmpValue n) { curLevel = n; }

	void InitEnv(SBEnv *eg, int key = 60, int vel = 127)
	{
		if (eg)
		{
			SetSustain(eg->sustain);
			FrqValue km = (FrqValue) (60 - key);
			SetDelay(SoundBank::EnvRate(eg->delay));
			SetAttack(SoundBank::EnvRate(eg->attack + (((AmpValue) vel / 127.0) * eg->velAttack)));
			SetHold(SoundBank::EnvRate(eg->hold + (km * eg->keyHold)));
			SetDecay(SoundBank::EnvRate(eg->decay + (km * eg->keyDecay)));
			SetRelease(SoundBank::EnvRate(eg->release));
		}
		else
		{
			delayCount = 0;
			atkIncr = 1.0;
			holdCount = 0;
			decIncr = 1.0;
			susLevel = 0.0;
			relIncr = 1.0;
		}
		curLevel = 0;
		segNdx = 0;
	}

	void SetDelay(FrqValue rt)
	{
		holdCount = (bsInt32) (synthParams.sampleRate * rt);
	}

	void SetAttack(FrqValue rt)
	{
		FrqValue count = (synthParams.sampleRate * rt);
		if (count > 0)
			atkIncr = 1.0 / count;
		else
			atkIncr = 1.0;
	}

	void SetHold(FrqValue rt)
	{
		holdCount = (bsInt32) (synthParams.sampleRate * rt);
	}

	void SetDecay(FrqValue rt)
	{
		FrqValue count = (synthParams.sampleRate * rt);
		if (count > 0)
			decIncr = 1.0 / count;
		else
			decIncr = 1.0;
	}

	void SetRelease(FrqValue rt)
	{
		FrqValue count = (synthParams.sampleRate * rt);
		if (count > 0)
			relIncr = 1.0 / count;
		else
			relIncr = 1.0;
	}

	void SetSustain(AmpValue a)
	{
		if (a > 1.0)
			susLevel = 1.0;
		else
			susLevel = a;
	}

	void Reset(float initPhs)
	{
		if (initPhs >= 0)
		{
			curLevel = 0;
			segNdx = 0;
		}
	}

	int IsFinished()
	{
		return segNdx > 5;
	}

	void Release()
	{
		if (segNdx < 5)
			segNdx = 5;
	}

	AmpValue Gen()
	{
		switch (segNdx)
		{
		case 0: // delay
			if (--delayCount > 0)
				return 0.0;
			segNdx++;
		case 1:
			curLevel += atkIncr;
			if (curLevel >= 1.0)
			{
				segNdx++;
				return curLevel = 1.0;
			}
			return curLevel;// * curLevel;
		case 2:
			if (--holdCount > 0)
				return 1.0;
			segNdx++;
		case 3:
			curLevel -= decIncr;
			if (curLevel > susLevel)
				return curLevel;// * curLevel;
			segNdx++;
			curLevel = susLevel;
		case 4:
			return susLevel;
		case 5:
			curLevel -= relIncr;
			if (curLevel > 0.00001)
				return curLevel;// * curLevel;
			segNdx++;
			curLevel = 0.0;
			break;
		//case 6:
		//default:
		//	return 0.0;
		}
		return 0.0;
	}
};
//@}
#endif
