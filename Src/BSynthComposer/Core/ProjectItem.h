#ifndef PROJECT_ITEM_H
#define PROJECT_ITEM_H


enum PIType
{
	 PRJNODE_UNKNOWN = 0,
	 PRJNODE_PROJECT,
	 PRJNODE_WAVEOUT,
	 PRJNODE_FILELIST,
	 PRJNODE_SYNTH,
	 PRJNODE_NOTELIST,
	 PRJNODE_NOTEFILE,
	 PRJNODE_SEQLIST,
	 PRJNODE_SEQFILE,
	 PRJNODE_TEXTLIST,
	 PRJNODE_TEXTFILE,
	 PJRNODE_SCRIPTLIST,
	 PRJNODE_SCRIPT,
	 PRJNODE_LIBLIST,
	 PRJNODE_LIB,
	 PRJNODE_LIBINSTR,
	 PRJNODE_INSTRLIST,
	 PRJNODE_INSTR,
	 PRJNODE_WVTABLE,
	 PRJNODE_WVFLIST,
	 PRJNODE_WVFILE,
	 PRJNODE_MIXER,
	 PRJNODE_CHANNEL,
	 PRJNODE_REVERB,
	 PRJNODE_FLANGER,
	 PRJNODE_ECHO,
	 PRJNODE_FXITEM,
	 PRJNODE_SELINSTR
};

#define ITM_ENABLE_NEW   0x0001
#define ITM_ENABLE_ADD   0x0002
#define ITM_ENABLE_COPY  0x0004
#define ITM_ENABLE_REM   0x0008
#define ITM_ENABLE_EDIT  0x0010
#define ITM_ENABLE_PROPS 0x0020
#define ITM_ENABLE_SAVE  0x0040
#define ITM_ENABLE_CLOSE 0x0080
#define ITM_ENABLE_ERRS  0x0100

/// ProjectItem holds common meta-data for all project items.
class ProjectItem
{
protected:
	PIType   type;
	bsString name;
	bsString desc;
	EditorView *editor;
	ProjectItem *parent;
	int leaf;
	int change;
	int actions;
	void *psdata;

public:
	ProjectItem(PIType t = PRJNODE_UNKNOWN)
	{
		type = t;
		psdata = 0;
		editor = 0;
		parent = 0;
		leaf = 1;
		change = 0;
		actions = 0;
	}

	virtual ~ProjectItem() 
	{
	}

	inline int IsLeaf() { return leaf; }

	inline void SetType(PIType t) { type = t; }
	inline PIType GetType() { return type; }
	inline void SetParent(ProjectItem *p) { parent = p; }
	inline ProjectItem *GetParent() { return parent; }
	inline void SetName(const char *s) { name = s; }
	inline const char *GetName() { return name; }
	inline void SetDesc(const char *s) { desc = s; }
	inline const char *GetDesc() { return desc; }
	inline void SetEditor(EditorView *p) { editor = p; }
	inline EditorView *GetEditor() { return editor; }
	inline void SetPSData(void *p) { psdata = p; }
	inline void *GetPSData() { return psdata; }

	virtual int ItemActions() { return actions; }
	virtual int NewItem() { return 0; }
	virtual int AddItem() { return 0; }
	virtual int EditItem() { return 0; }
	virtual int SaveItem() { return 0; }
	virtual int CloseItem();
	virtual int CopyItem() { return 0; }
	virtual int RemoveItem();
	virtual int ItemProperties() { return 0; }
	virtual int GetChanged() { return change; }
	virtual void SetChange(int c) { change = c; }

	virtual int LoadProperties(PropertyBox *pb) { return 1; }
	virtual int SaveProperties(PropertyBox *pb) { return 1; }

	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
	virtual WidgetForm *CreateForm(int xo, int yo);

	static const char *GetFileSpec(int type);
	static const char *GetFileExt(int type);

};


/// A project item that references an external file
// A score file (Notelist)
// or a Sequencer file
// or a Script file
// or a Plain text file
// or a Instrument library file
class FileItem : public ProjectItem
{
protected:
	bsString file;
	bsString fullPath; // fully resolved path
	short useThis;
	short loaded;
public:

	FileItem(PIType t = PRJNODE_TEXTFILE) : ProjectItem(t)
	{
		useThis = 1;
		loaded = 0;
		actions = ITM_ENABLE_EDIT
			    | ITM_ENABLE_PROPS 
				| ITM_ENABLE_REM 
				| ITM_ENABLE_SAVE 
				| ITM_ENABLE_CLOSE;
	}

	inline const char *GetFile() { return file; }
	inline void SetFile(const char *f) { file = f; }
	inline void SetUse(short u) { useThis = u; }
	inline short GetUse() { return useThis; }
	inline short GetLoaded() { return loaded; }
	inline void SetLoaded(short ld) { loaded = ld; }
	inline bsString& PathBuffer() { return fullPath; }
	inline const char *GetFullPath() { return fullPath; }
	inline void SetFullPath(const char *p) { fullPath = p; }

	virtual int ItemProperties();
	virtual int EditItem();
	virtual int SaveItem();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);

	int Load(XmlSynthElem *node);
	int Save(XmlSynthElem *node);
};

class ScoreError : public SynthList<ScoreError>
{
private:
	nlSyntaxErr err;

public:
	ScoreError(nlSyntaxErr *e)
	{
		err.file = e->file;
		err.msg = e->msg;
		err.token = e->token;
		err.lineno = e->lineno;
	}

	inline const char *GetFile() { return err.file; }
	inline const char *GetMsg()  { return err.msg;  }
	inline const char *GetToken() { return err.token; }
	inline long GetLine() { return err.lineno; }
};

class NotelistItem;

class ErrCB : public nlErrOut
{
public:
	NotelistItem *itm;

	virtual void OutputDebug(const char *s);
	virtual void OutputError(const char *s);
	virtual void OutputError(nlSyntaxErr *se);
	virtual void OutputMessage(const char *s);
};

class NotelistItem : public FileItem
{
private:
	ScoreError *errFirst;
	ScoreError *errLast;
	short dbgLevel;

public:
	NotelistItem() : FileItem(PRJNODE_NOTEFILE)
	{
		errFirst = 0;
		errLast = 0;
		dbgLevel = 0;
		actions  = ITM_ENABLE_COPY
			     | ITM_ENABLE_REM
				 | ITM_ENABLE_EDIT
				 | ITM_ENABLE_SAVE
				 | ITM_ENABLE_CLOSE
				 | ITM_ENABLE_ERRS
				 | ITM_ENABLE_PROPS;
	}

	~NotelistItem()
	{
		ClearErrors();
	}

	inline void SetDebug(short d) { dbgLevel = d; }
	inline short GetDebug() { return dbgLevel; }

	virtual int CopyItem();

	ScoreError* EnumErrors(ScoreError *e);
	void AddError(nlSyntaxErr *e);
	void ClearErrors();
	int Convert(nlConverter& cvt);
	int Load(XmlSynthElem *node);
	int Save(XmlSynthElem *node);
};

class FileList : public ProjectItem
{
protected:
	bsString xmlChild;
	virtual FileItem *NewAdd(const char *file);

public:
	FileList(PIType t = PRJNODE_TEXTLIST) : ProjectItem(t)
	{
		xmlChild = "text";
		name = "Text Files";
		leaf = 0;
		actions  = ITM_ENABLE_NEW 
				 | ITM_ENABLE_ADD;
	}
	virtual FileItem *NewChild()
	{ 
		return new FileItem(PRJNODE_TEXTFILE); 
	}
	virtual const char *FileSpec()
	{
		return GetFileSpec(type);
	}
	virtual const char *FileExt()
	{
		return GetFileExt(type);
	}
	virtual int NewItem();
	virtual int AddItem();
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

class NotelistList : public FileList
{
public:
	NotelistList() : FileList(PRJNODE_NOTELIST) 
	{
		xmlChild = "score";
		name = "Notelists";
		leaf = 0;
	}

	int Convert(nlConverter& cvt);
	virtual FileItem *NewChild()
	{ 
		return (FileItem*) new NotelistItem; 
	}
};

class SeqList : public FileList
{
public:
	SeqList() : FileList(PRJNODE_SEQLIST) 
	{
		xmlChild = "seq";
		name = "Sequences";
		leaf = 0;
	}

	int LoadSequences(InstrManager *mgr, Sequencer *seq);

	virtual FileItem *NewChild()
	{ 
		return new FileItem(PRJNODE_SEQFILE); 
	}
};

class ScriptList : public FileList
{
public:
	ScriptList() : FileList(PJRNODE_SCRIPTLIST) 
	{
		xmlChild = "script";
		name = "Script Files";
		leaf = 0;
	}

	virtual FileItem *NewChild()
	{ 
		return new FileItem(PRJNODE_SCRIPT); 
	}

	int LoadScripts(nlConverter& cvt);
};

class InstrItem : public ProjectItem
{
private:
	InstrConfig *inc;
	InstrItem *copyOf;

public:
	InstrItem() : ProjectItem(PRJNODE_INSTR)
	{
		copyOf = 0;
		inc = 0;
		actions = ITM_ENABLE_COPY
				| ITM_ENABLE_EDIT
				| ITM_ENABLE_REM
				| ITM_ENABLE_SAVE
				| ITM_ENABLE_COPY
				| ITM_ENABLE_CLOSE
				| ITM_ENABLE_PROPS;
	}

	void SetActions(int a) { actions = a; }

	inline void SetConfig(InstrConfig *e)
	{
		inc = e;
		if (inc)
		{
			name = inc->GetName();
			desc = inc->GetDesc();
		}
	}

	inline InstrConfig *GetConfig()
	{
		return inc;
	}

	virtual int CopyItem();
	virtual int EditItem();
	virtual int RemoveItem();
	virtual int SaveItem();
	virtual int CloseItem();
	virtual int ItemProperties();
	virtual WidgetForm *CreateForm(int xo, int yo);

	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);

	static int NextInum();
};

class InstrList : public ProjectItem
{
public:
	InstrList() : ProjectItem(PRJNODE_INSTRLIST)
	{
		name = "Instruments";
		leaf = 0;
		actions = ITM_ENABLE_NEW;
	}

	virtual int NewItem();
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);

};

class LibfileItem : public FileItem
{
private:
	int propFn;
	int AddCopy(int f);

public:
	LibfileItem() : FileItem(PRJNODE_LIB)
	{
		leaf = 0;
		actions = ITM_ENABLE_COPY  // copy to project
			    | ITM_ENABLE_ADD   // copy from project
				| ITM_ENABLE_REM   // remove library from project
				| ITM_ENABLE_PROPS // edit library properties
				| ITM_ENABLE_SAVE; // save library changes
	}

	int LoadLib();
	int NextInum();

	virtual int CopyItem();
	virtual int AddItem();
	virtual int RemItem();
	virtual int ItemProperties();
	virtual int SaveItem();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);

};

class LibfileList : public FileList
{
protected:

public:
	LibfileList() : FileList(PRJNODE_LIBLIST)
	{
		xmlChild = "libfile";
		name = "Libraries";
		leaf = 0;
		actions = ITM_ENABLE_NEW  // create new library
			    | ITM_ENABLE_ADD; // add existing library
	}

	virtual FileItem *NewChild()
	{
		return new LibfileItem;
	}

	int LoadLibs();

	virtual int AddItem();
	virtual int NewItem();
};

class WavetableItem : public ProjectItem
{
private:
	short wvNdx;
	short wvID;
	short wvParts;
	short gibbs;
	short sumParts;
	bsInt32 *mult;
	double *amps;
	double *phs;

public:

	WavetableItem() : ProjectItem(PRJNODE_WVTABLE)
	{
		wvNdx = 0;
		wvID = 0;
		wvParts = 0;
		gibbs = 0;
		mult = 0;
		amps = 0;
		phs = 0;
		sumParts = 1;
		actions = ITM_ENABLE_EDIT 
			    | ITM_ENABLE_PROPS
				| ITM_ENABLE_COPY
				| ITM_ENABLE_SAVE
				| ITM_ENABLE_CLOSE
				| ITM_ENABLE_REM;
	}

	~WavetableItem()
	{
		DeleteParts();
	}

	inline short GetParts() { return wvParts; }
	inline void SetIndex(short n) { wvNdx = n; }
	inline short GetIndex() { return wvNdx; }
	inline void SetID(short n) { wvID = n; }
	inline short GetID() { return wvID; }
	inline void SetGibbs(short n) { gibbs = n; }
	inline short GetGibbs() { return gibbs; }
	inline void SetSum(short n) { sumParts = n; }
	inline short GetSum() { return sumParts; }

	inline int SetPart(int n, bsInt32 m, double a, double p)
	{
		if (n < wvParts)
		{
			mult[n] = m;
			amps[n] = a;
			phs[n] = p;
			return 0;
		}
		return -1;
	}

	inline int GetPart(int n, bsInt32 &m, double& a, double& p)
	{
		if (n < wvParts)
		{
			m = mult[n];
			a = amps[n];
			p = phs[n];
			return 0;
		}
		return -1;
	}

	int AllocParts(short n);
	void DeleteParts();
	int InitWaveform();

	virtual int CopyItem();
	virtual int EditItem();
	virtual int SaveItem();
	virtual int RemoveItem();
	virtual int ItemProperties();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
	virtual WidgetForm *CreateForm(int xo, int yo);
};

class WavefileItem : public FileItem
{
private:
	bsInt16 wvid;
	long loopStart;
	long loopEnd;
public:
	WavefileItem() : FileItem(PRJNODE_WVFILE)
	{
		loopStart = 0;
		loopEnd = 0;
		wvid = -1;
		actions = ITM_ENABLE_PROPS | ITM_ENABLE_REM;
	}

	inline void SetID(short id) { wvid = id; }
	inline short GetID() { return wvid; }

	virtual int EditItem();

	WaveFileIn *FindFile(bsInt16 id);
	int LoadFile();

	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

class WavefileList : public ProjectItem
{
private:

public:
	WavefileList() : ProjectItem(PRJNODE_WVFLIST)
	{
		name = "Wavefiles";
		leaf = 0;
		actions = ITM_ENABLE_ADD | ITM_ENABLE_EDIT;
	}

	int LoadFiles();

	virtual int AddItem();
	virtual int EditItem();
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};


class FxItem : public ProjectItem
{
protected:
	bsString fxType;
	short nchnl;
	short unit;
	AmpValue vol;
	AmpValue pan;
	AmpValue *send;
public:
	FxItem(PIType t = PRJNODE_FXITEM) : ProjectItem(t)
	{
		nchnl = 0;
		unit = -1;
		vol = 0;
		pan = 0;
		send = 0;
		actions = ITM_ENABLE_PROPS;
	}

	~FxItem()
	{
		delete send;
	}

	const char *GetFxType() { return fxType; }
	inline short GetUnit() { return unit; }
	inline void SetUnit(short n) { unit = n; }

	inline AmpValue GetVol() { return vol; }
	inline AmpValue GetPan() { return pan; }
	inline AmpValue GetSend(int ndx) { return (ndx < nchnl) ? send[ndx] : 0.0; }
	
	void SetVol(AmpValue v, int imm = 0);
	void SetPan(AmpValue p, int imm = 0);
	void SetSend(int ndx, AmpValue v, int imm = 0);

	virtual void SetChannels(short n);
	virtual void InitMixer();
	virtual void Reset() { };

	virtual int ItemProperties();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

class ReverbItem : public FxItem
{
private:
	AmpValue rvt;
	AmpValue lt[6];
	Reverb2 *rvrb;
public:
	ReverbItem();
	~ReverbItem()
	{
		delete rvrb;
	}

	virtual void Reset()
	{
		if (rvrb)
			rvrb->Clear();
	}

	virtual void InitMixer();

	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

class FlangerItem : public FxItem
{
private:
	AmpValue mix;
	AmpValue fb;
	AmpValue cntr;
	AmpValue depth;
	FrqValue sweep;
	Flanger *flngr;
public:
	FlangerItem() : FxItem(PRJNODE_FLANGER)
	{
		fxType = "flanger";
		mix = 0.7;
		fb = 0.0;
		cntr = 0.004;
		depth = 0.004;
		sweep = 0.15;
		flngr = new Flanger;
		// actions |= ITM_ENABLE_NEW | ITM_ENABLE_COPY | ITM_ENABLE_REM;
	}

	~FlangerItem()
	{
		delete flngr;
	}

	virtual void Reset()
	{
		if (flngr)
			flngr->Clear();
	}

	virtual void InitMixer();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

class EchoItem : public FxItem
{
private:
	FrqValue dly;
	AmpValue dec;
	DelayLineR *dlr;
public:
	EchoItem() : FxItem(PRJNODE_ECHO)
	{
		fxType = "echo";
		dly = 0;
		dec = 0;
		dlr = new DelayLineR;
		// actions |= ITM_ENABLE_NEW | ITM_ENABLE_COPY | ITM_ENABLE_REM;
	}

	~EchoItem()
	{
		delete dlr;
	}

	virtual void Reset()
	{
		if (dlr)
			dlr->Clear();
	}

	virtual void InitMixer();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

class ChannelItem : public ProjectItem
{
private:
	AmpValue vol;
	AmpValue pan;
	short on;
	short cn;
public:
	ChannelItem() : ProjectItem(PRJNODE_CHANNEL)
	{
		vol = 0;
		pan = 0;
		on = 0;
		cn = 0;
	}

	inline void SetChnl(int c) { cn = c; }
	inline short GetChnl() { return cn; }
	inline AmpValue GetVol() { return vol; }
	inline AmpValue GetPan() { return pan; }
	inline short GetOn() { return on; }
	void SetVol(AmpValue v, int imm = 0);
	void SetPan(AmpValue p, int imm = 0);
	void SetOn(short o, int imm = 0);

	virtual void InitMixer();
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

class MixerEdit;

class MixerItem : public ProjectItem
{
private:
	short mixChnl;
	short fxUnits;
	short mixPanType;
	int editX;
	int editY;
	AmpValue mixVolLft;
	AmpValue mixVolRgt;
	ChannelItem *inputs;
	FxItem **effects;
	MixerEdit *mixEdit;

public:
	MixerItem() : ProjectItem(PRJNODE_MIXER)
	{
		name = "Mixer";
		mixChnl = 0;
		fxUnits = 0;
		mixPanType = panTrig;
		mixVolLft = 1.0;
		mixVolRgt = 1.0;
		inputs = 0;
		effects = 0;
		leaf = 0;
		mixEdit = 0;
		editX = 0;
		editY = 0;
		actions = ITM_ENABLE_EDIT 
			    | ITM_ENABLE_PROPS
				| ITM_ENABLE_CLOSE
				| ITM_ENABLE_SAVE;
	}

	~MixerItem()
	{
		delete[] inputs;
		//for (int n = 0; n < fxUnits; n++)
		//	delete effects[n];
		delete effects;
	}

	inline void SetPanType(short n) { mixPanType = n; }
	inline short GetPanType() { return mixPanType; }

	void SetMasterVol(AmpValue lft, AmpValue rgt, int imm = 0);
	inline void GetMasterVol(AmpValue& lft, AmpValue& rgt)
	{
		lft = mixVolLft;
		rgt = mixVolRgt;
	}

	short GetMixerInputs() { return mixChnl; }
	short GetMixerEffects() { return fxUnits; }
	void SetMixerInputs(int num, int keep);
	void SetMixerEffects(int num, int keep);
	
	void SetChannelOn(int ndx, short on, int imm = 0);
	short GetChannelOn(int ndx);
	void SetChannelVol(int ndx, AmpValue vol, int imm = 0);
	AmpValue GetChannelVol(int ndx);
	void SetChannelPan(int ndx, AmpValue pan, int imm = 0);
	AmpValue GetChannelPan(int ndx);

	void SetEffectsVol(int ndx, AmpValue vol, int imm = 0);
	AmpValue GetEffectsVol(int ndx);
	void SetEffectsPan(int ndx, AmpValue pan, int imm = 0);
	AmpValue GetEffectsPan(int ndx);
	void SetEffectsSend(int fx, int cn, AmpValue lvl, int imm = 0);
	AmpValue GetEffectsSend(int fx, int cn);

	FxItem *GetEffectsItem(int fx);
	FxItem *AddEffect(const char *type);

	void InitMixer();
	void ResetMixer();
	void EditorClosed();

	virtual int ItemProperties();
	virtual int EditItem();
	virtual int SaveItem();
	virtual int CloseItem();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
	virtual WidgetForm *CreateForm(int xo, int yo);
};

///////////////////////////////////////////////////////////////////////////

class WaveoutItem : public ProjectItem
{
private:
	bsString outFile;
	float leadIn;
	float tailOut;
	short sampleFmt;
	WaveFile wvf16;
	WaveFileIEEE wvf32;
	WaveOut *wvOut;

public:
	WaveoutItem() : ProjectItem(PRJNODE_WAVEOUT)
	{
		name = "Output";
		leadIn = 0;
		tailOut = 0;
		sampleFmt = 0; // 16-bit PCM
		wvOut = 0;
		actions = ITM_ENABLE_PROPS;
	}

	inline void SetOutfile(const char *s) { outFile = s; }
	inline const char *GetOutfile() { return outFile; }
	inline void SetLeadIn(double n) { leadIn = n; }
	inline double GetLeadIn() { return leadIn; }
	inline void SetTailOut(double n) { tailOut = n; }
	inline double GetTailOut() { return tailOut; }
	inline void SetSampleFmt(short f) { sampleFmt = f; }
	inline short GetSampleFmt() { return sampleFmt; }
	
	WaveOut *GetOutput();
	WaveOut *InitOutput();
	int CloseOutput(WaveOut *wvout, Mixer *mix);

	virtual int ItemProperties();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

/////////////////////////////////////////////////////////////////////

class SynthItem : public ProjectItem
{
private:
	long sampleRate;
	long wtSize;
	long wtUser;
	long newSize;
	int loaded;
	
	WavetableItem *AddWavetable(int ndx);

public:
	SynthItem() : ProjectItem(PRJNODE_SYNTH)
	{
		name = "Synthesizer";
		desc = "Synthesizer parameters";
		sampleRate = 44100;
		wtSize = 16384;
		newSize = wtSize;
		loaded = 0;
		wtUser = 0;
		leaf = 0;
		actions = ITM_ENABLE_PROPS | ITM_ENABLE_NEW;
	}

	inline void SetSampleRate(long r) { sampleRate = r; }
	inline long GetSampleRate() { return sampleRate; }
	inline void SetWTSize(short n) { wtSize = n; }
	inline short GetWTSize() { return wtSize; }
	inline void SetWTUser(short n) { wtUser = n; }
	inline short GetWTUser() { return wtUser; }

	void InitSynth();
	void NewProject();

	virtual int NewItem();
	virtual int ItemProperties();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

/// used to maintain a list of files/directories, e.g., a search path
class PathListItem  : public SynthList<PathListItem >
{
public:
	bsString path;
	PathListItem(const char *p = 0)
	{
		path = p;
	}
};

class PathList : public ProjectItem
{
private:
	PathListItem head;
	PathListItem tail;

public:
	PathList() : ProjectItem(PRJNODE_FILELIST)
	{
		head.Insert(&tail);
	}

	~PathList() 
	{
		RemoveAll();
	}

	void RemoveAll()
	{
		PathListItem *itm;
		while ((itm = head.next) != &tail)
		{
			itm->Remove();
			delete itm;
		}
	}

	static int FullPath(const char *fname);
	void AddItem(const char *name);
	void AddItem(PathListItem *itm);
	void RemoveItem(const char *name);
	PathListItem* EnumList(PathListItem *itm);
	int ResolvePath(FileItem *fi);
	int FindOnPath(bsString& fullPath, const char *fname);

	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

class SynthProject : public ProjectItem
{
private:
	bsString author;
	bsString cpyrgt;
	bsString prjPath;
	bsString prjDir;
	float latency;
	bsString lastError;
	bsString wvInPath;

	void Init();

	void InitProject();

	static void SeqCallback(bsInt32 count, Opaque arg);
	void UpdateGenerator(bsInt32 count);
	int GenerateSequence(nlConverter& cvt);
	int GenerateToFile(long from, long to);

public:
	SynthItem     *synthInfo;
	NotelistList  *nlInfo;
	SeqList       *seqInfo;
	FileList      *txtInfo;
	ScriptList    *scriptInfo;
	MixerItem     *mixInfo;
	InstrList     *instrInfo;
	WaveoutItem   *wvoutInfo;
	WavefileList  *wfInfo;
	LibfileList   *libInfo;
	PathList      *libPath;

	Mixer mix;
	InstrManager mgr;
	Sequencer seq;

	SynthProject() : ProjectItem(PRJNODE_PROJECT)
	{
		leaf = 0;
		Init();
		actions = ITM_ENABLE_PROPS;
	}

	inline void SetAuthor(const char *s) { author = s; }
	inline const char *GetAuthor() { return author; }
	inline void SetCopyright(const char *s) { cpyrgt = s; }
	inline const char *GetCopyright() { return cpyrgt; }
	inline void SetWavePath(const char *s) { wvInPath = s; }
	inline const char *GetWavePath() { return wvInPath; }
	inline const char *WhatHappened() { return lastError; }

	static char *NormalizePath(char *path);
	static char *SkipProjectDir(char *path);
	static char *SaveStringCopy(const char *s);
	static int FullPath(const char *s);

	int NewProject(const char *fname = 0);
	int LoadProject(const char *fname);
	int SaveProject(const char *fname = 0);
	int Generate(int todisk, long from, long to);
	int Play();
	int IsPlaying();
	int PlayEvent(SeqEvent *evt);
	int Start();
	int Stop();

	void SetProjectPath(const char *path)
	{
		prjPath = path;
		int sl = prjPath.FindReverse(0, '/');
		prjPath.SubString(prjDir, 0, sl);
	}

	void GetProjectPath(bsString& path)
	{
		path = prjPath; 
	}

	void GetProjectDir(bsString& path)
	{
		path = prjDir;
	}

	int FindOnPath(bsString& fullPath, const char *fname);
	int FindForm(bsString& fullPath, char *fname);

	virtual int ItemProperties();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

extern SynthProject *theProject;

#endif
