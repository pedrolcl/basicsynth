//////////////////////////////////////////////////////////////////////
// BasicSynth - Modular Synthesis instrument 
//
// Maintains a list of unit generators and scans the list on each tick.
// In general, each ug implements a wrapper around one of the BasicSynth
// library GenUnit classes.
//
// Type          Use
//--------------------------------------------------
// OSCIL         Generic wavetable oscillator
// OSCILI        Wvetable oscillator using interpolation
// OSCILFM       Two oscillator FM generator
// OSCILAM       Two oscillator AM generator
// BUZZ          Bandwidth limited pulse wave
// ENVSEGN       Variable number of segments envelope
// ENVAR         Attack/Release envelope generator
// ENVADSR       Attack/Decay/Sustain/Release envelope generator
// LOWPASS       Lowpass filter using bi-quad form
// HIPASS        Highpass filter using bi-quad form
// LOWPASSR      Two-pole lowpass filter with resonant peak
// RESON         Constant gain resononant filter (bandpass)
// ALLPASS       First order Allpass filter
// FILTFIRN      N-th order FIR filter using convolution, highpass or lowpass
// DELAY         Delay line
// DELAYR        Recirculating delay line (a/k/a FDL, comb filter)
// DELAYV        Variable length delay line
// REVERB        Schroeder style reverb, 4 comb filters + 2 allpass filters
// FLANGER       Dynamic variable delay line with feedback
// RANDH         Random number generator (noise) with configurable sample rate
// RANDI         Random number generator (noise) with interpolation
//
// VALUE         Psuedo-generator used to hold a parmater value
// CALC          Calculates a value
// EXPR          Expression evaluator (TBD)
// OUT           Output object with panning
//
// The ModSynth instrument automatically creates parameters (VALUE nodes) for
// sample rate, note pitch, frequency, volume and duration. Internally generated
// nodes have a name beginning with '@' and are never copied or saved.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "Includes.h"
#include "ModSynth.h"

UGParam oscilParams[] = 
{
	{ UGOSC_AMP, UGP_ALL, "amplitude", 0.0f, 1.0f }, 
	{ UGOSC_FRQ, UGP_ALL, "frequency", 0.0f, 20000.0f }, 
	{ UGOSC_MUL, UGP_ALL, "multiple",  0.0f, 10.0f }, 
	{ UGOSC_MOD, UGP_ALL, "modulate" }, 
	{ UGOSC_WVT, UGP_ALL, "wavetable" }, 
	{ 0, UGP_RUN, "in" },
	{ -1, 0, 0 }
};

UGParam oscil2Params[] = 
{
	{ UGOSC_AMP, UGP_ALL, "amplitude", 0.0f, 1.0f }, 
	{ UGOSC_FRQ, UGP_ALL, "frequency", 0.0f, 20000.0f }, 
	{ UGOSC_MUL, UGP_ALL, "multiple",  0.0f, 10.0f }, 
	{ UGOSC_MOD, UGP_ALL, "modulate" }, 
	{ UGOSC_WVT, UGP_ALL, "wavetable"  }, 
	{ UGOSC_IOM, UGP_ALL, "iom", 0.0f, 50.0f }, 
	{ 0, UGP_RUN, "in" },
	{ -1, 0, 0 }
};

UGParam buzzParams[] = 
{
	{ UGOSC_AMP, UGP_ALL, "amplitude", 0.0f, 1.0f }, 
	{ UGOSC_FRQ, UGP_ALL, "frequency", 0.0f, 20000.0f }, 
	{ UGOSC_MUL, UGP_ALL, "harmonics" }, 
	{ UGOSC_MOD, UGP_ALL, "modulate" }, 
	{ 0, UGP_RUN, "in" },
	{ -1, 0, 0 }
};

UGParam noiseParams[] = 
{
	{ UGNZ_AMP, UGP_ALL, "amplitude", 0.0f, 1.0f },
	{ UGNZ_RTE, UGP_ALL, "rate",      0.0f, 1.0f },
	{ 0, UGP_RUN|UGP_DEF, "in" },
	{ -1, 0, 0 }
};

UGParam envARParams[] =
{
	{ UGAR_INP, UGP_RUN, "in" },
	{ UGAR_ATK, UGP_SAVE|UGP_LOAD|UGP_INIT, "attack" },
	{ UGAR_SUS, UGP_SAVE|UGP_LOAD|UGP_INIT, "peak" },
	{ UGAR_REL, UGP_SAVE|UGP_LOAD|UGP_INIT, "release" },
	{ UGAR_SON, UGP_SAVE|UGP_LOAD|UGP_INIT, "sustain" },
	{ UGAR_TYP, UGP_SAVE|UGP_LOAD|UGP_INIT, "curve" }, 
	{ UGAR_FIX, UGP_SAVE|UGP_LOAD|UGP_INIT, "time" },
	{ UGAR_SCL, UGP_SAVE|UGP_LOAD|UGP_INIT, "scale" },
	{ -1, 0, 0 }
};

UGParam envADSRParams[] =
{
	{ UGADSR_INP, UGP_RUN, "in" },
	{ UGADSR_STL, UGP_SAVE|UGP_LOAD|UGP_INIT, "start" },
	{ UGADSR_ATK, UGP_SAVE|UGP_LOAD|UGP_INIT, "attack" },
	{ UGADSR_PKL, UGP_SAVE|UGP_LOAD|UGP_INIT, "peak" },
	{ UGADSR_DEC, UGP_SAVE|UGP_LOAD|UGP_INIT, "decay" },
	{ UGADSR_SUS, UGP_SAVE|UGP_LOAD|UGP_INIT, "sustain" },
	{ UGADSR_REL, UGP_SAVE|UGP_LOAD|UGP_INIT, "release" },
	{ UGADSR_END, UGP_SAVE|UGP_LOAD|UGP_INIT, "end" },
	{ UGADSR_TYP, UGP_SAVE|UGP_LOAD|UGP_INIT, "curve" }, 
	{ UGADSR_FIX, UGP_SAVE|UGP_LOAD|UGP_INIT, "time" }, 
	{ UGADSR_SCL, UGP_SAVE|UGP_LOAD|UGP_INIT, "scale" },
	{ -1, 0, 0 }
};

UGParam envSegNParams[] =
{
	{ UGEG_INP,  UGP_RUN, "in" },
	{ UGEG_SEGN, UGP_SAVE|UGP_LOAD, "segments" },
	{ UGEG_ST,   UGP_SAVE|UGP_LOAD|UGP_INIT, "start" },
	{ UGEG_SUS,  UGP_SAVE|UGP_LOAD|UGP_INIT, "sustain" },
	{ UGEG_SCL,  UGP_SAVE|UGP_LOAD|UGP_INIT, "scale" },
	{ UGEG_SEGLVL(0), UGP_SAVE|UGP_LOAD|UGP_INIT, "s0.level" },
	{ UGEG_SEGRTE(0), UGP_SAVE|UGP_LOAD|UGP_INIT, "s0.rate" },
	{ UGEG_SEGTYP(0), UGP_SAVE|UGP_LOAD|UGP_INIT, "s0.curve" }, 
	{ UGEG_SEGFIX(0), UGP_SAVE|UGP_LOAD|UGP_INIT, "s0.time" }, 
	{ UGEG_SEGLVL(1), UGP_SAVE|UGP_LOAD|UGP_INIT, "s1.level" },
	{ UGEG_SEGRTE(1), UGP_SAVE|UGP_LOAD|UGP_INIT, "s1.rate" },
	{ UGEG_SEGTYP(1), UGP_SAVE|UGP_LOAD|UGP_INIT, "s1.curve" }, 
	{ UGEG_SEGFIX(1), UGP_SAVE|UGP_LOAD|UGP_INIT, "s1.time" }, 
	{ UGEG_SEGLVL(2), UGP_SAVE|UGP_LOAD|UGP_INIT, "s2.level" },
	{ UGEG_SEGRTE(2), UGP_SAVE|UGP_LOAD|UGP_INIT, "s2.rate" },
	{ UGEG_SEGTYP(2), UGP_SAVE|UGP_LOAD|UGP_INIT, "s2.curve" }, 
	{ UGEG_SEGFIX(2), UGP_SAVE|UGP_LOAD|UGP_INIT, "s2.time" }, 
	{ UGEG_SEGLVL(3), UGP_SAVE|UGP_LOAD|UGP_INIT, "s3.level" },
	{ UGEG_SEGRTE(3), UGP_SAVE|UGP_LOAD|UGP_INIT, "s3.rate" },
	{ UGEG_SEGTYP(3), UGP_SAVE|UGP_LOAD|UGP_INIT, "s3.curve" }, 
	{ UGEG_SEGFIX(3), UGP_SAVE|UGP_LOAD|UGP_INIT, "s3.time" }, 
	{ UGEG_SEGLVL(4), UGP_SAVE|UGP_LOAD|UGP_INIT, "s4.level" },
	{ UGEG_SEGRTE(4), UGP_SAVE|UGP_LOAD|UGP_INIT, "s4.rate" },
	{ UGEG_SEGTYP(4), UGP_SAVE|UGP_LOAD|UGP_INIT, "s4.curve" }, 
	{ UGEG_SEGFIX(4), UGP_SAVE|UGP_LOAD|UGP_INIT, "s4.time" }, 
	{ UGEG_SEGLVL(5), UGP_SAVE|UGP_LOAD|UGP_INIT, "s5.level" },
	{ UGEG_SEGRTE(5), UGP_SAVE|UGP_LOAD|UGP_INIT, "s5.rate" },
	{ UGEG_SEGTYP(5), UGP_SAVE|UGP_LOAD|UGP_INIT, "s5.curve" }, 
	{ UGEG_SEGFIX(5), UGP_SAVE|UGP_LOAD|UGP_INIT, "s5.time" }, 
	{ UGEG_SEGLVL(6), UGP_SAVE|UGP_LOAD|UGP_INIT, "s6.level" },
	{ UGEG_SEGRTE(6), UGP_SAVE|UGP_LOAD|UGP_INIT, "s6.rate" },
	{ UGEG_SEGTYP(6), UGP_SAVE|UGP_LOAD|UGP_INIT, "s6.curve" }, 
	{ UGEG_SEGFIX(6), UGP_SAVE|UGP_LOAD|UGP_INIT, "s6.time" }, 
	{ UGEG_SEGLVL(7), UGP_SAVE|UGP_LOAD|UGP_INIT, "s7.level" },
	{ UGEG_SEGRTE(7), UGP_SAVE|UGP_LOAD|UGP_INIT, "s7.rate" },
	{ UGEG_SEGTYP(7), UGP_SAVE|UGP_LOAD|UGP_INIT, "s7.curve" }, 
	{ UGEG_SEGFIX(7), UGP_SAVE|UGP_LOAD|UGP_INIT, "s7.time" }, 
	{ UGEG_SEGLVL(8), UGP_SAVE|UGP_LOAD|UGP_INIT, "s8.level" },
	{ UGEG_SEGRTE(8), UGP_SAVE|UGP_LOAD|UGP_INIT, "s8.rate" },
	{ UGEG_SEGTYP(8), UGP_SAVE|UGP_LOAD|UGP_INIT, "s8.curve" }, 
	{ UGEG_SEGFIX(8), UGP_SAVE|UGP_LOAD|UGP_INIT, "s8.time" }, 
	{ UGEG_SEGLVL(9), UGP_SAVE|UGP_LOAD|UGP_INIT, "s9.level" },
	{ UGEG_SEGRTE(9), UGP_SAVE|UGP_LOAD|UGP_INIT, "s9.rate" },
	{ UGEG_SEGTYP(9), UGP_SAVE|UGP_LOAD|UGP_INIT, "s9.curve" }, 
	{ UGEG_SEGFIX(9), UGP_SAVE|UGP_LOAD|UGP_INIT, "s9.time" }, 
	{ UGEG_SEGLVL(10), UGP_SAVE|UGP_LOAD|UGP_INIT, "s10.level" },
	{ UGEG_SEGRTE(10), UGP_SAVE|UGP_LOAD|UGP_INIT, "s10.rate" },
	{ UGEG_SEGTYP(10), UGP_SAVE|UGP_LOAD|UGP_INIT, "s10.curve" }, 
	{ UGEG_SEGFIX(10), UGP_SAVE|UGP_LOAD|UGP_INIT, "s10.time" }, 
	{ UGEG_SEGLVL(11), UGP_SAVE|UGP_LOAD|UGP_INIT, "s11.level" },
	{ UGEG_SEGRTE(11), UGP_SAVE|UGP_LOAD|UGP_INIT, "s11.rate" },
	{ UGEG_SEGTYP(11), UGP_SAVE|UGP_LOAD|UGP_INIT, "s11.curve" }, 
	{ UGEG_SEGFIX(11), UGP_SAVE|UGP_LOAD|UGP_INIT, "s11.time" }, 
	{ -1, 0, 0 }
};

UGParam filterParams1[] = 
{
	{ 0, UGP_GEN, "in" }, 
	{ UGFLT_FRQ, UGP_ALL, "fc" }, 
	{ UGFLT_GAIN, UGP_ALL, "gain" },
	{ UGFLT_CRT, UGP_INIT|UGP_SAVE|UGP_LOAD, "cr" },
	{ -1, 0, 0 }
};

UGParam filterParams2[] = 
{
	{ 0, UGP_GEN, "in" },
	{ UGFLT_FRQ, UGP_ALL, "fc" },
	{ UGFLT_RES, UGP_ALL, "res" },
	{ UGFLT_CRT, UGP_INIT|UGP_SAVE|UGP_LOAD, "cr" },
	{ -1, 0, 0 }
};

// used for FIRn and Averaging filters
UGParam filterParams3[] = 
{
	{ 0, UGP_GEN, "in" },
	{ UGFLT_FRQ, UGP_ALL, "fc" },
	{ UGFLT_ORD, UGP_INIT|UGP_SAVE|UGP_LOAD, "order" },
	{ UGFLT_CRT, UGP_INIT|UGP_SAVE|UGP_LOAD, "cr" },
	{ UGFLT_LHP, UGP_INIT|UGP_SAVE|UGP_LOAD, "hp" },
	{ -1, 0, 0 }
};

UGParam allpassParams[] =
{
	{ 0, UGP_RUN, "in" },
	{ 1, UGP_ALL, "d" },
	{ -1, 0, 0 }
};

UGParam valOut[] = 
{
	{ UGOUT_INP, UGP_GEN, "in" },
	{ UGOUT_LFT, UGP_GEN, "left" },
	{ UGOUT_RGT, UGP_GEN, "right" },
	{ UGOUT_VOL, UGP_ALL, "volume" },
	{ UGOUT_PAN, UGP_ALL, "pan" },
	{ UGOUT_PON, UGP_ALL, "pon" },
	{ -1, 0, 0 }
};

UGParam valueParam[] = 
{
	{ UGVAL_INP, UGP_GEN|UGP_INIT, "in" },
	{ -1, 0, 0 }
};

UGParam calcParams[] = 
{
	{ UGCALC_V1, UGP_ALL, "val1" },
	{ UGCALC_V2, UGP_ALL, "val2" },
	{ UGCALC_OP, UGP_ALL, "op" },
	{ 0, UGP_RUN|UGP_DEF, "in" },
	{ -1, 0, 0 }
};

UGParam scaleParams[] = 
{
	{ UGSCL_INP, UGP_ALL, "in" },
	{ UGSCL_OMX, UGP_ALL, "outmax" },
	{ UGSCL_OMN, UGP_ALL, "outmin" },
	{ UGSCL_IMX, UGP_ALL, "inmax" },
	{ UGSCL_IMN, UGP_ALL, "inmin" },
	{ -1, 0, 0 }
};

UGParam tableParams[] =
{
	{ UGTBL_NDX, UGP_ALL, "index" },
	{ UGTBL_WVT, UGP_ALL, "wavetable" },
	{ UGTBL_INT, UGP_ALL, "interp" },
	{ -1, 0, 0 }
};

UGParam delayParams[] =
{
	{ UGDLY_INP, UGP_RUN, "in" },
	{ UGDLY_DLY, UGP_INIT|UGP_SAVE|UGP_LOAD, "delay" },
	{ UGDLY_DEC, UGP_INIT|UGP_SAVE|UGP_LOAD, "decay" },
	{ UGDLY_VOL, UGP_ALL, "volume" },
	{ -1, 0, 0 }
};

UGParam delayParamsV[] =
{
	{ UGDLY_INP, UGP_RUN, "in" },
	{ UGDLY_DLY, UGP_INIT|UGP_SAVE|UGP_LOAD, "delay" },
	{ UGDLY_DEC, UGP_INIT|UGP_SAVE|UGP_LOAD, "decay" },
	{ UGDLY_VOL, UGP_ALL, "volume" },
	{ UGDLY_VRT, UGP_ALL, "vrt" },
	{ -1, 0, 0 }
};

UGParam flangerParams[] =
{
	{ UGFLNG_INP, UGP_RUN, "in" },
	{ UGFLNG_LVL, UGP_INIT|UGP_SAVE|UGP_LOAD, "level" },
	{ UGFLNG_MIX, UGP_INIT|UGP_SAVE|UGP_LOAD, "mix" },
	{ UGFLNG_FB,  UGP_INIT|UGP_SAVE|UGP_LOAD, "feedback" },
	{ UGFLNG_CTR, UGP_INIT|UGP_SAVE|UGP_LOAD, "center" },
	{ UGFLNG_DPTH,UGP_INIT|UGP_SAVE|UGP_LOAD, "depth" },
	{ UGFLNG_SWP, UGP_INIT|UGP_SAVE|UGP_LOAD, "sweep" },
	{ -1, 0, 0 }
};

UGParam reverbParams[] =
{
	{ UGRVB_INP, UGP_RUN, "in" },
	{ UGRVB_VOL, UGP_INIT|UGP_SAVE|UGP_LOAD, "volume" },
	{ UGRVB_RVT, UGP_INIT|UGP_SAVE|UGP_LOAD, "rtime" },
	{ -1, 0, 0 }
};

ModSynthUGType ugTypes[] = 
{
	{ "OSCIL",    "Osc", "Wavetable Oscillator", UGOscil::Construct },
	{ "OSCILI",   "OscI", "Wavetable Oscillator (Interpolation)", UGOscilI::Construct },
	{ "OSCILFM",  "FM",  "Two oscillator FM generator", UGOscilFM::Construct },
	{ "OSCILAM",  "AM",  "Two oscillator AM generator", UGOscilAM::Construct },
	{ "BUZZ",     "BUZZ", "Pulse wave generator", UGBuzz::Construct },
	{ "ENVSEGN",  "EG",  "Multi-segment envelope generator", UGEnvSegN::Construct },
	{ "ENVAR",    "AR",  "AR envelope generator", UGEnvAR::Construct },
	{ "ENVADSR",  "ADSR", "ADSR envelope generator", UGEnvADSR::Construct },
	{ "LOWPASS",  "LP",   "Lowpass filter (bi-quad)", UGLowpass::Construct },
	{ "HIPASS",   "HP",   "Highpass filter (bi-quad)", UGHipass::Construct },
	{ "LOWPASSR", "LPr",  "Lowpass filter (2-pole, resonant)", UGLowpassR::Construct },
	{ "RESON",    "RES",  "Reson filter", UGReson::Construct },
	{ "ALLPASS",  "AP",   "Allpass filter", UGAllpass::Construct },
	{ "FILTFIRN", "FIRn", "Nth-order FIR filter", UGFilterFIRn::Construct },
	{ "DELAY",    "Delay", "Delay line", UGDelay::Construct },
	{ "DELAYR",   "FB-Dly", "Delay line with feedback", UGDelayR::Construct },
	{ "DELAYV",   "V-Dly", "Variable delay line", UGDelayV::Construct },
	{ "REVERB",   "Rvrb", "Schroeder type reverb", UGReverb::Construct },
	{ "FLANGER",  "Flngr", "Flanger/chorus", UGFlanger::Construct },
	{ "RANDH",    "RndH",  "Variable rate noise generator", UGRandH::Construct },
	{ "RANDI",    "RndI",  "Variable rate noise with interpolation", UGRandI::Construct },
	{ "CALC",     "Calc.",  "Calculate a value", UGCalc::Construct },
	{ "SCALE",    "Scale", "Scale a value", UGScale::Construct },
	{ "TABLE",    "Table", "Select values from a wavetable", UGTable::Construct },
	{ "VALUE",    "Value", "Holds a single value", 0 }, // used for default params: dur,vol,frq,pitch
	{ "OUT",      "Out",   "Output with optional panning", 0 /*UGOut::Construct*/ }, //<-- only created internally
	{ 0, 0, 0, 0 }
};


Instrument *ModSynth::ModSynthFactory(InstrManager *im, Opaque tmplt)
{
	ModSynth *ms = new ModSynth;
	ms->im = im;
	if (tmplt)
		ms->Copy((ModSynth*)tmplt);
	return (Instrument*)ms;
}

SeqEvent *ModSynth::ModSynthEventFactory(Opaque tmplt)
{
	VarParamEvent *vp = new VarParamEvent;
	if (tmplt)
	{
		ModSynth *ms = (ModSynth *)tmplt;
		vp->maxParam = ms->GetNumParams()+1;
	}
	else
		vp->maxParam = 1000;
	return (SeqEvent *)vp;
}

bsInt16 ModSynth::MapParamID(const char *name, Opaque tmplt)
{
	if (tmplt == 0)
		return -1;
	bsInt16 id = -1;
	ModSynth *ms = (ModSynth *)tmplt;
	char buf[128];
	strncpy(buf, name, 127);
	buf[127] = 0;
	char *inp = strchr(buf, '.');
	if (inp == NULL)
		inp = "in";
	else
		*inp++ = 0;
	ModSynthUG *ug = ms->FindUnit(buf);
	if (ug)
	{
		id = ug->GetID() << UGID_SHIFT;
		const UGParam *p = ug->FindParam(inp);
		if (p)
			id |= p->index & UGID_MASK;
	}
	return id;
}

const char *ModSynth::MapParamName(bsInt16 id, Opaque tmplt)
{
	static bsString paramNameBuf;

	if (tmplt == 0)
		return "";

	ModSynth *ms = (ModSynth *)tmplt;
	bsInt16 uid = id >> UGID_SHIFT;
	bsInt16 pid = id & UGID_MASK;
	ModSynthUG *ug = ms->FirstUnit();
	while (ug)
	{
		if (ug->GetID() == uid)
		{
			const UGParam *p = ug->GetParamList();
			while (p->name)
			{
				if (p->index == pid)
				{
					paramNameBuf = ug->GetName();
					paramNameBuf += ".";
					paramNameBuf += p->name;
					return paramNameBuf;
				}
				p++;
			}
			break;
		}
		ug = ms->NextUnit(ug);
	}
	return "";
}

ModSynth::ModSynth()
{
	dur = 0;
	chnl = 0;
	frq = 0;
	im = 0;
	head.Insert(&tail);
	head.SetName("@sr");
	head.SetInput(0, synthParams.sampleRate);
	tail.SetName("out");
	tail.SetID(1);
	tail.InitDefault();
	numParam = tail.GetNumInputs();
	frqParam = AddParam("@frq", P_FREQ);
	volParam = AddParam("@vol", P_VOLUME);
	pitParam = AddParam("@pitch", P_PITCH);
	durParam = AddParam("@dur", P_DUR);
	numUnits = 0;
}

ModSynth::~ModSynth()
{
	ModSynthUG *ug;
	while ((ug = head.next) != &tail)
	{
		ug->Remove();
		delete ug;
	}
}

void ModSynth::Start(SeqEvent *evt)
{
	SetParams((VarParamEvent *)evt);

	ModSynthUG *ug;
	for (ug = head.next; ug; ug = ug->next)
		ug->Start();
}

void ModSynth::Param(SeqEvent *evt)
{
	SetParams((VarParamEvent *)evt);
}

void ModSynth::Stop()
{
	ModSynthUG *ug = head.next;
	do
		ug->Stop();
	while ((ug = ug->next) != 0);
}

int ModSynth::IsFinished()
{
	ModSynthUG *ug = head.next;
	do
	{
		if (!ug->IsFinished())
			return 0;
	} while ((ug = ug->next) != 0);
	return 1;
}

void ModSynth::Tick()
{
	ModSynthUG *ug = head.next;
	do
		ug->Tick();
	while ((ug = ug->next) != 0);
	tail.Output(im, chnl);
}

int ModSynth::SetParams(VarParamEvent *vp)
{
	chnl = vp->chnl;
	durParam->SetInput(0, (float)vp->duration / synthParams.sampleRate);
	volParam->SetInput(0, (float)vp->vol);
	pitParam->SetInput(0, (float)vp->pitch);
	frqParam->SetInput(0, (float)vp->frq);

	int err = 0;
	bsInt16 *ids = vp->idParam;
	float *vals = vp->valParam;
	int count = vp->numParam;
	while (count-- > 0)
		err |= SetParam(*ids++, *vals++);
	return err;
}

int ModSynth::GetParams(VarParamEvent *vp)
{
	vp->chnl = chnl;
	vp->frq = frqParam->GetInput(0);
	vp->vol = volParam->GetInput(0);
	vp->duration = (bsInt32) (durParam->GetInput(0) * synthParams.sampleRate);
	ModSynthUG *ug;
	for (ug = head.next; ug; ug = ug->next)
	{
		bsInt16 idval = ug->GetID() << UGID_SHIFT;
		const UGParam *p = ug->GetParamList();
		while (p->index >= 0)
		{
			if ((p->when & UGP_RUN) && !(p->when & UGP_DEF))
				vp->SetParam(idval | (p->index & UGID_MASK), ug->GetInput(p->index));
			p++;
		}
	}
	return 0;
}

int ModSynth::SetParam(bsInt16 idval, float val)
{
	ModSynthUG *ug;
	bsInt16 uid = idval >> UGID_SHIFT;
	for (ug = head.next; ug; ug = ug->next)
	{
		if (ug->GetID() == uid)
		{
			ug->SetInput(idval & UGID_MASK, val);
			return 0;
		}
	}
	return 1;
}

int ModSynth::GetParam(bsInt16 idval, float *val)
{
	ModSynthUG *ug;
	bsInt16 uid = idval >> UGID_SHIFT;
	for (ug = head.next; ug; ug = ug->next)
	{
		if (ug->GetID() == uid)
		{
			*val = ug->GetInput(idval & UGID_MASK);
			return 0;
		}
	}
	return 1;
}

// FIXME: the Copy code is used to instantiate
// an instrument for performance. As such, it needs
// to be as fast as possible. We have to make a copy
// of each unit generator and then make copies of
// all connections. Currently this code does a search
// on the ug name to make connections. It probably
// should build an map of ug pointers during the first
// pass and then it won't be necessary to search
// for each ug and parameter by name in the second
// pass.
// Even better, we should probably keep a cache
// of instances so that we only need to reset the
// configured parameters.
void ModSynth::Copy(ModSynth *tp)
{
	// First copy all unit generators
	ModSynthUG *ugOld;
	numUnits = tp->numUnits;
	numParam = tp->numParam;
	
	// don't copy the head (in params) or tail (out samples)
	for (ugOld = tp->head.next; ugOld != &tp->tail; ugOld = ugOld->next)
	{
		if (*ugOld->GetName() != '@')
			tail.InsertBefore(ugOld->Copy());
	}
	tail.SetInput(3, tp->tail.GetInput(3)); // volume
	tail.SetInput(4, tp->tail.GetInput(4)); // pan set
	tail.SetInput(5, tp->tail.GetInput(5)); // pan on/off

	// Now copy all connections
	for (ugOld = &tp->head; ugOld; ugOld = ugOld->next)
		CopyConn(ugOld);
}

void ModSynth::CopyConn(ModSynthUG *ugOld)
{
	ModSynthUG *ugNew;
	ModSynthUG *dst;
	ModSynthConn *con;

	ugNew = FindUnit(ugOld->GetName());
	con = ugOld->ConnectList(0);
	while (con)
	{
		if (con->ug)
		{
			dst = FindUnit(con->ug->GetName());
			if (dst)
				ugNew->AddConnect(dst, con->index, con->when);
		}
		con = ugOld->ConnectList(con);
	}
}

int ModSynth::GetNumUnits()
{
	if (numUnits == 0)
	{
		ModSynthUG *ug;
		for (ug = head.next; ug != &tail; ug = ug->next)
		{
			if (*ug->GetName() != '@')
				numUnits++;
		}
	}
	return numUnits;
}

ModSynthUG *ModSynth::AddUnit(const char *type, const char *name, ModSynthUG *before)
{
	ModSynthUG *ug = FindUnit(name);
	if (ug)
		return 0;

	ModSynthUGType *tp = FindType(type);
	if (!tp)
		return 0;

	ug = tp->fn();
	if (ug)
	{
		if (before)
			before->InsertBefore(ug);
		else
			tail.InsertBefore(ug);
		ug->SetID(numUnits + P_USER);
		ug->SetName(name);
		numUnits++;
		numParam += ug->GetNumInputs();
	}
	return ug;
}

ModSynthUGType *ModSynth::FindType(const char *type)
{
	ModSynthUGType *tp = ugTypes;
	while (tp->name)
	{
		if (strcmp(tp->name, type) == 0)
			return tp;
		tp++;
	}
	return 0;
}

ModSynthUG *ModSynth::FindUnit(const char *name)
{
	ModSynthUG *ug;
	for (ug = &head; ug; ug = ug->next)
	{
		if (ug->MatchName(name))
			return ug;
	}
	return 0;
}

void ModSynth::MoveUnit(ModSynthUG *ug, ModSynthUG *before)
{
	if (ug && before
	 && ug != &head && ug != &tail 
	 && before != &head)
	{
		ug->Remove();
		before->InsertBefore(ug);
	}
}

void ModSynth::RemoveUnit(ModSynthUG *ug, int dodel)
{
	ModSynthUG *ug2;
	for (ug2 = &head; ug2; ug2 = ug2->next)
		ug2->RemoveConnect(ug);
	if (ug != &head && ug != &tail && *ug->GetName() != '@')
	{
		ug->Remove();
		numParam -= ug->GetNumInputs();
		if (dodel)
			delete ug;
	}
}

ModSynthUG *ModSynth::FirstUnit()
{
	return NextUnit(&head);
}

ModSynthUG *ModSynth::NextUnit(ModSynthUG *ug)
{
	if (ug)
		return ug->next;
	return 0;
}

int ModSynth::GetNumParams()
{
	if (numParam == 0)
	{
		ModSynthUG *ug;
		for (ug = head.next; ug; ug = ug->next)
			numParam += ug->GetNumInputs();
	}
	return numParam;
}

UGValue *ModSynth::AddParam(const char *name, bsInt16 id)
{
	UGValue *ug = new UGValue;
	ug->SetID(id);
	ug->SetName(name);
	head.Insert(ug);
	numParam += ug->GetNumInputs();
	return ug;
}

void ModSynth::Connect(ModSynthUG *src, ModSynthUG *dst, int input, int when)
{
	src->AddConnect(dst, input, when);
}

void ModSynth::Connect(ModSynthUG *ug, const char *dst)
{
	if (ug == 0)
		return;

	char buf[128];
	strncpy(buf, dst, 127);
	buf[127] = 0;
	char *inp = strchr(buf, '.');
	if (inp == NULL)
		inp = "in";
	else
		*inp++ = 0;

	ModSynthUG *dstug = FindUnit(buf);
	if (dstug)
	{
		const UGParam *p = dstug->FindParam(inp);
		if (p)
			ug->AddConnect(dstug, p->index, p->when);
	}
}

void ModSynth::Connect(const char *src, const char *dst)
{
	Connect(FindUnit(src), dst);
}

void ModSynth::Disconnect(ModSynthUG *src, ModSynthUG *dst, int index)
{
	src->RemoveConnect(dst, index);
}

int ModSynth::Load(XmlSynthElem *parent)
{
	ModSynthUG *ug;
	char *ugtype;
	char *ugname;
	XmlSynthElem *child;
	XmlSynthElem *sib;
	child = parent->FirstChild();
	while (child)
	{
		if (child->TagMatch("ugen"))
		{
			ugtype = 0;
			ugname = 0;
			short id = 0;
			child->GetAttribute("type", &ugtype);
			child->GetAttribute("name", &ugname);
			child->GetAttribute("id", id);
			if (ugtype && ugname)
			{
				if (strcmp(ugname, "out"))
					ug = AddUnit(ugtype, ugname);
				else
					ug = &tail;
				if (ug)
				{
					ug->SetID(id);
					ug->Load(child);
				}
			}
			delete ugtype;
			delete ugname;
		}
		else if (child->TagMatch("connect"))
		{
			char *ugsrc = 0;
			char *ugdst = 0;
			child->GetAttribute("src", &ugsrc);
			child->GetAttribute("dst", &ugdst);
			if (ugsrc && ugdst)
				Connect(ugsrc, ugdst);
			delete ugsrc;
			delete ugdst;
		}
		sib = child->NextSibling();
		delete child;
		child = sib;
	}
	return 0;
}

int ModSynth::Save(XmlSynthElem *parent)
{
	int err = 0;
	XmlSynthElem *child;
	ModSynthUG *ug;
	for (ug = head.next; ug; ug = ug->next)
	{
		if (*ug->GetName() != '@')
		{
			child = parent->AddChild("ugen");
			if (child)
			{
				child->SetAttribute("type", ug->GetType());
				child->SetAttribute("name", ug->GetName());
				child->SetAttribute("id", (short)ug->GetID());
				err |= ug->Save(child);
				delete child;
			}
			else
				err++;
		}
	}

	for (ug = &head; ug; ug = ug->next)
		err |= SaveConnect(parent, ug);

	return err;
}

int ModSynth::SaveConnect(XmlSynthElem *parent, ModSynthUG *ug)
{
	bsString dst;
	int err = 0;
	const UGParam *param;
	XmlSynthElem *child;
	ModSynthConn *con;
	con = ug->ConnectList(0);
	while (con)
	{
		if (con->ug)
		{
			child = parent->AddChild("connect");
			if (child)
			{
				child->SetAttribute("src", ug->GetName());
				dst = con->ug->GetName();
				param = con->ug->GetParamList();
				while (param->name)
				{
					if (param->index == con->index)
					{
						dst += '.';
						dst += param->name;
						break;
					}
					param++;
				}
				child->SetAttribute("dst", dst);
				delete child;
			}
			else
				err++;
		}
		con = ug->ConnectList(con);
	}
	return err;
}

void ModSynth::DumpConnect(void (*fn)(const char *))
{
	ModSynthUG *ug;
	for (ug = &head; ug; ug = ug->next)
		ug->DumpUnit(fn);
	for (ug = &head; ug; ug = ug->next)
		ug->DumpConnect(fn);
}
