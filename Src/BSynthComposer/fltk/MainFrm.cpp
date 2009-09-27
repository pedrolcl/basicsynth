#include "globinc.h"
#include "FormEditorFltk.h"
#include "TextEditorFltk.h"
#include "PropertiesDlgFltk.h"
#include "OptionsDlg.h"
#include "MainFrm.h"

MainFrame *mainWnd;

#define MENU_H 30
#define TABS_H 40
#define TREE_W 200
#define KBD_H  200
// TODO: keyboard height is now variable

MainFrame::MainFrame(int X, int Y, int W, int H, const char* t)
		: Fl_Double_Window(X, Y, W, H, t), tabs(0), tree(0), kbd(0)
{
	end();
	prjFileTypes = "Project files (*.bsprj)\tXML files (*.xml)\tAll files (*)";
	mainWnd = this;
	size_range(400, 200);
	mnu = new MainMenu(W);
	add(mnu);
	tree = new ProjectTreeFltk(0, MENU_H, TREE_W, H-(KBD_H+MENU_H));
	add(tree);
	tree->hide();
	kbd = new KbdGenDlg(0, H-KBD_H, W, KBD_H);
	add(kbd);
	kbd->hide();
	tabs = new TabsView(TREE_W, MENU_H, W-TREE_W, H-(MENU_H+KBD_H));
	add(tabs);
	tabs->hide();
	prjFrame = static_cast<ProjectFrame*>(this);
	curItem = 0;
	curEditor = 0;
	Layout();
}

MainFrame::~MainFrame()
{
	prjFrame = 0;
}

void MainFrame::resize(int X, int Y, int W, int H)
{
	prjOptions.frmLeft = X;
	prjOptions.frmTop = Y;
	prjOptions.frmWidth = W;
	prjOptions.frmHeight = H;
	Fl_Double_Window::resize(X, Y, W, H);
	if (mnu)
	{
		mnu->size(W, MENU_H);
		mnu->redraw();
	}
	Layout();
}

void MainFrame::Layout()
{
	int wx;
	int wy;
	int ww;
	int wh;
	int kh;

	wx = 0;
	wy = MENU_H;
	ww = w();
	wh = h() - MENU_H;
	kh = 0;

	if (kbd && kbd->visible())
	{
		kh = kbd->h();
		wh -= kh;
		kbd->resize(0, h()-kh, w(), kh);
		kbd->redraw();
	}

	if (tree && ((Fl_Widget*)tree)->visible())
	{
		tree->resize(wx, wy, TREE_W, wh);
		tree->redraw();
		wx += TREE_W;
		ww -= TREE_W;
	}

	if (tabs && tabs->visible())
	{
		tabs->resize(wx, wy, ww, TABS_H);
		tabs->redraw();
		wh -= TABS_H;
		wy += TABS_H;
	}

	int count = children();
	int index;
	for (index = 0; index < count; index++)
	{
		Fl_Widget *ch = child(index);
		if (ch && ch != mnu && ch != tree && ch != kbd && ch != tabs)
			ch->resize(wx, wy, ww, wh);
	}
}

void MainFrame::ViewProject()
{
	if (!tree)
		return;

	int v = !((Fl_Widget*)tree)->visible();
	if (v)
		tree->show();
	else
		tree->hide();
	mnu->CheckProject(v);
	Layout();
	redraw();
}

void MainFrame::ViewKeyboard()
{
	if (!kbd)
		return;

	int v = !kbd->visible();
	if (v)
		kbd->show();
	else
		kbd->hide();
	mnu->CheckKeyboard(v);
	Layout();
	redraw();
}

void MainFrame::Generate(int autostart, int todisk)
{
	int wasPlaying = StopPlayer();
	GenerateDlg *dlg = new GenerateDlg(todisk);
	dlg->show();
	if (autostart)
		dlg->OnStart(1);
	if (wasPlaying)
		StartPlayer();
}


void MainFrame::ProjectOptions()
{
	ProjectOptionsDlg *dlg = new ProjectOptionsDlg;
	dlg->DoModal();
	delete dlg;
}

void MainFrame::ItemSelected(ProjectItem *p)
{
	mnu->ItemSelected(p);
}

void MainFrame::ItemDoubleClick(ProjectItem *p)
{
	if (p)
	{
		if (!p->EditItem())
			p->ItemProperties();
	}
}

EditorView *MainFrame::GetActiveEditor()
{
	if (tabs)
		return tabs->GetActiveItem();
	return 0;
}

void MainFrame::EditorSelected(EditorView *vw)
{
	mnu->EditorSelected(vw);
	if (vw)
	{
		Fl_Widget *wdg = (Fl_Widget*)vw->GetPSData();
		int count = children();
		int index;
		for (index = 0; index < count; index++)
		{
			Fl_Widget *ch = child(index);
			if (ch && ch != mnu && ch != tree && ch != kbd && ch != tabs)
			{
				if (wdg != ch)
					ch->hide();
			}
		}
		wdg->show();
		damage(FL_DAMAGE_CHILD, wdg->x(), wdg->y(), wdg->w(), wdg->h());
		ProjectItem *pi = vw->GetItem();
		if (pi && pi == tree->GetSelectedNode())
			mnu->ItemSelected(pi);
	}
	else
		redraw();
}

int MainFrame::Exit()
{
	if (CloseProject(1))
	{
		prjOptions.Save();
		//exit(0);
		Fl::delete_widget(this);
	}
	return 0;
}

void FixFileSpec(char *out, const char *in)
{
	// types are in the MSVC resource form: Descr|pattern|Descr|pattern|...
	// fltk form is Desc (pattern)\tDescr (pattern)\t...
	int cls = 0;
	const char *p1 = in;
	char *p2 = out;
	char *pe = out+510;
	while (*p1 && p2 < pe)
	{
		if (*p1 == '|')
		{
			if (cls)
			{
				*p2++ = ')';
				*p2++ = '\t';
				cls = 0;
			}
			else
			{
				*p2++ = ' ';
				*p2++ = '(';
				cls = 1;
			}
			p1++;
		}
		else
			*p2++ = *p1++;
	}
	if (cls)
		*p2++ = ')';
	p1 = "\tAll Files (*)";
	while (*p1 && p2 < pe)
		*p2++ = *p1++;
	*p2 = 0;
}

int MainFrame::BrowseFile(int open, char *file, const char *spec, const char *ext)
{
	char spcbuf[512];
	FixFileSpec(spcbuf, spec);
	char *ret = fl_file_chooser(open ? "Open" : "Save", spcbuf, file);
	if (ret)
	{
		strncpy(file, ret, 512);
		return 1;
	}
	return 0;
}

int MainFrame::BrowseFile(int open, bsString& file, const char *spec, const char *ext)
{
	char fnbuf[512];
	strcpy(fnbuf, file);
	int ret = BrowseFile(open, fnbuf, spec, ext);
	if (ret)
	{
		file = fnbuf;
		return 1;
	}
	return 0;
}

void MainFrame::AfterOpenProject()
{
	bsString colorsFile;
	if (theProject->FindForm(colorsFile, prjOptions.colorsFile))
		SynthWidget::colorMap.Load(colorsFile);

	mnu->EnableProject(1);
	mnu->CheckProject(1);
	mnu->CheckKeyboard(1);
	tree->show();
	kbd->Load();
	kbd->show();
	tabs->show();
	Layout();
}

int MainFrame::NewProject()
{
	if (ProjectFrame::NewProject())
	{
		AfterOpenProject();
		return 1;
	}
	return 0;
}

int MainFrame::OpenProject(const char *fname)
{
	if (ProjectFrame::OpenProject(fname))
	{
		AfterOpenProject();
		return 1;
	}
	return 0;
}
int MainFrame::CloseProject(int q)
{
	if (ProjectFrame::CloseProject(0))
	{
		mnu->EnableProject(0);
		mnu->ItemSelected(0);
		mnu->EditorSelected(0);
		return 1;
	}
	return 0;
}

int MainFrame::Verify(const char *msg, const char *title)
{
	return fl_choice(msg, "Cancel", "OK", NULL);
}

PropertyBox *MainFrame::CreatePropertyBox(ProjectItem *pi, int type)
{
	PropertyBox *pb = 0;
	if (type == 0)
		type = pi->GetType();
	switch (type)
	{
	case PRJNODE_PROJECT:
	case PRJNODE_SYNTH:
	case PRJNODE_WAVEOUT:
		pb = (PropertyBox *) new ProjectPropertiesDlg();
		break;
	case PRJNODE_MIXER:
		pb = (PropertyBox *) new MixerSetupDlg(pi);
		break;
	case PRJNODE_REVERB:
	case PRJNODE_FLANGER:
	case PRJNODE_ECHO:
		break;
	case PRJNODE_NOTEFILE:
	case PRJNODE_SEQFILE:
	case PRJNODE_TEXTFILE:
	case PRJNODE_SCRIPT:
		pb = (PropertyBox *) new FilePropertiesDlg(pi);
		break;
	case PRJNODE_INSTR:
		pb = (PropertyBox *) new InstrPropertiesDlg(pi);
		break;
	case PRJNODE_WVTABLE:
		pb = (PropertyBox *) new NamePropertiesDlg(pi);
		break;
	case PRJNODE_WVFILE:
		pb = (PropertyBox *) new WavefilePropertiesDlg(pi);
		break;
	case PRJNODE_LIBLIST:
		break;
	case PRJNODE_LIB:
		break;
	case PRJNODE_LIBINSTR:
		break;
	}
	return pb;
}

FormEditor *MainFrame::CreateFormEditor(ProjectItem *pi)
{
	//printf("Create form editor %s\n", pi->GetName());
	int ww = w();
	int wx = 0;
	int wy = MENU_H+TABS_H;
	int wh = h() - wy;
	if (tree && ((Fl_Widget*)tree)->visible())
	{
		wx += tree->w();
		ww -= tree->w();
	}
	if (kbd && kbd->visible())
		wh -= kbd->h();
	FormEditorFltk *formEd = new FormEditorFltk(pi, wx, wy, ww, wh);
	add(formEd);
	formEd->SetPSData((void *)static_cast<Fl_Widget*>(formEd));
	tabs->AddItem(formEd);
	EditorSelected(formEd);

	return static_cast<FormEditor*>(formEd);
}

TextEditor *MainFrame::CreateTextEditor(ProjectItem *pi)
{
	int ww = w();
	int wx = 0;
	int wy = MENU_H+TABS_H;
	int wh = h() - wy;
	if (tree && ((Fl_Widget*)tree)->visible())
	{
		wx += tree->w();
		ww -= tree->w();
	}
	if (kbd && kbd->visible())
		wh -= kbd->h();
	TextEditorFltk *ed = new TextEditorFltk(wx, wy, ww, wh);
	add(ed);
	ed->SetPSData((void *)static_cast<Fl_Widget*>(ed));
	ed->SetItem(pi);
	tabs->AddItem(ed);
	EditorSelected(ed);

	return static_cast<TextEditor*>(ed);
}

void MainFrame::EditStateChanged()
{
	mnu->EditorSelected(tabs->GetActiveItem());
}

int MainFrame::OpenEditor(ProjectItem *pi)
{
	EditorView *ed = pi->GetEditor();
	if (ed)
		EditorSelected(ed);
	else
		pi->EditItem();
	return 1;
}

int MainFrame::CloseEditor(ProjectItem *pi)
{
	EditorView *ed = pi->GetEditor();
	int issel = ed == tabs->GetActiveItem();
	pi->SetEditor(0);
	if (ed)
	{
		Fl_Widget *wdg = (Fl_Widget *) ed->GetPSData();
		remove(wdg);
		tabs->RemoveItem(ed);
		delete ed;
	}
	if (issel)
		EditorSelected(tabs->GetActiveItem());
	if (pi == tree->GetSelectedNode())
		mnu->ItemSelected(pi);
	return 1;
}

int MainFrame::CloseAllEditors()
{
	tabs->RemoveAll();
	return 1;
}

int MainFrame::SaveAllEditors(int q)
{
	return 1;
}


int MainFrame::QueryValue(const char *prompt, char *value, int len)
{
	QueryValueDlg dlg;
	int ok = dlg.Activate(prompt, value, len);
	//make_current();
	return ok;
}

int MainFrame::Alert(const char *msg, const char *title)
{
	fl_message(msg);
	return 1;
}

void MainFrame::InitPlayer()
{
	kbd->InitInstrList();
	kbd->UpdateChannels();
}

void MainFrame::ClearPlayer()
{
	kbd->Clear();
}

int MainFrame::StopPlayer()
{
	return kbd->Stop();
}

void MainFrame::StartPlayer()
{
	kbd->Start();
}

// these functions are used when there is a "live" player
// that needs notifications of instrument changes...
void MainFrame::InstrAdded(InstrConfig *inst)
{
	kbd->AddInstrument(inst);
}

void MainFrame::InstrRemoved(InstrConfig *inst)
{
	kbd->RemoveInstrument(inst);
}

void MainFrame::InstrChanged(InstrConfig *inst)
{
	kbd->UpdateInstrument(inst);
}

void MainFrame::MixerChanged()
{
	kbd->UpdateChannels();
}
