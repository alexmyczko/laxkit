//
//	
//    The Laxkit, a windowing toolkit
//    Please consult https://github.com/Laidout/laxkit about where to send any
//    correspondence about this software.
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; If not, see <http://www.gnu.org/licenses/>.
//
//    Copyright (C) 2004-2010 by Tom Lechner
//
#ifndef _LAX_FUNCFRAME_H
#define _LAX_FUNCFRAME_H


#include <lax/anxapp.h>

namespace Laxkit {


class FuncFrame : public anXWindow
{
 protected:
	int arrangedstate;
	struct WinDef {
		anXWindow *win;
		char *x,*y,*w,*h;
		WinDef(anXWindow *nwin,const char *xx,const char *yy,const char *ww,const char *hh);
		~WinDef() { if (x) delete[] x; if (y) delete[] y; if (w) delete[] w; if (h) delete[] h; }
	};
	PtrStack<WinDef> windefs;
 public:
	char **vars;
	int *varval;
	int nvars;
	FuncFrame(anXWindow *parnt,const char *nname,const char *ntitle,unsigned long nstyle,
								int xx,int yy,int ww,int hh,int brder,
								anXWindow *prev,unsigned long owner,const char *mes);
	virtual ~FuncFrame();
	virtual const char *whattype() { return "FuncFrame"; }
	virtual void Refresh();
	virtual void SyncWin(int addwindow=0);
	virtual int getval(const char *def);
	virtual void AddVar(const char *var,int val);
	virtual void AddWin(anXWindow *win,const char *x,const char *y,const char *w,const char *h);
	virtual int Resize(int nw,int nh);
	virtual int MoveResize(int nx,int ny,int nw,int nh);
};

} // namespace Laxkit

#endif

