###########################################################################
# Master makefile for BasicSynth examples
#
# "make all" makes all executables
# "make [examplenn]" to make one specific example
# "make clean" removes the executable images from $(BSBIN)
#
# Dan Mitchell (http://basicsynth.com)
###########################################################################
include ../BasicSynth.cfg

EXAMPLES= \
example01 \
example02 \
example02a \
example02b \
example03 \
example04 \
example4a \
example05 \
example06 \
example07 \
example07a \
example07b \
example08 \
example09 \
example9a \
example10

EXES= \
$(BSBIN)/example01 \
$(BSBIN)/example02 \
$(BSBIN)/example02a \
$(BSBIN)/example02b \
$(BSBIN)/example03 \
$(BSBIN)/example04 \
$(BSBIN)/example4a \
$(BSBIN)/example05 \
$(BSBIN)/example06 \
$(BSBIN)/example07 \
$(BSBIN)/example07a \
$(BSBIN)/example07b \
$(BSBIN)/example08 \
$(BSBIN)/example09 \
$(BSBIN)/example9a \
$(BSBIN)/example10

all: $(EXAMPLES)
	@echo All done

clean:
	-@rm $(EXES)
	@echo $(BSBIN) cleaned
	
example01:
	cd Example01; make $(MAKEFLAGS)

example02:
	cd Example02; make $(MAKEFLAGS)

example02a:
	cd Example02a; make $(MAKEFLAGS)

example02b:
	cd Example02b; make $(MAKEFLAGS)

example03:
	cd Example03; make $(MAKEFLAGS)

example04:
	cd Example04; make $(MAKEFLAGS)

example4a:
	cd Example4a; make $(MAKEFLAGS)

example05:
	cd Example05; make $(MAKEFLAGS)

example06:
	cd Example06; make $(MAKEFLAGS)

example07:
	cd Example07; make $(MAKEFLAGS)

example07a:
	cd Example07a; make $(MAKEFLAGS)

example07b:
	cd Example07b; make $(MAKEFLAGS)

example08:
	cd Example08; make $(MAKEFLAGS)

example09:
	cd Example09; make $(MAKEFLAGS)

example9a:
	cd Example9a; make $(MAKEFLAGS)

example10:
	cd Example10; make $(MAKEFLAGS)
