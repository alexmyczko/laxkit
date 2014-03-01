//
//	
//    The Laxkit, a windowing toolkit
//    Please consult http://laxkit.sourceforge.net about where to send any
//    correspondence about this software.
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//    Copyright (C) 2004-2007,2011,2014 by Tom Lechner
//
#ifndef _LAX_FREEHANDINTERFACE_H
#define _LAX_FREEHANDINTERFACE_H

#include <lax/interfaces/aninterface.h>
#include <lax/interfaces/linestyle.h>
#include <lax/interfaces/coordinate.h>

namespace LaxInterfaces { 


enum FreehandEditorStyles {
	FREEHAND_Till_Closed     =(1<<0), //mouse down drag out a line, up and clicking adds points
	FREEHAND_Coordinates     =(1<<1), //Construct Coordinate points
	FREEHAND_Flatpoints      =(1<<2), //Create a list of flatpoints
	FREEHAND_Raw_Path        =(1<<3), //Create a PathsData
	FREEHAND_Poly_Path       =(1<<4), //Create a PathsData
	FREEHAND_Bez_Path        =(1<<5), //Create a PathsData
	FREEHAND_Bez_Outline     =(1<<6), //Create a PathsData
	FREEHAND_Mesh            =(1<<7), //Create a PathsData
	FREEHAND_Notify_All_Moves=(1<<8), //send events to owner upon every move
	FREEHAND_MAX
};

class RawPoint {
  public:
	flatpoint p;
	int flag;
	clock_t time;
	double pressure;
	double tiltx,tilty;
	RawPoint() { time=0; pressure=0; tiltx=tilty=0; flag=0; }
	RawPoint(flatpoint pp) { p=pp; time=0; pressure=0; tiltx=tilty=0; flag=0; }
};

typedef Laxkit::PtrStack<RawPoint> RawPointLine;

class FreehandInterface : public anInterface
{
  protected:
	char showdecs;
	LineStyle linestyle;
	//Laxkit::ShortcutHandler *sc;

	int findLine(int id);

	virtual int send(int i);
	virtual void RecurseReduce(RawPointLine *l, int start, int end, double epsilon);
	virtual void RecurseReducePressure(RawPointLine *l, int start, int end, double epsilon);
	virtual RawPointLine *Reduce(int i, double epsilon);
	virtual RawPointLine *ReducePressure(int i, double epsilon);
	virtual Coordinate *BezApproximate(RawPointLine *l);

  public:
	unsigned int freehand_style;
	double brush_size;
	double smooth_pixel_threshhold;
	Laxkit::ScreenColor linecolor;
	Laxkit::ScreenColor pointcolor;

	Laxkit::PtrStack<RawPointLine> lines;
	Laxkit::NumStack<int> deviceids;

	FreehandInterface(anInterface *nowner, int nid,Laxkit::Displayer *ndp);
	virtual ~FreehandInterface();
	virtual anInterface *duplicate(anInterface *dup);
	virtual const char *IconId() { return "Freehand"; }
	const char *Name();
	const char *whattype() { return "FreehandInterface"; }
	const char *whatdatatype() { return NULL; } // is creation only
	Laxkit::MenuInfo *ContextMenu(int x,int y,int deviceid);
	//virtual Laxkit::ShortcutHandler *GetShortcuts();
	//virtual int PerformAction(int action);

	virtual int UseThis(Laxkit::anObject *nlinestyle,unsigned int mask=0);
	virtual int InterfaceOn();
	virtual int InterfaceOff();
	virtual void Clear(SomeData *d);
	virtual int Refresh();
	virtual int MouseMove(int x,int y,unsigned int state, const Laxkit::LaxMouse *d);
	virtual int LBDown(int x,int y,unsigned int state,int count, const Laxkit::LaxMouse *d);
	virtual int LBUp(int x,int y,unsigned int state, const Laxkit::LaxMouse *d);
	//virtual int WheelUp  (int x,int y,unsigned int state,int count, const Laxkit::LaxMouse *d);
	//virtual int WheelDown(int x,int y,unsigned int state,int count, const Laxkit::LaxMouse *d);
	//virtual int CharInput(unsigned int ch, const char *buffer,int len,unsigned int state, const Laxkit::LaxKeyboard *d);
	//virtual int KeyUp(unsigned int ch,unsigned int state, const Laxkit::LaxKeyboard *d);

};

} // namespace LaxInterfaces

#endif

