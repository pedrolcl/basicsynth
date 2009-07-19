/////////////////////////////////////////////////////////////////////////
// BasicSynth - Example 4a (Chapter 8)
//
// Soundfont player
//
// use: example4a soundfont -bbank -ppreset -m -d(1|2|3)
// dump 1 = print internal representation
// dump 2 = print raw file contents
// dump 3 = print both
/////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SynthDefs.h"
#include "SynthList.h"
#include "WaveTable.h"
#include "WaveFile.h"
#include "EnvGenSeg.h"
#include "GenWaveWT.h"
#include "Mixer.h"
#include "SFFile.h"
#include "SFGen.h"

char *opNames[] = 
{
"0 startAddrsOffset",
"1 endAddrsOffset",
"2 startloopAddrsOffset",
"3 endloopAddrsOffset",
"4 startAddrsCoarseOffset",
"5 modLfoToPitch",
"6 vibLfoToPitch",
"7 modEnvToPitch",
"8 initialFilterFc",
"9 initialFilterQ",
"10 modLfoToFilterFc",
"11 modEnvToFilterFc",
"12 endAddrsCoarseOffset",
"13 modLfoToVolume",
"14 unused1",
"15 chorusEffectsSend ",
"16 reverbEffectsSend",
"17 pan",
"18 unused2",
"19 unused3",
"20 unused4",
"21 delayModLFO",
"22 freqModLFO",
"23 delayVibLFO",
"24 freqVibLFO",
"25 delayModEnv",
"26 attackModEnv",
"27 holdModEnv",
"28 decayModEnv",
"29 sustainModEnv",
"30 releaseModEnv",
"31 keynumToModEnvHold",
"32 keynumToModEnvDecay",
"33 delayVolEnv",
"34 attackVolEnv",
"35 holdVolEnv",
"36 decayVolEnv",
"37 sustainVolEnv",
"38 releaseVolEnv",
"39 keynumToVolEnvHold",
"40 keynumToVolEnvDecay",
"41 instrument",
"42 reserved1",
"43 keyRange",
"44 velRange",
"45 startloopAddrsCoarseOffset",
"46 keynum ",
"47 velocity",
"48 initialAttenuation",
"49 reserved2",
"50 endloopAddrsCoarseOffset",
"51 coarseTune",
"52 fineTune",
"53 sampleID",
"54 sampleModes",
"55 reserved3",
"56 scaleTuning",
"57 exclusiveClass",
"58 overridingRootKey",
"59 unused5",
"60 endOper"
};

static char opUndef[80];
static const char *OpName(short ndx)
{
	if (ndx < 0 || ndx > 60)
	{
		sprintf_s(opUndef, 80, "%d undefined", ndx);
		return opUndef;
	}
	return opNames[ndx];
}

char opAmntBuf[40];
const char *OpAmount(sfGenList& gen)
{
	switch (gen.sfGenOper)
	{
	case sfgKeyRange:
	case sfgVelRange:
		sprintf_s(opAmntBuf, "{%d,%d}", gen.genAmount.ranges.byLo, gen.genAmount.ranges.byHi);
		break;
	default:
		sprintf_s(opAmntBuf, "%d", gen.genAmount.shAmount);
		break;
	}
	return opAmntBuf;
}

/*FrqValue EnvRate(short rt)
{
	if (rt < -12000)
		return 0.0;
	return (FrqValue) pow(2.0, (double) rt / 1200.0);
}

AmpValue EnvLevel(short lvl)
{
	if (lvl >= 960)
		return 0.0;
	return (AmpValue) pow(10.0, (double) lvl / -200.0);
}*/


void Dump(SFZone *zp)
{
	if (zp->instZone)
	{
		printf("  Zone %s\n", (const char *)zp->name);
		printf("   Range: {%d,%d} {%d,%d}\n", zp->lowKey, zp->highKey, zp->lowVel, zp->highVel);
		printf("   Envelope: A=%d H=%d D=%d S=%d R=%d\n", zp->genVals[sfgAttackVolEnv], 
			zp->genVals[sfgHoldVolEnv], zp->genVals[sfgDecayVolEnv],
			zp->genVals[sfgSustainVolEnv], zp->genVals[sfgReleaseVolEnv]);
		printf("           : A=%f H=%f D=%f S=%f R=%f\n", 
			SFEnvRate(zp->genVals[sfgAttackVolEnv]),
			SFEnvRate(zp->genVals[sfgHoldVolEnv]),
			SFEnvRate(zp->genVals[sfgDecayVolEnv]),
			SFEnvLevel(zp->genVals[sfgSustainVolEnv]),
			SFEnvRate(zp->genVals[sfgReleaseVolEnv]));
		printf("   Sample sr=%5.1f rf=%5.3f chan=%d key=%d cents=%d loop{%d,%d}, pan=%1.3f mode=%d\n",
			zp->rate, zp->recFreq, zp->chan, zp->keyNum, zp->cents,
			zp->loopStart, zp->loopEnd, zp->pan, zp->mode);
	}
}

void Dump(SFInstr *ip)
{
	printf(" Instrument '%s'\n", (const char*)ip->instrName);
	SFZone *z = ip->zoneHead.next;
	int n = 0;
	while (z != &ip->zoneTail)
	{
		Dump(z);
		z = z->next;
	}
}
void Dump(SFPreset *pp)
{
	printf("Preset '%s' on bank %d, index %d\n", (const char *)pp->presetName, pp->presetBnk, pp->presetNdx);
	for (int n = 0; n < pp->instrCount; n++)
	{
		if (pp->instrList[n])
			Dump(pp->instrList[n]);
	}
}

void Dump(SFSoundBank *sb)
{
	for (int bnkNo = 0; bnkNo < 129; bnkNo++)
	{
		if (sb->presetBank[bnkNo] != 0)
		{
			for (int preNo = 0; preNo < 128; preNo++)
			{
				SFPreset *p = sb->presetBank[bnkNo][preNo];
				if (p)
					Dump(p);
			}
		}
	}
}

void Dump(SFFile *sf)
{
	char nameTemp[24];
	short bagNdx1, bagNdx2;
	short genNdx1, genNdx2;
	short modNdx1, modNdx2;
	short ibagNdx1, ibagNdx2;
	short igenNdx1, igenNdx2;
	short imodNdx1, imodNdx2;
	int n;

	if (sf->sfbnk)
	{
		printf("File version: %d.%d\n", sf->sfbnk->info.wMajorFile, sf->sfbnk->info.wMinorFile);
		printf("Version     : %d.%d\n", sf->sfbnk->info.wMajorVer, sf->sfbnk->info.wMinorVer);
		printf("Sound Engine: %s\n", sf->sfbnk->info.szSoundEngine);
		printf("ROM         : %s\n", sf->sfbnk->info.szROM);
		printf("Name        : %s\n", sf->sfbnk->info.szName);
		printf("Date        : %s\n", sf->sfbnk->info.szDate);
		printf("Engineer    : %s\n", sf->sfbnk->info.szEng);
		printf("Product     : %s\n", sf->sfbnk->info.szProduct);
		printf("Copyright   : %s\n", sf->sfbnk->info.szCopyright);
		printf("Comment     : %s\n", sf->sfbnk->info.szComment);
		printf("Tools       : %s\n", sf->sfbnk->info.szTools);
	}

	int m = sf->npresets-1;
	for (n = 0; n < m; n++)
	{
		memcpy(nameTemp, sf->phdr[n].achPresetName, 20);
		nameTemp[20] = 0;
		bagNdx1 = sf->phdr[n].wPresetBagNdx;
		bagNdx2 = sf->phdr[n+1].wPresetBagNdx;
		printf("PHDR %d: '%-20s' Bank %d, Preset %d, bags: %d -> %d\n", n, nameTemp, sf->phdr[n].wBank, sf->phdr[n].wPreset, bagNdx1, bagNdx2);
		while (bagNdx1 < bagNdx2)
		{
			genNdx1 = sf->pbag[bagNdx1].wGenNdx;
			modNdx1 = sf->pbag[bagNdx1].wModNdx;
			genNdx2 = sf->pbag[bagNdx1+1].wGenNdx;
			modNdx2 = sf->pbag[bagNdx1+1].wModNdx;
			printf("  PBAG %d: GEN = %d:%d MOD = %d:%d\n", bagNdx1, genNdx1, genNdx2, modNdx1, modNdx2);
			while (modNdx1 < modNdx2)
			{
				printf("    PMOD %d: srcOp[%d] = %s dstOp[%d] = %s amnt = %d\n", modNdx1, 
					sf->pmod[modNdx1].sfModSrcOper, OpName(sf->pmod[modNdx1].sfModSrcOper), 
					sf->pmod[modNdx1].sfModDestOper, OpName(sf->pmod[modNdx1].sfModDestOper),
					(int)sf->pmod[modNdx1].modAmount);
				modNdx1++;
			}
			while (genNdx1 < genNdx2)
			{
				printf("    PGEN %d: genOp[%d] = %s amnt = %d\n", genNdx1, 
					sf->pgen[genNdx1].sfGenOper, OpName(sf->pgen[genNdx1].sfGenOper),
					sf->pgen[genNdx1].genAmount.shAmount);
				if (sf->pgen[genNdx1].sfGenOper == sfgInstrument)
				{
					int nn = sf->pgen[genNdx1].genAmount.wAmount;
					memcpy(nameTemp, sf->inst[nn].achInstName, 20);
					nameTemp[20] = 0;
					ibagNdx1 = sf->inst[nn].wInstBagNdx;
					ibagNdx2 = sf->inst[nn+1].wInstBagNdx;
					printf("INST %d: '%-20s' bags: %d -> %d\n", nn, nameTemp, ibagNdx1, ibagNdx2);
					while (ibagNdx1 < ibagNdx2)
					{
						igenNdx1 = sf->ibag[ibagNdx1].wGenNdx;
						igenNdx2 = sf->ibag[ibagNdx1+1].wGenNdx;
						imodNdx1 = sf->ibag[ibagNdx1].wModNdx;
						imodNdx2 = sf->ibag[ibagNdx1+1].wModNdx;
						printf("  IBAG %d: GEN = %d:%d MOD = %d:%d\n", ibagNdx1, igenNdx1, igenNdx2, imodNdx1, imodNdx2);
						while (imodNdx1 < imodNdx2)
						{
							printf("    IMOD %d: srcOp[%d] = %s dstOp[%d] = %s amnt = %d\n", imodNdx1, 
								sf->imod[imodNdx1].sfModSrcOper, OpName(sf->imod[imodNdx1].sfModSrcOper),
								sf->imod[imodNdx1].sfModDestOper, OpName(sf->imod[imodNdx1].sfModDestOper),
								sf->imod[imodNdx1].modAmount);
							imodNdx1++;
						}
						while (igenNdx1 < igenNdx2)
						{
							printf("    IGEN %d: genOp[%d] = %s amnt = %s\n", igenNdx1, 
								sf->igen[igenNdx1].sfGenOper, OpName(sf->igen[igenNdx1].sfGenOper), 
								OpAmount(sf->igen[igenNdx1]));
							if (sf->igen[igenNdx1].sfGenOper == 53) // sampleID)
							{
								int id = sf->igen[igenNdx1].genAmount.wAmount;
								memcpy(nameTemp, sf->shdr[id].achSampleName, 20);
								printf("      SAMP: %d %s st=%d end=%d ls=%d le=%d sr=%d k=%d ty=%d lnk=%d\n", 
									id, nameTemp,
									sf->shdr[id].dwStart, sf->shdr[id].dwEnd, 
									sf->shdr[id].dwStartloop, sf->shdr[id].dwEndloop, 
									sf->shdr[id].dwSampleRate, sf->shdr[id].byOriginalKey,
									sf->shdr[id].sfSampleType, sf->shdr[id].wSampleLink);
							}
							igenNdx1++;
						}
						ibagNdx1++;
					}
				}
				genNdx1++;
			}
			bagNdx1++;
		}
	}
}

class SFPlayer
{
private:
	GenWaveSF oscl;
	GenWaveSF oscr;
	EnvGenADSR eg;
	Panner panr;
	Panner panl;
	SFZone *zoner;
	SFZone *zonel;
	SFInstr *instr;
	int pitch;
	int mono;
	FrqValue frq;
public:

	SFPlayer()
	{
		zoner = 0;
		zonel = 0;
		instr = 0;
		pitch = 57;
		frq = 440;
		mono = 0; // force mono mode
	}

	void Init(int pit, SFInstr *in, int m = 0)
	{
		pitch = pit;
		frq = synthParams.GetFrequency(pit); 
		instr = in;
		mono = m;
	}

	void Start()
	{
		zoner = 0;
		zonel = 0;

		int sfpit = pitch+12; // shift from BasicSynth to MIDI range
		if (instr)
		{
			zoner = instr->GetZone(sfpit, 60, 0);
			zonel = instr->GetZone(sfpit, 60, 1);
		}
		//fprintf(stderr, "Pitch %d(%d)\n", pitch, sfpit);
		//if (!zoner && !zonel)
		//	fprintf(stderr, "  No zones.\n");

		if (zoner)
		{
			eg.SetAtkRt(SFEnvRate(zoner->genVals[sfgAttackVolEnv]));
			//volEnv.SetAtkLvl(EnvLevel(zoner->genVals[]));
			eg.SetAtkLvl(1.0);
			eg.SetDecRt(SFEnvRate(zoner->genVals[sfgDecayVolEnv]));
			eg.SetSusLvl(SFEnvLevel(zoner->genVals[sfgSustainVolEnv]));
			eg.SetRelRt(SFEnvRate(zoner->genVals[sfgReleaseVolEnv]));
			eg.Reset(0);
			oscr.InitSF(frq, zoner);
			panr.Set(panSqr, zoner->pan);
			//fprintf(stderr, "  Zone: mode %d, {%d,%d}, %d, %4.3f sample %s\n", zoner->mode, 
			//	zoner->lowKey, zoner->highKey, zoner->keyNum, zoner->pan, (const char *)zoner->name);
		}
		else
			eg.InitADSR(0.0, 0.01, 1.0, 0.01, 0.95, 0.1, 0.0, linSeg, 1);

		if (zonel)
		{
			oscl.InitSF(frq, zonel);
			panl.Set(panSqr, zonel->pan);
			//fprintf(stderr, "  Zone: mode %d, {%d,%d}, %d, %4.3f sample %s\n", zonel->mode,
			//	zonel->lowKey, zonel->highKey, zonel->keyNum, zonel->pan, (const char *)zonel->name);
		}
	}

	void Stop()
	{
		eg.Release();
		if (zoner)
			oscr.Release();
		if (zonel)
			oscl.Release();
	}

	int IsFinished()
	{
		return eg.IsFinished() || (zoner && oscr.IsFinished());
	}

	void Tick(Mixer *mix)
	{
		AmpValue oscValr = 0;
		AmpValue oscVall = 0;
		AmpValue ampVal = eg.Gen();
		if (zoner)
			oscValr = ampVal * oscr.Gen();
		if (zonel)
			oscVall = ampVal * oscl.Gen();
		if (mono)
			mix->ChannelIn(0, (oscValr + oscVall) / 2.0);
		else
		{
			mix->ChannelIn2(0, 
				(oscVall * panl.panlft) + (oscValr * panr.panlft),
				(oscVall * panl.panrgt) + (oscValr * panr.panrgt));
		}
	}
};

Mixer mix;
char *wavePrefix = "";

void PlayPreset(SFSoundBank *bnk, int bnkNum, int preNum, int mono)
{
	SFPreset *pre = bnk->GetPreset(bnkNum, preNum);
	if (pre == 0)
	{
		fprintf(stderr, "Bank %d and preset %d does not exist\n", bnkNum, preNum);
		return;
	}

	fprintf(stderr, "Playing bank %d, preset %d : %s\n", bnkNum, preNum, (const char *)pre->presetName);

	WaveFile wf;
	char waveFile[1024];
	strcpy(waveFile, wavePrefix);
	strcat(waveFile, pre->presetName);
	strcat(waveFile, ".wav");
	if (wf.OpenWaveFile(waveFile, 2))
	{
		fprintf(stderr, "Cannot open file '%s'\n", (const char *)waveFile);
		return;
	}

	long silence = synthParams.isampleRate / 10;
	long dur = synthParams.isampleRate / 4;
	long t;
	AmpValue left, right;
	SFPlayer play;
	SFInstr *instr;

	// loop over all instruments and all zones
	instr = 0;
	int pit;
	for (int inum = 0; inum < pre->instrCount; inum++)
	{
		if ((instr = pre->GetInstr(inum)) == 0)
			continue;

		SFZone *zone = 0;
		while ((zone = instr->EnumZones(zone)) != 0)
		{
			if (zone->chan == 1) // left channel of stereo sound?
				continue;
			if (zone->keyNum == -1)
				continue;
			fprintf(stderr, "  zone %s, %d [%d,%d]\n", (const char *)zone->name, zone->keyNum, zone->lowKey, zone->highKey);
			// THere are soundfont files out there where the key number is not
			// within the range of [low,high] (believe it or not)...
			if (zone->keyNum < zone->lowKey || zone->keyNum > zone->highKey)
				pit = (zone->highKey + zone->lowKey) / 2;
			else
				pit = zone->keyNum;
			play.Init(pit-12, instr, mono);
			play.Start();
			for (t = 0; t < dur; t++)
			{
				play.Tick(&mix);
				mix.Out(&left, &right);
				wf.Output2(left, right);
			}
			play.Stop();
			while (!play.IsFinished())
			{
				play.Tick(&mix);
				mix.Out(&left, &right);
				wf.Output2(left, right);
			}
		}
		for (t = 0; t < dur; t++)
			wf.Output2(0.0, 0.0);
		int pitches[10] = { 36, 43, 48, 50, 52, 53, 55, 57, 59, 60 };
		for (pit = 0; pit < 10; pit++)
		{
			play.Init(pitches[pit], instr, mono);
			play.Start();
			for (t = 0; t < dur; t++)
			{
				play.Tick(&mix);
				mix.Out(&left, &right);
				wf.Output2(left, right);
			}
			play.Stop();
			while (!play.IsFinished())
			{
				play.Tick(&mix);
				mix.Out(&left, &right);
				wf.Output2(left, right);
			}
		}
	}
	wf.CloseWaveFile();
}

void usage()
{
	fprintf(stderr, "use: example4a soundfont -bbank -ppreset -m -d(1|2|3)\n");
	fprintf(stderr, "     -d1 = print internal representation\n");
	fprintf(stderr, "     -d2 = print raw file contents\n");
	fprintf(stderr, "     -d3 = print both\n");
	exit(1);
}

int main(int argc, char *argv[])
{

	if (argc < 2)
		usage();

	int doDump = 0;
	char *fileName = argv[1];
	char *waveFile = 0;
	int bnkNum = -1;
	int preNum = -1;
	int mono = 0;

	int argn;
	for (argn = 2; argn < argc; argn++)
	{
		char *ap = argv[argn];
		if (*ap++ != '-')
			usage();
		switch (*ap++)
		{
		case 'b':
			bnkNum = atoi(ap);
			break;
		case 'p':
			preNum = atoi(ap);
			break;
		case 'd':
			if (*ap)
				doDump = atoi(ap);
			else
				doDump = 1;
			break;
		case 'w':
			wavePrefix = ap;
			break;
		case 'm':
			mono = 1;
			break;
		default:
			fprintf(stderr, "Invalid argument: %s\n", argv[argn]);
			usage();
			break;
		}
	}

	InitSynthesizer();

	SFFile sounds;
	SFSoundBank *bnk = sounds.LoadSoundBank(fileName, doDump & 2);
	if (doDump)
	{
		if (doDump & 2)
			Dump(&sounds);
		if (doDump & 1)
			Dump(bnk);
		exit(0);
	}

	mix.MasterVolume(1.0, 1.0);
	mix.SetChannels(1);
	mix.ChannelVolume(0, 1.0);
	mix.ChannelPan(0, 0, panTrig);
	mix.ChannelOn(0, 1);

	if (bnkNum == -1)
	{
		for (bnkNum = 0; bnkNum <= 128; bnkNum++)
		{
			if (preNum == -1)
			{
				for (preNum = 0; preNum < 128; preNum++)
					PlayPreset(bnk, bnkNum, preNum, mono);
			}
			else
				PlayPreset(bnk, bnkNum, preNum, mono);
		}
	}
	else
	{
		if (preNum == -1)
		{
			for (preNum = 0; preNum < 128; preNum++)
				PlayPreset(bnk, bnkNum, preNum, mono);
		}
		else
			PlayPreset(bnk, bnkNum, preNum, mono);
	}

	return 0;
}
