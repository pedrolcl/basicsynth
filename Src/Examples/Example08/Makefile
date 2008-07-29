include ../../BasicSynth.cfg

$(BSBIN)/example08: main.cpp
	$(CPP) -lm $(CPPFLAGS) -o $@ main.cpp $(CMNLIB)
	
example08: $(BSBIN)/example08

clean:
	-rm main.o
	-rm $(BSBIN)/example08
	
main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h \
	$(BSINC)/EnvGen.h $(BSINC)/GenWave.h \
	$(BSINC)/WaveTable.h $(BSINC)/GenWaveWT.h \
	$(BSINC)/GenWaveX.h $(BSINC)/Mixer.h \
	$(BSINC)/Sequencer.h $(BSINC)/SequenceFile.h
