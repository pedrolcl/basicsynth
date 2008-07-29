include ../../BasicSynth.cfg

$(BSBIN)/example02: main.cpp
	$(CPP) -lm $(CPPFLAGS) -o $@ main.cpp $(CMNLIB)

example02: $(BSBIN)/example02

clean:
	-rm main.o
	-rm $(BSBIN)/example02

main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h
