#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "SynthEdit.h"
#include "ToneSynthEd.h"
#include "FMSynthEd.h"
#include "MatSynthEd.h"
#include "SubSynthEd.h"
#include "AddSynthEd.h"
#include "WFSynthEd.h"
#include "ChufferEd.h"
#include "ModSynthEd.h"

int InstrItem::EditItem()
{
	FormEditor *fe = prjFrame->CreateFormEditor(this);
	return 1;
}

int InstrItem::SaveItem()
{
	FormEditor *fe = (FormEditor*)editor;
	if (fe)
		fe->GetForm()->SetParams();
	return 0; 
}

int InstrItem::CloseItem()
{
	return prjFrame->CloseEditor(this);
}
int InstrItem::ItemProperties()
{
	int ok = 0;
	PropertyBox *pb = prjFrame->CreatePropertyBox(this, 0);
	if (pb)
	{
		ok = pb->Activate(1);
		delete pb;
	}
	return ok;
}

int InstrItem::LoadProperties(PropertyBox *pb)
{
	int enableType = 0;
	pb->SetValue(PROP_NAME, name, 0);
	pb->SetValue(PROP_DESC, desc, 0);
	if (inc)
	{
		pb->SetValue(PROP_INUM, (long)inc->inum, 0);
		pb->SetSelection(PROP_ITYP, (void*)inc->instrType);
	}
	else
	{
		pb->SetValue(PROP_INUM, (long) NextInum(), 0);
		if (copyOf && copyOf->inc)
			pb->SetSelection(PROP_ITYP, (void*)copyOf->inc->instrType);
		else
			enableType = 1;
	}
	pb->EnableValue(PROP_ITYP, enableType);
	return 1;
}

int InstrItem::SaveProperties(PropertyBox *pb)
{
	int isNew = 0;
	int nameChange = 0;

	long inum = 0;
	pb->GetValue(PROP_INUM, inum);
	pb->GetValue(PROP_NAME, name);
	pb->GetValue(PROP_DESC, desc);
	if (!inc)
	{
		Opaque tp = 0;
		InstrMapEntry *instType = 0;
		pb->GetSelection(PROP_ITYP, (void**)&instType);
		if (!inum || !instType)
		{
			prjFrame->Alert("You must select an instrument type and number", "Wait...");
			return 0;
		}
		// If an instrument has a manufTmplt set, the template
		// will typically be something other than a pre-allocated instrument.
		// In that case we can't copy this instrument.
		if (instType->manufTmplt)
			tp = instType->manufTmplt(0);
		else if (instType->manufInstr)
		{
			if (copyOf && copyOf->inc && instType == copyOf->inc->instrType)
				tp = copyOf->inc->instrTmplt;
			tp = (Opaque) instType->manufInstr(0, tp);
		}
		// It's legit to create an instrument without a template.
		// When instantiated, you will get a "blank" instrument.
		// NB: the editors may not correctly edit such an entity!
		inc = theProject->mgr.AddInstrument(inum, instType, tp);
		if (!inc)
		{
			prjFrame->Alert("Could not create instrument instance", "Ooops...");
			return 0;
		}
		isNew = 1;
	}
	else
	{
		inc->inum = inum;
		const char *oldName = inc->GetName();
		nameChange = (!oldName || strcmp(oldName, name));
	}
	inc->SetName(name);
	inc->SetDesc(desc);
	if (isNew)
		prjFrame->InstrAdded(inc);
	else if (nameChange)
		prjFrame->InstrChanged(inc);
	return 1;
}

int InstrItem::CopyItem()
{
	InstrItem *inew = new InstrItem;
	inew->SetParent(parent);
	bsString nn("Copy of ");
	nn += name;
	inew->SetName(nn);
	inew->copyOf = this;
	prjTree->AddNode(inew);
	if (inew->ItemProperties())
	{
		prjTree->UpdateNode(inew);
		theProject->SetChange(1);
		return 1;
	}
	prjTree->RemoveNode(inew);
	delete inew;
	return 0;
}

int InstrItem::RemoveItem()
{
	bsString msg;
	msg = "Remove instrument ";
	msg += name;
	msg += '?';
	if (prjFrame->Verify(msg, "Wait...") == 1)
	{
		prjFrame->InstrRemoved(inc);
		theProject->SetChange(1);
		return 1;
	}
	return 0;
}

WidgetForm *InstrItem::CreateForm(int xo, int yo)
{
	if (inc == 0 || inc->instrType == 0)
		return new WidgetForm;

	const char *type = inc->instrType->itype;
	char form[MAX_PATH];
	strncpy(form, type, MAX_PATH-7);
	strcat(form, "Ed.xml");

	bsString path;
	theProject->FindForm(path, form);

	SynthEdit *ed;
	if (strcmp("MatrixSynth", type) == 0)
		ed = new MatrixSynthEdit;
	else if (strcmp("Tone", type) == 0)
		ed = new ToneSynthEdit;
	else if (strcmp("ToneFM", type) == 0)
		ed = new ToneFMSynthEdit;
	else if (strcmp("FMSynth", type) == 0)
		ed = new FMSynthEdit;
	else if (strcmp("AddSynth", type) == 0)
		ed = new AddSynthEdit;
	else if (strcmp("SubSynth", type) == 0)
		ed = new SubSynthEdit;
	else if (strcmp("WFSynth", type) == 0)
		ed = new WFSynthEdit;
	else if (strcmp("Chuffer", type) == 0)
		ed = new ChufferEdit;
	else if (strcmp("ModSynth", type) == 0)
		ed = new ModSynthEdit;
	else
		ed = new SynthEdit;
	if (ed)
	{
		ed->Load(path, xo, yo);
		ed->SetInstrument(inc);
	}
	return ed;
}

int InstrItem::Load(XmlSynthElem *node)
{
	// instruments are loaded by the instrument manager
	inc = theProject->mgr.LoadInstr(node);
	ProjectItem::Load(node);
	return 0;
}

int InstrItem::Save(XmlSynthElem *node)
{
	ProjectItem::Save(node);
	if (inc)
	{
		node->SetAttribute("id", (short)inc->inum);
		node->SetAttribute("type", inc->instrType->GetType());
		Instrument *ip = (Instrument *)inc->instrTmplt;
		if (ip)
			ip->Save(node);
	}
	return 0;
}

int InstrItem::NextInum()
{
	int inum = 1;
	InstrConfig *icnew = 0;
	while ((icnew = theProject->mgr.EnumInstr(icnew)) != 0)
	{
		if (icnew->inum >= inum)
			inum = icnew->inum+1;
	}
	return inum;
}

////////////////////////////////////////////////////////////////

int InstrList::Load(XmlSynthElem *node)
{
	int err = 0;
	XmlSynthElem *child = node->FirstChild();
	while (child)
	{
		if (child->TagMatch("instr"))
		{
			InstrItem *ii = new InstrItem;
			ii->SetParent(this);
			ii->Load(child);
			//ii->SetConfig(theProject->mgr.LoadInstr(child));
			prjTree->AddNode(ii);
		}
		XmlSynthElem *sib = child->NextSibling();
		delete child;
		child = sib;
	}
	return err;
}

int InstrList::Save(XmlSynthElem *node)
{
	XmlSynthElem *child = node->AddChild("instrlib");

	ProjectItem *pi = prjTree->FirstChild(this);
	while (pi)
	{
		if (pi->GetType() == PRJNODE_INSTR)
		{
			XmlSynthElem *instr = child->AddChild("instr");
			InstrItem *ii = (InstrItem *) pi;
			ii->Save(instr);
			delete instr;
		}
		pi = prjTree->NextSibling(pi);
	}
	delete child;
	return 0;
}

int InstrList::NewItem()
{
	InstrItem *inew = new InstrItem;
	inew->SetParent(this);
	inew->SetName("*New*");
	prjTree->AddNode(inew);
	if (inew->ItemProperties())
	{
		prjTree->UpdateNode(inew);
		inew->EditItem();
		theProject->SetChange(1);
		return 1;
	}
	prjTree->RemoveNode(inew);
	delete inew;
	return 0;
}
