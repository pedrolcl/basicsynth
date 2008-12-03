//////////////////////////////////////////////////////////////////
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

int XmlSynthElem::GetAttribute(char *attrName, short& val)
{
	if (pElem)
	{
		int v;
		if (pElem->QueryIntAttribute(attrName, &v) == TIXML_SUCCESS)
		{
			val = (short) v;
			return 0;
		}
	}
	val = 0;
	return -1;
}

int XmlSynthElem::GetAttribute(char *attrName, long& val)
{
	if (pElem)
	{
		int v;
		if (pElem->QueryIntAttribute(attrName, &v) == TIXML_SUCCESS)
		{
			val = (long) v;
			return 0;
		}
	}
	val = 0;
	return -1;
}

int XmlSynthElem::GetAttribute(char *attrName, float& val)
{
	if (pElem)
	{
		if (pElem->QueryFloatAttribute(attrName, &val) == TIXML_SUCCESS)
			return 0;
	}

	val = 0.0;
	return -1;
}

int XmlSynthElem::GetAttribute(char *attrName, double& val)
{
	if (pElem)
	{
		if (pElem->QueryDoubleAttribute(attrName, &val) == TIXML_SUCCESS)
			return 0;
	}
	val = 0.0;
	return -1;
}

int XmlSynthElem::GetAttribute(char *attrName, char **val)
{
	if (pElem)
	{
		const char *s = pElem->Attribute(attrName);
		if (s)
		{
			*val = new char[strlen(s)+1];
			strcpy(*val, s);
			return 0;
		}
	}
	*val = NULL;
	return -1;
}

int XmlSynthElem::SetAttribute(char *attrName, short val)
{
	if (pElem)
	{
		pElem->SetAttribute(attrName, (int) val);
		return 0;
	}
	return -1;
}

int XmlSynthElem::SetAttribute(char *attrName, long val)
{
	if (pElem)
	{
		pElem->SetAttribute(attrName, (int) val);
		return 0;
	}
	return -1;
}

int XmlSynthElem::SetAttribute(char *attrName, float val)
{
	if (pElem)
	{
		pElem->SetDoubleAttribute(attrName, (double) val);
		return 0;
	}
	return -1;
}

int XmlSynthElem::SetAttribute(char *attrName, double val)
{
	if (pElem)
	{
		pElem->SetDoubleAttribute(attrName, val);
		return 0;
	}
	return -1;
}

int XmlSynthElem::SetAttribute(char *attrName, const char *val)
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
			XmlSynthElem *ret = new XmlSynthElem;
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

XmlSynthElem *XmlSynthDoc::NewDoc(char *roottag)
{
	return NewDoc(roottag, 0);
}

XmlSynthElem *XmlSynthDoc::NewDoc(char *roottag, XmlSynthElem *root)
{
	doc = new TiXmlDocument;
	if (doc)
	{
		if (root == 0)
			root = new XmlSynthElem(this);
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

XmlSynthElem *XmlSynthDoc::Open(char *fname)
{
	return Open(fname, 0);
}

XmlSynthElem *XmlSynthDoc::Open(char *fname, XmlSynthElem* root)
{
	doc = new TiXmlDocument;
	doc->SetTabSize(0);
	if (fname && doc->LoadFile(fname))
	{
		if (root == 0)
			root = new XmlSynthElem(this);
		root->SetNode(doc->RootElement());
		return root;
	}
	return 0;
}

int XmlSynthDoc::Save(char *fname)
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
