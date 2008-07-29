include ../../BasicSynth.cfg

$(BSBIN)/example06: main.cpp
	$(CPP) -lm $(CPPFLAGS) -o $@ main.cpp $(CMNLIB)
	
example06: $(BSBIN)/example06

clean:
	-rm main.o
	-rm $(BSBIN)/example06
	
main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h \
	$(BSINC)/EnvGen.h $(BSINC)/GenWave.h \
	$(BSINC)/WaveTable.h $(BSINC)/GenWaveWT.h \
	$(BSINC)/Mixer.h
