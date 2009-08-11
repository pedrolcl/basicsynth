#include "globinc.h"
#include "TextEditorFltk.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <io.h>

static void TextChangedCB(int pos, int nInserted, int nDeleted, int nRestyled, 
						  const char* deletedText, void* cbArg)
{
	((TextEditorFltk*)cbArg)->TextChanged(pos, nInserted, nDeleted, nRestyled);
}


TextEditorFltk::TextEditorFltk(int X, int Y, int W, int H) : Fl_Text_Editor(X, Y, W, H, 0)
{
	findFlags = 0;
	pi = 0;
	Fl_Text_Buffer *buf = new Fl_Text_Buffer;
	buffer(buf);
	buf->add_modify_callback(TextChangedCB, (void*)this);
}

TextEditorFltk::~TextEditorFltk()
{
}

void TextEditorFltk::TextChanged(int pos, int inserted, int deleted, int restyled)
{
	if (inserted || deleted)
	{
		changed = 1;
		//Restyle(pos);
	}
}

void TextEditorFltk::UpdateUI()
{
}


ProjectItem *TextEditorFltk::GetItem()
{
	return pi;
}

void TextEditorFltk::SetItem(ProjectItem *p)
{
	pi = p;
	pi->SetEditor(this);
}

void TextEditorFltk::Undo()
{
	buffer()->undo();
}

void TextEditorFltk::Redo()
{

}

void TextEditorFltk::Cut()
{
	kf_cut(0, this);
}

void TextEditorFltk::Copy()
{
	kf_copy(0, this);
}

void TextEditorFltk::Paste()
{
	kf_paste(0, this);
}

void TextEditorFltk::Find()
{
	// open the find/replace dialog
}

void TextEditorFltk::FindNext()
{
}

void TextEditorFltk::SelectAll()
{
	Fl_Text_Buffer *buf = buffer();
	buf->select(0, buf->length());
}

void TextEditorFltk::GotoLine(int ln)
{
	if (ln < 0)
	{
		char value[40];
		value[0] = 0;
		if (!prjFrame->QueryValue("Line number:", value, 40))
			return;
		ln = atoi(value);
	}
	Fl_Text_Buffer *buf = buffer();
	buf->skip_lines(0, ln);
}

void TextEditorFltk::GotoPosition(int pos)
{
	Fl_Text_Buffer *buf = buffer();
	buf->skip_displayed_characters(0, pos);
}

void TextEditorFltk::Cancel()
{
}

long TextEditorFltk::EditState()
{
	Fl_Text_Buffer *buf = buffer();
	return 0;
}

int TextEditorFltk::IsChanged()
{
	return changed;
}

int TextEditorFltk::IsKeyword(char *txt)
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

// TODO: read the line into a buffer, then set the styles
void TextEditorFltk::Restyle(int position)
{
}

int TextEditorFltk::OpenFile(const char *fname)
{
	if (pi == NULL)
		return -1;
	if (!theProject->FullPath(fname))
		theProject->FindOnPath(file, fname);
	else
		file = fname;

	Fl_Text_Buffer *buf = buffer();
	if (buf)
		buf->loadfile(file);
	changed = 0;

	return 0;
}

int TextEditorFltk::SaveFile(const char *fname)
{
	if (pi == NULL)
		return -1;

	if (fname == 0 || *fname == 0)
	{
		if (file.Length() == 0)
		{
			const char *spc = ProjectItem::GetFileSpec(pi->GetType());
			if (!prjFrame->BrowseFile(0, file, spc, 0))
				return -1;
		}
	}
	else
		file = fname;

	Fl_Text_Buffer *buf = buffer();
	if (buf != 0)
	{
		if (buf->outputfile(file, 0, buf->length()))
		{
			prjFrame->Alert("Error saving file.", "Ooops...");
			return -1;
		}
		changed = 0;
		return 0;
	}
	return -1;
}

int TextEditorFltk::GetText(bsString& text)
{
	Fl_Text_Buffer *buf = buffer();
	if (buf != 0)
		text.Attach(buf->text());
	else
		text = "";
	return text.Length();
}

int TextEditorFltk::SetText(bsString& text)
{
	Fl_Text_Buffer *buf = buffer();
	if (buf)
		buf->text(text);
	return 0;
}

int TextEditorFltk::Find(int flags, const char *text)
{
	Fl_Text_Buffer *buf = buffer();
	int wrap = 0;
	int curPos = insert_position();
	int foundPos = 0;
	int found = buf->search_forward(curPos, text, &foundPos, flags & TXTFIND_MATCHCASE);
	if (!found && curPos > 0)
	{
		found = buf->search_forward(0, text, &foundPos, flags & TXTFIND_MATCHCASE);
		wrap = 1;
	}
	if (found)
	{
		buf->select(foundPos, foundPos+strlen(text));
		return wrap;
	}
	return -1;
}

int TextEditorFltk::MatchSel(int flags, const char *ftext)
{
	Fl_Text_Buffer *buf = buffer();
	int startSel;
	int endSel;
	int r = buf->selection_position(&startSel, &endSel);
	if (startSel == endSel)
		return 0;
	int foundPos = 0;
	if (buf->search_forward(startSel, ftext, &foundPos, flags & TXTFIND_MATCHCASE))
	{
		if (foundPos == startSel)
			return 1;
	}
	return 0;
}

void TextEditorFltk::Replace(const char *rtext)
{
	Fl_Text_Buffer *buf = buffer();
	buf->replace_selection(rtext);
}

int TextEditorFltk::ReplaceAll(int flags, const char *ftext, const char *rtext, SelectInfo& info)
{
	Fl_Text_Buffer *buf = buffer();

	int match = flags & TXTFIND_MATCHCASE;
	int flen = strlen(ftext);
	int count = 0;
	int curPos = info.startPos;
	int foundPos = 0;

	while (buf->search_forward(curPos, ftext, &foundPos, match))
	{
		if (foundPos > info.endPos)
			break;
		count++;
		buf->select(foundPos, foundPos+flen);
		buf->remove_selection();
		buf->insert(foundPos, rtext);
		curPos = foundPos + flen;
	}

	return count;
}


int TextEditorFltk::GetSelection(SelectInfo& info)
{
	Fl_Text_Buffer *buf = buffer();
	if (buf == 0)
		return 0;
	int r = buf->selection_position(&info.startPos, &info.endPos);
	if (r)
	{
		info.startLn = buf->line_start(info.startPos);
		info.endLn = buf->line_start(info.endPos);
	}
	return r;
}

void TextEditorFltk::SetSelection(SelectInfo& info)
{
	Fl_Text_Buffer *buf = buffer();
	if (buf)
		buf->select(info.startPos, info.endPos);
}

void TextEditorFltk::Resize(wdgRect& r)
{
}

