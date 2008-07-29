///////////////////////////////////////////////////////////
// BasicSynth - Example 3 (Chapter 6)
//
// Program to calculate complex waveforms
// 1 - summation of first 8 partials
// 2 - sawtooth wave
// 2a - inverse saw wave
// 3 - triangle wave
// 4 - square wave
// 4a - 25% pulse wave
// 5 - frequency modulation
// 6 - phase modulation
// 7 - amplituded modulation
// 8 - ring modulation
// 9 - noise
//
// use: Example03 [duration [pitch [volume]]]
//
// Copyright 2008, Daniel R. Mitchell
///////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SynthDefs.h"
#include "WaveFile.h"
#include "EnvGen.h"

#define twoDivPI (2.0/PI)

int main(int argc, char *argv[])
{
	int pitch = 60;  // Middle C
	FrqValue duration = 1.0;
	AmpValue peakAmp = 1.0;
	AmpValue volume = 0.0;
	long n;

	if (argc > 1)
		duration = atof(argv[1]);
	if (argc > 2)
		pitch = atoi(argv[2]);
	if (argc > 3)
		peakAmp = atof(argv[3]);

	InitSynthesizer();
	FrqValue frequency = synthParams.GetFrequency(pitch);

	WaveFile wf;
	if (wf.OpenWaveFile("example03.wav", 1))
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}

	long numSounds = 9;

	EnvGen eg;
	eg.InitEG(0.707, duration, 0.1, 0.2);

	long totalSamples = (long) ((synthParams.sampleRate * duration) + 0.5);

	/////////////////////////////////////////////////
	// 1 - summation of sine waves
	/////////////////////////////////////////////////
#define NUMPARTS 8
	PhsAccum phase[NUMPARTS];
	AmpValue ampPartial[NUMPARTS];
	FrqValue partNums[NUMPARTS] = {1, 2, 3, 4, 5, 6, 7, 8}; // sawtooth wave
//	FrqValue partNums[NUMPARTS] = {1, 3, 5, 7, 9, 11, 13, 15}; // "Square" wave
	PhsAccum phsIncr[NUMPARTS];
	int partMax = NUMPARTS;

	// limit to 2-4 partials to produce fewer upper
	// harmonics, a more "mellow" sound
	//partMax = 4;

	phsIncr[0] = synthParams.frqRad * frequency;
	phase[0] = 0.0f;
	ampPartial[0] = 1.0f;

	// Bandwidth limiting code -
	int partCount = 1;
	while (partCount < partMax)
	{
		PhsAccum tmp = phsIncr[0] * partNums[partCount];
		if (tmp >= PI)
			break;
		phsIncr[partCount] = tmp;
		phase[partCount] = 0;
		ampPartial[partCount] = 1 / partNums[partCount];
		partCount++;
	}


	// scale determined "empirically" - 
	// we can just add the amplitudes of partials, but 
	// usually won't get maximum amp depending on phase
	// of harmonics.
	// choose the maximum absolute amp value for scale
	AmpValue posMax = 0.f;
	AmpValue scale = 1.43f;   // sawtooth, 8 parts
	//scale = 1.53; // sawtooth, 4 parts, no gibbs
	//scale = 0.9274f; // Square wave

	// Negating the amplitude of even numbered overtones
	// will invert a sawtooth into a ramp...
	for (n = 1; n < partMax; n += 2)
		ampPartial[n] = -ampPartial[n];

	// Increasing amplitudes of overtones creates narrower "pulse"
	//for (n = 1; n < partMax;  n++)
	//	ampPartial[n] *= 2.0f;
	//scale = 3.02f;

	// sigma calculation to correct for Gibbs phenonmenon
	AmpValue sigmaK = PI / (float) partCount;
	AmpValue sigmaN = 0;
	AmpValue ampN;

	eg.Reset();
	AmpValue value = 0;
	for (n = 0; n < totalSamples; n++)
	{
		value = 0;
		sigmaN = 0;
		for (int p = 0; p < partCount; p++)
		{
			ampN = ampPartial[p];
			if (p > 0)
				ampN *= sinf(sigmaN) / sigmaN;
			sigmaN += sigmaK;
			value = value + (sinf(phase[p]) * ampN);
			phase[p] = phase[p] + phsIncr[p];
			if (phase[p] >= twoPI)
				phase[p] = phase[p] - twoPI;
		}
		if (fabs(value) > posMax)
			posMax = value;
		wf.Output1(eg.Gen() * (value / scale));
	}
	//printf("max amp=%1.8f, %i\n", posMax, wf.GetOOR());

	/////////////////////////////////////////////////
	// 2 - sawtooth wave
	/////////////////////////////////////////////////
	AmpValue sawIncr = (2 * frequency) / synthParams.sampleRate;
	AmpValue sawValue = -1;
	eg.Reset();
	for (n = 0; n < totalSamples; n++)
	{
		wf.Output1(eg.Gen() * sawValue);
		if ((sawValue += sawIncr) >= 1)
			sawValue -= 2;
	}

	/////////////////////////////////////////////////
	// 2a - inverse sawtooth wave
	/////////////////////////////////////////////////
	sawValue = 1;
	eg.Reset();
	for (n = 0; n < totalSamples; n++)
	{
		wf.Output1(eg.Gen() * sawValue);
		if ((sawValue -= sawIncr) <= -1)
			sawValue += 2;
	}

	/////////////////////////////////////////////////
	// 3 - triangle wave
	/////////////////////////////////////////////////
	AmpValue triValue = 0;
	PhsAccum triPhase = 0;
	PhsAccum triIncr  = synthParams.frqRad * frequency;
	eg.Reset();

	for (n = 0; n < totalSamples; n++)
	{
		triValue = (AmpValue)(triPhase * twoDivPI);
		if (triValue < 0)
			triValue = 1.0f + triValue;
		else
			triValue = 1.0f - triValue;
		wf.Output1(eg.Gen() * triValue);
		if ((triPhase += triIncr) >= PI)
			triPhase -= twoPI;
	}

	triIncr = 4 * frequency / synthParams.sampleRate;
	triValue = 0;
	eg.Reset();
	for (n = 0; n < totalSamples; n++)
	{
		wf.Output1(eg.Gen() * triValue);
		if ((triValue += triIncr) > 1.0)
		{
			triValue -= triIncr;
			triIncr = -triIncr;
		}
		else if (triValue < -1.0)
		{
			triValue -= triIncr;
			triIncr = -triIncr;
		}
	}
	
	/////////////////////////////////////////////////
	// 4 - square waves
	/////////////////////////////////////////////////

	PhsAccum sqPhase;
	PhsAccum sqMidPoint;
	PhsAccum sqPeriod;
	
	// Calculate using radians
	PhsAccum sqPhsIncr;
	sqPhase = 0;
	sqPhsIncr = synthParams.frqRad * frequency;
	eg.Reset();
	for (n = 0; n < totalSamples; n++)
	{
		value = eg.Gen();
		if (sqPhase >= 0)
			wf.Output1(value);
		else
			wf.Output1(-value);
		if ((sqPhase += sqPhsIncr) >= PI)
			sqPhase -= twoPI;
	}

	// Calculate using time
	sqPhsIncr = 1.0 / synthParams.sampleRate;
	sqPeriod = 1.0 / frequency;
	sqMidPoint = sqPeriod / 2;
	sqPhase = 0;
	eg.Reset();
	for (n = 0; n < totalSamples; n++)
	{
		value = eg.Gen();
		if (sqPhase >= 0)
			wf.Output1(value);
		else
			wf.Output1(-value);
		if ((sqPhase += sqPhsIncr) >= sqMidPoint)
			sqPhase -= sqPeriod;
	}

	// Calculate using samples
	sqPeriod = synthParams.sampleRate / frequency;
	sqMidPoint = sqPeriod / 2;
	sqPhase = 0;
	//long sqPeriodi = (long) ((synthParams.sampleRate / frequency) + 0.5);
	//long sqMidPointi = sqPeriod / 2;
	//long sqPhasei = 0;
	eg.Reset();
	for (n = 0; n < totalSamples; n++)
	{
		value = eg.Gen();
		if (sqPhase >= 0)
			wf.Output1(value);
		else
			wf.Output1(-value);
		if (++sqPhase >= sqMidPoint)
			sqPhase -= sqPeriod;
	}

	/////////////////////////////////////////////////
	// 4a - 25% pulse wave
	/////////////////////////////////////////////////
	sqPhase = 0;
	sqMidPoint = sqPeriod / 4; // = (sqPeriod * dutyCycle) / 100
	eg.Reset();

	for (n = 0; n < totalSamples; n++)
	{
		value = eg.Gen();
		if (sqPhase >= 0)
			wf.Output1(value);
		else
			wf.Output1(-value);
		if (++sqPhase >= sqMidPoint)
			sqPhase -= sqPeriod;
	}

	/////////////////////////////////////////////////
	// 5 - Frequency Modulation (FM)
	/////////////////////////////////////////////////
	FrqValue modFrequency = frequency * 3;
	PhsAccum modIncr = synthParams.frqRad * modFrequency;
	PhsAccum modPhase = 0;
	PhsAccum carIncr = 0;
	PhsAccum carPhase = 0;
	AmpValue modAmp = 2 * modFrequency;
	AmpValue modValue = 0;

	eg.Reset();

	for (n = 0; n < totalSamples; n++)
	{
		wf.Output1(eg.Gen() * sinf(carPhase));
		modValue = modAmp * sinf(modPhase);
		carIncr = synthParams.frqRad * (frequency + modValue);
		carPhase = carPhase + carIncr;
		modPhase = modPhase + modIncr;
		if (carPhase >= twoPI)
			carPhase -= twoPI;
		else if (carPhase < 0)
			carPhase += twoPI;
		if (modPhase >= twoPI)
			modPhase -= twoPI;
	}

	/////////////////////////////////////////////////
	// 6 - Phase Modulation (PM)
	/////////////////////////////////////////////////
	carIncr = synthParams.frqRad * frequency;
	modAmp = synthParams.frqRad * (2 * modFrequency); // 2000.0f; // convert frequency to radians/sec
	eg.Reset();

	for (n = 0; n < totalSamples; n++)
	{
		wf.Output1(eg.Gen() * sinf(carPhase));
		modValue = modAmp * sinf(modPhase);
		carPhase = carPhase + carIncr + modValue;
		modPhase = modPhase + modIncr;
		if (carPhase >= twoPI)
			carPhase -= twoPI;
		else if (carPhase < 0)
			carPhase += twoPI;
		if (modPhase >= twoPI)
			modPhase -= twoPI;
	}

	/////////////////////////////////////////////////
	// 7 - Amplitude Modulation (AM)
	/////////////////////////////////////////////////
	modFrequency = frequency * 2.5;
	modIncr = synthParams.frqRad * modFrequency;
	modAmp = 1.0;
	carPhase = 0;
	modPhase = 0;
	AmpValue modScale = 1 / (1 + modAmp);
	eg.Reset();

	for (n = 0; n < totalSamples; n++)
	{
		modValue = 1.0 + (modAmp * sinf(modPhase));
		wf.Output1(eg.Gen() * (sinf(carPhase) * modValue) * modScale);
		carPhase += carIncr;
		if (carPhase >= twoPI)
			carPhase -= twoPI;
		modPhase += modIncr;
		if (modPhase >= twoPI)
			modPhase -= twoPI;
	}

	/////////////////////////////////////////////////
	// 8 - Ring Modulation (AM)
	/////////////////////////////////////////////////
	modAmp = 1.0;
	carPhase = 0;
	modPhase = 0;
	eg.Reset();

	for (n = 0; n < totalSamples; n++)
	{
		wf.Output1(eg.Gen() * sinf(carPhase) * modAmp * sinf(modPhase));
		carPhase += carIncr;
		if (carPhase >= twoPI)
			carPhase -= twoPI;
		modPhase += modIncr;
		if (modPhase >= twoPI)
			modPhase -= twoPI;
	}

	/////////////////////////////////////////////////
	// 9 - White noise
	/////////////////////////////////////////////////
	eg.Reset();

	for (n = 0; n < totalSamples; n++)
	{
		value = ((AmpValue) rand() - (RAND_MAX/2)) / (RAND_MAX/2);
		wf.Output1(eg.Gen() * value);
	}

	wf.CloseWaveFile();

	return 0;
}
