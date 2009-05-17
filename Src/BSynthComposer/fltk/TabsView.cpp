#include "globinc.h"
#include "MainFrm.h"

static void tvCB(Fl_Widget *w, void *arg)
{
	mainWnd->EditorSelected((EditorView *)arg);
}

TabsView::TabsView(int X, int Y, int W, int H) 
	: Fl_Group(X, Y, W, H, 0), 
	chooser(0)
{
	chooser = new Fl_Choice(X, Y+5, W, H-10);
	end();
}

TabsView::~TabsView()
{
}

void TabsView::AddItem(EditorView *ed)
{
	ProjectItem *prjItm = ed->GetItem();
	if (prjItm)
	{
		int ndx = chooser->add(prjItm->GetName(), (int)0, tvCB, (void *)ed, 0);
		chooser->value(ndx);
		chooser->redraw();
	}
}

void TabsView::RemoveItem(EditorView *ed)
{
	const Fl_Menu_Item *mnuItm = chooser->menu();
	if (mnuItm == 0)
		return;
	int ndx = 0;
	const Fl_Menu_Item* mnuSel = chooser->mvalue();
	while (mnuItm->label())
	{
		if (mnuItm->user_data() == (void*)ed)
		{
			chooser->remove(ndx);
			if (mnuItm != mnuSel)
				chooser->value(mnuSel);
			else
				chooser->value(chooser->menu());
			break;
		}
		mnuItm++;
		ndx++;
	}
}

void TabsView::RemoveAll()
{
	const Fl_Menu_Item *mnuItm = chooser->menu();
	if (!mnuItm)
		return;
	while (mnuItm->label())
	{
		EditorView *ed = (EditorView *)mnuItm->user_data();
		if (ed)
		{
			ProjectItem *prjItm = ed->GetItem();
			if (prjItm)
				prjItm->SetEditor(0);
			// there may be a better place to do this...
			Fl_Widget *wdg = (Fl_Widget *) ed->GetPSData();
			if (wdg)
			{
				mainWnd->remove(wdg);
				ed->SetPSData(0);
			}
			delete ed;
		}
		mnuItm++;
	}
	chooser->clear();
}

void TabsView::SetActiveItem(EditorView *ed)
{
	const Fl_Menu_Item *mnuItm = chooser->menu();
	if (!mnuItm)
		return;
	while (mnuItm->label())
	{
		if (mnuItm->user_data() == (void*)ed)
		{
			chooser->value(mnuItm);
			break;
		}
		mnuItm++;
	}
}

EditorView *TabsView::GetActiveItem()
{
	const Fl_Menu_Item* mnuItm = chooser->mvalue();
	if (!mnuItm || !mnuItm->label())
	{
		mnuItm = chooser->menu();
		if (mnuItm)
			chooser->value(mnuItm);
	}
	if (mnuItm)
		return (EditorView *) mnuItm->user_data();
	return 0;
}
