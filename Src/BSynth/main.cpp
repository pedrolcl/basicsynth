///////////////////////////////////////////////////////////
// BasicSynth - command-line synthesizer with Notelist
//
// use: BSynth project
///////////////////////////////////////////////////////////
#include "BSynth.h"

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
	ProjectFileList *libPath;
	long sampleRate;
	long wtSize;
	long wtUser;
	long mixChnl;
	double mixVolLft;
	double mixVolRgt;

	long outType;
	long lastOOR;

	WaveFile wvf;
	Sequencer seq;
	Mixer mix;
	InstrManager mgr;
	BSynthError err;
	nlConverter cvt;

	static void Monitor(bsInt32 cnt, Opaque arg)
	{
		((SynthProject *)arg)->Update(cnt);
	}

	void Update(bsInt32 cnt)
	{
		long oor = wvf.GetOOR();
		if (oor > lastOOR)
		{
			fprintf(stdout, " %ld samples out-of-range\n", oor-lastOOR);
			lastOOR = oor;
		}
		fprintf(stdout, "\r%d:%02d", cnt / 60, cnt % 60);
	}

	SynthProject()
	{
		name = 0;
		author = 0;
		descr = 0;
		cpyrgt = 0;
		title = 0;
		outFile = 0;
		wvPath = 0;
		libPath = 0;
		sampleRate = 44100;
		wtSize = 16384;
		wtUser = 0;
		mixChnl = 0;
		mixVolLft = 1.0;
		mixVolRgt = 1.0;
	}
	~SynthProject()
	{
	}

	void Init()
	{
		mgr.Init(&mix, &wvf);
		mgr.AddType("Tone", ToneInstr::ToneFactory, ToneInstr::ToneEventFactory);
		mgr.AddType("ToneFM", ToneFM::ToneFMFactory, ToneFM::ToneFMEventFactory);
		mgr.AddType("AddSynth", AddSynth::AddSynthFactory, AddSynth::AddSynthEventFactory);
		mgr.AddType("SubSynth", SubSynth::SubSynthFactory, SubSynth::SubSynthEventFactory);
		mgr.AddType("FMSynth", FMSynth::FMSynthFactory, FMSynth::FMSynthEventFactory);
		mgr.AddType("MatrixSynth", MatrixSynth::MatrixSynthFactory, MatrixSynth::MatrixSynthEventFactory);
		mgr.AddType("WFSynth", WFSynth::WFSynthFactory, WFSynth::WFSynthEventFactory);
	}

	int LoadProject(char *prjFname)
	{
		int errcnt = 0;
		bsString fullPath;
		char *fname;
		XmlSynthDoc doc;
		XmlSynthElem *root;

		if ((root = doc.Open(prjFname)) == NULL)
		{
			fprintf(stderr, "Cannot open project %s\n", prjFname);
			return -1;
		}

		if (!root->TagMatch("synthprj"))
		{
			fprintf(stderr, "No synthprj node for %s\n", prjFname);
			delete root;
			return -1;
		}

		// three passes - 
		// 1) global info
		// 2) load instruments 
		// 3) score files
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
			else if (child->TagMatch("synth"))
			{
				child->GetAttribute("sr", sampleRate);
				child->GetAttribute("wt", wtSize);
				child->GetAttribute("usr", wtUser);
				InitSynthesizer((bsInt32)sampleRate, (bsInt32)wtSize, (bsInt32)wtUser);
			}
			else if (child->TagMatch("mixer"))
			{
				if (child->GetAttribute("chnls", mixChnl) == 0)
				{
					mix.SetChannels(mixChnl);
					child->GetAttribute("lft", mixVolLft);
					child->GetAttribute("rgt", mixVolRgt);
					mix.MasterVolume(mixVolLft, mixVolRgt);
					if (mixChnl > 0)
					{
						XmlSynthElem *mixElem = child->FirstChild();
						while (mixElem)
						{
							long cn;
							long on;
							double pan;
							double vol;
							if (mixElem->TagMatch("chnl"))
							{
								cn = -1;
								mixElem->GetAttribute("cn", cn);
								if (cn >= 0 && cn < mixChnl)
								{
									if (mixElem->GetAttribute("on", on) == 0)
										mix.ChannelOn(cn, on);
									if (mixElem->GetAttribute("vol", vol) == 0)
										mix.ChannelVolume(cn, vol);
									if (mixElem->GetAttribute("pan", pan) == 0)
										mix.ChannelPan(cn, panTrig, pan);
								}
							}
							sib = mixElem->NextSibling();
							delete mixElem;
							mixElem = sib;
						}
					}
				}
			}
			sib = child->NextSibling();
			delete child;
			child = sib;
		}
		if (name)
			fprintf(stdout, "%s\n", name);
		if (title)
			fprintf(stdout, "%s\n", name);
		if (author)
			fprintf(stdout, "%s\n", author);
		if (cpyrgt)
			fprintf(stdout, "%s\n", cpyrgt);
		if (descr)
			fprintf(stdout, "%s\n", descr);

		child = root->FirstChild();
		while (child != NULL)
		{
			if (child->TagMatch("libFile"))
			{
				fname = 0;
				child->GetContent(&fname);
				if (fname)
				{
					fprintf(stdout, "Load library %s\n", fname);
					if (FindOnPath(fullPath, fname))
					{
						if (LoadInstrLib(mgr, fname))
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
					delete fname;
				}
			}
			else if (child->TagMatch("instrLib"))
			{
				if (LoadInstrLib(mgr, child))
				{
					fprintf(stderr, "Error loading instrLib\n");
					errcnt++;
				}
			}
			sib = child->NextSibling();
			delete child;
			child = sib;
		}

		//cvt.SetDebugLevel(2);
		cvt.SetErrorCallback(&err);
		cvt.SetInstrManager(&mgr);
		cvt.SetSequencer(&seq);
		cvt.SetSampleRate(sampleRate);

		child = root->FirstChild();
		while (child)
		{
			if (child->TagMatch("score"))
			{
				fname = 0;
				child->GetContent(&fname);
				if (fname)
				{
					if (FindOnPath(fullPath, fname))
					{
						fprintf(stdout, "Convert %s\n", fname);
						if (cvt.Convert(fullPath, NULL))
							errcnt++;
					}
					else
					{
						fprintf(stderr, "Cannot find score file: %s\n", (const char *)fullPath);
						errcnt++;
					}
					delete fname;
				}
			}
			else if (child->TagMatch("seq"))
			{
				fname = 0;
				child->GetContent(&fname);
				if (fname)
				{
					if (FindOnPath(fullPath, fname))
					{
						fprintf(stdout, "Load %s\n", fname);
						SequenceFile seqFileLoad;
						seqFileLoad.Init(&mgr, &seq);
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
						fprintf(stderr, "Cannot find seq file: %s\n", (const char *)fullPath);
						errcnt++;
					}
					delete fname;
				}
			}
			sib = child->NextSibling();
			delete child;
			child = sib;
		}
		delete root;
		doc.Close();

		return errcnt;
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

	int Generate()
	{
		fprintf(stdout, "Generate sequence\n");
		int errcnt = cvt.Generate();
		if (errcnt == 0 && outFile)
		{
			fprintf(stdout, "Generate wavefile %s\n", outFile);
			wvf.OpenWaveFile(outFile, 2);
			lastOOR = 0;
			seq.SetCB(Monitor, synthParams.isampleRate, (Opaque)this);
			int n = seq.Sequence(mgr);
			wvf.CloseWaveFile();
			lastOOR = wvf.GetOOR() - lastOOR;
			if (lastOOR > 0)
				fprintf(stdout, " %ld samples out-of-range\r", lastOOR);
			fprintf(stdout, "\nDone.");
		}
		return errcnt;
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
	}
	else
	{
		prj.Init();
		int errcnt = prj.LoadProject(argv[1]);
		if (errcnt == 0)
			errcnt = prj.Generate();
	}

#if defined(USE_MSXML)
	CoUninitialize();
#endif
	return 0;
}
