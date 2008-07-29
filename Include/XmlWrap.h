
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

class XmlSynthDoc;

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

	friend class XmlSynthDoc;

public:
	XmlSynthElem(XmlSynthDoc *p = NULL);
	~XmlSynthElem();
	XmlSynthElem *FirstChild();
	XmlSynthElem *NextSibling();
	XmlSynthElem *AddChild(const char *childTag);
	const char *TagName();
	int GetAttribute(char *attrName, long& val);
	int GetAttribute(char *attrName, double& val);
	int GetAttribute(char *attrName, char **val);
	int SetAttribute(char *attrName, long val);
	int SetAttribute(char *attrName, double val);
	int SetAttribute(char *attrName, const char *val);
	int TagMatch(const char *tag);
	int SetContent(const char *data);
	int GetContent(char **data);
};

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

public:
	XmlSynthDoc();
	~XmlSynthDoc();
	XmlSynthElem *CreateElement(XmlSynthElem *parent, const char *tag);
	XmlSynthElem *NewDoc(char *roottag);
	XmlSynthElem *Open(char *fname);
	int Save(char *fname);
	int Close();
};

#endif
