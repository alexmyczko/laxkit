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
//    Copyright (C) 2004-2007,2012,2015 by Tom Lechner
//

#include <lax/anobject.h>
#include <lax/misc.h>
#include <lax/strmanip.h>

#include <iostream>
using namespace std;
#define DBG 

namespace Laxkit {

//-------------------------- anObject ----------------------------
/*! \class anObject
 * \brief Base class for all Laxkit objects
 * 
 * In the constructor, object_id gets assinged a number returned from getUniqueNumber().
 *
 * Also provides inc_count() and dec_count() for reference counting.
 * Objects are created with a count of 1.
 */
	
DBG static int numofanObject=0;
DBG unsigned int CHECK=881;

//! Set object_id=getUniqueNumber().
anObject::anObject()
{
	suppress_debug=0;
	DBG numofanObject++;
	
	object_id=getUniqueNumber(); 
	object_idstr=NULL;
	DBG cerr <<"anObject tracker "<<object_id<<"   created    num of anObjects: "<<numofanObject<<endl;

	DBG if (object_id==CHECK) {
	DBG 	cerr <<" Agh!"<<endl;
	DBG }

	_count=1; 
}

/*! \fn const char *anObject::whattype()
 * \brief Returns the type of anObject.
 *
 * Usually, this corresponds to the C++ class name. However, it is also intended
 * to be used to say what the object should be considered to be. For instance, if you
 * subclass ImagePatchInterface to behave better, the program should still think of it
 * as an ImagePatchInterface, which is something the C++ typeinfo features are not
 * really designed for.
 */


//! Empty virtual destructor.
anObject::~anObject()
{
	DBG numofanObject--;
	DBG cerr <<"anObject tracker "<<object_id<<"   destroyed "<<(object_idstr?object_idstr:"(?)")<<" num of anObjects: "<<numofanObject<<endl;

	if (object_idstr) delete[] object_idstr;
}

/*! \fn anObject *anObject::ObjectOwner()
 *
 * By default, NULL is returned. This can be used, for instance, by ResourceManager or by
 * some interface data to determine if this object is a shared resource, or is directly owned
 * by someone.
 */


/*! Called whenever the object contents is changed, usually calls for screen refresh.
 *
 * Default doesn't actually do anything. See LaxInterfaces::SomeData for more useful stuff.
 */
void anObject::touchContents()
{ 
	//previewtime=0; //time() doesn't change often enough, so we have to force this to 0..
	//modtime=time(NULL); 
}



//---------------- reference counting stuff


/*! \var int anObject::_count
 * \brief The reference count of the object.
 *
 * Controlled with inc_count() and dec_count(). When the count
 * is less or equal to 0, the object is deleted. See dec_count() for more.
 */

	

/*! \fn int anObject::inc_count()
 * \brief Increment the data's count by 1. Returns count.
 */
int anObject::inc_count()
{
	_count++;

	DBG if (object_id==CHECK) {
	DBG 	cerr <<" Agh!"<<endl;
	DBG }
	DBG if (!suppress_debug) {
	DBG   cerr <<"refcounted anobject inc count, now: "<<_count<<endl;
	DBG   cerr<<whattype()<<" "<<object_id<<" inc counted: "<<_count<<"  "<<(object_idstr?object_idstr:"(?)")<<endl;
	DBG }
	return _count; 
}

//! Decrement the count of the data, deleting if count is less than or equal to 0.
/*! If count gets decremented to 0, then call "delete this".
 *
 * Returns the count. If 0 is returned, the item is gone, and should
 * not be accessed any more.
 */
int anObject::dec_count()
{
	_count--;
	DBG if (!suppress_debug) {
	DBG   cerr <<"refcounted anobject dec count, now: "<<_count<<(_count==0?", deleting":"")<<endl;
	DBG   cerr<<(whattype() ? whattype() : "(no whattype)")<<" "<<object_id<<" dec counted: "<<_count<<"  "<<(object_idstr?object_idstr:"(?)")<<endl;
	DBG }
	DBG if (object_id==CHECK) {
	DBG 	cerr <<" Agh!"<<endl;
	DBG }

	if (_count<=0) {
		int yesdelete=1;
		int c=_count;
		if (yesdelete) delete this;
		return c;
	}
	return _count; 
}

/*! Return the id of the object. If NULL, then create a default one and return that.
 */
const char *anObject::Id()
{
	if (object_idstr) return object_idstr;
	else object_idstr=make_id(whattype());
	return object_idstr;
}

/*! Warning! Passing in NULL will make object_idstr==NULL!
 */
const char *anObject::Id(const char *newid)
{
	makestr(object_idstr,newid);
	return object_idstr;
}


} // namespace Laxkit

