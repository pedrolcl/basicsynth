#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <SynthDefs.h>
#include <WaveFile.h>
#include <SynthList.h>
#include <SFFile.h>

SFSoundBank SFSoundBank::SoundBankList;

class SFSoundBankTrans
{
public:
	AmpValue *ampVals;
	FrqValue *rteVals;
	FrqValue *timeCents;
	SFSoundBankTrans();
	~SFSoundBankTrans();
};

static SFSoundBankTrans trans;

SFSoundBankTrans::SFSoundBankTrans()
{
	int n;
	ampVals = new AmpValue[960];
	double lvl = 0;
	for (n = 0; n < 960; n++)
	{
		ampVals[n] = (AmpValue) pow(10.0, (double) lvl / -200.0);
		lvl += 1.0;
	}
	rteVals = new FrqValue[20000];
	double rt = -12000;
	for (n = 0; n < 20000; n++)
	{
		rteVals[n] = (FrqValue) pow(2.0, rt / 1200.0);
		rt += 1.0;
	}
	timeCents = new FrqValue[2400];
	double t = -1200;
	for (n = 0; n < 2400; n++)
	{
		timeCents[n] = (FrqValue) pow(2.0, t / 1200.0);
		t += 1.0;
	}
}

SFSoundBankTrans::~SFSoundBankTrans()
{
	delete ampVals;
	ampVals = 0;
	delete rteVals;
	rteVals = 0;
	delete timeCents;
}

FrqValue SFEnvRate(short rt)
{
	if (rt <= -12000)
		return 0.0;
	if (rt >= 8000)
		return trans.rteVals[19999];
	return trans.rteVals[rt+12000];
	//return (FrqValue) pow(2.0, (double) rt / 1200.0);
}

AmpValue SFEnvLevel(short lvl)
{
	if (lvl == 0)
		return 1.0;
	if (lvl >= 960)
		return 0.0;
	return trans.ampVals[lvl];
//	return (AmpValue) pow(10.0, (double) lvl / -200.0);
}

FrqValue SFTimeCents(short val)
{
	if (val <= -1200)
		return 0.5;
	if (val >= 1200)
		return 1.0;
	return trans.timeCents[val+1200];
}

FrqValue SFKeyScaleTime(short key, short amt)
{
	return SFTimeCents((60 - key) * amt);
}

FrqValue SFAbsCents(short amt)
{
	return SFRelCents(8.197, amt);
}

FrqValue SFRelCents(FrqValue f, short amt)
{
	return f * pow(2, (double) amt / 12000);
}

void SFSoundBank::DeleteBankList()
{
	SFSoundBank *bnk;
	while ((bnk = SoundBankList.next) != 0)
	{
		bnk->Remove();
		delete bnk;
	}
}

SFSoundBank *SFSoundBank::FindBank(const char *name)
{
	SFSoundBank *bnk;
	for (bnk = SoundBankList.next; bnk; bnk = bnk->next)
	{
		if (bnk->name.Compare(name) == 0)
			break;
	}
	return bnk;
}


SFFile::SFFile()
{
	file.SetBufSize(0x10000);
	npresets = 0;
	phdr = 0;
	npbags = 0;
	pbag = 0;
	npmods = 0;
	pmod = 0;
	npgens = 0;
	pgen = 0;
	ninsts = 0;
	inst = 0;
	nibags = 0;
	ibag = 0;
	nimods = 0;
	imod = 0;
	nigens = 0;
	igen = 0;
	nshdrs = 0;
	shdr = 0;
	sfbnk = 0;
	loadMods = 0;
}

SFFile::~SFFile()
{
	delete phdr;
	delete pbag;
	delete pmod;
	delete pgen;
	delete inst;
	delete ibag;
	delete imod;
	delete igen;
	delete shdr;
}

SFSoundBank *SFFile::LoadSoundBank(const char *fname, int mods)
{
	if (file.FileOpen(fname))
		return 0;

	sfChunk rchk;
	// read the RIFF chunk.
	file.FileRead(&rchk, sfChunkSize);
	if (rchk.ckid != SF_RIFF_CHUNK)
	{
		file.FileClose();
		return 0;
	}

	// when set, we will load modulator records, otherwise not
	loadMods = mods;

	// Read the sfbk chunk
	unsigned long id = 0;
	file.FileRead(&id, 4);
	if (id != SF_SFBK_FORMAT)
	{
		file.FileClose();
		return 0;
	}

	sfbnk = new SFSoundBank;

	sfChunk chk;
	int err = 0;
	long riffSize = rchk.cksz - 4;
	while (riffSize > 0)
	{
		// read the next chunk header
		if (file.FileRead(&chk, sfChunkSize) != sfChunkSize)
			break;
		riffSize -= sfChunkSize;
		if (chk.ckid == SF_LIST_CHUNK)
		{
			if (ListChunk(chk))
			{
				err = -1;
				break;
			}
		}
		else
			file.FileSkip(chk.cksz);
		riffSize -= chk.cksz;
	}

	file.FileClose();
	if (err)
	{
		delete sfbnk;
		return 0;
	}

	BuildSoundBank();

	return sfbnk;
}

int SFFile::ReadData(void **pdata, long cksz)
{
	if (*pdata)
		free(*pdata);
	*pdata = malloc(cksz);
	if (*pdata == 0)
		return -1;
	if (file.FileRead(*pdata, cksz) != cksz)
		return -1;
	return 0;
}

void SFFile::ReadString(long len, bsString *str)
{
	char *s = (char *)malloc(len+1);
	file.FileRead(s, len);
	str->Attach(s);
}

int SFFile::InfoChunk(long cksz)
{
	if (sfbnk == 0)
	{
		file.FileSkip(cksz);
		return 0;
	}

	sfChunk chk;
	while (cksz > 0)
	{
		memset(&chk, 0, sizeof(chk));
		if (file.FileRead(&chk, sfChunkSize) != sfChunkSize)
			return -1;
		cksz -= sfChunkSize;
		switch (chk.ckid)
		{
		case SF_IFIL_CHUNK:
			// File Version
			file.FileRead(&sfbnk->info.wMajorFile, 2);
			file.FileRead(&sfbnk->info.wMinorFile, 2);
			break;
		case SF_ISNG_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szSoundEngine);
			break;
		case SF_IROM_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szROM);
			break;
		case SF_IVER_CHUNK:
			// Version
			file.FileRead(&sfbnk->info.wMajorVer, 2);
			file.FileRead(&sfbnk->info.wMinorVer, 2);
			break;
		case SF_INAM_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szName);
			break;
		case SF_ICRD_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szDate);
			break;
		case SF_IENG_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szEng);
			break;
		case SF_IPRD_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szProduct);
			break;
		case SF_ICOP_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szCopyright);
			break;
		case SF_ICMT_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szComment);
			break;
		case SF_ISFT_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szTools);
			break;
		default:
			file.FileSkip(chk.cksz);
			break;
		}
		cksz -= chk.cksz;
	}
	return 0;
}

#define SAMPBUF_SZB 65536
#define SAMPBUF_SZS (SAMPBUF_SZB/sizeof(short))

int SFFile::SDTAChunk(long cksz)
{
	if (sfbnk == 0)
	{
		file.FileSkip(cksz);
		return 0;
	}

	float div = 32767.0; // for 16 bit samples (2^15 - 1)
	long ssz;
	AmpValue *sp;
	sfChunk chk;
	while (cksz > 0)
	{
		if (file.FileRead(&chk, sfChunkSize) != sfChunkSize)
			return -1;
		cksz -= 8;
		if (chk.ckid == SF_SMPL_CHUNK)
		{
			sfbnk->sampleSize = chk.cksz / sizeof(short);
			sfbnk->samples = new AmpValue[sfbnk->sampleSize];
			sp = sfbnk->samples;

			short *buf = new short[SAMPBUF_SZS];
			ssz = chk.cksz;
			while (ssz > 0)
			{
				int toRead = ssz > SAMPBUF_SZB ? SAMPBUF_SZB : ssz;
				if (file.FileRead(buf, toRead) != toRead)
					break;
				ssz -= toRead;
				short *bp = buf;
				while (toRead > 0)
				{
					*sp++ = (AmpValue) *bp++;
					toRead -= sizeof(short);
				}
			}
			delete buf;
		}
		else if (chk.ckid == SF_SM24_CHUNK)
		{
			if (sfbnk->samples)
			{
				sp = sfbnk->samples;
				div = 8388607.0; // for 24 bit samples (2^23 - 1)
				unsigned char *buf = new unsigned char[SAMPBUF_SZB];
				ssz = chk.cksz;
				while (ssz > 0)
				{
					int toRead = ssz > SAMPBUF_SZB ? SAMPBUF_SZB : ssz;
					if (file.FileRead(buf, toRead) != toRead)
						break;
					ssz -= toRead;
					unsigned char *bp = buf;
					while (toRead > 0)
					{
						*sp++ = (*sp * 256.0) + (AmpValue) *bp++;
						toRead--;
					}
				}
				delete buf;
			}
			else
				file.FileSkip(chk.cksz);
		}
		else
		{
			file.FileSkip(chk.cksz);
		}
		cksz -= chk.cksz;
	}

	// normalize samples to range [-1,+1]
	sp = sfbnk->samples;
	ssz = sfbnk->sampleSize;
	while (--ssz >= 0)
		*sp++ /= div;

	return cksz != 0;
}

int SFFile::PDTAChunk(long cksz)
{
	int err = 0;
	sfChunk chk;
	while (cksz > 0)
	{
		if (file.FileRead(&chk, sfChunkSize) != sfChunkSize)
			return -1;
		cksz -= 8;
		err = 0;
		switch (chk.ckid)
		{
		case SF_PHDR_CHUNK:
			npresets = (chk.cksz / sfPresetHeaderSize);
			if (npresets > 0)
				err = ReadData((void**)&phdr, chk.cksz);
			break;
		case SF_PBAG_CHUNK:
			npbags = chk.cksz / sfBagSize;
			if (npbags > 0)
				err = ReadData((void**)&pbag, chk.cksz);
			break;
		case SF_PMOD_CHUNK:
			npmods = chk.cksz / sfModListSize;
			if (npmods > 0)
				err = ReadData((void**)&pmod, chk.cksz);
			break;
		case SF_PGEN_CHUNK:
			npgens = chk.cksz / sfGenListSize;
			if (npgens > 0)
				err = ReadData((void**)&pgen, chk.cksz);
			break;
		case SF_INST_CHUNK:
			ninsts = chk.cksz / sfInstSize;
			if (ninsts > 0)
				err = ReadData((void**)&inst, chk.cksz);
			break;
		case SF_IBAG_CHUNK:
			nibags = chk.cksz / sfBagSize;
			if (nibags > 0)
				err = ReadData((void**)&ibag, chk.cksz);
			break;
		case SF_IMOD_CHUNK:
			nimods = chk.cksz / sfModListSize;
			if (nimods > 0)
				err = ReadData((void**)&imod, chk.cksz);
			break;
		case SF_IGEN_CHUNK:
			nigens = chk.cksz / sfGenListSize;
			if (nigens > 0)
				err = ReadData((void**)&igen, chk.cksz);
			break;
		case SF_SHDR_CHUNK:
			nshdrs = chk.cksz / sfSampleSize;
			if (nshdrs > 0)
				err = ReadData((void**)&shdr, chk.cksz);
			break;
		default:
			file.FileSkip(chk.cksz);
			break;
		}
		if (err != 0)
			return -1;
		cksz -= chk.cksz;
	}
	return 0;
}

int SFFile::ListChunk(sfChunk& chk)
{
	int err = 0;
	unsigned long id = 0;
	if (file.FileRead(&id, 4) != 4)
		return -1;
	switch (id)
	{
	case SF_INFO_FORMAT:
		err = InfoChunk(chk.cksz - 4);
		break;
	case SF_SDTA_FORMAT:
		err = SDTAChunk(chk.cksz - 4);
		break;
	case SF_PDTA_FORMAT:
		err = PDTAChunk(chk.cksz - 4);
		break;
	default:
		file.FileSkip(chk.cksz - 4);
		break;
	}
	return err;
}

void SFFile::BuildInstrument(SFInstr *in, int n)
{
	if (n < 0 || n >= ninsts)
		return;

	char nameTemp[24];
	short bagNdx1, bagNdx2;
	short genNdx1, genNdx2;
	sfSample *sh;
	sfGenList *ig;

	memcpy(nameTemp, inst[n].achInstName, 20);
	nameTemp[20] = 0;
	in->instrName = nameTemp;

	SFZone *globZone = 0;
	bagNdx1 = inst[n].wInstBagNdx;
	bagNdx2 = inst[n+1].wInstBagNdx;
	while (bagNdx1 < bagNdx2)
	{
		genNdx1 = ibag[bagNdx1].wGenNdx;
		genNdx2 = ibag[bagNdx1+1].wGenNdx;
		SFZone *zone = in->AddZone();
		if (globZone)
			memcpy(zone->genVals, globZone->genVals, sizeof(zone->genVals));

		while (genNdx1 < genNdx2)
		{
			ig = &igen[genNdx1];
			zone->genVals[ig->sfGenOper] = ig->genAmount.shAmount;
			switch (ig->sfGenOper)
			{
			case sfgKeyRange:
				zone->lowKey = ig->genAmount.ranges.byLo;
				zone->highKey = ig->genAmount.ranges.byHi;
				break;
			case sfgVelRange:
				zone->lowVel = ig->genAmount.ranges.byLo;
				zone->highVel = ig->genAmount.ranges.byHi;
				break;
			case sfgSampleID:
				// this is specified as the last gen num.
				// we can assume all modifications have been set in genVals[]
				zone->instZone = 1;
				sh = &shdr[ig->genAmount.wAmount];
				memcpy(nameTemp, sh->achSampleName, 20);
				nameTemp[20] = 0;
				zone->name = nameTemp;
				zone->cents = (bsInt16) sh->chCorrection
					        + zone->genVals[sfgCoarseTune] * 100
							+ zone->genVals[sfgFineTune];
				zone->keyNum = zone->genVals[sfgOverridingRootKey];
				if (zone->keyNum == -1)
					zone->keyNum = sh->byOriginalKey;
				zone->rate = (FrqValue) sh->dwSampleRate;
				zone->sampleLen = sh->dwEnd - sh->dwStart;
				if (sh->sfSampleType == 1 || sh->sfSampleType == 2)
					zone->chan = 0; // right channel or mono sample
				else
					zone->chan = 1;
				zone->mode = zone->genVals[sfgSampleModes];
				zone->tableEnd = (sh->dwEnd - sh->dwStart) 
					           + zone->genVals[sfgEndAddrsOffset]
				               + (zone->genVals[sfgEndAddrsCoarseOffset] * 65536);
				zone->loopStart = (sh->dwStartloop - sh->dwStart)
					            + zone->genVals[sfgStartloopAddrsOffset]
								+ (zone->genVals[sfgStartloopAddrsCoarseOffset] * 65536);
				zone->loopEnd = (sh->dwEndloop - sh->dwStart)
					          + zone->genVals[sfgEndloopAddrsOffset]
							  + (zone->genVals[sfgEndloopAddrsCoarseOffset] * 65536);
				zone->sample = &sfbnk->samples[sh->dwStart
					                          + zone->genVals[sfgStartAddrsOffset]
					                          + (zone->genVals[sfgStartAddrsCoarseOffset]*65536)];
				break;
			case sfgPan:
				zone->pan = (FrqValue) ig->genAmount.shAmount / 500.0;
				break;
			}
			genNdx1++;
		}
		if (zone->keyNum != -1)
		{
			zone->recFreq = synthParams.GetFrequency(zone->keyNum - 12);
			if (zone->cents != 0)
				zone->recFreq *= synthParams.GetCentsMult(-zone->cents);
		}
		if (globZone == 0 && !zone->instZone)
			globZone = zone;

		if (loadMods)
		{
			genNdx1 = ibag[bagNdx1].wModNdx;
			genNdx2 = ibag[bagNdx1+1].wModNdx;
			while (genNdx1 < genNdx2)
			{
				sfModList *mp = &imod[genNdx1];
				SFModInfo *mi = in->AddModInfo();
				mi->srcOp = mp->sfModSrcOper;
				mi->dstOp = mp->sfModDestOper;
				mi->value = mp->modAmount;
				mi->srcAmntOp = mp->sfModAmtSrcOper;
				mi->transOp = mp->sfModTransOper;
				genNdx1++;
			}
		}
		bagNdx1++;
	}
}

void SFFile::BuildPreset(int n)
{
	char nameTemp[24];
	short bagNdx1, bagNdx2;
	short genNdx1, genNdx2;

	SFPreset *preset = sfbnk->AddPreset(phdr[n].wBank, phdr[n].wPreset);
	if (preset == 0)
		return;

	memcpy(nameTemp, phdr[n].achPresetName, 20);
	nameTemp[20] = '\0';
	preset->presetName = nameTemp;

	bagNdx1 = phdr[n].wPresetBagNdx;
	bagNdx2 = phdr[n+1].wPresetBagNdx;
	preset->AllocInstr(bagNdx2 - bagNdx1);
	int instrNdx = 0;
	while (bagNdx1 < bagNdx2)
	{
		genNdx1 = pbag[bagNdx1].wGenNdx;
		genNdx2 = pbag[bagNdx1+1].wGenNdx;
		while (genNdx1 < genNdx2)
		{
			sfGenList *pg = &pgen[genNdx1];
			preset->genVals[pg->sfGenOper] = pg->genAmount.shAmount;
			if (pg->sfGenOper == sfgInstrument)
			{
				SFInstr *instr = new SFInstr;
				instr->instrNdx = instrNdx;
				preset->instrList[instrNdx++] = instr;
				BuildInstrument(instr, pg->genAmount.shAmount);
			}
			genNdx1++;
		}
		if (loadMods)
		{
			genNdx1 = pbag[bagNdx1].wModNdx;
			genNdx2 = pbag[bagNdx1+1].wModNdx;
			while (genNdx1 < genNdx2)
			{
				sfModList *ml = &pmod[genNdx1];
				SFModInfo *mi = preset->AddModInfo();
				mi->srcOp = ml->sfModSrcOper;
				mi->dstOp = ml->sfModDestOper;
				mi->value = ml->modAmount;
				mi->srcAmntOp = ml->sfModAmtSrcOper;
				mi->transOp = ml->sfModTransOper;
				genNdx1++;
			}
		}
		bagNdx1++;
	}
	preset->InitZoneMap();
}

void SFFile::BuildSoundBank()
{
	int n, m;

	m = npresets-1;
	for (n = 0; n < m; n++)
		BuildPreset(n);
}

void InitGenVals(short *genVals)
{
	memset(genVals, 0, sizeof(short)*sfgEndOper);
	genVals[sfgInitialFilterQ] = 13500;
	genVals[sfgScaleTuning] = 100;
	genVals[sfgKeynum] = (127 << 8);
	genVals[sfgVelocity] = (127 << 8);
	genVals[sfgOverridingRootKey] = -1;
	genVals[sfgDelayVolEnv] = -12000;
	genVals[sfgAttackVolEnv] = -12000;
	genVals[sfgHoldVolEnv] = -12000;
	genVals[sfgDecayVolEnv] = -12000;
	genVals[sfgSustainVolEnv] = 0;
	genVals[sfgReleaseVolEnv] = -12000;
}