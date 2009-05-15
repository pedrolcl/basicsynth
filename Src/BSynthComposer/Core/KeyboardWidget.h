#ifndef KEYBOARD_WIDGET_H
#define KEYBOARD_WIDGET_H

class KeyboardWidget : public SynthWidget
{
private:
	int lastKey;
	int baseKey;
	int octs;
	int whtKeys;
	int blkKeys;
	int playing;
	wdgRect *rcWhite;
	wdgRect *rcBlack;
	wdgRect *rcLastKey;
	wdgRect upd;
	int knWhite[7];
	int knBlack[5];

	InstrConfig *activeInstr;
	InstrConfig *selectInstr;
	bsInt32 evtID;
	AmpValue curVol;
	FrqValue curDur;
	int curRhythm;
	int curChnl;

	class RecNote : public SynthList<RecNote>
	{
	public:
		int key;
		int dur;
		int vol;
		RecNote(int k, int d, int v)
		{
			key = k;
			dur = d;
			vol = v;
		}
	};

	int recording;
	int recGroup;
	int useSharps;
	RecNote *recHead;
	RecNote *recTail;

	int FindKey(int x, int y);
	void InvalidateLast();

	void PitchString(int pit, bsString& str);
	void RhythmString(int rhy, bsString& str);
	void NumberString(int val, bsString& str);
	void ClearNotes();
	void CopyToClipboard(bsString& pitches);

public:
	KeyboardWidget();
	~KeyboardWidget();
	void SetOctaves(int n);
	void Setup();
	virtual void SetArea(wdgRect& r);
	virtual int BtnDown(int x, int y, int ctrl, int shift);
	virtual int BtnUp(int x, int y, int ctrl, int shift);
	virtual int MouseMove(int x, int y, int ctrl, int shift);
	virtual int Tracking() { return playing; }
	virtual void Paint(DrawContext dc);
	virtual int Load(XmlSynthElem *elem);

	void SetInstrument(InstrConfig *ip)
	{
		selectInstr = ip;
	}

	void SetChannel(int n)
	{
		curChnl = n;
	}

	void SetDuration(float d)
	{
		curRhythm = (int) d;
		curDur = 2.0f / d;
	}

	void SetVolume(float v)
	{
		curVol = v;
	}

	void SetRecord(int on)
	{
		recording = on;
	}

	void SetRecGroup(int on)
	{
		recGroup = on;
	}

	void SetRecSharps(int on)
	{
		useSharps = on;
	}

	void CopyNotes();

	void PlayNote(int key, int e);
};

#endif
