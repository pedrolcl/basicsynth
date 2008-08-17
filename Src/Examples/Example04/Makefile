include ../../BasicSynth.cfg

$(BSBIN)/example04: main.cpp
	$(CPP) -lm $(CPPFLAGS) -o $@ main.cpp $(CMNLIB)
	
example04: $(BSBIN)/example04

clean:
	-rm main.o
	-rm $(BSBIN)/example04
	
main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h
