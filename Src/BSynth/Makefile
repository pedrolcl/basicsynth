include ../BasicSynth.cfg

$(BSBIN)/BSynth: main.cpp $(CMNLIB) $(NLLIB) $(INSTLIB)
	$(CPP) -lxml2 -lm -I../Notelist $(CPPFLAGS) -o $@ main.cpp \
		$(NLLIB) $(INSTLIB) $(CMNLIB)
	-cp testinst.xml testprj.xml testprj.wav test.nl test2.nl jig.nl jig.xml $(BSBIN)
	-cp tst*synth.xml $(BSBIN)
	-cp tst*synth.nl $(BSBIN)

BSynth: $(BSBIN)/BSynth

clean:
	-rm $(BSBIN)/testinst.xml $(BSBIN)/testprj.xml
	-rm $(BSBIN)/testprj.wav
	-rm $(BSBIN)/test.nl $(BSBIN)/test2.nl
	-rm $(BSBIN)/BSynth
	-rm $(BSBIN)/tst*synth.xml
	-rm $(BSBIN)/tst*synth.nl
