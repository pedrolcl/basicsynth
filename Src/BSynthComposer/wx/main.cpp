//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "MainFrame.h"

#if _MSC_VER
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Winmm.lib")
#endif

extern bool InitResources();

bool SynthApp::OnInit()
{
	// use '.' for decimal point
	setlocale(LC_NUMERIC, "C");

	wxInitAllImageHandlers();
	SetAppName("wxBasicsynth");
	SetAppDisplayName("BasicSynth");
	if (!InitResources())
	{
		wxMessageBox("Cannot find resource.xml", "Fatal");
		return false;
	}
	prjOptions.Load();

	knobCache = new WidgetImageCache;
	switchCache = new WidgetImageCache;

	MainFrame *frame = new MainFrame(prjOptions.programName,
		wxPoint(prjOptions.frmLeft,prjOptions.frmTop),
		wxSize(prjOptions.frmWidth,prjOptions.frmHeight));
	frame->Show( true );
	SetTopWindow( frame );
	return true;
}

void SynthApp::CleanUp()
{
	delete knobCache;
	delete switchCache;
}

IMPLEMENT_APP(SynthApp)

