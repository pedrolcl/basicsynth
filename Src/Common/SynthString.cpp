#include <stdlib.h>
#include <string.h>
#include <SynthString.h>

char *bsString::nulStr = "";

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
		if (newLen >= maxLen)
			Allocate(newLen);
		curLen = newLen;
	}
	return curLen;
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
