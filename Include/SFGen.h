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

#ifndef SFGEN_H
#define SFGEN_H

class GenWaveSF : public GenWaveWTLoop
{
public:
	void InitSF(FrqValue f, SBZone *zone, int skipAttack = 0)
	{
		//InitWTLoop(f, zone->recFreq, zone->rate, zone->tableEnd, 
		//           zone->loopStart, zone->loopEnd, 
		//           zone->loopMode, zone->sample);
		frq = f;
		phase = 0;
		if (zone && zone->sample)
		{
			recFrq = zone->recFreq;
			//period = (PhsAccum) zone->rate / recFrq;
			period = (PhsAccum) zone->recPeriod;
			rateRatio = zone->rate / synthParams.sampleRate;
			piMult = rateRatio / recFrq;
			tableEnd = zone->tableEnd;
			loopStart = zone->loopStart;
			loopEnd = zone->loopEnd;
			loopLen = loopEnd - loopStart;
			wavetable = zone->sample;
			loopMode = zone->mode;
			if (loopMode == 0) // no looping
				state = 2;
			else
			{
				state = 0;
				if (skipAttack)
					phase = loopStart;
			}
		}
		else
		{
			wavetable = wtSet.wavSin;
			period = synthParams.ftableLength;
			loopMode = 0;
			state = 2;
			tableEnd = 0;
		}
		phsIncr = f * piMult;
	}

	/// Combination of SetFrequency(f) + Reset(-1)
	inline void UpdateFrequency(FrqValue f)
	{
		phsIncr = f * piMult;
	}

	void Copy(GenWaveSF *o)
	{
		frq = o->frq;
		recFrq = o->recFrq;
		period = o->period;
		rateRatio = o->rateRatio;
		piMult = o->piMult;
		tableEnd = o->tableEnd;
		loopStart = o->loopStart;
		loopEnd = o->loopEnd;
		loopLen = o->loopLen;
		wavetable = o->wavetable;
		loopMode = o->loopMode;
		state = o->state;
		phase = o->phase;
		phsIncr = o->phsIncr;
	}
};

/// Envelop generator for sound founts.
/// This EG differs from the typical BasicSynth EG in the following ways:
/// 1. Start, peak and end levels are fixed at 0, 1, 0 respectively
/// 2. Attack, decay and relase are constant-rate calculations
/// 3. Levels values follow an exponential convex curve (n^2)
/// 4. Initialization is from values in a SBZone object.
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

	void InitEnv(SBInstr *in, SBEnv *eg, int key = 60, int vel = 127)
	{
		if (eg)
		{
			FrqValue km = (FrqValue) (60 - key);
			SetDelay(eg->delay);
			SetAttack(eg->attack * in->velAtkRate[vel]);
			SetHold(eg->hold * in->keyHoldRate[key]);
			SetDecay(eg->decay * in->keyDecRate[key]);
			SetRelease(eg->release);
			SetSustain(eg->sustain);
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
		if (a >= 1.0)
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
			if (delayCount > 1)
			{
				delayCount--;
				return 0.0;
			}
			segNdx++;
		case 1:
			curLevel += atkIncr;
			if (curLevel < 1.0)
				return curLevel * curLevel;
			segNdx++;
			curLevel = 1.0;
		case 2:
			if (holdCount > 1)
			{
				holdCount--;
				return 1.0;
			}
			segNdx++;
		case 3:
			curLevel -= decIncr;
			if (curLevel > susLevel)
				return curLevel * curLevel;
			segNdx++;
			curLevel = susLevel;
		case 4:
			return susLevel;
		case 5:
			curLevel -= relIncr;
			if (curLevel > 0)
				return curLevel * curLevel;
			segNdx++;
			curLevel = 0;
		case 6:
			return 0.0;
		}
		return 0.0;
	}
};
#endif
