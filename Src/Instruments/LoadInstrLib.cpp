
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
	XmlSynthDoc doc;
	XmlSynthElem *root = doc.Open((char*)fname);
	if (root == NULL)
		return -1;

	XmlSynthElem *next;
	XmlSynthElem *instr = root->FirstChild();
	while (instr != NULL)
	{
		if (instr->TagMatch("instr"))
			LoadInstr(mgr, instr);
		next = instr->NextSibling();
		delete instr;
		instr = next;
	}

	doc.Close();

	return 0;
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
		instTyp = mgr.FindType(type);
		if (instTyp)
		{
			Instrument *ip = instTyp->manufInstr(&mgr, 0);
			if (ip)
			{
				ip->Load(instr);
				instr->GetAttribute("id", inum);
				instEnt = mgr.AddInstrument(inum, instTyp, (Opaque) ip);
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
		}
		delete type;
	}
	return instEnt;
}

