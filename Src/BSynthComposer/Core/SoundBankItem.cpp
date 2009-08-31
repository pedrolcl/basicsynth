//////////////////////////////////////////////////////////////////////
// BasicSynth - Project item that represents a SF2 or DLS file
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include <SFFile.h>
#include <DLSFile.h>
#include "WindowTypes.h"
#include "ProjectItem.h"

extern int SelectSoundBankPreset(SFPlayerInstr *instr);

SoundBankItem::~SoundBankItem()
{
	if (name.Length() > 0)
	{
		SoundBank *bnk = SoundBank::SoundBankList.FindBank(name);
		if (bnk)
			bnk->Unlock();
	}
}

int SoundBankItem::LoadFile()
{
	if (loaded)
		return 0;

	SoundBank *bnk = 0;
	if (SFFile::IsSF2File(fullPath))
	{
		SFFile sndfile;
		bnk = sndfile.LoadSoundBank(fullPath, preload, normalize);
	}
	else if (DLSFile::IsDLSFile(fullPath))
	{
		DLSFile dls;
		bnk = dls.LoadSoundBank(fullPath, preload, normalize);
	}
	// TODO: 
	// Add type for loading individual wave files into a SoundBank object
	if (bnk)
	{
		loaded = 1;
		bnk->name = name;
		bnk->Lock();
		SoundBank::SoundBankList.Insert(bnk);
	}

	return 0;
}

int SoundBankItem::AddItem()
{
	InstrMapEntry *instType = theProject->mgr.FindType("SoundBank");
	if (!instType)
	{
		prjFrame->Alert("Cannot locate SoundBank insrument type", "Huh?");
		return 0;
	}

	SFPlayerInstr *inst = new SFPlayerInstr;
	if (SelectSoundBankPreset(inst))
	{
		inst->SetParam(21, 0.001); // attack
		inst->SetParam(22, 1.0);   // peak
		inst->SetParam(24, 1.0);   // sustain
		inst->SetParam(25, 0.01);  // release
		inst->SetParam(43, 0.0);   // lfo amount

		int inum = theProject->instrInfo->NextInum();
		InstrConfig *inc = theProject->mgr.AddInstrument(inum, instType, inst);

		InstrItem *itm = new InstrItem;
		itm->SetParent(theProject->instrInfo);
		itm->SetConfig(inc);
		prjTree->AddNode(itm);
		prjFrame->InstrAdded(inc);
		if (itm->ItemProperties())
			itm->EditItem();
		return 1;
	}
	delete inst;
	return 0;
}

int SoundBankItem::EditItem()
{
	return 0;
}

int SoundBankItem::RemoveItem()
{
	if (prjFrame->Verify("Remove sound bank from project?", "Wait...") == 1)
	{
		theProject->SetChange(1);
		return 1;
	}
	return 0;
}

int SoundBankItem::Load(XmlSynthElem *node)
{
	FileItem::Load(node);
	if (SynthProject::FullPath(file))
		fullPath = file;
	else
		theProject->FindOnPath(fullPath, file);
	node->GetAttribute("pre", preload);
	node->GetAttribute("nrm", normalize);
	return 0;
}

int SoundBankItem::Save(XmlSynthElem *node)
{
	int err = FileItem::Save(node);
	node->SetAttribute("pre", preload);
	node->SetAttribute("nrm", normalize);
	return err;
}

int SoundBankItem::LoadProperties(PropertyBox *pb)
{
	FileItem::LoadProperties(pb);
	pb->SetState(PROP_PRELOAD, preload);
	pb->SetValue(PROP_SCALE, normalize, 0);
	pb->EnableValue(PROP_NAME, !loaded);

	return 1;
}

int SoundBankItem::SaveProperties(PropertyBox *pb)
{
	FileItem::SaveProperties(pb);

	pb->GetState(PROP_PRELOAD, preload);
	pb->GetValue(PROP_SCALE, normalize);
	return 1;
}

//////////////////////////////////////////////////////////////////

int SoundBankList::LoadFiles()
{
	SoundBankItem *itm = (SoundBankItem*)prjTree->FirstChild(this);
	while (itm)
	{
		if (itm->GetType() == PRJNODE_SOUNDBANK)
			itm->LoadFile();
		itm = (SoundBankItem*) prjTree->NextSibling(itm);
	}

	return 0;
}

int SoundBankList::AddItem()
{
	bsString file;
	if (prjFrame->BrowseFile(1, file, GetFileSpec(type), GetFileExt(type)))
	{
		SoundBankItem *itm = (SoundBankItem *)NewAdd(file);
		if (itm)
		{
			itm->LoadFile();
			return 1;
		}
	}
	return 0;
}

