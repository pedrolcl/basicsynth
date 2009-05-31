//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "KeyboardDlg2.h"
//#include "GenerateDlg.h"

void KeyboardWidget::Paint(DrawContext dc)
{
	if (!rcWhite || !rcBlack)
		return;

	Graphics *gr = (Graphics*)dc;
	Region clipRgn;
	gr->GetClip(&clipRgn);
	int all = 0;
	if (upd.IsEmpty())
		all = 1;
	else
	{
		Rect rc(upd.x, upd.y, upd.w, upd.h);
		gr->SetClip(rc);
	}

	// I'm not sure why, but every now and then there is a memory exception
	// deep, deep down in GDI+. For now, we just trap the exception and blaze
	// onward. The worst case is part of the screen is not redrawn.
	try
	{
		//SolidBrush bk(Color(bgColor));
		//gr->Clear(Color(92,92,64));
		RectF wr(rcWhite[0].x, rcWhite[0].y, rcWhite[0].w, rcWhite[0].h);
		LinearGradientBrush lgw(wr, Color(255, 255, 255), Color(32,32,32), 0.0, FALSE);
		LinearGradientBrush lgk(wr, Color(220, 220, 220), Color(8,8,8), 0.0, FALSE);
		REAL fact[4] = { 1.0f, 0.0f, 0.0f, 1.0f, };
		REAL pos[4] = { 0.0f, 0.1f, 0.9f, 1.0f };
		lgw.SetBlend(fact, pos, 4);
		lgk.SetBlend(fact, pos, 4);
		wdgRect *rp = rcWhite;
		int i;
		for (i = 0; i < whtKeys; i++)
		{
			if (all || rp->Intersects(upd))
			{
				if (rp == rcLastKey)
					gr->FillRectangle(&lgk, rp->x, rp->y, rp->w, rp->h);
				else
					gr->FillRectangle(&lgw, rp->x, rp->y, rp->w, rp->h);
			}
			rp++;
		}

		SolidBrush lgb(Color(8,8,8));
		SolidBrush lgk2(Color(64,64,64));
		rp = rcBlack;
		for (i = 0; i < blkKeys; i++)
		{
			if (all || upd.Intersects(*rp))
			{
				if (rp == rcLastKey)
					gr->FillRectangle(&lgk2, rp->x, rp->y, rp->w, rp->h);
				else
					gr->FillRectangle(&lgb, rp->x, rp->y, rp->w, rp->h);
			}
			rp++;
		}
		Pen pn(Color(0,0,0));
		gr->DrawRectangle(&pn, area.x, area.y, area.w, area.h);
	}
	catch(...)
	{
		OutputDebugString("Funky exception in GDI+...\r\n");
	}
	upd.SetEmpty();
	gr->SetClip(&clipRgn);
}

// TODO: This should be moved to ProjectFrame
void KeyboardWidget::CopyToClipboard(bsString& str)
{
	OpenClipboard(_Module.mainWnd);
	EmptyClipboard();
	size_t len = str.Length() + 1;
	if (len > 1)
	{
		HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, len);
		if (h)
		{
			char *p2 = (char *) GlobalLock(h);
			strncpy(p2, (const char *)str, len);
			GlobalUnlock(h);
			SetClipboardData(CF_TEXT, h);
		} 
	}
	CloseClipboard();
}


///////////////////////////////////////////////////////////////////////////////

KeyboardDlg2::KeyboardDlg2()
{
	form = 0;
	bgColor.SetFromCOLORREF(RGB(0x80,0x80,0x80));
}

KeyboardDlg2::~KeyboardDlg2()
{
	delete form;
	DeleteObject(listFont);
}

void KeyboardDlg2::Load()
{
	form = new KeyboardForm();
	form->SetFormEditor(this);
	wdgColor clr;
	if (SynthWidget::colorMap.Find("bg", clr))
		bgColor.SetValue((ARGB)clr);
	bsString fileName;
	theProject->FindForm(fileName, "KeyboardEd.xml");
	form->Load(fileName, 0, 0);
	SynthWidget *wdg = form->GetInstrList();
	wdgRect a = wdg->GetArea();
	wdg->Remove();
	delete wdg;
	instrList.SetWindowPos(NULL, a.x, a.y, a.w, a.h, SWP_NOZORDER|SWP_NOACTIVATE);
	instrList.ShowWindow(SW_SHOW);
	if (instrList.GetCount() > 0)
	{
		instrList.SetCurSel(0);
		InstrConfig *ic = (InstrConfig*)instrList.GetItemDataPtr(0);
		form->GetKeyboard()->SetInstrument(ic);
	}

	int cx = 200;
	int cy = 100;
	form->GetSize(cx, cy);
	cx += 10;
	cy += 10;
	SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
}

LRESULT KeyboardDlg2::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// TODO: get the height of the standard font.
	listFont = CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 0, 0, 
		DEFAULT_CHARSET,  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
		DEFAULT_QUALITY, FF_DONTCARE|DEFAULT_PITCH , "Tahoma");
	RECT rcList;
	rcList.left = 5;
	rcList.top = 5;
	rcList.right = 190;
	rcList.bottom = 180;
	instrList.Create(m_hWnd, &rcList, NULL, WS_CHILD|WS_BORDER|LBS_NOTIFY|LBS_SORT|WS_VSCROLL, 0, 101);
	instrList.SetFont(listFont, 0);
	return 0;
}

LRESULT KeyboardDlg2::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(&ps);
	Graphics gr(dc);
	RECT rcWnd;
	GetClientRect(&rcWnd);
	Bitmap *offscrn = new Bitmap(rcWnd.right, rcWnd.bottom);
	Graphics *ctx = Graphics::FromImage(offscrn);
	SolidBrush br(bgColor);
	ctx->Clear(bgColor);
	if (form)
		form->RedrawForm((DrawContext)ctx);
	gr.DrawImage(offscrn, 0, 0);
	delete ctx;
	delete offscrn;

	EndPaint(&ps);
	return 0;
}

LRESULT KeyboardDlg2::OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// paint will completely redraw the background.
	return 0;
}

LRESULT KeyboardDlg2::OnBtnDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (form)
	{
		short x = LOWORD(lParam);
		short y = HIWORD(lParam);
		form->BtnDn((int)x, (int)y, 
			GetKeyState(VK_CONTROL) & 0x8000, 
			GetKeyState(VK_SHIFT) & 0x8000);
	}
	return 0;
}

LRESULT KeyboardDlg2::OnBtnUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (form)
	{
		short x = LOWORD(lParam);
		short y = HIWORD(lParam);
		form->BtnUp((int)x, (int)y, 
			GetKeyState(VK_CONTROL) & 0x8000, 
			GetKeyState(VK_SHIFT) & 0x8000);
	}
	return 0;
}

LRESULT KeyboardDlg2::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (form)
	{
		short x = LOWORD(lParam);
		short y = HIWORD(lParam);
		form->MouseMove((int)x, (int)y, 
			GetKeyState(VK_CONTROL) & 0x8000, 
			GetKeyState(VK_SHIFT) & 0x8000);
	}
	return 0;
}

LRESULT KeyboardDlg2::OnInstrChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (form)
	{
		InstrConfig *ic = 0;
		int sel = instrList.GetCurSel();
		if (sel != CB_ERR)
			ic = (InstrConfig*)instrList.GetItemDataPtr(sel);
		form->GetKeyboard()->SetInstrument(ic);
	}
	return 0;
}

int KeyboardDlg2::IsRunning()
{
	if (theProject)
		return theProject->IsPlaying();
	return 0;
}

int KeyboardDlg2::Stop()
{
	if (form)
		return form->Stop();
	return 0;
}

int KeyboardDlg2::Start()
{
	if (form)
		return form->Start();
	return 0;
}

void KeyboardDlg2::Clear()
{
	if (form)
	{
		form->Stop();
		form->GetKeyboard()->SetInstrument(0);
	}
	instrList.ResetContent();
}

void KeyboardDlg2::InitInstrList()
{
	instrList.ResetContent();
	if (!theProject)
		return;

	InstrConfig *ic = 0;
	while ((ic = theProject->mgr.EnumInstr(ic)) != 0)
		AddInstrument(ic);
	if (instrList.GetCount() > 0)
	{
		instrList.SetCurSel(0);
		ic = (InstrConfig*)instrList.GetItemDataPtr(0);
		if (form)
			form->GetKeyboard()->SetInstrument(ic);
	}
}

void KeyboardDlg2::AddInstrument(InstrConfig *ic)
{
	char fnbuf[20];
	const char *np = ic->GetName();
	if (*np == '[')
	{
		// internal control instrument - ignore it
		return;
	}
	if (!np || *np == 0)
	{
		np = fnbuf;
		sprintf(fnbuf, "#%d", ic->inum);
	}
	int ndx = instrList.AddString(np);
	instrList.SetItemDataPtr(ndx, ic);
}

void KeyboardDlg2::SelectInstrument(InstrConfig *ic)
{
	InstrConfig *ic2;
	int count = instrList.GetCount();
	int ndx = 0;
	while (ndx < count)
	{
		ic2 = (InstrConfig*)instrList.GetItemDataPtr(ndx);
		if (ic == ic2)
		{
			instrList.SetCurSel(ndx);
			if (form)
				form->GetKeyboard()->SetInstrument(ic);
			break;
		}
		ndx++;
	}
}

void KeyboardDlg2::RemoveInstrument(InstrConfig *ic)
{
	if (form)
		form->GetKeyboard()->SetInstrument(0);

	InstrConfig *ic2;
	int count = instrList.GetCount();
	int ndx = 0;
	while (ndx < count)
	{
		ic2 = (InstrConfig*)instrList.GetItemDataPtr(ndx);
		if (ic == ic2)
		{
			instrList.DeleteString(ndx);
			break;
		}
		ndx++;
	}
}

// this is called if the instrument name gets changed.
void KeyboardDlg2::UpdateInstrument(InstrConfig *ic)
{
	const char *nm = ic->GetName();
	if (nm == 0 || *nm == '[')
		return;
	InstrConfig *ic2;
	int count = instrList.GetCount();
	int ndx = 0;
	while (ndx < count)
	{
		ic2 = (InstrConfig*)instrList.GetItemDataPtr(ndx);
		if (ic == ic2)
		{
			instrList.DeleteString(ndx);
			ndx = instrList.AddString(nm);
			instrList.SetItemDataPtr(ndx, ic);
			break;
		}
		ndx++;
	}
}

void KeyboardDlg2::UpdateChannels()
{
}

