include ../../BasicSynth.cfg

CPPFLAGS+=-I$(BSDIR)/Src/Notelist -I../Core -DPOSIX=1
ALSA=-lasound

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
SynthWidgetFltk.o \
TabsView.o \
TextEditorFltk.o \
WvtableSelectDlg.o

CORELIB=../Lib/core.a

fltk: $(BSBIN)/bsynthcomposer

$(BSBIN)/bsynthcomposer: $(OBJS) $(CMNLIB) $(NLLIB) $(INSTLIB) $(CORELIB)
	$(CPP) $(XMLLIB) $(CPPFLAGS) -o $(BSBIN)/bsynthcomposer $(OBJS) \
		$(CORELIB) -lfltk -lfltk_images \
		$(NLLIB) $(INSTLIB) $(CMNLIB) $(ALSA) -lm
		
clean:
	-rm $(OBJS)
	-rm $(BSBIN)/bsynthcomposer

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
	OptionsDlg.h \
	../Core/ProjectOptions.h

ProjectTreeFltk.o: \
	ProjectTreeFltk.cpp \
	ProjectTreeFltk.h

PropertiesDlgFltk.o: \
	PropertiesDlgFltk.cpp \
	PropertiesDlgFltk.h

SoundBankSelectFltk.o: \
	SoundBankSelectFltk.cpp \
	SoundBankSelectFltk.h

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
