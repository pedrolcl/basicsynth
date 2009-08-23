include ../BasicSynth.cfg

OBJS=\
LoadInstrLib.o \
AddSynth.o \
Chuffer.o \
FMSynth.o \
GMPlayer.o \
LFO.o \
PitchBend.o \
MatrixSynth.o \
MixerControl.o \
ModSynth.o \
SFPlayer.o \
SubSynth.o \
Tone.o \
WFSynth.o

$(INSTLIB): $(OBJS)
	-@rm $@
	$(AR) r $@ $(OBJS)

all: $(INSTLIB)

clean:
	-@rm $(OBJS)
	-@rm $(INSTLIB)

AddSynth.cpp: AddSynth.h Includes.h

Chuffer.cpp: Chuffer.h Includes.h

FMSynth.cpp: FMSynth.h Includes.h

LFO.cpp: LFO.h Includes.h

GMPlayer.cpp: GMPlayer.h $(BSINC)/SoundBank.h $(BSINC)/MIDIControl.h

PitchBend.cpp: PitchBend.h Includes.h

MatrixSynth.cpp: MatrixSynth.h Includes.h

MixerControl.cpp: MixerControl.h Includes.h

ModSynth.cpp: ModSynth.h ModSynthUG.h UGOscil.h UGFilter.h UGEnvGen.h UGDelay.h UGCalc.h Includes.h

SFPlayer.cpp: SFPlayer.h

SubSynth.cpp: SubSynth.h Includes.h

Tone.cpp: Tone.h Includes.h

WFSynth.cpp: WFSynth.h Includes.h
