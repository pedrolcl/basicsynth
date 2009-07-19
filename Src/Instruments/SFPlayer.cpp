//////////////////////////////////////////////////////////////////////
/// @file SFPlayer.cpp Implementation of SoundFont(R) playback for BasicSynth
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "SFPlayer.h"

/// Create an instance of the SFPlayer instrument
Instrument *SFPlayerInstr::SFPlayerInstrFactory(InstrManager *m, Opaque tmplt)
{
	SFPlayerInstr *ip;
	if (tmplt)
		ip = new SFPlayerInstr((SFPlayerInstr*)tmplt);
	else
		ip = new SFPlayerInstr;
	ip->im = m;
	return ip;
}

/// Create an event object for SFPlayerInstr
SeqEvent *SFPlayerInstr::SFPlayerEventFactory(Opaque tmplt)
{
	VarParamEvent *ep = new VarParamEvent;
	ep->maxParam = 61;
	return (SeqEvent*)ep;
}

/// Allocate a variable parameters object (needed for editor)
VarParamEvent *SFPlayerInstr::AllocParams()
{
	return (VarParamEvent*) SFPlayerEventFactory(0);
}

/// Constructor - no template
SFPlayerInstr::SFPlayerInstr()
{
	chnl = 0;
	pitch = 57;
	monoSet = 1;
	mono = 1;
	vol = 1.0;
	frq = 440.0;

	sndbnk = 0;
	preset = 0;
	bnkNum = -1;
	preNum = -1;
	sfEnv = 1;
	xfade = 0;

	// default 'boxcar' envelope
	volEnv.SetAtkRt(0.001);
	volEnv.SetAtkLvl(1.0);
	volEnv.SetDecRt(0.0);
	volEnv.SetSusLvl(1.0);
	volEnv.SetRelRt(0.001);
	volSet.Alloc(3, 0.0, 1);

	modPan = 0;
	modFrq = 0;
	modVol = 0;
	modPanScale = 0.0;
	modFrqScale = 0.0;
	modVolScale = 0.0;

	fmCM = 0;
	fmIM = 0.0;

	zonel = 0;
	zoner = 0;
	zonel2 = 0;
	zoner2 = 0;
	im = 0;
}

/// Constructor - with template
SFPlayerInstr::SFPlayerInstr(SFPlayerInstr *tp)
{
	zonel = 0;
	zoner = 0;
	zonel2 = 0;
	zoner2 = 0;
	im = 0;
	xfade = 0;
	mono = 1;

	Copy(tp);
}

SFPlayerInstr::~SFPlayerInstr()
{
}

/// Set the sound bank object directly.
/// This can also be done by setting the sound bank alias
void SFPlayerInstr::SetSoundBank(SFSoundBank *b)
{
	sndbnk = b;
	preset = 0;
}

void SFPlayerInstr::FindPreset()
{
	if (sndbnk == 0)
		sndbnk = SFSoundBank::FindBank(sndFile);
	if (sndbnk != 0)
		preset = sndbnk->GetPreset(bnkNum, preNum);
}

void SFPlayerInstr::Copy(SFPlayerInstr *tp)
{
	frq = tp->frq;
	sndbnk = tp->sndbnk;
	preset = tp->preset;
	bnkNum = tp->bnkNum;
	preNum = tp->preNum;
	sndFile = tp->sndFile;
	preName = tp->preName;
	sfEnv = tp->sfEnv;

	chnl = tp->chnl;
	pitch = tp->pitch;
	monoSet = tp->monoSet;
	vol = tp->vol;
	frq = tp->frq;
	modPan = tp->modPan;
	modFrq = tp->modFrq;
	modVol = tp->modVol;
	modPanScale = tp->modPanScale;
	modFrqScale = tp->modFrqScale;
	modVolScale = tp->modVolScale;

	fmCM = tp->fmCM;
	fmIM = tp->fmIM;

	volSet.Copy(&tp->volSet);
	modSet.Copy(&tp->modSet);
	//volEnv.Copy(&tp->volEnv);
	//modEnv.Copy(&tp->modEnv);
	vibLFO.Copy(&tp->vibLFO);
	modLFO.Copy(&tp->modLFO);
}

/// Start playing a note.
/// Locate the preset if needed, then the zone(s).
/// Initialize oscillators and envelopes.
void SFPlayerInstr::Start(SeqEvent *evt)
{
	VarParamEvent *params = (VarParamEvent *)evt;
	pitch = params->pitch;
	frq = params->frq;
	chnl = params->chnl;
	vol  = params->vol;

	SetParams(params);

	zoner = zoner2 = 0;
	zonel = zonel2 = 0;
	xfade = 0;
	mono = monoSet;

	FrqValue oscFrq = frq;
	if (preset == 0)
		FindPreset();
	if (preset)
	{
		int sfpit = pitch+12; // shift from BasicSynth to MIDI range
		zoner = preset->zoneMap[0][sfpit];
		if (zoner)
		{
			oscr.InitSF(oscFrq, zoner);
			panr.Set(panSqr, zoner->pan);
		}
		zonel = preset->zoneMap[1][sfpit];
		if (zonel)
		{
			oscl.InitSF(oscFrq, zonel);
			panl.Set(panSqr, zonel->pan);
		}
		if (!zonel && zoner->pan == 0.0)
			mono = 1;
		if (sfEnv && zoner)
		{
			//AmpValue it = SFEnvLevel(zoner->genVals[sfgInitialAttenuation]);
			volEnv.SetAtkRt(
				  SFEnvRate(zoner->genVals[sfgAttackVolEnv]) 
				+ SFEnvRate(zoner->genVals[sfgDelayVolEnv]));
			volEnv.SetAtkLvl(1.0);
			volEnv.SetDecRt(
				  SFEnvRate(zoner->genVals[sfgDecayVolEnv])
				+ SFEnvRate(zoner->genVals[sfgHoldVolEnv]));
			volEnv.SetSusLvl(SFEnvLevel(zoner->genVals[sfgSustainVolEnv]));
			volEnv.SetRelRt(SFEnvRate(zoner->genVals[sfgReleaseVolEnv]));
		}
		else
			volEnv.SetEnvDef(&volSet);
	}
	volEnv.Reset(0);
	if (modFrq & 4)
	{
		FrqValue fm = oscFrq * fmCM;
		oscfm.InitWT(fm, WT_SIN);
		fmDf = fmIM * fm;
	}
	if ((modFrq | modPan) & 2)
	{
		modEnv.SetEnvDef(&modSet);
		modEnv.Reset(0);
	}
	modLFO.Reset(0);
	if (modFrq & 1)
	{
		vibLFO.SetSigFrq(oscFrq);
		vibLFO.Reset(0);
	}
}

void SFPlayerInstr::Param(SeqEvent *evt)
{
	VarParamEvent *params = (VarParamEvent *)evt;
	SetParams((VarParamEvent*)evt);

	vol  = params->vol;

	if (params->pitch != pitch) // pitch change
	{
		pitch = params->pitch;
		int sfpit = pitch+12;
		zoner2 = preset->zoneMap[0][sfpit];
		if (zoner2 != zoner)
		{
			// begin cross-fade to new sample.
			oscr2.InitSF(params->frq, zoner2, 1);
			if (zonel)
				oscl2.InitSF(params->frq, zonel2 = preset->zoneMap[1][sfpit], 1);
			// 50ms cross fade
			xfade = synthParams.sampleRate / 10;
			fadeEG.InitSegTick(xfade, 0.0, 1.0);
		}
		else
		{
			zoner2 = 0;
			zonel2 = 0;
			oscr.UpdateFrequency(params->frq);
			oscl.UpdateFrequency(params->frq);
		}
		frq = params->frq;
		if (modFrq & 1)
			vibLFO.SetSigFrq(frq);
		if (modFrq & 4)
		{
			FrqValue fm = frq * fmCM;
			oscfm.SetFrequency(fm);
			oscfm.Reset(-1);
			fmDf = fmIM * fm;
		}
	}
	vibLFO.Reset(-1);
	modLFO.Reset(-1);
}

void SFPlayerInstr::Stop()
{
	oscr.Release();
	oscl.Release();
	volEnv.Release();
	modEnv.Release();
}

/// Produce the next sample.
void SFPlayerInstr::Tick()
{
	FrqValue newFrq = frq;
	AmpValue panAdd = 0;

	AmpValue ampVal = vol * volEnv.Gen();

	AmpValue modOut = modLFO.Gen();
	if (modPan & 1)
		panAdd = modOut * modPanScale;
	if (modVol & 1)
		ampVal += modOut * modVolScale;

	if ((modFrq | modPan) & 2)
	{
		// run the modulation envelope
		AmpValue modVal = modEnv.Gen();
		if (modFrq & 2)	// apply to pitch bend
			newFrq = frq * synthParams.GetCentsMult((int) (modVal * modFrqScale));
		if (modPan & 2) // apply to pan
			panAdd += modVal * modPanScale;
	}
	if (modFrq & 1)
		newFrq += vibLFO.Gen();
	if (modFrq & 4)
	{
		if (modFrq & 3)
		{
			FrqValue fm = newFrq * fmCM;
			oscfm.SetFrequency(fm);
			oscfm.Reset(-1);
			fmDf = fmIM * fm;
		}
		newFrq += oscfm.Gen() * fmDf;
	}

	AmpValue oscValR = 0.0;
	AmpValue oscValL = 0.0;
	AmpValue out = 0.0;
	if (zoner)
	{
		if (modFrq)
			oscr.UpdateFrequency(newFrq);
		oscValR = oscr.Gen();
		out = oscValR;
	}
	if (zonel)
	{
		if (modFrq)
			oscl.UpdateFrequency(newFrq);
		oscValL = oscl.Gen();
		out = (out + oscValL) / 2;
	}
	if (xfade > 0)
	{
		AmpValue amp1 = fadeEG.Gen();
		AmpValue amp0 = 1.0 - amp1;
		if (zoner2)
		{
			oscValR = (amp0 * oscValR) + (amp1 * oscr2.Gen());
			out = oscValR;
		}
		if (zonel2)
		{
			oscValL = (amp0 * oscValL) + (amp1 * oscl2.Gen());
			out = (out + oscValL) / 2;
		}
		if (--xfade == 0)
		{
			if ((zoner = zoner2) != 0)
			{
				oscr.Copy(&oscr2);
				zoner2 = 0;
			}
			if ((zonel = zonel2) != 0)
			{
				oscl.Copy(&oscl2);
				zonel2 = 0;
			}
		}
	}
	if (monoSet || mono)
	{
		im->Output(chnl, out * ampVal);
	}
	else
	{
		if (modPan)
		{
			panr.Set(panSqr, panAdd);
			panl.Set(panSqr, panAdd);
		}

		im->Output2(chnl,
			ampVal * ((oscValR * panr.panlft) + (oscValL * panl.panlft)),
			ampVal * ((oscValR * panr.panrgt) + (oscValL * panl.panrgt)));
	}
}

int  SFPlayerInstr::IsFinished()
{
	return volEnv.IsFinished() || (zoner && oscr.IsFinished());
}

void SFPlayerInstr::Destroy()
{
	delete this;
}

int SFPlayerInstr::SetParams(VarParamEvent *params)
{
	int err = 0;

	bsInt16 *id = params->idParam;
	float *valp = params->valParam;
	int n = params->numParam;
	while (n-- > 0)
		err += SetParam(*id++, *valp++);
	return err;
}

int SFPlayerInstr::SetParam(bsInt16 idval, float val)
{
	bsInt16 cmp;
	switch (idval)
	{
	case 16:
		cmp = (bsInt16) val;
		if (cmp != bnkNum)
		{
			bnkNum = cmp;
			preset = 0;
		}
		break;
	case 17:
		cmp = (bsInt16) val;
		if (cmp != preNum)
		{
			preNum = cmp;
			preset = 0;
			FindPreset();
		}
		break;
	case 18:
		monoSet = (int) val;
		break;
	case 19:
		sfEnv = (int) val;
		break;

	case 20:
		volSet.SetStart(AmpValue(val));
		break;
	case 21:
		volSet.SetRate(0, FrqValue(val));
		break;
	case 22:
		volSet.SetLevel(0, AmpValue(val));
		break;
	case 23:
		volSet.SetRate(1, FrqValue(val));
		break;
	case 24:
		volSet.SetLevel(1, AmpValue(val));
		break;
	case 25:
		volSet.SetRate(2, FrqValue(val));
		break;
	case 26:
		volSet.SetLevel(2, AmpValue(val));
		break;
	case 27:
		volSet.SetType(0, (EGSegType) (int) val);
		volSet.SetType(1, (EGSegType) (int) val);
		volSet.SetType(2, (EGSegType) (int) val);
		break;

	case 30:
		modSet.SetStart(AmpValue(val));
		break;
	case 31:
		modSet.SetRate(0, FrqValue(val));
		break;
	case 32:
		modSet.SetLevel(0, AmpValue(val));
		break;
	case 33:
		modSet.SetRate(1, FrqValue(val));
		break;
	case 34:
		modSet.SetLevel(1, AmpValue(val));
		break;
	case 35:
		modSet.SetRate(2, FrqValue(val));
		break;
	case 36:
		modSet.SetLevel(2, AmpValue(val));
		break;
	case 37:
		modSet.SetType(0, (EGSegType) (int) val);
		modSet.SetType(1, (EGSegType) (int) val);
		modSet.SetType(2, (EGSegType) (int) val);
		break;

	case 40:
		vibLFO.SetFrequency(FrqValue(val));
		break;
	case 41:
		vibLFO.SetWavetable((int) val);
		break;
	case 42:
		vibLFO.SetAttack(FrqValue(val));
		break;
	case 43:
		vibLFO.SetLevel(AmpValue(val));
		break;

	case 44:
		modLFO.SetFrequency(FrqValue(val));
		break;
	case 45:
		modLFO.SetWavetable((int) val);
		break;
	case 46:
		modLFO.SetAttack(FrqValue(val));
		break;
	case 47:
		modLFO.SetLevel(AmpValue(val));
		break;
	case 50:
		modFrq = (bsInt16) val;
		break;
	case 51:
		modFrqScale = val;
		break;
	case 52:
		modPan = (bsInt16) val;
		break;
	case 53:
		modPanScale = val;
		break;
	case 54:
		modVol = (bsInt16) val;
		break;
	case 55:
		modVolScale = val;
		break;

	case 60:
		fmCM = val;
		break;
	case 61:
		fmIM = val;
		break;

	default:
		return 1;
	}
	return 0;
}

int SFPlayerInstr::GetParam(bsInt16 idval, float *val)
{
	switch (idval)
	{
	case 16:
		*val = (float) bnkNum;
		break;
	case 17:
		*val = (float) preNum;
		break;
	case 18:
		*val = (float) monoSet;
		break;
	case 19:
		*val = (float) sfEnv;
		break;

	case 20:
		*val = volSet.GetStart();
		break;
	case 21:
		*val = volSet.GetRate(0);
		break;
	case 22:
		*val = volSet.GetLevel(0);
		break;
	case 23:
		*val = volSet.GetRate(1);
		break;
	case 24:
		*val = volSet.GetLevel(1);
		break;
	case 25:
		*val = volSet.GetRate(2);
		break;
	case 26:
		*val = volSet.GetLevel(2);
		break;
	case 27:
		*val = (float) (int) volSet.GetType(0);
		break;

	case 30:
		*val = modSet.GetStart();
		break;
	case 31:
		*val = modSet.GetRate(0);
		break;
	case 32:
		*val = modSet.GetLevel(0);
		break;
	case 33:
		*val = modSet.GetRate(1);
		break;
	case 34:
		*val = modSet.GetLevel(1);
		break;
	case 35:
		*val = modSet.GetRate(2);
		break;
	case 36:
		*val = modSet.GetLevel(2);
		break;
	case 37:
		*val = (float) (int) modSet.GetType(0);
		break;

	case 40:
		*val = vibLFO.GetFrequency();
		break;
	case 41:
		*val = (float) vibLFO.GetWavetable();
		break;
	case 42:
		*val = vibLFO.GetAttack();
		break;
	case 43:
		*val = vibLFO.GetLevel();
		break;

	case 44:
		*val = modLFO.GetFrequency();
		break;
	case 45:
		*val = (float) modLFO.GetWavetable();
		break;
	case 46:
		*val = modLFO.GetAttack();
		break;
	case 47:
		*val = modLFO.GetLevel();
		break;

	case 50:
		*val = (float) modFrq;
		break;
	case 51:
		*val = modFrqScale;
		break;
	case 52:
		*val = (float) modPan;
		break;
	case 53:
		*val = modPanScale;
		break;
	case 54:
		*val = (float) modVol;
		break;
	case 55:
		*val = modVolScale;
		break;

	case 60:
		*val = fmCM;
		break;
	case 61:
		*val = fmIM;
		break;

	default:
		return 1;
	}
	return 0;
}

int SFPlayerInstr::GetParams(VarParamEvent *params)
{
	params->SetParam(P_PITCH, (float)pitch);
	params->SetParam(P_FREQ, (float)frq);
	params->SetParam(P_VOLUME, (float)vol);
	params->SetParam(16, (float) bnkNum);
	params->SetParam(17, (float) preNum);
	params->SetParam(18, (float) monoSet);
	params->SetParam(19, (float) sfEnv);

	params->SetParam(20, (float) volSet.GetStart());
	params->SetParam(21, (float) volSet.GetRate(0));
	params->SetParam(22, (float) volSet.GetLevel(0));
	params->SetParam(23, (float) volSet.GetRate(1));
	params->SetParam(24, (float) volSet.GetLevel(1));
	params->SetParam(25, (float) volSet.GetRate(2));
	params->SetParam(26, (float) volSet.GetLevel(2));
	params->SetParam(27, (float) volSet.GetType(0));

	params->SetParam(30, (float) modSet.GetStart());
	params->SetParam(31, (float) modSet.GetRate(0));
	params->SetParam(32, (float) modSet.GetLevel(0));
	params->SetParam(33, (float) modSet.GetRate(1));
	params->SetParam(34, (float) modSet.GetLevel(1));
	params->SetParam(35, (float) modSet.GetRate(2));
	params->SetParam(36, (float) modSet.GetLevel(2));
	params->SetParam(37, (float) modSet.GetType(0));

	params->SetParam(40, (float) vibLFO.GetFrequency());
	params->SetParam(41, (float) vibLFO.GetWavetable());
	params->SetParam(42, (float) vibLFO.GetAttack());
	params->SetParam(43, (float) vibLFO.GetLevel());

	params->SetParam(44, (float) modLFO.GetFrequency());
	params->SetParam(45, (float) modLFO.GetWavetable());
	params->SetParam(46, (float) modLFO.GetAttack());
	params->SetParam(47, (float) modLFO.GetLevel());

	params->SetParam(50, (float) modFrq);
	params->SetParam(51, (float) modFrqScale);
	params->SetParam(52, (float) modPan);
	params->SetParam(53, (float) modPanScale);
	params->SetParam(54, (float) modVol);
	params->SetParam(55, (float) modVolScale);

	params->SetParam(60, (float) fmCM);
	params->SetParam(61, (float) fmIM);

	return 0;
}

////////////////////////////////////////////////////////////////

static InstrParamMap sfPlayerParams[] = 
{
	{"bank", 16 }, 

	{"fmcm", 60 },
	{"fmim", 61 },

	{"modAttack", 31},
	{"modDecay", 33},
	{"modEnd", 36},
	{"modFrq", 50 }, 
	{"modFrqScale", 51 },
	{"modLfoAttack", 47},
	{"modLfoFrq", 45},
	{"modLfoLevel", 48},
	{"modLfoWT", 46},
	{"modPan", 52 },
	{"modPanScale", 53 }, 
	{"modPeak", 33}, 
	{"modRelease", 35},
	{"modStart", 30},
	{"modSustain", 34},
	{"modType", 37},
	{"modVol", 54},
	{"modVolScale", 55},

	{"monoSet", 18},
	{"preset", 17 }, 

	{"sfenv", 19 },

	{"vibLfoAttack", 40},
	{"vibLfoFrq", 41},
	{"vibLfoLevel", 42},
	{"vibLfoWT", 43},

	{"volAttack", 21},
	{"volDecay", 23}, 
	{"volEnd", 26},
	{"volPeak", 22}, 
	{"volRelease", 25},
	{"volStart", 20},
	{"volSustain", 24},
	{"volType", 27}
};

bsInt16 SFPlayerInstr::MapParamID(const char *name, Opaque tmplt)
{
	return SearchParamID(name, sfPlayerParams, sizeof(sfPlayerParams)/sizeof(InstrParamMap));
}

const char *SFPlayerInstr::MapParamName(bsInt16 id, Opaque tmplt)
{
	return SearchParamName(id, sfPlayerParams, sizeof(sfPlayerParams)/sizeof(InstrParamMap));
}


int SFPlayerInstr::LoadEnv(XmlSynthElem *elem, EnvDef& envSet)
{
	double dvals[7];
	long ival;
	elem->GetAttribute("st",  dvals[0]);
	elem->GetAttribute("atk", dvals[1]);
	elem->GetAttribute("pk",  dvals[2]);
	elem->GetAttribute("dec", dvals[3]);
	elem->GetAttribute("sus", dvals[4]);
	elem->GetAttribute("rel", dvals[5]);
	elem->GetAttribute("end", dvals[6]);
	elem->GetAttribute("ty", ival);
	envSet.Alloc(3, dvals[0], 1);
	envSet.Set(0, dvals[1], dvals[2], (EGSegType) ival, 1);
	envSet.Set(1, dvals[3], dvals[4], (EGSegType) ival, 1);
	envSet.Set(2, dvals[5], dvals[6], (EGSegType) ival, 1);
	return 0;
}

int SFPlayerInstr::Load(XmlSynthElem *parent)
{
	char *cval;

	XmlSynthElem elem;
	XmlSynthElem celem;
	XmlSynthElem *next = parent->FirstChild(&elem);
	while (next != NULL)
	{
		if (elem.TagMatch("venv"))
			LoadEnv(&elem, volSet);
		else if (elem.TagMatch("menv"))
			LoadEnv(&elem, modSet);
		else if (elem.TagMatch("vlfo"))
			vibLFO.Load(&elem);
		else if (elem.TagMatch("mlfo"))
			modLFO.Load(&elem);
		else if (elem.TagMatch("sf"))
		{
			elem.GetAttribute("bank", bnkNum);
			elem.GetAttribute("preset", preNum);
			elem.GetAttribute("mono", monoSet);
			elem.GetAttribute("modfrq", modFrq);
			elem.GetAttribute("modpan", modPan);
			elem.GetAttribute("modvol", modVol);
			elem.GetAttribute("frqscl", modFrqScale);
			elem.GetAttribute("panscl", modPanScale);
			elem.GetAttribute("volscl", modVolScale);
			elem.GetAttribute("fmcm", fmCM);
			elem.GetAttribute("fmim", fmIM);

			XmlSynthElem *child = elem.FirstChild(&celem);
			while (child)
			{
				if (celem.TagMatch("file"))
				{
					celem.GetContent(&cval);
					if (cval)
						sndFile.Attach(cval);
				}
				else if (celem.TagMatch("preset"))
				{
					celem.GetContent(&cval);
					if (cval)
						preName.Attach(cval);
				}
				child = celem.NextSibling(&celem);
			}
		}
		next = elem.NextSibling(&elem);
	}
	if (sndFile.Length() > 0)
		sndbnk = SFSoundBank::FindBank(sndFile);

	return 0;
}

int SFPlayerInstr::SaveEnv(XmlSynthElem *elem, EnvDef& env)
{
	elem->SetAttribute("st", env.GetStart());
	elem->SetAttribute("atk", env.GetRate(0));
	elem->SetAttribute("pk",  env.GetLevel(0));
	elem->SetAttribute("dec", env.GetRate(1));
	elem->SetAttribute("sus", env.GetLevel(1));
	elem->SetAttribute("rel", env.GetRate(2));
	elem->SetAttribute("end", env.GetRate(2));
	elem->SetAttribute("ty", (short)env.GetType(0));
	return 0;
}

int SFPlayerInstr::Save(XmlSynthElem *parent)
{
	XmlSynthElem elem;
	XmlSynthElem celem;

	if (!parent->AddChild("sf", &elem))
		return -1;

	elem.SetAttribute("bank", bnkNum);
	elem.SetAttribute("preset", preNum);
	elem.SetAttribute("mono", monoSet);
	elem.SetAttribute("modfrq", modFrq);
	elem.SetAttribute("modpan", modPan);
	elem.SetAttribute("modvol", modVol);
	elem.SetAttribute("frqscl", modFrqScale);
	elem.SetAttribute("panscl", modPanScale);
	elem.SetAttribute("volscl", modVolScale);
	elem.SetAttribute("fmcm", fmCM);
	elem.SetAttribute("fmim", fmIM);

	if (!elem.AddChild("file", &celem))
		return -1;
	celem.SetContent(sndFile);

	if (!elem.AddChild("preset", &celem))
		return -1;
	celem.SetContent(preName);

	if (!parent->AddChild("venv", &elem))
		return -1;
	SaveEnv(&elem, volSet);

	if (!parent->AddChild("menv", &elem))
		return -1;
	SaveEnv(&elem, modSet);

	if (!parent->AddChild("vlfo", &elem))
		return -1;
	vibLFO.Save(&elem);

	if (!parent->AddChild("mlfo", &elem))
		return -1;
	modLFO.Save(&elem);

	return 0;
}
