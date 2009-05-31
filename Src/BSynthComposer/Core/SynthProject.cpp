//////////////////////////////////////////////////////////////////////
// BasicSynth - Project item that represents the whole project.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"

// Global variables
SynthProject *theProject;
ProjectTree  *prjTree;
GenerateWindow *prjGenerate;
ProjectFrame *prjFrame;
ProjectOptions prjOptions;

static void DestroyTemplate(Opaque tp)
{
	Instrument *ip = (Instrument *)tp;
	delete ip;
}

void SynthProject::Init()
{
	synthInfo = 0;
	nlInfo = 0;
	seqInfo = 0;
	txtInfo = 0;
	scriptInfo = 0;
	mixInfo = 0;
	instrInfo = 0;
	wvoutInfo = 0;
	wfInfo = 0;
	libInfo = 0;
	libPath = 0;
	change = 0;

	InstrMapEntry *ime;
	if (prjOptions.inclInstr & 0x001)
	{
		ime = mgr.AddType("Tone", ToneInstr::ToneFactory, ToneInstr::ToneEventFactory);
		ime->paramToID = ToneInstr::MapParamID;
		ime->paramToName = ToneInstr::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}

	if (prjOptions.inclInstr & 0x002)
	{
		ime = mgr.AddType("ToneFM", ToneFM::ToneFMFactory, ToneFM::ToneFMEventFactory);
		ime->paramToID = ToneFM::MapParamID;
		ime->paramToName = ToneFM::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}
	if (prjOptions.inclInstr & 0x004)
	{
		ime = mgr.AddType("AddSynth", AddSynth::AddSynthFactory, AddSynth::AddSynthEventFactory);
		ime->paramToID = AddSynth::MapParamID;
		ime->paramToName = AddSynth::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}
	if (prjOptions.inclInstr & 0x008)
	{
		ime = mgr.AddType("SubSynth", SubSynth::SubSynthFactory, SubSynth::SubSynthEventFactory);
		ime->paramToID = SubSynth::MapParamID;
		ime->paramToName = SubSynth::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}

	if (prjOptions.inclInstr & 0x010)
	{
		ime = mgr.AddType("FMSynth", FMSynth::FMSynthFactory, FMSynth::FMSynthEventFactory);
		ime->paramToID = FMSynth::MapParamID;
		ime->paramToName = FMSynth::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}
	if (prjOptions.inclInstr & 0x020)
	{
		ime = mgr.AddType("MatrixSynth", MatrixSynth::MatrixSynthFactory, MatrixSynth::MatrixSynthEventFactory);
		ime->paramToID = MatrixSynth::MapParamID;
		ime->paramToName = MatrixSynth::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}
	if (prjOptions.inclInstr & 0x040)
	{
		ime = mgr.AddType("ModSynth", ModSynth::ModSynthFactory, ModSynth::ModSynthEventFactory);
		ime->paramToID = ModSynth::MapParamID;
		ime->paramToName = ModSynth::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}
	if (prjOptions.inclInstr & 0x080)
	{
		ime = mgr.AddType("WFSynth", WFSynth::WFSynthFactory, WFSynth::WFSynthEventFactory);
		ime->paramToID = WFSynth::MapParamID;
		ime->paramToName = WFSynth::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}
	if (prjOptions.inclInstr & 0x100)
	{
		ime = mgr.AddType("Chuffer", Chuffer::ChufferFactory, Chuffer::ChufferEventFactory);
		ime->paramToID = Chuffer::MapParamID;
		ime->paramToName = Chuffer::MapParamName;
		ime->paramToName = Chuffer::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}
	if (prjOptions.inclInstr & 0x200)
	{
		ime = mgr.AddType("MixerControl", MixerControl::MixerControlFactory, MixerControl::MixerControlEventFactory);
		ime->paramToID = MixerControl::MapParamID;
		ime->paramToName = MixerControl::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
		InstrConfig *mi = mgr.AddInstrument(0, ime, 0);
		mi->SetName("[mixer]");
	}
}

void SynthProject::InitProject()
{
	name = "Project";
	parent = 0;
	prjTree->AddNode(this);

	synthInfo = new SynthItem;
	synthInfo->SetParent(this);
	prjTree->AddNode(synthInfo);

	wvoutInfo = new WaveoutItem;
	wvoutInfo->SetParent(this);
	prjTree->AddNode(wvoutInfo);

	mixInfo = new MixerItem;
	mixInfo->SetParent(this);
	prjTree->AddNode(mixInfo);

	nlInfo = new NotelistList;
	nlInfo->SetParent(this);
	if (prjOptions.inclNotelist)
		prjTree->AddNode(nlInfo);

	scriptInfo = new ScriptList;
	scriptInfo->SetParent(this);
	if (prjOptions.inclScripts)
		prjTree->AddNode(scriptInfo);

	seqInfo = new SeqList;
	seqInfo->SetParent(this);
	if (prjOptions.inclSequence)
		prjTree->AddNode(seqInfo);

	txtInfo = new FileList;
	txtInfo->SetParent(this);
	if (prjOptions.inclTextFiles)
		prjTree->AddNode(txtInfo);

	instrInfo = new InstrList;
	instrInfo->SetParent(this);
	prjTree->AddNode(instrInfo);

	wfInfo = new WavefileList;
	wfInfo->SetParent(this);
	prjTree->AddNode(wfInfo);

	libInfo = new LibfileList;
	libInfo->SetParent(this);
	if (prjOptions.inclLibraries)
		prjTree->AddNode(libInfo);

	libPath = new PathList;
	libPath->SetParent(this);
}

int SynthProject::ItemProperties()
{
	if (prjFrame)
		prjFrame->StopPlayer();
	PropertyBox *pb = prjFrame->CreatePropertyBox(this, 0);
	if (pb)
	{
		if (pb->Activate(1))
			change = 1;
		delete pb;
	}
	return change;
}

int SynthProject::LoadProperties(PropertyBox *pb)
{
	pb->SetCaption("Project Properties");
	pb->SetValue(PROP_PRJ_NAME, name, 0);
	pb->SetValue(PROP_PRJ_AUTH, author, 0);
	pb->SetValue(PROP_PRJ_CPYR, cpyrgt, 0);
	pb->SetValue(PROP_PRJ_DESC, desc, 0);
	pb->SetValue(PROP_PRJ_PATH, prjPath, 0);
	wvoutInfo->LoadProperties(pb);
	synthInfo->LoadProperties(pb);
	pb->SetValue(PROP_PRJ_WVIN, wvInPath, 0);
	return 1;
}

int SynthProject::SaveProperties(PropertyBox *pb)
{
	pb->GetValue(PROP_PRJ_NAME, name);
	pb->GetValue(PROP_PRJ_AUTH, author);
	pb->GetValue(PROP_PRJ_CPYR, cpyrgt);
	pb->GetValue(PROP_PRJ_DESC, desc);
	pb->GetValue(PROP_PRJ_PATH, prjPath);
	wvoutInfo->SaveProperties(pb);
	synthInfo->SaveProperties(pb);
	pb->GetValue(PROP_PRJ_WVIN, wvInPath);
	return 1;
}

int SynthProject::Load(XmlSynthElem *node)
{
	char *content;
	XmlSynthElem *child = node->FirstChild();
	while (child)
	{
		if (child->TagMatch("name"))
		{
			child->GetContent(&content);
			name.Attach(content);
			content = 0;
		}
		else if (child->TagMatch("descr"))
		{
			child->GetContent(&content);
			desc.Attach(content);
			content = 0;
		}
		else if (child->TagMatch("author"))
		{
			child->GetContent(&content);
			author.Attach(content);
			content = 0;
		}
		else if (child->TagMatch("cpyrgt"))
		{
			child->GetContent(&content);
			cpyrgt.Attach(content);
			content = 0;
		}
		else if (child->TagMatch("wvdir"))
		{
			child->GetContent(&content);
			wvInPath.Attach(content);
			content = 0;
		}
		else if (child->TagMatch("synth"))
		{
			synthInfo->Load(child);
		}
		else if (child->TagMatch("out"))
		{
			wvoutInfo->Load(child);
		}
		else if (child->TagMatch("mixer"))
		{
			mixInfo->Load(child);
		}
		else if (child->TagMatch("wvfile"))
		{
			WavefileItem *wv = new WavefileItem;
			wv->SetParent(wfInfo);
			wv->Load(child);
			prjTree->AddNode(wv);
		}
		else if (child->TagMatch("libdir"))
		{
			char *path = 0;
			if (child->GetContent(&path) == 0)
			{
				libPath->AddItem(path);
				delete path;
			}
		}
		else if (child->TagMatch("libfile"))
		{
			LibfileItem *lib = new LibfileItem;
			lib->SetParent(libInfo);
			lib->Load(child);
			if (prjOptions.inclLibraries)
				prjTree->AddNode(lib);
		}
		else if (child->TagMatch("score"))
		{
			NotelistItem *ni = new NotelistItem;
			ni->SetParent(nlInfo);
			ni->Load(child);
			if (prjOptions.inclNotelist)
				prjTree->AddNode(ni);
		}
		else if (child->TagMatch("seq"))
		{
			FileItem *fi = new FileItem;
			fi->SetParent(seqInfo);
			fi->Load(child);
			if (prjOptions.inclSequence)
				prjTree->AddNode(fi);
		}
		else if (child->TagMatch("text"))
		{
			FileItem *fi = new FileItem;
			fi->SetParent(txtInfo);
			fi->Load(child);
			if (prjOptions.inclTextFiles)
				prjTree->AddNode(fi);
		}
		else if (child->TagMatch("script"))
		{
			FileItem *fi = new FileItem;
			fi->SetParent(scriptInfo);
			fi->Load(child);
			if (prjOptions.inclScripts)
				prjTree->AddNode(fi);
		}

		XmlSynthElem *sib = child->NextSibling();
		delete child;
		child = sib;
	}

	if (wvInPath.Length())
		synthParams.wvPath = wvInPath;
	else
		synthParams.wvPath = prjDir;
	synthInfo->InitSynth();
	mixInfo->InitMixer();
	wfInfo->LoadFiles();
	if (prjOptions.inclLibraries)
		libInfo->LoadLibs();

	// We need to wait to do instrument loading until after the synth is initialized
	child = node->FirstChild();
	while (child)
	{
		if (child->TagMatch("instrlib"))
		{
			instrInfo->Load(child);
		}
		XmlSynthElem *sib = child->NextSibling();
		delete child;
		child = sib;
	}

	bsString colorFile;
	if (FindForm(colorFile, prjOptions.colorsFile))
		SynthWidget::colorMap.Load(colorFile);
	return 0;
}

int SynthProject::Save(XmlSynthElem *node)
{
	// node points to the document root
	//node->SetAttribute("name", name);

	XmlSynthElem *child;
	
	child = node->AddChild("name");
	child->SetContent(name);
	delete child;

	child = node->AddChild("descr");
	child->SetContent(desc);
	delete child;

	child = node->AddChild("author");
	child->SetContent(author);
	delete child;

	child = node->AddChild("cpyrgt");
	child->SetContent(cpyrgt);
	delete child;

	child = node->AddChild("wvdir");
	child->SetContent(wvInPath);
	delete child;

	synthInfo->Save(node);
	wvoutInfo->Save(node);
	mixInfo->Save(node);
	wfInfo->Save(node);
	libInfo->Save(node);
	instrInfo->Save(node);
	nlInfo->Save(node);
	seqInfo->Save(node);
	txtInfo->Save(node);
	scriptInfo->Save(node);

	return 0;
}


int SynthProject::NewProject(const char *fname)
{
	InitProject();
	SetProjectPath(fname);
	SetAuthor(prjOptions.defAuthor);
	SetCopyright(prjOptions.defCopyright);
	bsString colorFile;
	if (FindForm(colorFile, prjOptions.colorsFile))
		SynthWidget::colorMap.Load(colorFile);
	int ok = ItemProperties();
	synthInfo->NewProject();
	mixInfo->SetMixerInputs(1, 0);
	mixInfo->SetChannelOn(0, 1);
	mixInfo->SetChannelVol(0, 0.5);
	mixInfo->SetChannelPan(0, 0.0);
	mixInfo->InitMixer();
	if (ok)
		SaveProject();
	return 1;
}

int SynthProject::LoadProject(const char *fname)
{
	InitProject();

	lastError = "";

	XmlSynthDoc doc;
	XmlSynthElem *root;
	if ((root = doc.Open((char*)fname)) == NULL)
	{
		lastError = "Cannot open file: ";
		lastError += fname;
		lastError += "Project files must be valid XML.";
		return -1;
	}

	if (!root->TagMatch("synthprj"))
	{
		lastError = "The project file has the wrong root tag.";
		delete root;
		return -2;
	}

	SetProjectPath(fname);

	int err = Load(root);

	delete root;
	doc.Close();

	prjTree->UpdateNode(this);

	change = 0;
	return err;
}

int SynthProject::SaveProject(const char *fname)
{
	if (fname == 0)
	{
		if (prjPath.Length() == 0)
		{
			lastError = "Project name was not specified";
			return -1;
		}
		fname = prjPath;
	}
	else
		SetProjectPath(fname);

	XmlSynthDoc doc;
	XmlSynthElem *root;
	if ((root = doc.NewDoc("synthprj")) == NULL)
	{
		lastError = "Cannot create new project document";
		return -1;
	}

	int err = Save(root);

	delete root;
	if (err == 0)
	{
		if (doc.Save((char*)fname))
		{
			lastError = "Cannot write output file";
			return -1;
		}
	}
	change = 0;
	return err;
}

int SynthProject::CopyFiles(const char *oldDir, const char *newDir)
{
	int err = 0;
	if (nlInfo)
		err |= nlInfo->CopyFiles(oldDir, newDir);
	if (seqInfo)
		err |= seqInfo->CopyFiles(oldDir, newDir);
	if (txtInfo)
		err |= txtInfo->CopyFiles(oldDir, newDir);
	if (scriptInfo)
		err |= scriptInfo->CopyFiles(oldDir, newDir);
	if (libInfo)
		err |= libInfo->CopyFiles(oldDir, newDir);
	return err;
}

char *SynthProject::NormalizePath(char *path)
{
	char *sl = path;
	while ((sl = strchr(sl, '\\')) != NULL)
		*sl++ = '/';
	return path;
}

char *SynthProject::SkipProjectDir(char *path)
{
	size_t len = theProject->prjDir.Length();
	if (strncmp(theProject->prjDir, path, len) == 0 && path[len] == '/')
		return path+len+1;
	return path;
}

int SynthProject::FullPath(const char *s)
{
	return PathList::FullPath(s);
}


int SynthProject::FindOnPath(bsString& fullPath, const char *fname)
{
	return libPath->FindOnPath(fullPath, fname);
}

int SynthProject::FindForm(bsString& fullPath, char *fname)
{
	fullPath = prjOptions.formsDir;
	fullPath += '/';
	fullPath += fname;
	if (SynthFileExists(fullPath))
		return 1;
	return FindOnPath(fullPath, fname);
}

void SynthProject::SeqCallback(bsInt32 count, Opaque arg)
{
	((SynthProject*)arg)->UpdateGenerator(count);
}

void SynthProject::UpdateGenerator(bsInt32 count)
{
	AmpValue lftPeak;
	AmpValue rgtPeak;
	mix.Peak(lftPeak, rgtPeak);
	prjGenerate->UpdatePeak(lftPeak, rgtPeak);
	prjGenerate->UpdateTime(count);
	if (prjGenerate->WasCanceled())
	{
		if (prjGenerate)
			prjGenerate->AddMessage("Halting sequencer...");
		seq.Halt();
	}
}

int SynthProject::GenerateSequence(nlConverter& cvt)
{
	if (prjGenerate)
		prjGenerate->AddMessage("Generate sequences...");

	mix.Reset();
	seq.Reset();
	seq.SetCB(SeqCallback, synthParams.isampleRate, (Opaque)this);

	int err = 0;
	if (nlInfo)
		err |= nlInfo->Convert(cvt);
	if (seqInfo)
		err |= seqInfo->LoadSequences(&mgr, &seq);
	if (err == 0)
	{
		ErrCB ecb;
		ecb.itm = 0;
		cvt.SetErrorCallback(&ecb);
		err = cvt.Generate();
	}
	return err;
}

int SynthProject::GenerateToFile(long from, long to)
{
	if (!wvoutInfo)
		return -1;
	mgr.Init(&mix, wvoutInfo->GetOutput());

	nlConverter cvt;
	cvt.SetInstrManager(&mgr);
	cvt.SetSequencer(&seq);
	cvt.SetSampleRate(synthParams.sampleRate);

	if (GenerateSequence(cvt))
		return -1;

	WaveOut *wvOut = wvoutInfo->InitOutput();
	if (!wvOut)
	{
		if (prjGenerate)
			prjGenerate->AddMessage("Could not initialize output.");
		return -1;
	}
	if (prjGenerate)
		prjGenerate->AddMessage("Start sequencer...");
	seq.Sequence(mgr, from*synthParams.isampleRate, to*synthParams.isampleRate);
	wvoutInfo->CloseOutput(wvOut, &mix);

	// reset in case of dynamic mixer control changes
	mixInfo->InitMixer();
	return 0;
}

//////////////////////////////////////////////////////////
// These are the platform-independent versions. They do
// not include direct playback of the sequence or keyboard.
// On a platform that supports direct playback to the 
// sound card, set NO_LIVE_PLAY to 0, copy this code to
// the platform specific library, and add the implementation
// of playback. See SynthProjectWin for an example.
//////////////////////////////////////////////////////////

#if NO_LIVE_PLAY
int SynthProject::Generate(int todisk, long from, long to)
{
	if (!todisk || !wvoutInfo)
		return -1;
	return GenerateToFile(from, to);
}

int SynthProject::Play()
{
	return 0;
}

int SynthProject::Stop()
{
	return 0;
}

int SynthProject::PlayEvent(SeqEvent *evt)
{
	delete evt;
	return 0;
}

#endif

ProjectOptions::ProjectOptions()
{
	memset(installDir, 0, MAX_PATH);
	memset(formsDir, 0, MAX_PATH);
	strcpy(colorsFile, "Colors.xml");
	memset(defAuthor, 0, MAX_PATH);
	memset(defCopyright, 0, MAX_PATH);
	memset(defPrjDir, 0, MAX_PATH);
	memset(defLibDir, 0, MAX_PATH);
	memset(defWaveIn, 0, MAX_PATH);
	memset(defWaveOut, 0, MAX_PATH);
	inclNotelist = 1;
	inclSequence = 0;
	inclScripts = 0;
	inclTextFiles = 0;
	inclLibraries = 0;
	inclInstr = 0xfff;
	playBuf = 0.2;
}

// ProjectOptions Load and Save are platform specific
