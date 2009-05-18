#ifndef PROJECT_OPTIONS_DLG
#define PROJECT_OPTIONS_DLG

class ProjectOptionsDlg : public Fl_Window
{
private:
	Fl_Input *nameInp;
	Fl_Input *cpyrInp;
	Fl_Check_Button *incSco;
	Fl_Check_Button *incSeq;
	Fl_Check_Button *incTxt;
	Fl_Check_Button *incScr;
	Fl_Input *prjfInp;
	Fl_Button *prjfBrowse;
	Fl_Input *wvinInp;
	Fl_Button *wvinBrowse;
	Fl_Input *formInp;
	Fl_Button *formBrowse;
	Fl_Input *colorsInp;
	Fl_Input *libsInp;
	Fl_Button *libsBrowse;
	Fl_Input *latency;
	Fl_Button *okBtn;
	Fl_Button *canBtn;

	int doneInput;

public:
	ProjectOptionsDlg();

	int DoModal();
	void OnOK();
	void OnCancel();
};

#endif
