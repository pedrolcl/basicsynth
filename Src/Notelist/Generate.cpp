//////////////////////////////////////////////////////////////////////
// Notelist Generator and Script node classes
//
// These classes do the work of generating sequencer events from the
// tokenized script. Each script node has an Exec method to do any
// work. Executing the script is thus a matter of walking the list
// of tokens, calling Exec on each. The bulk of the work is done
// by nlNoteNode and nlExprNode. Most other nodes merely set
// values on the current voice, or invoke other nodes to do the
// work.
//
// Evaluation of expressions is done with a stack based calculator.
// The stack is contained within the generator object.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <BasicSynth.h>
#include "NLConvert.h"

//////////////////////////////////////////////////////////////////////
// Generator
//////////////////////////////////////////////////////////////////////

// The stack size limits the depth of parenthesis, not the
// length of the expression. If you need more than 20 levels
// of stack, you need to think more about what you are doing...
#define GENSTACKSIZE 20

nlGenerate::nlGenerate()
{
	mainSeq = new nlSequence;
	curSeq = mainSeq;
	
	InitStack();

	for (int i = 0; i < MAXFGEN; i++)
		iFnGen[i] = NULL;

	freqmode = 0;
	voldbmode = 0;
	maxParam = MAXPARAM;
	curVoice = NULL;
	voiceList = NULL;
	beat = 4;
	secBeat = 1.0;
	nlVersion = NOTELIST_VERSION;
	synclist = NULL;
}

nlGenerate::~nlGenerate()
{
	for (int i = 0; i < MAXFGEN; i++)
	{
		if (iFnGen[i] != NULL)
			delete iFnGen[i];
	}

	delete[] vStack;

	Clear();
}

void nlGenerate::Clear()
{
	nlSyncMark *smp;
	while ((smp = synclist) != NULL)
	{
		synclist = smp->next;
		delete smp;
	}
	nlVoice *vp;
	while ((vp = voiceList) != NULL)
	{
		voiceList = vp->next;
		delete vp;
	}
	delete mainSeq;
	mainSeq = NULL;
	curSeq = NULL;
}

void nlGenerate::Reset()
{
	Clear();
	mainSeq = new nlSequence;
	curSeq = mainSeq;
	curVoice = NULL;
	beat = 4;
	secBeat = 1.0;
	spCur = &vStack[0];
	spEnd = &vStack[GENSTACKSIZE-1];
}

nlVoice *nlGenerate::SetCurVoice(int n)
{
	nlVoice *vp = voiceList;
	while (vp)
	{
		if (vp->voiceNum == n)
			break;
		vp = vp->next;
	}
	if (vp == NULL)
	{
		vp = new nlVoice;
		vp->voiceNum = n;
		vp->genPtr = this;
		vp->SetMaxParam(maxParam);
		vp->next = voiceList;
		if (nlVersion < 1.0)
			vp->articParam = 1;
		voiceList = vp;
	}
	curVoice = vp;
	cvtPtr->BeginVoice(curVoice);
	return curVoice;
}

// main entry point for the generator
int nlGenerate::Run()
{
	cvtPtr->BeginNotelist();
	mainSeq->Play();
	cvtPtr->EndNotelist();

	return 0;
}

void nlGenerate::InitStack()
{
	vStack = new nlVarValue[GENSTACKSIZE];
	spCur = &vStack[0];
	spEnd = &vStack[GENSTACKSIZE-1];
}

void nlGenerate::PushStack(long n)
{
	spCur->SetValue(n);
	if (spCur < spEnd)
		spCur++;
}

void nlGenerate::PushStack(double d)
{
	spCur->SetValue(d);
	if (spCur < spEnd)
		spCur++;
}

void nlGenerate::PushStack(const char *s)
{
	spCur->SetValue(s);
	if (spCur < spEnd)
		spCur++;
}

void nlGenerate::PushStack(nlVarValue *v)
{
	spCur->SetValue(v);
	if (spCur < spEnd)
		spCur++;
}

void nlGenerate::PopStack(long *n)
{
	if (spCur > vStack)
		spCur--;
	spCur->GetValue(n);
}

void nlGenerate::PopStack(double *d)
{
	if (spCur > vStack)
		spCur--;
	spCur->GetValue(d);
}

void nlGenerate::PopStack(nlVarValue *v)
{
	if (spCur > vStack)
		spCur--;
	spCur->CopyValue(v);
}

void nlGenerate::PopStack(char **s)
{
	if (spCur > vStack)
		spCur--;
	spCur->GetValue(s);
}

nlSequence *nlGenerate::AddSequence(const char *id)
{
	nlSequence *pseq = new nlSequence(id);
	pseq->Append(&mainSeq);
	return pseq;
}

nlSequence *nlGenerate::AddSequence(int id)
{
	nlSequence *pseq = new nlSequence(id);
	pseq->Append(&mainSeq);
	return pseq;
}

nlSequence *nlGenerate::SetCurSeq(nlSequence *p)
{
	nlSequence *pold = curSeq;
	curSeq = p;
	return pold;
}

nlSequence *nlGenerate::FindSequence(nlVarValue *id)
{
	if (mainSeq)
		return mainSeq->FindSequence(id);
	return NULL;
}

// AddNode appends a new script node to the 
// current sequence. 
nlScriptNode *nlGenerate::AddNode(nlScriptNode *node)
{
	node->SetGen(this);
	return curSeq->AddNode(node);
}

nlScriptNode *nlGenerate::AddNode(int token, const char *text)
{
	nlScriptNode *p = new nlScriptNode;
	p->SetToken(token);
	p->SetValue(text);
	return AddNode(p);
}

nlScriptNode *nlGenerate::AddNode(int token, long val)
{
	nlScriptNode *p = new nlScriptNode;
	p->SetToken(token);
	p->SetValue(val);
	return AddNode(p);
}

nlScriptNode *nlGenerate::AddNode(int token, short v1, short v2)
{
	nlScriptNode *p = new nlScriptNode;
	p->SetToken(token);
	p->SetValue((long) v1 << 16 | (long) v2);
	return AddNode(p);
}

nlScriptNode *nlGenerate::AddNode(int token, double val)
{
	nlScriptNode *p = new nlScriptNode;
	p->SetToken(token);
	p->SetValue(val);
	return AddNode(p);
}

///////////////////////////////////////////////////////////
// Move the current time for this voice to the point
// set by the associated MARK statement. All MARK points
// are stored in a linked list.
///////////////////////////////////////////////////////////
void nlGenerate::SyncTo(nlVarValue *v)
{
	nlSyncMark *p = synclist;
	while (p != NULL)
	{
		if (p->id.Compare(v) == 0)
		{
			curVoice->curTime = p->t;
			break;
		}
		p = p->next;
	}
}

///////////////////////////////////////////////////////////
// Save the current time for this voice and associate
// it with the ID value. Create a new SYNC point if needed,
// or just update if the ID already exists.
///////////////////////////////////////////////////////////
void nlGenerate::MarkTo(nlVarValue *v)
{
	nlSyncMark *p = synclist;
	while (p != NULL)
	{
		if (p->id.Compare(v) == 0)
		{
			p->t = curVoice->curTime;
			return;
		}
		p = p->next;
	}

	p = new nlSyncMark;
	p->t = curVoice->curTime;
	v->CopyValue(&p->id);
	p->next = synclist;
	synclist = p;
}

///////////////////////////////////////////////////////////
// Sequence - a sequence is a linked list of tokenized 
// script. It can represent the main statement list, a
// named sequence, or a branch from another node in the
// main sequence (i.e. LOOP, IF...THEN or WHILE).
///////////////////////////////////////////////////////////
nlSequence::nlSequence(const char *n)
{
	if (n)
		id.SetValue(n);
	Init();
}

nlSequence::nlSequence(int n)
{
	id.SetValue((long)n);
	Init();
}

nlSequence::nlSequence()
{
	Init();
}

void nlSequence::Init()
{
	head = new nlScriptNode;
	head->SetToken(T_START);
	tail = head;
	next = NULL;
}

nlSequence::~nlSequence()
{
	nlScriptNode *node;
	while ((node = head) != NULL)
	{
		head = head->GetNext();
		delete node;
	}
}

///////////////////////////////////////////////////////////
// Play a sequence is what actually generates the events.
// All this has to do is Exec() each node in turn.
///////////////////////////////////////////////////////////
void nlSequence::Play()
{
	nlScriptNode *node = head->GetNext();
	while (node != NULL)
		node = node->Exec();
}

nlScriptNode *nlSequence::AddNode(nlScriptNode *node)
{
	node->Append(tail);
	tail = node;
	return node;
}

void nlSequence::Append(nlSequence **seq)
{
	while (*seq != NULL)
		seq = &(*seq)->next;
	*seq = this;
}

nlSequence *nlSequence::FindSequence(nlVarValue *find)
{
	if (id.Compare(find) == 0)
		return this;
	if (next != NULL)
		return next->FindSequence(find);
	return NULL;
}

///////////////////////////////////////////////////////////
// Voice - a voice holds a set of parameters for generating
// a series of notes. Each voice has its own current time,
// volume, instrument, channel, transposition, etc., along
// with the last values for pitch, rhythm, volume and params.
///////////////////////////////////////////////////////////

nlVoice::nlVoice()
{
	lastPit = 48;
	curTime = 0.0;
	lastDur = 1;
	lastVol = 100.0;
	lastArtic = 0;
	volMul = 1.0;
	maxParam = 0;
	cntParam = 0;
	lastParam = 0;
	paramVal = 0;
	params = 0;
	instr = 1;
	chnl = 0;
	articType = T_OFF;
	articParam = 0;
	transpose = 0;
	doublex = 0;
	doublev = 0.0;
	instname = NULL;
	loopCount = 0;
}

nlVoice::~nlVoice()
{
	if (instname)
		delete instname;
	if (lastParam)
		delete[] lastParam;
	if (paramVal)
		delete[] paramVal;
	if (params)
		delete[] params;
}

///////////////////////////////////////////////////////////
// Var(iant) Value - base class for anything that needs
// to store a value. The value can be double, int, or char*
// Polymorphic methods provide automatic type conversion.
// Simply call GetValue() or SetValue() with a value.
///////////////////////////////////////////////////////////

void nlVarValue::ClearValue()
{
	if (vt == vtText)
		delete txtVal;
	vt = vtNull;
}

void nlVarValue::SetValue(const char *p)
{
	ClearValue();
	vt = vtText;
	txtVal = StrMakeCopy(p);
}

void nlVarValue::SetValue(long n)
{
	ClearValue();
	vt = vtNum;
	lVal = n;
}

void nlVarValue::SetValue(double d)
{
	ClearValue();
	vt = vtReal;
	dblVal = d;
}

void nlVarValue::SetValue(nlVarValue *v)
{
	v->CopyValue(this);
}

void nlVarValue::GetValue(char **p)
{
	if (vt != vtText)
		ChangeType(vtText);

	*p = StrMakeCopy(txtVal);
}

void nlVarValue::GetValue(long *n)
{
	if (vt != vtNum)
		ChangeType(vtNum);

	*n = lVal;
}

void nlVarValue::GetValue(double *d)
{
	if (vt != vtReal)
		ChangeType(vtReal);

	*d = dblVal;
}

void nlVarValue::ChangeType(vType vnew)
{
	long lTmp;
	double dTmp;
	char *txtTmp;

	switch (vt)
	{
	case vtNull:
		switch (vnew)
		{
		case vtNull:
			return;
		case vtText:
			txtVal = new char[1];
			txtVal[0] = '\0';
			break;
		case vtNum:
			lVal = 0;
			break;
		case vtReal:
			dblVal = 0.0;
			break;
		}
		break;
	case vtText:
		switch (vnew)
		{
		case vtNull:
			delete txtVal;
			break;
		case vtText:
			break;
		case vtNum:
			lTmp = atoi(txtVal);
			delete txtVal;
			lVal = lTmp;
			break;
		case vtReal:
			dTmp = atof(txtVal);
			delete txtVal;
			dblVal = dTmp;
			break;
		}
		break;
	case vtNum:
		switch (vnew)
		{
		case vtNull:
			break;
		case vtText:
			txtTmp = new char[65];
			IntToStr(lVal, txtTmp);
			txtVal = txtTmp;
			break;
		case vtNum:
			break;
		case vtReal:
			dTmp = (double) lVal;
			dblVal = dTmp;
			break;
		}
		break;
	case vtReal:
		switch (vnew)
		{
		case vtNull:
			break;
		case vtText:
			txtTmp = new char[80];
			FltToStr(dblVal, txtTmp, 80);
			//ecvt(dblVal, 10, &iTmp, &iTmp);
			txtVal = txtTmp;
			break;
		case vtNum:
			lTmp = (long) dblVal;
			lVal = lTmp;
			break;
		case vtReal:
			break;
		}
		break;
	}
	vt = vnew;
}

void nlVarValue::CopyValue(nlVarValue *p)
{
	if (p == NULL)
		return;
	p->ClearValue();
	switch (vt)
	{
	case vtText:
		p->txtVal = StrMakeCopy(txtVal);
		break;
	case vtNum:
		p->lVal = lVal;
		break;
	case vtReal:
		p->dblVal = dblVal;
		break;
	}
	p->vt = vt;
}

int nlVarValue::Compare(nlVarValue *p)
{
	switch (vt)
	{
	case vtNull:
		if (p->vt == vtNull)
			return 0;
		return -1;
	case vtText:
		switch (p->vt)
		{
		case vtNull:
			return 1;
		case vtText:
			return strcmp(txtVal, p->txtVal);
		case vtNum:
			return (int) (atol(txtVal) - p->lVal);
		case vtReal:
			return (int) (atof(txtVal) - p->dblVal);
		}
		break;
	case vtNum:
		switch (p->vt)
		{
		case vtNull:
			return 1;
		case vtText:
			return (int) (lVal - atol(p->txtVal));
		case vtNum:
			return (int) (lVal - p->lVal);
		case vtReal:
			return (int) ((double)lVal - p->dblVal);
		}
		break;
	case vtReal:
		switch (p->vt)
		{
		case vtNull:
			return 1;
		case vtText:
			return (int) (dblVal - atof(p->txtVal));
		case vtNum:
			return (int) (dblVal - (double) p->lVal);
		case vtReal:
			return (int) (dblVal - p->dblVal);
		}
		break;
	}

	return -1; //??
}

///////////////////////////////////////////////////////////
// VOICE expr notelsit
///////////////////////////////////////////////////////////

nlScriptNode *nlVoiceNode::Exec()
{
	if (next == NULL)
		return NULL;

	// get the voice number and switch voices
	nlScriptNode *notes = next->Exec();

	long vnum = 0;
	next->GetValue(&vnum);
	genPtr->SetCurVoice(vnum);

	if (notes != NULL)
		return notes->Exec();

	// OOOPS
	return NULL;
}

///////////////////////////////////////////////////////////
// INST expr - select the instrument for this voice
///////////////////////////////////////////////////////////

nlScriptNode *nlInstnumNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlVoice *pv = genPtr->GetCurVoice();
	if (pv == NULL)
		return NULL;
	nlConverter *cvtPtr = genPtr->GetConverter();
	if (cvtPtr == NULL)
		return NULL;

	nlScriptNode *ret = next->Exec();
	if (next->vt == vtText)
		pv->instr = (long) cvtPtr->FindInstrNum(next->txtVal);
	else
		next->GetValue(&pv->instr);
	cvtPtr->BeginInstr();

	return ret;
}

///////////////////////////////////////////////////////////
// CHANNEL n - set the mixer input channel for this voice
///////////////////////////////////////////////////////////
nlScriptNode *nlChnlNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlVoice *vox = genPtr->GetCurVoice();
	if (vox == NULL)
		return NULL;

	nlScriptNode *ret = next->Exec();
	next->GetValue(&vox->chnl);
	return ret;
}

///////////////////////////////////////////////////////////
// BEGIN notelist END
///////////////////////////////////////////////////////////

nlScriptNode *nlBlockNode::Exec()
{
	nlScriptNode *note = next;
	while (note != NULL)
	{
		if (note->GetToken() == T_END)
			return note->GetNext();
		note = note->Exec();
	}
	return NULL;
}

///////////////////////////////////////////////////////////
// TEMPO n,t - set the "tempo", values that control how
// rhythm values are calculated.
///////////////////////////////////////////////////////////

nlScriptNode *nlTempoNode::Exec()
{
	if (next == NULL)
		return NULL;

	double b, t;

	nlScriptNode *rate;
	if (next->GetToken() == T_DUR)
	{
		// hack a little. we don't want the duration
		// node to calculate actual time value.
		((nlDurNode*)next)->GetBeatValue(&b);
		rate = next->GetNext();
	}
	else
	{
		rate = next->Exec();
		next->GetValue(&b);
	}

	if (rate == NULL)
		return NULL;

	nlScriptNode *ret = rate->Exec();
	rate->GetValue(&t);
	genPtr->SetTempo(b, 60.0 / t);
	
	return ret;
}

///////////////////////////////////////////////////////////
// TIME n - set current voice time
///////////////////////////////////////////////////////////

nlScriptNode *nlTimeNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *ret = next->Exec();
	nlVoice *vox = genPtr->GetCurVoice();
	if (vox)
		next->GetValue(&vox->curTime);
	return ret;
}

///////////////////////////////////////////////////////////
// MARK "id" - save the current time 
///////////////////////////////////////////////////////////

nlScriptNode *nlMarkNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *ret = next->Exec();
	genPtr->MarkTo(next);
	return ret;
}

///////////////////////////////////////////////////////////
// SYNC "id" - set the current time
///////////////////////////////////////////////////////////

nlScriptNode *nlSyncNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *ret = next->Exec();
	genPtr->SyncTo(next);
	return ret;
}

///////////////////////////////////////////////////////////
// VOL level - set the volume multiplier for this voice
///////////////////////////////////////////////////////////

nlScriptNode *nlVolumeNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *ret = next->Exec();
	nlVoice *vox = genPtr->GetCurVoice();
	if (vox)
	{
		double d = 0;
		next->GetValue(&d);
		if (genPtr->GetVersion() >= 1.0)
			vox->volMul = d / 100.0;
		else
			vox->volMul = d;
	}
	return ret;
}

///////////////////////////////////////////////////////////
// TRANSPOSE amnt - set a transpostion amount for this voice.
// The transposition is in semi-tones and is chromatic. The
// value is added to the pitch just before the note event
// is produced.
///////////////////////////////////////////////////////////

nlScriptNode *nlTransposeNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *ret = next->Exec();
	nlVoice *vox = genPtr->GetCurVoice();
	if (vox)
	{
		next->GetValue(&vox->transpose);
		if (genPtr->GetFrequencyMode())
			vox->transpose = (int) pow(2.0, (double)vox->transpose / 12.0);
	}
	return ret;
}

///////////////////////////////////////////////////////////
// DOUBLE OFF | transpose [, volume]
// Doubling a note creates two events for each note.
// This is handled in the NoteNode; here we just store
// the parameters in the current voice object.
///////////////////////////////////////////////////////////

nlScriptNode *nlDoubleNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlVoice *vox = genPtr->GetCurVoice();
	if (next->GetToken() == T_OFF)
	{
		vox->doublex = 0;
		vox->doublev = 0;
		return next->GetNext();
	}
	nlScriptNode *vol = next->Exec();
	if (vox)
		next->GetValue(&vox->doublex);
	if (vol->GetToken() != T_COMMA)
	{
		vox->doublev = vox->volMul;
		return vol;
	}

	vol = vol->GetNext();
	nlScriptNode *ret = vol->Exec();
	if (vox)
	{
		double val;
		vol->GetValue(&val);
		vox->doublev = val / 100.0;
	}
	return ret;
}

///////////////////////////////////////////////////////////
// IF cond notelist ELSE notelist
///////////////////////////////////////////////////////////
nlIfNode::~nlIfNode()
{
	delete ifPart;
	delete elPart;
}

nlScriptNode *nlIfNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *ret = next->Exec();

	long cond = 0;
	next->GetValue(&cond);
	if (cond)
		ifPart->Play();
	else
		elPart->Play();

	return ret;
}

///////////////////////////////////////////////////////////
// WHILE cond notelist
///////////////////////////////////////////////////////////
nlWhileNode::~nlWhileNode()
{
	delete wseq;
}

nlScriptNode *nlWhileNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *ret = next->Exec();
	if (wseq)
	{
		long cond = 0;
		next->GetValue(&cond);
		while (cond)
		{
			wseq->Play();
			next->Exec();
			next->GetValue(&cond);
		}
	}

	return ret;
}

///////////////////////////////////////////////////////////
// LOOP notelist
///////////////////////////////////////////////////////////

nlLoopNode::~nlLoopNode()
{
	delete lseq;
}

nlScriptNode *nlLoopNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlVoice *vox = genPtr->GetCurVoice();

	nlScriptNode *ret = next->Exec();
	if (lseq)
	{
		long count = 0;
		next->GetValue(&count);

		long savCount = 0;
		if (vox)
			savCount = vox->loopCount;

		for (long n = 0; n < count; n++)
		{
			if (vox)
				vox->loopCount = n;
			lseq->Play();
		}
		if (vox)
			vox->loopCount = savCount;
	}
	return next;
}

///////////////////////////////////////////////////////////
// duration (%n) - calculate the actual time value for
// a rhythm based on the current tempo.
///////////////////////////////////////////////////////////

void nlDurNode::GetValue(long *n)
{
	double d;
	GetValue(&d);
	*n = (long) d;
}

void nlDurNode::GetValue(double *d)
{
	if (vt != vtReal)
		ChangeType(vtReal);
	if (dblVal == 0.0)
		*d = 0.0;
	else
	{
		*d = genPtr->ConvertRhythm(dblVal);
		if (isDotted)
			*d += (*d / 2);
	}
}

void nlDurNode::CopyValue(nlVarValue *p)
{
	p->vt = vtReal;
	GetValue(&p->dblVal);
}

///////////////////////////////////////////////////////////
// Variable Reference - a little wasteful, but we just
// copy the current value into the var node so that the
// expression evaluator can treat this just like a constant.
// In other words, an indirect reference...
///////////////////////////////////////////////////////////

nlScriptNode *nlVarNode::Exec()
{
	if (symb)
		symb->CopyValue(this);
	return next;
}

///////////////////////////////////////////////////////////
// Variable Assignment
///////////////////////////////////////////////////////////
nlScriptNode *nlSetNode::Exec()
{
	if (next == NULL)
		return NULL;
	nlScriptNode *ret = next->Exec();
	if (symb)
		next->CopyValue(symb);
	return ret;
}

///////////////////////////////////////////////////////////
// Expression - stack based calculator. The parser has
// converted expressions to postfix notation (RPN). 
// Operands are pushed on the stack.
// Operators pop the appropriate values from the stack,
// perform the op, then push the result back on the stack.
// The final result is popped from the stack
// and becomes the value of the ExprNode object. 
// Evaluation stops when a script node is encountered
// that is not an operator or operand. This can in theory
// be a problem if two expressions were to appear in series
// in the script. Currently this is avoided by inserting
// things like comma nodes into the node list. :)
// There is a slight bug here - if the ExprNode is the
// last thing in a sequence, it never pops the value...
// That's not a problem if the sequence is correctly
// produced since there should be a EOF as the last node.
///////////////////////////////////////////////////////////

nlScriptNode *nlExprNode::Exec()
{
	double d1;
	double d2;
	long fno;
	long val1;
	long val2;
	nlFunctionData *pfn;
	char *pstr1;
	char *pstr2;
	char *pstr3;
	nlScriptEngine *eng;

	nlVoice *vox = genPtr->GetCurVoice();

	nlScriptNode *p = next;
	while (p != NULL)
	{
		switch (p->GetToken())
		{
		case T_NUM:
			genPtr->PushStack(p);
			break;
		case T_PIT:
			genPtr->PushStack(p);
			break;
		case T_DUR:
			p->Exec();
			genPtr->PushStack(p);
			break;
		case T_STRLIT:
			genPtr->PushStack(p);
			break;
		case T_VAR:
			p->Exec();
			genPtr->PushStack(p);
			break;
		case T_ADDOP:
			genPtr->PopStack(&d2);
			genPtr->PopStack(&d1);
			genPtr->PushStack(d1 + d2);
			break;
		case T_SUBOP:
			genPtr->PopStack(&d2);
			genPtr->PopStack(&d1);
			genPtr->PushStack(d1 - d2);
			break;
		case T_MULOP:
			genPtr->PopStack(&d2);
			genPtr->PopStack(&d1);
			genPtr->PushStack(d1 * d2);
			break;
		case T_DIVOP:
			genPtr->PopStack(&d2);
			genPtr->PopStack(&d1);
			if (d2 == 0.0)
			{
				if (d1 == 0.0)
					genPtr->PushStack(1L); // hmmmm
				else
					genPtr->PushStack(0L);
			}
			else
				genPtr->PushStack(d1 / d2);
			break;
		case T_EXPOP:
			genPtr->PopStack(&d2);
			genPtr->PopStack(&d1);
			genPtr->PushStack(pow(d1, d2));
			break;
		case T_CATOP:
			genPtr->PopStack(&pstr1);
			genPtr->PopStack(&pstr2);
			pstr3 = StrPaste(pstr2, pstr1);
			genPtr->PushStack(pstr3);
			delete pstr1;
			delete pstr2;
			delete pstr3;
			break;
		case T_GTOP:
			genPtr->PopStack(&d2);
			genPtr->PopStack(&d1);
			genPtr->PushStack((long) (d1 > d2));
			break;
		case T_LTOP:
			genPtr->PopStack(&d2);
			genPtr->PopStack(&d1);
			genPtr->PushStack((long)(d1 < d2));
			break;
		case T_GEOP:
			genPtr->PopStack(&d2);
			genPtr->PopStack(&d1);
			genPtr->PushStack((long) (d1 >= d2));
			break;
		case T_LEOP:
			genPtr->PopStack(&d2);
			genPtr->PopStack(&d1);
			genPtr->PushStack((long)(d1 <= d2));
			break;
		case T_EQOP:
			genPtr->PopStack(&d2);
			genPtr->PopStack(&d1);
			genPtr->PushStack((long)(d1 == d2));
			break;
		case T_NEOP:
			genPtr->PopStack(&d2);
			genPtr->PopStack(&d1);
			genPtr->PushStack((long)(d1 != d2));
			break;
		case T_NEG:
			genPtr->PopStack(&d1);
			genPtr->PushStack(-d1);
			break;
		case T_NOT:
			genPtr->PopStack(&val1);
			genPtr->PushStack((long)!val1);
			break;
		case T_AND:
			genPtr->PopStack(&val1);
			genPtr->PopStack(&val2);
			genPtr->PushStack((long)(val1 && val2));
			break;
		case T_OR:
			genPtr->PopStack(&val1);
			genPtr->PopStack(&val2);
			genPtr->PushStack((long)(val1 || val2));
			break;
		case T_FGEN:
			d1 = 0.0;
			genPtr->PopStack(&d2);
			genPtr->PopStack(&fno);
			if (fno >= 0 && fno < MAXFGEN)
			{
				pfn = genPtr->iFnGen[fno];
				if (pfn != NULL)
					d1 = pfn->Iterate(d2);
			}
			genPtr->PushStack(d1);
			break;
		case T_RAND:
			genPtr->PopStack(&d1);
			genPtr->PopStack(&d2);
			genPtr->PushStack((((d1 - d2) * (double) rand()) / RAND_MAX) + d2);
			break;
		case T_COUNT:
			if (vox)
				genPtr->PushStack(vox->loopCount);
			else
				genPtr->PushStack(0L);
			break;
		case T_CURTIME:
			if (vox)
				genPtr->PushStack(vox->curTime);
			else
				genPtr->PushStack(0L);
			break;
		case T_CURVOL:
			if (vox)
				genPtr->PushStack(vox->lastVol);
			else
				genPtr->PushStack(0L);
			break;
		case T_CURPIT:
			if (vox)
				genPtr->PushStack(vox->lastPit);
			else
				genPtr->PushStack(0L);
			break;
		case T_CURDUR:
			if (vox)
				genPtr->PushStack(vox->lastDur);
			else
				genPtr->PushStack(0L);
			break;
		case T_EVAL:
			genPtr->PopStack(&pstr1);
			eng = genPtr->GetConverter()->GetScriptEngine();
			if (eng)
			{
				nlVarValue out;
				eng->EvalScript(pstr1, out);
				genPtr->PushStack(&out);
			}
			else
				genPtr->PushStack(0L);
			delete pstr1;
			break;
		default:
			// copy tos to value
			genPtr->PopStack(this);
			return p;
		}
		p = p->GetNext();
	}

	return NULL;
}

///////////////////////////////////////////////////////////
// PARAM n, value - set a specific parameter value. The
// values are stored in the current voice and used by
// the NoteNode object. This is convenient when we want
// to set a high-numbered parameter without modifying all
// other parameters. The automatic repeat of parameter values
// will insure things work right.
///////////////////////////////////////////////////////////

nlScriptNode *nlParamNode::Exec()
{
	if (next == NULL)
		return NULL;

	long pno = -1;
	nlScriptNode *pval = next->Exec();
	next->GetValue(&pno);
	nlScriptNode *pret = pval->Exec();
	nlVoice *pv = genPtr->GetCurVoice();
	if (pv && pno >= 0 && pno < pv->maxParam)
		pval->GetValue(&pv->lastParam[pno]);

	return pret;
}

///////////////////////////////////////////////////////////
// WRITE stuff - send text out to the console. This was
// originally intended to add text to a CSound score file
// that could not be easily defined in Notelist.
///////////////////////////////////////////////////////////

nlScriptNode *nlWriteNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *ret = next->Exec();
	char *str = 0;
	next->GetValue(&str);
	genPtr->GetConverter()->Write(str);
	delete str;
	return ret;
}

///////////////////////////////////////////////////////////
// PLAY sequence - play a named sequence
///////////////////////////////////////////////////////////
nlScriptNode *nlPlayNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *ret = next->Exec();
	nlSequence *seq = genPtr->FindSequence(next);
	if (seq != NULL)
		seq->Play();
	return ret;
}

///////////////////////////////////////////////////////////
// INIT fn type start, end, steps - initialize a built-in
// function generator. The expression evaluator calls the
// code to actually produce the series of values.
///////////////////////////////////////////////////////////

nlScriptNode *nlInitFnNode::Exec()
{
	if (next == NULL)
		return NULL;

	long fn;
	next->GetValue(&fn);
	if (fn < 0 || fn >= MAXFGEN)
		return NULL;
	nlFunctionData *pfn = genPtr->iFnGen[fn];
	if (pfn == NULL)
	{
		pfn = new nlFunctionData;
		genPtr->iFnGen[fn] = pfn;
	}


	nlScriptNode *start = next->GetNext();
	nlScriptNode *end = start->Exec();
	nlScriptNode *step = end->Exec();
	nlScriptNode *ret = step->Exec();

	double startVal;
	double endVal;
	double stepVal;

	start->GetValue(&startVal);
	end->GetValue(&endVal);
	step->GetValue(&stepVal);
	int type = next->GetToken();

	pfn->Init(startVal, endVal, stepVal, type);

	return ret;
}

///////////////////////////////////////////////////////////
// ARTIC type value - set the articulation. Articulation
// modifies the actual duration of a note. This allows
// for overlapping notes (legato) or spacing between notes
// (staccto) while using the rhythm values to control note
// start time alone. 
///////////////////////////////////////////////////////////

nlScriptNode *nlArticNode::Exec()
{
	nlVoice *pv = genPtr->GetCurVoice();
	GetValue(&pv->articType);

	if (genPtr->GetVersion() < 1.0)
		return next;

	nlScriptNode *pList = next;
	if (pList == NULL)
		return NULL;
	if (pv->articType != T_OFF)
	{
		int tok = pList->GetToken();
		if (tok == T_PARAM)
		{
			pv->articParam = 1;
			pList = pList->GetNext();
		}
		else
		{
			nlScriptNode *pListNext = pList->Exec();
			pList->GetValue(&pv->lastArtic);
			pv->articParam = 0;
			pList = pListNext;
		}
	}
	else
	{
		pv->articParam = 0;
		pv->lastArtic = 0;
	}
	return pList;
}

///////////////////////////////////////////////////////////
// MAP instr, id[=scale] - setup an instrument parameter map.
// The map allows selecting a sub-set of an instruments
// parameters. THis is especially useful when the number of
// parameters is large. It is also handy if you want to switch
// instruments without reentering all the parameters. A simple
// instrument can be used for quick test of the sequence, then
// changed to a more complex instrument for final sound output.
///////////////////////////////////////////////////////////
nlScriptNode *nlMapNode::Exec()
{
	nlScriptNode *pList = next;
	if (pList == NULL)
		return NULL;

	if (genPtr == NULL)
		return NULL;

	nlConverter *cvtPtr = genPtr->GetConverter();
	if (cvtPtr == NULL)
		return NULL;

	nlScriptNode *pListNext;
	long tmp;
	int pn = 0;
	int inum = -1;
	double scale;

	int tok = pList->GetToken();
	if (tok == T_STRLIT)
	{
		// look-up instrument by name
		char *name = 0;
		pList->GetValue(&name);
		inum = cvtPtr->FindInstrNum(name);
		delete name;
	}
	else if (tok == T_NUM)
	{
		// look-up instrument by number
		pList->GetValue(&tmp);
		inum = (int) tmp;
	}
	else
		return NULL;

	cvtPtr->InitParamMap(inum);

	pList = pList->GetNext();
	while (pList != NULL)
	{
		pListNext = pList->Exec();
		if (pList->GetType() == vtText)
			tmp = (long) cvtPtr->GetParamID(inum, pList->RefStr());
		else
			pList->GetValue(&tmp);
		pList = pListNext;
		if (pList->GetToken() == T_COL)
		{
			pListNext = pList->GetNext();
			pList = pListNext->Exec();
			pListNext->GetValue(&scale);
		}
		else
			scale = 1.0;
		cvtPtr->SetParamMap(inum, pn++, (int)tmp, scale);
		if (pList->GetToken() == T_COMMA)
			pList = pList->GetNext();
		else
			break;
	}

	return pList;
}

///////////////////////////////////////////////////////////
// MAXPARAM n - set the maximum number of parameters stored
// with a note.
///////////////////////////////////////////////////////////

nlScriptNode *nlMaxParamNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *p = next->Exec();
	long num = 0;
	next->GetValue(&num);
	genPtr->SetMaxParam(num);
	return p;
}

///////////////////////////////////////////////////////////
// OPTION opt on|off -- set an option
///////////////////////////////////////////////////////////
nlScriptNode *nlOptNode::Exec()
{
	switch (token)
	{
	case T_FREQ:
		genPtr->SetFrequencyMode(lVal);
		break;
	case T_VOLDB:
		genPtr->SetVoldbMode(lVal);
		break;
	}
	return next;
}

///////////////////////////////////////////////////////////
// CALL expr - pass the expression as a string to the
// external script engine.
///////////////////////////////////////////////////////////
nlScriptNode *nlCallNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *p = next->Exec();
	nlScriptEngine *eng = genPtr->GetConverter()->GetScriptEngine();
	if (eng)
	{
		char *str;
		next->GetValue(&str);
		eng->CallScript(str);
		delete str;
	}
	return p;
}

///////////////////////////////////////////////////////////
// [note] pitch, rhythm, volume, params.
// This is the node that does the work of actually generating
// a sequencer event. It combines currently set options with
// the values from the note statement.
//
// Note to self : this could be refactored.
// Various values are stored in the curVoice object and then
// used by the Converter object, while others are passed in
// as arguments to BeginNote. At one time, the "last" vals
// were stored as static members of the note node, but are
// now stored in the current voice object, resulting in an inconsistent
// coupling between the generator, cur voice and converter.
// Part of the problem is that the last specified values may be 
// different from the values we want to use in event generation
// due to tempo, articulation, transposition, "tie" and "sus".
// It might be better to set curVoice with "last" values and then
// have MakeEvent calculate any variation in values directly instead
// of calculating them here and passing the values as arguments. Moving
// the calculations to the convertor would make scripting more 
// consistent and also simplify export to other formats (e.g. MIDI).
nlScriptNode *nlNoteNode::Exec()
{
	nlVoice *vox = genPtr->GetCurVoice();
	nlConverter *cvt = genPtr->GetConverter();
	if (next == NULL || vox == NULL || cvt == NULL)
		return NULL;

	long numParms = 0;
	double totalDur = 0.0;
	double dRemainDur;
	double offsetDur = vox->curTime;

	nlScriptNode *list = vox->pitch.Exec(next);

	if (list && list->GetToken() == T_COMMA)
		list = vox->duration.Exec(list->GetNext());
	else
		vox->duration.InitSingle(vox->lastDur);

	if (list && list->GetToken() == T_COMMA)
		list = vox->volume.Exec(list->GetNext());
	else
		vox->volume.InitSingle(vox->lastVol);

	if (vox->articParam)
	{
		if (list && list->GetToken() == T_COMMA)
			list = vox->artic.Exec(list->GetNext());
		else
			vox->artic.InitSingle(vox->lastArtic);
	}

	// parameters
	while (list && list->GetToken() == T_COMMA)
	{
		// We have to exec the node even if we can't use the value(s)...
		list = vox->params[numParms].Exec(list->GetNext());
		if (numParms < vox->maxParam)
			numParms++;
	}

	int isFirst = 1;
	int isMore;
	while (1)
	{
		isMore  = vox->pitch.GetNextValue(&vox->lastPit);
		isMore |= vox->duration.GetNextValue(&vox->lastDur);
		isMore |= vox->volume.GetNextValue(&vox->lastVol);
		if (vox->articParam)
			isMore |= vox->artic.GetNextValue(&vox->lastArtic);
		if (!isMore)
			break;
		
		if (sus)
		{
			if (isFirst)
			{
				totalDur = vox->lastDur;
				dRemainDur = vox->lastDur;
			}
			else
			{
				offsetDur += vox->lastDur;
				dRemainDur -= vox->lastDur;
				vox->lastDur = dRemainDur;
			}
		}
		else if (add)
		{
			if (isFirst)
				totalDur = vox->lastDur;
			else
				offsetDur += vox->lastDur;
		}

		for (int np = 0; np < numParms; np++)
		{
			isMore |= vox->params[np].GetNextValue(&vox->lastParam[np]);
			vox->paramVal[np] = vox->lastParam[np];
		}

		double thisPit = vox->lastPit;
		if (thisPit >= 0)
		{
			if (vox->transpose)
			{
				if (genPtr->GetFrequencyMode())
					thisPit *= vox->transpose; // pow(2, vox->transpose/12.0);
				else
					thisPit += vox->transpose;
			}
			double thisVol;
			if (genPtr->GetVersion() < 1.0)
				thisVol = (vox->lastVol / 327.67) * vox->volMul;
			else
				thisVol = vox->lastVol * vox->volMul;

			if (!add || isFirst)
			{
				double thisDur = vox->lastDur;
				switch (vox->articType)
				{
				case T_PCNT:
					thisDur = (thisDur * vox->lastArtic) / 100.0;
					break;
				case T_FIXED:
					thisDur = vox->lastArtic;
					break;
				case T_ADD:
					thisDur = thisDur + vox->lastArtic;
					break;
				//case T_OFF: <-- implied
				//	break;
				}
				cvt->BeginNote(offsetDur, thisDur, thisVol, thisPit, numParms, vox->paramVal);
				if (vox->doublex)
					cvt->BeginNote(offsetDur, thisDur, thisVol*vox->doublev, thisPit+vox->doublex, numParms, vox->paramVal);
			}
			else
				cvt->ContinueNote(offsetDur, thisVol, thisPit, numParms, vox->paramVal);
		}
		if (vox->pitch.simul)
		{
			if (vox->lastDur > totalDur)
				totalDur = vox->lastDur;
		}
		else if (!sus && !add)
		{
			offsetDur += vox->lastDur;
			totalDur += vox->lastDur;
		}

		isFirst = 0;
	}

	vox->curTime += totalDur;

	return list;
}

///////////////////////////////////////////////////////////
// Note data holds current information for pitch, rhythm
// volume or parameters. If a group was specified, an
// array of values is stored. Otherwise, a single value
// is stored.
///////////////////////////////////////////////////////////

nlScriptNode *nlNoteData::Exec(nlScriptNode *list)
{
	index = 0;
	simul = 0;
	if (list == NULL)
		return NULL;

	int endToken = 0;
	int theToken = list->GetToken();

	nlScriptNode *listNext;
	if (theToken == T_OBRACE)
		endToken = T_CBRACE;
	else if (theToken == T_OBRACK)
	{
		simul = 1;
		endToken = T_CBRACK;
	}

	if (endToken != 0)
	{
		list->GetValue(&count);
		if (count > alloc)
		{
			delete[] values;
			values = new nlVarValue[count];
			alloc = count;
		}
		list = list->GetNext();
		nlVarValue *val = values;
		int nn = 0;
		int tok;
		while (list != NULL && (tok = list->GetToken()) != endToken)
		{
			listNext = list->Exec();
			list->CopyValue(val++);
			if (listNext->GetToken() == T_COMMA)
				list = listNext->GetNext();
			else
				list = listNext;
		}
		return list->GetNext();
	}

	if (alloc == 0)
	{
		values = new nlVarValue[1];
		alloc = 1;
	}
	count = 1;
	listNext = list->Exec();
	list->CopyValue(values);
	return listNext;
}

int nlNoteData::GetNextValue(double *d)
{
	if (index < count && values)
	{
		values[index++].GetValue(d);
		return 1;
	}
	return 0;
}

int nlNoteData::GetNextValue(long *n)
{
	if (index < count && values)
	{
		values[index++].GetValue(n);
		return 1;
	}
	return 0;
}
double nlFunctionData::Iterate(double dur)
{
	if (fnType == T_RAND)
		return (range * (double) rand()) / RAND_MAX;
	if (curVal >= count)
		return endVal;
	double val = curVal / count;
	if (fnType == T_EXP)
		val = pow(val, pos ? 1.5 : 0.5);
	else if (fnType == T_LOG)
		val = pow(val, pos ? 0.5 : 1.5);
	curVal += dur;
	return (val * range) + offset;
}
