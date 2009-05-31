//////////////////////////////////////////////////////////////////////
// BasicSynth - Specialized widget form for the virtual keyboard.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"

int KeyboardForm::Load(const char *fileName, int xo, int yo)
{
	SynthWidget *wdg;

	mainGroup->Clear();
	WidgetForm::Load(fileName, xo, yo);
	kbd = (KeyboardWidget*)mainGroup->FindID(2);

	wdg = mainGroup->FindID(22); // volume
	wdg->SetValue(1.0f);
	kbd->SetVolume(1.0f);

	wdg = mainGroup->FindID(25); // channel
	wdg->SetValue(0.0f);
	kbd->SetChannel(0);

	wdg = mainGroup->FindID(31); // rhythm
	wdg->SetValue(4);
	kbd->SetDuration(4);

	wdg = mainGroup->FindID(39); // sharps/flats
	wdg->SetValue(1);
	kbd->SetRecSharps(1);

	return 0;
}

int KeyboardForm::Start()
{
	if (theProject)
	{
		SynthWidget *wdg = mainGroup->FindID(30);
		wdg->SetState(1);
		Redraw(wdg);
		return theProject->Start();
	}
	return 0;
}

int KeyboardForm::Stop()
{
	if (theProject)
	{
		SynthWidget *wdg = mainGroup->FindID(30);
		wdg->SetState(0);
		Redraw(wdg);
		return theProject->Stop();
	}
	return 0;
}

void KeyboardForm::ValueChanged(SynthWidget *wdg)
{
	int n, m;
	switch (wdg->GetID())
	{
	case 22: // volume
		kbd->SetVolume(wdg->GetValue());
		break;
	case 26:
		wdg = mainGroup->FindID(25);
		n = (int) wdg->GetValue();
		m = theProject->mixInfo->GetMixerInputs();
		if (++n >= m)
			n = 0;
		kbd->SetChannel(n);
		wdg->SetValue((float)n);
		Redraw(wdg);
		break;
	case 31: // rhythm
		kbd->SetDuration(wdg->GetValue());
		break;
	case 30: // player on/off
		if (theProject)
		{
			if (wdg->GetState())
				theProject->Start();
			else
				theProject->Stop();
		}
		else
			wdg->SetState(0);
		break;
	case 37:
		kbd->SetRecord(wdg->GetState());
		break;
	case 38:
		kbd->SetRecGroup(wdg->GetState());
		break;
	case 39:
		kbd->SetRecSharps((int)wdg->GetValue());
		break;
	case 42:
		kbd->CopyNotes();
		break;
	case 44:
		// pop-up generate and route to speaker
		prjFrame->Generate(1, 1);
		break;
	case 45:
		// pop-up generate and route to disk
		prjFrame->Generate(1, 0);
		break;
	}
}