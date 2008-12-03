/////////////////////////////////////////////////////
// BasicSynth XML Wrapper class definitions
//
/// @file XmlWrap.h Wrapper classes to provide a portable abstraction over the XML DOM.
//
// define USE_MSXML to use the MS Windows msxml dll
// define USE_LIBXML to use the libxml2 library
// define USE_TINYXML to use the tinyxml library
// define none to create a dummy XML class
// 
// Compile the appropriate .cpp file as well:
//  XmlWrapW.cpp - MSXML
//  XmlWrapU.cpp - Libxml2
//  XmlWrapN.cpp - no XML support
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////
#if !defined(_XMLWRAP_H_)
#define _XMLWRAP_H

#if defined(USE_MSXML)
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <atlbase.h>
//#import "msxml3.dll" raw_interfaces_only named_guids 
//using namespace MSXML2;
#include <msxml2.h>
#endif
#if defined(USE_LIBXML)
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#endif
#if defined(USE_TINYXML)
#include <tinyxml.h>
#endif

class XmlSynthDoc;

/// An XML element node.
class XmlSynthElem
{
private:
	XmlSynthDoc *doc;

#if defined(USE_MSXML)
	char *nodeTag;
	CComQIPtr<IXMLDOMElement> pElem;
	void SetNode(IUnknown *pnode);
#endif
#if defined(USE_LIBXML)
	xmlNodePtr pElem;
	void SetNode(xmlNodePtr pnode) { pElem = pnode; }
	xmlNodePtr NextElement(xmlNodePtr e);
#endif
#if defined(USE_TINYXML)
	TiXmlElement *pElem;
	void SetNode(TiXmlElement *pnode) { pElem = pnode; }
#endif
	
	friend class XmlSynthDoc;

public:
	XmlSynthElem(XmlSynthDoc *p = 0);
	~XmlSynthElem();
	void Clear();
	XmlSynthElem *FirstChild();
	XmlSynthElem *FirstChild(XmlSynthElem *childElem);
	XmlSynthElem *NextSibling();
	XmlSynthElem *NextSibling(XmlSynthElem *childElem);
	XmlSynthElem *AddChild(const char *childTag);
	XmlSynthElem *AddChild(const char *childTag, XmlSynthElem *childElem);
	const char *TagName();
	int GetAttribute(char *attrName, short& val);
	int GetAttribute(char *attrName, long& val);
	int GetAttribute(char *attrName, float& val);
	int GetAttribute(char *attrName, double& val);
	int GetAttribute(char *attrName, char **val);
	int SetAttribute(char *attrName, short val);
	int SetAttribute(char *attrName, long val);
	int SetAttribute(char *attrName, float val);
	int SetAttribute(char *attrName, double val);
	int SetAttribute(char *attrName, const char *val);
	int TagMatch(const char *tag);
	int SetContent(const char *data);
	int GetContent(char **data);
};

/// An XML document.
class XmlSynthDoc
{
private:
#if defined(USE_MSXML)
	CComQIPtr<IXMLDOMDocument> pDoc;
	int GetXmlDoc();
#endif
#if defined(USE_LIBXML)
	xmlDocPtr doc;
	xmlNodePtr root;
#endif
#if defined(USE_TINYXML)
	TiXmlDocument *doc;
#endif

public:
	XmlSynthDoc();
	~XmlSynthDoc();
	XmlSynthElem *CreateElement(XmlSynthElem *parent, const char *tag);
	XmlSynthElem *CreateElement(XmlSynthElem *parent, const char *tag, XmlSynthElem *childElem);
	XmlSynthElem *NewDoc(char *roottag);
	XmlSynthElem *NewDoc(char *roottag, XmlSynthElem *rootElem);
	XmlSynthElem *Open(char *fname);
	XmlSynthElem *Open(char *fname, XmlSynthElem *rootElem);
	int Save(char *fname);
	int Close();
};

#endif
