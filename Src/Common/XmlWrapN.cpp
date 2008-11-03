//////////////////////////////////////////////////////////////////
// BasicSynth
//
// NULL XmlWrapper - dummy functions for systems that don't 
// load instruments and projects from XML files. This will
// resolve references, but always returns error.
//
// To implement configuration storage using something other
// than MSXML and LIBXML2, copy this file, implement each
// method, and link in your object instead of this one.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////
#include <XmlWrap.h>
#if !defined(USE_MSXML) && !defined(USE_LIBXML)

XmlSynthElem::XmlSynthElem(XmlSynthDoc *p)
{
}

XmlSynthElem::~XmlSynthElem()
{
}

XmlSynthElem *XmlSynthElem::FirstChild()
{
	return 0;
}

XmlSynthElem *XmlSynthElem::NextSibling()
{
	return 0;
}

XmlSynthElem *XmlSynthElem::AddChild(const char *childTag)
{
	return 0;
}

const char *XmlSynthElem::TagName()
{
	return "";
}

int XmlSynthElem::GetAttribute(char *attrName, short& val)
{
	val = 0;
	return -1;
}

int XmlSynthElem::GetAttribute(char *attrName, long& val)
{
	val = 0;
	return -1;
}

int XmlSynthElem::GetAttribute(char *attrName, float& val)
{
	val = 0.0;
	return -1;
}

int XmlSynthElem::GetAttribute(char *attrName, double& val)
{
	val = 0.0;
	return -1;
}

int XmlSynthElem::GetAttribute(char *attrName, char **val)
{
	*val = NULL;
	return -1;
}

int XmlSynthElem::SetAttribute(char *attrName, short val)
{
	return -1;
}

int XmlSynthElem::SetAttribute(char *attrName, long val)
{
	return -1;
}

int XmlSynthElem::SetAttribute(char *attrName, float val)
{
	return -1;
}

int XmlSynthElem::SetAttribute(char *attrName, double val)
{
	return -1;
}

int XmlSynthElem::SetAttribute(char *attrName, const char *val)
{
	return -1;
}

int XmlSynthElem::TagMatch(const char *tag)
{
	return -1;
}

int XmlSynthElem::SetContent(const char *data)
{
	return -1;
}

int XmlSynthElem::GetContent(char **data)
{
	*data = 0;
	return -1;
}

//////////////////////////////////////

XmlSynthDoc::XmlSynthDoc()
{
}

XmlSynthDoc::~XmlSynthDoc()
{
}

XmlSynthElem *XmlSynthDoc::CreateElement(XmlSynthElem *parent, const char *tag)
{
	return 0;
}

XmlSynthElem *XmlSynthDoc::NewDoc(char *roottag)
{
	return 0;
}

XmlSynthElem *XmlSynthDoc::Open(char *fname)
{
	return 0;
}

int XmlSynthDoc::Save(char *fname)
{
	return -1;
}

int XmlSynthDoc::Close()
{
	return -1;
}

#else
int _not_using_xml_null = 1;
#endif
