///////////////////////////////////////////////////////////
// BasicSynth - command-line synthesizer with Notelist
//
// use: BSynth project
///////////////////////////////////////////////////////////
#include "BSynth.h"

class ProjectFileList : 
	public SynthList<ProjectFileList>
{
public:
	char *str;

	ProjectFileList() {	str = 0; }
	~ProjectFileList() { delete str; }
};

class SynthProject
{
public:
	char *name; // name of the project
	char *author; // author/composer
	char *descr;
	char *cpyrgt; // copyright
	char *title;
	char *outFile;
	ProjectFileList *wvPath;
	ProjectFileList *scoreList;
	ProjectFileList *seqList;
	ProjectFileList *libPath;
	ProjectFileList *libFile;
	long sampleRate;
	long wtSize;
	long wtUser;
	long mixChnl;
	double mixVolLft;
	double mixVolRgt;
	double *mixChnlVol;
	double *mixChnlPan;
	long   *mixChnlOn;

	long outType; // 1 = WAV, 2 = MIDI

	SynthProject()
	{
		name = 0;
		author = 0;
		descr = 0;
		cpyrgt = 0;
		title = 0;
		outFile = 0;
		wvPath = 0;
		scoreList = 0;
		seqList = 0;
		libFile = 0;
		libPath = 0;
		sampleRate = 44100;
		wtSize = 16384;
		wtUser = 0;
		mixChnl = 0;
		mixVolLft = 1.0;
		mixVolRgt = 1.0;
		mixChnlVol = 0;
		mixChnlPan = 0;
		mixChnlOn = 0;
	}
	~SynthProject()
	{
	}

	int LoadProject(char *fname)
	{
		XmlSynthDoc doc;
		XmlSynthElem *root;
		if ((root = doc.Open(fname)) == NULL)
			return -1;

		if (!root->TagMatch("synthprj"))
		{
			delete root;
			return -1;
		}

		XmlSynthElem *child = root->FirstChild();
		XmlSynthElem *sib;
		while (child != NULL)
		{
			if (child->TagMatch("name"))
				child->GetContent(&name);
			else if (child->TagMatch("author"))
				child->GetContent(&author);
			else if (child->TagMatch("desc"))
				child->GetContent(&descr);
			else if (child->TagMatch("cpyrgt"))
				child->GetContent(&cpyrgt);
			else if (child->TagMatch("out"))
			{
				child->GetAttribute("type", outType);
				child->GetContent(&outFile);
			}
			else if (child->TagMatch("wvdir"))
			{
				ProjectFileList *lib = new ProjectFileList;
				child->GetContent(&lib->str);
				if (wvPath)
					wvPath->Insert(lib);
				else
					wvPath = lib;
			}
			else if (child->TagMatch("libpath"))
			{
				ProjectFileList *lib = new ProjectFileList;
				child->GetContent(&lib->str);
				if (libPath)
					libPath->Insert(lib);
				else
					libPath = lib;
			}
			else if (child->TagMatch("libfile"))
			{
				ProjectFileList *lib = new ProjectFileList;
				child->GetContent(&lib->str);
				if (libFile)
					libFile->Insert(lib);
				else
					libFile = lib;
			}
			else if (child->TagMatch("instr"))
			{
			}
			else if (child->TagMatch("score"))
			{
				ProjectFileList *fn = new ProjectFileList;
				child->GetContent(&fn->str);
				if (scoreList)
					scoreList->Insert(fn);
				else
					scoreList = fn;
			}
			else if (child->TagMatch("seq"))
			{
				ProjectFileList *fn = new ProjectFileList;
				child->GetContent(&fn->str);
				if (seqList)
					seqList->Insert(fn);
				else
					seqList = fn;
			}
			else if (child->TagMatch("synth"))
			{
				child->GetAttribute("sr", sampleRate);
				child->GetAttribute("wt", wtSize);
				child->GetAttribute("usr", wtUser);
			}
			else if (child->TagMatch("mixer"))
			{
				child->GetAttribute("chnls", mixChnl);
				child->GetAttribute("lft", mixVolLft);
				child->GetAttribute("rgt", mixVolRgt);
				if (mixChnl > 0)
				{
					mixChnlVol = new double[mixChnl];
					mixChnlPan = new double[mixChnl];
					mixChnlOn  = new long[mixChnl];
					long cn;
					for( cn = 0; cn < mixChnl; cn++)
					{
						mixChnlVol[cn] = 0.1;
						mixChnlPan[cn] = 0.0;
						mixChnlOn[cn] = 1;
					}
					XmlSynthElem *mixElem = child->FirstChild();
					while (mixElem)
					{
						if (mixElem->TagMatch("chnl"))
						{
							cn = -1;
							mixElem->GetAttribute("cn", cn);
							if (cn >= 0 && cn < mixChnl)
							{
								mixElem->GetAttribute("on", mixChnlOn[cn]);
								mixElem->GetAttribute("vol", mixChnlVol[cn]);
								mixElem->GetAttribute("pan", mixChnlPan[cn]);
							}
						}
						sib = mixElem->NextSibling();
						delete mixElem;
						mixElem = sib;
					}
				}
			}
			sib = child->NextSibling();
			delete child;
			child = sib;
		}
		delete root;
		doc.Close();

		return 0;
	}

	int FindOnPath(bsString& fullPath, char *fname)
	{
		int gotFile = 0;
		fullPath = fname;
		ProjectFileList *libs = libPath;
		while (!(gotFile = SynthFileExists(fullPath)) && libs)
		{
			fullPath = libs->str;
			fullPath += "/";
			fullPath += fname;
			libs = libs->next;
		}
		return gotFile;
	}


};

class BSynthError : public nlErrOut
{
public:

	virtual void OutputDebug(char *s)
	{
		fputs(s, stderr);
		fputc('\n', stderr);
	}

	virtual void OutputError(char *s)
	{
		fputs(s, stderr);
		fputc('\n', stderr);
	}

	virtual void OutputMessage(char *s)
	{
		fputs(s, stdout);
		fputc('\n', stdout);
	}
};

SynthProject prj;

int main(int argc, char *argv[])
{
#if defined(USE_MSXML)
	CoInitialize(0);
#endif
	if (argc < 2)
	{
		fprintf(stderr, "use: BSynth project\n");
		exit(1);
	}

	if (prj.LoadProject(argv[1]))
	{
		fprintf(stderr, "Cannot open project %s\n", argv[1]);
		exit(1);
	}

	int errcnt = 0;
	WaveFile wvf;
	Sequencer seq;
	Mixer mix;
	InstrManager mgr;

	InitSynthesizer((bsInt32)prj.sampleRate, (bsInt32)prj.wtSize, (bsInt32)prj.wtUser);

	mgr.Init(&mix, &wvf);
	mgr.AddType("Tone", ToneInstr::ToneFactory, ToneInstr::ToneEventFactory);
	mgr.AddType("ToneFM", ToneFM::ToneFMFactory, ToneFM::ToneFMEventFactory);
	mgr.AddType("AddSynth", AddSynth::AddSynthFactory, AddSynth::AddSynthEventFactory);
	mgr.AddType("SubSynth", SubSynth::SubSynthFactory, SubSynth::SubSynthEventFactory);
	mgr.AddType("FMSynth", FMSynth::FMSynthFactory, FMSynth::FMSynthEventFactory);
	mgr.AddType("MatrixSynth", MatrixSynth::MatrixSynthFactory, MatrixSynth::MatrixSynthEventFactory);
	mgr.AddType("WFSynth", WFSynth::WFSynthFactory, WFSynth::WFSynthEventFactory);

	mix.SetChannels((int)prj.mixChnl);
	mix.MasterVolume((AmpValue)prj.mixVolLft, (AmpValue)prj.mixVolRgt);
	int chnl;
	for (chnl = 0; chnl < prj.mixChnl; chnl++)
	{
		if (prj.mixChnlOn)
			mix.ChannelOn(chnl, prj.mixChnlOn[chnl]);
		if (prj.mixChnlVol)
			mix.ChannelVolume(chnl, prj.mixChnlVol[chnl]);
		if (prj.mixChnlPan)
			mix.ChannelPan(chnl, 0, prj.mixChnlPan[chnl]);
	}


	bsString fullPath;
	ProjectFileList *instrFile = prj.libFile;
	while (instrFile)
	{
		if (prj.FindOnPath(fullPath, instrFile->str))
		{
			if (LoadInstrLib(mgr, fullPath))
			{
				fprintf(stderr, "Error loading %s\n", (const char*)fullPath);
				errcnt++;
			}
		}
		else
		{
			fprintf(stderr, "Cannot find instr. file %s\n", (const char*)fullPath);
			errcnt++;
		}

		instrFile = instrFile->next;
	}

	SequenceFile seqFileLoad;
	seqFileLoad.Init(&mgr, &seq);
	ProjectFileList *seqFile = prj.seqList;
	while (seqFile != NULL)
	{
		if (prj.FindOnPath(fullPath, seqFile->str))
		{
			if (seqFileLoad.LoadFile(fullPath))
			{
				bsString ebuf;
				seqFileLoad.GetError(ebuf);
				fprintf(stderr, "Error load sequence %s\n%s\n", (const char*)fullPath, (const char*)ebuf);
				errcnt++;
			}
		}
		else
		{
			fprintf(stderr, "Cannot find score file: %s\n", (const char *)fullPath);
			errcnt++;
		}
		seqFile = seqFile->next;
	}


	BSynthError err;
	nlConverter cvt;
	//cvt.SetDebugLevel(2);
	cvt.SetErrorCallback(&err);
	cvt.SetInstrManager(&mgr);
	cvt.SetSequencer(&seq);
	cvt.SetSampleRate(synthParams.sampleRate);

	ProjectFileList *scoreFile = prj.scoreList;
	while (scoreFile != NULL)
	{
		if (prj.FindOnPath(fullPath, scoreFile->str))
		{
			if (cvt.Convert(fullPath, NULL))
				errcnt++;
		}
		else
		{
			fprintf(stderr, "Cannot find score file: %s\n", (const char *)fullPath);
			errcnt++;
		}
		scoreFile = scoreFile->next;
	}

	if (errcnt == 0)
		errcnt = cvt.Generate();

	if (errcnt == 0 && prj.outFile)
	{
		wvf.OpenWaveFile(prj.outFile, 2);
		int n = seq.Sequence(mgr);
		wvf.CloseWaveFile();
	}

#if defined(USE_MSXML)
	CoUninitialize();
#endif
	return errcnt;
}
