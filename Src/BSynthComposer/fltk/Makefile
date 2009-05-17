include ../../BasicSynth.cfg

INCFLTK=-I$(HOME)/fltk-1.1.9 \
-I$(HOME)/fltk-1.1.9/png \
-I$(HOME)/fltk-1.1.9/jpeg \
-I$(HOME)/fltk-1.1.9/zlib

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
SynthWidgetFltk.o \
TabsView.o \
TextEditorFltk.o \
WvtableSelectDlg.o

LIBFLTK=$(HOME)/fltk-1.1.9/lib/libfltk.a \
$(HOME)/fltk-1.1.9/lib/libfltk_images.a \
$(HOME)/fltk-1.1.9/lib/libfltk_jpeg.a \
$(HOME)/fltk-1.1.9/lib/libfltk_png.a \
-ldl -lm -lXext -lX11

fltk: $(OBJS) $(CMNLIB) $(NLLIB) $(INSTLIB)
	$(CPP) $(XMLLIB) -lm $(CPPFLAGS) -o $@ $(OBJS) \
		../Lib/core.a -lfltk -lfltk_images \
		$(NLLIB) $(INSTLIB) $(CMNLIB)
		
clean:
	-rm $(OBJS)
	-rm fltk
