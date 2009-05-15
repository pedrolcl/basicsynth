#include "ComposerGlobal.h"
#include "WindowTypes.h"
#include "ProjectItem.h"

///////////////////////////////////////////////
// FIXME: Wavetables can also be allocated
// in library files. we need to discover the
// total number of wavetables and init
// appropriately. In an initial load from the
// project file, libraries are loaded after this
// point and will extend the wavetable set as
// needed. BUT - if InitSynth() is called again,
// the library tables will get discarded and then...
// crash.
///////////////////////////////////////////////

void SynthItem::InitSynth()
{
	loaded = 1;
	InitSynthesizer(sampleRate, wtSize, wtUser);

	WavetableItem *wv = (WavetableItem *)prjTree->FirstChild(this);
	while (wv)
	{
		if (wv->GetType() == PRJNODE_WVTABLE)
		{
			if (wv->GetParts() == 0)
				wv->AllocParts(1);
			wv->InitWaveform();
		}
		wv = (WavetableItem *)prjTree->NextSibling(wv);
	}
}

int SynthItem::ItemProperties()
{
	return theProject->ItemProperties();
}

WavetableItem *SynthItem::AddWavetable(int ndx)
{
	WavetableItem *wvitm = new WavetableItem;
	char nm[40];
	snprintf(nm, 40, "#%d", WT_USR(ndx));
	wvitm->SetName(nm);
	wvitm->SetSum(1);
	wvitm->SetGibbs(0);
	wvitm->SetParent(this);
	wvitm->SetID(WT_USR(ndx));
	wvitm->SetIndex(wtSet.wavTblMax);
	prjTree->AddNode(wvitm);
	return wvitm;
}

int SynthItem::NewItem()
{
	WavetableItem *wnew;
	wnew = AddWavetable(wtUser++);
	wnew->AllocParts(1);
	wnew->SetPart(0, 1, 1.0, 0.0);
	wnew->InitWaveform();
	wnew->ItemProperties();
	prjTree->UpdateNode(wnew);
	theProject->SetChange(1);
	wnew->EditItem();
	return 1;
}

int SynthItem::LoadProperties(PropertyBox *pb)
{
	pb->SetValue(PROP_PRJ_SRT, sampleRate, 0);
	pb->SetValue(PROP_PRJ_WTSZ, newSize, 0);
	pb->SetValue(PROP_PRJ_WTU, wtUser, 0);
	if (loaded)
		pb->EnableValue(PROP_PRJ_WTU, 0);
	return 1;
}

int SynthItem::SaveProperties(PropertyBox *pb)
{
	long newRate = 0;
	pb->GetValue(PROP_PRJ_SRT, newRate);
	if (newRate != sampleRate)
	{
		sampleRate = newRate;
		if (loaded)
			synthParams.Init(sampleRate, wtSize);
	}

	// wtUser is calculated as the project is saved/loaded
	// except for a new project where we let the user
	// specify the number of extra wavetables to start with.
	if (!loaded)
		pb->GetValue(PROP_PRJ_WTU, wtUser);

	// If wtSize gets changed, we need to re-initialize the synthesizer.
	// At present we can't do that reliably. So, the change is stored and
	// will be applied on the next project open. As with wtUser, for a 
	// new project, the user can specify a value that will be applied
	// for the first init.
	pb->GetValue(PROP_PRJ_WTSZ, newSize);
	if (newSize != wtSize)
	{
		if (loaded)
			prjFrame->Alert("Change to the wavetable size will be applied the next time the project is loaded.", "Note!");
		else
			wtSize = newSize;
	}
	return 1;
}

void SynthItem::NewProject()
{
	if (wtUser > 0)
	{
		for (long ndx = 0; ndx < wtUser; ndx++)
			AddWavetable(ndx);
		wtSet.SetMax(wtSet.wavTblMax + wtUser);
	}
	InitSynth();
}

int SynthItem::Load(XmlSynthElem *node)
{
	node->GetAttribute("sr", sampleRate);
	node->GetAttribute("wt", wtSize);
	newSize = wtSize;
	wtUser = 0;
	int wvIndex = WT_USR(0);

	WavetableItem *wvitm;
	XmlSynthElem *wvnode = node->FirstChild();
	while (wvnode)
	{
		if (wvnode->TagMatch("wvtable"))
		{
			wvitm = new WavetableItem;
			wvitm->SetParent(this);
			wvitm->SetIndex(wvIndex++);
			wvitm->Load(wvnode);
			prjTree->AddNode(wvitm);
			short n = wvitm->GetID() - WT_USR(0);
			if (n >= wtUser)
				wtUser = n+1;
		}
		XmlSynthElem *sib = wvnode->NextSibling();
		delete wvnode;
		wvnode = sib;
	}
	return 0;
}

int SynthItem::Save(XmlSynthElem *node)
{
	XmlSynthElem *synth = node->AddChild("synth");
	if (!synth)
		return -1;

	int err = 0;
	synth->SetAttribute("sr", sampleRate);
	synth->SetAttribute("wt", newSize);

	short usr = 0;
	ProjectItem *pi = prjTree->FirstChild(this);
	while (pi)
	{
		if (pi->GetType() == PRJNODE_WVTABLE)
		{
			XmlSynthElem *child = synth->AddChild("wvtable");
			if (child)
			{
				usr++;
				pi->Save(child);
				delete child;
			}
			else
				err++;
		}
		pi = prjTree->NextSibling(pi);
	}
	synth->SetAttribute("usr", usr);
	delete synth;
	return err;
}

