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
	void InitSF(FrqValue f, SFZone *zone, int skipAttack = 0)
	{
		//InitWTLoop(f, zone->recFreq, zone->rate, zone->tableEnd, 
		//           zone->loopStart, zone->loopEnd, 
		//           zone->loopMode, zone->sample);
		frq = f;
		if (zone)
		{
			recFrq = zone->recFreq;
			period = (PhsAccum) zone->rate / recFrq;
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

#endif
