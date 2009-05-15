#include "StdAfx.h"
#include "TextEditor.h"
#include "FindReplDlg.h"

static FindReplDlg findDlg;

void LoadEditorDLL()
{
	HMODULE h;
#if _DEBUG
	h = LoadLibrary("SciLexerD.dll");
	if (!h)
#endif
	h = LoadLibrary("SciLexer.dll");
}

TextEditorWin::TextEditorWin()
{
	findFlags = 0;
	pi = 0;
	bgColor.SetFromCOLORREF(RGB(0x80,0x80,0x80));
}

TextEditorWin::~TextEditorWin()
{
	if ((TextEditorWin*)findDlg.GetEditor() == this)
		findDlg.SetEditor(0);
}

void TextEditorWin::TextChanged()
{
	changed = 1;
}

void TextEditorWin::UpdateUI()
{
	prjFrame->EditStateChanged();
}

ProjectItem *TextEditorWin::GetItem()
{
	return pi;
}

void TextEditorWin::SetItem(ProjectItem *p)
{
	pi = p;
	pi->SetEditor(this);
}

void TextEditorWin::Undo()
{
	edwnd.Undo();
}

void TextEditorWin::Redo()
{
	edwnd.Redo();
}

void TextEditorWin::Cut()
{
	edwnd.Cut();
}

void TextEditorWin::Copy()
{
	edwnd.Copy();
}

void TextEditorWin::Paste()
{
	edwnd.Paste();
}

void TextEditorWin::Find()
{
	if (!findDlg.IsWindow())
		findDlg.Create(_Module.mainWnd);
	findDlg.SetEditor(this);
	findDlg.ShowWindow(SW_SHOWNORMAL);
}

void TextEditorWin::FindNext()
{
	if (findText.Length() == 0)
		Find();
	else
		Find(findFlags, findText);
}

void TextEditorWin::SelectAll()
{
	edwnd.SetSelAll();
}

void TextEditorWin::GotoLine(int ln)
{
	if (ln < 0)
	{
		char value[40];
		value[0] = 0;
		if (!prjFrame->QueryValue("Line number:", value, 40))
			return;
		ln = atoi(value);
	}
	edwnd.GotoLine(ln);
}

void TextEditorWin::Cancel()
{
	edwnd.Undo();
}

long TextEditorWin::EditState()
{
	long flags = VW_ENABLE_FILE | VW_ENABLE_GOTO | VW_ENABLE_SELALL | VW_ENABLE_FIND;
	if (edwnd.CanPaste())
		flags |= VW_ENABLE_PASTE;
	if (edwnd.CanRedo())
		flags |= VW_ENABLE_REDO;
	if (edwnd.CanUndo())
		flags |= VW_ENABLE_UNDO;
	long start = 0;
	long end = 0;
	edwnd.GetSelection(start, end);
	if (start != end)
		flags |= VW_ENABLE_COPY|VW_ENABLE_CUT;
	return flags;
}

int TextEditorWin::IsChanged()
{
	return edwnd.IsChanged();
}

int TextEditorWin::IsKeyword(char *txt)
{
	int kw = 0;
	switch (*txt)
	{
	case 'A':
		if ( strcmp(txt, "ARTIC") == 0
		  || strcmp(txt, "AND") == 0
		  || strcmp(txt, "ADD") == 0)
			kw = 1;
		break;
	case 'B':
		kw = strcmp(txt, "BEGIN") == 0;
		break;
	case 'C':
		if ( strcmp(txt, "CHANNEL") == 0
		  || strcmp(txt, "CHNL") == 0
		  || strcmp(txt, "COUNT") == 0
		  || strcmp(txt, "CALL") == 0
		  || strcmp(txt, "CURPIT") == 0
		  || strcmp(txt, "CURDUR") == 0
		  || strcmp(txt, "CURVOL") == 0
		  || strcmp(txt, "CURTIME") == 0)
			kw = 1;
		break;
	case 'D':
		if (strcmp(txt, "DOUBLE") == 0
		 || strcmp(txt, "DO") == 0
		 || strcmp(txt, "VAR") == 0)
			kw = 1;
		break;
	case 'E':
		if (strcmp(txt, "END") == 0
		 || strcmp(txt, "EXP") == 0
		 || strcmp(txt, "EVAL") == 0
		 || strcmp(txt, "ELSE") == 0)
			kw = 1;
		break;
	case 'F':
		if (strcmp(txt, "FIXED") == 0
		 || strcmp(txt, "FGEN") == 0
		 || strcmp(txt, "FREQUENCY") == 0)
			kw = 1;
		break;
	case 'I':
		if (strcmp(txt, "INSTR") == 0
		 || strcmp(txt, "INSTRUMENT") == 0
		 || strcmp(txt, "INIT") == 0
		 || strcmp(txt, "INCLUDE") == 0
		 || strcmp(txt, "IF") == 0)
			kw = 1;
		break;
	case 'L':
		if (strcmp(txt, "LOOP") == 0
		 || strcmp(txt, "LINE") == 0
		 || strcmp(txt, "LOG") == 0)
			kw = 1;
		break;
	case 'M':
		if (strcmp(txt, "MARK") == 0
		 || strcmp(txt, "MAP") == 0
		 || strcmp(txt, "MAXPARAM") == 0
		 || strcmp(txt, "MIXER") == 0
		 || strcmp(txt, "MIDDLEC") == 0)
			kw = 1;
		break;
	case 'N':
		if (strcmp(txt, "NOT") == 0
		 || strcmp(txt, "NOTE") == 0)
			kw = 1;
		break;
	case 'O':
		if (strcmp(txt, "ON") == 0
		 || strcmp(txt, "OFF") == 0
		 || strcmp(txt, "OR") == 0
		 || strcmp(txt, "OPTION") == 0)
			kw = 1;
		break;
	case 'P':
		if (strcmp(txt, "PARAM") == 0
		 || strcmp(txt, "PLAY") == 0
		 || strcmp(txt, "PERCENT") == 0)
			kw = 1;
		break;
	case 'R':
		if (strcmp(txt, "RAND") == 0
		 || strcmp(txt, "REPEAT") == 0)
			kw = 1;
		break;
	case 'S':
		if (strcmp(txt, "SUS") == 0
		 || strcmp(txt, "SUSTAIN") == 0
		 || strcmp(txt, "SEQ") == 0
		 || strcmp(txt, "SEQUENCE") == 0
		 || strcmp(txt, "SYNC") == 0
		 || strcmp(txt, "SET") == 0
		 || strcmp(txt, "SCRIPT") == 0
		 || strcmp(txt, "SYS") == 0
		 || strcmp(txt, "SYSTEM") == 0)
			kw = 1;
		break;
	case 'T':
		if (strcmp(txt, "TIE") == 0
		 || strcmp(txt, "TRANSPOSE") == 0
		 || strcmp(txt, "TIME") == 0
		 || strcmp(txt, "TEMPO") == 0
		 || strcmp(txt, "THEN") == 0)
			kw = 1;
		break;
	case 'V':
		if (strcmp(txt, "VOICE") == 0
		 || strcmp(txt, "VOL") == 0
		 || strcmp(txt, "VOLUME") == 0
		 || strcmp(txt, "VAR") == 0
		 || strcmp(txt, "VARIABLE") == 0
		 || strcmp(txt, "VERSION") == 0
		 || strcmp(txt, "VER") == 0)
			kw = 1;
		break;
	case 'W':
		if (strcmp(txt, "WHILE") == 0
		 || strcmp(txt, "WRITE") == 0)
			kw = 1;
		break;
	}
	return kw;
}

// TODO: read the line into a buffer... Scintilla somtimes requests
// restyling all the way to the end of the file, even when only
// one char on one line has changed...
void TextEditorWin::Restyle(int position)
{
	int inkw = 0;
	int inq = 0;
	char kwbuf[40];
	int kwpos;
	int lineNumber = edwnd.CallScintilla(SCI_LINEFROMPOSITION, edwnd.SendMessage(SCI_GETENDSTYLED));
    int startPos = edwnd.CallScintilla(SCI_POSITIONFROMLINE, lineNumber);
#if _DEBUG
	int sizTotal = edwnd.CallScintilla(SCI_GETLENGTH, 0, 0);
	ATLTRACE("Restyle from %d to %d (total = %d)\n", startPos, position, sizTotal);
#endif
	int lenPos;
	int chPos = startPos;
	int ch = edwnd.CallScintilla(SCI_GETCHARAT, chPos);
	while (chPos < position)
	{
		if (strchr(",;{}[]():=", ch) != 0)
		{
			if (startPos != chPos)
			{
				edwnd.CallScintilla(SCI_STARTSTYLING, startPos, 0x1F);
				//ATLTRACE("Set styling 1 %d %d '%s'\n", startPos, (chPos - startPos));
				edwnd.CallScintilla(SCI_SETSTYLING, chPos - startPos, NLSTYLE_DEFAULT);
			}
			edwnd.CallScintilla(SCI_STARTSTYLING, chPos, 0x1F);
			//ATLTRACE("Set styling 2 (delimiter) %c %d\n", ch, chPos);
			edwnd.CallScintilla(SCI_SETSTYLING, 1, NLSTYLE_DELIM);
			ch = edwnd.CallScintilla(SCI_GETCHARAT, ++chPos);
			startPos = chPos;
		}
		else if (ch == '!' || ch == '\'')
		{
			if (startPos != chPos)
			{
				edwnd.CallScintilla(SCI_STARTSTYLING, startPos, 0x1F);
				//ATLTRACE("Set styling 3 %d\n", (chPos - startPos));
				edwnd.CallScintilla(SCI_SETSTYLING, chPos - startPos, NLSTYLE_DEFAULT);
			}
			lineNumber = edwnd.CallScintilla(SCI_LINEFROMPOSITION, chPos);
			int eol = edwnd.CallScintilla(SCI_GETLINEENDPOSITION, lineNumber);
			edwnd.CallScintilla(SCI_STARTSTYLING, chPos, 0x1F);
			//ATLTRACE("Set styling 4 %d\n", eol - chPos);
			edwnd.CallScintilla(SCI_SETSTYLING, eol - chPos, NLSTYLE_COMMENT);
			chPos = eol+1;
			ch = edwnd.CallScintilla(SCI_GETCHARAT, chPos);
			startPos = chPos;
		}
		else if (ch == '"')
		{
			if (startPos != chPos)
			{
				edwnd.CallScintilla(SCI_STARTSTYLING, startPos, 0x1F);
				//ATLTRACE("Set styling 5 %d\n", (chPos - startPos));
				edwnd.CallScintilla(SCI_SETSTYLING, chPos - startPos, NLSTYLE_DEFAULT);
			}
			startPos = chPos;
			do
			{
				ch = edwnd.CallScintilla(SCI_GETCHARAT, ++chPos);
			} while (chPos < position && ch != '"');
			lenPos = chPos - startPos;
			if (ch == '"')
				lenPos++;
			//ATLTRACE("Set styling 6 %d\n", lenPos);
			edwnd.CallScintilla(SCI_STARTSTYLING, startPos, 0x1F);
			edwnd.CallScintilla(SCI_SETSTYLING, lenPos, NLSTYLE_QUOTE);
			ch = edwnd.CallScintilla(SCI_GETCHARAT, ++chPos);
			startPos = chPos;
		}
		else if ((ch >= 'a' && ch < 'z') || (ch >= 'A' && ch <= 'Z'))
		{
			// possible keyword
			int delim = 0;
			kwpos = 0;
			int kwStart = chPos;
			do
			{
				if (kwpos < 39)
					kwbuf[kwpos++] = (ch >= 'a' && ch <= 'z') ? (ch - 'a' + 'A') : ch;
				ch = edwnd.CallScintilla(SCI_GETCHARAT, ++chPos);
			} while (strchr(" \t\n\r;:,(){}[]*/-+^&|#=<>\"'!", ch) == 0 && chPos < position);
			kwbuf[kwpos] = 0;
			int kw = 0;
			if (strchr("ABCDEFG", kwbuf[0]) != 0)
			{
				if (kwbuf[0] == '#' || kwbuf[0] == 'b' || isdigit(kwbuf[0]))
					kw = 0;
				else
					kw = IsKeyword(kwbuf);
			}
			else if (strchr("WHQES", kwbuf[0]) != 0)
			{
				if (kwbuf[0] == 0)
					kw = 0;
				else
					kw = IsKeyword(kwbuf);
			}
			else
				kw = IsKeyword(kwbuf);
			if (kw)
			{
				if (startPos != kwStart)
				{
					edwnd.CallScintilla(SCI_STARTSTYLING, startPos, 0x1F);
					//ATLTRACE("Set styling 7 %d %d '%s'\n", startPos, kwStart - startPos);
					edwnd.CallScintilla(SCI_SETSTYLING, kwStart - startPos, NLSTYLE_DEFAULT);
				}
				//ATLTRACE("Set styling 8 %d %d %s\n", kwStart, chPos - kwStart, kwbuf);
				edwnd.CallScintilla(SCI_STARTSTYLING, kwStart, 0x1F);
				edwnd.CallScintilla(SCI_SETSTYLING, chPos - kwStart, NLSTYLE_KEYWORD);
				startPos = chPos;
			}
		}
		else
			ch = edwnd.CallScintilla(SCI_GETCHARAT, ++chPos);
	}
	if (startPos != chPos)
	{
		edwnd.CallScintilla(SCI_STARTSTYLING, startPos, 0x1F);
		//ATLTRACE("Set styling 9 %d %d\n", startPos, chPos - startPos);
		edwnd.CallScintilla(SCI_SETSTYLING, chPos - startPos, NLSTYLE_DEFAULT);
	}
}

int TextEditorWin::OpenFile(const char *fname)
{
	if (pi == NULL)
		return -1;
	if (!theProject->FullPath(fname))
		theProject->FindOnPath(file, fname);
	else
		file = fname;

	HANDLE fh;
	fh = CreateFile(file, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fh == INVALID_HANDLE_VALUE)
		return -1;

	// get file size
	DWORD sizHigh = 0;
	DWORD siz = GetFileSize(fh, &sizHigh);
	if (sizHigh)
	{
		// bigger than 4 gigabytes? No way... :)
		CloseHandle(fh);
		return -2;
	}

	if (siz > 0)
	{
		char *text = new char[siz+1];
		if (text != NULL)
		{
			DWORD nread = 0;
			ReadFile(fh, (LPVOID)text, siz, &nread, NULL);
			text[siz] = 0;
			bsString tmp;
			tmp.Attach(text);
			SetText(tmp);
		}
	}

	edwnd.CallScintilla(SCI_SETSAVEPOINT);

	CloseHandle(fh);
	return 0;
}

int TextEditorWin::SaveFile(const char *fname)
{
	if (pi == NULL)
		return -1;

	if (fname == 0 || *fname == 0)
	{
		if (file.Length() == 0)
		{
			const char *spc = ProjectItem::GetFileSpec(pi->GetType());
			const char *ext = ProjectItem::GetFileExt(pi->GetType());
			if (!prjFrame->BrowseFile(0, file, spc, ext))
				return -1;
		}
	}

	int ret = 0;
	bsString text;
	int siz = GetText(text);

	HANDLE fh = CreateFile(file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fh != INVALID_HANDLE_VALUE)
	{
		if (siz > 0)
		{
			DWORD nwrit = 0;
			WriteFile(fh, (LPVOID) (const char *)text, siz, &nwrit, NULL);
			if (siz != nwrit)
				ret = -2;
		}
		CloseHandle(fh);
	}
	else
		ret = -1;

	edwnd.CallScintilla(SCI_SETSAVEPOINT);

	return ret;
}

int TextEditorWin::GetText(bsString& text)
{
	char *t = NULL;
	int siz = edwnd.Length();
	t = new char[siz+1];
	if (t != NULL)
	{
		if (siz > 0)
			edwnd.GetText(t, siz+1);
		else
			*t = 0;
	}
	text.Attach(t);
	return siz;
}

int TextEditorWin::SetText(bsString& text)
{
	edwnd.SetText(text);
	return 0;
}

int TextEditorWin::Find(int flags, const char *text)
{
	findFlags = flags;
	findText = text;
	int sciFlags = edwnd.SetSearchFlags(flags);
	int wrap = 0;
	int start = edwnd.CurrentPos();
	int found = edwnd.Find(text, start);
	if (found == -1 && start > 0)
	{
		found = edwnd.Find(text, 0, start);
		wrap = 1;
	}
	if (found >= 0)
		return wrap;
	return -1;
}

int TextEditorWin::MatchSel(int flags, const char *text)
{
	edwnd.SetSearchFlags(flags);
	return edwnd.Match(text);
}

void TextEditorWin::Replace(const char *rtext)
{
	edwnd.ReplaceSel(rtext);
}

int TextEditorWin::ReplaceAll(int flags, const char *ftext, const char *rtext, SelectInfo& info)
{
	findFlags = flags;
	findText = ftext;
	int count = 0;
	int flen = (int)strlen(ftext);
	int rlen = (int)strlen(rtext);
	edwnd.SetSearchFlags(flags);
	edwnd.CallScintilla(SCI_BEGINUNDOACTION, 0, 0);
	SelectInfo si;
	si.startPos = info.startPos;
	si.endPos = info.endPos;
	if (si.startPos == si.endPos)
	{
		si.endPos = edwnd.CallScintilla(SCI_GETLENGTH, 0, 0);
		si.endLn = edwnd.CallScintilla(SCI_LINEFROMPOSITION, si.endPos, 0)+1;
		si.endCh = 0;
	}
	else
	{
		si.endLn = info.endLn;
		si.endCh = info.endCh;
	}

	do
	{
		edwnd.CallScintilla(SCI_SETTARGETSTART, si.startPos, 0);
		edwnd.CallScintilla(SCI_SETTARGETEND, si.endPos, 0);
		if (edwnd.CallScintilla(SCI_SEARCHINTARGET, flen, (LPARAM) ftext) == -1)
			break;
		count++;
		int msg = (flags & SCFIND_REGEXP) ? SCI_REPLACETARGETRE : SCI_REPLACETARGET;
		edwnd.CallScintilla(msg, rlen, (LPARAM) rtext);
		si.startPos = edwnd.CallScintilla(SCI_GETTARGETEND, 0, 0);
		si.endPos = edwnd.CallScintilla(SCI_POSITIONFROMLINE, si.endLn, 0) + si.endCh;
	} while (si.startPos <= si.endPos);
	edwnd.CallScintilla(SCI_ENDUNDOACTION);
	return count;
}


int TextEditorWin::GetSelection(SelectInfo& info)
{
	return edwnd.GetSelection(info);
}

void TextEditorWin::SetSelection(SelectInfo& info)
{
	edwnd.SetSelection(info);
}

/////////////////////////////////////////////////////////////////////////////////////

LRESULT TextEditorWin::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rc;
	GetClientRect(&rc);
	edwnd.m_hWnd = CreateWindowEx(0,
		"Scintilla","", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, rc.right - rc.left, rc.bottom - rc.top, m_hWnd, (HMENU)ID_EDIT_WND, _Module.GetModuleInstance(), NULL);
	if (edwnd.IsWindow())
	{
		edwnd.Init();
		if (pi)
		{
			if (pi->GetType() == PRJNODE_NOTEFILE)
				edwnd.SetupStyling();
		}
	}
	return 0;
}

LRESULT TextEditorWin::OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	edwnd.SetFocus();
	findDlg.SetEditor(this);
	return 0;
}

LRESULT TextEditorWin::OnShow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (findDlg.GetEditor() == this)
		findDlg.SetEditor(wParam ? this : 0);
	return 0;
}

LRESULT TextEditorWin::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rc;
	GetClientRect(&rc);
	edwnd.SetWindowPos(HWND_TOP, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE|SWP_NOZORDER);
	return 0;
}

LRESULT TextEditorWin::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(&ps);
//	RECT rcWnd;
//	GetClientRect(&rcWnd);
//	Graphics gr(dc);
//	SolidBrush br(bgColor);
//	gr.FillRectangle(&br, 0, 0, rcWnd.right+1, rcWnd.bottom+1);
	EndPaint(&ps);
	return 0;
}

LRESULT TextEditorWin::OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 1;
}

LRESULT TextEditorWin::OnEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TextChanged();
	return 0;
}

LRESULT TextEditorWin::OnEditUpdateUI(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	UpdateUI();
	return 0;
}

LRESULT TextEditorWin::OnEditRestyle(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	Scintilla::SCNotification *scn = (Scintilla::SCNotification *) pnmh;
	Restyle(scn->position);
	return 0;
}
