/////////////////////////////////////////////////////////////////////
// BasicSynth Example09a (Chapter 16)
//
// MIDI file playback
//
// This program loads a GM soundbank (SF2 or DLS) and a MIDI file (.mid)
// and produces a wave file.
//
// use: Example09 [-vn] [-s] soundfont.sf2 infile.mid outfile.wav
// -v = master volume level
// -s = silent output
//
// Copyright 2009, Daniel R. Mitchell
/////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <BasicSynth.h>
#include <Instruments.h>
#include <SFFile.h>
#include <DLSFile.h>
#include <SMFFile.h>
#include <MIDIControl.h>
#include <GMPlayer.h>

void useage()
{
	fprintf(stderr, "use: example9a [-vn] [-s] soundbank midifile wavefile\n");
	fprintf(stderr, "    -vn = master volume level, default 1.0\n");
	fprintf(stderr, "    -s  = don't print info or time\n");
	exit(1);
}

int verbose = 1;
time_t clkTime = 0;
long genTime = 0;
void GenCallback(bsInt32 count, Opaque arg)
{
	if (verbose)
		fprintf(stderr, "Time %02d:%02d\r", (genTime / 60), (genTime %60));
	genTime++;
}

int main(int argc, char *argv[])
{
	InitSynthesizer();

	bsString midFile;
	bsString sbFile;
	bsString wavFile;
	AmpValue vol = 2.0;

	MIDIControl mc;
	WaveFile wvf;
	Sequencer seq;
	Mixer mix;
	InstrManager inmgr;
	SFFile sf;
	DLSFile dls;
	SMFFile smf;
	SMFInstrMap map[16];
	int i;
	int nchnls;
	bsInt32 chnls[16];

	int argn = 1;
	char *ap = argv[argn];
	while (argn < argc && *ap == '-')
	{
		if (ap[1] == 'v')
			vol = atof(&ap[2]);
		else if (ap[1] == 's')
			verbose = 0;
		else
			useage();
		argn++;
		ap = argv[argn];
	}

	if (argn < argc)
		sbFile = argv[argn++];
	if (argn < argc)
		midFile = argv[argn++];
	if (argn < argc)
		wavFile = argv[argn];
	if (sbFile.Length() == 0 || midFile.Length() == 0 || wavFile.Length() == 0)
		useage();

	SoundBank *sb = 0;
	if (SFFile::IsSF2File(sbFile))
		sb = sf.LoadSoundBank(sbFile, 0, 0.375);
	else if (DLSFile::IsDLSFile(sbFile))
		sb = dls.LoadSoundBank(sbFile, 0, 1.0);
	if (!sb)
	{
		fprintf(stderr, "Failed to load soundbank %s\n", (const char *)sbFile);
		exit(1);
	}
	sb->Lock();

	if (verbose)
	{
		printf("Soundbank:\n%s\nVersion %d.%d.%d.%d\n%s\n\n", (const char*)sb->info.szName, 
			sb->info.wMajorFile, sb->info.wMinorFile, 
			sb->info.wMajorVer, sb->info.wMinorVer, 
			(const char*)sb->info.szCopyright);
	}

	if (smf.LoadFile(midFile))
	{
		fprintf(stderr, "Errror loading .mid file %s\n", (const char *)midFile);
		exit(1);
	}

	if (verbose)
		printf("Song:\n%s%s%s\n%s\n", smf.SeqName(), smf.Copyright(), smf.TimeSignature(), smf.KeySignature());

	// This shows using one player config for all channels.
	// When	using the GMManager, instruments get the patch
	// directly from the MIDIChannelControl object. Envelope
	// is set from the SF2/DLS file.

	// Add GMManager type to instrument manager
	InstrMapEntry *ime = inmgr.AddType("GMPlayer",
		GMManager::InstrFactory,
		GMManager::EventFactory,
		GMManager::TmpltFactory);
	ime->dumpTmplt = GMManager::TmpltDump;

	// Create the template instrument
	GMManager *instr = new GMManager;
	instr->SetMidiControl(&mc);
	instr->SetSoundFile(sb->name);
	instr->SetSoundBank(sb);

	// Add it as template to the available instrument list.
	InstrConfig *inc = inmgr.AddInstrument(0, ime, instr);
	for (int i = 0; i < 16; i++)
	{
		map[i].inc = inc;
		// The bank and patch params can be used to send
		// the preset number to the instrument in the START event. 
		//map[i].bnkParam = ??;
		//map[i].preParam = ??;
		map[i].bnkParam = -1;
		map[i].preParam = -1;
	}
	mc.channel[9].bank = 128; // channel 10 is always drum bank

	smf.GenerateSeq(&seq, &map[0], sb);
	nchnls = smf.GetChannelMap(chnls);
	if (nchnls == 0)
	{
		fprintf(stderr, "No notes found!\n");
		exit(0);
	}

	// Initialize the mixer
	mix.MasterVolume(vol, vol);
	mix.SetChannels(nchnls);
	for (i = 0; i < nchnls; i++)
	{
		mix.ChannelVolume(i, 1.0);
		mix.ChannelOn(i, chnls[i] > 0);
	}

	// run the sequencer
	wvf.OpenWaveFile(wavFile, 2);
	inmgr.Init(&mix, &wvf);
	seq.SetController(&mc.seqControl);
	seq.SetCB(GenCallback, synthParams.isampleRate, 0);
	genTime = 0;
	time(&clkTime);
	seq.Sequence(inmgr, 0, 0);
	wvf.CloseWaveFile();

	if (verbose)
	{
		long clkTimeDiff = (long) (time(0) - clkTime);
		printf("\n%02d:%02d in %02ld:%02ld %d%%\n", 
			genTime / 60, genTime % 60, clkTimeDiff / 60, clkTimeDiff % 60,
			(clkTimeDiff * 100) / genTime);
	}

	sb->Unlock();

	return 0;
}
