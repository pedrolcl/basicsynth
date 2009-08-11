#include "globinc.h"
#include "MainFrm.h"

static char ProductName[80] = "BasicSynth";
static char product[] = "basicsynth";

static int xtoi(const char *p)
{
	int h;
	int x = 0;
	while (*p)
	{
		if (*p >= 0 && *p <= '9')
			h = *p - '0';
		else if (*p >= 'A' && *p <= 'F')
			h = (*p - 'A') + 10;
		else if (*p >= 'a' && *p <= 'f')
			h = (*p - 'a') + 10;
		else
			break;
		x = (x << 4) | h;
		p++;
	}
	return x;
}

void ProjectOptions::Load()
{
	char *hm = getenv("HOME");
	if (hm == 0)
		hm = getenv("HOMEPATH"); // Windows name for HOME
	if (hm == 0)
		hm = "./";

	strcpy(prjOptions.installDir, hm);
	strcat(prjOptions.installDir, product);

	bsString cfgFile;
	cfgFile = hm;
	cfgFile += "/.bsynth";
	FILE *fp = fopen(cfgFile, "r");
	if (fp)
	{
		char lnbuf[1024];
		while (fgets(lnbuf, 1024, fp))
		{
			char *eq = strchr(lnbuf, '\n');
			if (eq)
				*eq = 0;
			if ((eq = strchr(lnbuf, '=')) != NULL)
			{
				*eq++ = 0;
				if (strcmp(lnbuf, "Install") == 0)
					strcpy(prjOptions.installDir, eq);
				else if (strcmp(lnbuf, "Forms") == 0)
					strcpy(prjOptions.formsDir, eq);
				else if (strcmp(lnbuf, "Colors") == 0)
					strcpy(prjOptions.colorsFile, eq);
				else if (strcmp(lnbuf, "Instrlib") == 0)
					strcpy(prjOptions.defLibDir, eq);
				else if (strcmp(lnbuf, "Projects") == 0)
					strcpy(prjOptions.defPrjDir, eq);
				else if (strcmp(lnbuf, "WaveIn") == 0)
					strcpy(prjOptions.defWaveIn, eq);
				else if (strcmp(lnbuf, "Author") == 0)
					strcpy(prjOptions.defAuthor, eq);
				else if (strcmp(lnbuf, "Copyright") == 0)
					strcpy(prjOptions.defCopyright, eq);
				else if (strcmp(lnbuf, "InclNotelist") == 0)
					prjOptions.inclNotelist = atoi(eq);
				else if (strcmp(lnbuf, "InclSequence") == 0)
					prjOptions.inclSequence = atoi(eq);
				else if (strcmp(lnbuf, "InclScripts") == 0)
					prjOptions.inclScripts = atoi(eq);
				else if (strcmp(lnbuf, "InclTextFiles") == 0)
					prjOptions.inclTextFiles = atoi(eq);
				else if (strcmp(lnbuf, "InclLibraries") == 0)
					prjOptions.inclLibraries = atoi(eq);
				else if (strcmp(lnbuf, "InclInstruments") == 0)
					prjOptions.inclInstr = xtoi(eq);
				else if (strcmp(lnbuf, "Latency") == 0)
					prjOptions.playBuf = atof(eq);
				else if (strcmp(lnbuf, "MIDIDeviceName") == 0)
					strcpy(prjOptions.midiDeviceName, eq);
				else if (strcmp(lnbuf, "MIDIDevice") == 0)
					midiDevice = atoi(eq);
			}
		}
		fclose(fp);
	}
	else
	{
#if _WIN32
		strcpy(prjOptions.installDir, "c:/Program Files/");
#else
		strcpy(prjOptions.installDir, "/usr/etc/");
#endif
		strcat(prjOptions.installDir, product);

		strcpy(prjOptions.defAuthor, "Me");
		strcpy(prjOptions.defCopyright, "2009");
		strcpy(prjOptions.defLibDir, prjOptions.installDir);
		strcat(prjOptions.defLibDir, "/libs");
		strcpy(prjOptions.defWaveIn, prjOptions.installDir);
		strcat(prjOptions.defWaveIn, "/wavefiles");
		strcpy(prjOptions.formsDir, prjOptions.installDir);
		strcat(prjOptions.formsDir, "/forms");
		return;
	}
	SynthProject::NormalizePath(prjOptions.installDir);
	SynthProject::NormalizePath(prjOptions.defPrjDir);
	SynthProject::NormalizePath(prjOptions.defLibDir);
	SynthProject::NormalizePath(prjOptions.defWaveIn);
	SynthProject::NormalizePath(prjOptions.formsDir);
}

void ProjectOptions::Save()
{
	char *hm = getenv("HOME");
	if (hm == 0)
	{
		hm = getenv("HOMEPATH"); // Windows name for HOME
		if (hm == 0)
			hm = ".";
	}
	bsString cfgFile;
	cfgFile = hm;
	cfgFile += "/.bsynth";
	FILE *fp = fopen(cfgFile, "w");
	if (fp)
	{
		fprintf(fp, "Install=%s\n", prjOptions.installDir);
		fprintf(fp, "Forms=%s\n", prjOptions.formsDir);
		fprintf(fp, "Colors=%s\n", prjOptions.colorsFile);
		fprintf(fp, "Instrlib=%s\n", prjOptions.defLibDir);
		fprintf(fp, "Projects=%s\n", prjOptions.defPrjDir);
		fprintf(fp, "WaveIn=%s\n", prjOptions.defWaveIn);
		fprintf(fp, "Author=%s\n", prjOptions.defAuthor);
		fprintf(fp, "Copyright=%s\n", prjOptions.defCopyright);
		fprintf(fp, "InclNotelist=%d\n", prjOptions.inclNotelist);
		fprintf(fp, "InclSequence=%d\n", prjOptions.inclSequence);
		fprintf(fp, "InclScripts=%d\n", prjOptions.inclScripts);
		fprintf(fp, "InclTextFiles=%d\n", prjOptions.inclTextFiles);
		fprintf(fp, "InclLibraries=%d\n", prjOptions.inclLibraries);
		fprintf(fp, "InclInstruments=%x\n", prjOptions.inclInstr);
		fprintf(fp, "Latency=%f\n", prjOptions.playBuf);
		fprintf(fp, "MIDIDeviceName=%s\n", prjOptions.midiDeviceName);
		fprintf(fp, "MIDIDevice=%d\n", prjOptions.midiDevice);
		fclose(fp);
	}
}

int main(int argc, char *argv[])
{
	prjOptions.Load();
	Fl::scheme("plastic");
	int width = Fl::w();
	if (width > 1024)
		width = 1024;
	int height = Fl::h();
	if (height > 768)
		height = 768;
	int left = (Fl::w() - width) / 2;
	int top  = (Fl::h() - height) / 2;
	MainFrame *mainwnd = new MainFrame(left, top, width, height, ProductName);
	mainwnd->show();
	Fl::lock();
 	int r = Fl::run();
	prjOptions.Save();
	return r;
}
