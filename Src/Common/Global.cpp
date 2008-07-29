//////////////////////////////////////////////////////////////////
// BasicSynth
//
// Global objects and functions for BasicSynth
//
// Copyright 2008, Daniel R. Mitchell
/////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <math.h>
#include <SynthDefs.h>
#include <WaveTable.h>

SynthConfig synthParams;
WaveTableSet wtSet;

int InitSynthesizer(bsInt32 sr, bsInt32 wtlen, bsInt32 wtusr)
{
	synthParams.Init(sr, wtlen);
	wtSet.Init(wtusr);
	return 0;
}
