include ../BasicSynth.cfg

OBJS=\
LoadInstrLib.o \
AddSynth.o \
FMSynth.o \
LFO.o \
PitchBend.o \
MatrixSynth.o \
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

FMSynth.cpp: FMSynth.h Includes.h

LFO.cpp: LFO.h Includes.h

PitchBend.cpp: PitchBend.h Includes.h

MatrixSynth.cpp: MatrixSynth.h Includes.h

SubSynth.cpp: SubSynth.h Includes.h

Tone.cpp: Tone.h Includes.h

WFSynth.cpp: WFSynth.h Includes.h
