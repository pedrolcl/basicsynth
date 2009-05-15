#ifndef _ENVWIDGET_H_
#define _ENVWIDGET_H_

class EnvelopeWidget : public SynthWidget
{
private:
	int numSegs;
	SegVals *vals;
	float start;
	int susOn;
	int border;
	float tmRange;
	float ampRange;
	wdgColor frClr;

public:
	EnvelopeWidget();
	~EnvelopeWidget();

	void SetTime(float t)
	{
		tmRange = t;
	}

	void SetLevel(float a)
	{
		ampRange = a;
	}

	void SetStart(float st)
	{
		start = st;
	}

	void SetSus(int on)
	{
		susOn = on;
	}

	void SetSegs(int n);
	void SetVal(int n, float rt, float lvl);
	virtual int Load(XmlSynthElem *elem);
	virtual void Paint(DrawContext dc);
};

#endif
