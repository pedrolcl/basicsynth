//////////////////////////////////////////////////////////////////////
// BasicSynth instrument library load functions
//
// Copyright 2008, Daniel R. Mitchell
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "LoadInstrLib.h"
#include "AddSynth.h"
#include "FMSynth.h"
#include "MatrixSynth.h"
#include "SubSynth.h"
#include "Tone.h"
#include "WFSynth.h"

int LoadInstrLib(InstrManager& mgr, const char *fname)
{
	int err;
	XmlSynthDoc doc;
	XmlSynthElem *root = doc.Open((char*)fname);
	if (root != NULL)
		err = LoadInstrLib(mgr, root);
	else
		err = -1;
	doc.Close();
	return err;
}

int LoadInstrLib(InstrManager& mgr, XmlSynthElem *root)
{
	int err = 0;
	XmlSynthElem *instr = root->FirstChild();
	XmlSynthElem *next;
	while (instr != NULL)
	{
		if (instr->TagMatch("instr"))
		{
			if (LoadInstr(mgr, instr) == 0)
				err++;
		}
		next = instr->NextSibling();
		delete instr;
		instr = next;
	}
	return err;
}

InstrMapEntry *LoadInstr(InstrManager& mgr, XmlSynthElem *instr)
{
	if (instr == 0)
		return 0;

	InstrMapEntry *instEnt = 0;
	InstrMapEntry *instTyp = 0;

	instEnt = 0;
	long inum = 0;

	char *type = NULL;
	char *name = NULL;
	char *desc = NULL;
	if (!instr->GetAttribute("type", &type) && type)
	{
		Opaque tp;
		instTyp = mgr.FindType(type);
		if (instTyp)
		{
			if (instTyp->manufTmplt)
				tp = instTyp->manufTmplt(instr);
			else if (instTyp->manufInstr)
			{
				Instrument *ip = instTyp->manufInstr(&mgr, 0);
				if (ip)
					ip->Load(instr);
				tp = (Opaque) ip;
			}
			else
				tp = 0;
			instr->GetAttribute("id", inum);
			instEnt = mgr.AddInstrument(inum, instTyp, tp);
			if (instEnt)
			{
				instr->GetAttribute("desc", &desc);
				instr->GetAttribute("name", &name);
				instEnt->SetName(name);
				instEnt->SetDesc(desc);
				instEnt->SetType(type);
				delete name;
				delete desc;
			}
		}
		delete type;
	}
	return instEnt;
}

