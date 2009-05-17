#include "globinc.h"
#include "MainFrm.h"

static void treeCB(Fl_Widget *w, void *v)
{
	ProjectItem *selItm = 0;
	Fl_Select_Browser* b = (Fl_Select_Browser*)w;
	int index = b->value();
	if (index)
		selItm = (ProjectItem*)b->data(index);
	mainWnd->ItemSelected(selItm);
	if (Fl::event_button2())
		printf("Button 2\n");
	if (Fl::event_clicks() > 0)
		mainWnd->ItemDoubleClick(selItm);
}

ProjectTreeFltk::ProjectTreeFltk(int X, int Y, int W, int H)
: Fl_Hold_Browser(X, Y, W, H, 0)
{
	prjTree = static_cast<ProjectTree*>(this);
	callback(treeCB);
}

ProjectTreeFltk::~ProjectTreeFltk()
{
	prjTree = 0;
}

void ProjectTreeFltk::RemoveAll()
{
	int count = size();
	int index;
	for (index = count; index > 0; index--)
	{
		ProjectItem *itm = (ProjectItem *)data(index);
		data(index, 0);
		delete itm;
	}
	clear();
}

int ProjectTreeFltk::FindItem(ProjectItem *itm)
{
	int count = size();
	int index;
	for (index = 1; index <= count; index++)
	{
		if (data(index) == (void*)itm)
			return index;
	}
	return 0;
}

ProjectItem *ProjectTreeFltk::GetSelectedNode()
{
	int count = size();
	int index;
	for (index = 1; index <= count; index++)
	{
		if (selected(index))
			return (ProjectItem *)data(index);
	}
	return 0;
}

void ProjectTreeFltk::SelectNode(ProjectItem *itm)
{
	int index = FindItem(itm);
	value(index);
}

void ProjectTreeFltk::AddNode(ProjectItem *itm, ProjectItem *sib)
{
	if (!itm)
		return;

	bsString str;
	ProjectItem *parent = itm->GetParent();
	ProjectItem *p2 = parent;
	while (p2)
	{
		str += "  ";
		p2 = p2->GetParent();
	}
	if (!itm->IsLeaf())
		str += '+';
	else
		str += ' ';
	str += itm->GetName();

	int count = size();
	int index = 0;
	if (sib)
		index = FindItem(sib) + 1;
	else if (parent)
	{
		// find the last child of the parent
		index = FindItem(parent);
		if (index)
		{
			while (++index <= count)
			{
				sib = (ProjectItem *) data(index);
				if (sib->GetParent() != parent)
					break;
			}
		}
		else
			index = 1;
	}
	else
		index = size()+1;

	insert(index, str, (void*)itm);
}

void ProjectTreeFltk::RemoveNode(ProjectItem *itm)
{
	if (!itm)
		return;
	int index = FindItem(itm);
	if (index)
		remove(index);
}

void ProjectTreeFltk::UpdateNode(ProjectItem *itm)
{
	if (!itm)
		return;

	int index = FindItem(itm);
	if (index)
	{
		bsString str;
		ProjectItem *parent = itm->GetParent();
		while (parent)
		{
			str += "  ";
			parent = parent->GetParent();
		}
		if (!itm->IsLeaf())
			str += '+';
		else
			str += ' ';
		str += itm->GetName();
		text(index, str);
	}
}

void ProjectTreeFltk::MoveNode(ProjectItem *itm, ProjectItem *prev)
{
}

ProjectItem *ProjectTreeFltk::FirstChild(ProjectItem *itm)
{
	if (!itm)
		return 0;

	int index = FindItem(itm);
	if (index)
	{
		ProjectItem *ch = (ProjectItem *)data(index+1);
		if (ch && ch->GetParent() == itm)
			return ch;
	}
	return 0;
}

ProjectItem *ProjectTreeFltk::NextSibling(ProjectItem *itm)
{
	if (!itm)
		return 0;

	int index = FindItem(itm);
	ProjectItem *sib = (ProjectItem *)data(index+1);
	if (sib && sib->GetParent() == itm->GetParent())
		return sib;
	return 0;
}
