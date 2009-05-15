#ifndef _KNOBWIDGET_H_
#define _KNOBWIDGET_H_

#pragma once
#define pi2 (3.141592654*2)
// PI * 1.25
#define pi125 3.926990817
// PI * 1.50
#define pi150 4.71238898

enum KnobImageType
{
	nullKnob = 0,
	plainKnob = 1,
	blackKnob = 2,
	baseKnob = 3
};

struct KnobData : public ImageData
{
	long highClr;
	long faceClr;
	long fgClr;
	long bgClr;
	int tick;
	int style;
	float angle;
	void *bm;

	KnobData()
	{
		bm = 0;
	}

	virtual ~KnobData()
	{
		DestroyImage();
	}

	virtual int Compare(ImageData *id)
	{
		if (ImageData::Compare(id))
		{
			KnobData *kd = (KnobData *)id;
			return style == kd->style
				&& angle == kd->angle
				&& faceClr == kd->faceClr
				&& highClr == kd->highClr
				&& fgClr == kd->fgClr
				&& bgClr == kd->bgClr;
		}
		return 0;
	}

	virtual void *GetImage(int n)
	{
		return bm;
	}

	// platform specific image creation functions
	virtual void CreateImage();
	virtual void DestroyImage();
};

extern WidgetImageCache *knobCache;

class KnobWidget : public SynthWidget
{
protected:
	float value;
	float minval;
	float maxval;
	float range;
	float scale;
	float warp[4];
	int prec;
	int moving;
	wdgPoint centerPt;
	wdgPoint valuePt;
	wdgPoint lastPt;
	wdgRect dial;
	wdgColor faceClr;
	wdgColor highClr;
	float faceAng;
	float faceAmt[3];
	float facePos[3];
	ImageCacheItem *knbImg;

	int tickMaj;
	int tickMin;
	int tickLen;
	int style;

public:
	KnobWidget();
	virtual ~KnobWidget();
	virtual void SetArea(wdgRect& r);
	virtual void SetValue(float v);
	virtual float GetValue();

	virtual int Tracking();
	virtual int SetFocus();
	virtual int LoseFocus();

	float Round(float v);
	void SetScale(float s);
	void SetRange(float lo, float hi, int s = 3);
	void SetFace(wdgColor& fc, wdgColor& hc, float ang = 135.0, int sty = 0);
	void SetTicks(int maj, int min, int len);
	void SetColors(wdgColor& bg, wdgColor& fg);
	int CalcIndicator();
	int GetKnobImage(KnobImageType t);
	int ChangeValue(int x, int y, int ctrl, int shift);

	virtual int BtnDown(int x, int y, int ctrl, int shift);
	virtual int BtnUp(int x, int y, int ctrl, int shift);
	virtual int MouseMove(int x, int y, int ctrl, int shift);
	virtual int Load(XmlSynthElem *elem);

	virtual void CalcCenter();
	virtual void CreateImage();
	virtual void Paint(DrawContext dc);
	virtual void DrawTicks(DrawContext dc, wdgRect& bk, wdgRect& fc);
};

class KnobBlack : public KnobWidget
{
private:
	wdgRect shaft;
public:
	virtual void CalcCenter();
	virtual void CreateImage();
	virtual void Paint(DrawContext dc);
};

class KnobBased : public KnobWidget
{
private:
	wdgRect base;
	wdgRect shaft;
public:
	virtual void CalcCenter();
	virtual void CreateImage();
	virtual void Paint(DrawContext dc);
};

class FrequencyGroup : public WidgetGroup
{
protected:
	SynthWidget *track;
	SynthWidget *fine;
	SynthWidget *course;
	float value;

public:
	FrequencyGroup();
	virtual ~FrequencyGroup();
	virtual float GetValue();
	virtual void SetValue(float v);
	virtual void SetPairs(int knbCourse, int knbFine, int lblCourse, int lblFine);
	virtual int Load(XmlSynthElem *elem);
	virtual SynthWidget *HitTest(int x, int y);
	virtual void ValueChanged(SynthWidget *wdg);
};

#endif
