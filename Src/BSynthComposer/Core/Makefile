include ../../BasicSynth.cfg
CORELIB=../Lib/core.a

CPPFLAGS+=-I$(BSDIR)/Src/Notelist

OBJS=\
AddSynthEd.o \
ChufferEd.o \
EnvelopeWidget.o \
FileItem.o \
FMSynthEd.o \
InstrItem.o \
KeyboardForm.o \
KeyboardWidget.o \
KnobWidget.o \
LibItem.o \
MatSynthEd.o \
ModSynthEd.o \
MixerEdit.o \
MixerItem.o \
NotelistItem.o \
PathList.o \
ProjectFrame.o \
ProjectItem.o \
SoundBankItem.o \
SoundBankEd.o \
SliderWidget.o \
SubSynthEd.o \
SwitchWidget.o \
SynthEdit.o \
SynthItem.o \
SynthProject.o \
SynthWidget.o \
TextWidget.o \
ToneSynthEd.o \
WavefileItem.o \
WaveoutItem.o \
WavetableEdit.o \
WavetableItem.o \
WFSynthEd.o \
WidgetForm.o

$(CORELIB): $(OBJS)
	-@rm $@
	@$(AR) r $@ $(OBJS)

all: $(CORELIB)

clean:
	-@rm $(OBJS)
	-@rm $(CORELIB)

