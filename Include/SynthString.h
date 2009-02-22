/////////////////////////////////////////////////////////
// BasicSynth string object
//
/// \file SynthString.h
/// Defines a string class.
/// This is not a very clever or extensive string class,
/// but it does all that BasicSynth needs.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
////////////////////////////////////////////////////////
/// \addtogroup grpGeneral
#ifndef _SYNTHSTRING_
#define _SYNTHSTRING_
/*@{*/

#include <string.h>

/// String management class. bsString maintains a variable
/// length buffer and string size. Operations are avaialble to assign, append
/// and compare strings.
class bsString
{
private:
	char *theStr;        // character string buffer
	size_t maxLen;       // size of allocated buffer
	size_t curLen;       // current actual string length
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

	/// Get the length of the string.
	size_t Length() const
	{
		return curLen;
	}

	/// Cast to a char* type
	operator const char *()
	{
		if (theStr)
			return theStr;
		return nulStr;
	}

	/// Get a character at a specific position.
	char operator[](size_t n)
	{
		if (n >= 0 && n < curLen)
			return theStr[n];
		return 0;
	}

	/// Allocate the internal buffer. This will be called automatically when
	/// needed, but may also be called directly to pre-allocate a buffer. This
	/// is useful if multple appends to the string are intended.
	/// \param n size of buffer to allocate
	char *Allocate(size_t n);

	/// Set the string length. If the new length is less than the current length,
	/// the string is truncated. If the length is longer, the string buffer is extended.
	/// If -1 is used for the new length, the actual string length is calculated from
	/// the buffer content.
	/// \param newLen new length of the string
	int SetLen(int newLen = -1);

	/// \name String assignment
	/// Assignment to the string
	/// The character string is copied to the internal buffer.
	/// The internal buffer is allocated or extended as needed.
	/// \param s string to assign to the object
	//@{
	bsString& Assign(const char *s);

	bsString& Assign(const bsString& s)
	{
		if (Allocate(s.Length()))
		{
			strcpy(theStr, s.theStr);
			curLen = s.curLen;
		}
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
	//@}

	/// \name String concatenation
	/// Append to the string.
	/// New characters are added to the end of the buffer.
	/// If needed, the buffer is automatically extended. The new string length is calculated as well.
	/// \param s string to append
	//@{
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
	//@}

	/// \name String Comparison
	/// Compare two strings. String comparison is character by character and may be
	/// case sensitive or not. The return value follows the C library strcmp convention.
	/// \param s string to compare with
	//@{
	int Compare(const char *s);

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
	//@}

	/// Convert to upper case.
	bsString& Upper();
	/// Convert to lower case.
	bsString& Lower();

	/// Find the first instance of ch after start
	int Find(int start, int ch);

	/// Extract the sub-string
	size_t SubString(bsString& out, int start, size_t len);
};
/*@}*/
#endif
