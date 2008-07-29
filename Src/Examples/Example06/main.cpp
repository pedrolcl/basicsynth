/////////////////////////////////////////////////////////////////////////
// BasicSynth - Example 6 (Chapter 9)
//
// Mixing and Panning
//
// use: Example06
//
// Copyright 2008, Daniel R. Mitchell
/////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SynthDefs.h"
#include "WaveFile.h"
#include "EnvGen.h"
#include "GenWaveWT.h"
#include "Mixer.h"

int main(int argc, char *argv[])
{
	InitSynthesizer();

	GenWave32 wv1;
	GenWave32 wv2;
	EnvGen eg1;
	EnvGen eg2;
	WaveFile wvf;
	Mixer theMix;

	if (wvf.OpenWaveFile("example06.wav", 2))
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}
	theMix.SetChannels(2);
	theMix.MasterVolume(1.0, 1.0);
	theMix.ChannelOn(0, true);
	theMix.ChannelVolume(0, 0.5);
	AmpValue lftOut;
	AmpValue rgtOut;

	AmpValue panset[] = { 1.0, 0.5, 0.0, -0.5, -1.0 };
	AmpValue panset2[] = { 45.0, 22.5, 0.0, -22.5, -45.0 };
	AmpValue panset3[] = { 1.0, 0.7, 0.0, -0.7, -1.0 };
	FrqValue frequency[5];
	long numSounds = 5;

	// make a little scale...
	frequency[0] = synthParams.GetFrequency(60);
	frequency[1] = synthParams.GetFrequency(62);
	frequency[2] = synthParams.GetFrequency(64);
	frequency[3] = synthParams.GetFrequency(65);
	frequency[4] = synthParams.GetFrequency(67);

	AmpValue volume;
	AmpValue value;

	FrqValue duration = 0.5;
	long totalSamples = (long) (duration * synthParams.sampleRate);
	long snd, n;

	eg1.InitEG(1.0, duration, 0.1, 0.2);
	eg2.InitEG(1.0, duration, 0.2, 0.1);

	// linear
	for (snd = 0; snd < numSounds; snd++)
	{
		wv1.InitWT(frequency[snd], WT_SAW);
		eg1.Reset();

		theMix.ChannelPan(0, panLin, panset[snd]);

		for (n = 0; n < totalSamples; n++)
		{
			volume = eg1.Gen();
			value = wv1.Gen();
			theMix.ChannelIn(0, value * volume);
			theMix.Out(&lftOut, &rgtOut);
			wvf.Output2(lftOut, rgtOut);
		}
	}

	// log
	for (snd = 0; snd < numSounds; snd++)
	{
		wv1.InitWT(frequency[snd], WT_SAW);
		eg1.Reset();

		theMix.ChannelPan(0, panLog, panset2[snd]);

		for (n = 0; n < totalSamples; n++)
		{
			volume = eg1.Gen();
			value = wv1.Gen();
			theMix.ChannelIn(0, value * volume);
			theMix.Out(&lftOut, &rgtOut);
			wvf.Output2(lftOut, rgtOut);
		}
	}

	// attenuate
	for (snd = 0; snd < numSounds; snd++)
	{
		wv1.InitWT(frequency[snd], WT_SAW);
		eg1.Reset();

		theMix.ChannelPan(0, panAtn, panset3[snd]);

		for (n = 0; n < totalSamples; n++)
		{
			volume = eg1.Gen();
			value = wv1.Gen();
			theMix.ChannelIn(0, value * volume);
			theMix.Out(&lftOut, &rgtOut);
			wvf.Output2(lftOut, rgtOut);
		}
	}

	//////////////////////////////////////////

	theMix.ChannelOn(1, true);
	theMix.ChannelVolume(1, 0.5);

	for (snd = 0; snd < numSounds; snd++)
	{
		wv1.InitWT(frequency[snd], WT_SAW);
		wv2.InitWT(frequency[snd]/2, WT_SAW);
		eg1.Reset();
		eg2.Reset();

		theMix.ChannelPan(0, panLin, panset[snd]);
		theMix.ChannelPan(1, panLin, panset[numSounds - snd - 1]);

		for (n = 0; n < totalSamples; n++)
		{
			volume = eg1.Gen();
			value = wv1.Gen();
			theMix.ChannelIn(0, value * volume);

			volume = eg2.Gen();
			value = wv2.Gen();
			theMix.ChannelIn(1, value * volume);
			theMix.Out(&lftOut, &rgtOut);
			wvf.Output2(lftOut, rgtOut);
		}
	}

	wv1.InitWT(frequency[0], WT_SAW);
	wv2.InitWT(1.0, WT_SIN);
	eg1.InitEG(1.0, 2.0, 0.8, 0.8);
	totalSamples = synthParams.isampleRate*2;
	theMix.ChannelPan(0, panLin, 0);

	for (n = 0; n < totalSamples; n++)
	{
		theMix.ChannelPan(0, panLin, wv2.Gen());
		theMix.ChannelIn(0, eg1.Gen() * wv1.Gen());
		theMix.Out(&lftOut, &rgtOut);
		wvf.Output2(lftOut, rgtOut);
	}

	wvf.CloseWaveFile();

	return 0;
}
