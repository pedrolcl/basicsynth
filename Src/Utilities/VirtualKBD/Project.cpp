/////////////////////////////////////////////////////////////////////////////
// Extract instrument definitions from BSynth project file.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Project.h"

int SynthProject::LoadSynth(XmlSynthElem *root, InstrManager& mgr)
{
	bsString fullPath;
	char *fname;
	int errcnt = 0;
	XmlSynthElem *child = root->FirstChild();
	XmlSynthElem *sib;

	while (child != NULL)
	{
		if (child->TagMatch("synth"))
		{
			child->GetAttribute("sr", sampleRate);
			child->GetAttribute("wt", wtSize);
			child->GetAttribute("usr", wtUser);
			InitSynthesizer((bsInt32)sampleRate, (bsInt32)wtSize, (bsInt32)wtUser);
			long wvNdx;
			long wvParts;
			long gibbs;
			bsInt32 *mult;
			double *amps;
			double *phs;
			XmlSynthElem *wvnode = child->FirstChild();
			while (wvnode)
			{
				if (wvnode->TagMatch("wvtable"))
				{
					if (wvnode->GetAttribute("ndx", wvNdx)
					 && wvnode->GetAttribute("parts", wvParts) && wvParts > 0)
					{
						if (!wvnode->GetAttribute("gibbs", gibbs))
							gibbs = 0;
						mult = new bsInt32[wvParts];
						amps = new double[wvParts];
						phs = new double[wvParts];
						long ptndx;
						for (ptndx = 0; ptndx < wvParts; ptndx++)
						{
							mult[ptndx] = 0;
							amps[ptndx] = 0.0;
							phs[ptndx] = 0.0;
						}
						ptndx = 0;
						XmlSynthElem *ptnode = wvnode->FirstChild();
						while (ptnode && ptndx < wvParts)
						{
							if (ptnode->TagMatch("part"))
							{
								long m;
								ptnode->GetAttribute("mul", m);
								mult[ptndx] = m;
								ptnode->GetAttribute("amp", amps[ptndx]);
								ptnode->GetAttribute("phs", phs[ptndx]);
								ptndx++;
							}
							sib = ptnode->NextSibling();
							delete ptnode;
							ptnode = sib;
						}

						wtSet.SetWaveTable(wvNdx, ptndx, mult, amps, phs, gibbs);
						delete mult;
						delete amps;
						delete phs;
					}
				}
				sib = wvnode->NextSibling();
				delete wvnode;
				wvnode = sib;
			}
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
			fname = 0;
			child->GetContent(&fname);
			if (fname)
			{
				if (FindOnPath(fullPath, fname))
				{
					if (LoadInstrLib(mgr, fname))
						errcnt++;
				}
				else
					errcnt++;
				delete fname;
			}
		}
		else if (child->TagMatch("instrlib"))
		{
			if (LoadInstrLib(mgr, child))
				errcnt++;
		}

		sib = child->NextSibling();
		delete child;
		child = sib;
	}

	return errcnt;
}

int SynthProject::LoadProject(char *prjFname, InstrManager& mgr)
{
	int errcnt = 0;

	XmlSynthDoc doc;
	XmlSynthElem *root;

	if ((root = doc.Open(prjFname)) == NULL)
		return -1;

	if (root->TagMatch("synthprj"))
	{
		errcnt = LoadSynth(root, mgr);
	}
	else if (root->TagMatch("instrlib"))
	{
		sampleRate = 44100;
		wtSize = 16384;
		wtUser = 0;
		InitSynthesizer(sampleRate, wtSize, wtUser);
		if (LoadInstrLib(mgr, root))
			errcnt++;
	}
	else
		errcnt++;

	delete root;
	doc.Close();

	return errcnt;
}

int SynthProject::FindOnPath(bsString& fullPath, char *fname)
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
