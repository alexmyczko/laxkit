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
//    Copyright (C) 2010 by Tom Lechner
//
#ifndef _LAX_INTRECTANGLE_H
#define _LAX_INTRECTANGLE_H


namespace Laxkit {

/*! \class IntRectangle */
class IntRectangle
{
 public:
	int x,y;
	int width,height;

	IntRectangle() { x=y=width=height=0; }
	IntRectangle(int nx,int ny,int nw,int nh) { x=nx; y=ny; width=nw; height=nh; }

	void set(int nx,int ny,int nw,int nh) { x=nx; y=ny; width=nw; height=nh; }
	int pointIsIn(int xx,int yy) { return xx>=x && xx<x+width && yy>=y && yy<y+height; }
};

/*! \class DoubleRectangle */
class DoubleRectangle
{
 public:
	double x,y;
	double width,height;

	DoubleRectangle() { x=y=width=height=0; }
	DoubleRectangle(double nx,double ny,double nw,double nh) { x=nx; y=ny; width=nw; height=nh; }

	void set(double nx,double ny,double nw,double nh) { x=nx; y=ny; width=nw; height=nh; }
	int pointIsIn(double xx,double yy) { return xx>=x && xx<x+width && yy>=y && yy<y+height; }
};

} //namespace Laxkit

#endif

