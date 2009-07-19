///////////////////////////////////////////////////////////
// BasicSynth - SoundFont sound bank
//
/// @file SFSoundBank.h SoundFont(R) sound bank classes
//
// These classes are for the in-memory sound bank.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////

#ifndef SFSOUNDBANK_H
#define SFSOUNDBANK_H

extern void InitGenVals(short *genVals);

struct SFInfo
{
	bsInt16 wMajorFile;
	bsInt16 wMinorFile;
	bsInt16 wMajorVer;
	bsInt16 wMinorVer;
	bsString szSoundEngine;
	bsString szROM;
	bsString szName;
	bsString szDate;
	bsString szEng;
	bsString szProduct;
	bsString szCopyright;
	bsString szComment;
	bsString szTools;

	SFInfo()
	{
		wMajorFile = 2;
		wMinorFile = 0;
		wMajorVer = 2;
		wMinorVer = 0;
	}
};

class SFModInfo : public SynthList<SFModInfo>
{
public:
	short srcOp;
	short dstOp;
	short transOp;
	short value;
	short srcAmntOp;
};

class SFZone : public SynthList<SFZone>
{
public:
	bsString  name;      // zone name
	bsInt32   sampleLen; // total length of 'samples'
	AmpValue *sample;    // array of amp values
	AmpValue  pan;       // pan 
	FrqValue  rate;      // recording sample rate
	FrqValue  recFreq;   // recording frequency
	bsInt32   tableStart;// first playable sample
	bsInt32   tableEnd;  // last playable sample
	bsInt32   loopStart; // the phase where we start looping
	bsInt32   loopEnd;   // the phase where the loop ends (one sample past the last sample to loop)
	bsInt32   loopLen;   // loopEnd - loopStart
	bsInt32   recPeriod; // nominal waveform period in samples
	bsInt16   cents;     // detune amount (multiplier for frequency)
	bsInt16   keyNum;    // 0-127, the nominal frequency is 440 * pow(2, (keyNum - 69)/12)
	bsInt16   chan;      // 0 = right/mono, 1 = left;
	bsInt16   mode;      // 0 = no loop, 
	                     // 1 = loop continuously
	                     // 3 = loop during key, then play remainder
	bsInt16   lowKey;    // lowest pitch for this sample
	bsInt16   highKey;   // highest pitch for this sample
	bsInt16   lowVel;    // lowest MIDI note-on velocity
	bsInt16   highVel;   // highest MIDI note-on velocity
	bsInt16   instZone;  // if set, this is an instrument zone, otherwise the global zone.

	// The values above are extracted out for convienience.
	// The genVals array stores the original file values for reference.
	short genVals[sfgEndOper];

	SFZone()
	{
		sampleLen = 0;
		tableStart = 0;
		tableEnd = 0;
		sample = 0;
		loopStart = 0;
		loopEnd = 0;
		loopLen = 0;
		rate = 44100.0;
		recFreq = 440.0;
		keyNum = -1;
		cents = 0;
		chan = 0;
		mode = 0;
		lowKey = 0;
		highKey = 127;
		lowVel = 0;
		highVel = 127;
		instZone = 0;
		pan = 0.0;
		InitGenVals(genVals);
	}

	~SFZone()
	{
	}

	void SetGenVal(short op, short val)
	{
		if (op >= 0 && op < 60)
			genVals[op] = val;
	}

	short GetGenVal(short op, short val)
	{
		if (op >= 0 && op < 60)
			return genVals[op];
		return 0;
	}

};

class SFInstr : public SynthList<SFInstr>
{
public:
	SFZone zoneHead;
	SFZone zoneTail;
	SFModInfo modHead;
	SFModInfo modTail;
	bsString instrName;
	bsInt16  instrNdx;

	SFInstr()
	{
		instrNdx = -1;
		zoneHead.Insert(&zoneTail);
		modHead.Insert(&modTail);
	}

	~SFInstr()
	{
		SFZone *zone;
		while ((zone = zoneHead.next) != &zoneTail)
		{
			zone->Remove();
			delete zone;
		}
		SFModInfo *mod;
		while ((mod = modHead.next) != &modTail)
		{
			mod->Remove();
			delete mod;
		}
	}

	SFZone *AddZone()
	{
		SFZone *zone = new SFZone;
		zoneTail.InsertBefore(zone);
		return zone;
	}

	SFZone *GetZone(int key, int vel, int lr)
	{
		SFZone *zone;
		for (zone = zoneHead.next; zone != &zoneTail; zone = zone->next)
		{
			if (zone->instZone && zone->chan == lr
			 && key >= zone->lowKey && key <= zone->highKey
			 && vel >= zone->lowVel && vel <= zone->highVel)
				return zone;
		}
		return 0;
	}

	SFZone *EnumZones(SFZone *zone)
	{
		if (zone)
			zone = zone->next;
		else
			zone = zoneHead.next;
		if (zone == &zoneTail)
			return 0;
		return zone;
	}

	SFModInfo *GetModInfo(short src, short dst)
	{
		SFModInfo *mod;
		for (mod = modHead.next; mod != &modTail; mod = mod->next)
		{
			if (mod->srcOp == src && mod->dstOp == dst)
				return mod;
		}
		return 0;
	}

	SFModInfo *AddModInfo()
	{
		SFModInfo *mod = new SFModInfo;
		modTail.InsertBefore(mod);
		return mod;
	}

};

class SFPreset
{
public:
	int       instrCount;
	SFInstr **instrList;
	bsString presetName;
	bsInt16  presetBnk;
	bsInt16  presetNdx;
	short genVals[sfgEndOper];
	SFModInfo modHead;
	SFModInfo modTail;
	SFZone *zoneMap[2][128];

	SFPreset(bsInt16 bnk, bsInt16 ndx)
	{
		InitGenVals(genVals);
		modHead.Insert(&modTail);
		presetNdx = ndx;
		presetBnk = bnk;
		instrList = 0;
		instrCount = 0;
		memset(zoneMap, 0, sizeof(zoneMap));
	}

	~SFPreset()
	{
		for (int n = 0; n < instrCount; n++)
		{
			if (instrList[n])
				delete instrList[n];
		}
		delete instrList;

		SFModInfo *mod;
		while ((mod = modHead.next) != &modTail)
		{
			mod->Remove();
			delete mod;
		}
	}

	void AllocInstr(int n)
	{
		instrCount = n;
		instrList = new SFInstr*[n];
		while (--n >= 0)
			instrList[n] = 0;
	}

	SFInstr *AddInstr()
	{
		SFInstr *instr = new SFInstr;
		for (int inum = 0; inum < instrCount; inum++)
		{
			if (instrList[inum] == 0)
			{
				instrList[inum] = instr;
				instr->instrNdx = inum;
				break;
			}
		}
		return instr;
	}

	/// Get the sample for MIDI key 'k' on channel lr (0 = mono/right, 1 = left)
	/// Melodic instruments will have one instrument entry.
	/// Drum kits have multiple instrument entries.
	/// @param key MIDI key number (0-127)
	/// @param vel Note-on velocity (0-127)
	/// @param lr right or left channel
	SFZone *GetSample(int key, int vel, int lr)
	{
		// check the zone map first (mask to keep in range without conditionals)
		SFZone *zone = zoneMap[lr & 1][key & 0x7f];
		if (zone && vel >= zone->lowVel && vel <= zone->highVel)
			return zone;

		// do an complete search of all instruments & zones
		SFInstr *instr;
		for (int inum = 0; inum < instrCount; inum++)
		{
			if ((instr = instrList[inum]) != 0)
			{
				if ((zone = instr->GetZone(key, vel, lr)) != 0)
					return zone;
			}
		}
		return 0;
	}

	SFInstr *GetInstr(int inum)
	{
		if (inum < instrCount)
			return instrList[inum];
		return 0;
	}

	SFInstr *GetInstr(const char *name)
	{
		SFInstr *instr;
		for (int inum = 0; inum < instrCount; inum++)
		{
			if ((instr = instrList[inum]) != 0)
			{
				if (instr->instrName.Compare(name) == 0)
					return instr;
			}
		}
		return 0;
	}

	void SetGenVal(short op, short val)
	{
		if (op >= 0 && op < sfgEndOper)
			genVals[op] = val;
	}

	short GetGenVal(short op, short val)
	{
		if (op >= 0 && op < sfgEndOper)
			return genVals[op];
		return 0;
	}

	SFModInfo *GetModInfo(short src, short dst)
	{
		SFModInfo *mod;
		for (mod = modHead.next; mod != &modTail; mod = mod->next)
		{
			if (mod->srcOp == src && mod->dstOp == dst)
				return mod;
		}
		return 0;
	}

	SFModInfo *AddModInfo()
	{
		SFModInfo *mod = new SFModInfo;
		modTail.InsertBefore(mod);
		return mod;
	}

	int InitZoneMap()
	{
		int k;
		for (k = 0; k < 128; k++)
		{
			zoneMap[0][k] = 0;
			zoneMap[1][k] = 0;
		}

		SFInstr *instr;
		for (int inum = 0; inum < instrCount; inum++)
		{
			if ((instr = instrList[inum]) != 0)
			{
				SFZone *zone = 0;
				while ((zone = instr->EnumZones(zone)) != 0)
				{
					if (zone->instZone)
					{
						int lr = zone->chan;
						for (k = zone->lowKey; k <= zone->highKey; k++)
							zoneMap[lr][k] = zone;
					}
				}
			}
		}
		return 0;
	}
};

class SFSoundBank : public SynthList<SFSoundBank>
{
public:
	bsString file;
	bsString name;
	SFInfo info;
	SFPreset **presetBank[129];
	AmpValue *samples;
	bsInt32  sampleSize;

	static SFSoundBank SoundBankList;
	static void DeleteBankList();
	static SFSoundBank *FindBank(const char *name);

	SFSoundBank()
	{
		samples = 0;
		sampleSize = 0;
		memset(&presetBank[0], 0, sizeof(presetBank));
		samples = 0;
		sampleSize = 0;
	}

	~SFSoundBank()
	{
		for (int b = 0; b < 129; b++)
		{
			SFPreset **presetList = presetBank[b];
			if (presetList != 0)
			{
				for (int n = 0; n < 128; n++)
					if (presetList[n] != 0)
						delete presetList[n];
				delete presetList;
				presetBank[b] = 0;
			}
		}
		delete samples;
	}

	SFPreset *AddPreset(bsInt16 bank, bsInt16 ndx)
	{
		if (bank < 0 || bank > 128 || ndx < 0 || ndx > 127)
			return 0;

		SFPreset **presetList = presetBank[bank];
		if (presetList == 0)
		{
			presetList = new SFPreset*[128];
			presetBank[bank] = presetList;
			memset(presetList, 0, sizeof(SFPreset*)*128);
		}
		if (presetList[ndx] == 0)
			presetList[ndx] = new SFPreset(bank, ndx);
		return presetList[ndx];
	}

	SFPreset *GetPreset(bsInt16 bank, bsInt16 ndx)
	{
		if (bank < 0 || bank > 128 || ndx < 0 || ndx > 127)
			return 0;
		SFPreset **presetList = presetBank[bank];
		if (presetList == 0)
			return 0;
		return presetList[ndx];
	}

};

extern FrqValue SFEnvRate(short rt);
extern AmpValue SFEnvLevel(short rt);
extern FrqValue SFAbsCents(short amt);
extern FrqValue SFRelCents(FrqValue f, short amt);

#endif
