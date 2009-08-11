#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <SynthDefs.h>
#include <WaveFile.h>
#include <SynthList.h>
#include <SFFile.h>

SFFile::SFFile()
{
	preload = 1;
	atnScl = 0.375;

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
	InitGenVals(hdrVals);
	samples = 0;
	sampleSize = 0;
	sampleFileOffs1 = 0;
	sampleFileOffs2 = 0;
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

int SFFile::IsSF2File(const char *fname)
{
	FileReadBuf file;
	int isSF2 = 0;
	if (file.FileOpen(fname) == 0)
	{
		// read the RIFF chunk.
		sfChunk rchk;
		rchk.ckid = 0;
		file.FileRead(&rchk, 8);
		if (rchk.ckid == SF_RIFF_CHUNK)
		{
			// Read the format
			bsInt32 id = 0;
			file.FileRead(&id, 4);
			if (id == SF_SFBK_FORMAT)
				isSF2 = 1;
		}
		file.FileClose();
	}
	return isSF2;
}

SoundBank *SFFile::LoadSoundBank(const char *fname, int pre, float scl)
{
	preload = pre;
	atnScl = scl;

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

	// Read the sfbk chunk
	bsInt32 id = 0;
	file.FileRead(&id, 4);
	if (id != SF_SFBK_FORMAT)
	{
		file.FileClose();
		return 0;
	}

	sfbnk = new SoundBank;
	sfbnk->file = fname;
	sampleFileOffs1 = 0;
	sampleFileOffs2 = 0;

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

	if (err)
	{
		delete sfbnk;
		return 0;
	}

	BuildSoundBank();
	file.FileClose();

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
		//case SF_IROM_CHUNK:
		//	ReadString(chk.cksz, &sfbnk->info.szROM);
		//	break;
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

int SFFile::SDTAChunk(long cksz)
{
	sfChunk chk;
	while (cksz > 0)
	{
		if (file.FileRead(&chk, sfChunkSize) != sfChunkSize)
			return -1;
		cksz -= 8;
		if (chk.ckid == SF_SMPL_CHUNK)
		{
			sampleSize = chk.cksz / sizeof(short);
			sampleFileOffs1 = file.FilePosition();
			//samples = new short[sampleSize];
			//file.FileRead(samples, chk.cksz);
			file.FileSkip(chk.cksz);
		}
		else if (chk.ckid == SF_SM24_CHUNK)
		{
			sampleFileOffs2 = file.FilePosition();
			file.FileSkip(chk.cksz);
		}
		cksz -= chk.cksz;
	}

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
	bsInt32 id = 0;
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

SBSample *SFFile::CreateSample(sfSample *shdr, short id)
{
	SBSample *samp = sfbnk->GetSample(id, 0);
	if (samp != 0)
		return samp;
	samp = sfbnk->AddSample(id);
	samp->channels = 1;
	samp->sampleLen = (shdr->dwEnd - shdr->dwStart);
	samp->filepos = sampleFileOffs1 + (shdr->dwStart * 2);
	if (sampleFileOffs2 != 0)
	{
		samp->format = 3;
		samp->filepos = sampleFileOffs2 + shdr->dwStart;
	}
	else
		samp->format = 1;

	if (preload)
		sfbnk->LoadSample(samp, file);

	return samp;
}

// List of gens that are additive
int SFFile::AddHeaderGen(short gen)
{
	switch (gen)
	{
	case sfgStartAddrsOffset:
	case sfgEndAddrsOffset:
	case sfgStartloopAddrsOffset:
	case sfgEndloopAddrsOffset:
	case sfgStartAddrsCoarseOffset:
	case sfgEndAddrsCoarseOffset:
	case sfgInstrument:
	case sfgKeyRange:
	case sfgVelRange:
	case sfgStartloopAddrsCoarseOffset:
	case sfgKeynum:
	case sfgVelocity:
	case sfgEndloopAddrsCoarseOffset:
	case sfgSampleModes:
	case sfgScaleTuning:
	case sfgExclusiveClass:
	case sfgOverridingRootKey:
	case sfgSampleID:
	case sfgUnused1:
	case sfgUnused2:
	case sfgUnused3:
	case sfgUnused4:
	case sfgUnused5:
	case sfgReserved1:
	case sfgReserved2:
	case sfgReserved3:
		return 0;
	}
	return 1;
}

void SFFile::BuildInstrument(SBInstr *in, int n)
{
	if (n < 0 || n >= ninsts)
		return;

	char nameTemp[24];
	short bagNdx1, bagNdx2;
	short genNdx1, genNdx2;
	sfSample *sh;
	sfGenList *ig;

	double modval;
	short genVals[sfgEndOper];
	short globVals[sfgEndOper];
	InitGenVals(globVals);
	globVals[sfgKeyRange] = hdrVals[sfgKeyRange];
	globVals[sfgVelRange] = hdrVals[sfgVelRange];

	SBZone *globZone = 0;
	SBSample *samp;
	bagNdx1 = inst[n].wInstBagNdx;
	bagNdx2 = inst[n+1].wInstBagNdx;
	while (bagNdx1 < bagNdx2)
	{
		memcpy(genVals, globVals, sizeof(globVals));

		int inszone = 0;
		genNdx1 = ibag[bagNdx1].wGenNdx;
		genNdx2 = ibag[bagNdx1+1].wGenNdx;
		while (genNdx1 < genNdx2)
		{
			ig = &igen[genNdx1];
			genVals[ig->sfGenOper] = ig->genAmount.shAmount;
			if (ig->sfGenOper == sfgSampleID)
				inszone = 1;
			genNdx1++;
		}

		if (inszone)
		{
			for (genNdx1 = 0; genNdx1 < sfgEndOper; genNdx1++)
			{
				if (AddHeaderGen(genNdx1))
					genVals[genNdx1] += hdrVals[genNdx1];
			}

			SBZone *zone = in->AddZone();
			zone->lowKey = genVals[sfgKeyRange] & 0xff;
			zone->highKey = (genVals[sfgKeyRange] >> 8) & 0xff;
			zone->lowVel = genVals[sfgVelRange] & 0xff;
			zone->highVel = (genVals[sfgVelRange] >> 8) & 0xff;
			sh = &shdr[genVals[sfgSampleID]];
			zone->name = CopyName(nameTemp, sh->achSampleName);
			zone->cents = (bsInt16) sh->chCorrection
				        + genVals[sfgCoarseTune] * 100
						+ genVals[sfgFineTune];
			zone->keyNum = genVals[sfgOverridingRootKey];
			if (zone->keyNum == -1)
				zone->keyNum = sh->byOriginalKey;
			zone->rate = (FrqValue) sh->dwSampleRate;
			zone->recFreq = synthParams.GetFrequency(zone->keyNum - 12);
			if (zone->cents != 0)
				zone->recFreq *= synthParams.GetCentsMult(-zone->cents);
			zone->recPeriod = (bsInt32) (zone->rate / zone->recFreq);
			zone->sampleLen = sh->dwEnd - sh->dwStart;
			zone->volAtten = SFAttenuation(-genVals[sfgInitialAttenuation]);
			zone->pan = (FrqValue) genVals[sfgPan] / 500.0;
			if (zone->pan < -1.0)
				zone->pan = -1.0;
			else if (zone->pan > 1.0)
				zone->pan = 1.0;
			if (sh->sfSampleType == 4)
				zone->chan = 1;
			else if (sh->sfSampleType == 8)
			{
				// hmmm... "not fully defined in SF2" (I'm guessing here...)
				sfSample *sh2 = &shdr[sh->wSampleLink];
				if (sh2->sfSampleType & 4)
					zone->chan = 1;
				else
					zone->chan = 0;

			}
			else //if (sh->sfSampleType == 1 || sh->sfSampleType == 2)
				zone->chan = 0; // right channel or mono sample
			// HACK around non-standard stuff...
			SBZone *z2 = 0;
			while ((z2 = in->EnumZones(z2)) != 0)
			{
				if (z2 != zone
					&& z2->chan == zone->chan
					&& z2->lowKey == zone->lowKey
					&& z2->highKey == zone->highKey
					&& z2->lowVel == zone->lowVel
					&& z2->highVel == zone->highVel
					&& z2->pan != zone->pan)
				{
					// OK - we have two zones that have the same key and velocity,
					// are marked as "mono" samples, but are panned to different 
					// locations. This is a pseudo-stero pair...
					if (z2->pan < 0)
					{
						z2->chan = 1; // left channel
						zone->chan = 0;
					}
					else
					{
						z2->chan = 0;
						zone->chan = 1;
					}
					break;
				}
			}
			zone->mode = genVals[sfgSampleModes];
			zone->tableStart = genVals[sfgStartAddrsOffset] 
			                 + (genVals[sfgStartAddrsCoarseOffset]*32768);
			zone->tableEnd = (sh->dwEnd - sh->dwStart)
				           + genVals[sfgEndAddrsOffset]
			               + (genVals[sfgEndAddrsCoarseOffset] * 32768)
						   + zone->tableStart;
			zone->loopStart = (sh->dwStartloop - sh->dwStart)
				            + genVals[sfgStartloopAddrsOffset]
							+ (genVals[sfgStartloopAddrsCoarseOffset] * 32768)
						    + zone->tableStart;
			zone->loopEnd = (sh->dwEndloop - sh->dwStart)
				          + genVals[sfgEndloopAddrsOffset]
						  + (genVals[sfgEndloopAddrsCoarseOffset] * 32768)
						  + zone->tableStart;

			samp = CreateSample(sh, genVals[sfgSampleID]);
			if (samp->sample)
				zone->sample = &samp->sample[zone->tableStart];
			else
				zone->sample = 0;
			zone->sampleNdx = samp->index;
			zone->volEg.delay = SFEnvRate(genVals[sfgDelayVolEnv]);
			zone->volEg.attack = SFEnvRate(genVals[sfgAttackVolEnv]);
			zone->volEg.hold = SFEnvRate(genVals[sfgHoldVolEnv]);
			zone->volEg.decay = SFEnvRate(genVals[sfgDecayVolEnv]);
			zone->volEg.release = SFEnvRate(genVals[sfgReleaseVolEnv]);
			zone->volEg.sustain = SFEnvLevel(-genVals[sfgSustainVolEnv]);
			zone->modEg.delay = SFEnvRate(genVals[sfgDelayModEnv]);
			zone->modEg.attack = SFEnvRate(genVals[sfgAttackModEnv]);
			zone->modEg.hold = SFEnvRate(genVals[sfgHoldModEnv]);
			zone->modEg.decay = SFEnvRate(genVals[sfgDecayModEnv]);
			zone->modEg.release = SFEnvRate(genVals[sfgReleaseModEnv]);
			zone->modEg.sustain = SFEnvLevel(-genVals[sfgSustainModEnv]);
			zone->vibAmount = SFRelCents(genVals[sfgVibLfoToPitch]);
			zone->vibRate = SFAbsCents(genVals[sfgFreqVibLFO]);
			zone->vibDelay = SFEnvRate(genVals[sfgDelayVibLFO]);
			if (genVals[sfgKeynumToVolEnvDecay])
			{
				modval = (double) genVals[sfgKeynumToVolEnvDecay];
				for (int k = zone->lowKey; k <= zone->highKey; k++)
					in->keyDecRate[k] = SFEnvRate(((60.0 - (double)k) * modval));
			}
			if (genVals[sfgKeynumToVolEnvHold])
			{
				modval = (double) genVals[sfgKeynumToVolEnvHold];
				for (int k = zone->lowKey; k <= zone->highKey; k++)
					in->keyHoldRate[k] = SFEnvRate(((60.0 - (double)k) * modval));
			}
			genNdx1 = ibag[bagNdx1].wModNdx;
			genNdx2 = ibag[bagNdx1+1].wModNdx;
			while (genNdx1 < genNdx2)
			{
				//ApplyModulator(&imod[genNdx1], instr);
				sfModList *mp = &imod[genNdx1];
				SBModInfo *mi = zone->AddModInfo();
				mi->srcOp = mp->sfModSrcOper;
				mi->dstOp = mp->sfModDestOper;
				mi->value = mp->modAmount;
				mi->srcAmntOp = mp->sfModAmtSrcOper;
				mi->transOp = mp->sfModTransOper;
				genNdx1++;
			}
		}
		else // global zone
			memcpy(globVals, genVals, sizeof(globVals));

		bagNdx1++;
	}
}

void SFFile::BuildPreset(int n)
{
	char nameTemp[24];
	short bagNdx1, bagNdx2;
	short genNdx1, genNdx2;

	SBInstr *instr = sfbnk->AddInstr(phdr[n].wBank, phdr[n].wPreset);
	if (instr == 0)
		return;
	instr->instrNdx = phdr[n].wPreset;

	// Preset level modifiers are additive so we init them to 0
	// N.B.: valid PHDR vales are all additive, except - 
	// key and velocity range generators can occur
	// in the PGEN record, but do not add to the IGEN record!
	memset(&hdrVals, 0, sizeof(hdrVals));
	hdrVals[sfgVelRange] = 127 << 8;
	hdrVals[sfgKeyRange] = 127 << 8;

	instr->instrName = CopyName(nameTemp, phdr[n].achPresetName);

	bagNdx1 = phdr[n].wPresetBagNdx;
	bagNdx2 = phdr[n+1].wPresetBagNdx;
	int instrNdx = 0;
	while (bagNdx1 < bagNdx2)
	{
		genNdx1 = pbag[bagNdx1].wGenNdx;
		genNdx2 = pbag[bagNdx1+1].wGenNdx;
		while (genNdx1 < genNdx2)
		{
			sfGenList *pg = &pgen[genNdx1];
			if (AddHeaderGen(pg->sfGenOper)
			 || pg->sfGenOper == sfgKeyRange
			 || pg->sfGenOper == sfgVelRange)
			{
				hdrVals[pg->sfGenOper] = pg->genAmount.shAmount;
			}
			if (pg->sfGenOper == sfgInstrument)
			{
				BuildInstrument(instr, pg->genAmount.shAmount);
				// anything after this record should be ignored
				//break;
			}
			genNdx1++;
		}

		genNdx1 = pbag[bagNdx1].wModNdx;
		genNdx2 = pbag[bagNdx1+1].wModNdx;
		while (genNdx1 < genNdx2)
		{
			//ApplyModulator(&imod[genNdx1], instr);
			sfModList *ml = &pmod[genNdx1];
			SBModInfo *mi = instr->AddModInfo();
			mi->srcOp = ml->sfModSrcOper;
			mi->dstOp = ml->sfModDestOper;
			mi->value = ml->modAmount;
			mi->srcAmntOp = ml->sfModAmtSrcOper;
			mi->transOp = ml->sfModTransOper;
			genNdx1++;
		}

		bagNdx1++;
	}
	instr->loaded = preload;
	instr->InitZoneMap();
}

void SFFile::BuildSoundBank()
{
	int n, m;

	m = npresets-1;
	for (n = 0; n < m; n++)
		BuildPreset(n);
}

FrqValue SFFile::SFEnvRate(short rt)
{
	if (rt <= -12000)
		return 0.0;
	return (FrqValue) pow(2.0, (double) rt / 1200.0);
}

AmpValue SFFile::SFEnvLevel(short amt)
{
	if (amt >= 960)
		return 0.0;
	return (AmpValue) pow(10.0, (double) amt / 200.0);
}

AmpValue SFFile::SFAttenuation(short amt)
{
	if (amt >= 960)
		return 0.0;
	return (AmpValue) pow(10.0, ((float) amt * atnScl)/ 200.0);
}

FrqValue SFFile::SFTimeCents(short val)
{
	if (val <= -1200)
		return 0.5;
	if (val >= 1200)
		return 1.0;
	return (FrqValue) pow(2.0, (double) val / 1200.0);
}

FrqValue SFFile::SFKeyScaleTime(short key, short amt)
{
	return SFTimeCents((60 - key) * amt);
}

FrqValue SFFile::SFAbsCents(short amt)
{
	return 8.197 * SFRelCents(amt);
}

FrqValue SFFile::SFRelCents(short amt)
{
	if (amt == 0)
		return 1.0;
	return pow(2, (double) amt / 12000);
}

char *SFFile::CopyName(char *dst, char *src)
{
	memcpy(dst, src, 20);
	dst[20] = '\0';
	for (int ndx = 19; ndx >= 0 && dst[ndx] == ' '; ndx--)
		dst[ndx] = '\0';
	return dst;
}


void SFFile::InitGenVals(short *genVals)
{
	memset(genVals, 0, sizeof(short)*sfgEndOper);
	genVals[sfgVelRange] = 127 << 8;
	genVals[sfgKeyRange] = 127 << 8;
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

	genVals[sfgDelayModEnv] = -12000;
	genVals[sfgAttackModEnv] = -12000;
	genVals[sfgHoldModEnv] = -12000;
	genVals[sfgDecayModEnv] = -12000;
	genVals[sfgSustainModEnv] = 0;
	genVals[sfgReleaseModEnv] = -12000;
}