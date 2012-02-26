//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
// setting these macros makes VisualStudio IntelliSense work correctly!
#ifdef _MSC_VER
#define wxUSE_MENUS 1
#define wxUSE_TOOLBAR 1
#define wxUSE_STATUSBAR 1
#define wxUSE_TREECTRL 1
#define wxUSE_NOTEBOOK 1
#define wxUSE_CONFIG 1
#define wxUSE_STDPATHS 1
#define wxUSE_REGEX 1
#define wxUSE_FILE 1
#define wxUSE_XRC 1
#define wxUSE_TEXTCTRL 1
#define wxUSE_BUTTON 1
#define wxUSE_TOGGLEBTN 1
#define wxUSE_RADIOBTN 1
#define wxUSE_GAUGE 1
//#define wxUSE_GRAPHICS_CONTEXT 1
#endif

#include <wx/app.h>
#include <wx/window.h>
#include <wx/button.h>
#include <wx/bmpbuttn.h>
#include <wx/radiobut.h>
#include <wx/tglbtn.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/textctrl.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/image.h>
#include <wx/config.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/dirdlg.h>
#include <wx/notebook.h>
#include <wx/treectrl.h>
#include <wx/gauge.h>
#include <wx/splitter.h>
#include <wx/toolbar.h>
#include <wx/filedlg.h>
#include <wx/dialog.h>
#include <wx/utils.h>
#include <wx/docview.h>
#include <wx/stdpaths.h>
#include <wx/file.h>
#include <wx/regex.h>
#include <wx/xrc/xmlres.h>
#include <wx/html/helpctrl.h>
#include <wx/stc/stc.h>

#include <wx/graphics.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <BasicSynth.h>
#include <SynthThread.h>
#include <Instruments.h>
#include <NLConvert.h>

#include <ComposerCore.h>

#ifdef _MSC_VER
#pragma warning(disable: 4800) // conversion to bool
#endif

#define ID_SPLITTER_WND 200
#define ID_PROJECT_WND  201
#define ID_KEYBOARD_WND 202
#define ID_TAB_WND      203

class SynthApp: public wxApp
{
	virtual bool OnInit();
    virtual void CleanUp();
};

