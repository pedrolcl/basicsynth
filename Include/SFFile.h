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
#include <SFSoundBank.h>

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

	int loadMods;

	FileReadBuf file;
	SFSoundBank *sfbnk;

	void ReadString(long len, bsString *str);
	int ReadData(void **pdata, long cksz);

	int InfoChunk(long cksz);
	int SDTAChunk(long cksz);
	int PDTAChunk(long cksz);
	int ListChunk(sfChunk& chk);

	void BuildInstrument(SFInstr *in, int n);
	void BuildPreset(int n);
	void BuildSoundBank();

public:
	SFFile();
	~SFFile();

	SFSoundBank *LoadSoundBank(const char *fname, int mods = 0);
};

extern void InitGenVals(short *genVals);

#endif

