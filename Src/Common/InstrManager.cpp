//////////////////////////////////////////////////////////////////
/// @file InstrManager.cpp Instrument manager file load functions.
//
// BasicSynth
//
// Sequencer code that is not inline
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SynthDefs.h>
#include <SynthString.h>
#include <WaveTable.h>
#include <WaveFile.h>
#include <Mixer.h>
#include <SynthList.h>
#include <XmlWrap.h>
#include <SeqEvent.h>
#include <Instrument.h>

// Load an instrument library from the file "fname"
// The file must be an XML file with a document node of "instrlib"
int InstrManager::LoadInstrLib(const char *fname)
{
	int err;
	XmlSynthDoc doc;
	XmlSynthElem *root = doc.Open((char*)fname);
	if (root != NULL)
		err = LoadInstrLib(root);
	else
		err = -1;
	doc.Close();
	return err;
}

// Load an instrument library from the XML document
// node "root". root must point to a node of type "instrlib"
int InstrManager::LoadInstrLib(XmlSynthElem *root)
{
	int err = 0;
	XmlSynthElem *instr = root->FirstChild();
	XmlSynthElem *next;
	while (instr != NULL)
	{
		if (instr->TagMatch("instr"))
		{
			if (LoadInstr(instr) == 0)
				err++;
		}
		else if (instr->TagMatch("wvtable"))
		{
			if (LoadWavetable(instr))
				err++;
		}
		next = instr->NextSibling();
		delete instr;
		instr = next;
	}
	return err;
}

// Load an instrument defintion from the XML node
// "instr" which must be of type <instr>.
InstrConfig *InstrManager::LoadInstr(XmlSynthElem *instr)
{
	if (instr == 0)
		return 0;

	InstrConfig *instEnt = 0;
	InstrMapEntry *instTyp = 0;

	instEnt = 0;
	long inum = 0;

	char *type = NULL;
	char *name = NULL;
	char *desc = NULL;
	if (instr->GetAttribute("type", &type) == 0)
	{
		Opaque tp;
		instTyp = FindType(type);
		if (instTyp)
		{
			if (instTyp->manufTmplt)
				tp = instTyp->manufTmplt(instr);
			else if (instTyp->manufInstr)
			{
				Instrument *ip = instTyp->manufInstr(this, 0);
				if (ip)
					ip->Load(instr);
				tp = (Opaque) ip;
			}
			else
				tp = 0;
			instr->GetAttribute("id", inum);
			instEnt = AddInstrument(inum, instTyp, tp);
			if (instEnt)
			{
				instr->GetAttribute("desc", &desc);
				instr->GetAttribute("name", &name);
				instEnt->SetName(name);
				instEnt->SetDesc(desc);
				delete name;
				delete desc;
			}
		}
		delete type;
	}
	return instEnt;
}

int InstrManager::LoadWavetable(XmlSynthElem *wvnode)
{
	short sumParts = 1;
	long wvID = -1;
	long wvNdx = -1;
	long wvParts = 0;
	long gibbs = 0;
	bsInt32 *mult;
	double *amps;
	double *phs;

	if (wvnode->GetAttribute("type", sumParts))
		sumParts = 1;
	if (wvnode->GetAttribute("parts", wvParts))
		return -1;
	if (wvParts <= 0)
		return -1;

	if (wvnode->GetAttribute("id", wvID) == 0)
	{
		wvNdx = wtSet.FindWavetable(wvID);
		if (wvNdx == -1)
		{
			wvNdx = wtSet.GetFreeWavetable(wvID);
			if (wvNdx == -1)
				wvNdx = wtSet.wavTblMax;
		}
	}
	else
	{
		if (wvnode->GetAttribute("ndx", wvNdx))
			return -1;
		wvID = wvNdx;
	}

	if (wvNdx >= wtSet.wavTblMax)
		wtSet.SetMax(wvNdx+4);
	wtSet.wavSet[wvNdx].wavID = wvID;

	wvnode->GetAttribute("gibbs", gibbs);
	mult = new bsInt32[wvParts];
	if (mult == 0)
		return -1;
	amps = new double[wvParts];
	if (amps == 0)
	{
		delete[] mult;
		return -1;
	}
	phs = new double[wvParts];
	if (phs == 0)
	{
		delete[] amps;
		delete[] mult;
		return -1;
	}
	long ptndx = 0;
	XmlSynthElem *ptnode = wvnode->FirstChild();
	XmlSynthElem *sib;
	while (ptnode && ptndx < wvParts)
	{
		if (ptnode->TagMatch("part"))
		{
			long m;
			ptnode->GetAttribute("mul", m);
			mult[ptndx] = (bsInt32) m;
			ptnode->GetAttribute("amp", amps[ptndx]);
			ptnode->GetAttribute("phs", phs[ptndx]);
			ptndx++;
		}
		sib = ptnode->NextSibling();
		delete ptnode;
		ptnode = sib;
	}

	if (sumParts == 1)
		wtSet.SetWaveTable(wvNdx, ptndx, mult, amps, phs, gibbs);
	else if (sumParts == 2)
		wtSet.SegWaveTable(wvNdx, ptndx, phs, amps);

	delete[] mult;
	delete[] amps;
	delete[] phs;

	return 0;
}
