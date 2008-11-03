This directory contains stuff for automated source code documentation using Doxygen.

Doxyfile.txt - Doxygen project file
BasicSynthLaout.xml - layout file for BasicSynth documentation

You can edit these files as desired to customize the documentation as you see fit.

Edit Doxyfile.txt and set the path names to the correct directories for your 
installation of BasicSynth. In general, replace:
"C:/Documents and Settings/Dan Mitchell/My Documents/BasicSynth"
with the root directory for BasicSynth (e.g., $HOME/basicsynth).

The default versions only scan the Include directory. If you want to document
the example instruments and/or notelist source, add those directories to the 
Doxyfile.txt line for INPUT=...

Output is produced in the BasicSynth/Doxygen/html directory by default.

For Windows users, get the Doxygen program from http://www.doxygen.org
or http://sourceforge.net/projects/doxygen and install it. If you want nice class
diagrams you must also install Graphviz (http://graphviz.org).

For Debian GNU/Linux users, there are apptitude packages for both.

apt-get install doxygen
apt-get install doxygen-doc
apt-get install doxygen-gui
apt-get install graphviz
apt-get install graphviz-doc

