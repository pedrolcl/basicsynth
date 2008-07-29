// Generate.cpp: implementation of the nlGenerate class.
//
//////////////////////////////////////////////////////////////////////

#include "NLConvert.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define GENSTACKSIZE 20

nlGenerate::nlGenerate()
{
	pMain = new nlSequence("<Main>");
	curSeq = pMain;
	
	InitStack();

	for (int i = 0; i < MAXFGEN; i++)
		iFnGen[i] = NULL;

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
	while (synclist)
	{
		smp = synclist->next;
		synclist = smp->next;
		delete smp;
	}
	nlVoice *vp;
	while (voiceList)
	{
		vp = voiceList;
		voiceList = vp->next;
		delete vp;
	}
	delete pMain;
	pMain = NULL;
	curSeq = NULL;
}

void nlGenerate::Reset()
{
	Clear();
	pMain = new nlSequence("<Main>");
	curSeq = pMain;
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
		if (nlVersion < 3.0)
			vp->articParam = 1;
		voiceList = vp;
	}
	curVoice = vp;
	cvtPtr->BeginVoice(curVoice);
	return curVoice;
}

int nlGenerate::Run()
{
	cvtPtr->BeginNotelist();
	pMain->Play();
	cvtPtr->EndNotelist();

	return 0;
}

void nlGenerate::SetMaxParam(long n)
{
	maxParam = n;
//	if (curVoice)
//		curVoice->SetMaxParam(n);
//	nlVoice *vp;
//	for (vp = voiceList; vp; vp = vp->next)
//		vp->SetMaxParam(n);
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

void nlGenerate::PushStack(char *s)
{
	spCur->SetValue(s);
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

nlSequence *nlGenerate::AddSequence(char *name)
{
	nlSequence *pseq = new nlSequence(name);
	pseq->Append(&pMain);
	return pseq;
}

nlSequence *nlGenerate::SetCurSeq(nlSequence *p)
{
	nlSequence *pold = curSeq;
	curSeq = p;
	return pold;
}

nlSequence *nlGenerate::FindSequence(char *name)
{
	return pMain->FindSequence(name);
}

nlScriptNode *nlGenerate::AddNode(nlScriptNode *pn)
{
	pn->SetGen(this);
	return curSeq->AddNode(pn);
}


nlScriptNode *nlGenerate::AddNode(int token, char *text)
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

nlSequence::nlSequence(char *n)
{
	name = StrMakeCopy(n);
	iHead = new nlScriptNode;
	iHead->SetToken(T_START);
	iTail = iHead;
	iCur = NULL;
	pNext = NULL;
}

nlSequence::~nlSequence()
{
	delete name;
	while (iHead != NULL)
	{
		iCur = iHead;
		iHead = iHead->GetNext();
		delete iCur;
	}
}

void nlSequence::Play()
{
	nlScriptNode *p = iHead->GetNext();
	while (p != NULL)
		p = p->Exec();
}

nlScriptNode *nlSequence::AddNode(nlScriptNode *pn)
{
	pn->Append(iTail);
	iTail = pn;
	return pn;
}

void nlSequence::Append(nlSequence **pList)
{
	while (*pList != NULL)
		pList = &(*pList)->pNext;
	*pList = this;
}

nlSequence *nlSequence::FindSequence(char *pfind)
{
	if (CompareToken(name, pfind) == 0)
		return this;
	if (pNext != NULL)
		return pNext->FindSequence(pfind);
	return NULL;
}

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

nlScriptNode::nlScriptNode()
{
	token = -1;
	next = NULL;
}

nlScriptNode::~nlScriptNode()
{
}

void nlVarValue::ClearValue()
{
	if (vt == vtText)
		delete txtVal;
	vt = vtNull;
}

void nlVarValue::SetValue(char *p)
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

nlScriptNode *nlVoiceNode::Exec()
{
	if (next == NULL)
		return NULL;

	// get the voice number and switch voices
	nlScriptNode *p = next->Exec();

	long vnum;
	next->GetValue(&vnum);
	genPtr->SetCurVoice(vnum);

	if (p != NULL)
		return p->Exec();

	// OOOPS
	return NULL;
}

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

	nlScriptNode *pr = next->Exec();
	if (next->vt == vtText)
		pv->instr = (long) cvtPtr->FindInstrNum(next->txtVal);
	else
		next->GetValue(&pv->instr);
	cvtPtr->BeginInstr();

	return pr;
}

nlScriptNode *nlMixNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlVoice *pv = genPtr->GetCurVoice();
	if (pv == NULL)
		return NULL;

	nlScriptNode *p = next->Exec();
	next->GetValue(&pv->chnl);
	return p;
}

nlScriptNode *nlBlockNode::Exec()
{
	nlScriptNode *p = next;
	while (p != NULL)
	{
		if (p->GetToken() == T_END)
			return p->GetNext();
		p = p->Exec();
	}
	return NULL;
}


nlScriptNode *nlTempoNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *p, *p2;
	if (next->GetToken() == T_DUR)
	{
		// hack a little. we don't want the duration
		// node to calculate actual time value.
		((nlDurNode*)next)->GetBeatValue(&genPtr->beat);
		p = next->GetNext();
	}
	else
	{
		p = next->Exec();
		next->GetValue(&genPtr->beat);
	}

	if (p == NULL)
		return NULL;

	p2 = p->Exec();
	double t;
	p->GetValue(&t);
	genPtr->secBeat = 60.0 / t;
	
	return p2;
}

nlScriptNode *nlTimeNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *p = next->Exec();
	nlVoice *vp = genPtr->GetCurVoice();
	if (vp)
		next->GetValue(&vp->curTime);
	return p;
}

nlScriptNode *nlMarkNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *p = next->Exec();
	genPtr->MarkTo(next);
	return p;
}

nlScriptNode *nlSyncNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *p = next->Exec();
	genPtr->SyncTo(next);
	return p;
}

nlScriptNode *nlVolumeNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *p = next->Exec();
	nlVoice *pv = genPtr->GetCurVoice();
	if (pv)
	{
		double d = 0;
		next->GetValue(&d);
		if (genPtr->nlVersion >= 3.0)
			pv->volMul = d / 100.0;
		else
			pv->volMul = d;
	}
	return p;
}

nlScriptNode *nlTransposeNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *p = next->Exec();
	nlVoice *vp = genPtr->GetCurVoice();
	if (vp)
		next->GetValue(&vp->transpose);
	return p;
}

nlScriptNode *nlLoopNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlVoice *pv = genPtr->GetCurVoice();
	if (pv == NULL)
		return NULL;

	// push the loop count
	long savCount = pv->loopCount;

	nlScriptNode *startLoop = next->Exec();
	if (startLoop == NULL)
		return NULL;

	long count = 0;
	next->GetValue(&count);
	// we need to run the loop at least once to find the end...
	if (count == 0)
		count = 1;

	nlScriptNode *endLoop;
	for (long n = 0; n < count; n++)
	{
		pv->loopCount = n;
		endLoop = startLoop->Exec();
	}

	// pop the loop count
	pv->loopCount = savCount;

	return endLoop;
}

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
		*d = genPtr->beat / dblVal * genPtr->secBeat;
		if (isDotted)
			*d += (*d / 2);
	}
}

void nlDurNode::CopyValue(nlVarValue *p)
{
	p->vt = vtReal;
	GetValue(&p->dblVal);
}

nlScriptNode *nlExprNode::Exec()
{
	double d1;
	double d2;
	long pit;
	long fno;
	nlIntegratorData *pfn;
	char *pstr1;
	char *pstr2;
	char *pstr3;

	nlVoice *pv = genPtr->GetCurVoice();

	nlScriptNode *p2;
	nlScriptNode *p = next;
	while (p != NULL)
	{
		switch (p->GetToken())
		{
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
				genPtr->PushStack(d2); // hmmmm
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
			pstr3 = StrPaste(pstr1, pstr2);
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
		case T_NUM:
			p->GetValue(&d1);
			genPtr->PushStack(d1);
			break;
		case T_PIT:
			p->GetValue(&pit);
			genPtr->PushStack(pit);
			break;
		case T_DUR:
			p->Exec();
			p->GetValue(&d1);
			genPtr->PushStack(d1);
			break;
		case T_STRLIT:
			p->GetValue(&pstr1);
			genPtr->PushStack(pstr1);
			delete pstr1;
			break;
		case T_NEG:
			genPtr->PopStack(&d1);
			genPtr->PushStack(-d1);
			break;
		case T_FGEN:
			p = p->GetNext();
			p2 = p->Exec();
			p->GetValue(&fno);
			p = p2->Exec();
			p2->GetValue(&d2);
			pfn = genPtr->iFnGen[fno];
			if (pfn != NULL)
				d1 = pfn->Iterate(d2);
			else
				d1 = 0.0;
			genPtr->PushStack(d1);
			continue;
		case T_RAND:
			p = p->GetNext();
			p2 = p->Exec();
			p->GetValue(&d1);
			p = p2->Exec();
			p2->GetValue(&d2);
			genPtr->PushStack((((d2 - d1) * (double) rand()) / RAND_MAX) + d1);
			continue;
		case T_COUNT:
			if (pv)
				genPtr->PushStack(pv->loopCount);
			else
				genPtr->PushStack(0L);
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

nlScriptNode *nlParamNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *pnext = next->Exec();
	long pno = -1;
	next->GetValue(&pno);
	next = pnext->Exec();
	nlVoice *pv = genPtr->GetCurVoice();
	if (pv)
	{
		if (pno < 0)
		{
			pnext->GetValue(&pv->cntParam);
			if (pv->cntParam > pv->maxParam)
				pv->cntParam = pv->maxParam;
		}
		else if (pno < pv->maxParam)
			pnext->GetValue(&pv->lastParam[pno]);
	}

	return next;
}

nlScriptNode *nlNoteNode::Exec()
{
	if (next == NULL)
		return NULL;
	nlVoice *pv = genPtr->GetCurVoice();
	if (pv == NULL)
		return NULL;
	nlConverter *pc = genPtr->GetConverter();

	long numParms = 0;
	double totalDur = 0.0;
	double dRemainDur;
	double offsetDur = pv->curTime;

	nlScriptNode *pList = next;

	pList = pv->pitch.Exec(next);

	if (pList && pList->GetToken() == T_COMMA)
		pList = pv->duration.Exec(pList->GetNext());
	else
		pv->duration.InitSingle(pv->lastDur);

	if (pList && pList->GetToken() == T_COMMA)
		pList = pv->volume.Exec(pList->GetNext());
	else
		pv->volume.InitSingle(pv->lastVol);

	if (pv->articParam)
	{
		if (pList && pList->GetToken() == T_COMMA)
			pList = pv->artic.Exec(pList->GetNext());
		else
			pv->artic.InitSingle(pv->lastArtic);
	}

	// parameters
	while (pList && pList->GetToken() == T_COMMA)
	{
		// We have to exec the node even if we can't use the value(s)...
		// Notice that this will use the later params if we have too many!!!
		pList = pv->params[numParms].Exec(pList->GetNext());
		if (numParms < pv->maxParam)
			numParms++;
	}

	int isFirst = 1;
	int isMore;
	while (1)
	{
		isMore  = pv->pitch.GetNextValue(&pv->lastPit);
		isMore |= pv->duration.GetNextValue(&pv->lastDur);
		isMore |= pv->volume.GetNextValue(&pv->lastVol);
		if (pv->articParam)
			isMore |= pv->artic.GetNextValue(&pv->lastArtic);
		if (!isMore)
			break;
		
		if (bSus)
		{
			if (isFirst)
			{
				totalDur = pv->lastDur;
				dRemainDur = pv->lastDur;
			}
			else
			{
				offsetDur += pv->lastDur;
				dRemainDur -= pv->lastDur;
				pv->lastDur = dRemainDur;
			}
		}

		for (int np = 0; np < numParms; np++)
		{
			isMore |= pv->params[np].GetNextValue(&pv->lastParam[np]);
			pv->paramVal[np] = pv->lastParam[np];
		}

		double thisPit = pv->lastPit;
		if (thisPit >= 0)
		{
			thisPit += pv->transpose;
			double thisVol;
			if (genPtr->nlVersion < 3.0)
				thisVol = (pv->lastVol / 327.67) * pv->volMul;
			else
				thisVol = pv->lastVol * pv->volMul;

			if (!bAdd)
			{
				double thisDur = pv->lastDur;
				switch (pv->articType)
				{
				case T_PCNT:
					thisDur = (thisDur * pv->lastArtic) / 100.0;
					break;
				case T_FIXED:
					thisDur = pv->lastArtic;
					break;
				case T_ADD:
					thisDur = thisDur + pv->lastArtic;
					break;
				//case T_OFF: <-- implied
				//	break;
				}
				pc->BeginNote(offsetDur, thisDur, thisVol, thisPit, numParms, pv->paramVal);
			}
			else
				pc->ContinueNote(offsetDur, thisVol, thisPit, numParms, pv->paramVal);
		}
		if (pv->pitch.nSimul)
		{
			if (pv->lastDur > totalDur)
				totalDur = pv->lastDur;
		}
		else if (!bSus)
		{
			offsetDur += pv->lastDur;
			if (!bAdd)
				totalDur += pv->lastDur;
			else
				totalDur = pv->lastDur;
		}

		isFirst = 0;
	}

	pv->curTime += totalDur;

	return pList;
}

nlScriptNode *nlNoteData::Exec(nlScriptNode *pList)
{
	nIndex = 0;
	nSimul = 0;
	if (pList == NULL)
		return NULL;

	int nEndTok = 0;
	int theToken = pList->GetToken();

	nlScriptNode *pListNext;
	if (theToken == T_OBRACE)
		nEndTok = T_CBRACE;
	else if (theToken == T_OBRACK)
	{
		nSimul = 1;
		nEndTok = T_CBRACK;
	}

	if (nEndTok != 0)
	{
		pList->GetValue(&nCount);
		if (nCount > nAlloc)
		{
			delete[] pValues;
			pValues = new nlVarValue[nCount];
			nAlloc = nCount;
		}
		pList = pList->GetNext();
		nlVarValue *pVal = pValues;
		int nn = 0;
		int tok;
		while (pList != NULL && (tok = pList->GetToken()) != nEndTok)
		{
			pListNext = pList->Exec();
			pList->CopyValue(pVal);
			pVal++;
			pList = pListNext;
			if (pList->GetToken() == T_COMMA)
				pList = pList->GetNext();
		}
		return pList->GetNext();
	}

	if (nAlloc == 0)
	{
		pValues = new nlVarValue[1];
		nAlloc = 1;
	}
	nCount = 1;
	pListNext = pList->Exec();
	pList->CopyValue(pValues);
	return pListNext;
}

int nlNoteData::GetNextValue(double *d)
{
	if (nIndex < nCount)
	{
		pValues[nIndex].GetValue(d);
		nIndex++;
		return 1;
	}
	return 0;
}

int nlNoteData::GetNextValue(long *n)
{
	if (nIndex < nCount && pValues)
	{
		pValues[nIndex].GetValue(n);
		nIndex++;
		return 1;
	}
	return 0;
}

nlScriptNode *nlWriteNode::Exec()
{
	genPtr->GetConverter()->Write(txtVal);
	return next;
}

nlScriptNode *nlPlayNode::Exec()
{
	nlSequence *p = genPtr->FindSequence(name);
	if (p != NULL)
		p->Play();
	return next;
}

nlScriptNode *nlInitFnNode::Exec()
{
	if (next == NULL)
		return NULL;

	nlScriptNode *endExpr;
	nlScriptNode *p = next;

	long fn;
	p->GetValue(&fn);
	if (fn < 0 || fn >= MAXFGEN)
		return NULL;
	nlIntegratorData *pfn = genPtr->iFnGen[fn];
	if (pfn == NULL)
	{
		pfn = new nlIntegratorData;
		genPtr->iFnGen[fn] = pfn;
	}

	int type = p->GetToken();
	p = p->GetNext();

	double beginVal;
	double endVal;
	double durVal;

	endExpr = p->Exec();
	p->GetValue(&beginVal);
	p = endExpr;
	
	endExpr = p->Exec();
	p->GetValue(&endVal);
	p = endExpr;

	endExpr = p->Exec();
	p->GetValue(&durVal);

	pfn->Init(beginVal, endVal, durVal, type);

	return endExpr;
}

nlScriptNode *nlArticNode::Exec()
{
	nlVoice *pv = genPtr->GetCurVoice();
	GetValue(&pv->articType);

	if (genPtr->nlVersion < 3.0)
		return next;

	nlScriptNode *pList = next;
	if (pList == NULL)
		return NULL;
	int tok = pList->GetToken();
	if (tok == T_PARAM)
	{
		pv->articParam = 1;
		pList = pList->GetNext();
	}
	else if (tok == T_COMMA)
	{
		pList = pList->GetNext();
		nlScriptNode *pListNext = pList->Exec();
		pList->GetValue(&pv->lastArtic);
		pv->articParam = 0;
		pList = pListNext;
	}
	else if (pv->articType == T_OFF)
		pv->articParam = 0;
	return pList;
}

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
	
	pList = pList->GetNext();
	if (pList == NULL)
		return NULL;

	pList = pList->GetNext();
	while (pList != NULL)
	{
		pListNext = pList->Exec();
		pList->GetValue(&tmp);
		pList = pListNext;
		if (pList->GetToken() == T_EQ)
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

double nlIntegratorData::Iterate(double dDur)
{
	if (fnType == T_RAND)
		return (range * (double) rand()) / RAND_MAX;
	if (curVal >= count)
		return endVal;
	double dVal = curVal / count;
	if (fnType == T_EXP)
		dVal = pow(dVal, pos ? 1.5 : 0.5);
	else if (fnType == T_LOG)
		dVal = pow(dVal, pos ? 0.5 : 1.5);
	curVal += dDur;
	return (dVal * range) + offset;
}
