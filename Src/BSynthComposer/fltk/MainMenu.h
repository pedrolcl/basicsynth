#ifndef MAIN_MENU_H
#define MAIN_MENU_H

class MainMenu : public Fl_Menu_Bar
{
public:
	MainMenu(int w);

	void ItemSelected(ProjectItem *pi);
	void EditorSelected(EditorView *vw);
	void EnableProject(int e);
	void EnableFile(int e);
	void EnableItem(int ndx, int e);
	void CheckProject(int ck);
	void CheckKeyboard(int ck);
};

#endif

