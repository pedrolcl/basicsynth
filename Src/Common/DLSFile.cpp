#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <SynthDefs.h>
#include <WaveFile.h>
#include <SynthList.h>
#include <SFDefs.h>
#include <SoundBank.h>
#include <DLSFile.h>

DLSFile::DLSFile()
{
	preload = 1;
	atnScl = 1.0;
}

DLSFile::~DLSFile()
{
}

int DLSFile::IsDLSFile(const char *fname)
{
	FileReadBuf file;
	int isDLS = 0;
	if (file.FileOpen(fname) == 0)
	{
		// read the RIFF chunk.
		dlsChunk rchk;
		rchk.ckid = 0;
		file.FileRead(&rchk, 8);
		if (rchk.ckid == DLS_RIFF_CHUNK)
		{
			// Read the format
			bsInt32 id = 0;
			file.FileRead(&id, 4);
			if (id == DLS_FILE_FORMAT)
				isDLS = 1;
		}
		file.FileClose();
	}
	return isDLS;
}

SoundBank *DLSFile::LoadSoundBank(const char *fname, int pre, float scl)
{
	preload = pre;
	atnScl = scl;

	file.SetBufSize(0x10000);
	if (file.FileOpen(fname))
		return 0;

	dlsChunk rchk;
	rchk.ckid = 0;
	rchk.cksz = 0;
	// read the RIFF chunk.
	file.FileRead(&rchk, 8);
	if (rchk.ckid != DLS_RIFF_CHUNK)
	{
		file.FileClose();
		return 0;
	}

	// Read the format
	bsInt32 id = 0;
	file.FileRead(&id, 4);
	if (id != DLS_FILE_FORMAT)
	{
		file.FileClose();
		return 0;
	}
	SoundBank *sb = 0;
	if (info.Read(file, rchk.cksz - 4) == 0)
		sb = BuildSoundBank(fname);
	file.FileClose();
	return sb;
}

int DLSFileInfo::Read(FileReadBuf& file, bsUint32 riffSize)
{
	dlsChunk chk;
	int err = 0;
	while (riffSize > 0 && err == 0)
	{
		// read the next chunk header
		if (file.FileRead(&chk, 8) != 8)
			break;
		riffSize -= 8;
		if (chk.ckid == DLS_VERS_CHUNK)
		{
			if (file.FileRead(&vers, 8) != 8)
				err = -1;
		}
		else if (chk.ckid == DLS_DLID_CHUNK)
		{
			if (file.FileRead(&id, 16) != 16)
				err = -1;
		}
		else if (chk.ckid == DLS_COLH_CHUNK)
		{
			if (file.FileRead(&colh, 4) != 4)
				err = -1;
		}
		else if (chk.ckid == DLS_PTBL_CHUNK)
		{
			if (file.FileRead(&ptbl, sizeof(ptbl)) != sizeof(ptbl))
				err = -1;
			else
			{
				if (ptbl.size > 8)
					file.FileSkip(ptbl.size - 8);
				cues = new bsInt32[ptbl.cues];
				file.FileRead(cues, ptbl.cues*4);
			}
		}
		else if (chk.ckid == DLS_LIST_CHUNK)
		{
			bsUint32 fmt = 0;
			file.FileRead(&fmt, 4);
			if (fmt == DLS_LINS_FORMAT)
				err = ins.Read(file, chk.cksz - 4);
			else if (fmt == DLS_WVPL_FORMAT)
				err = wvpl.Read(file, chk.cksz - 4);
			else if (fmt == DLS_INFO_FORMAT)
				err = inflist.Read(file, chk.cksz-4);
			else
				file.FileSkip(chk.cksz - 4);
		}
		else
			file.FileSkip(chk.cksz);
		riffSize -= chk.cksz;
	}

	return err;
}

int DLSWvplInfo::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf(" WVPLChunk(%d)\n", chksz);
	filepos = file.FilePosition();
	int err = 0;
	bsInt32 index = 0;
	dlsChunk chk;
	while (chksz > 0 && err == 0)
	{
		if (file.FileRead(&chk, 8) != 8)
			return -1;
		chksz -= 8;
		if (chk.ckid == DLS_LIST_CHUNK)
		{
			bsUint32 fmt = 0;
			if (file.FileRead(&fmt, 4) != 4)
				err = -1;
			else if (fmt == DLS_WAVE_FORMAT)
			{
				DLSWaveInfo *wi = AddItem();
				wi->index = index++;
				err = wi->Read(file, chk.cksz - 4);
			}
			else
				file.FileSkip(chk.cksz - 4);
		}
		else
			file.FileSkip(chk.cksz);
		chksz -= chk.cksz;
	}
	return err;
}

int DLSWaveInfo::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf("  WAVEChunk(%d)\n", chksz);
	int err = 0;
	bsUint32 fmt = 0;
	dlsChunk chk;
	while (chksz > 0 && err == 0)
	{
		if (file.FileRead(&chk, 8) != 8)
			return -1;
		chksz -= 8;
		switch (chk.ckid)
		{
		case DLS_DLID_CHUNK:
			file.FileRead(&id, sizeof(id));
			break;
		case DLS_FMT__CHUNK:
			if (file.FileRead(&wvfmt, 16) != 16)
				err = -1;
			else if (chk.cksz > 16)
				file.FileSkip(chk.cksz - 16);
			break;
		case DLS_DATA_CHUNK:
			err = ReadSamples(file, chk.cksz);
			break;
		case DLS_WSMP_CHUNK:
			if (file.FileRead(&wvsmp, sizeof(wvsmp)) != sizeof(wvsmp))
			{
				err = -1;
				break;
			}
			if (wvsmp.size > sizeof(wvsmp))
				file.FileSkip(wvsmp.size - sizeof(wvsmp));
			if (wvsmp.sampleLoops > 0)
			{
				file.FileRead(&loop, sizeof(loop));
				if (loop.size > sizeof(loop))
					file.FileSkip(loop.size - sizeof(loop));
				// just-in-case
				for (bsUint32 n = 1; n < wvsmp.sampleLoops; n++)
					file.FileSkip(loop.size);
			}
			break;
		case DLS_LIST_CHUNK:
			file.FileRead(&fmt, 4);
			if (fmt == DLS_INFO_FORMAT)
				err = info.Read(file, chk.cksz - 4);
			else
				file.FileSkip(chk.cksz - 4);
			break;
		default:
			file.FileSkip(chk.cksz);
			break;
		}
		chksz -= chk.cksz;
	}
	return err;
}

int DLSWaveInfo::ReadSamples(FileReadBuf& file, bsUint32 chksz)
{
	//printf("DATAChunk(%d)\n", chksz);

	// save file position - we will load samples later.
	filepos = file.FilePosition();

	file.FileSkip(chksz);
	sampsize = chksz / (wvfmt.bitsPerSamp / 8);
	return 0;
}

int DLSInsList::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf("LINSChunk(%d)\n", chksz);

	int index = 0;
	int err = 0;
	dlsChunk chk;
	while (chksz > 0 && err == 0)
	{
		if (file.FileRead(&chk, 8) != 8)
			return -1;
		chksz -= 8;
		if (chk.ckid == DLS_LIST_CHUNK)
		{
			bsUint32 fmt = 0;
			file.FileRead(&fmt, 4);
			if (fmt == DLS_INS__FORMAT)
			{
				DLSInsInfo *ii = AddItem();
				ii->index = index++;
				err = ii->Read(file, chk.cksz - 4);
			}
			else
				file.FileSkip(chk.cksz - 4);
		}
		else
			file.FileSkip(chk.cksz);
		chksz -= chk.cksz;
	}
	return 0;
}

int DLSInsInfo::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf(" INSChunk(%d)\n", chksz);
	int err = 0;

	bsUint32 fmt;
	dlsChunk chk;
	while (chksz > 0 && err == 0)
	{
		if (file.FileRead(&chk, 8) != 8)
			return -1;
		chksz -= 8;
		switch (chk.ckid)
		{
		case DLS_DLID_CHUNK:
			if (file.FileRead(&id, 16) != 16)
				err = -1;
			break;
		case DLS_INSH_CHUNK:
			if (file.FileRead(&hdr, sizeof(hdr)) != sizeof(hdr))
				err = -1;
			break;
		case DLS_LIST_CHUNK:
			fmt = 0;
			if (file.FileRead(&fmt, 4) != 4)
			{
				err = -1;
				break;
			}
			switch (fmt)
			{
			case DLS_LRGN_FORMAT:
				err = rgnlist.Read(file, chk.cksz - 4);
				break;
			case DLS_LART_FORMAT:
			case DLS_LAR2_FORMAT:
				err = lart.Read(file, chk.cksz - 4);
				break;
			case DLS_INFO_FORMAT:
				err = info.Read(file, chk.cksz - 4);
				break;
			default:
				file.FileSkip(chk.cksz - 4);
				break;
			}
			break;
		default:
			file.FileSkip(chk.cksz);
			break;
		}
		chksz -= chk.cksz;
	}
	return err;
}

int DLSRgnList::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf("  LRGNChunk(%d)\n", chksz);
	int err = 0;
	dlsChunk chk;
	while (chksz > 0 && err == 0)
	{
		if (file.FileRead(&chk, 8) != 8)
			return -1;
		chksz -= 8;
		if (chk.ckid == DLS_LIST_CHUNK)
		{
			bsUint32 fmt = 0;
			file.FileRead(&fmt, 4);
			if (fmt == DLS_RGN__FORMAT || fmt == DLS_RGN2_FORMAT)
			{
				DLSRgnInfo *ri = AddItem();
				err = ri->Read(file, chk.cksz - 4);
			}
			else
				file.FileSkip(chk.cksz - 4);
		}
		else
			file.FileSkip(chk.cksz);
		chksz -= chk.cksz;
	}
	return err;
}

int DLSRgnInfo::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf("   RGNChunk(%d)\n", chksz);
	int err = 0;

	bsUint32 fmt = 0;
	dlsChunk chk;
	while (chksz > 0 && err == 0)
	{
		if (file.FileRead(&chk, 8) != 8)
			return -1;
		chksz -= 8;
		switch (chk.ckid)
		{
		case DLS_RGNH_CHUNK:
			//printf("    RGNHChunk(%d) %d\n", chk.cksz, sizeof(rgnh));
			if (chk.cksz > sizeof(rgnh))
			{
				file.FileRead(&rgnh, sizeof(rgnh));
				file.FileSkip(chk.cksz-sizeof(rgnh));
			}
			else
				file.FileRead(&rgnh, chk.cksz);
			break;
		case DLS_WSMP_CHUNK:
			//printf("    WSMPChunk(%d)\n", chk.cksz);
			wsmpValid = 1;
			file.FileRead(&wsmp, sizeof(wsmp));
			if (wsmp.size > sizeof(wsmp))
				file.FileSkip(wsmp.size - sizeof(wsmp));
			if (wsmp.sampleLoops > 0)
			{
				file.FileRead(&loop, sizeof(loop));
				if (loop.size > sizeof(loop))
					file.FileSkip(loop.size - sizeof(loop));
				// just-in-case
				for (bsUint32 n = 1; n < wsmp.sampleLoops; n++)
					file.FileSkip(loop.size);
			}
			break;
		case DLS_WLNK_CHUNK:
			//printf("    WLNKChunk(%d)\n", chk.cksz);
			file.FileRead(&wlnk, sizeof(wlnk));
			break;
		case DLS_LIST_CHUNK:
			//printf("    LISTChunk(%d)\n", chk.cksz);
			file.FileRead(&fmt, 4);
			if (fmt == DLS_LART_FORMAT || fmt == DLS_LAR2_FORMAT)
				err = lart.Read(file, chk.cksz - 4);
			else if (fmt == DLS_INFO_FORMAT)
				file.FileSkip(chk.cksz - 4);
			else
			{
				/*char nmbuf[5];
				memcpy(nmbuf, &fmt, 4);
				nmbuf[4] = 0;
				printf("List format %s\n", nmbuf);*/
				file.FileSkip(chk.cksz - 4);
			}
			break;
		default:
			file.FileSkip(chk.cksz);
			break;
		}
		chksz -= chk.cksz;
	}
	return err;
}

int DLSArtList::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf("    LARTChunk(%d)\n", chksz);
	int err = 0;

	dlsChunk chk;
	while (chksz > 0 && err == 0)
	{
		if (file.FileRead(&chk, 8) != 8)
			return -1;
		chksz -= 8;
		if (chk.ckid == DLS_ART1_CHUNK || chk.ckid == DLS_ART2_CHUNK)
		{
			DLSArtInfo *ci = AddItem();
			ci->Read(file, chk.cksz);
		}
		else
			file.FileSkip(chk.cksz);
		chksz -= chk.cksz;
	}
	return err;
}

int DLSArtInfo::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf("     ArtNChunk(%d)\n", chksz);

	bsUint32 sz = 0;
	file.FileRead(&sz, 4);
	file.FileRead(&connections, 4);
	if (sz > 8)
		file.FileSkip(sz - 8);
	if (connections == 0)
		return 0;

	info = new dlsConnection[connections];
	for (bsUint32 ndx = 0; ndx < connections; ndx++)
	{
		if (file.FileRead(&info[ndx], 12) != 12)
			return -1;
	}
	return 0;
}


int DLSInfoList::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf(" InfoList(%d)\n", chksz);

	int err = 0;
	dlsChunk chk;
	while (chksz > 0 && err == 0)
	{
		if (file.FileRead(&chk, 8) != 8)
			return -1;
		chksz -= 8;
		if (chk.cksz > 0)
		{
			if (chk.cksz & 1)
				chk.cksz++;
			DLSInfoStr *str = AddItem();
			str->id = chk.ckid;
			str->Read(file, chk.cksz);
			chksz -= chk.cksz;
		}
	}
	return err;
}

int DLSInfoStr::Read(FileReadBuf& file, bsUint32 cksz)
{
	//printf("  InfoStr(%d)\n", cksz);
	char *s = (char *)malloc(cksz+1);
	if (file.FileRead(s, cksz) == cksz)
	{
		s[cksz] = '\0';
		while (--cksz >= 0)
		{
			if (s[cksz] && s[cksz] != ' ')
				break;
			s[cksz] = '\0';
		}
		str.Attach(s);
		return 0;
	}
	free(s);
	return -1;
}

FrqValue DLSFile::DLSTimeCents(bsInt32 tc)
{
	return SoundBank::EnvRate((FrqValue) tc / 65536.0);
}

AmpValue DLSFile::DLSRelGain(bsInt32 cb)
{
	return SoundBank::Attenuation((AmpValue) cb / 65536.0);
}

AmpValue DLSFile::DLSAttenuation(bsInt32 cb)
{
	return SoundBank::Attenuation(((AmpValue) cb * atnScl)/ 65536.0);
}

FrqValue DLSFile::DLSPitchCents(bsInt32 pc)
{
	return 440.0 * pow(2.0, (((double)pc / 65536.0) - 6900.0) / 1200.0);
}

FrqValue DLSFile::DLSCents(bsInt32 pc)
{
	if (pc == 0)
		return 1.0;
	return SoundBank::PitchCents((FrqValue) pc / 65536.0);
}

AmpValue DLSFile::DLSPercent(bsInt32 pct)
{
	return (AmpValue) pct / (1000.0 * 65536.0);
}

SoundBank *DLSFile::BuildSoundBank(const char *fname)
{
	SoundBank *sbnk = new SoundBank;

	sbnk->file = fname;
	sbnk->info.wMajorFile = info.vers.dwVersionMS >> 16;
	sbnk->info.wMinorFile = info.vers.dwVersionMS & 0xffff;
	sbnk->info.wMajorVer = info.vers.dwVersionLS >> 16;
	sbnk->info.wMinorVer = info.vers.dwVersionLS & 0xffff;
	sbnk->info.szName = info.inflist.GetInfo(DLS_INAM_CHUNK);
	sbnk->info.szDate = info.inflist.GetInfo(DLS_ICRD_CHUNK);
	sbnk->info.szEng = info.inflist.GetInfo(DLS_IENG_CHUNK);
	sbnk->info.szProduct = info.inflist.GetInfo(DLS_IPRD_CHUNK);
	sbnk->info.szCopyright = info.inflist.GetInfo(DLS_ICOP_CHUNK);
	sbnk->info.szComment = info.inflist.GetInfo(DLS_ICMT_CHUNK);

	DLSWaveInfo *wvi = 0;
	while ((wvi = info.wvpl.EnumWave(wvi)) != 0)
	{
		SBSample *sp = sbnk->AddSample(wvi->index);
		//sp->filepos = info.cues[wvi->index] + info.wvpl.filepos;
		//if (sp->filepos != wvi->filepos) // testing
		sp->filepos = wvi->filepos;
		sp->sampleLen = wvi->sampsize;
		sp->rate = wvi->wvfmt.sampleRate;
		sp->channels = wvi->wvfmt.channels;
		if (wvi->wvfmt.fmtTag == 1)
			sp->format = wvi->wvfmt.bitsPerSamp == 8 ? 0 : 1;
		else if (wvi->wvfmt.fmtTag == 2)
			sp->format = 2;
		if (preload)
			sbnk->LoadSample(sp, file);
	}

	double modval;

	DLSInsInfo *ins = 0;
	DLSRgnInfo *rgn;
	DLSArtInfo *art;
	while ((ins = info.ins.EnumIns(ins)) != 0)
	{
		SBInstr *instr = sbnk->AddInstr(ins->GetBank(), ins->GetProg());
		if (instr == 0)
			continue;

		instr->instrName = ins->info.GetInfo(DLS_INAM_CHUNK);

		art = 0;
		while ((art = ins->lart.EnumArt(art)) != 0)
		{
			modval = art->GetConnection(CONN_SRC_KEYNUMBER, CONN_SRC_NONE, CONN_DST_EG1_DECAYTIME, 0) / 65536.0;
			if (modval != 0)
			{
				for (int n = 0; n < 128; n++)
					instr->keyDecRate[n] = SoundBank::EnvRate(((double)n / 128.0) * modval);
			}
			modval = art->GetConnection(CONN_SRC_KEYONVELOCITY, CONN_SRC_NONE, CONN_DST_EG1_ATTACKTIME, 0) / 65536.0;
			if (modval != 0)
			{
				for (int v = 0; v < 128; v++)
					instr->velAtkRate[v] = SoundBank::EnvRate(((double)v / 128.0) * modval);
			}
		}

		rgn = 0;
		while ((rgn = ins->rgnlist.EnumRgn(rgn)) != 0)
		{
			SBZone *zone = instr->AddZone();

			zone->keyNum = rgn->wsmp.unityNote;
			zone->cents = rgn->wsmp.fineTune;
			zone->lowKey = rgn->rgnh.rangeKey.low;
			zone->highKey = rgn->rgnh.rangeKey.high;
			zone->lowVel = rgn->rgnh.rangeVel.low;
			zone->highVel = rgn->rgnh.rangeVel.high;
			zone->chan = (rgn->wlnk.channel & 2) ? 1 : 0;
			if (rgn->wsmpValid)
			{
				zone->volAtten = DLSAttenuation(rgn->wsmp.attenuation);
				zone->mode = rgn->wsmp.sampleLoops > 0;
				zone->loopStart = rgn->loop.start;
				zone->loopLen = rgn->loop.length;
				zone->loopEnd = zone->loopStart + zone->loopLen;
			}

			// Apply instrument articulation
			art = 0;
			while ((art = ins->lart.EnumArt(art)) != 0)
				ApplyArt(zone, art);

			// Apply zone articulation
			art = 0;
			while ((art = rgn->lart.EnumArt(art)) != 0)
			{
				ApplyArt(zone, art);
				//zone->volAtten = DLSAttenuation(art->GetDefault(CONN_DST_ATTENUATION, rgn->wsmp.attenuation));
				modval = art->GetConnection(CONN_SRC_KEYNUMBER, CONN_SRC_NONE, CONN_DST_EG1_DECAYTIME, 0) / 65536.0;
				if (modval != 0)
				{
					for (int n = zone->lowKey; n <= zone->highKey; n++)
						instr->keyDecRate[n] = SoundBank::EnvRate(((double)n / 128.0) * modval);
				}
			}

			// Connect to sample
			DLSWaveInfo *wv = info.wvpl.FindWave(rgn->wlnk.tableIndex);
			if (wv)
			{
				// is the info in the wv->wvfmt the same as
				// what is in the region?
				zone->rate = wv->wvfmt.sampleRate;
				if (zone->cents == 0)
					zone->cents = wv->wvsmp.fineTune;
				if (zone->keyNum == 0)
					zone->keyNum = wv->wvsmp.unityNote;
				zone->sample = wv->samples;
				zone->sampleNdx = wv->index;
				zone->sampleLen = wv->sampsize;
				zone->tableEnd = wv->sampsize;
				zone->name = wv->info.GetInfo(DLS_INAM_CHUNK);
				zone->recFreq = synthParams.GetFrequency(zone->keyNum - 12);
				if (zone->cents != 0)
					zone->recFreq *= synthParams.GetCentsMult(-zone->cents);
				zone->recPeriod = (bsInt32) (zone->rate / zone->recFreq);
				if (!rgn->wsmpValid)
				{
					zone->volAtten = DLSAttenuation(rgn->wsmp.attenuation);
					zone->mode = wv->wvsmp.sampleLoops > 0;
					zone->loopStart = wv->loop.start;
					zone->loopLen = wv->loop.length;
					zone->loopEnd = zone->loopStart + zone->loopLen;
				}
			}
		}

		instr->InitZoneMap();
		instr->loaded = preload;
	}

	return sbnk;
}

#define TC0 -786432000L  // (-12000*65536) 

void DLSFile::ApplyArt(SBZone *zone, DLSArtInfo *art)
{
	//zone->volAtten = DLSRelGain(art->GetDefault(CONN_DST_ATTENUATION, 0));

	zone->vibRate = DLSPitchCents(art->GetDefault(CONN_DST_LFO_FREQUENCY, 0));
	zone->vibDelay = DLSTimeCents(art->GetDefault(CONN_DST_LFO_STARTDELAY, TC0));
	zone->vibAmount = DLSCents(art->GetConnection(CONN_SRC_LFO, CONN_SRC_NONE, CONN_DST_PITCH, 0));

	zone->volEg.attack = DLSTimeCents(art->GetDefault(CONN_DST_EG1_ATTACKTIME, TC0));
	zone->volEg.hold = DLSTimeCents(art->GetDefault(CONN_DST_EG1_HOLDTIME, TC0));
	zone->volEg.decay = DLSTimeCents(art->GetDefault(CONN_DST_EG1_DECAYTIME, TC0));
	zone->volEg.release = DLSTimeCents(art->GetDefault(CONN_DST_EG1_RELEASETIME, TC0));
	zone->volEg.sustain = DLSPercent(art->GetDefault(CONN_DST_EG1_SUSTAINLEVEL, 0));

	zone->modEg.attack = DLSTimeCents(art->GetDefault(CONN_DST_EG2_ATTACKTIME, TC0));
	zone->modEg.hold = DLSTimeCents(art->GetDefault(CONN_DST_EG2_HOLDTIME, TC0));
	zone->modEg.decay = DLSTimeCents(art->GetDefault(CONN_DST_EG2_DECAYTIME, TC0));
	zone->modEg.release = DLSTimeCents(art->GetDefault(CONN_DST_EG2_RELEASETIME, TC0));
	zone->modEg.sustain = DLSPercent(art->GetDefault(CONN_DST_EG2_SUSTAINLEVEL, 0));

	zone->pan = DLSPercent(art->GetDefault(CONN_DST_PAN, 0));

}
