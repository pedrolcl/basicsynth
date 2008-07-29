// WavePlot.cpp: implementation of the CWavePlot class.
//
//////////////////////////////////////////////////////////////////////

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
		y = rc.top + (cy / 2);
		MoveToEx(dc, 0, y, NULL);
		LineTo(dc, cx, y);
		SelectObject(dc, sav);

		GenWaveI wv;
		wv.InitWT((float) periods * synthParams.sampleRate / (float) cx, WT_USR(0));

		float val = wv.Gen();
		y = (int) ((((val + 1.0) / 2) * (float) cy) + 0.5);
		MoveToEx(dc, rc.left, rc.top + (cy - y), NULL);
		for (x = 0; x < cx; x++)
		{
			val = wv.Gen();
			y = (int) (((val + 1.0) / 2) * (float) cy);
			LineTo(dc, rc.left + x, rc.top + (cy - y));
		}
		//LineTo(dc, rc.left + cx + 1, rc.top + (cy / 2));
	}

	return 0;
}
