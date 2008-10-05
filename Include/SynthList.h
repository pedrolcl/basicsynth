/////////////////////////////////////////////////////
// BasicSynth double-linked list class
//
// N.B. This is NOT a general purpose linked list class.
// This is low-overhead, fast and simple, which is what 
// the synthesizer needs. :)
//
// Copyright 2008, Daniel R. Mitchell
/////////////////////////////////////////////////////
#ifndef _SYNTHLIST_
#define _SYNTHLIST_

// The Insert/InsertBefore methods take the NEW node that
// should be added to the list...
// you CANNOT do: newNode->Insert(oldNode) !!!!
template<class T> class SynthList
{
public:
	T *next;
	T *prev;

	SynthList()
	{
		next = prev = NULL;
	}

	T *Insert(T *pnew)
	{
		pnew->next = next;
		pnew->prev = (T*)this;
		if (next)
			next->prev = pnew;
		next = pnew;
		return pnew;
	}

	T *InsertBefore(T *pnew)
	{
		pnew->prev = prev;
		pnew->next = (T*)this;
		if (prev)
			prev->next = pnew;
		prev = pnew;
		return pnew;
	}

	T *Remove()
	{
		T *pold = next;
		if (next)
			next->prev = prev;
		if (prev)
			prev->next = next;
		next = NULL;
		prev = NULL;
		return pold;
	}
};
#endif
