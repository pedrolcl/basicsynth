include ../../BasicSynth.cfg

$(BSBIN)/example4a: main.cpp
	$(CPP) -lm $(CPPFLAGS) -o $@ main.cpp $(CMNLIB)
	
example4a: $(BSBIN)/example4a

clean:
	-rm main.o
	-rm $(BSBIN)/example4a
	
main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h
