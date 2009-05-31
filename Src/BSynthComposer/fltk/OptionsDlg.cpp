#include "globinc.h"
#include "OptionsDlg.h"
#include "MainFrm.h"

static void OkCB(Fl_Widget *wdg, void *arg)
{
	((ProjectOptionsDlg*)arg)->OnOK();
}

static void CanCB(Fl_Widget *wdg, void *arg)
{
	((ProjectOptionsDlg*)arg)->OnCancel();
}

static void BrowseCB(Fl_Widget *wdg, void *arg)
{
	Fl_Input *inp = (Fl_Input *)arg;
	bsString file;
	file = inp->value();
	if (mainWnd->BrowseFile(1, file, "All files (*)", 0))
		inp->value(file);
}

ProjectOptionsDlg::ProjectOptionsDlg()
	: Fl_Window(100, 100, 540, 500, "Project Options")
{
	int txtHeight = 25;
	int txtSpace = 30;
	int ypos = 5;

	nameInp = new Fl_Input(90, ypos, 440, txtHeight, "Author: ");
	nameInp->value(prjOptions.defAuthor);
	ypos += txtSpace;

	cpyrInp = new Fl_Input(90, ypos, 440, txtHeight, "Copyright: ");
	cpyrInp->value(prjOptions.defCopyright);
	ypos += txtSpace;

	Fl_Box *lbl = new Fl_Box(5, ypos, 85, txtHeight, "Include:");
	incSco = new Fl_Check_Button(90, ypos, 80, txtHeight, "Notelists");
	incSco->value(prjOptions.inclNotelist);

	incSeq = new Fl_Check_Button(180, ypos, 80, txtHeight, "Sequences");
	incSeq->value(prjOptions.inclSequence);

	incTxt = new Fl_Check_Button(270, ypos, 80, txtHeight, "Text Files");
	incTxt->value(prjOptions.inclTextFiles);

	incScr = new Fl_Check_Button(360, ypos, 80, txtHeight, "Scripts");
	incScr->value(prjOptions.inclScripts);

	incLib = new Fl_Check_Button(450, ypos, 80, txtHeight, "Libraries");
	incLib->value(prjOptions.inclLibraries);

	ypos += txtSpace;

	prjfInp = new Fl_Input(90, ypos, 440, txtHeight, "Project Files: ");
	prjfInp->value(prjOptions.defPrjDir);
//	prjfBrowse = new Fl_Button(470, ypos, 20, txtHeight, "...");
//	prjfBrowse->callback(BrowseCB, (void*)&prjfInp);
	ypos += txtSpace;

	wvinInp = new Fl_Input(90, ypos, 440, txtHeight, "Wave Files: ");
	wvinInp->value(prjOptions.defWaveIn);
//	wvinBrowse = new Fl_Button(470, ypos, 20, txtHeight, "...");
//	wvinBrowse->callback(BrowseCB, (void*)&wvinInp);
	ypos += txtSpace;

	formInp = new Fl_Input(90, ypos, 440, txtHeight, "Form Files: ");
	formInp->value(prjOptions.formsDir);
//	formBrowse = new Fl_Button(470, ypos, 20, txtHeight, "...");
//	formBrowse->callback(BrowseCB, (void*)&formInp);
	ypos += txtSpace;

	colorsInp = new Fl_Input(90, ypos, 440, txtHeight, "Colors: ");
	colorsInp->value(prjOptions.colorsFile);
	ypos += txtSpace;

	libsInp = new Fl_Input(90, ypos, 440, txtHeight, "Library Files: ");
	libsInp->value(prjOptions.defLibDir);
//	libsBrowse = new Fl_Button(470, ypos, 20, txtHeight, "...");
//	libsBrowse->callback(BrowseCB, (void*)libsInp);
	ypos += txtSpace;

	latency = new Fl_Input(120, ypos, 100, txtHeight, "Playback Latency");
	char buf[80];
	snprintf(buf, 80, "%f", prjOptions.playBuf);
	latency->value(buf);

	okBtn = new Fl_Button(420, ypos, 50, txtHeight, "OK");
	okBtn->callback(OkCB, (void*)this);

	canBtn = new Fl_Button(480, ypos, 50, txtHeight, "Cancel");
	canBtn->callback(CanCB, (void*)this);
	ypos += txtSpace;

	end();
	resize(mainWnd->x()+100, mainWnd->y()+100, 540, ypos+5);
}

void ProjectOptionsDlg::OnOK()
{
	strcpy(prjOptions.defAuthor, nameInp->value());
	strcpy(prjOptions.defCopyright, cpyrInp->value());
	prjOptions.inclNotelist = incSco->value();
	prjOptions.inclSequence = incSeq->value();
	prjOptions.inclTextFiles = incTxt->value();
	prjOptions.inclScripts = incScr->value();
	prjOptions.inclLibraries = incLib->value();
	strcpy(prjOptions.defPrjDir, prjfInp->value());
	strcpy(prjOptions.defWaveIn, wvinInp->value());
	strcpy(prjOptions.formsDir, formInp->value());
	strcpy(prjOptions.colorsFile, colorsInp->value());
	strcpy(prjOptions.defLibDir, libsInp->value());
	prjOptions.playBuf = atof(latency->value());
	doneInput = 1;
}

void ProjectOptionsDlg::OnCancel()
{
	doneInput = 2;
}

int ProjectOptionsDlg::DoModal()
{
	doneInput = 0;
	set_modal();
	show();
	while (!doneInput)
		Fl::wait();

	return doneInput == 1 ? 1 : 0;
}

