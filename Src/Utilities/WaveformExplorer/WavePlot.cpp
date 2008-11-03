//////////////////////////////////////////////////////////////////////
// Plot the waveform to screen or metafile.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WavePlot.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWavePlot::CWavePlot()
{
	periods  = 1;
}

CWavePlot::~CWavePlot()
{
}

LRESULT CWavePlot::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC dc(m_hWnd);

	RECT rc;
	GetClientRect(&rc);
	FillRect(dc, &rc, (HBRUSH) GetStockObject(WHITE_BRUSH));
	InflateRect(&rc, -2, -2);
	Plot(dc, rc);
	return 0;
}

int CWavePlot::Plot(HDC dc, RECT& rc)
{
	int cx = rc.right - rc.left;
	int cy = rc.bottom - rc.top;

	if (cx > 0 && cy > 0)
	{
		int x, y;

		HPEN dots = CreatePen(PS_DOT, 0, RGB(128,128,128));
		HGDIOBJ sav = SelectObject(dc, dots);
		int mid = rc.top + (cy / 2);
		MoveToEx(dc, 0, mid, NULL);
		LineTo(dc, cx, mid);
		SelectObject(dc, sav);

		GenWaveI wv;
		wv.InitWT((float) periods * synthParams.sampleRate / (float) cx, WT_USR(0));

		float val = wv.Gen();
		y = (int) (val * mid);
		MoveToEx(dc, rc.left, mid - y, NULL);
		for (x = 0; x < cx; x++)
		{
			val = wv.Gen();
			y = (int) (val * mid);
			LineTo(dc, rc.left + x, mid - y);
		}
	}

	return 0;
}
