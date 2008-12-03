//////////////////////////////////////////////////////////////////////
// BasicSynth WaveFile playback instrument
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _WFSYNTH_H_
#define _WFSYNTH_H_

#define WFSYNTH_MAX_WAVEFILES 100

class WFSynth : public Instrument
{
private:
	AmpValue *samples;
	bsInt32 sampleNumber;
	bsInt32 sampleTotal;
	bsInt16 fileID;
	bsInt16 looping;
	bsInt16 playAll;
	bsInt16 wfUsed[WFSYNTH_MAX_WAVEFILES];
	int chnl;
	EnvGenAR eg;
	AmpValue vol;

	InstrManager *im;

public:
	WFSynth();
	virtual ~WFSynth();

	static Instrument *WFSynthFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *WFSynthEventFactory(Opaque tmplt);
	static bsInt16     MapParamID(const char *name);
	static int GetCacheCount();
	static int AddToCache(const char *filename, bsInt16 id);
	static void ClearCache();
	WaveFileIn *GetCacheEntry(int n);

	void Copy(WFSynth *tp);
	virtual void Start(SeqEvent *evt);
	virtual void Param(SeqEvent *evt);
	virtual void Stop();
	virtual void Tick();
	virtual int  IsFinished();
	virtual void Destroy();

	void UseWavefile(int n, int yn)
	{
		wfUsed[n] = yn;
	}
	void SelectWavefile(bsInt16 id)
	{
		fileID = id;
	}

	int Load(XmlSynthElem *parent);
	int Save(XmlSynthElem *parent);
	int GetParams(VarParamEvent *params);
	int SetParams(VarParamEvent *params);
};

#endif
