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

/// GenWaveSF classes handle four distinct scenarios:
/// 1) A single sample with mono sample data
/// 2) A single sample with stereo sample data
/// 3) Two linked and phase-locked zones, each with separate sample
/// 3) Two unlinked zones that are not phase locked.
class GenWaveSF
{
public:
	virtual void InitSF(FrqValue f, SBZone *zone, int skipAttack) = 0;
	virtual void UpdateFrequency(FrqValue f) = 0;
	virtual void Tick(AmpValue& lft, AmpValue& rgt) = 0;
	virtual void Release() = 0;
	virtual int  IsFinished() = 0;
};

class GenWaveSF1 : public GenWaveSF
{
private:
	GenWaveWTLoop osc;

public:
	void InitSF(FrqValue f, SBZone *zone, int skipAttack = 0)
	{
		if (zone)
		{
			osc.InitWTLoop(f, zone->recFreq, zone->rate, zone->tableEnd, 
		           zone->loopStart, zone->loopEnd, 
		           zone->mode, zone->sample->sample);
			if (skipAttack)
				osc.Reset(zone->loopStart);
		}
		else
		{
			osc.InitWTLoop(f, f, synthParams.sampleRate, 0, 0, 0, 1, wtSet.GetWavetable(WT_SIN));
		}
	}

	/// Combination of SetFrequency(f) + Reset(-1)
	void UpdateFrequency(FrqValue f)
	{
		osc.UpdateFrequency(f);
	}

	void Tick(AmpValue& lft, AmpValue& rgt)
	{
		lft = osc.Gen() * 0.5;
		rgt = lft;
	}

	void Release()
	{
		osc.Release();
	}

	int IsFinished()
	{
		return osc.IsFinished();
	}
};

class GenWaveSF2  : public GenWaveSF
{
private:
	GenWaveWTLoop2 osc;

public:
	void InitSF(FrqValue f, SBZone *zone, int skipAttack = 0)
	{
		if (zone)
		{
			AmpValue *wtr;
			AmpValue *wtl;
			SBSample *samp = zone->sample;
			if (samp->channels == 2)
			{
				wtl = &samp->sample[zone->tableStart];
				wtr = &samp->linked[zone->tableStart];
			}
			else if (samp->linkSamp)
			{
				wtr = &samp->sample[zone->tableStart];
				samp = samp->linkSamp;
				wtl = &samp->sample[zone->tableStart];
				if (zone->chan == 1) // sample is the left channel
				{
					AmpValue *tmp = wtr;
					wtr = wtl;
					wtl = tmp;
				}
			}
			osc.InitWTLoop(f, zone->recFreq, zone->rate, zone->tableEnd, 
		           zone->loopStart, zone->loopEnd, zone->mode, wtl);
			osc.SetWavetable2(wtr);
			if (skipAttack)
				osc.Reset(zone->loopStart);
		}
		else
		{
			AmpValue *wt = wtSet.GetWavetable(WT_SIN);
			osc.InitWTLoop(f, f, synthParams.sampleRate, 0, 0, 0, 1, wt);
			osc.SetWavetable2(wt);
		}
	}

	/// Combination of SetFrequency(f) + Reset(-1)
	void UpdateFrequency(FrqValue f)
	{
		osc.UpdateFrequency(f);
	}

	void Tick(AmpValue& lft, AmpValue& rgt)
	{
		osc.Gen2(lft, rgt);
	}

	void Release()
	{
		osc.Release();
	}

	int IsFinished()
	{
		return osc.IsFinished();
	}
};


class GenWaveSF3 : public GenWaveSF
{
private:
	GenWaveWTLoop oscl;
	GenWaveWTLoop oscr;

public:
	void InitSF(FrqValue f, SBZone *zone, int skipAttack = 0)
	{
		if (zone)
		{
			SBZone *zone1;
			SBZone *zone2;
			if (zone->chan == 1)
			{
				zone2 = zone;
				zone1 = zone->linkZone;
				if (zone1 == 0)
					zone1 = zone2;
			}
			else
			{
				zone1 = zone;
				zone2 = zone->linkZone;
				if (zone2 == 0)
					zone2 = zone1;
			}
			oscr.InitWTLoop(f, zone1->recFreq, zone1->rate, zone1->tableEnd, 
		           zone1->loopStart, zone1->loopEnd, zone1->mode,
				   &zone1->sample->sample[zone1->tableStart]);
			oscl.InitWTLoop(f, zone2->recFreq, zone2->rate, zone2->tableEnd, 
		           zone2->loopStart, zone2->loopEnd, zone2->mode, 
				   &zone2->sample->sample[zone2->tableStart]);
			if (skipAttack)
			{
				oscl.Reset(zone1->loopStart);
				oscr.Reset(zone2->loopStart);
			}
		}
		else
		{
			AmpValue *wt = wtSet.GetWavetable(WT_SIN);
			oscl.InitWTLoop(f, f, synthParams.sampleRate, 0, 0, 0, 1, wt);
			oscr.InitWTLoop(f, f, synthParams.sampleRate, 0, 0, 0, 1, wt);
		}
	}

	/// Combination of SetFrequency(f) + Reset(-1)
	void UpdateFrequency(FrqValue f)
	{
		oscl.UpdateFrequency(f);
		oscr.UpdateFrequency(f);
	}

	void Tick(AmpValue& lft, AmpValue& rgt)
	{
		lft = oscl.Gen();
		rgt = oscr.Gen();
	}

	void Release()
	{
		oscl.Release();
		oscr.Release();
	}

	int IsFinished()
	{
		return oscl.IsFinished() && oscr.IsFinished();
	}

};

/// Envelop generator for sound founts.
/// This EG differs from the typical BasicSynth EG in the following ways:
/// 1. Start, peak and end levels are normalized to 0, 1, 0.
/// 2. Attack, decay and relase are constant-rate calculations
/// 3. Attack level follows an exponential convex curve (n^2)
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
#endif
