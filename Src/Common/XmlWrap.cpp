#include "Includes.h"

XmlSynthElem::XmlSynthElem(XmlSynthDoc *p)
{
	doc = p;
	nodeTag = NULL;
}

XmlSynthElem::~XmlSynthElem()
{
#if defined(_WINDOWS)
	pElem.Release();
#endif
}

#if defined(_WINDOWS)
char *WideToMulti(LPOLESTR str, UINT len)
{
	int need = WideCharToMultiByte(CP_UTF8, 0, str, len, NULL, 0, NULL, NULL);
	char *ret = new char[need+1];
	if (ret != NULL)
	{
		WideCharToMultiByte(CP_UTF8, 0, str, len, ret, need+1, NULL, NULL);
		ret[need] = 0;
	}
	return ret;
}

void XmlSynthElem::SetNode(IUnknown *pnode)
{
	if (nodeTag)
	{
		delete nodeTag;
		nodeTag = NULL;
	}

	pElem = pnode;
	if (pElem)
	{
		CComBSTR bsTag;
		pElem->get_tagName(&bsTag.m_str);
		nodeTag = WideToMulti(bsTag.m_str, bsTag.Length());
	}
}
#endif

XmlSynthElem *XmlSynthElem::FirstChild()
{
	XmlSynthElem *newElem = NULL;
#if defined(_WINDOWS)
	if (pElem)
	{
		CComPtr<IXMLDOMNode> pnode;
		HRESULT hr = pElem->get_firstChild(&pnode);
		if (hr == S_OK && pnode)
		{
			newElem = new XmlSynthElem(doc);
			newElem->SetNode(pnode);
		}
	}
#endif
	return newElem;
}

XmlSynthElem *XmlSynthElem::NextSibling()
{
	XmlSynthElem *newElem = NULL;
#if defined(_WINDOWS)
	if (pElem)
	{
		CComPtr<IXMLDOMNode> pnode;
		HRESULT hr = pElem->get_nextSibling(&pnode);
		if (hr == S_OK && pnode)
		{
			newElem = new XmlSynthElem(doc);
			newElem->SetNode(pnode);
		}
	}
#endif
	return newElem;
}

XmlSynthElem *XmlSynthElem::AddChild(const char *childTag)
{
	if (doc)
		return doc->CreateElement(this, childTag);
	return NULL;
}

const char *XmlSynthElem::TagName()
{
	return nodeTag;
}

int XmlSynthElem::GetAttribute(char *attrName, long& val)
{
	int rv = -1;
#if defined(_WINDOWS)
	if (pElem)
	{
		CComBSTR name(attrName);
		CComVariant v;
		if (pElem->getAttribute(name, &v) == S_OK)
		{
			v.ChangeType(VT_I4);
			val = v.lVal;
			rv = 0;
		}
		else
			val = 0;
	}
#endif
	return rv;
}

int XmlSynthElem::GetAttribute(char *attrName, double& val)
{
	int rv = -1;
#if defined(_WINDOWS)
	if (pElem)
	{
		CComBSTR name(attrName);
		CComVariant v;
		if (pElem->getAttribute(name, &v) == S_OK)
		{
			v.ChangeType(VT_R8);
			val = v.dblVal;
			rv = 0;
		}
		else
			val = 0;
	}
#endif
	return rv;
}

int XmlSynthElem::GetAttribute(char *attrName, char **val)
{
	int rv = -1;
	*val = NULL;
#if defined(_WINDOWS)
	if (pElem)
	{
		CComBSTR name(attrName);
		CComVariant v;
		if (pElem->getAttribute(name, &v) == S_OK)
		{
			v.ChangeType(VT_BSTR);
			*val = WideToMulti(v.bstrVal, SysStringLen(v.bstrVal));
			if (*val)
				rv = 0;
		}
	}
#endif
	return rv;
}

int XmlSynthElem::SetAttribute(char *attrName, long val)
{
	int rv = -1;
#if defined(_WINDOWS)
	if (pElem)
	{
		CComBSTR name(attrName);
		CComVariant v(val);
		if (pElem->setAttribute(name, v) == S_OK)
			rv = 0;
	}
#endif
	return rv;
}

int XmlSynthElem::SetAttribute(char *attrName, double val)
{
	int rv = -1;
#if defined(_WINDOWS)
	if (pElem)
	{
		CComBSTR name(attrName);
		CComVariant v(val);
		if (pElem->setAttribute(name, v) == S_OK)
			rv = 0;
	}
#endif
	return rv;
}

int XmlSynthElem::SetAttribute(char *attrName, const char *val)
{
	int rv = -1;
#if defined(_WINDOWS)
	if (pElem)
	{
		CComBSTR name(attrName);
		CComVariant v(val);
		if (pElem->setAttribute(name, v) == S_OK)
			rv = 0;
	}
#endif
	return rv;
}

int XmlSynthElem::TagMatch(const char *tag)
{
	if (nodeTag != NULL)
		return _stricmp(nodeTag, tag) == 0;
	return tag == NULL;
}

int XmlSynthElem::SetContent(const char *data)
{
	int rv = -1;
#if defined(_WINDOWS)
	if (pElem)
	{
		CComBSTR v(data);
		if (pElem->put_text(v) == S_OK)
			rv = 0;
	}
#endif
	return rv;
}

int XmlSynthElem::GetContent(char **data)
{
	int rv = -1;
#if defined(_WINDOWS)
	if (pElem)
	{
		CComBSTR v;
		if (pElem->get_text(&v.m_str) == S_OK)
		{
			*data = WideToMulti(v.m_str, v.Length());
			if (*data)
				rv = 0;
		}
	}
#endif
	return rv;
}

//////////////////////////////////////

XmlSynthDoc::XmlSynthDoc()
{
}

XmlSynthDoc::~XmlSynthDoc()
{
}

#if defined(_WINDOWS)
int XmlSynthDoc::GetXmlDoc()
{
	if (!pDoc)
	{
		HRESULT hr = pDoc.CoCreateInstance(L"MSXML2.DomDocument");
		if (hr != S_OK)
			return -1;
	}

	return 0;
}
#endif

XmlSynthElem *XmlSynthDoc::CreateElement(XmlSynthElem *parent, const char *tag)
{
	XmlSynthElem *newElem = new XmlSynthElem(this);
#if defined(_WINDOWS)
	if (pDoc)
	{
		CComBSTR bsTag(tag);
		CComPtr<IXMLDOMElement> pnew;
		CComPtr<IXMLDOMNode> pout;
		pDoc->createElement(bsTag, &pnew);
		parent->pElem->appendChild(pnew, &pout);
		newElem->pElem = pout;
	}
#endif
	return newElem;
}

XmlSynthElem *XmlSynthDoc::NewDoc(char *roottag)
{
	XmlSynthElem *rootElem = new XmlSynthElem(this);
#if defined(_WINDOWS)
	if (!GetXmlDoc())
	{
		HRESULT hr;
		CComPtr<IXMLDOMNode> pout;
		CComPtr<IXMLDOMProcessingInstruction> pi;
		pDoc->createProcessingInstruction(L"xml", L"version='1.0'", &pi);
		pDoc->appendChild(pi, &pout);
		pout.Release();

		if (roottag == NULL)
			roottag = "synth";

		CComBSTR bsTag(roottag);
		CComPtr<IXMLDOMElement> pnew;
		hr = pDoc->createElement(bsTag, &pnew);
		if (hr == S_OK)
		{
			pDoc->appendChild(pnew, &pout);
			rootElem->pElem = pout;
		}
	}
#endif
	return rootElem;
}

XmlSynthElem *XmlSynthDoc::Open(char *fname)
{
	if (fname == NULL || strlen(fname) == 0)
		return NULL;
	XmlSynthElem *rootElem = new XmlSynthElem(this);
#if defined(_WINDOWS)
	if (!GetXmlDoc())
	{
		HRESULT hr; 
		CComVariant vname(fname);
		VARIANT_BOOL bSucess = 0;
		hr = pDoc->load(vname, &bSucess);
		if (hr == S_OK && bSucess)
		{
			CComPtr<IXMLDOMElement> xmlroot;
			pDoc->get_documentElement(&xmlroot);
			rootElem->SetNode(xmlroot);
		}
	}
#endif
	return rootElem;
}

int XmlSynthDoc::Save(char *fname)
{
	int rv = -1;
#if defined(_WINDOWS)
	if (pDoc)
	{
		CComVariant vdest(fname);
		rv = pDoc->save(vdest) == S_OK ? 0 : -1;
	}
#endif
	return rv;
}

int XmlSynthDoc::Close()
{
#if defined(_WINDWOS)
	pDoc.Relese();
#endif
	return 0;
}
