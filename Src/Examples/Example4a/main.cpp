/////////////////////////////////////////////////////////////////////////
// BasicSynth - Example 4a (Chapter 8)
//
// Soundfont player
//
// use: example4a soundfont -bbank -ppreset -m -d(1|2|3) -wprefix -n[0|1] -l[0|1]
// -d1 = print internal representation
// -d2 = print raw file contents
// -d3 = print both
// -m  = mono
// -n  = normalize
// -l  = preload all sounds
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
#include "DLSFile.h"
#include "SFGen.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

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
		snprintf(opUndef, 80, "%d undefined", ndx);
		return opUndef;
	}
	return opNames[ndx];
}

static char modUndef[80];
static const char *ModName(short ndx)
{
	if (ndx & 0x80) 
	{
		snprintf(modUndef, 80, "CC# %d", ndx & 0x7f);
		return modUndef;
	}
	else
	{
		switch (ndx & 0x7f)
		{
		case 0:
			return "No Controller";
		case 2:
			return "Note-on velocity";
		case 3:
			return "Note-on key";
		case 10:
			return "Poly Pressure";
		case 13:
			return "Channel Pressure";
		case 14:
			return "Pitch Wheel";
		case 16:
			return "Pitch Wheel Sens.";
		case 127:
			return "Link";
		}
	}
	snprintf(modUndef, 80, "Mod %04x", ndx);
	return modUndef;
}

char opAmntBuf[40];
const char *OpAmount(sfGenList& gen)
{
	switch (gen.sfGenOper)
	{
	case sfgKeyRange:
	case sfgVelRange:
		snprintf(opAmntBuf, 40, "{%d,%d}", (int)gen.genAmount.ranges.byLo, (int)gen.genAmount.ranges.byHi);
		break;
	default:
		snprintf(opAmntBuf, 40, "%d", (int)gen.genAmount.shAmount);
		break;
	}
	return opAmntBuf;
}

FrqValue FrqCents(FrqValue pc)
{
	return pow(2.0, pc / 1200.0);
}

void Dump(SBZone *zp)
{
	printf("  Zone %s\n", (const char *)zp->name);
	printf("   Range: {%d,%d} {%d,%d}\n", zp->lowKey, zp->highKey, zp->lowVel, zp->highVel);
	printf("   Vol EG: A=%6.3f H=%6.3f D=%6.3f S=%6.3f R=%6.3f V=%6.3f\n",
		zp->volEg.attack, zp->volEg.hold, zp->volEg.decay, 
		zp->volEg.sustain, zp->volEg.release, zp->volAtten);
	printf("         : A=%6.3f H=%6.3f D=%6.3f S=%6.3f R=%6.3f V=%6.3f\n",
		SoundBank::EnvRate(zp->volEg.attack),
		SoundBank::EnvRate(zp->volEg.hold),
		SoundBank::EnvRate(zp->volEg.decay), 
		zp->volEg.sustain, 
		SoundBank::EnvRate(zp->volEg.release),
		SoundBank::Attenuation(zp->volAtten));
	printf("   Mod EG: A=%6.3f H=%6.3f D=%6.3f S=%6.3f R=%6.3f F=%6.3f\n",
		zp->modEg.attack, zp->modEg.hold, zp->modEg.decay, 
		zp->modEg.sustain, zp->modEg.release, zp->modEnvFrq);
	printf("         : A=%6.3f H=%6.3f D=%6.3f S=%6.3f R=%6.3f\n",
		SoundBank::EnvRate(zp->modEg.attack),
		SoundBank::EnvRate(zp->modEg.hold),
		SoundBank::EnvRate(zp->modEg.decay), 
		zp->modEg.sustain, 
		SoundBank::EnvRate(zp->modEg.release));
	printf("   Vib LFO: F=%6.3f D=%6.3f F=%6.3f\n", zp->vibLfo.rate, zp->vibLfo.delay, zp->vibLfoFrq);
	printf("   Mod LFO: F=%6.3f D=%6.3f F=%6.3f A=%6.3f\n", zp->modLfo.rate, zp->modLfo.delay, zp->modLfoFrq, zp->modLfoVol);
	printf("   Sample sr=%5.1f rf=%5.3f chan=%d key=%d cents=%d loop{%d,%d}, end=%d pan=%1.3f mode=%d\n",
		zp->rate, zp->recFreq, zp->chan, zp->keyNum, zp->cents,
		zp->loopStart, zp->loopEnd, zp->tableEnd, zp->pan, zp->mode);
	SBModInfo *mi = 0;
	while ((mi = zp->EnumModInfo(mi)) != 0)
	{
		printf("   Mod: src=%s dst=%s trn=%d sa=%d val=%d\n", 
			ModName(mi->srcOp), OpName(mi->dstOp),
			mi->transOp, mi->srcAmntOp, mi->value);
	}
	printf("   data 0x%08x %d\n", zp->sample, zp->sampleLen);
}

void Dump(SBInstr *ip)
{
	printf(" Instrument '%s' bank=%d prog=%d\n", (const char*)ip->instrName, ip->bank, ip->prog);
	SBModInfo *mi = 0;
	while ((mi = ip->EnumModInfo(mi)) != 0)
	{
		printf("  Mod: src=%s dst=%s trn=%d sa=%d val=%d\n", 
			ModName(mi->srcOp), OpName(mi->dstOp),
			mi->transOp, mi->srcAmntOp, mi->value);
	}
	SBZone *z = 0;
	while ((z = ip->EnumZones(z)) != 0)
		Dump(z);

}

void Dump(SoundBank *sb)
{
	printf("Name '%s'\n", (const char *)sb->info.szName);
	printf("Copy '%s'\n", (const char *)sb->info.szCopyright);
	printf("Cmnt '%s'\n", (const char *)sb->info.szComment);
	printf("Date '%s'\n", (const char *)sb->info.szDate);
	printf("Vers %d.%d %d.%d\n", 
		sb->info.wMajorFile, sb->info.wMinorFile,
		sb->info.wMajorVer, sb->info.wMinorVer);

	for (int bnkNo = 0; bnkNo < 129; bnkNo++)
	{
		if (sb->instrBank[bnkNo] != 0)
		{
			for (int preNo = 0; preNo < 128; preNo++)
			{
				SBInstr *p = sb->instrBank[bnkNo][preNo];
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
		printf("Sound Engine: %s\n", (const char *)sf->sfbnk->info.szSoundEngine);
		printf("Name        : %s\n", (const char *)sf->sfbnk->info.szName);
		printf("Date        : %s\n", (const char *)sf->sfbnk->info.szDate);
		printf("Engineer    : %s\n", (const char *)sf->sfbnk->info.szEng);
		printf("Product     : %s\n", (const char *)sf->sfbnk->info.szProduct);
		printf("Copyright   : %s\n", (const char *)sf->sfbnk->info.szCopyright);
		printf("Comment     : %s\n", (const char *)sf->sfbnk->info.szComment);
		printf("Tools       : %s\n", (const char *)sf->sfbnk->info.szTools);
	}

	printf("Samples     : total=%d offs1=%u offs2=%u\n", sf->sampleSize, sf->sampleFileOffs1, sf->sampleFileOffs2);

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
			printf(" PBAG %d: GEN = %d:%d MOD = %d:%d\n", bagNdx1, genNdx1, genNdx2, modNdx1, modNdx2);
			while (modNdx1 < modNdx2)
			{
				printf("  PMOD %d: srcOp[%d] = %s dstOp[%d] = %s amnt = %d\n", modNdx1, 
					sf->pmod[modNdx1].sfModSrcOper, ModName(sf->pmod[modNdx1].sfModSrcOper), 
					sf->pmod[modNdx1].sfModDestOper, OpName(sf->pmod[modNdx1].sfModDestOper),
					(int)sf->pmod[modNdx1].modAmount);
				modNdx1++;
			}
			while (genNdx1 < genNdx2)
			{
				printf("  PGEN %d: genOp[%d] = %s amnt = %d\n", genNdx1, 
					sf->pgen[genNdx1].sfGenOper, OpName(sf->pgen[genNdx1].sfGenOper),
					sf->pgen[genNdx1].genAmount.shAmount);
				if (sf->pgen[genNdx1].sfGenOper == sfgInstrument)
				{
					int nn = sf->pgen[genNdx1].genAmount.wAmount;
					memcpy(nameTemp, sf->inst[nn].achInstName, 20);
					nameTemp[20] = 0;
					ibagNdx1 = sf->inst[nn].wInstBagNdx;
					ibagNdx2 = sf->inst[nn+1].wInstBagNdx;
					printf("  INST %d: '%-20s' bags: %d -> %d\n", nn, nameTemp, ibagNdx1, ibagNdx2);
					while (ibagNdx1 < ibagNdx2)
					{
						igenNdx1 = sf->ibag[ibagNdx1].wGenNdx;
						igenNdx2 = sf->ibag[ibagNdx1+1].wGenNdx;
						imodNdx1 = sf->ibag[ibagNdx1].wModNdx;
						imodNdx2 = sf->ibag[ibagNdx1+1].wModNdx;
						printf("   IBAG %d: GEN = %d:%d MOD = %d:%d\n", ibagNdx1, igenNdx1, igenNdx2, imodNdx1, imodNdx2);
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
								printf("     SAMP: %d %s st=%d end=%d ls=%d le=%d sr=%d k=%d ty=%d lnk=%d\n", 
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

FrqValue DLSTimeCents(bsInt32 tc)
{
	return pow(2.0, ((double)tc / (1200.0*65536.0)));
}

AmpValue DLSRelGain(bsInt32 cb)
{
	double x = (double) cb / 65536.0;
	if (x >= 960.0)
		return 0.0;
	return pow(10.0, x / -200.0);
}

FrqValue DLSPitchCents(bsInt32 pc)
{
	return 440.0 * pow(2.0, (((double)pc / 65536.0) - 6900.0) / 1200.0);
}

void Dump(DLSArtInfo *art)
{
	printf("    Artic:\n");
	for (bsUint32 n = 0; n < art->connections; n++)
	{
		dlsConnection *blk = &art->info[n];
		printf("      blk(%d) ctl=%d src=%x dst=%x xf=%x scl=%d\n              ", n,
			blk->control, blk->source, 
			blk->destination, blk->transform, blk->scale);
		switch (blk->source)
		{
		case CONN_SRC_NONE:
			printf("SRC_NONE ");
			break;
		case CONN_SRC_LFO:
			printf("SRC_LFO ");
			break;
		case CONN_SRC_KEYONVELOCITY:
			printf("SRC_KEYONVELOCITY ");
			break;
		case CONN_SRC_KEYNUMBER:
			printf("SRC_KEYNUMBER ");
			break;
		case CONN_SRC_EG1:
			printf("SRC_EG1 ");
			break;
		case CONN_SRC_EG2:
			printf("SRC_EG2 ");
			break;
		case CONN_SRC_PITCHWHEEL:
			printf("SRC_PITCHWHEEL ");
			break;
		// Midi Controllers 0-127
		case CONN_SRC_CC1:
			printf("SRC_CC1 ");
			break;
		case CONN_SRC_CC7:
			printf("SRC_CC7 ");
			break;
		case CONN_SRC_CC10:
			printf("SRC_CC10 ");
			break;
		case CONN_SRC_CC11:
			printf("SRC_CC11 ");
			break;
		default:
			printf("CONN_SRC UNKNOWN: %d\n", blk->source);
			break;
		}
		switch (blk->destination)
		{
		// Generic Destinations
		case CONN_DST_NONE:
			printf("DST_NONE");
			break;
		case CONN_DST_ATTENUATION:
			printf("DST_ATTENUATION %f", DLSRelGain(blk->scale));
			break;
		case CONN_DST_RESERVED:
			printf("DST_RESERVED %d", blk->scale);
			break;
		case CONN_DST_PITCH:
			printf("DST_PITCH %d", blk->scale);
			break;
		case CONN_DST_PAN:
			printf("DST_PAN %f", ((double)blk->scale / (10.0 * 65536.0)) / 100.0);
			break;
		// LFO Destinations
		case CONN_DST_LFO_FREQUENCY:
			printf("DST_LFO_FREQUENCY %f", DLSPitchCents(blk->scale));
			break;
		case CONN_DST_LFO_STARTDELAY:
			printf("DST_LFO_STARTDELAY %f", DLSTimeCents(blk->scale));
			break;
		// EG1 Destinations
		case CONN_DST_EG1_ATTACKTIME:
			printf("DST_EG1_ATTACKTIME %f", DLSTimeCents(blk->scale));
			break;
		case CONN_DST_EG1_DECAYTIME:
			printf("DST_EG1_DECAYTIME %f", DLSTimeCents(blk->scale));
			break;
		case CONN_DST_EG1_RESERVED:
			printf("DST_EG1_RESERVED %f ", DLSTimeCents(blk->scale));
			break;
		case CONN_DST_EG1_RELEASETIME:
			printf("DST_EG1_RELEASETIME %f", DLSTimeCents(blk->scale));
			break;
		case CONN_DST_EG1_SUSTAINLEVEL:
			printf("DST_EG1_SUSTAINLEVEL %f", (double)blk->scale / (1000.0*65536.0));
			break;
		// EG2 Destinations
		case CONN_DST_EG2_ATTACKTIME:
			printf("DST_EG2_ATTACKTIME %f", DLSTimeCents(blk->scale));
			break;
		case CONN_DST_EG2_DECAYTIME:
			printf("DST_EG2_DECAYTIME %f", DLSTimeCents(blk->scale));
			break;
		case CONN_DST_EG2_RESERVED:
			printf("DST_EG2_RESERVED %f", DLSTimeCents(blk->scale));
			break;
		case CONN_DST_EG2_RELEASETIME:
			printf("DST_EG2_RELEASETIME %f", DLSTimeCents(blk->scale));
			break;
		case CONN_DST_EG2_SUSTAINLEVEL:
			printf("DST_EG2_SUSTAINLEVEL %f", (double)blk->scale / (1000.0*65536.0));
			break;
		default:
			printf("CONN_DST UNKNOWN: %d", blk->destination);
			break;
		}
		printf("\n");
	}
}

void Dump(DLSRgnInfo *rgn)
{
	printf("    Region grp=%d opt=%d key={%d,%d} vel={%d,%d} ex=%d\n", 
		rgn->rgnh.keyGroup, rgn->rgnh.options, 
		rgn->rgnh.rangeKey.low, rgn->rgnh.rangeKey.high,
		rgn->rgnh.rangeVel.low, rgn->rgnh.rangeVel.high,
		rgn->rgnh.exclusive);

	printf("           atn=%d (%f) ft=%d (%f) opt=%d loop=%d key=%d\n", 
		rgn->wsmp.attenuation, pow(10.0, (double)rgn->wsmp.attenuation / (200.0 * 65536.0)),
		rgn->wsmp.fineTune, pow(2.0, (double)rgn->wsmp.fineTune / 1200.0),
		rgn->wsmp.options,	rgn->wsmp.sampleLoops, rgn->wsmp.unityNote);
	printf("           loop len=%d sz=%d st=%d ty=%d\n",
		rgn->loop.length, rgn->loop.size, rgn->loop.start, rgn->loop.type);
	printf("           ch=%d opt=%d pg=%d ti=%d\n", 
		rgn->wlnk.channel, rgn->wlnk.options, rgn->wlnk.phaseGroup, rgn->wlnk.tableIndex);

	DLSArtInfo *art = 0;
	while ((art = rgn->lart.EnumArt(art)) != 0)
		Dump(art);
}

void Dump(DLSInsInfo *in)
{
	printf("  Instr: bnk=%d prg=%d m/d=%d\n",
		in->GetBank(), in->GetProg(), in->IsDrum());
	printf("     ID %08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x\n", 
		in->id.data1, in->id.data2, in->id.data3,
		in->id.data4[0], in->id.data4[1], in->id.data4[2], in->id.data4[3],
		in->id.data4[4], in->id.data4[5], in->id.data4[6], in->id.data4[7]);

	DLSInfoStr *is = 0;
	while ((is = in->info.EnumInfo(is)) != 0)
		printf("      '%s'\n", (const char *)is->str);

	DLSRgnInfo *rgn = 0;
	while ((rgn = in->rgnlist.EnumRgn(rgn)) != 0)
		Dump(rgn);

	DLSArtInfo *art = 0;
	while ((art = in->lart.EnumArt(art)) != 0)
		Dump(art);
}

void Dump(DLSWaveInfo *wvi)
{
	printf("  Wave %d size=%d\n", wvi->index, wvi->sampsize);

	DLSInfoStr *is = 0;
	while ((is = wvi->info.EnumInfo(is)) != 0)
		printf("      '%s'\n", (const char *)is->str);

	printf("    ID %08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x\n", 
		wvi->id.data1, wvi->id.data2, wvi->id.data3,
		wvi->id.data4[0], wvi->id.data4[1], wvi->id.data4[2], wvi->id.data4[3],
		wvi->id.data4[4], wvi->id.data4[5], wvi->id.data4[6], wvi->id.data4[7]);
	printf("    FMT t=%d ch=%d sr=%d bps=%d bits=%d algn=%d\n",
		wvi->wvfmt.fmtTag,
		wvi->wvfmt.channels,
		wvi->wvfmt.sampleRate,
		wvi->wvfmt.avgBytesPerSec,
		wvi->wvfmt.bitsPerSamp,
		wvi->wvfmt.blockAlign);

	printf("    SAMP atn=%d (%f) ft=%d (%f) opt=%d loop=%d key=%d\n", 
		wvi->wvsmp.attenuation, pow(10.0, (double)wvi->wvsmp.attenuation / (200.0 * 65536.0)),
		wvi->wvsmp.fineTune, pow(2.0, (double)wvi->wvsmp.fineTune / 1200.0),
		wvi->wvsmp.options,	wvi->wvsmp.sampleLoops, wvi->wvsmp.unityNote);
	printf("    LOOP len=%d sz=%d st=%d ty=%d\n",
		wvi->loop.length, wvi->loop.size, wvi->loop.start, wvi->loop.type);
}

void Dump(DLSFileInfo *dls)
{
	printf("DLS FILE: \n");
	printf(" VER %d.%d.%d.%d\n", 
		dls->vers.dwVersionMS >> 16, 
		dls->vers.dwVersionMS & 0xffff,
		dls->vers.dwVersionLS >> 16,
		dls->vers.dwVersionLS & 0xffff);
	printf(" DLID %08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x\n", 
		dls->id.data1, dls->id.data2, dls->id.data3,
		dls->id.data4[0], dls->id.data4[1], dls->id.data4[2], dls->id.data4[3],
		dls->id.data4[4], dls->id.data4[5], dls->id.data4[6], dls->id.data4[7]);
	printf(" instruments = %d\n", dls->colh.instruments);
	printf(" pool cues: %d\n", dls->ptbl.cues);
	/*
	for (int c = 0; c < dls->ptbl.cues; c++)
		printf("    cue %d = %d\n", c, dls->cues[c]);
	*/
	DLSInfoStr *inf = 0;
	while ((inf = dls->inflist.EnumInfo(inf)) != 0)
		printf("%s\n", (const char *)inf->str);

	DLSInsInfo *in = 0;
	while ((in = dls->ins.EnumIns(in)) != 0)
		Dump(in);

	DLSWaveInfo *wv = 0;
	while ((wv = dls->wvpl.EnumWave(wv)) != 0)
		Dump(wv);
}


class SFEnvelope
{
private:
	AmpValue curLevel;
	AmpValue susLevel;
	AmpValue atkIncr;
	AmpValue decIncr;
	AmpValue relIncr;
	bsInt32  delayCount;
	bsInt32  holdCount;
	int      segNdx;
public:
	SFEnvelope()
	{
		curLevel = 0;
		susLevel = 0;
		atkIncr = 1.0;
		decIncr = 1.0;
		relIncr = 1.0;
		delayCount = 0;
		holdCount = 0;
		segNdx = 0;
	}

	// rate is in timecents = 1200 * log2(secs)
	void SetDelay(FrqValue tc)
	{
		holdCount = (bsInt32) (synthParams.sampleRate * tc);
	}

	void SetAttack(FrqValue tc)
	{
		FrqValue count = (synthParams.sampleRate * tc);
		if (count > 0)
			atkIncr = 1.0 / count;
		else
			atkIncr = 1.0;
	}

	void SetHold(FrqValue tc)
	{
		holdCount = (bsInt32) (synthParams.sampleRate * tc);
	}

	void SetDecay(FrqValue tc)
	{
		FrqValue count = (synthParams.sampleRate * tc);
		if (count > 0)
			decIncr = 1.0 / count;
		else
			decIncr = 1.0;
	}

	void SetRelease(FrqValue tc)
	{
		FrqValue count = (synthParams.sampleRate * tc);
		if (count > 0)
			relIncr = 1.0 / count;
		else
			relIncr = 1.0;
	}

	// sustain: 0 <= lvl <= 1.0
	void SetSustain(AmpValue lvl)
	{
		susLevel = lvl;
	}

	void Start()
	{
		segNdx = 0;
	}

	int IsFinished()
	{
		return segNdx > 5;
	}

	void Release()
	{
		if (segNdx < 5)
			segNdx = 5;
	}

	AmpValue Gen()
	{
		switch (segNdx)
		{
		case 0: // delay
			if (delayCount > 1)
			{
				delayCount--;
				return 0.0;
			}
			segNdx++;
		case 1:
			curLevel += atkIncr;
			if (curLevel < 1.0)
				return curLevel * curLevel;
			segNdx++;
			curLevel = 1.0;
		case 2:
			if (holdCount > 1)
			{
				holdCount--;
				return 1.0;
			}
			segNdx++;
		case 3:
			curLevel -= decIncr;
			if (curLevel > susLevel)
				return curLevel;
			segNdx++;
			curLevel = susLevel;
		case 4:
			return susLevel;
		case 5:
			curLevel -= relIncr;
			if (curLevel > 0)
				return curLevel;
			segNdx++;
			curLevel = 0;
		case 6:
			return 0.0;
		}
		return 0.0;
	}
};

class SFPlayer
{
private:
	AmpValue vol;
	GenWaveSF *osc;
	GenWaveWT vib;
	GenWaveWT mod;
	EnvGenSF volEnv;  // volume envelope
	EnvGenSF modEnv;
	Panner panr;
	Panner panl;
	SBZone *zone;
	SBInstr *instr;
	int pitch;
	int vel;
	int mono;
	bsInt32 vibDelay;
	bsInt32 modDelay;
	FrqValue frq;
public:

	SFPlayer()
	{
		vol = 1.0;
		zone = 0;
		pitch = 57;
		frq = 440;
		mono = 0; // force mono mode
		vel = 60;
		osc = 0;
	}

	void Init(int pit, SBInstr *in, SBZone *z)
	{
		if (osc)
			delete osc;
		instr = in;
		vol = 1.0;
		pitch = pit;
		frq = synthParams.GetFrequency(pit-12); 

		osc = new GenWaveSF1;
		if ((zone = z) != 0)
		{
			osc->InitSF(frq, zone, 0);
			volEnv.InitEnv(&zone->volEg, pit, 100);
			modEnv.InitEnv(&zone->modEg, pit, 100);
			vibDelay = (bsInt32) (zone->vibLfo.delay * synthParams.sampleRate);
			modDelay = (bsInt32) (zone->modLfo.delay * synthParams.sampleRate);
			vib.InitWT(zone->vibLfo.rate, WT_SIN);
			mod.InitWT(zone->modLfo.rate, WT_SIN);
			panr.Set(panSqr, zone->pan);
			panl.Set(panSqr, zone->pan);
			vol = zone->volAtten;
		}
		else
		{
			osc->InitSF(frq, 0, 0);
			volEnv.InitEnv(0, pit, 100);
			modEnv.InitEnv(0, pit, 100);
			vib.InitWT(5.2, WT_SIN);
			mod.InitWT(5.2, WT_SIN);
			panr.Set(panSqr, 0);
			panl.Set(panSqr, 0);
			vol = 1.0;
		}
		volEnv.Reset(0);
		modEnv.Reset(0);
	}

	void Init(int pit, SBInstr *in)
	{
		if (osc)
			delete osc;

		mono = 0;
		instr = in;
		vol = 1.0;
		pitch = pit;
		frq = synthParams.GetFrequency(pit-12); 

		SBZone *zone = in->GetZone(pit, 100);
		if (zone->sample->channels == 2 || zone->sample->linkSamp)
			osc = new GenWaveSF2;
		else if (zone->linkZone)
			osc = new GenWaveSF3;
		else
			osc = new GenWaveSF1;
		osc->InitSF(frq, zone, 0);
		volEnv.InitEnv(&zone->volEg, pit, 100);
		modEnv.InitEnv(&zone->modEg, pit, 100);
		vibDelay = (bsInt32) (zone->vibLfo.delay * synthParams.sampleRate);
		modDelay = (bsInt32) (zone->modLfo.delay * synthParams.sampleRate);
		vib.InitWT(zone->vibLfo.rate, WT_SIN);
		mod.InitWT(zone->modLfo.rate, WT_SIN);
		panr.Set(panSqr, zone->pan);
		if (zone->linkZone)
			panl.Set(panSqr, zone->linkZone->pan);
		else
			panl.Set(panSqr, zone->pan);
		vol = zone->volAtten;
	}

	void Stop()
	{
		volEnv.Release();
		modEnv.Release();
		osc->Release();
	}

	int IsFinished()
	{
		return volEnv.IsFinished() || osc->IsFinished();
	}

	void Tick(WaveOutBuf *wf)
	{
		FrqValue frqPC = modEnv.Gen() * zone->modEnvFrq;
		AmpValue ampCB = vol;

		if (vibDelay == 0)
			frqPC += vib.Gen() * zone->vibLfoFrq;
		else
			vibDelay--;

		if (modDelay == 0)
		{
			AmpValue m = mod.Gen();
			frqPC += m * zone->modLfoFrq;
			ampCB += m * zone->modLfoVol;
		}
		else
			modDelay--;

		ampCB += (1.0 - volEnv.Gen()) * 960.0;

		FrqValue frqVal = frq * synthParams.GetCentsMult((int) frqPC);
//		AmpValue ampVal = volEnv.Gen() * synthParams.AttenCB((int)ampCB);
		AmpValue ampVal = synthParams.AttenCB((int)ampCB);

		osc->UpdateFrequency(frqVal);

		AmpValue oscValr;
		AmpValue oscVall;
		osc->Tick(oscVall, oscValr);
		wf->Output2(
			ampVal * ((oscVall * panl.panlft) + (oscValr * panr.panlft)),
			ampVal * ((oscVall * panl.panrgt) + (oscValr * panr.panrgt)));
	}
};

char *wavePrefix = "";

void PlayPreset(SoundBank *bnk, int bnkNum, int preNum, int mono)
{
	SBInstr *instr = bnk->GetInstr(bnkNum, preNum);
	if (instr == 0)
	{
		fprintf(stderr, "Bank %d and preset %d does not exist\n", bnkNum, preNum);
		return;
	}

	fprintf(stderr, "Playing bank %d, preset %d : %s\n", bnkNum, preNum, (const char *)instr->instrName);

	WaveFile wf;
	char waveFile[1024];
	strcpy(waveFile, wavePrefix);
	strcat(waveFile, instr->instrName);
	strcat(waveFile, ".wav");
	char *sep = &waveFile[strlen(wavePrefix)];
	while (*sep)
	{
		if (*sep == '/' || *sep == '\\')
			*sep = '_';
		sep++;
	}

	if (wf.OpenWaveFile(waveFile, 2))
	{
		fprintf(stderr, "Cannot open file '%s'\n", (const char *)waveFile);
		return;
	}

	long silence = (long) (synthParams.sampleRate * 0.1);
	long dur = synthParams.isampleRate;
	long t;
	SFPlayer play;

	// loop over all zones
	int pit;
	SBZone *zone = 0;
	while ((zone = instr->EnumZones(zone)) != 0)
	{
		fprintf(stderr, "  zone %s, %d [%d,%d] ch=%d\n", 
			(const char *)zone->name, zone->keyNum, 
			zone->lowKey, zone->highKey, zone->chan);
		if (zone->keyNum == -1) // global zone?
		{
			fprintf(stderr, "   Skipped...\n", zone->chan, zone->keyNum);
			continue;
		}
		//fprintf(stderr, "  env: A=%f H=%f D=%f S=%f R=%f\n",
		//		zone->volEgAttack, zone->volEgHold, zone->volEgDecay, 
		//		zone->volEgSustain, zone->volEgRelease);
		//fprintf(stderr, "  frq  rf=%f per=%d sr=%f\n",zone->recFreq, zone->recPeriod, zone->rate);
		//fprintf(stderr, "  samp %u len=%d st=%u end=%u ls=%u le=%u\n",
		//	zone->sample, zone->sampleLen,
		//	zone->tableStart, zone->tableEnd, 
		//	zone->loopStart, zone->loopEnd);

		// There are soundfont files out there where the key number is not
		// within the range of [low,high] (believe it or not)...
		if (zone->keyNum < zone->lowKey || zone->keyNum > zone->highKey)
			pit = (zone->highKey + zone->lowKey) / 2;
		else
			pit = zone->keyNum;
		dur = zone->sampleLen;
		if (zone->mode) // looped
			dur *= 4;
		play.Init(pit, instr, zone);
		for (t = 0; t < dur; t++)
			play.Tick(&wf);
		play.Stop();
		while (!play.IsFinished())
			play.Tick(&wf);
		for (t = 0; t < silence; t++)
			wf.Output1(0.0);
		if (wf.GetOOR() > 0)
		{
			printf("Out of range=%d\n", wf.GetOOR());
			wf.ClrOOR();
		}
	}
	if (bnkNum != 128)
	{
		dur = synthParams.isampleRate / 4;
		int pitches[] = { 36, 48, 60, 72, 84, -1 };
		for (pit = 0; pitches[pit] != -1; pit++)
		{
			play.Init(pitches[pit], instr);
			for (t = 0; t < dur; t++)
				play.Tick(&wf);
			play.Stop();
			while (!play.IsFinished())
				play.Tick(&wf);
		}
	}
	wf.CloseWaveFile();
}

void usage()
{
	fprintf(stderr, "use: example4a soundfont -bbank -ppreset -m -d(1|2|3) -wprefix -l[0|1] -n[0|1]\n");
	fprintf(stderr, "     -d1 = print internal representation\n");
	fprintf(stderr, "     -d2 = print raw file contents\n");
	fprintf(stderr, "     -d3 = print both\n");
	fprintf(stderr, "     -m  = force monophonic\n");
	fprintf(stderr, "     -w  = prefix to output file name\n");
	fprintf(stderr, "     -l0 = don't preload samples\n");
	fprintf(stderr, "     -l1 = preload all samples (default)\n");
	fprintf(stderr, "     -l0 = don't normalize sample amplitude (default)\n");
	fprintf(stderr, "     -l1 = normalize sample amplitude\n");
	fprintf(stderr, "     -bn = only output bank #n, else all banks\n");
	fprintf(stderr, "     -pn = only output patch #n, else all patches\n");
	exit(1);
}

int main(int argc, char *argv[])
{

	if (argc < 2)
		usage();

	int preload = 1;
	float scl = 0.0;
	int doDump = 0;
	char *fileName = argv[1];
	char *waveFile = 0;
	int bnkNum = -1;
	int preNum = -1;
	int mono = 0;
	int isDLS = 0;
	int isSF2 = 0;

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
		case 'l':
			if (*ap)
				preload = atoi(ap);
			else
				preload = 1;
			break;
		case 's':
			scl = atof(ap);
			break;
		default:
			fprintf(stderr, "Invalid argument: %s\n", argv[argn]);
			usage();
			break;
		}
	}
	InitSynthesizer();

	SoundBank *bnk = 0;
	isSF2 = SFFile::IsSF2File(fileName);
	if (!isSF2)
	{
		isDLS = DLSFile::IsDLSFile(fileName);
		if (!isDLS)
		{
			fprintf(stderr, "File format is not SF2 or DLS\n");
			exit(1);
		}
		DLSFile df;
		if (scl == 0.0)
			scl = 1.0;
		bnk = df.LoadSoundBank(fileName, preload, scl);
		if (doDump & 2)
			Dump(df.GetInfo());
	}
	else
	{
		SFFile sounds;
		if (scl == 0.0)
			scl = 0.375;
		bnk = sounds.LoadSoundBank(fileName, preload, scl);
		if (doDump & 2)
			Dump(&sounds);
	}
	if (!bnk)
	{
		fprintf(stderr, "No bank\n");
		exit(1);
	}
	if (doDump)
	{
		if (doDump & 1)
			Dump(bnk);
		exit(0);
	}

	bnk->Lock();
	if (bnkNum == -1)
	{
		for (bnkNum = 0; bnkNum <= 128; bnkNum++)
		{
			if (!bnk->instrBank[bnkNum])
				continue;
			if (preNum == -1)
			{
				for (int pn = 0; pn < 128; pn++)
					PlayPreset(bnk, bnkNum, pn, mono);
			}
			else
				PlayPreset(bnk, bnkNum, preNum, mono);
		}
	}
	else
	{
		if (preNum == -1)
		{
			for (int pn = 0; pn < 128; pn++)
				PlayPreset(bnk, bnkNum, pn, mono);
		}
		else
			PlayPreset(bnk, bnkNum, preNum, mono);
	}
	bnk->Unlock();

	return 0;
}
