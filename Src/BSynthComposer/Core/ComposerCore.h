//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef COMPOSER_CORE_H
#define COMPOSER_CORE_H

#ifndef MAX_PATH
#define MAX_PATH 512
#endif
#ifdef _WIN32
# ifndef snprintf
#  define snprintf _snprintf
# endif
#endif

class ProjectOptions
{
public:
	char installDir[MAX_PATH];
	char formsDir[MAX_PATH];
	char colorsFile[MAX_PATH];
	char defAuthor[MAX_PATH];
	char defCopyright[MAX_PATH];
	char defPrjDir[MAX_PATH];
	char defLibDir[MAX_PATH];
	char defWaveIn[MAX_PATH];
	char defWaveOut[MAX_PATH];
	int inclNotelist;
	int inclSequence;
	int inclScripts;
	int inclTextFiles;
	int inclLibraries;
	int inclInstr;
	float playBuf;

	ProjectOptions();
	void Load();
	void Save();
};

extern ProjectOptions prjOptions;

#include "WindowTypes.h"
#include "ProjectItem.h"
#include "SynthWidget.h"
#include "KnobWidget.h"
#include "SliderWidget.h"
#include "SwitchWidget.h"
#include "TextWidget.h"
#include "EnvelopeWidget.h"
#include "WaveWidget.h"
#include "KeyboardWidget.h"
#include "WidgetForm.h"
#include "KeyboardForm.h"
#include "SynthEdit.h"

#endif
