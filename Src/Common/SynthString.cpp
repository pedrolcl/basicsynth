//////////////////////////////////////////////////////////////////
/// @file SynthString.cpp Implementation of bsString
//
// BasicSynth Library
//
// String class implementation
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <SynthString.h>

const char *bsString::nulStr = "";

char *bsString::Allocate(size_t n)
{
	n++; // for null terminator
	if (n >= maxLen)
	{
		// round up to 16-byte boundary
		n = (n + 15) & ~0xF;
		char *newStr = new char[n];
		if (newStr == 0)
			return 0;
		memset(newStr, 0, n);
		if (theStr)
		{
			memcpy(newStr, theStr, maxLen);
			delete theStr;
		}
		theStr = newStr;
		maxLen = n;
	}
	return theStr;
}

int bsString::SetLen(int newLen)
{
	if (theStr == NULL)
		curLen = 0;
	else if (newLen == -1)
		curLen = strlen(theStr);
	else
	{
		if ((size_t)newLen >= maxLen)
			Allocate((size_t)newLen);
		curLen = newLen;
		theStr[curLen] = 0;
	}
	return (int) curLen;
}

bsString& bsString::Assign(const char *s)
{
	if (s)
	{
		size_t n = strlen(s);
		if (Allocate(n))
		{
			strcpy(theStr, s);
			curLen = n;
		}
	}
	else
	{
		curLen = 0;
		if (theStr)
			*theStr = 0;
	}
	return *this;
}

bsString& bsString::Append(const char *s)
{
	if (s)
	{
		size_t n = curLen + strlen(s);
		if (Allocate(n))
		{
			strcat(theStr, s);
			curLen = n;
		}
	}
	return *this;
}

// return < 0 if this < s1
// return > 0 if this > s1
// return 0 if this == s1
// This is ANSI only, not true MBCS
// Most of the code in BS only looks for ==

int bsString::CompareNC(const char *s1)
{
	char *s2 = theStr;
	if (!s1)
	{
		if (s2)
			return 1;
		return 0;
	}
	if (!s2)
		return -1;

	char c1, c2;
	while (*s1 && *s2)
	{
		if ((c1 = *s1++) >= 'a' && c1 <= 'z')
			c1 = 'A' + c1 - 'a';
		if ((c2 = *s2++) >= 'a' && c2 <= 'z')
			c2 = 'A' + c2 - 'a';
		if (c1 != c2)
			return c2 - c1;
	}
	return *s2 - *s1;
}

int bsString::Compare(const char *s1)
{
	if (!s1)
	{
		if (theStr)
			return 1;
		return 0;
	}
	if (!theStr)
		return -1;
	return strcmp(theStr, s1);
}

bsString& bsString::Upper()
{
	if (curLen > 0)
	{
		char ch;
		char *s = theStr;
		for (size_t n = curLen; n > 0;  n--)
		{
			if ((ch = *s) >= 'a' && ch <= 'z')
				*s = ch - 'a' + 'A';
			s++;
		}
	}
	return *this;
}

bsString& bsString::Lower()
{
	if (curLen > 0)
	{
		char ch;
		char *s = theStr;
		for (size_t n = curLen; n > 0;  n--)
		{
			if ((ch = *s) >= 'A' && ch <= 'Z')
				*s = ch - 'A' + 'a';
			s++;
		}
	}
	return *this;
}

int bsString::Find(int start, int ch)
{
	if (theStr == 0)
		return -1;
	if (start >= (int)curLen)
		start = (int)curLen-1;
	if (start < 0)
		start = 0;
	const char *p1 = strchr(&theStr[start], ch);
	if (p1)
		return (int) (p1 - theStr);
	return -1;
}

int bsString::FindReverse(int start, int ch)
{
	if (theStr == 0)
		return -1;
	if (start >= (int)curLen)
		return -1;
	if (start < 0)
		start = 0;
	const char *p1 = strrchr(&theStr[start], ch);
	if (p1)
		return (int) (p1 - theStr);
	return -1;
}

size_t bsString::SubString(bsString& out, int start, size_t len)
{
	if (len > curLen)
		len = curLen;
	if (start < 0)
		start = 0;
	out.Allocate(len);
	out.curLen = 0;
	if (start < (int)curLen)
	{
		char *p1 = &theStr[start];
		char *p2 = out.theStr;
		while (len > 0 && start++ < (int)curLen)
		{
			*p2++ = *p1++;
			len--;
			out.curLen++;
		}
		*p2 = 0;
	}
	return out.curLen;
}

int bsString::SplitPath(bsString& base, bsString& file, int inclSep)
{
	if (curLen == 0)
	{
		base = NULL;
		file = NULL;
		return 0;
	}
	int slash = FindReverse(0, '\\');
	if (slash < 0)
	{
		if ((slash = FindReverse(0, '/')) < 0)
			slash = Find(0, ':');
	}
	if (slash >= 0)
	{
		SubString(base, 0, slash + (inclSep ? 1 : 0));
		SubString(file, slash+1, -1);
		return 2;
	}
	base = NULL;
	file = theStr;
	return 1;
}

void bsString::Attach(char *str, int cl, int ml)
{
	if (theStr)
		delete theStr;
	theStr = str;
	if (str == 0)
	{
		curLen = 0;
		maxLen = 0;
	}
	else
	{
		if (cl < 0)
		curLen = strlen(str);
		else
			curLen = cl;
		if (ml < 0)
			maxLen = curLen + 1;
		else
			maxLen = ml;
	}
}

char *bsString::Detach(int *cl, int *ml)
{
	char *str = theStr;
	theStr = 0;
	if (cl)
		*cl = (int)curLen;
	if (ml)
		*ml = (int)maxLen;
	curLen = 0;
	maxLen = 0;
	return str;
}
