/////////////////////////////////////////////////////////
// BasicSynth string object
//
// This is not a very clever or extensive string class,
// but it does all that BasicSynth needs.
////////////////////////////////////////////////////////
#ifndef _SYNTHSTRING_
#define _SYNTHSTRING_

#include <string.h>

class bsString
{
private:
	char *theStr;
	size_t maxLen;
	size_t curLen;
	static char *nulStr;
public:
	bsString()
	{
		theStr = NULL;
		maxLen = 0;
		curLen = 0;
	}

	bsString(char *s)
	{
		theStr = NULL;
		maxLen = 0;
		curLen = 0;
		Assign(s);
	}

	bsString(bsString& s)
	{
		theStr = NULL;
		maxLen = 0;
		curLen = 0;
		Assign(s);
	}

	~bsString()
	{
		delete theStr;
	}

	size_t Length() const
	{
		return curLen;
	}

	operator const char *()
	{
		if (theStr)
			return theStr;
		return nulStr;
	}

	char operator[](size_t n)
	{
		if (n >= 0 && n < curLen)
			return theStr[n];
		return 0;
	}

	char *Allocate(size_t n);
	int SetLen(int newLen = -1);

	bsString& Assign(const char *s);

	bsString& Assign(const bsString& s)
	{
		if (Allocate(s.Length()))
			strcpy(theStr, s.theStr);
		return *this;
	}

	bsString& operator=(const char *s)
	{
		return Assign(s);
	}

	bsString& operator=(const bsString& s)
	{
		return Assign(s);
	}

	bsString& Append(const char *s);

	bsString& operator+=(const char *s)
	{
		return Append(s);
	}

	bsString& operator+=(const bsString& s)
	{
		return Append(s.theStr);
	}

	bsString& operator+=(char ch)
	{
		if (Allocate(curLen+1))
		{
			theStr[curLen++] = ch;
			theStr[curLen] = 0;
		}
		return *this;
	}

	int Compare(const char *s1);

	int Compare(bsString& s)
	{
		return Compare(s.theStr);
	}

	int operator==(const char *s)
	{
		return Compare(s) == 0;
	}

	int operator==(bsString& s)
	{
		return Compare(s) == 0;
	}

	int operator!=(const char *s)
	{
		return Compare(s) != 0;
	}

	int operator!=(bsString& s)
	{
		return Compare(s) != 0;
	}

	int CompareNC(const char *s1);

	int CompareNC(bsString& s)
	{
		return CompareNC(s.theStr);
	}

	bsString& Upper();
	bsString& Lower();

};

#endif
