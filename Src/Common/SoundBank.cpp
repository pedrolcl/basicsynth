#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <SynthDefs.h>
#include <SynthList.h>
#include <SoundBank.h>

SoundBank SoundBank::SoundBankList;

void SoundBank::DeleteBankList()
{
	SoundBank *bnk;
	while ((bnk = SoundBankList.next) != 0)
	{
		bnk->Remove();
		delete bnk;
	}
}

SoundBank *SoundBank::FindBank(const char *name)
{
	SoundBank *bnk;
	for (bnk = SoundBankList.next; bnk; bnk = bnk->next)
	{
		if (bnk->name.Compare(name) == 0)
			break;
	}
	return bnk;
}

// duration as tc = 1200 * log2(sec)
// tc ranges from -12000 (1ms) to the maximum rate.
// max rate varies, but is usually no more than 8000 (100s)
FrqValue SoundBank::EnvRate(FrqValue tc)
{
	if (tc <= -12000.0)
		return 0.0;
	return pow(2.0, tc / 1200.0);
}

// Attenuation in centibels cb = 200 * log10(amp)
// cb ranges from 0 (no attenuation) to -960 (silence)
AmpValue SoundBank::Attenuation(AmpValue cb)
{
	if (cb >= 960)
		return 0.0;
	return pow(10.0, cb / 200.0);
}

// Pitch in cents: pc = 1200 log2(df)
// df is frequency deviation.
// the return value is a multiplier for the
// base frequency.
FrqValue SoundBank::PitchCents(FrqValue pc)
{
	return pow(2.0, pc / 1200.0);
}

/// Load samples. These will always create a valid
/// pointer to sample values, but the values will
/// be zero on failure.
int SoundBank::LoadInstr(SBInstr *in)
{
	if (in->loaded)
		return 0;

	FileReadBuf f;
	// big enough to hold a typical sample, 
	// not too big to avoid reading things we don't need.
	f.SetBufSize(65536); 
	if (f.FileOpen(file) != 0)
		return -1;
	int r = LoadInstr(in, f);
	f.FileClose();
	return r;
}

int SoundBank::LoadInstr(SBInstr *in, FileReadBuf& f)
{
	if (in->loaded)
		return 0;

	int err = 0;

	SBZone *zone = 0;
	while ((zone = in->EnumZones(zone)) != 0)
	{
		SBSample *samp = samples;
		while (samp)
		{
			if (samp->index == zone->sampleNdx)
			{
				err |= LoadSample(samp, f);
				zone->sample = &samp->sample[zone->tableStart];
				zone->sampleLen = samp->sampleLen;
				zone->tableEnd = zone->sampleLen+1;
				break;
			}
			samp = samp->next;
		}
	}
	in->loaded = 1;
	return err;
}

int SoundBank::LoadSample(SBSample *samp)
{
	if (samp->sample)
		return 0;

	FileReadBuf f;
	f.SetBufSize(samp->sampleLen * 2 * samp->channels); // typical
	if (f.FileOpen(file) != 0)
	{
		bsUint32 samplen = samp->sampleLen;
		samp->sample = new AmpValue[samplen+2];
		AmpValue *sp = samp->sample;
		for (bsUint32 cnt = 0; cnt < samplen; cnt++)
			*sp++ = 0.0;
		*sp++ = 0.0;
		*sp++ = 0.0;
		return -1;
	}
	int r = LoadSample(samp, f);
	f.FileClose();
	return r;
}

// We can have one block of either 1 or 2 channel,
// or one block for each channel
//  
int SoundBank::LoadSample(SBSample *samp, FileReadBuf& f)
{
	if (samp->sample)
		return 0;

	samp->sample = new AmpValue[samp->sampleLen + 2];

	int err = 0;
	if (samp->channels == 1)
		err = ReadSamples1(samp, f);
	else
		err = ReadSamples2(samp, f);

	return err;
}

int SoundBank::ReadSamples1(SBSample *samp, FileReadBuf& f)
{
	bsInt32 cnt;
	bsInt32 samplen = samp->sampleLen;

	AmpValue *sp = samp->sample;
	if (samp->filepos == 0)
	{
		for (cnt = 0; cnt < samplen; cnt++)
			*sp++ = 0.0;
		return -1;
	}
	f.FileRewind(samp->filepos);
	if (samp->format == 0) // 8-bit
	{
		for (cnt = 0; cnt < samplen; cnt++)
			*sp++ = (AmpValue) (f.ReadCh() - 128) / 128.0;
	}
	else if (samp->format == 1 || samp->format == 3) // 16-bit
	{
		for (cnt = 0; cnt < samplen; cnt++)
		{
			short val = f.ReadCh() | (f.ReadCh() << 8);
			*sp++ = (AmpValue) val / 32768.0;
		}
		if (samp->format == 3 && samp->filepos2 != 0) // 24-bit
		{
			f.FileRewind(samp->filepos2);
			sp = samp->sample;
			for (cnt = 0; cnt < samplen; cnt++)
				*sp++ += ((AmpValue) f.ReadCh() / 8388608.0);
		}
	}
	else if (samp->format == 2)
	{
		// FIXME: little-endian only...
		float val;
		for (cnt = 0; cnt < samplen; cnt++)
		{
			f.FileRead(&val, 4);
			*sp++ = val;
		}
	}
	*sp++ = 0;
	*sp++ = 0;
	return 0;
}

int SoundBank::ReadSamples2(SBSample *samp, FileReadBuf& f)
{
	bsInt32 cnt;
	bsInt32 samplen = samp->sampleLen;
	AmpValue *sp = samp->sample;

	if (samp->filepos == 0)
	{
		for (cnt = 0; cnt <= samplen; cnt++)
			*sp++ = 0;
		return -1;
	}

	f.FileRewind(samp->filepos);
	if (samp->format == 0)
	{
		for (cnt = 0; cnt < samplen; cnt++)
		{
			short val1 = f.ReadCh() - 128;
			short val2 = f.ReadCh() - 128;
			*sp++ = ((AmpValue) val1 + (AmpValue) val2) / 256.0;
		}
	}
	else if (samp->format == 1)
	{
		for (cnt = 0; cnt < samplen; cnt++)
		{
			short val1 = (short) (f.ReadCh() | (f.ReadCh() << 8));
			short val2 = (short) (f.ReadCh() | (f.ReadCh() << 8));
			*sp++ = ((AmpValue) val1 + (AmpValue) val2) / 65536.0;
		}
	}
	else if (samp->format == 2)
	{
		float val1, val2;
		for (cnt = 0; cnt < samplen; cnt++)
		{
			f.FileRead(&val1, 4);
			f.FileRead(&val2, 4);
			*sp++ = (val1 + val2) / 2.0;
		}
	}
	*sp++ = 0;
	*sp = 0;
	return 0;
}