///////////////////////////////////////////////////////////
// BasicSynth - SoundFont File
//
/// @file SFFile.h SoundFont(R) file loader.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////

#ifndef SFFILE_H
#define SFFILE_H

#include <SFDefs.h>
#include <SoundBank.h>

class SFFile
{
public:
	int npresets;
	sfPresetHeader *phdr;

	int npbags;
	sfBag *pbag;

	int npmods;
	sfModList *pmod;

	int npgens;
	sfGenList *pgen;

	int ninsts;
	sfInst *inst;

	int nibags;
	sfBag *ibag;

	int nimods;
	sfModList *imod;

	int nigens;
	sfGenList *igen;

	int nshdrs;
	sfSample *shdr;

	int preload;
	float atnScl;

	FileReadBuf file;
	SoundBank *sfbnk;
	short hdrVals[sfgEndOper];

	short *samples;
	bsUint32 sampleSize;
	bsUint32 sampleFileOffs1;
	bsUint32 sampleFileOffs2;

	void ReadString(long len, bsString *str);
	int ReadData(void **pdata, long cksz);

	int InfoChunk(long cksz);
	int SDTAChunk(long cksz);
	int PDTAChunk(long cksz);
	int ListChunk(sfChunk& chk);

	SBSample *CreateSample(sfSample *shdr, short id);

	int  AddHeaderGen(short gen);
	void ApplyModulator(SBZone *zone, sfModList *mp);
	void BuildInstrument(SBInstr *in, int n, int pbagNdx);
	void BuildPreset(int n);
	void BuildSoundBank();

	FrqValue SFEnvRate(short rt);
	AmpValue SFEnvLevel(short amt);
	AmpValue SFAttenuation(short amt);
	AmpValue SFPercent(short amt);
	FrqValue SFFrequency(short amt);
	FrqValue SFRelCents(short amt);
	//FrqValue SFTimeCents(short amt);
	//FrqValue SFKeyScaleTime(short key, short amt);
	char *CopyName(char *dst, char *src);
	void InitGenVals(short *genVals);

public:
	SFFile();
	~SFFile();

	static int IsSF2File(const char *fname);
	SoundBank *LoadSoundBank(const char *fname, int pre = 1, float scl = 0.375);
};

#endif
