///////////////////////////////////////////////////////////
// BasicSynth - Example 2 (Chapter 6)
//
// Envelope Generators
// 1 - interpolate linear, log, exponential
// 2 - state machine
// 3 - interpolate multiple segments ADSR
// 4 - multiple segments, state machine
//
// This example has all code in-line. For the class-based versions, see Example02a
//
// use: Example02 [duration [pitch [volume]]]
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SynthDefs.h"
#include "WaveFile.h"

int main(int argc, char *argv[])
{
	int pitch = 48; // Middle C
	FrqValue duration = 1;
	AmpValue peakAmp = 1;

	if (argc > 1)
		duration = atof(argv[1]);
	if (argc > 2)
		pitch = atoi(argv[2]);
	if (argc > 3)
		peakAmp = atof(argv[3]);

	InitSynthesizer();
	FrqValue frequency = synthParams.GetFrequency(pitch);
	PhsAccum phaseIncr = synthParams.frqRad * frequency;
	PhsAccum phase = 0;

	long silence = (long) (synthParams.sampleRate * 0.1);
	long totalSamples = (long) ((synthParams.sampleRate * duration) + 0.5);

	long attackTime = (long) (0.2 * synthParams.sampleRate);
	long decayTime  = (long) (0.4 * synthParams.sampleRate);
	long sustainTime = totalSamples - (attackTime + decayTime);
	long decayStart = totalSamples - decayTime;
	AmpValue envInc = peakAmp / (float) attackTime;
	AmpValue volume = 0;

	long n;

	WaveFile wf;
	if (wf.OpenWaveFile("example02.wav", 1))
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}

	/////////////////////////////////////////////////
	// Method 1 - simple integration, linear
	/////////////////////////////////////////////////
	long sampleNumber = 0;
	for (n = 0; n < totalSamples; n++)
	{
		if (n < attackTime || n > decayStart)
			volume += envInc;
		else if (n == attackTime)
			volume = peakAmp;
		else if (n == decayStart)
			envInc = -volume / (float) decayTime;
		wf.Output1(volume * sinv(phase));
		if ((phase += phaseIncr) >= twoPI)
			phase -= twoPI;
	}

	for (n = 0; n < silence; n++)
		wf.Output1(0);

	/////////////////////////////////////////////////
	// Method 1 - exponential
	/////////////////////////////////////////////////
	phase = 0;
	volume = 0;
	float expMin = 0.2;
	float expMax = 1.0+expMin;
	float expNow = expMin;
	float expMul = pow(expMax/expMin, 1.0f / (float) attackTime);

	for (n = 0; n < totalSamples; n++)
	{
		if (n < attackTime || n > decayStart)
		{
			expNow *= expMul;
			volume = (expNow - expMin) * peakAmp;
		}
		else if (n == attackTime)
		{
			volume = peakAmp;
			expNow = expMax;
		}
		else if (n == decayStart)
		{
			expMul = pow(expMin/expMax, 1.0f / (float) decayTime);
		}
		wf.Output1(volume * sinv(phase));
		if ((phase += phaseIncr) >= twoPI)
			phase -= twoPI;
	}

	for (n = 0; n < silence; n++)
		wf.Output1(0.0);

	/////////////////////////////////////////////////
	// Method 1 - log
	/////////////////////////////////////////////////
	phase = 0;
	volume = 0;
	expNow = expMax;
	expMul = pow(expMin/expMax, 1.0f / (float) attackTime);

	for (n = 0; n < totalSamples; n++)
	{
		if (n < attackTime || n > decayStart)
		{
			expNow *= expMul;
			volume = (1.0f - (expNow - expMin)) * peakAmp;
		}
		else if (n == attackTime)
		{
			volume = peakAmp;
			expNow = expMin;
		}
		else if (n == decayStart)
			expMul = pow(expMax/expMin, 1.0f / (float) decayTime);
		wf.Output1(volume * sinv(phase));
		if ((phase += phaseIncr) >= twoPI)
			phase -= twoPI;
	}

	for (n = 0; n < silence; n++)
		wf.Output1(0);


	/////////////////////////////////////////////////
	// Method 2 - simple state machine
	/////////////////////////////////////////////////
	long envCount = attackTime;
	int  envState = 0;

	envInc = peakAmp / (float) attackTime;

	phase = 0;
	volume = 0;

	for (n = 0; n < totalSamples; n++)
	{
		switch (envState)
		{
		case 0:
			if (envCount > 0)
			{
				volume += envInc;
				envCount--;
			}
			else
			{
				volume = peakAmp;
				envCount = sustainTime;
				envState = 1;
			}
			break;
		case 1:
			if (envCount > 0)
				envCount--;
			else
			{
				envCount = decayTime;
				envInc = volume / (float) decayTime;
				envState = 2;
			}
			break;
		case 2:
			if (envCount > 0)
			{
				volume -= envInc;
				envCount--;
			}
			else
			{
				volume = 0;
				envState = 3;
			}
			break;
		case 3:
			break;
		}
		wf.Output1(volume * sinv(phase));
		if ((phase += phaseIncr) >= twoPI)
			phase -= twoPI;
	}

	for (n = 0; n < silence; n++)
		wf.Output1(0);

	/////////////////////////////////////////////////
	// Method 3 - multiple segments (ADSR)
	/////////////////////////////////////////////////
	float envPeak;
	float envStep;
	float envLevel[4];
	float envIncr[4];
	long  envTime[4];
	int maxEnvIndex = 4;
	int envIndex = -1;

	envLevel[0] = 1.0 * peakAmp;
	envLevel[1] = 0.7 * peakAmp;
	envLevel[2] = 0.7 * peakAmp;
	envLevel[3] = 0.0;
	envTime[0] = (long) (0.1 * synthParams.sampleRate);
	envTime[1] = (long) (0.2 * synthParams.sampleRate);
	envTime[2] = (long) (0.5 * synthParams.sampleRate);
	envTime[3] = (long) (0.2 * synthParams.sampleRate);
	
	// pre-calculate increments
	envIncr[0] = envLevel[0] / envTime[0];
	for (n = 1; n < maxEnvIndex; n++)
	{
		if (envTime[n] > 0)
			envIncr[n] = (envLevel[n] - envLevel[n-1]) / envTime[n];
		else
			envIncr[n] = (envLevel[n] - envLevel[n-1]);
	}

	phase = 0;
	volume = 0;
	envCount = 0;
	envPeak = 0;
	for (n = 0; n < totalSamples; n++)
	{
		if (--envCount <= 0)
		{
			volume = envPeak;
			if (++envIndex < maxEnvIndex)
			{
				envCount = envTime[envIndex];
				envStep = envIncr[envIndex];
				envPeak = envLevel[envIndex];
			}
			else
				envStep = 0;
		}
		else
		{
			volume += envStep;
		}
		wf.Output1(volume * sinv(phase));
		if ((phase += phaseIncr) >= twoPI)
			phase -= twoPI;
	}

	for (n = 0; n < silence; n++)
		wf.Output1(0);

	/////////////////////////////////////////////////
	// Method 4 - multiple segments state machine
	/////////////////////////////////////////////////
	float atkLevel[2];
	float decLevel[2];
	long atkTime[2];
	long decTime[2];
	long atkMaxIndex = 2;
	long decMaxIndex = 2;

	atkLevel[0] = 1.0 * peakAmp;
	atkLevel[1] = 0.7 * peakAmp;
	decLevel[0] = 0.2 * peakAmp;
	decLevel[1] = 0.0;
	atkTime[0] = (long) (0.1 * synthParams.sampleRate);
	atkTime[1] = (long) (0.2 * synthParams.sampleRate);
	decTime[0] = (long) (0.1 * synthParams.sampleRate);
	decTime[1] = (long) (0.2 * synthParams.sampleRate);
	
	sustainTime = totalSamples - (atkTime[0] + atkTime[1] + decTime[0] + decTime[1]);
	phase = 0;
	volume = 0;
	envCount = 0;
	envIndex = -1;
	envState = 0;
	envPeak = 0;

	for (n = 0; n < totalSamples; n++)
	{
		switch (envState)
		{
		case 0: // attack
			if (--envCount <= 0)
			{
				volume = envPeak;
				if (++envIndex < atkMaxIndex)
				{
					envPeak = atkLevel[envIndex];
					envCount = atkTime[envIndex];
					if (envCount < 1)
						envCount = 1;
					envStep = (envPeak - volume) / envCount;
				}
				else
				{
					envCount = sustainTime;
					envStep = 0.0;
					envState = 1;
				}
			}
			else
				volume += envStep;
			break;
		case 1: // sustain
			if (--envCount <= 0)
			{
				envIndex = -1;
				envState = 2;
			}
			break;
		case 2: // release
			if (--envCount <= 0)
			{
				volume = envPeak;
				if (++envIndex < decMaxIndex)
				{
					envPeak = decLevel[envIndex];
					envCount = decTime[envIndex];
					if (envCount < 1)
						envCount = 1;
					envStep = (envPeak - volume) / envCount;
				}
				else
				{
					envCount = 0;
					envStep = 0.0;
					volume = 0.0;
					envState = 3;
				}
			}
			else
				volume += envStep;
			break;
		case 3:
			break;
		}
		wf.Output1(volume * sinv(phase));
		if ((phase += phaseIncr) >= twoPI)
			phase -= twoPI;
	}

	for (n = 0; n < silence; n++)
		wf.Output1(0);

	wf.CloseWaveFile();

	int oor = wf.GetOOR();
	if (oor)
		printf("%d Samples out of range...\n", oor);

	return 0;
}
