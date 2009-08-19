/////////////////////////////////////////////////////////////////////
// BasicSynth Example09a (Chapter 16)
//
// MIDI file loading and playback
//
// use: Example09 [-vn] soundfont.sf2 infile.mid outfile.wav
// -v = master volume level
//
// Copyright 2008, Daniel R. Mitchell
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
	fprintf(stderr, "use: example9a [-vn] soundfount midifile wavfile\n");
	exit(1);
}

time_t clkTime = 0;
long genTime = 0;
void GenCallback(bsInt32 count, Opaque arg)
{
	fprintf(stderr, "Time %02d:%02d\r", (genTime / 60), (genTime %60));
	genTime++;
}

// Specialized instrument manager that bypasses the mixer.
// We can do this because the volume levels are controlled
// by the MIDI CC# 7 on a per-channel basis. The mixer inputs
// are therefore, redunant.
class GMInstrManager : public InstrManager
{
protected:
	AmpValue outLft;
	AmpValue outRgt;
	AmpValue masterVol;
//	GMManager *gm;

public:
	GMInstrManager()
	{
		masterVol = 1.0;
//		gm = new GMManager;
	}

	~GMInstrManager()
	{
//		delete gm;
	}

	void SetVolume(AmpValue v)
	{
		masterVol = v;
	}

/*	virtual Instrument *Allocate(bsInt16 inum)
	{
		return GMManager::InstrFactory(this, gm);
	}

	virtual Instrument *Allocate(InstrConfig *in)
	{
		return GMManager::InstrFactory(this, gm);
	}

	virtual void Deallocate(Instrument *ip)
	{
		ip->Destroy();
	}

	virtual SeqEvent *ManufEvent(bsInt16 inum)
	{
		return GMManager::EventFactory(gm);
	}

	virtual SeqEvent *ManufEvent(InstrConfig *in)
	{
		return GMManager::EventFactory(gm);
	}
*/
	/// Start is called by the sequencer when the sequence starts.
	virtual void Start()
	{
		outLft = 0;
		outRgt = 0;
	}
	
	virtual void Tick()
	{
		wvf->Output2(masterVol * outLft, masterVol * outRgt);
		outLft = 0;
		outRgt = 0;
	}

	virtual void FxSend(int unit, AmpValue val)
	{
		// TODO: send to reverb/chorus
	}

	virtual void Output(int ch, AmpValue val)
	{
		outLft += val;
		outRgt += val;
	}

	virtual void Output2(int ch, AmpValue lft, AmpValue rgt)
	{
		outLft += lft;
		outRgt += rgt;
	}
};

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
	//InstrManager inmgr;
	GMInstrManager inmgr;
	SFFile sf;
	DLSFile dls;
	SMFFile smf;
	SMFInstrMap map[16];
	int i;
	int nchnls;
	bsInt32 chnls[16];

	int argn = 1;
	char *ap = argv[argn];
	if (argc > 1 && *ap == '-' && ap[1] == 'v')
	{
		vol = atof(&ap[2]);
		argn++;
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

	if (smf.LoadFile(midFile))
	{
		fprintf(stderr, "Errror loading .mid file %s\n", (const char *)midFile);
		exit(1);
	}

	// This shows using one player config for all channels.
	// When	using the GMManager, instruments get the patch
	// directly from the MIDIChannelControl object. Envelope
	// is set from the SF2 file.
	// Alternatively, create a separate instrument for
	// each channel, and set the bank, patch and envelope
	// from the SeqEvent object passed in on start, or
	// ignore the program change and go with a fixed
	// synthesis sound on each channel.

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

	fprintf(stderr, "%s%s%s%s\n", smf.SeqName(), smf.Copyright(), smf.TimeSignature(), smf.KeySignature());

	inmgr.SetVolume(vol);

	// Initialize the mixer
	mix.MasterVolume(vol, vol);
	mix.SetChannels(nchnls);
	for (i = 0; i < nchnls; i++)
	{
		mix.ChannelVolume(i, 1.0);
		mix.ChannelOn(i, chnls[i] > 0);
		//printf("%d events on channel %d\n", chnls[i], i);
	}

	wvf.OpenWaveFile(wavFile, 2);
	inmgr.Init(&mix, &wvf);
	seq.SetController(&mc.seqControl);
	seq.SetCB(GenCallback, synthParams.isampleRate, 0);
	genTime = 0;
	time(&clkTime);
	seq.Sequence(inmgr, 0, 0);
	wvf.CloseWaveFile();

	long clkTimeDiff = (long) (time(0) - clkTime);
	printf("\n%02d:%02d in %02ld:%02ld %d%%\n", 
		genTime / 60, genTime % 60, clkTimeDiff / 60, clkTimeDiff % 60,
		(clkTimeDiff * 100) / genTime);

	return 0;
}
