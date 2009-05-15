#include "ComposerGlobal.h"
#include "ComposerCore.h"

ProjectFrame::ProjectFrame()
{
}

ProjectFrame::~ProjectFrame()
{
}

////////////////// Project Tree Item Functions /////////////////////////////////

void ProjectFrame::EditItem()
{
	ProjectItem *itm = prjTree->GetSelectedNode();
	if (itm != NULL)
		OpenEditor(itm);
}

void ProjectFrame::SaveItem()
{
	ProjectItem *itm = prjTree->GetSelectedNode();
	if (itm)
		itm->SaveItem();
}

void ProjectFrame::CloseItem()
{
	ProjectItem *itm = prjTree->GetSelectedNode();
	if (itm)
		itm->CloseItem();
}

void ProjectFrame::NewItem()
{
	ProjectItem *itm = prjTree->GetSelectedNode();
	if (itm)
		itm->NewItem();
}

void ProjectFrame::AddItem()
{
	ProjectItem *itm = prjTree->GetSelectedNode();
	if (itm)
		itm->AddItem();
}

void ProjectFrame::CopyItem()
{
	ProjectItem *itm = prjTree->GetSelectedNode();
	if (itm)
		itm->CopyItem();
}

void ProjectFrame::RemoveItem()
{
	ProjectItem *itm = prjTree->GetSelectedNode();
	if (itm && itm->RemoveItem())
	{
		CloseEditor(itm);
		prjTree->RemoveNode(itm);
		delete itm;
	}
}

void ProjectFrame::ItemProperties()
{
	ProjectItem *itm = prjTree->GetSelectedNode();
	if (itm)
	{
		if (itm->ItemProperties())
			prjTree->UpdateNode(itm);
	}
}

////////////////// Edit Functions /////////////////////////////////

void ProjectFrame::EditUndo()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->Undo();
}


void ProjectFrame::EditRedo()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->Redo();
}


void ProjectFrame::EditCopy()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->Copy();
}


void ProjectFrame::EditCut()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->Cut();
}

void ProjectFrame::EditPaste()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->Paste();
}

void ProjectFrame::EditFind()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->Find();
}

void ProjectFrame::EditFindNext()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->FindNext();
}

void ProjectFrame::EditSelectAll()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->SelectAll();
}

void ProjectFrame::EditGoto()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->GotoLine(-1);
}

int ProjectFrame::SaveFile()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
	{
		ProjectItem *itm = vw->GetItem();
		if (itm)
			return itm->SaveItem();
	}
	return 0;
}

int ProjectFrame::CloseFile()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		return CloseEditor(vw->GetItem());
	return 0;
}

////////////////// Project Functions //////////////////////////////
int ProjectFrame::QuerySaveProject()
{
	int res = Verify("Project has unsaved changes. Save?", "Wait...");
	if (res == 1)
		return SaveProject(0);
	return res;
}

int ProjectFrame::NewProject()
{
	if (!CloseProject(1))
		return 0;

	const char *spc = ProjectItem::GetFileSpec(PRJNODE_PROJECT);
	const char *ext = ProjectItem::GetFileExt(PRJNODE_PROJECT);
	bsString path;
	if (!BrowseFile(0, path, spc, ext))
		return 0;

	theProject = new SynthProject;
	if (!theProject)
		return 0;

	int rv = theProject->NewProject(path);
	InitPlayer();
	return rv;
}

int ProjectFrame::OpenProject(const char *fname)
{
	bsString file;
	if (fname == 0)
	{
		const char *spc = ProjectItem::GetFileSpec(PRJNODE_PROJECT);
		const char *ext = ProjectItem::GetFileExt(PRJNODE_PROJECT);
		if (!BrowseFile(1, file, spc, ext))
			return 0;
		fname = file;
	}

	if (!CloseProject(1))
		return 0;

	theProject = new SynthProject;
	if (!theProject)
		return 0;

	if (theProject->LoadProject(fname))
	{
		bsString msg;
		msg = "Could not load project: ";
		msg += theProject->WhatHappened();
		Alert(msg, "Ooops...");
		prjTree->RemoveAll();
		theProject = 0;
		return 0;
	}
	InitPlayer();
	return 1;
}

int ProjectFrame::CloseProject(int query)
{
	if (theProject)
	{
		if (query && theProject->GetChanged())
		{
			if (QuerySaveProject() < 0)
				return 0;
		}

		CloseAllEditors();
		ClearPlayer();
		prjTree->RemoveAll();
		theProject = 0;
	}
	return 1;
}

int ProjectFrame::SaveProject(int saveas)
{
	bsString path;
	theProject->GetProjectPath(path);
	if (saveas || path.Length() == 0)
	{
		const char *spc = ProjectItem::GetFileSpec(PRJNODE_PROJECT);
		const char *ext = ProjectItem::GetFileExt(PRJNODE_PROJECT);
		if (!BrowseFile(0, path, spc, ext))
			return -1;
	}
	if (!SaveAllEditors(0))
		return -1;
	return theProject->SaveProject(path);
}

void ProjectFrame::GenerateStarted()
{
}

void ProjectFrame::GenerateFinished()
{
}
