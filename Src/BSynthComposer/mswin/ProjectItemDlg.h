
template<class PD> class ProjectItemDlg : 
	public PropertyBox
{
protected:
	ProjectItem *pi;

public:
	ProjectItemDlg()
	{
		pi = 0;
	}
	
	virtual ~ProjectItemDlg<PD>()
	{
	}

	virtual int GetFieldID(int id, int& idval)
	{
		idval = -1;
		return 0;
	}

	virtual int GetLabelID(int id, int& idlbl)
	{
		idlbl = -1;
		return 0;
	}

	virtual void EnableValue(int id, int enable)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
			EnableWindow(pt->GetDlgItem(idval), enable);
	}

	virtual void SetCaption(const char *text)
	{
		PD *pt = static_cast<PD*>(this);
		pt->SetWindowText(text);
	}

	virtual void SetState(int id, short onoff)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
			pt->CheckDlgButton(idval, onoff);
	}

	virtual int GetState(int id, short& onoff)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			onoff = pt->IsDlgButtonChecked(idval);
			return 1;
		}
		return 0;
	}

	virtual void SetValue(int id, float val, const char *lbl)
	{
		char txt[40];
		snprintf(txt, 40, "%f", val);
		SetValue(id, txt, lbl);
	}

	virtual void SetValue(int id, long val, const char *lbl)
	{
		char txt[40];
		snprintf(txt, 40, "%d", val);
		SetValue(id, txt, lbl);
	}

	virtual void SetValue(int id, const char *val, const char *lbl)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		int idlbl = -1;
		if (GetFieldID(id, idval))
		{
			pt->SetDlgItemText(idval, val);
			if (lbl)
			{
				if (GetLabelID(id, idval))
					pt->SetDlgItemText(idval, lbl);
			}
		}
	}

	virtual int GetValue(int id, float& val)
	{
		char txt[40];
		if (GetValue(id, txt, 40))
		{
			val = atof(txt);
			return 1;
		}
		return 0;
	}

	virtual int GetValue(int id, long& val)
	{
		char txt[40];
		if (GetValue(id, txt, 40))
		{
			val = atol(txt);
			return 1;
		}
		return 0;
	}

	virtual int GetValue(int id, char *val, int len)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			pt->GetDlgItemText(idval, val, len);
			return 1;
		}
		*val = 0;
		return 0;
	}

	virtual int GetValue(int id, bsString& val)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			int len = ::GetWindowTextLength(pt->GetDlgItem(idval));
			char *buf = new char[len+1];
			pt->GetDlgItemText(idval, buf, len+1);
			val.Attach(buf);
			return 1;
		}
		return 0;
	}

	virtual int GetSelection(int id, short& sel)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			// get item selection from list box
			sel = (short) pt->SendDlgItemMessage(idval, LB_GETCURSEL);
			if (sel != LB_ERR)
				return 1;
		}
		return 0;
	}

	virtual void SetSelection(int id, short sel)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			// set item selection for list box
			pt->SendDlgItemMessage(idval, LB_SETCURSEL, (WPARAM)sel);
		}			
	}

	virtual int GetSelection(int id, void **sel)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		*sel = 0;
		if (GetFieldID(id, idval))
		{
			CListBox lb = pt->GetDlgItem(idval);
			int index = lb.GetCurSel();
			if (index != LB_ERR)
			{
				*sel = lb.GetItemDataPtr(index);
				return 1;
			}
		}
		return 0;
	}

	virtual void SetSelection(int id, void *sel)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			// set item selection for list box
			CListBox lb = pt->GetDlgItem(idval);
			int count = lb.GetCount();
			int index;
			for (index = 0; index < count; index++)
			{
				if (lb.GetItemDataPtr(index) == sel)
				{
					lb.SetCurSel(index);
					break;
				}
			}
		}			
	}

	virtual int ListChildren(int id, ProjectItem *parent)
	{
		PD *pt = static_cast<PD*>(this);
		int count = 0;
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			CListBox list = pt->GetDlgItem(idval);
			if (list.IsWindow())
			{
				ProjectItem *itm = prjTree->FirstChild(parent);
				while (itm)
				{
					int ndx = list.AddString(itm->GetName());
					list.SetItemDataPtr(ndx, itm);
					itm = prjTree->NextSibling(itm);
					count++;
				}
			}
		}
		return count;
	}

	virtual int GetListCount(int id, int& count)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			CListBox list = pt->GetDlgItem(idval);
			if (list.IsWindow())
			{
				count = list.GetCount();
				return 1;
			}
		}
		count = 0;
		return 0;
	}

	virtual ProjectItem *GetListItem(int id, int ndx)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			CListBox list = pt->GetDlgItem(idval);
			if (list.IsWindow())
			{
				if (ndx < list.GetCount())
					return (ProjectItem*)list.GetItemDataPtr(ndx);
			}
		}
		return 0;
	}

	ProjectItem *GetItem()
	{
		return pi;
	}

	void SetItem(ProjectItem *p)
	{
		pi = p;
	}

	virtual int Activate(int modal)
	{
		PD *pt = static_cast<PD*>(this);
		if (modal)
			return pt->DoModal();
		if (pt->Create(::GetActiveWindow()))
			pt->ShowWindow(SW_SHOW);
		return 0;
	}
};