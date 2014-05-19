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
//    Copyright (C) 2014 by Tom Lechner
//
#ifndef _LAX_ENGRAVERFILLINTERFACE_H
#define _LAX_ENGRAVERFILLINTERFACE_H

#include <lax/interfaces/patchinterface.h>
#include <lax/interfaces/pathinterface.h>
#include <lax/interfaces/imageinterface.h>
#include <lax/interfaces/curvemapinterface.h>
#include <lax/screencolor.h>
#include <lax/curvewindow.h>

//#include <lax/interfaces/selection.h>

namespace LaxInterfaces {


//------------------------------------- EngraverFillData ------------------------
class LinePoint
{
  public:
	double s,t;
	int row,col;
	double weight;
	double spacing; //visual measure, to be used when remapping
	int group;
	bool on;

	int needtosync; //0 no, 1: s,t -> p, 2: p->s,t
	flatpoint p; //(s,t) transformed by the mesh

	LinePoint *next, *prev;

	LinePoint() { on=true; row=col=0; s=t=0; weight=1; spacing=-1; next=prev=NULL; needtosync=1; group=0; }
	LinePoint(double ss, double tt, double ww, int ngroup=0) { on=true; next=prev=NULL; s=ss; t=tt; weight=ww; spacing=-1; group=ngroup; }

	void Set(double ss,double tt, double nweight) { s=ss; t=tt; if (nweight>=0) weight=nweight; needtosync=1; }
	void Set(LinePoint *pp);
	void Clear();
	void Add(LinePoint *np);
};

class EngraverFillData : public PatchData
{
 protected:
  	
 public:
	Laxkit::PtrStack<LinePoint> lines;
	flatvector direction;
	int nlines;
	FillStyle fillstyle;

	double default_spacing;
	double zero_threshhold; //weight<this are considered off
	double broken_threshhold; //if nonzero, zero_threshhold<weight<this means use broken line of this thickness

	EngraverFillData();
	//EngraverFillData(double xx,double yy,double ww,double hh,int nr,int nc,unsigned int stle);
	virtual ~EngraverFillData(); 
	virtual const char *whattype() { return "EngraverFillData"; }
	virtual SomeData *duplicate(SomeData *dup);

	virtual void dump_out(FILE *f,int indent,int what,Laxkit::anObject *context);
	virtual void dump_in_atts(LaxFiles::Attribute *att,int flag,Laxkit::anObject *context);
	virtual void dump_out_svg(const char *file);
	virtual PathsData *MakePathsData();

	virtual void Set(double xx,double yy,double ww,double hh,int nr,int nc,unsigned int stle);
	//virtual unsigned long WhatColorLong(double s,double t);
	//virtual int WhatColor(double s,double t,Laxkit::ScreenColor *color_ret);
	//virtual int hasColorData();

	//virtual void zap();

	virtual int PointOn(LinePoint *p);
	virtual void FillRegularLines(double weight, double spacing);
	virtual void Sync();
	virtual void ReverseSync(bool asneeded);
	virtual void BezApproximate(Laxkit::NumStack<flatvector> &fauxpoints, Laxkit::NumStack<flatvector> &points);
	virtual void MorePoints();
};


//------------------------------ EngraverFillInterface -------------------------------

class EngraverTraceSettings
{
  public:
	int group;
	Laxkit::CurveInfo value_to_weight;
	double traceobj_opacity;
	bool continuous_trace;

	ObjectContext *tracecontext;
	SomeData *traceobject;
	char *identifier;
	unsigned char *trace_sample_cache;
	int samplew, sampleh;

	 //black and white cache:
	int tw,th; //dims of trace_ref_bw
	unsigned char *trace_ref_bw;

	EngraverTraceSettings();
	~EngraverTraceSettings();
	void ClearCache(bool obj_too);
};

class EngraverPointGroup
{
  public:
	enum PointGroupType {
		PGROUP_Linear,
		PGROUP_Radial,
		PGROUP_Spiral,
		PGROUP_Circular,
		PGROUP_MAX
	};

	int id; //the group number in LinePoint
	char *name;
	EngraverTraceSettings trace;

	int type; //what manner of lines
	double type_d;   //parameter for type, for instance, an angle for spirals
	double spacing;
	flatpoint position,direction;

	double dash_length;
	double zero_threshhold;
	double broken_threshhold;

	EngraverPointGroup();
	EngraverPointGroup(int nid,const char *nname, int ntype, flatpoint npos, flatpoint ndir, double ntype_d);
	virtual ~EngraverPointGroup();
	virtual flatpoint Direction(double s,double t);
	virtual LinePoint *LineFrom(double s,double t);

	virtual void Fill(EngraverFillData *data); //fill in x,y = 0..1,0..1
	virtual void FillRegularLines(EngraverFillData *data);
};

class EngraverFillInterface : public PatchInterface
{
 protected:
	Laxkit::MenuInfo modes;
	EngraverFillData *edata;
	int mode;
	int controlmode;
	int submode;
	int show_points;

	 //general tool settings
	double brush_radius; //screen pixels
	Laxkit::CurveInfo thickness; //ramp of thickness brush
	double default_spacing;
	double default_zero_threshhold; //weight<this are considered off
	double default_broken_threshhold; //if nonzero, zero_threshhold<weight<this means use broken line of this thickness

	 //trace settings..
	bool show_trace;
	bool continuous_trace;
	Laxkit::CurveInfo tracemap;
	Laxkit::DoubleBBox tracebox;
	EngraverTraceSettings trace;

	 //default orientation
	flatpoint orient_direction;
	flatpoint orient_position;

	Laxkit::ScreenColor fgcolor,bgcolor;

	int lasthover;
	flatpoint hover;
	//Selection *selection;

	
	CurveMapInterface curvemapi;

	virtual void ChangeMessage(int forwhich);
	virtual int scanEngraving(int x,int y, int *category);
	virtual int PerformAction(int action);
 public:
	EngraverFillInterface(int nid, Laxkit::Displayer *ndp);
	virtual ~EngraverFillInterface();
	virtual Laxkit::ShortcutHandler *GetShortcuts();
	virtual const char *IconId() { return "Engraver"; }
	virtual const char *Name();
	virtual const char *whattype() { return "EngraverFillInterface"; }
	virtual const char *whatdatatype() { return "EngraverFillData"; }
	virtual anInterface *duplicate(anInterface *dup);
	virtual int UseThisObject(ObjectContext *oc);
	virtual int UseThis(anObject *newdata,unsigned int mask=0);
	virtual int UseThis(int id,int ndata);
	virtual int DrawData(anObject *ndata,anObject *a1=NULL,anObject *a2=NULL,int info=0);
	virtual int LBDown(int x,int y,unsigned int state,int count,const Laxkit::LaxMouse *d);
	virtual int LBUp(int x,int y,unsigned int state,const Laxkit::LaxMouse *d);
	virtual int MouseMove(int x,int y,unsigned int state,const Laxkit::LaxMouse *d);
	virtual int CharInput(unsigned int ch,const char *buffer,int len,unsigned int state,const Laxkit::LaxKeyboard *d);
	virtual int KeyUp(unsigned int ch,unsigned int state,const Laxkit::LaxKeyboard *d);
	virtual int Refresh();
	virtual int Event(const Laxkit::EventData *data, const char *mes);
	virtual Laxkit::MenuInfo *ContextMenu(int x,int y,int deviceid);

	virtual void DrawTracingTools();
	virtual void DrawLineGradient(double minx,double maxx,double miny,double maxy);
	virtual void DrawShadeGradient(double minx,double maxx,double miny,double maxy);
	virtual void deletedata();
	virtual PatchData *newPatchData(double xx,double yy,double ww,double hh,int nr,int nc,unsigned int stle);
	//virtual void drawpatch(int roff,int coff);
	//virtual void patchpoint(PatchRenderContext *context, double s0,double ds,double t0,double dt,int n);
	virtual int ChangeMode(int newmode);
	virtual int Trace();
};

} //namespace LaxInterfaces

#endif


