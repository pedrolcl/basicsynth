///////////////////////////////////////////////////////////
// Instruments test
//
// ToneInstr
// SubSynth
// AddSynth
// FMSynth
// MatixSynth
// WFSynth
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <BasicSynth.h>
#include <Instruments.h>

#pragma warning(disable : 4996)

//#define ADD_REVERB 1

long evidcount = 1;
float startTime = 0;
InstrManager inMgr;
Sequencer seq;
Mixer mix;
Reverb2 rvrb;
WaveFile wvf;


static void AddEvent(bsInt16 inum, int pit, float dur)
{
	SeqEvent *evt;
	evt = inMgr.ManufEvent(inum);
	evt->evid = evidcount++;
	evt->type = SEQEVT_START;
	evt->SetParam(P_INUM, (float)inum);
	evt->SetParam(P_CHNL, 0.0);
	evt->SetParam(P_START, startTime);
	evt->SetParam(P_DUR, dur);
	evt->SetParam(P_PITCH, (float)pit);
	evt->SetParam(P_VOLUME, 0.7);
	startTime += dur;
	seq.AddEvent(evt);
}

static void AddSequence(bsInt16 inum, float dur)
{
	for (int pit = 48; pit <= 53; pit++)
		AddEvent(inum, pit, dur);
	startTime += 0.1;
}

static void DestroyTemplate(Opaque tp)
{
	Instrument *ip = (Instrument *)tp;
	delete ip;
}

int main(int argc, char *argv[])
{
#if defined(_WINDOWS)
	CoInitialize(0);
#endif

	char *fname = "data.xml";
	if (argc > 1)
		fname = argv[1];

	InitSynthesizer();

	mix.SetChannels(2);
	mix.MasterVolume(1.0, 1.0);
	mix.ChannelOn(0, 1);
	mix.ChannelOn(1, 1);
	mix.ChannelVolume(0, 1.0);
	mix.ChannelVolume(1, 1.0);
#ifdef ADD_REVERB
	mix.SetFxChannels(1);
	mix.FxInit(0, &rvrb, 0.1);
	mix.FxLevel(0, 0, 0.2);
	mix.FxLevel(0, 1, 0.2);
	rvrb.InitReverb(1.0, 2.0);
#endif

	inMgr.Init(&mix, &wvf);

	inMgr.AddType("Tone", ToneInstr::ToneFactory, ToneInstr::ToneEventFactory);
	inMgr.AddType("ToneFM", ToneFM::ToneFMFactory, ToneFM::ToneFMEventFactory);
	inMgr.AddType("AddSynth", AddSynth::AddSynthFactory, AddSynth::AddSynthEventFactory);
	inMgr.AddType("SubSynth", SubSynth::SubSynthFactory, SubSynth::SubSynthEventFactory);
	inMgr.AddType("FMSynth", FMSynth::FMSynthFactory, FMSynth::FMSynthEventFactory);
	inMgr.AddType("MatrixSynth", MatrixSynth::MatrixSynthFactory, MatrixSynth::MatrixSynthEventFactory);
	inMgr.AddType("WFSynth", WFSynth::WFSynthFactory, WFSynth::WFSynthEventFactory);

	XmlSynthDoc doc;
	XmlSynthElem *root = doc.Open(fname);
	if (!root)
	{
		printf("Cannot open file %s\n", fname);
		exit(1);
	}

	// Optional: use LoadInstrLib(inMgr, fname)
	// but we want to discover the inum values
	// and add sequences programaticaly...

	XmlSynthElem *next;
	XmlSynthElem *inst = root->FirstChild();
	while (inst != NULL)
	{
		if (inst->TagMatch("instr"))
		{
			InstrMapEntry *ent = LoadInstr(inMgr, inst);
			ent->dumpTmplt = DestroyTemplate;
			if (strcmp(ent->GetType(), "WFSynth") == 0)
				AddEvent(ent->inum, 48, 1.0);
			else
				AddSequence(ent->inum, 0.25);
		}

		next = inst->NextSibling();
		delete inst;
		inst = next;
	}
	doc.Close();

	if (wvf.OpenWaveFile("example10.wav", 2))
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}
	seq.Sequence(inMgr);

#ifdef ADD_REVERB
	// drain the reverb...
	AmpValue lv;
	AmpValue rv;
	long n = synthParams.isampleRate;
	while (n-- > 0)
	{
		mix.Out(&lv, &rv);
		wvf.Output2(lv, rv);
	}
#endif

	wvf.CloseWaveFile();

	///////////////////////////////////////////////////////////////
	// Code to test instrument save functions...
/*	root = doc.NewDoc("instrlib");
	InstrMapEntry *ime = inMgr.EnumInstr(0);
	while (ime)
	{
		Instrument *ip = (Instrument *)ime->instrTmplt;
		if (ip)
		{
			inst = root->AddChild("instr");
			inst->SetAttribute("id", ime->inum);
			inst->SetAttribute("type", ime->itype);
			inst->SetAttribute("name", ime->iname);
			inst->SetAttribute("desc", ime->idesc);
			ip->Save(inst);
			delete inst;
		}
		ime = inMgr.EnumInstr(ime);
	}
	bsString outxml;
	outxml = "_";
	outxml += fname;
	doc.Save((char *) (const char *)outxml);*/
	///////////////////////////////////////////////////////////////

	return 0;
}
