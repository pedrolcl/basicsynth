#ifndef PROJECTOPTIONS_H
#define PROJECTOPTIONS_H

#ifndef MAX_PATH
#define MAX_PATH 512
#endif
#if defined(_WIN32) && _WIN32
#define WIN32_REGISTRY 1
#endif

class ProjectOptions
{
public:
	char programName[MAX_PATH];
	char installDir[MAX_PATH];
	char formsDir[MAX_PATH];
	char colorsFile[MAX_PATH];
	char defAuthor[MAX_PATH];
	char defCopyright[MAX_PATH];
	char defPrjDir[MAX_PATH];
	char defLibDir[MAX_PATH];
	char defWaveIn[MAX_PATH];
	char defWaveOut[MAX_PATH];
	int inclNotelist;
	int inclSequence;
	int inclScripts;
	int inclTextFiles;
	int inclLibraries;
	int inclSoundFonts;
	int inclInstr;
	int inclMIDI;
	int midiDevice;
	char midiDeviceName[MAX_PATH];
	char waveDevice[MAX_PATH];
	float playBuf;
	int frmTop;
	int frmLeft;
	int frmWidth;
	int frmHeight;
	int frmMax;
#if defined(_WIN32) && _WIN32
	HWND dsoundHWND;
	GUID *waveID;
#endif

	ProjectOptions();
	void Load();
	void Save();
};

extern ProjectOptions prjOptions;

#endif
