include ../../BasicSynth.cfg
CORELIB=../Lib/core.a

CPPFLAGS+=-I$(BSDIR)/Src/Notelist

OBJS=\
AddSynthEd.o \
BuzzSynthEd.o \
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
MIDIControlEd.o \
ModSynthEd.o \
MixerEdit.o \
MixerItem.o \
NotelistItem.o \
PathList.o \
ProjectFrame.o \
ProjectItem.o \
ProjectOptions.o \
SoundBankItem.o \
SoundBankEd.o \
SliderWidget.o \
SubSynthEd.o \
SwitchWidget.o \
SynthEdit.o \
SynthItem.o \
SynthProject.o \
SynthProjectPS.o \
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

AddSynthEd.cpp: ComposerGlobal.h ComposerCore.h SynthEdit.h AddSynthEd.h

BuzzSynthEd.cpp: ComposerGlobal.h ComposerCore.h SynthEdit.h BuzzSynthEd.h

ChufferEd.cpp: ComposerGlobal.h ComposerCore.h SynthEdit.h ChufferEd.h

ComposerGlobal.cpp: ComposerGlobal.h

EnvelopeWidget.cpp: ComposerGlobal.h WindowTypes.h SynthWidget.h EnvelopeWidget.h

FileItem.cpp: ComposerGlobal.h WindowTypes.h ProjectItem.h

FMSynthEd.cpp: ComposerGlobal.h ComposerCore.h SynthEdit.h FMSynthEd.h

InstrItem.cpp: ComposerGlobal.h ComposerCore.h SynthEdit.h ToneSynthEd.h FMSynthEd.h MatSynthEd.h SubSynthEd.h AddSynthEd.h WFSynthEd.h ChufferEd.h ModSynthEd.h SoundBankEd.h

KeyboardForm.cpp: ComposerGlobal.h ComposerCore.h

KeyboardWidget.cpp: ComposerGlobal.h ComposerCore.h $(BSINC)/MIDIDefs.h

KnobWidget.cpp: ComposerGlobal.h WindowTypes.h SynthWidget.h KnobWidget.h WidgetForm.h

LibItem.cpp: ComposerGlobal.h WindowTypes.h ProjectItem.h

MatSynthEd.cpp: ComposerGlobal.h ComposerCore.h SynthEdit.h MatSynthEd.h

MIDIControlEd.cpp: ComposerGlobal.h ComposerCore.h MIDIControlEd.h

MixerEdit.cpp: ComposerGlobal.h ComposerCore.h MixerEdit.h

MixerItem.cpp: ComposerGlobal.h ComposerCore.h MixerEdit.h

ModSynthEd.cpp: ComposerGlobal.h ComposerCore.h SynthEdit.h ModSynthEd.h

NotelistItem.cpp: ComposerGlobal.h WindowTypes.h ProjectItem.h

PathList.cpp: ComposerGlobal.h WindowTypes.h ProjectItem.h

ProjectFrame.cpp: ComposerGlobal.h ComposerCore.h

ProjectItem.cpp: ComposerGlobal.h ComposerCore.h

ProjectOptions.cpp: ComposerGlobal.h ComposerCore.h

SliderWidget.cpp: ComposerGlobal.h WindowTypes.h SynthWidget.h SliderWidget.h WidgetForm.h

SoundBankEd.cpp: ComposerGlobal.h ComposerCore.h SynthEdit.h SoundBankEd.h

SoundBankItem.cpp: ComposerGlobal.h $(BSINC)/SFFile.h $(BSINC)/DLSFile.h WindowTypes.h ProjectItem.h

SubSynthEd.cpp: ComposerGlobal.h ComposerCore.h SynthEdit.h SubSynthEd.h

SwitchWidget.cpp: ComposerGlobal.h WindowTypes.h SynthWidget.h SwitchWidget.h WidgetForm.h

SynthEdit.cpp: ComposerGlobal.h ComposerCore.h SynthEdit.h

SynthItem.cpp: ComposerGlobal.h ComposerCore.h MIDIControlEd.h

SynthProject.cpp: ComposerGlobal.h ComposerCore.h

SynthProjectPS.cpp: ComposerGlobal.h ComposerCore.h $(BSINC)/WaveOutALSA.h 

SynthWidget.cpp: ComposerGlobal.h WindowTypes.h SynthWidget.h KnobWidget.h SliderWidget.h SwitchWidget.h TextWidget.h EnvelopeWidget.h KeyboardWidget.h WaveWidget.h WidgetForm.h

TextWidget.cpp: ComposerGlobal.h WindowTypes.h SynthWidget.h TextWidget.h WidgetForm.h

ToneSynthEd.cpp: ComposerGlobal.h ComposerCore.h SynthEdit.h ToneSynthEd.h

WavefileItem.cpp: ComposerGlobal.h WindowTypes.h ProjectItem.h

WaveoutItem.cpp: ComposerGlobal.h WindowTypes.h ProjectItem.h

WavetableEdit.cpp: ComposerGlobal.h ComposerCore.h WavetableEdit.h

WavetableItem.cpp: ComposerGlobal.h ComposerCore.h WavetableEdit.h

WFSynthEd.cpp: ComposerGlobal.h ComposerCore.h SynthEdit.h WFSynthEd.h

WidgetForm.cpp: ComposerGlobal.h ComposerCore.h
