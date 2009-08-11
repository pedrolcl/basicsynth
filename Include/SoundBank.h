///////////////////////////////////////////////////////////
// BasicSynth - SoundFont sound bank
//
/// @file SoundBank.h SoundFont(R) sound bank classes
//
// These classes are for the in-memory sound bank.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////

#ifndef SOUNDBANK_H
#define SOUNDBANK_H
#include <SynthFile.h>

/// @brief Soundbank file information
/// The strings are read from INFO chunks in the soundbank file.
/// Not all INFO possible chunk IDs are represented. This list
/// contains information useful to display to the user in a GUI
/// synthesizer.
struct SBInfo
{
	bsInt16 wMajorFile;
	bsInt16 wMinorFile;
	bsInt16 wMajorVer;
	bsInt16 wMinorVer;
	bsString szSoundEngine;
	bsString szName;
	bsString szDate;
	bsString szEng;
	bsString szProduct;
	bsString szCopyright;
	bsString szComment;
	bsString szTools;

	SBInfo()
	{
		wMajorFile = 0;
		wMinorFile = 0;
		wMajorVer = 0;
		wMinorVer = 0;
	}
};

class SBModInfo : public SynthList<SBModInfo>
{
public:
	short srcOp;
	short dstOp;
	short transOp;
	short srcAmntOp;
	bsInt32 value;
};

/// @brief SBSample contains a block of samples read from the file.
/// For SF2 files, there is only one block holding all
/// samples. However, we divide that one block into smaller blocks.
/// DLS files have multiple blocks, potentially one for each region.
/// The filepos member is included to allow for incremental loading of sample information.
/// (@sa SoundBank::GetSample())
class SBSample : public SynthList<SBSample>
{
public:
	AmpValue *sample;    ///< mono sample
	bsUint32  filepos;   ///< file offset for samples
	bsUint32  filepos2;  ///< offset for LSB in SF2 24-bit format
	bsInt32   rate;      ///< recording sample rate
	bsInt32   sampleLen; ///< total length of 'samples'
	bsInt32   index;     ///< index/id number
	bsInt16   format;    ///< 0 = 8-bit, 1=16-bit, 2=IEEE float, 3=SF2 24-bit
	bsInt16   channels;  ///< 1 = mono, 2 = stero (others not supported)

	SBSample(int n = 0)
	{
		index = n;
		sample = 0;
		sampleLen = 0;
		rate = synthParams.sampleRate;
		filepos = 0;
		filepos2 = 0;
		format = -1;
		channels = 0;
	}

	~SBSample()
	{
		delete sample;
	}
};

/// @brief Envelope definition for a soundbank instrument.
/// This is a 6-segment envelope: delay, attack, hold,
/// decay, sustain, release. 
class SBEnv
{
public:
	FrqValue  delay;
	FrqValue  attack;
	FrqValue  hold;
	FrqValue  decay;
	FrqValue  release;
	AmpValue  peak;
	AmpValue  sustain;

	SBEnv()
	{
		delay = 0;
		attack = 0;
		hold = 0;
		decay = 0;
		release = 0;
		peak = 1.0;
		sustain = 0;
	}
};

/// @brief SBZone represents a wavetable for a range of keys.
/// The zone includes a pointer to the wavetable, loop points,
/// frequency, and envelope information. The key and velocity
/// range values are used to associate the zone with a range
/// of pitches. Instruments have from 1 to 128 zones. 
/// Melodic instruments typically have zones that cover a small
/// range of pitches per zone.
/// Percussion instruments typically have one zone per sound,
/// each mapped to one key number.
class SBZone : public SynthList<SBZone>
{
public:
	bsString  name;      ///< zone name
	bsInt32   sampleNdx; ///< index into sample list
	AmpValue *sample;    ///< array of amp values (the wavetable)
	bsUint32  sampleLen;
	AmpValue  pan;       ///< pan 
	FrqValue  rate;
	FrqValue  recFreq;   ///< recording frequency
	bsInt32   tableStart;///< first playable sample (usually 0)
	bsInt32   tableEnd;  ///< one past last playable sample
	bsInt32   loopStart; ///< the phase where we start looping
	bsInt32   loopEnd;   ///< the phase where the loop ends (one sample past the last sample to loop)
	bsInt32   loopLen;   ///< loopEnd - loopStart
	bsInt32   recPeriod; ///< nominal waveform period in samples
	bsInt16   keyNum;    ///< 0-127, the nominal frequency is 440 * pow(2, (keyNum - 69)/12)
	bsInt16   cents;     ///< detune amount (multiplier for frequency) frq *= pow(2, cents / 1200);
	bsInt16   chan;      ///< 0 = right/mono, 1 = left;
	bsInt16   mode;      ///< 0 = no loop, 
	                     ///< 1 = loop continuously
	                     ///< 3 = loop during key, then play remainder
	bsInt16   lowKey;    ///< lowest pitch for this sample
	bsInt16   highKey;   ///< highest pitch for this sample
	bsInt16   lowVel;    ///< lowest MIDI note-on velocity
	bsInt16   highVel;   ///< highest MIDI note-on velocity

	AmpValue  volAtten;  ///< Peak volume in dB of attenuation
	SBEnv     volEg;     ///< Volume envelope
	SBEnv     modEg;     ///< Modulator envelope

	AmpValue  vibAmount; ///< LFO 1 (vibrato) amount
	FrqValue  vibRate;   ///< LFO 1 (vibrato) rate (frequency)
	FrqValue  vibDelay;  ///< LFO 1 (vibrato) onset delay

	AmpValue  modAmount; ///< LFO 2 (modulation) amount
	FrqValue  modRate;   ///< LFO 2 (modulation) rate (frequency)
	FrqValue  modDelay;  ///< LFO 2 (modulation) onset delay

	SBModInfo modHead;
	SBModInfo modTail;

	SBZone()
	{
		Init();
		modHead.Insert(&modTail);
	}

	~SBZone()
	{
		SBModInfo *mod;
		while ((mod = modHead.next) != &modTail)
		{
			mod->Remove();
			delete mod;
		}
	}

	void Init()
	{
		sampleNdx = 0;
		tableStart = 0;
		tableEnd = 0;
		sample = 0;
		loopStart = 0;
		loopEnd = 0;
		loopLen = 0;
		recFreq = 440.0;
		keyNum = -1;
		cents = 0;
		chan = 0;
		mode = 0;
		lowKey = 0;
		highKey = 127;
		lowVel = 0;
		highVel = 127;
		pan = 0.0;
		volAtten = 1.0;

		vibAmount = 0;
		vibRate = 5.2;
		vibDelay = 0;

		modAmount = 0;
		modRate = 5.2;
		modDelay = 0;
	}

	SBModInfo *GetModInfo(short src, short dst)
	{
		SBModInfo *mod;
		for (mod = modHead.next; mod != &modTail; mod = mod->next)
		{
			if (mod->srcOp == src && mod->dstOp == dst)
				return mod;
		}
		return 0;
	}

	SBModInfo *AddModInfo()
	{
		SBModInfo *mod = new SBModInfo;
		modTail.InsertBefore(mod);
		return mod;
	}

	SBModInfo *EnumModInfo(SBModInfo *mi)
	{
		if (mi == 0)
			mi = &modHead;
		mi = mi->next;
		if (mi == &modTail)
			return 0;
		return mi;
	}

	/// @brief Check this zone for a match to key and velocity.
	inline int Match(int key, int vel)
	{
		return key >= lowKey && key <= highKey
			&& vel >= lowVel && vel <= highVel;
	}

	inline int MatchKey(int key)
	{
		return key >= lowKey && key <= highKey;
	}

	inline int MatchVel(int key)
	{
		return key >= lowVel && key <= highVel;
	}
};

class SBInstr : public SynthList<SBInstr>
{
public:
	SBZone zoneHead;
	SBZone zoneTail;
	SBModInfo modHead;
	SBModInfo modTail;
	bsString instrName;
	bsInt16  instrNdx;
	bsInt16  bank;
	bsInt16  prog;
	bsInt16  loaded;
	bsInt16  lowKey;
	bsInt16  highKey;
	SBZone *zoneMap[2][128];
	FrqValue keyDecRate[128];
	FrqValue keyHoldRate[128];
	FrqValue velAtkRate[128];
	FrqValue velVolume[128];

	SBInstr()
	{
		lowKey = 0;
		highKey = 127;
		instrNdx = -1;
		bank = 0;
		prog = 0;
		loaded = 0;
		zoneHead.Insert(&zoneTail);
		modHead.Insert(&modTail);
		memset(zoneMap, 0, sizeof(zoneMap));
		for (int n = 0; n < 128; n++)
		{
			keyDecRate[n] = 1.0;
			keyHoldRate[n] = 1.0;
			velAtkRate[n] = 1.0;
			velVolume[n] = synthParams.AttenCB(960.0 * (127 - (double) n) / 127.0);
		}
	}

	~SBInstr()
	{
		SBZone *zone;
		while ((zone = zoneHead.next) != &zoneTail)
		{
			zone->Remove();
			delete zone;
		}
		SBModInfo *mod;
		while ((mod = modHead.next) != &modTail)
		{
			mod->Remove();
			delete mod;
		}
	}

	SBZone *AddZone()
	{
		SBZone *zone = new SBZone;
		zoneTail.InsertBefore(zone);
		return zone;
	}

	int InitZoneMap()
	{
		int k;
		for (k = 0; k < 128; k++)
		{
			zoneMap[0][k] = 0;
			zoneMap[1][k] = 0;
		}

		SBZone *zone = 0;
		for (zone = zoneHead.next; zone != &zoneTail; zone = zone->next)
		{
			int lr = zone->chan & 1;
			for (k = zone->lowKey; k <= zone->highKey; k++)
			{
				if (zoneMap[lr][k] == 0)
					zoneMap[lr][k] = zone;
			}
		}

		return 0;
	}

	SBZone *GetZone(int key, int vel, int lr)
	{
		// check the zone map first (mask to keep in range without conditionals)
		SBZone *zone = zoneMap[lr & 1][key & 0x7f];
		if (!zone)
			return 0; // no zone for this key/channel

		if (zone->MatchVel(vel))
			return zone;

		// do a complete search of all zones to try and match key+velocity
		SBZone *zone2;
		for (zone2 = zoneHead.next; zone2 != &zoneTail; zone2 = zone2->next)
		{
			if (zone2->chan == lr && zone2->Match(key, vel))
				return zone2;
		}

		// No match? ignore velocity..
		return zone;
	}

	SBZone *EnumZones(SBZone *zone)
	{
		if (zone)
			zone = zone->next;
		else
			zone = zoneHead.next;
		if (zone == &zoneTail)
			return 0;
		return zone;
	}

	SBModInfo *GetModInfo(short src, short dst)
	{
		SBModInfo *mod;
		for (mod = modHead.next; mod != &modTail; mod = mod->next)
		{
			if (mod->srcOp == src && mod->dstOp == dst)
				return mod;
		}
		return 0;
	}

	SBModInfo *AddModInfo()
	{
		SBModInfo *mod = new SBModInfo;
		modTail.InsertBefore(mod);
		return mod;
	}

	SBModInfo *EnumModInfo(SBModInfo *mi)
	{
		if (mi == 0)
			mi = &modHead;
		mi = mi->next;
		if (mi == &modTail)
			return 0;
		return mi;
	}

};

//////////////////////////////////////////////////////////////
/// @brief SoundBank holds a collection of instruments.
/// A SoundBank is typically loaded from either a
/// SF2 file, DLS file, or from individual WAVE files.
/// It could be constructed directly in memory if
/// appropriate.
///
/// Instruments are represented by bank and program (patch)
/// numbers, with 128 patch settings per bank.
/// For MIDI, there are potentially 16k banks. The bank
/// number is constructed from CC# 0 (MSB) and CC# 32 (LSB) as:
/// @code
/// bank = (MSB << 7) | LSB
/// @endcode
/// A GM soundbank typically has two banks of importance,
/// bank 0 (melody instruments) and bank 128 (drum instruments).
/// This SoundBank class supports 129 banks. The entire
/// MIDI bank range could be handled with some type of
/// bank mapping list, but for now, if the LSB is zero,
/// use the MSB part as the bank number.
///
/// Soundbank files typically encode pitch, time and
/// volume with a fixed point value, scaled appropriately.
/// Pitch is in cents; time is in timecents; volume
/// is in centibels. This class includes static methods 
/// to convert the integer representation
/// into the appropriate floating point value.
///
/// All soundbank objects should be attached to the static
/// SoundBankList member and located using FindBank.
//////////////////////////////////////////////////////////////
class SoundBank : public SynthList<SoundBank>
{
public:
	bsString file;                 ///< file name (empty if constructed in memory)
	bsString name;                 ///< symbolic name
	SBInfo info;                   ///< INFO records (copyright, version, etc.)
	SBInstr **instrBank[129];      ///< array of instruments[bank][patch]
	SBSample *samples;             ///< list of sample blocks

	static SoundBank SoundBankList;
	static void DeleteBankList();
	static SoundBank *FindBank(const char *name);

	static FrqValue PitchCents(FrqValue pc);
	static FrqValue EnvRate(FrqValue tc);
	static AmpValue Attenuation(AmpValue cb);

	SoundBank()
	{
		samples = 0;
		memset(&instrBank[0], 0, sizeof(instrBank));
	}

	~SoundBank()
	{
		for (int b = 0; b < 129; b++)
		{
			SBInstr **instrList = instrBank[b];
			if (instrList != 0)
			{
				for (int n = 0; n < 128; n++)
					if (instrList[n] != 0)
						delete instrList[n];
				delete instrList;
				instrBank[b] = 0;
			}
		}
		SBSample *samp;
		while ((samp = samples) != 0)
		{
			samples = samp->next;
			delete samp;
		}
	}

	/// @brief Add an instrument to the soundbank.
	/// @param bank bank number
	/// @param prog program (patch) number
	/// @return pointer to empty instrument definition
	SBInstr *AddInstr(bsInt16 bank, bsInt16 prog)
	{
		if (prog < 0 || prog > 127)
			return 0;
		if (bank > 128)
		{
			if ((bank & 0x7F) == 0)
				bank >>= 7;
			if (bank > 128)
				return 0;
		}

		SBInstr **instrList = instrBank[bank];
		if (instrList == 0)
		{
			instrList = new SBInstr*[128];
			instrBank[bank] = instrList;
			memset(instrList, 0, sizeof(SBInstr*)*128);
		}
		SBInstr *ip = instrList[prog];
		if (ip == 0)
		{
			ip = new SBInstr;
			ip->bank = bank;
			ip->prog = prog;
			instrList[prog] = ip;
		}
		return ip;
	}

	/// @brief Locate an instrument.
	/// @param bank bank number
	/// @param prog patch number
	/// @return pointer to instrument definition, or null
	SBInstr *GetInstr(bsInt16 bank, bsInt16 prog, int load = 1)
	{
		if (bank < 0 || bank > 128 || prog < 0 || prog > 127)
			return 0;
		SBInstr **instrList = instrBank[bank];
		if (instrList == 0)
			return 0;
		SBInstr *in = instrList[prog];
		if (in && load && !in->loaded)
			LoadInstr(in);
		return in;
	}

	/// @brief Add a sample block.
	/// @param ndx index (id) for this sample
	/// @return pointer to empty sample
	SBSample *AddSample(int ndx)
	{
		SBSample *samp = new SBSample(ndx);
		if (samples)
			samples->InsertBefore(samp);
		samples = samp;
		return samp;
	}

	/// @brief Locate a sample by index
	/// This function will attempt to dynamically
	/// load the sample data if the sample data is null.
	/// The sample object must already be initialized
	/// with the correct file offset, size and format.
	/// @param ndx index (id) of the sample
	/// @return pointer to sample
	SBSample *GetSample(int ndx, int load = 1)
	{
		SBSample *samp = samples;
		while (samp)
		{
			if (samp->index == ndx)
			{
				if (samp->sample == 0 && load)
					LoadSample(samp);
				return samp;
			}
			samp = samp->next;
		}
		return 0;
	}

	/// @brief Load sample data from the original file.
	/// If the sample cannot be loaded, a block of zeros is allocated.
	/// @param samp pointer to sample block object.
	/// @return 0 on success, non-zero on failure.
	/// @{
	int LoadSample(SBSample *samp);
	int LoadSample(SBSample *samp, FileReadBuf& f);
	int LoadInstr(SBInstr *instr);
	int LoadInstr(SBInstr *instr, FileReadBuf& f);
	int ReadSamples1(SBSample *samp, FileReadBuf& f);
	int ReadSamples2(SBSample *samp, FileReadBuf& f);
	/// @}
};


#endif
