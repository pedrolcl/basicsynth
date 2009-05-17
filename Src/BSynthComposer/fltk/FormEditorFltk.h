#ifndef _FORM_EDITOR_H
#define _FORM_EDITOR_H

class FormEditorFltk : 
	public Fl_Group,
	public FormEditor
{
protected:
	ProjectItem *item;
	WidgetForm *form;
	int setCapture;
	int formW;
	int formH;
	Fl_Scrollbar *vscrl;
	Fl_Scrollbar *hscrl;
	wdgColor bgColor;
	wdgColor fgColor;

public:
	FormEditorFltk(ProjectItem *pi, int x, int y, int w, int h);
	virtual ~FormEditorFltk();

	void draw();
	int handle(int e);
	void resize(int X, int Y, int W, int H);

	void CopyToClipboard();
	void PasteFromClipboard();

	ProjectItem *GetItem()
	{
		return item;
	}

	void SetItem(ProjectItem *p)
	{
		item = p;
		label(p->GetName());
	}

	void SetForm(WidgetForm *wf);
	
	WidgetForm *GetForm()
	{ 
		return form; 
	}

	void GetSize(int& cx, int& cy)
	{
		if (form)
			form->GetSize(cx, cy);
		else
		{
			cx = 100;
			cy = 100;
		}
	}

	void Undo() 
	{
		if (form)
			form->Cancel();
	}

	void Redo() { }
	void SelectAll() { }
	void GotoLine(int ln) { }
	void Find() { }
	void FindNext() { }
	void SetMarker() { }
	void SetMarkerAt(int line) { }
	void NextMarker() { }
	void PrevMarker() { }
	void ClearMarkers() { }

	void Cancel()
	{
		if (form)
			form->Cancel();
	}

	int IsChanged()
	{
		if (form)
			return form->IsChanged();
		return 0;
	}

	long EditState();

	void Copy() { CopyToClipboard(); }
	void Cut()  { CopyToClipboard(); }
	void Paste() { PasteFromClipboard(); }
	void Capture();
	void Release();
	void Resize();
	void Redraw(SynthWidget *wdg);
	SynthWidget *SystemWidget(const char *) { return 0; }
	void DrawWidget(SynthWidget *wdg);
	void OnScroll();
};

#endif
