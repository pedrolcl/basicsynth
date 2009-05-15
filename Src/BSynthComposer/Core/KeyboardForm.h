#ifndef KEYBOARD_FORM_H
#define KEYBOARD_FORM_H

class KeyboardForm : public WidgetForm
{
private:
	KeyboardWidget *kbd;

public:
	KeyboardForm()
	{
		kbd = 0;
	}

	KeyboardWidget *GetKeyboard()
	{
		return kbd;
	}

	SynthWidget *GetInstrList()
	{
		return mainGroup->FindID(1);
	}

	int Start();
	int Stop();

	virtual int Load(const char *fileName, int xo, int yo);
	virtual void ValueChanged(SynthWidget *wdg);
};

#endif
