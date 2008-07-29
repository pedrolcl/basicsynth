include ../../BasicSynth.cfg

$(BSBIN)/example07: main.cpp
	$(CPP) -lm $(CPPFLAGS) -o $@ main.cpp $(CMNLIB)
	
example07: $(BSBIN)/example07

clean:
	-rm main.o
	-rm $(BSBIN)/example07
	
main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h \
	$(BSINC)/EnvGen.h $(BSINC)/GenWave.h \
	$(BSINC)/WaveTable.h $(BSINC)/GenWaveWT.h \
	$(BSINC)/Mixer.h $(BSINC)/DelayLine.h
