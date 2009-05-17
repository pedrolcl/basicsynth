#ifndef PROJECT_TREE_FLTK_H
#define PROJECT_TREE_FLTK_H

class ProjectTreeFltk : 
	public Fl_Hold_Browser,
	public ProjectTree
{
private:
public:
	ProjectTreeFltk(int X, int Y, int W, int H);
	virtual ~ProjectTreeFltk();

	int FindItem(ProjectItem *itm);

	virtual void AddNode(ProjectItem *itm, ProjectItem *sib = 0);
	virtual void SelectNode(ProjectItem *itm);
	virtual void RemoveNode(ProjectItem *itm);
	virtual void UpdateNode(ProjectItem *itm);
	virtual void MoveNode(ProjectItem *itm, ProjectItem *prev);
	virtual void RemoveAll();
	virtual ProjectItem *FirstChild(ProjectItem *itm);
	virtual ProjectItem *NextSibling(ProjectItem *itm);
	virtual ProjectItem *GetSelectedNode();
};
#endif
