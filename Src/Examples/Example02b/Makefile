include ../../BasicSynth.cfg

$(BSBIN)/example02b: main.cpp
	$(CPP) -lm $(CPPFLAGS) -o $@ main.cpp $(CMNLIB)
	
example02b: $(BSBIN)/example02b

clean:
	-rm main.o
	-rm $(BSBIN)/example02b
	
main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h
