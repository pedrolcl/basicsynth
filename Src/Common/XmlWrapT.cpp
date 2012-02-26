//////////////////////////////////////////////////////////////////
/// @file XmlWrapT.cpp TinyXML wrapper
//
// BasicSynth
//
// TinyXML XmlWrapper - see: www.sourceforge.net/projects/tinyxml
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////
#include <XmlWrap.h>
#if defined(USE_TINYXML)
#include <stdlib.h>
#include <math.h>
#include <string.h>
#if _WIN32 && _MSC_VER
#define snprintf _snprintf
#endif


XmlSynthElem::XmlSynthElem(XmlSynthDoc *p)
{
	doc = p;
	pElem = 0;
}

XmlSynthElem::~XmlSynthElem()
{
}

void XmlSynthElem::Clear()
{
	pElem = 0;
}

XmlSynthElem *XmlSynthElem::FirstChild()
{
	if (pElem)
	{
		XmlSynthElem *ret = new XmlSynthElem(doc);
		if (FirstChild(ret))
			return ret;
		delete ret;
	}
	return 0;
}

XmlSynthElem *XmlSynthElem::FirstChild(XmlSynthElem *ret)
{
	if (pElem)
	{
		const TiXmlElement *n = pElem->FirstChildElement();
		if (n)
		{
			ret->doc = doc;
			ret->SetNode((TiXmlElement *)n);
			return ret;
		}
	}
	ret->SetNode(0);
	return 0;
}

XmlSynthElem *XmlSynthElem::NextSibling()
{
	if (pElem)
	{
		XmlSynthElem *ret = new XmlSynthElem(doc);
		if (NextSibling(ret))
			return ret;
		delete ret;
	}
	return 0;
}

XmlSynthElem *XmlSynthElem::NextSibling(XmlSynthElem *ret)
{
	if (pElem)
	{
		const TiXmlElement *n = pElem->NextSiblingElement();
		if (n)
		{
			ret->doc = doc;
			ret->SetNode((TiXmlElement*)n);
			return ret;
		}
	}
	ret->SetNode(0);
	return 0;
}

XmlSynthElem *XmlSynthElem::AddChild(const char *childTag)
{
	if (pElem)
	{
		XmlSynthElem *ret = new XmlSynthElem(doc);
		if (AddChild(childTag, ret))
			return ret;
		delete ret;
	}
	return 0;
}

XmlSynthElem *XmlSynthElem::AddChild(const char *childTag, XmlSynthElem *ret)
{
	if (pElem && childTag)
	{
		TiXmlElement *n = new TiXmlElement(childTag);
		if (n)
		{
			pElem->LinkEndChild(n);
			ret->doc = doc;
			ret->SetNode(n);
			return ret;
		}
	}
	ret->SetNode(0);
	return 0;
}

const char *XmlSynthElem::TagName()
{
	if (pElem)
		return pElem->Value();
	return "";
}

int XmlSynthElem::GetAttribute(const char *attrName, short& val)
{
	if (pElem)
	{
		const char *str = pElem->Attribute(attrName);
		if (str)
		{
			val = atoi(str);
			return 0;
		}
	}
	val = 0;
	return -1;
}

int XmlSynthElem::GetAttribute(const char *attrName, long& val)
{
	if (pElem)
	{
		const char *str = pElem->Attribute(attrName);
		if (str)
		{
			val = atol(str);
			return 0;
		}
	}
	val = 0;
	return -1;
}

int XmlSynthElem::GetAttribute(const char *attrName, float& val)
{
	if (pElem)
	{
		const char *str = pElem->Attribute(attrName);
		if (str)
		{
			val = (float) atof(str);
			return 0;
		}
	}

	val = 0.0;
	return -1;
}

int XmlSynthElem::GetAttribute(const char *attrName, double& val)
{
	if (pElem)
	{
		const char *str = pElem->Attribute(attrName);
		if (str)
		{
			val = atof(str);
			return 0;
		}
	}
	val = 0.0;
	return -1;
}

int XmlSynthElem::GetAttribute(const char *attrName, char **val)
{
	if (pElem)
	{
		const char *s = pElem->Attribute(attrName);
		if (s)
		{
			size_t len = strlen(s)+1;
			*val = new char[len];
			if (*val)
			{
				strcpy(*val, s);
				return 0;
			}
		}
	}
	*val = NULL;
	return -1;
}

int XmlSynthElem::SetAttribute(const char *attrName, short val)
{
	if (pElem)
	{
		pElem->SetAttribute(attrName, (int) val);
		return 0;
	}
	return -1;
}

int XmlSynthElem::SetAttribute(const char *attrName, long val)
{
	if (pElem)
	{
		pElem->SetAttribute(attrName, (int) val);
		return 0;
	}
	return -1;
}

int XmlSynthElem::SetAttribute(const char *attrName, float val)
{
	return SetAttribute(attrName, (double) val);
}

int XmlSynthElem::SetAttribute(const char *attrName, double val)
{
	if (pElem)
	{
		char strval[40];
		if (snprintf(strval, 40, "%g", val) >= 0)
		{
			pElem->SetAttribute(attrName, strval);
			return 0;
		}
	}
	return -1;
}

int XmlSynthElem::SetAttribute(const char *attrName, const char *val)
{
	if (pElem)
	{
		pElem->SetAttribute(attrName, val ? val : "");
		return 0;
	}
	return -1;
}

int XmlSynthElem::TagMatch(const char *tag)
{
	if (pElem)
	{
		if (strcmp(pElem->Value(), tag) == 0)
			return 1;
	}
	return 0;
}

int XmlSynthElem::SetContent(const char *data)
{
	if (pElem)
	{
		TiXmlText *t = new TiXmlText(data ? data : "");
		if (t)
		{
			pElem->LinkEndChild(t);
			return 0;
		}
	}
	return -1;
}

int XmlSynthElem::GetContent(char **data)
{
	if (pElem)
	{
		const TiXmlNode *n = pElem->FirstChild();
		const TiXmlText *t;
		while (n)
		{
			if ((t = n->ToText()))
			{
				const char *s = t->Value();
				if (s)
				{
					*data = new char[strlen(s)+1];
					strcpy(*data, s);
					return 0;
				}
			}
			n = n->NextSibling();
		}
	}
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
	return parent->AddChild(tag);
}

XmlSynthElem *XmlSynthDoc::NewDoc(const char *roottag)
{
	return NewDoc(roottag, 0);
}

XmlSynthElem *XmlSynthDoc::NewDoc(const char *roottag, XmlSynthElem *root)
{
	doc = new TiXmlDocument;
	if (doc)
	{
		if (root == 0)
			root = new XmlSynthElem(this);
		else
			root->doc = this;
		if (root)
		{
			TiXmlDeclaration *d = new TiXmlDeclaration("1.0","UTF-8","yes");
			if (d)
				doc->LinkEndChild(d);
			TiXmlElement *e = new TiXmlElement(roottag ? roottag : "synth");
			if (e)
			{
				doc->LinkEndChild(e);
				root->SetNode(e);
				return root;
			}
		}
	}
	return 0;
}

XmlSynthElem *XmlSynthDoc::Open(const char *fname)
{
	return Open(fname, 0);
}

XmlSynthElem *XmlSynthDoc::Open(const char *fname, XmlSynthElem* root)
{
	doc = new TiXmlDocument;
	doc->SetTabSize(0);
	if (fname && doc->LoadFile(fname))
	{
		if (root == 0)
			root = new XmlSynthElem(this);
		else
			root->doc = this;
		root->SetNode(doc->RootElement());
		return root;
	}
	return 0;
}

int XmlSynthDoc::Save(const char *fname)
{
	if (doc && fname)
	{
		if (doc->SaveFile(fname))
			return 0;
	}
	return -1;
}

int XmlSynthDoc::Close()
{
	delete doc;
	doc = 0;
	return 0;
}

#else
int _not_using_tiny_xml = 1;
#endif
