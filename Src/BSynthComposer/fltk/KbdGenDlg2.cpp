#include "globinc.h"
#if POSIX
#include <pthread.h>
#endif
#include "MainFrm.h"

long GenerateDlg::playFrom;
long GenerateDlg::playTo;
long GenerateDlg::playLive = 0;


#if _WIN32
static CRITICAL_SECTION genDlgGuard;
HANDLE genThreadH;
DWORD  genThreadID;

static void GenDlgLock()
{
	EnterCriticalSection(&genDlgGuard);
}
static void GenDlgUnlock()
{
	LeaveCriticalSection(&genDlgGuard);
}

static DWORD WINAPI GenerateProc(void* param)
{
	try
	{
		theProject->Generate(!GenerateDlg::playLive, GenerateDlg::playFrom, GenerateDlg::playTo);
	}
	catch(...)
	{
	}
	prjGenerate->Finished();
	ExitThread(0);
	return 0;
}

int GenerateDlg::StartThread()
{
	InitializeCriticalSection(&genDlgGuard);
	genThreadH = CreateThread(NULL, 0, GenerateProc, NULL, CREATE_SUSPENDED, &genThreadID);
	if (genThreadH == INVALID_HANDLE_VALUE)
		return -1;
	ResumeThread(genThreadH);
	return 0;
}

void GenerateDlg::EndThread()
{
	WaitForSingleObject(genThreadH, 10000);
	DeleteCriticalSection(&genDlgGuard);
}

#endif

#if POSIX
static pthread_mutex_t genDlgGuard;
static pthread_t genThreadID;

// use a mutex to sync threads. 
// could possibly use pthread_barrier_* ?

static void GenDlgLock()
{
	pthread_mutex_lock(&genDlgGuard);
}

static void GenDlgUnlock()
{
	pthread_mutex_unlock(&genDlgGuard);
}

static void *GenerateProc(void *param)
{
	// synchronize with the main thread...
	GenDlgLock();
	GenDlgUnlock();
	try
	{
		theProject->Generate(!GenerateDlg::playLive, GenerateDlg::playFrom, GenerateDlg::playTo);
	}
	catch (...)
	{
	}
	prjGenerate->Finished();
	pthread_exit((void*)0);
	return 0;
}

int GenerateDlg::StartThread()
{
	pthread_mutex_init(&genDlgGuard, NULL);
	GenDlgLock();
	int err = pthread_create(&genThreadID, NULL, GenerateProc, 0);
	GenDlgUnlock();
	return err;
}

void GenerateDlg::EndThread()
{
	pthread_join(genThreadID, NULL); 
	pthread_mutex_destroy(&genDlgGuard);
}
#endif

static void FormatTime(Fl_Input *wdg, long secs)
{
	char txt[40];
	snprintf(txt, 40, "%3d:%02d", secs / 60, secs % 60);
	wdg->value(txt);
}

static long GetTimeValue(Fl_Input *wdg)
{
	char buf[80];
	strncpy(buf, wdg->value(), 80);
	long sec = 0;
	char *col = strchr(buf, ':');
	if (col == NULL)
	{
		sec = atol(buf);
	}
	else
	{
		*col++ = 0;
		sec = (atol(buf) * 60) + atol(col);
	}
	return sec;
}


static void CloseCB(Fl_Widget *wdg, void *arg)
{
	((GenerateDlg*)arg)->OnClose();
}

static void StartCB(Fl_Widget *wdg, void *arg)
{
	((GenerateDlg*)arg)->OnStart(0);
}

static void StopCB(Fl_Widget *wdg, void *arg)
{
	((GenerateDlg*)arg)->OnStop();
}

static void PauseCB(Fl_Widget *wdg, void *arg)
{
	((GenerateDlg*)arg)->OnPause();
}

GenerateDlg::GenerateDlg(int live) : Fl_Window(100, 100, 600, 320, "Generate")
{
	playLive = live;
	prjGenerate = static_cast<GenerateWindow*>(this);

	lastTime = 0;
	canceled = 0;

	Fl_Box *lbl;
	Fl_Group *grp;

	lbl = new Fl_Box(5, 5, 80, 30, "Out @->");
	lbl->labelcolor(0x00400000);
	grp = new Fl_Group(85, 5, 270, 30, 0);
	spkrBtn = new Fl_Light_Button(90, 5, 80, 30, "Live");
	spkrBtn->type(FL_RADIO_BUTTON);
	spkrBtn->value(playLive);
	diskBtn = new Fl_Light_Button(180, 5, 80, 30, "Disk");
	diskBtn->type(FL_RADIO_BUTTON);
	diskBtn->value(!playLive);
	grp->end();

	grp = new Fl_Group(0, 40, 270, 35, 0);
	lbl = new Fl_Box(5, 40, 80, 30, "Play");
	allBtn = new Fl_Light_Button(90, 40, 80, 30, "All");
	allBtn->type(FL_RADIO_BUTTON);
	allBtn->value(1);
	someBtn = new Fl_Light_Button(180, 40, 80, 30, "Part");
	someBtn->type(FL_RADIO_BUTTON);
	someBtn->value(0);
	grp->end();

	lbl = new Fl_Box(5, 75, 80, 30, "From");
	fromInp = new Fl_Input(90, 75, 170, 30, 0);
	lbl = new Fl_Box(5, 110, 80, 30, "To");
	toInp = new Fl_Input(90, 110, 170, 30, 0);

	startBtn = new Fl_Button(10, 145, 80, 30, "Start"); 
	startBtn->callback(StartCB, (void*)this);
	startBtn->activate();

	stopBtn = new Fl_Button(95, 145, 80, 30, "Stop"); 
	stopBtn->callback(StopCB, (void*)this);
	stopBtn->deactivate();

	pauseBtn = new Fl_Check_Button(180, 145, 80, 30, "Pause"); 
	pauseBtn->callback(PauseCB, (void*)this);
	pauseBtn->deactivate();
	pauseBtn->value(0);

	lbl = new Fl_Box(5, 180, 80, 30, "Time");
	tmInp = new Fl_Output(90, 180, 170, 30, 0);
	lbl = new Fl_Box(5, 215, 80, 30, "Peak Left");
	pkLft = new Fl_Output(90, 215, 170, 30, 0);
	lbl = new Fl_Box(5, 250, 80, 30, "Peak Right");
	pkRgt = new Fl_Output(90, 250, 170, 30, 0);

	closeBtn = new Fl_Button(90, 285, 170, 30, "Close");
	closeBtn->callback(CloseCB, (void*)this);

	msgInp = new Fl_Text_Display(280, 5, 310, 310, 0);
	msgInp->buffer(new Fl_Text_Buffer);

	end();
	resizable(0);
	resize((Fl::w() - w()) / 2, (Fl::h() - h()) / 2, w(), h());
	set_modal();
}

GenerateDlg::~GenerateDlg()
{
	prjGenerate = 0;
	//if (mainWnd)
	//	mainWnd->make_current();
}

void GenerateDlg::AddMessage(const char *s)
{
	GenDlgLock();
	lastMsg += s;
	lastMsg += '\n';
	GenDlgUnlock();
	Fl::awake((void*)4);
}

void GenerateDlg::UpdateTime(long tm)
{
	GenDlgLock();
	lastTime = tm;
	GenDlgUnlock();
	Fl::awake((void*)3);
}

void GenerateDlg::UpdatePeak(AmpValue lft, AmpValue rgt)
{
	GenDlgLock();
	lftPeak = lft;
	rgtPeak = rgt;
	GenDlgUnlock();
	// FLTK seems to drop messages, (at least under Windows)
	// so we only send the message after UpdateTime...
	//Fl::awake((void*)2);
}

void GenerateDlg::Finished()
{
	// The background thread has stopped generating...
	Fl::awake((void*)1);
}

int GenerateDlg::WasCanceled()
{
	return canceled;
}

void GenerateDlg::OnStart(int autoStart)
{
	canceled = 0;
	Fl_Text_Buffer *msgbuf = msgInp->buffer();
	msgbuf->append("---------- Start ----------\n");
	lastMsg = "";
	if (!autoStart)
	{
		if (someBtn->value())
		{
			playFrom = GetTimeValue(fromInp);;
			playTo = GetTimeValue(toInp);
		}
		else
		{
			playFrom = 0;
			playTo = 0;
		}
		playLive = spkrBtn->value();
	}
	lastTime = 0;
	FormatTime(tmInp, playFrom);
	lftPeak = 0;
	rgtPeak = 0;
	lftMax = 0;
	rgtMax = 0;
	FormatPeak();
	StartThread();
	stopBtn->activate();
	pauseBtn->activate();
	startBtn->deactivate();
	closeBtn->deactivate();
	while (Fl::wait() > 0)
	{
		void *msg = Fl::thread_message();
		if (msg)
		{
			if (msg == (void*)1)
				break;
			GenDlgLock();
			//if (msg == (void*)2)
			//	FormatPeak();
			if (msg == (void*)3)
			{
				FormatTime(tmInp, lastTime);
				FormatPeak();
			}
			else if (msg == (void*)4)
			{
				msgbuf->append(lastMsg);
				lastMsg = "";
			}
			GenDlgUnlock();
		}
	}
	EndThread();
	char buf[100];
	snprintf(buf, 100, "Peak: [%.6f, %.6f]\n-------- Finished ---------\n", lftMax, rgtMax);
	msgbuf->append(buf);
	stopBtn->deactivate();
	pauseBtn->deactivate();
	startBtn->activate();
	closeBtn->activate();
}

void GenerateDlg::OnStop()
{
	Fl_Text_Buffer *msgbuf = msgInp->buffer();
	msgbuf->append("*Cancel*\nHalting sequencer...\n");
	canceled = 1;
}

void GenerateDlg::OnPause()
{
	if (pauseBtn->value())
		theProject->Pause();
	else
		theProject->Resume();
}

void GenerateDlg::OnClose()
{
	Fl::delete_widget(this);
}

void GenerateDlg::FormatPeak()
{
	char pkText[200];
	snprintf(pkText,80, " %.6f", lftPeak);
	pkLft->value(pkText);

	snprintf(pkText,80, " %.6f", rgtPeak);
	pkRgt->value(pkText);

	if (lftPeak > 1.0 || rgtPeak > 1.0)
	{
		snprintf(pkText, 200, "Out of range (%.6f, %.6f) at %02d:%02d\n", 
			lftPeak, rgtPeak, lastTime / 60, lastTime % 60);
		msgInp->buffer()->append(pkText);
	}
	if (lftPeak > lftMax)
		lftMax = lftPeak;
	if (rgtPeak > rgtMax)
		rgtMax = rgtPeak;
}

////////////////////////////////////////////////////////////////////////////
/// Virtual Keyboard
////////////////////////////////////////////////////////////////////////////

static void InstrSelCB(Fl_Widget *wdg, void *arg)
{
	KbdGenDlg *dlg = (KbdGenDlg *)wdg->parent();
	if (dlg)
		dlg->OnInstrChange();
}

KbdGenDlg::KbdGenDlg(int X, int Y, int W, int H) : Fl_Group(X, Y, W, H, 0)
{
	form = 0;
	end();
	box(FL_NO_BOX);

	instrList = new Fl_Hold_Browser(X, Y+5, 200, H-15);
	instrList->callback(InstrSelCB);
	add(instrList);

	resizable(NULL);
}

KbdGenDlg::~KbdGenDlg()
{
	delete form;
}

int KbdGenDlg::handle(int e)
{
	int s = 0;
	int mx = Fl::event_x();
	int my = Fl::event_y();
	if (mx >= instrList->x() && mx <= instrList->x() + instrList->w()
	 && my >= instrList->y() && my <= instrList->y() + instrList->h())
	{
		//printf("Pass to hscroll\n");
		if (instrList->handle(e) && e == FL_PUSH)
			Fl::pushed(instrList);
		return 0;
	}

	if (form && (e == FL_PUSH || e == FL_DRAG || e == FL_RELEASE))
	{
		switch (e)
		{
		case FL_PUSH:
			form->BtnDn(mx, my, Fl::event_shift(), Fl::event_ctrl());
			break;
		case FL_DRAG:
			form->MouseMove(mx, my, Fl::event_shift(), Fl::event_ctrl());
			break;
		case FL_RELEASE:
			form->BtnUp(mx, my, Fl::event_shift(), Fl::event_ctrl());
			break;
		}
		return 1;
	}
	return 0;
}

void KbdGenDlg::resize(int X, int Y, int W, int H)
{
//	printf("FormEditor: resize(%d,%d,%d,%d)\n", X, Y, W,H);
	int dx = X - x();
	int dy = Y - y();
	Fl_Widget::resize(X, Y, W, H);
	if (form)
		form->MoveTo(X+5, Y+5);
	instrList->resize(instrList->x() + dx, instrList->y() + dy, instrList->w(), instrList->h());
}

void KbdGenDlg::draw()
{
	if (damage() == FL_DAMAGE_CHILD)
	{
		update_child(*instrList);
	}
	else
	{ // total redraw
		int offs[2] = {0,0};
		DrawContext ctx = (DrawContext)&offs[0];
		int part = damage() == FL_DAMAGE_USER1;
		if (!part)
			fl_push_clip(x(), y(), w(), h());
		Fl_Color clr = (Fl_Color) (bgColor << 8);
		fl_color(clr);
		fl_rectf(x(), y(), w(), h());
		fl_draw_box(FL_EMBOSSED_FRAME, x(), y(), w(), h(), clr);
		if (form)
			form->RedrawForm(ctx);
		if (!part)
			fl_pop_clip();
		// now draw all the children atop the background:
		draw_child(*instrList);
	}
}

void KbdGenDlg::Load()
{
	if (form)
		return;

	form = new KeyboardForm();
	form->SetFormEditor(this);
	wdgColor clr;
	if (SynthWidget::colorMap.Find("bg", clr))
		bgColor = clr;
	bsString fileName;
	theProject->FindForm(fileName, "KeyboardEd.xml");
	form->Load(fileName, 0, 0);
	SynthWidget *wdg = form->GetInstrList();
	wdgRect a = wdg->GetArea();
	wdg->Remove();
	delete wdg;

	instrList->resize(x()+a.x+5, y()+a.y+5, a.w, a.h);
	instrList->redraw();
	int sel = instrList->value();
	if (sel > 0 && form)
		form->GetKeyboard()->SetInstrument((InstrConfig*)instrList->data(sel));

	int cx = 200;
	int cy = 100;
	form->GetSize(cx, cy);
	cx += 15;
	cy += 15;
	size(cx, cy);
}

void KbdGenDlg::OnInstrChange()
{
	if (form)
	{
		int sel = instrList->value();
		if (sel > 0 && form)
			form->GetKeyboard()->SetInstrument((InstrConfig*)instrList->data(sel));
	}
}

int KbdGenDlg::Stop()
{
	if (form)
		return form->Stop();
	return 0;
}

int KbdGenDlg::Start()
{
	if (form)
		return form->Start();
	return 0;
}

void KbdGenDlg::Clear()
{
	instrList->clear();
	if (form)
		form->GetKeyboard()->SetInstrument(0);
}

void KbdGenDlg::InitInstrList()
{
	instrList->clear();
	if (!theProject)
		return;

	InstrConfig *ic = 0;
	while ((ic = theProject->mgr.EnumInstr(ic)) != 0)
		AddInstrument(ic);
	if (instrList->size() > 0)
	{
		instrList->select(1);
		if (form)
			form->GetKeyboard()->SetInstrument((InstrConfig*)instrList->data(1));
	}
}

void KbdGenDlg::AddInstrument(InstrConfig *ic)
{
	const char *nm = ic->GetName();
	if (*nm != '[')
		instrList->add(nm, (void*)ic);
}

void KbdGenDlg::RemoveInstrument(InstrConfig *ic)
{
	int count = instrList->size();
	for (int ndx = 1; ndx <= count; ndx++)
	{
		if (instrList->data(ndx) == ic)
		{
			instrList->remove(ndx);
			break;
		}
	}
}

void KbdGenDlg::UpdateInstrument(InstrConfig *ic)
{
	int count = instrList->size();
	for (int ndx = 1; ndx <= count; ndx++)
	{
		if (instrList->data(ndx) == ic)
		{
			instrList->text(ndx, ic->GetName());
			break;
		}
	}
}

void KbdGenDlg::UpdateChannels()
{
}


/////////////////////////////////////////////////////////////////////////////
// The virtual keyboard widget. See the Windows version for discussion.
/////////////////////////////////////////////////////////////////////////////

void KeyboardWidget::CopyToClipboard(bsString& str)
{
}

void KeyboardWidget::Paint(DrawContext dc)
{
	//printf("Keyboard Draw background %d,%d,%d,%d\n", x(), y(), w(), h());

	fl_color(92,92,64);
	fl_rectf(area.x, area.y, area.w, area.h);
	if (!rcWhite || !rcBlack)
		return;


	wdgRect *rp = rcWhite;
	int i;
	for (i = 0; i < whtKeys; i++)
	{
		if (fl_not_clipped(rp->x, rp->y, rp->w, rp->h))
		{
			//printf("Keyboard Draw white key %d = %d,%d,%d,%d\n", i, rp->x, rp->y, rp->w, rp->h);
			if (rp == rcLastKey)
				fl_color(128, 128, 128);
			else
				fl_color(240, 240, 230);
			fl_rectf(rp->x, rp->y, rp->w, rp->h);
			fl_color(8,8,8);
			fl_line(rp->x, rp->y, rp->x, rp->y+rp->h-1);
		}
		rp++;
	}

	rp = rcBlack;
	for (i = 0; i < blkKeys; i++)
	{
		if (fl_not_clipped(rp->x, rp->y, rp->w, rp->h))
		{
			//printf("Keyboard Draw black key %d = %d,%d,%d,%d\n", i, rp->x, rp->y, rp->w, rp->h);
			if (rp == rcLastKey)
				fl_color(64, 64, 64);
			else
				fl_color(8, 8, 8);
			fl_rectf(rp->x, rp->y, rp->w, rp->h);
		}
		rp++;
	}

	fl_color(0,0,0);
	int rw = (rcWhite[whtKeys-1].x + rcWhite[0].w) - rcWhite[0].x;
	fl_rect(rcWhite[0].x, rcWhite[0].y, rw, rcWhite[0].h);
}

/////////////////////////////////////////////////////////////////////////////
// MIDI Input
// For now, MIDI input is connected to the virtual keyboard widget and
// simulates hitting a "key" on the screen keyboard.
// This may change later into a generic MIDI in that cooperates with
// Sequencer and MIDIControl objects.
/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
static HMIDIIN midiIn;

struct MEvent
{
	short mmsg;
	short val1;
	short val2;
	DWORD ts;
};

static MEvent eventBuf[500];
static int ebWrite = 0;
static int ebRead = 0;

void CALLBACK MidiCB(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	KeyboardWidget *kbd = (KeyboardWidget *) dwInstance;
	if (wMsg == MIM_DATA)
	{
		while (ebRead != ebWrite)
		{
			MEvent *p = &eventBuf[ebRead++];
			if (ebRead >= 500)
				ebRead = 0;
			kbd->MidiRcv(p->mmsg, p->val1, p->val2, p->ts);
		}

		short mmsg = dwParam1 & 0xFF;
		if (mmsg != 0xfe)
			kbd->MidiRcv(mmsg, (dwParam1 >> 8) & 0xff, (dwParam1 >> 16) & 0xFF, dwParam2);
	}
	else if (wMsg == MIM_MOREDATA)
	{
		// buffer this event and return immediately.
		// on the next MIM_DATA, process the buffered events
		MEvent *p = &eventBuf[ebWrite++];
		if (ebWrite >= 500)
			ebWrite = 0;
		p->mmsg = dwParam1 & 0xFF;
		p->val1 = (dwParam1 >> 8) & 0xFF;
		p->val2 = (dwParam1 >> 16) & 0xFF;
		p->ts = dwParam2;
	}
}

void KeyboardWidget::MidiIn(int onoff)
{
	if (midiOn == onoff)
		return;
	midiOn = onoff;
	if (onoff)
	{
		if (midiIn == 0)
		{
			MMRESULT err = midiInOpen(&midiIn, prjOptions.midiDevice, (DWORD_PTR) MidiCB, 
				(DWORD_PTR) this, CALLBACK_FUNCTION|MIDI_IO_STATUS);
			if (err != MMSYSERR_NOERROR)
				return;
		}
		midiInStart(midiIn);
	}
	else if (midiIn)
	{
		midiInStop(midiIn);
		midiInClose(midiIn);
		midiIn = 0;
	}
}
#endif

#ifdef UNIX
// This is Linux only as it uses ALSA.
// Some code taken from seqdemo.c by Matthias Nagorni

static int midiHalt = 0;

// perform input through the ALSA sequencer.
// this is really the wrong way to go for BasicSynth.
// The driver has "decoded" the event but we must
// paste the info back together for compatibility.
// The MIDIController object will then "decode" things
// again... *sigh*
// So - suggest using the raw midi instead.
static void *MidiInputSeq(void *param)
{
	snd_seq_t *midiHandle;
	int midiPort;
	int numPoll;
	struct pollfd *midiPoll;

	if (snd_seq_open(&midiHandle, prjOptions.midiDeviceName, SND_SEQ_OPEN_INPUT, 0) < 0)
	{
		fprintf(stderr, "Cannot open '%s' as MIDI input\n", prjOptions.midiDeviceName);
		return 0;
	}
	snd_seq_set_client_name(midiHandle, "BasicSynth");
	midiPort = snd_seq_create_simple_port(midiHandle, "BasicSynth",
	           SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
	           SND_SEQ_PORT_TYPE_APPLICATION);
	if (midiPort < 0)
		return 0;

	numPoll = snd_seq_poll_descriptors_count(midiHandle, POLLIN);
	midiPoll = new struct pollfd[numPoll];
	snd_seq_poll_descriptors(midiHandle, midiPoll, numPoll, POLLIN);

	KeyboardWidget *kbd = (KeyboardWidget *)param;
	unsigned int timestamp = 0;

	while (!midiHalt)
	{
		if (poll(midiPoll, numPoll, 2000) > 0)
		{
			snd_seq_event_t *ev;
			do
			{
				snd_seq_event_input(midiHandle, &ev);
				switch (ev->type) 
				{
				case SND_SEQ_EVENT_CONTROLLER:
					kbd->MidiRcv(MIDI_CTLCHG|ev->data.control.channel, 
						ev->data.control.param, ev->data.control.value, ev->time.tick);
					break;
				case SND_SEQ_EVENT_PITCHBEND:
					kbd->MidiRcv(MIDI_CTLCHG|ev->data.control.channel, 
						ev->data.control.param, ev->data.control.value, ev->time.tick);
					break;
				case SND_SEQ_EVENT_PGMCHANGE:
					kbd->MidiRcv(MIDI_PRGCHG|ev->data.control.channel, 
						ev->data.control.param, ev->data.control.value, ev->time.tick);
					break;
				case SND_SEQ_EVENT_CHANPRESS:
					kbd->MidiRcv(MIDI_CHNAT|ev->data.control.channel, 
						ev->data.control.param, ev->data.control.value, ev->time.tick);
					break;
				case SND_SEQ_EVENT_NOTEON:
					kbd->MidiRcv(MIDI_NOTEON|ev->data.note.channel, 
						ev->data.note.note, ev->data.note.velocity, ev->time.tick);
					break;
				case SND_SEQ_EVENT_NOTEOFF: 
					kbd->MidiRcv(MIDI_NOTEOFF|ev->data.note.channel, 
						ev->data.note.note, ev->data.note.off_velocity, ev->time.tick);
					break;
				}
			    snd_seq_free_event(ev);
			} while (snd_seq_event_input_pending(midiHandle, 0) > 0);
		}
	}
	snd_seq_close(midiHandle);
}

static void *MidiInputRaw(void *param)
{
	snd_rawmidi_t *midiHandle;

	if (snd_rawmidi_open(&midiHandle, NULL, prjOptions.midiDeviceName, 0) < 0)
	{
		midiHalt = 1;
		return 0;
	}

	fprintf(stderr, "Opened MIDI %s\n", prjOptions.midiDeviceName);
	
	KeyboardWidget *kbd = (KeyboardWidget *)param;
	
	snd_rawmidi_drain(midiHandle); 

	unsigned char inb;
	unsigned char mmsg = 0;
	unsigned char val[2];
	int  valCount = 0;
	unsigned long timestamp = 0; // todo

	while (!midiHalt)
	{
		if (snd_rawmidi_read(midiHandle, &inb, 1) < 0)
			midiHalt = 1;
		else if (inb & 0x80)
		{
			// MIDI command byte
			if ((inb & 0xf0) == 0xf0)
			{
				switch (inb)
				{
				case MIDI_SYSEX:  //  0xF0
					while (snd_rawmidi_read(midiHandle, &inb, 1) < 0)
					{
						// todo: add data to sysex buffer
						if (inb == MIDI_ENDEX)
							break;
					}
					break;
				case MIDI_SNGPOS: // 0xF2
					snd_rawmidi_read(midiHandle, &inb, 1);
				case MIDI_SNGSEL: // 0xF3
					snd_rawmidi_read(midiHandle, &inb, 1);
					break;
				case MIDI_TMCODE: // 0xF1
				case MIDI_TUNREQ: // 0xF6
				case MIDI_ENDEX:  // 0xF7
				case MIDI_TMCLK:  // 0xF8
				case MIDI_START:  // 0xFA
				case MIDI_CONT:   // 0xFB
				case MIDI_STOP:   // 0xFC
				case MIDI_ACTSNS: // 0xFE
				case MIDI_META:   // 0xFF
					break;
				}
			}
			else
			{
				mmsg = inb;
				valCount = 0;
			}
		}
		else
		{
			val[valCount++] = inb;
			if (valCount == 2)
			{
				// todo: update timestamp
				if (mmsg != MIDI_KEYAT)
					kbd->MidiRcv(mmsg, val[0], val[1], timestamp);
				valCount = 0;
			}
		}
	}
	snd_rawmidi_drain(midiHandle);
	snd_rawmidi_close(midiHandle);
	fprintf(stderr, "Closed MIDI\n");

	return 0;
}

void KeyboardWidget::MidiIn(int onoff)
{
	static pthread_t midiThreadID;
	if (midiOn == onoff)
		return;
	if (onoff)
	{
		midiHalt = 0;
		fprintf(stderr, "Startl MIDI in\n");
		pthread_create(&midiThreadID, NULL, MidiInputRaw, this);
	}
	else
	{
		midiHalt = 1;
		fprintf(stderr, "Kill MIDI in\n");
		pthread_cancel(midiThreadID);
		pthread_join(midiThreadID, NULL); 
	}
	midiOn = onoff;
}
#endif

