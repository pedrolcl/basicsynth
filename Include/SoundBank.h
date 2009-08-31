///////////////////////////////////////////////////////////
// BasicSynth - SoundFont sound bank
//
/// @file SoundBank.h Internal sound bank, loaded from SF2 or DLS files.
//
// These classes are for the in-memory sound bank.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
/// @addtogroup grpSoundbank
//@{

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

// Flags indicating active generators
#define SBGEN_VIBLFOF 0x0001
#define SBGEN_MODENVF 0x0002
#define SBGEN_PITWHLF 0x0004
#define SBGEN_MODLFOF 0x0010
#define SBGEN_MODWHLF 0x0020
#define SBGEN_BRTHFRQ 0x0040
#define SBGEN_EXPRFRQ 0x0080
#define SBGEN_MODLFOA 0x0100
#define SBGEN_MODWHLA 0x0200
#define SBGEN_BRTHAMP 0x0400
#define SBGEN_EXPRAMP 0x0800
#define SBGEN_MODLIST 0x1000
#define SBGEN_MODLFOX (SBGEN_MODLFOF|SBGEN_MODLFOA)
#define SBGEN_MODWHLX (SBGEN_VIBLFOF|SBGEN_MODLFOF|SBGEN_MODLFOA)
#define SBGEN_MODFRQX (SBGEN_VIBLFOF|SBGEN_MODLFOF|SBGEN_MODENVF)
#define SBGEN_MODAMPX (SBGEN_MODLFOA|SBGEN_BRTHAMP|SBGEN_EXPRAMP)

/// Internal generator Source or Destination values
#define SBGEN_PITCH   0x0000
#define SBGEN_VOLUME  0x0001
#define SBGEN_VIBFRQ  0x0002
#define SBGEN_VIBDLY  0x0003
#define SBGEN_VIBAMNT 0x0004
#define SBGEN_MODFRQ  0x0005
#define SBGEN_MODDLY  0x0006
#define SBGEN_MODAMNT 0x0007
#define SBGEN_EG1DLY  0x0009
#define SBGEN_EG1ATK  0x000A
#define SBGEN_EG1HOLD 0x000B
#define SBGEN_EG1DEC  0x000C
#define SBGEN_EG1SUS  0x000D
#define SBGEN_EG1REL  0x000E
#define SBGEN_EG2DLY  0x000F
#define SBGEN_EG2ATK  0x0010
#define SBGEN_EG2HOLD 0x0011
#define SBGEN_EG2DEC  0x0012
#define SBGEN_EG2SUS  0x0013
#define SBGEN_EG2REL  0x0014
#define SBGEN_EG2AMNT 0x0015
#define SBGEN_IATTEN  0x0016
#define SBGEN_KEYHLD  0x0017
#define SBGEN_KEYDEC  0x0018
#define SBGEN_VELATK  0x0019
#define SBGEN_VELCTY  0x001A
#define SBGEN_KEYNUM  0x001B

class SBModInfo : public SynthList<SBModInfo>
{
public:
	short srcOp;
	short dstOp;
	short srcAmntOp;
	short transOp;
	float value;
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
	AmpValue *linked;    ///< second array for 2 channel
	SBSample *linkSamp;  ///< linked, phase-locked sample object
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
		linked = 0;
		linkSamp = 0;
		sampleLen = 0;
		rate = synthParams.isampleRate;
		filepos = 0;
		filepos2 = 0;
		format = -1;
		channels = 0;
	}

	~SBSample()
	{
		delete sample;
		delete linked;
	}
};

/// @brief Envelope definition for a soundbank instrument.
/// This is a 6-segment envelope: delay, attack, hold,
/// decay, sustain, release. Time values are in timecents.
class SBEnv
{
public:
	FrqValue  delay;        ///< delay time (timecents
	FrqValue  attack;       ///< attack rate (timecents)
	FrqValue  hold;         ///< hold time (timecents)
	FrqValue  decay;        ///< decay rate (timecents)
	FrqValue  release;      ///< release rate (timecents)
	AmpValue  sustain;      ///< sustain level (percent)
	FrqValue  keyDecay;     ///< Key # to decay rate scale (timecents)
	FrqValue  keyHold;      ///< Key # to hold rate scale (timecents)
	FrqValue  velAttack;    ///< Velocity to attack rate scale (timecents)

	SBEnv()
	{
		delay = -12000.0;
		attack = -12000.0;
		hold = -12000.0;
		decay = -12000.0;
		release = -12000.0;
		sustain = 0.0;
		keyHold = 0.0;
		keyDecay = 0.0;
		velAttack = 0.0;
	}
};

/// @brief LFO oscillator information.
class SBLfo
{
public:
	FrqValue  rate;   ///< LFO rate (frequency)
	FrqValue  delay;  ///< LFO onset delay

	SBLfo()
	{
		rate = 4.5;
		delay = 0;
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
	SBSample *sample;    ///< the wavetable
	SBZone   *linkZone;  ///< link to another zone (stereo sample)
	bsUint32  sampleLen;
	AmpValue  pan;       ///< pan 
	FrqValue  rate;      ///< sample rate
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
	bsUint32  exclNote;  ///< Exclusive note id

	bsUint32  genFlags;  ///< flags indicating active modulators
	AmpValue  volAtten;  ///< Peak volume in dB of attenuation
	AmpValue  velScale;  ///< Attenuation scaling from note-on velocity
	SBEnv     volEg;     ///< Volume envelope
	SBEnv     modEg;     ///< Modulator envelope
	SBLfo     vibLfo;    ///< LFO 1 (vibrato)
	SBLfo     modLfo;    ///< LFO 2 (modulation)

	FrqValue  vibLfoFrq; ///< Vib LFO amount applied to frequency (cents)
	AmpValue  modLfoVol; ///< Mod LFO amount appiled to volume (centibels)
	FrqValue  modLfoFrq; ///< Mod LFO amount applied to frequency (cents)
	FrqValue  modEnvFrq; ///< Mod ENV amount applied to frequency (cents)
	AmpValue  volEnvAmp; ///< Vol ENV amount applied to amplitude

	FrqValue  pbfScale;  ///< Pitchwheel amount
	AmpValue  mwaScale;  ///< Modwheel CC volume amount
	FrqValue  mwfScale;  ///< Modwheel CC pitch amount
	AmpValue  expaScale; ///< Expression CC volume amount
	FrqValue  expfScale; ///< Expression CC pitch amount
	AmpValue  bthaScale; ///< Breath CC volume amount
	FrqValue  bthfScale; ///< Breath CC pitch amount
	AmpValue  rvrbSend;  ///< % to send to reverb unit
	AmpValue  chorusSend; ///< % to send to chorus unit

	SynthEnumList<SBModInfo> modList;

	SBZone()
	{
		Init();
	}

	~SBZone()
	{
	}

	/// Initialize member variables.
	void Init()
	{
		genFlags = 0;
		sampleNdx = 0;
		tableStart = 0;
		tableEnd = 0;
		sample = 0;
		linkZone = 0;
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
		exclNote = 0;
		pan = 0.0;
		volAtten = 0.0;
		velScale = 1.0;

		vibLfoFrq = 0;
		modLfoVol = 0;
		modLfoFrq = 0;
		modEnvFrq = 0;
		volEnvAmp = 960.0;

		mwaScale = 0.0;
		mwfScale = 0.0;
		expaScale = 0.0;
		expfScale = 0.0;
		bthaScale = 0.0;
		bthfScale = 0.0;
		rvrbSend = 0.0;
		chorusSend = 0.0;
	}

	/// @brief Get modulator information
	/// Locate any modulator info that connects 
	/// the given source generator to the given
	/// destination generator.
	/// @param src Source generator
	/// @param dst Destination generator
	SBModInfo *GetModInfo(short src, short dst)
	{
		SBModInfo *mod = 0;
		while ((mod = modList.EnumItem(mod)) != 0)
		{
			if (mod->srcOp == src && mod->dstOp == dst)
				return mod;
		}
		return 0;
	}

	/// Add a modulator connection.
	inline SBModInfo *AddModInfo()
	{
		return modList.AddItem();
	}

	/// List all modulator connections.
	/// On the first call, set mi = 0.
	inline SBModInfo *EnumModInfo(SBModInfo *mi)
	{
		return modList.EnumItem(mi);
	}

	/// @brief Check this zone for a match to key and velocity.
	inline int Match(int key, int vel)
	{
		return key >= lowKey && key <= highKey
			&& vel >= lowVel && vel <= highVel;
	}

	/// @brief Check this zone for a match to only the key.
	inline int MatchKey(int key)
	{
		return key >= lowKey && key <= highKey;
	}

	/// @brief Check this zone for a match to only the velocity.
	inline int MatchVel(int vel)
	{
		return vel >= lowVel && vel <= highVel;
	}

	/// @brief Set the generator flags.
	/// Generator flags are an optimization for playback.
	/// This function checks to see if the generator
	/// will affect playback and sets bits in the genVals
	/// member appropriately.
	void SetGenFlags()
	{
		if (vibLfoFrq != 0)
			genFlags |= SBGEN_VIBLFOF;
		if (modLfoFrq != 0)
			genFlags |= SBGEN_MODLFOF;
		if (modLfoVol != 0)
			genFlags |= SBGEN_MODLFOA;
		if (modEnvFrq != 0)
			genFlags |= SBGEN_MODENVF;
	}
};

/// @brief Soundbank Instrument
/// An instrument is a collection of zones and global modulators.
class SBInstr : public SynthList<SBInstr>
{
public:
	SynthEnumList<SBZone> zoneList;
	SynthEnumList<SBModInfo> modList;
	bsString instrName;
	bsInt16  instrNdx;
	bsInt16  bank;
	bsInt16  prog;
	bsInt16  loaded;
	bsInt16  lowKey;
	bsInt16  highKey;
	bsInt16  fixedKey;
	bsInt16  fixedVel;

	SBInstr()
	{
		fixedKey = -1;
		fixedVel = -1;
		lowKey = 0;
		highKey = 127;
		instrNdx = -1;
		bank = 0;
		prog = 0;
		loaded = 0;
	}

	~SBInstr()
	{
	}

	SBZone *AddZone()
	{
		return zoneList.AddItem();
	}

	/// @brief Locate a zone
	/// GenZone checks all zones to see if an entry
	/// exists which matches the key and velocity.
	/// @note
	/// There may be multiple zones that match.
	/// Use EnumZones to find all matches.
	/// @endnote
	SBZone *GetZone(int key, int vel)
	{
		SBZone *zone = 0;
		while ((zone = zoneList.EnumItem(zone)) != 0)
		{
			if (zone->Match(key, vel))
				break;
		}
		return zone;
	}

	/// Enumerate all zones.
	/// On the first call, zone should be set equal to NULL.
	/// @code
	/// SBZone *zone = 0;
	/// while ((zone = EnumZones(zone)) != 0)
	///    use zone;
	/// @endcode
	/// @param zone previous zone or NULL for first zone.
	/// @return next zone or NULL for no more zones.
	SBZone *EnumZones(SBZone *zone)
	{
		return zoneList.EnumItem(zone);
	}

	/// @brief Get modulator information
	/// Locate any modulator info that connects 
	/// the given source generator to the given
	/// destination generator.
	/// @param src Source generator
	/// @param dst Destination generator
	SBModInfo *GetModInfo(short src, short dst)
	{
		SBModInfo *mod = 0;
		while ((mod = modList.EnumItem(mod)) != 0)
		{
			if (mod->srcOp == src && mod->dstOp == dst)
				return mod;
		}
		return 0;
	}

	/// Add a new modulator.
	SBModInfo *AddModInfo()
	{
		return modList.AddItem();
	}

	/// Enumerate all modulators.
	/// On the first call, mi should be set equal to NULL.
	/// @code
	/// SBModInfo *mi = 0;
	/// while ((mi = EnumZones(mi)) != 0)
	///    use mi;
	/// @endcode
	/// @param mi previous modulator or NULL for first.
	/// @return next modulator or NULL for no more modulators.
	SBModInfo *EnumModInfo(SBModInfo *mi)
	{
		return modList.EnumItem(mi);
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
/// bank mapping list, but, for now, if the LSB is zero,
/// we use the MSB part as the bank number.
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
private:
	int lockCount;
public:
	bsString file;                 ///< file name (empty if constructed in memory)
	bsString name;                 ///< symbolic name
	SBInfo info;                   ///< INFO records (copyright, version, etc.)
	SBInstr **instrBank[129];      ///< array of instruments[bank][patch]
	SBSample *samples;             ///< list of sample blocks

	static SoundBank SoundBankList; ///< List of loaded soundbanks
	static void DeleteBankList();  ///< Remove all soundbanks
	static SoundBank *FindBank(const char *name); ///< Find soundbank by name

	static FrqValue *envRateTable;

	/// Convert relative cents to frequency multiplier.
	/// Pitch in cents is defined as: pc = 1200 log2(df), 
	/// where df is frequency deviation in Hz.
	/// @param pc pitch deviation in cents
	/// @return multiplier for the base frequency.
	static FrqValue PitchCents(FrqValue pc);

	/// Convert time cents to time
	/// Time cents is defined as tc = 1200 * log2(sec),
	/// where tc ranges from -12000 (1ms) to the maximum rate.
	/// Max rate varies, but is usually no more than 8000 (100s)
	/// @param tc time in time cents
	/// @return time in seconds
	static FrqValue EnvRate(FrqValue tc);

	/// Convert attenuation in centibles to linear amplitude.
	/// Centibels is defined as cb = 200 * log10(amp), where
	/// cb ranges from 0 (no attenuation) to -960 (silence)
	/// @param cb attenuation in centibels
	/// @return linear amplitude (0-1)
	static AmpValue Attenuation(AmpValue cb);

	SoundBank()
	{
		lockCount = 0;
		samples = 0;
		memset(&instrBank[0], 0, sizeof(instrBank));
		if (envRateTable == 0)
		{
			envRateTable = new FrqValue[20000];
			double x = -12000.0;
			for (int index = 0; index < 20000; index++)
			{
				envRateTable[index] = pow(2.0, x / 1200.0);
				x += 1.0;
			}
		}
	}

	~SoundBank()
	{
		Remove(); // justin-case

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

	/// @name SoundBank locking.
	/// A SoundBank object is likely shared by multiple instruments
	/// and editors. We do not want to delete until all instruments
	/// have released their reference. The lock count is
	/// used for this purpose.
	/// @{
	/// Lock the sound bank.
	int Lock() { return ++lockCount; }
	/// Unlock the sound bank.
	/// If this is the last reference, the object is deleted.
	int Unlock()
	{
		int cnt = --lockCount;
		if (lockCount <= 0)
		{
			Remove();
			delete this;
		}
		return cnt;
	}
	/// @}

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

	/// @name Sample Loading
	/// Load sample data from the original file.
	/// If the sample cannot be loaded, a block of zeros is allocated.
	/// @param samp pointer to sample block object.
	/// @param f already open file
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
//@}

#endif
