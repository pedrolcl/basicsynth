#ifndef KBDGENDLG_H
#define KDBGENDLG_H

class KbdGenDlg : 
	public Fl_Group,
	public FormEditor
{
private:
	Fl_Hold_Browser *instrList;
	KeyboardForm *form;
	wdgColor bgColor;

public:
	KbdGenDlg(int X, int Y, int W, int H);
	~KbdGenDlg();

	void draw();
	int handle(int e);
	void resize(int X, int Y, int W, int H);

	virtual ProjectItem *GetItem() { return 0; }
	virtual void SetItem(ProjectItem *p) { }
	virtual void Undo() { }
	virtual void Redo() { }
	virtual void Cut() { }
	virtual void Copy() { }
	virtual void Paste() { }
	virtual void Find() { }
	virtual void FindNext() { }
	virtual void SelectAll() { }
	virtual void GotoLine(int ln) { }
	virtual void SetMarker() { }
	virtual void SetMarkerAt(int line) { }
	virtual void NextMarker() { }
	virtual void PrevMarker() { }
	virtual void ClearMarkers() { }
	virtual void Cancel() { }
	virtual long EditState() { return 0; }
	virtual int IsChanged()  { return 0; }
	virtual void SetForm(WidgetForm *frm) { }

	virtual WidgetForm *GetForm()
	{
		return form;
	}

	virtual void Capture() { }
	virtual void Release() { }

	virtual void Redraw(SynthWidget *wdg)
	{
		if (wdg)
		{
			wdgRect area = wdg->GetArea();
			damage(FL_DAMAGE_USER1, area.x, area.y, area.w, area.h);
			SynthWidget *bud = wdg->GetBuddy2();
			while (bud)
			{
				area = bud->GetArea();
				damage(FL_DAMAGE_USER1, area.x, area.y, area.w, area.h);
				bud = bud->GetBuddy2();
			}
		}
		else
			redraw();
	}

	virtual void Resize() { }
	SynthWidget *SystemWidget(const char *) { return 0; }

	void OnInstrChange();

	void Load();
	int IsRunning();
	int Stop();
	int Start();
	void Clear();
	void InitInstrList();
	void AddInstrument(InstrConfig *ic);
	void RemoveInstrument(InstrConfig *ic);
	void UpdateInstrument(InstrConfig *ic);
	void SelectInstrument(InstrConfig *ic);
	void UpdateChannels();
};

class GenerateDlg : 
	public Fl_Window,
	public GenerateWindow
{
private:
	bsString lastMsg;
	long lastTime;
	int canceled;
	int genAuto;

	AmpValue lftPeak;
	AmpValue rgtPeak;
	AmpValue lftMax;
	AmpValue rgtMax;

	Fl_Button *spkrBtn;
	Fl_Button *diskBtn;
	Fl_Button *allBtn;
	Fl_Button *someBtn;
	Fl_Input *fromInp;
	Fl_Input *toInp;
	Fl_Button *startBtn;
	Fl_Button *stopBtn;
	Fl_Text_Display *msgInp;
	Fl_Button *closeBtn;
	Fl_Input *tmInp;
	Fl_Output *pkLft;
	Fl_Output *pkRgt;

	KbdGenDlg *kbdDlg;

	int StartThread();
	void EndThread();

public:
	GenerateDlg(KbdGenDlg *d);
	~GenerateDlg();

	static long playLive;
	static long playFrom;
	static long playTo;

	virtual void AddMessage(const char *s);
	virtual void UpdateTime(long tm);
	virtual void UpdatePeak(AmpValue lft, AmpValue rgt);
	virtual void Finished();
	virtual int WasCanceled();

	void OnStart(int autoStart);
	void OnStop();
	void OnClose();
	void FormatPeak();
};

#endif
