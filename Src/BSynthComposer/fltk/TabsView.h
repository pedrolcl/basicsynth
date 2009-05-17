#ifndef TABS_VIEW_H
#define TABS_VIEW_H

class TabsView : public Fl_Group
{
private:
	Fl_Choice *chooser;

public:
	TabsView(int X, int Y, int W, int H);
	~TabsView();

	void AddItem(EditorView *ed);
	void RemoveItem(EditorView *ed);
	void RemoveAll();
	void SetActiveItem(EditorView *ed);
	EditorView *GetActiveItem();
};

#endif
