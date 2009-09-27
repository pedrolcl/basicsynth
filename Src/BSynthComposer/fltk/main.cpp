#include "globinc.h"
#include "MainFrm.h"

int main(int argc, char *argv[])
{
	Fl::scheme("plastic");
	prjOptions.frmWidth = Fl::w();
	if (prjOptions.frmWidth > 1024)
		prjOptions.frmWidth = 1024;
	prjOptions.frmHeight = Fl::h();
	if (prjOptions.frmHeight > 768)
		prjOptions.frmHeight = 768;
	prjOptions.frmLeft = (Fl::w() - prjOptions.frmWidth) / 2;
	prjOptions.frmTop  = (Fl::h() - prjOptions.frmHeight) / 2;
	prjOptions.Load();
	MainFrame *mainwnd = new MainFrame(prjOptions.frmLeft, prjOptions.frmTop,
		prjOptions.frmWidth, prjOptions.frmHeight, prjOptions.programName);
	mainwnd->show();
#ifdef _WIN32
	prjOptions.dsoundHWND = fl_xid(mainwnd);
#endif
	Fl::lock();
 	int r = Fl::run();
	prjOptions.Save();
	return r;
}
