include ../../BasicSynth.cfg

CPPFLAGS+=-I$(BSDIR)/Src/Notelist -I../Core -DPOSIX=1

OBJS=\
main.o \
FormEditorFltk.o \
KbdGenDlg2.o \
MainFrm.o \
MainMenu.o \
OptionsDlg.o \
ProjectTreeFltk.o \
PropertiesDlgFltk.o \
SoundBankSelectFltk.o \
SynthProjectFltk.o \
SynthWidgetFltk.o \
TabsView.o \
TextEditorFltk.o \
WvtableSelectDlg.o

fltk: $(OBJS) $(CMNLIB) $(NLLIB) $(INSTLIB)
	$(CPP) $(XMLLIB) $(CPPFLAGS) -o $@ $(OBJS) \
		../Lib/core.a -lfltk -lfltk_images \
		$(NLLIB) $(INSTLIB) $(CMNLIB) -lasound -lm
		
clean:
	-rm $(OBJS)
	-rm fltk

FormEditorFltk.o: \
	FormEditorFltk.cpp \
	FormEditorFltk.h

KbdGenDlg2.o: \
	KbdGenDlg2.cpp \
	KbdGenDlg2.h

MainFrm.o: \
	MainFrm.cpp \
	FormEditorFltk.h \
	TextEditorFltk.h \
	PropertiesDlgFltk.h \
	OptionsDlg.h \
	MainMenu.h \
	ProjectTreeFltk.h \
	MainFrm.h

MainMenu.o: \
	MainMenu.cpp \
	MainMenu.h \
	ProjectTreeFltk.h

OptionsDlg.o: \
	OptionsDlg.cpp \
	OptionsDlg.h

ProjectTreeFltk.o: \
	ProjectTreeFltk.cpp \
	ProjectTreeFltk.h

PropertiesDlgFltk.o: \
	PropertiesDlgFltk.cpp \
	PropertiesDlgFltk.h

SoundBankSelectFltk.o: \
	SoundBankSelectFltk.cpp \
	SoundBankSelectFltk.h

SynthProjectFltk.o: \
	SynthProjectFltk.cpp

SynthWidgetFltk.o: \
	SynthWidgetFltk.cpp

TabsView.o: \
	TabsView.cpp \
	TabsView.h

TextEditorFltk.o: \
	TextEditorFltk.cpp \
	TextEditorFltk.h

WvtableSelectDlg.o: \
	WvtableSelectDlg.cpp \
	WvtableSelectDlg.h
