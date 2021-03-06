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
//    Copyright (C) 2004-2007,2010 by Tom Lechner
//


#include <lax/language.h>
#include <lax/filedialog.h>
#include <lax/filemenuinfo.h>
#include <lax/popupmenu.h>
#include <lax/filepreviewer.h>
#include <lax/inputdialog.h>
#include <lax/menubutton.h>
#include <lax/freedesktop.h>
#include <lax/overwrite.h>

#include <lax/fileutils.h>
using namespace LaxFiles;

#include <unistd.h>
#include <dirent.h>
#include <glob.h>

#include <iostream>
using namespace std;
#define DBG 


//Minimal File Selection Dialog
//
//[bm1] [bm2] [...]
//
//Path:  /stuff/blah/[previous/other/dirs]   <-- assumes current directory if blank
//File?  blah.cc   <-- cntl-enter adds (File) to path, else selects file
//Mask: *.cc *.h (3)    <-- says number of matches
//
// list of files....   |  preview
// ....                |
//
//[bookmark1][2][3][...]
//[Bookmark current][New dir][up][refresh][details][back][forward]
//
//[Ok] [Cancel]
//
//up/down selects path or file??? or does the control next/prev
//Perhaps have a separate SetPreferences mode??
//  sort abc
//  sort 123
//  sort by mod time, access time, size, type
//  - ascending
//  - dirs first
//  - ignore case
//  - sort by extension
//  - follow links
//
//
//
//
//New Bookmark
//------------
//Icon []
//Path _/blah/blah___
//Mask __*.cc_*.h__
//File yoda.cc
// * Session only
// * Program only
// * Global
// * Embed Icon
//
//
//
//New Dir (simple LineInput)
//-------
//Name? ________



namespace Laxkit {


//-------------------------------- FileDialog ------------------------------

/*! \class FileDialog
 * \brief A dialog for selecting files.
 *
 * \todo FILES_OPEN_ONE still allows multiple to be selected..
 * \todo *** progressive update when typing file name in file slot. if file is not part of
 *   currently selected files, then reset filelist selection. each filelist file selection
 *   should update the file slot
 * \todo ***need a special path box to allow easy sanity checking... maybe not even allow
 *   editing in path or mask, only in file....or only file and mask
 *
 * A dialog to open one or more files, save files, or select directories.
 * If FILES_OPEN_ONE, only one file may be opened. When a file is selected,
 * then a StrEventData with that file in it is sent to the dialog's owner.
 * If FILES_OPEN_MANY, then more than one file may be opened at one time, and
 * a StrsEventData is used instead, even if there is only 1 file selected.
 * 
 * \code
 * #define FILES_MINIMAL          (1<<16)
 * #define FILES_NO_BOOKMARKS     (1<<17)
 * #define FILES_DETAILS          (1<<18)
 * #define FILES_NO_FOLLOW_LINKS  (1<<19)
 * 
 *  // do not allow opening directories. They must be regular files.***imp me!
 * #define FILES_FILES_ONLY       (1<<20)
 * 
 *  // NEW is really same as OPEN_ONE, except file must not already exist?
 *  // SELECT_DIR is same as OPEN_ONE, but the selected thing must be a directory, 
 *  // 	or if none selected, then it is the current directory
 *  // OPEN allows multiple files (not directories) to be selected and returned,
 *  // 	and they don't have to all be in the same directory (be careful when selecting!!)
 * #define FILES_FROM_ONE_DIR     (1<<21)
 * #define FILES_NEW              (1<<22)
 * #define FILES_SELECT_DIR       (1<<23
 * #define FILES_OPEN_ONE         (1<<24)
 * #define FILES_OPEN_MANY        (1<<25)
 *  
 *  // SAVE and SAVE_AS allow the same things, but "Save" or "Save As" is a label somewhere
 * #define FILES_SAVE             (1<<26)
 * #define FILES_SAVE_AS          (1<<27)
 * #define FILES_ASK_TO_OVERWRITE (1<<28)
 *
 *  // Have the preview alongside the list of files.
 * #define FILES_PREVIEW          (1<<29)
 * 
 * #define FILES_NO_CANCEL        (1<<31) <-- do not app->destroy the window internally
 * \endcode
 */
/*! \var unsigned long FileDialog::dialog_style
 * \brief Style flags for the dialog.
 *  
 *  This by default has FILES_GLOBAL_BOOKMARK set in it. This allows saving bookmarks
 *  globally for the user.
 */


//! Constructor.
/*! If nrecentgroup!=NULL, then use recent files. If nrecentgroup=="" then any group will be used.
 * Otherwise, only recent items in ~/.recently-used with that group will show up in the Recent
 * button.
 *  
 * Note that the previewer is not created, and the windows other than file, path and mask
 * are not created until init(). All initial window layout is done in init().
 *
 * \todo *** need to work out styles, and passing to RowFrame..
 * \todo include prev?
 */
FileDialog::FileDialog(anXWindow *parnt,const char *nname,const char *ntitle,unsigned long nstyle,
		int xx,int yy,int ww,int hh,int brder,
		unsigned long nowner,const char *nsend,
		unsigned long ndstyle,
		//anXWindow *prev,unsigned long nowner,const char *nsend,int nid,
		const char *nfile,const char *npath,const char *nmask,
		const char *nrecentgroup)
	: RowFrame(parnt,nname,ntitle,(nstyle&0xffff)|ROWFRAME_ROWS|ROWFRAME_CENTER,
			   xx,yy,ww,hh,brder,
			   NULL,nowner,nsend,
			   5)
{
	dialog_style= ndstyle | FILES_GLOBAL_BOOKMARK;
	
	finalbuttons=-1;
	recentgroup=newstr(nrecentgroup);
	previewer=NULL;


	 //-------the rest of this function is building initial file/path/mask/ok controls
	anXWindow *last=NULL;

	 //create file input
	last=file=new LineInput(this,"file",NULL,0, 0,0,0,0, 1, NULL,object_id,"new file", _("File?"),lax_basename(nfile),0, 0,0,2,2,2,2);
	file->tooltip(_("Filename to use"));

	 //set up path input
	char *nnpath=NULL;
	if (npath) {
		if (file_exists(npath,1,NULL)!=S_IFDIR) {
			char *t=lax_dirname(npath,0);
			if (file_exists(t,1,NULL)!=S_IFDIR) npath=NULL;
			else { nnpath=t; t=NULL; }
			if (t) delete[] t;
		}
	}
	if (!nnpath && !npath) {
		if (dialog_style&FILES_OPENING) npath=app->load_dir;
		else npath=app->save_dir;
	}
	if (!nnpath && (!npath || npath[0]=='\0')) {
		 //char *t=get_current_dir_name(); // this is a gnu extension _GNU_SOURCE must have been defined somewhere..
		 char *t=getcwd(NULL,0); // this is a gnu extension _GNU_SOURCE must have been defined somewhere..
		 nnpath=newstr(t);
		 free(t);
	}
	if (!nnpath && npath) nnpath=newstr(npath);
	DBG cerr <<"--------->  nnpath:"<<(nnpath?nnpath:"null")<<"  npath:"<<(npath?npath:"null")<<endl;
	last=path=new LineInput(this,"path",NULL,0, 0,0,0,0, 1, last,object_id,"new path", _("Path:"),nnpath,0, 0,0,2,2,2,2);
	path->tooltip(_("Current Directory"));
	 
	 //set up mask input
	last=mask=new LineInput(this,"mask",NULL,0, 0,0,0,0, 1, last,object_id,"new mask", _("Mask:"),nmask?nmask:"*",0, 0,0,2,2,2,2);
	//mask->tooltip(_("Space separated list of\nmasks to apply to the path"));
	mask->tooltip(_("Mask to apply to the end of path"));
	
	app->addwindow(file,0,0);
	app->addwindow(path,0,0);
	app->addwindow(mask,0,0); //though these are addwindow'd, they are AddWin'd in init.

	filelist=NULL;
	getDirectory(nnpath);
	if (nnpath) delete[] nnpath;

	 //add an ok and optionally a cancel button to wholelist here, rather than in init()
	int textheight=app->defaultlaxfont->textheight();
	int linpheight=textheight+6;

	unsigned long okbutton=BUTTON_OK;
	if (dialog_style&(FILES_SAVE|FILES_SAVE_AS)) okbutton=BUTTON_SAVE;
	else if (dialog_style&(FILES_OPEN_MANY|FILES_OPEN_ONE)) okbutton=BUTTON_OPEN;
	ok=new Button(this,"fd-Ok",NULL,okbutton, 0,0,0,0, 1, 
			NULL,object_id,"ok",
			0, NULL,NULL,NULL,3,3);
	AddWin(ok,1, ok->win_w,0,50,50,0, linpheight,0,0,50,0, -1);
	finalbuttons=wholelist.n-1;

	if (!(dialog_style&FILES_NO_CANCEL)) {
		last=new Button(this,"fd-cancel",NULL,BUTTON_CANCEL, 0,0,0,0, 1, 
								 NULL,object_id,"cancel", 0,NULL,NULL,NULL,3,3);
		AddWin(last,1, last->win_w,0,50,50,0, linpheight,0,0,50,0, -1);
	}

}

FileDialog::~FileDialog()
{
	if (recentgroup) delete[] recentgroup;
}

/*! Unimplemented.
 *
 * \todo ****not necessary? ***would put a NULL terminated char ** list of files in state
 */
int FileDialog::GetState(void **state)
{ return 0; }

//! Use this group for recent files.
/*! If group==NULL, then do not use the recent file button. If group=="", then use any group.
 */
void FileDialog::Recent(const char *group)
{
	makestr(recentgroup,group);
}
	
//! Fill the files MenuInfo with all the matching files
/*!
 * \todo ****** wrap this stuff into a FileMenuInfo!!!!!??
 * 		how to make the FileMenuItem have path/mask
 * 		*** and don't forget about scandir!!
 * \todo *** should return 0 success, nonzero error bad path, etc.
 * \todo *** this needs much work to use multiple masks...
 */
int FileDialog::getDirectory(const char *npath)
{//***
	if (file_exists(npath,1,NULL)!=S_IFDIR) {
		char *pth=newstr(npath);
		appendstr(pth," BAD");
		path->SetText(pth);
		delete[] pth;
		return 1;
	}
	
	glob_t globbuf;
	char **patterns; // patterns in mask are assumed to be sanitized already.
	//***get pattern(s) =  path + mask
	patterns=new char*[2];
	patterns[0]=patterns[1]=NULL;
	makestr(patterns[0],npath);
	const char *msk=mask->GetCText();
	while (msk && isspace(*msk)) msk++;
	if (msk==NULL || msk[0]=='\0') msk="*";
	if (patterns[0]==NULL || patterns[0][0]=='\0') appendstr(patterns[0],msk); 
	else {
		appendstr(patterns[0],"/"); 
		appendstr(patterns[0],msk); 
	}
			//*** must sanity check the file name
			// maybe FileResolve(char *&path, char checkforexistencetoo) to remove 
			// ws, extra /, and expand ~

	int c=0,g;
	while (patterns[c]) { //see wordexp?<- does ~ exp., environ var insert, shell escapse, etc..
		if (!c) g=glob(patterns[c], GLOB_MARK|GLOB_PERIOD, NULL, &globbuf);
		else g=glob(patterns[c], GLOB_APPEND|GLOB_MARK|GLOB_PERIOD, NULL, &globbuf);
		
		DBG cerr <<"\n\nGlob test\nPattern: "<<patterns[c]<<"\nglob returns: "<<g<<endl;
		c++;
	}
	for (c=0; patterns[c]; c++) delete[] patterns[c];
	delete[] patterns; patterns=NULL;


	//*** must shrink down the list to remove duplicates
	//***should be more intelligent about when path is invalid
	
	files.menuitems.flush();
	
	if (!globbuf.gl_pathc) {
		globfree(&globbuf); 
		files.AddItem("..",NULL,
				0,               //id
				LAX_HAS_SUBMENU|LAX_OFF,   //state
				1,               //*** info 1==follow links
				NULL,0           //submenu
			);
		files.AddItem(".",NULL,
				1,               //id
				LAX_HAS_SUBMENU|LAX_OFF,   //state
				1,               //*** info 1==follow links
				NULL,0           //submenu
			);
		if (filelist) {
			filelist->Select(0);
			filelist->Sync();
			filelist->Needtodraw(1);
		}
		return 0; 
	}
	
	 // add all the globbuf.gl_pathv items to files..
	int hasdir,s;
	struct stat statbuf;
	for (c=0; c<(int)globbuf.gl_pathc; c++) {
		 // stat for hasdir
		hasdir=0;
		if (dialog_style&FILES_NO_FOLLOW_LINKS) s=lstat(globbuf.gl_pathv[c],&statbuf);
		else s=stat(globbuf.gl_pathv[c],&statbuf);
		if (s) { // error
			perror("filedialog getDirectory stat error");
		} else {
			if (S_ISDIR(statbuf.st_mode)) {	hasdir=1; } 
		}
		//DBG cerr <<"files: "<<globbuf.gl_pathv[c]<<endl;
		if (globbuf.gl_pathv[c][strlen(globbuf.gl_pathv[c])-1]=='/')
			globbuf.gl_pathv[c][strlen(globbuf.gl_pathv[c])-1]='\0';
		files.AddItem(basename(globbuf.gl_pathv[c]),
				c,               //id
				(hasdir?LAX_HAS_SUBMENU:0)|LAX_OFF,   //state
				1,               //*** info 1==follow links
				NULL,0           //submenu
			);
	}
	globfree(&globbuf);
	
	//DBG cerr <<"--==Before sorting..."<<endl;
	//DBG for (c=0; c<files.menuitems.n; c++) {
	//DBG 	cerr <<"files: "<<files.menuitems.e[c]->name<<endl;
	//DBG }

	files.Sort();
	
	//DBG cerr <<"--==After sorting..."<<endl;
	//DBG for (c=0; c<files.menuitems.n; c++) {
	//DBG 	cerr <<"files: "<<files.menuitems.e[c]->name<<endl;
	//DBG }
	
	if (filelist) {
		filelist->Select(0);
		filelist->Sync();
		filelist->Needtodraw(1);
	}
	return 0;
}

//! Set win_w and win_h to sane values if necessary.
int FileDialog::preinit()
{
	anXWindow::preinit();
	if (win_w==0) win_w=500;
	if (win_h==0) {
		int textheight=app->defaultlaxfont->textheight();
		win_h=20*(textheight+7)+20;
	}
	return 0;
}

/*!
 * \todo *** fix quickselect button, it is totally broken at the moment
 */
int FileDialog::init()
{
	if (!ValidDrawable()) return 1;

	int textheight=app->defaultlaxfont->textheight();
	int linpheight=textheight+6;
	anXWindow *last=NULL;
	Button *tbut=NULL;
	if (finalbuttons<0) finalbuttons=0;

	 //----Add FILE line input

	file->SetOwner(this);
	AddWin(file,1, 200,100,1000,50,0, file->win_h,0,0,50,0, finalbuttons++);

	 //----add optional Recent files button near File input
	char *recent=NULL;
	if (recentgroup) recent=recently_used(NULL,isblank(recentgroup)?NULL:recentgroup,0);
	if (recent) {
		MenuInfo *menu=NULL;
		menu=new MenuInfo;
		int n;
		char **bms=split(recent,'\n',&n);
		char *blah=NULL, *dir;
		for (int c=0; c<n-1; c++) {
			blah=newstr(lax_basename(bms[c]));
			appendstr(blah," -- ");
			dir=lax_dirname(bms[c],1);
			appendstr(blah,dir);
			delete[] dir;

			menu->AddItem(blah,c);

			delete[] blah;
			blah=NULL;
		}
		deletestrs(bms,n);
		delete[] recent;
		MenuButton *mb=NULL;
		mb=new MenuButton(this,"recent", NULL,
						 MENUBUTTON_LEFT|MENUBUTTON_SEND_STRINGS,
						 0,0,0,0,1,
						 NULL,object_id,"recent",
						 0,
						 menu,1,
						 _("Recent"));
		mb->pad=app->defaultlaxfont->textheight()/3;
		mb->tooltip(_("Select a recently used file"));
		file->ConnectControl(mb);
		AddWin(mb,1, mb->win_w,0,50,50,0, linpheight,0,0,50,0, finalbuttons++);
	}
	AddNull(finalbuttons++);

	
	 //Add PATH input
	path->SetOwner(this);
	AddWin(path,1, 200,100,1000,50,0, path->win_h,0,0,50,0, finalbuttons++);

	 //bookmarks button near path
	MenuInfo *bookmarkmenu=BuildBookmarks();
	MenuButton *menu=NULL;
	menu=new MenuButton(this,"bookmarks",NULL,
							 MENUBUTTON_LEFT|MENUBUTTON_SEND_STRINGS,
							 0,0,0,0,1,
							 path,object_id,"bookmarks",
							 0,
							 bookmarkmenu,1,
							 _("Bookmarks"));
	menu->tooltip(_("Select existing bookmark or make a new bookmark"));
	AddWin(menu,1, menu->win_w,0,50,50,0, linpheight,0,0,50,0, finalbuttons++);
	
	AddNull(finalbuttons++);

	 //Add MASK input
	mask->SetOwner(this);
	AddWin(mask,1, 200,100,1000,50,0, mask->win_h,0,0,50,0, finalbuttons++);
	AddNull(finalbuttons++);
	last=mask;
	
	last=filelist=new MenuSelector(this,"files",NULL, 0, 0,0,0,0,1,
			last,object_id,"files",
			MENUSEL_SEND_ON_UP|MENUSEL_CURSSELECTS|MENUSEL_TEXTCOLORS|
			MENUSEL_LEFT|MENUSEL_SUB_ON_LEFT|MENUSEL_SUB_FOLDER, &files,0);
	filelist->tooltip(_("Choose from these files.\nRight-click drag scrolls"));
	AddWin(filelist,1, 200,100,1000,50,0, 30,0,1000,50,0, finalbuttons++);

	if (dialog_style&FILES_PREVIEW) {
		char *blah=fullFilePath(NULL);
		last=previewer=new FilePreviewer(this,"previewer",NULL,MB_MOVE|MB_CENTER, 0,0,0,0, 1, blah);
		delete[] blah;
		AddWin(previewer,1, 100,50,500,50,0, 30,0,1000,50,0, finalbuttons++);
	}
	AddNull(finalbuttons++);

	 //[Bookmarks v][New dir][up][refresh][details][back][forward]


	last=tbut=new Button(this,"fd-bookmark",NULL,0, 0,0,0,0, 1, 
			last,object_id,"bookmark",
			0,_("Bookmark"),NULL,NULL,3,3);
	tbut->tooltip(_("Bookmark the current directory"));
	AddWin(tbut,1, tbut->win_w,0,50,50,0, linpheight,0,0,50,0, finalbuttons++);

	if (dialog_style&FILES_SAVING) {
		 //have "new dir" button only when saving
		last=tbut=new Button(this,"fd-new dir",NULL,0, 0,0,0,0, 1, 
								 last,object_id,"new dir",
								 0,_("New Dir"),NULL,NULL,3,3);
		last->tooltip(_("Create a new directory"));
		AddWin(tbut,1, tbut->win_w,0,50,50,0, linpheight,0,0,50,0, finalbuttons++);
	}
	
	last=tbut=new Button(this,"fd-up",NULL, 0, 0,0,0,0, 1, 
						last,object_id,"go up",
						0,_("Up"),NULL,NULL,3,3);
	last->tooltip(_("Go up a directory"));
	AddWin(tbut,1, tbut->win_w,0,50,50,0, linpheight,0,0,50,0, finalbuttons++);

	last=tbut=new Button(this,"fd-refresh",NULL,0, 0,0,0,0, 1, 
						last,object_id,"refresh",
						0,_("Refresh"),NULL,NULL,3,3);
	last->tooltip(_("Re-read current directory"));
	AddWin(tbut,1, tbut->win_w,0,50,50,0, linpheight,0,0,50,0, finalbuttons++);

	last=tbut=new Button(this,"fd-display format",NULL,0, 0,0,0,0, 1, 
						last,object_id,"details",
						0,_("Details"),NULL,NULL,3,3);
	tbut->State(LAX_GRAY);
	last->tooltip(_("(Not implemented yet)"));
	AddWin(tbut,1, tbut->win_w,0,50,50,0, linpheight,0,0,50,0, finalbuttons++);

	last=tbut=new Button(this,"fd-back",NULL,0, 0,0,0,0, 1, 
						last,object_id,"go back",
						0,_("Back"),NULL,NULL,3,3);
	tbut->State(LAX_GRAY);
	last->tooltip(_("(Not implemented yet)"));
	AddWin(tbut,1, tbut->win_w,0,50,50,0, linpheight,0,0,50,0, finalbuttons++);

	last=tbut=new Button(this,"fd-forward",NULL,0, 0,0,0,0, 1, 
						last,object_id,"go forward",
						0,_("Forward"),NULL,NULL,3,3);
	tbut->State(LAX_GRAY);
	last->tooltip(_("(Not implemented yet)"));
	AddWin(tbut,1, tbut->win_w,0,50,50,0, linpheight,0,0,50,0, finalbuttons++);
	AddNull(finalbuttons++);


	 //Now for the final buttons....
//	unsigned long okbutton=BUTTON_OK;
//	if (!ok && ) {
//		if (dialog_style&(FILES_SAVE|FILES_SAVE_AS)) okbutton=BUTTON_SAVE;
//		else if (dialog_style&(FILES_OPEN_MANY|FILES_OPEN_ONE)) okbutton=BUTTON_OPEN;
//		last=ok=new Button(this,"fd-Ok",NULL,okbutton, 0,0,0,0, 1, 
//				last,window,"ok", 0,NULL,NULL,NULL,3,3);
//	} else {
//		 //add to control loop
//		ok->SetOwner(window,NULL);
//		last->ConnectControl(ok,1);
//		last=ok;
//	}
//	finalbuttons=wholelist.n-1;
//	AddWin(ok,1, ok->win_w,0,50,50, linpheight,0,0,50, -1);
//--------------
	if (finalbuttons>=0) {
		 //connect the control loop
		WinFrameBox *box;
		for (int c=finalbuttons; c<wholelist.n; c++) {
			box=dynamic_cast<WinFrameBox *>(wholelist.e[c]);
			if (!box || !box->win()) continue;
			box->win()->SetOwner(this);
			last->ConnectControl(box->win());
			last=box->win();
		}
	}

	last->CloseControlLoop();
	Sync(1);

	return 0;
}

//! Add a new bookmark via anXApp::AppResource() and a "Bookmarks" resource.
/*! Return 0 for bookmark added, or positive for error and not added,
 * and -1 for already exists.
 *
 * Default is to add bookmark via add_bookmark(). This will store it
 * globally for the user. If this is not desired, then FILES_GLOBAL_BOOKMARK
 * must be removed from dialog_style.
 */
int FileDialog::newBookmark(const char *pth)
{
	if (pth && !strncmp(pth,"file://",7)) pth+=7;
	if (file_exists(pth,1,NULL)!=S_IFDIR) return 1;

	 //add globally
	if (dialog_style&FILES_GLOBAL_BOOKMARK) add_bookmark(pth,0);

	 //find any existing lax held bookmarks
	Attribute *att=app->AppResource("Bookmarks");

	 //check marks for duplicates
	if (att) {
		for (int c=0; c<att->attributes.n; c++) {
			if (!strcmp(pth,att->attributes.e[c]->name)) return -1;
		}
	}

	 //copy old resource to new resource
	Attribute *newatt=new Attribute("Bookmarks",NULL,NULL);
	if (att) {
		for (int c=0; c<att->attributes.n; c++) {
			newatt->push(att->attributes.e[c]->name);
		}
	}
	newatt->push(pth);

	 //install new resource
	app->AppResource(newatt);

	 //update bookmarks button if possible
	MenuInfo *info=BuildBookmarks();//***what's this??
	if (info) delete info;

	return 0;
}

/*! If the bookmark buttons does not exist, then return a new MenuInfo with
 * the bookmarks in it. If the button already exists, then replace its menu
 * with the new one, and return NULL.
 */
MenuInfo *FileDialog::BuildBookmarks()
{
	MenuInfo *bookmarkmenu=new MenuInfo;
	char *bookmarks=get_bookmarks(NULL,NULL);
	int n=0;
	if (bookmarks) {
		char **bms=split(bookmarks,'\n',&n);
		bookmarkmenu->AddItems((const char **)bms,n,0);
		deletestrs(bms,n);
		delete[] bookmarks;
	}
	Attribute *att=app->AppResource("Bookmarks");
	if (att) {
		int c2;
		for (int c=0; c<att->attributes.n; c++) {
			for (c2=0; c2<bookmarkmenu->menuitems.n; c2++) {
				if (!strcmp(bookmarkmenu->menuitems.e[c2]->name, att->attributes.e[c]->name)) break;
			}
			if (c2<att->attributes.n) continue; //skip if already there!
			bookmarkmenu->AddItem(att->attributes.e[c]->name);
			n++;
		}
	}
	if (n) bookmarkmenu->AddSep();
	bookmarkmenu->AddItem(_("Add current directory"),10000);
	MenuButton *b=(MenuButton *)findChildWindowByName("bookmarks");
	if (!b) return bookmarkmenu;
	b->SetMenu(bookmarkmenu,1);
	return NULL;
}

//! Change the text on the "Ok" button to something else.
/*! By default, that button will be "Ok" or "Open" or "Save".
 * This function allows simple further adjustment.
 *
 * If ttip!=NULL, then change the ok button tooltip also. If ttip!=NULL,
 * but is blank, then clear the tooltip.
 */
void FileDialog::OkButton(const char *textforok, const char *ttip)
{
	if (isblank(textforok) || !ok) return;

	ok->Label(textforok);
	ok->WrapToExtent(1);
	int okindex=findWindowIndex("fd-Ok");
	if (okindex>=0) {
		wholelist.e[okindex]->pw(ok->win_w);
	}

	if (ttip) ok->tooltip(isblank(ttip)?NULL:ttip);
}

//! Add another final button.
/*! By default, there is on "ok" type of button (whose text can be set with
 * OkButton()) at position 0, and a "cancel" button at position 1 (unless FILES_NO_CANCEL is 
 * set in dialog_style). Those buttons were added in the constructor.
 * This function allows easy addition of further buttons,
 * for instance to have three buttons: "Open", "Open a copy", and "Cancel".
 *
 * If id==0, then this button will act like a cancel button. id==1 is the same
 * as pressing the standard "ok" button. Applications may use any other value for
 * a button id.
 *
 * If position>=0, then put it at this position (starting at 0) of the list of final buttons.
 *
 * If ttip!=NULL, then that is the button's tooltip.
 */
void FileDialog::AddFinalButton(const char *text, const char *ttip, int id, int position)
{
	if (isblank(text)) return;

	Button *final=new Button(this,"fd-final",NULL, 0, 0,0,0,0, 1, 
									 NULL,object_id,"final",
									 id,
									 text,NULL,NULL,3,3);
	final->tooltip(ttip);
	AddWin(final,1, final->win_w,0,50,50,0, final->win_h,0,0,50,0, finalbuttons+position);
}

//! Remove a final button.
/*! If position<0, then remove all the final buttons.
 *
 * Return 0 for success, nonzero for error.
 *
 * \todo this needs testing
 */
int FileDialog::ClearFinalButton(int position)
{
	 //this assumes that the final buttons start at finalbuttons in wholelist,
	 //and go until the end of wholelist.
	char cont=0;
	if (position<0) {
		cont=1; 
		position=0; 
		ok=NULL;
	}
	if (finalbuttons<0) return 0;
	if (finalbuttons+position>=wholelist.n) return 1;
	do {
		WinFrameBox *box=dynamic_cast<WinFrameBox *>(wholelist.e[finalbuttons+position]);
		anXWindow *win=NULL;
		if (box) win=box->win();
		if (win) app->destroywindow(win);
		Pop(finalbuttons+position); //calls list.remove(index)
		if (finalbuttons==wholelist.n) { finalbuttons=-1; break; }
	} while (cont);
	return 0;
}

/*! \todo no sanity checking is done before mkdir(str)
 */
int FileDialog::Event(const EventData *data,const char *mes)
{
	DBG cerr <<"-----file dialog got: "<<mes<<endl;

	const SimpleMessage *s=dynamic_cast<const SimpleMessage*>(data);

	if (!strcmp(mes,"new directory")) {
		if (!s) return 1;
		if (isblank(s->str) || file_exists(s->str,0,NULL)) return 1;
		if (mkdir(s->str,S_IRWXU)==0) {
			path->SetText(s->str);
			RefreshDir();
		}
		return 0;

	} else if (!strcmp(mes,"recent")) {
		const StrsEventData *ss=dynamic_cast<const StrsEventData *>(data);
		if (!ss || !ss->n) return 1;

		const char *p=strstr(ss->strs[0]," -- ");
		if (!p) return 0;
		char *file=newstr(p+4);
		appendnstr(file, ss->strs[0], p-ss->strs[0]);
		SetFile(file);
		delete[] file;
		return 0;

	} else if (!strcmp(mes,"bookmarks")) {
		const StrsEventData *ss=dynamic_cast<const StrsEventData *>(data);
		if (!ss || !ss->n) return 1;
		if (!strcmp(ss->strs[0],_("Add current directory"))) {
			 //add current directory as a bookmark
			newBookmark(path->GetCText());
			return 0;
		}
		char *f=ss->strs[0];
		if (!strncmp(f,"file://",7)) f+=7;
		//if (file_exists(f,1,NULL)) Cd(f);
		if (f) Cd(f);
		return 0;

	} else if (!strcmp(mes,"files")) { // from menuselector

		 //selected "."
		if (s->info1==0 && files.menuitems.n && !strcmp(files.menuitems.e[0]->name,"."))
			{ RefreshDir(); return 0; }
		
		 //selected ".."
		if (s->info1==1 && files.menuitems.n>1 && !strcmp(files.menuitems.e[1]->name,".."))
			{ GoUp(); return 0; }

		 //if possible update file and path to reflect curitem, whether or not
		 // many items are selected.
		if (s->info1<files.menuitems.n) {
			MenuItem *m=files.findFromLine(s->info1);
			if (m && m->state&LAX_HAS_SUBMENU) {
				DBG cerr <<"....Cd("<<m->name<<")"<<endl;
				Cd(m->name);
				return 0;
			}
			char *newfile=files.menuitems.e[s->info1]->name;
			SetFile(newfile);
			//file->SetText(newfile);
			if (previewer) {
				char *blah=fullFilePath(NULL);
				previewer->Preview(blah);
				delete[] blah;
			}
		}

		 //clear any selected directories, if can only select files
		if (!(dialog_style&FILES_SELECT_DIR)) {
			for (int c=0; c<files.menuitems.n; c++) {
				if ((files.menuitems.e[c]->state&LAX_ON) && (files.menuitems.e[c]->state&LAX_HAS_SUBMENU)) {
					files.menuitems.e[c]->state^=LAX_ON;
					filelist->Needtodraw(1);
				}
			}
		}
		return 0;

	} else if (!strcmp(mes,"bookmark")) {
		newBookmark(path->GetCText());
		return 0;

	} else if (!strcmp(mes,"new file")) {
		 //text was changed in file input

		 // update path
		char *full=fullFilePath(NULL);
		simplify_path(full,1);
		char *dir,*f=NULL;
		if (file_exists(full,1,NULL)==S_IFDIR) {
			dir=full;
			full=NULL;
		} else {
			dir=lax_dirname(full,0);
			f=newstr(lax_basename(full));
		}
		if (dir) {
			getDirectory(dir);
			path->SetText(dir);
			delete[] dir;
		}
		int curpos=file->GetLineEdit()->GetCurpos();
		SetFile(f);
		file->GetLineEdit()->SetCurpos(curpos);
		if (f) delete[] f;
		if (full && previewer) previewer->Preview(full);
		if (full) delete[] full;
		return 0;

	} else if (!strcmp(mes,"final")) {
		if (send(s->info2)) closeWindow();
		return 0;

	} else if (!strcmp(mes,"ok")) {
		if (send(1)) closeWindow();
		return 0;

	} else if (!strcmp(mes,"cancel")) {
		closeWindow();
		return 0;

	} else if (!strcmp(mes,"overwrite")) {
		StrEventData *e=new StrEventData(s->str,1,0,0,0);
		app->SendMessage(e,win_owner,win_sendthis,object_id);
		closeWindow();
		return 0;

	} else if (!strcmp(mes,"new path")) { // rescan directory via mask..
		getDirectory(path->GetCText());
		return 0;

	} else if (!strcmp(mes,"new mask")) {
		getDirectory(path->GetCText());
		return 0;

	} else if (!strcmp(mes,"new dir")) {
		InputDialog *i= new InputDialog(NULL,"New Directory",NULL,ANXWIN_CENTER,
									 20,20,0,0, 0,
									 NULL,object_id,"new directory",
									 path->GetCText(),	  //start text
									 _("New directory?"), //label
									 _("Create"), 1,
									 _("Cancel"), 0);
		app->rundialog(i);
		return 0;

	} else if (!strcmp(mes,"go up")) {
		DBG cerr <<"***file dialog: "<<mes<<endl; //***
		GoUp();
		return 0;

	} else if (!strcmp(mes,"go back")) {
		DBG cerr <<"***file dialog: "<<mes<<endl; //***

	} else if (!strcmp(mes,"go forward")) {
		DBG cerr <<"***file dialog: "<<mes<<endl; //***

	} else if (!strcmp(mes,"refresh")) {
		//DBG cerr <<"***file dialog: "<<mes<<endl; //***
		RefreshDir();
		return 0;

	} else if (!strcmp(mes,"details")) { // toggle details/all together
		DBG cerr <<"***file dialog: "<<mes<<endl; //***

	}
	return 1;
}

//! In a new char[], return path/f, or path/file if f==NULL.
/*! If f (or file) appears to be an absolute path, use only that..
 */
char *FileDialog::fullFilePath(const char *f)
{
	if (!f) f=file->GetCText();
	char *full=newstr(f);

	if (full[0]=='~' && full[1]=='/') expand_home_inplace(full);
	if (full[0]=='/') return simplify_path(full,1);

	prependstr(full,"/");
	prependstr(full,path->GetCText());
	return simplify_path(full,1);//modifies and returns full
}

//! Set app->load_dir or app->save_dir to path
/*! If FILES_NO_CANCEL then set the app dirs, but do not call app->destroywindow(this).
 *
 * Returns 1 for window set to be destroyed, else 0.
 */
int FileDialog::closeWindow()
{
	if (path->GetCText()) {
		if (dialog_style&FILES_OPENING) makestr(app->load_dir,path->GetCText());
		else makestr(app->save_dir,path->GetCText());
	}
	if (!(dialog_style&FILES_NO_CANCEL)) { app->destroywindow(this); return 1; }
	return 0;
}

//! Send the file name(s) to owner.
/*! Returns 0 if nothing sent. Else nonzero.
 *
 * id will typically be 1 for "ok", and any other number will be the id from 
 * any additional final buttons.
 *
 * \todo if FILES_OPEN_MANY but none selected in the menuselector, defaults to
 *    sending a StrEventData rather than a StrsEventData... should probably make
 *    it send StrsEventData with one string...?
 */
int FileDialog::send(int id)
{
	if (!win_owner || !win_sendthis) return 0;

	char *blah=NULL;
	char *typedinfile=fullFilePath(NULL);
	DBG cerr <<"====Sending "<<(blah?blah:"(not sending!)")<<" to "<<win_owner<<endl;
	
	if (isblank(typedinfile)) {
		if (typedinfile) delete[] typedinfile;
		return 0;
	}
	
	if (dialog_style&FILES_OPEN_MANY) {
		int typedexists=0; //for the line that is typed, might not strictly be an item in list
		int numfiles=0;
		int f=file_exists(typedinfile,1,NULL);
		if ((dialog_style&FILES_FILES_ONLY) && !S_ISREG(f)) {}
		else typedexists=1;

		int *which=filelist->WhichSelected(LAX_ON);
		StrsEventData *e=new StrsEventData(NULL,win_sendthis,object_id,win_owner);
		e->info=id;
		e->strs=new char*[which[0]+typedexists];
		if (typedexists) {
			numfiles++;
			e->strs[0]=typedinfile;
		} else { delete[] typedinfile; typedinfile=NULL; }

		if (which!=NULL) {
			DBG cerr <<"********** open many files..."<<endl;
			const MenuItem *item;
			for (int c=0; c<which[0]; c++) {
				e->strs[numfiles]=NULL;
				item=filelist->Item(which[c+1]);

				if (item) {
					if (blah) delete[] blah;
					blah=path->GetText();
					if (blah[strlen(blah)-1]!='/') appendstr(blah,"/");
					appendstr(blah,item->name);
					f=file_exists(blah,1,NULL);
					if (typedinfile && !strcmp(typedinfile,blah)) continue; //is same as typed in

					 //only add file if is regular file and you want only files,
					 //or is any type of file or directory, and you want any kind of file
					if (((dialog_style&FILES_FILES_ONLY) && S_ISREG(f))
						 || (!(dialog_style&FILES_FILES_ONLY) && f)) {
						e->strs[numfiles]=blah;
						blah=NULL;
						numfiles++;
					}
				}
			}
		}
		e->n=numfiles;
		if (numfiles==0) {
			delete[] e->strs; e->strs=NULL;
			delete e;
		} else {
			app->SendMessage(e,win_owner,win_sendthis,object_id);
		}
		if (which) delete[] which;
		if (blah) delete[] blah;
		return 1;
	}

	if ((dialog_style&FILES_SAVING) && file_exists(typedinfile,0,NULL)) {
		app->rundialog(new Overwrite(object_id,"overwrite",typedinfile,0,0,0));
		return 0;
	}

	StrEventData *e=new StrEventData(typedinfile,id,0,0,0);
	app->SendMessage(e,win_owner,win_sendthis,object_id);
	delete[] typedinfile;
	return 1;
}

//! Refresh the current directory listing.
void FileDialog::RefreshDir() 
{
	getDirectory(path->GetCText());
}

//! Assume file is a file path, update file and path fields.
void FileDialog::SetFile(const char *f)
{
	const char *bname=lax_basename(f);
	char *dir=lax_dirname(f,1);
	file->SetText(bname);
	if (dir) {
		if (strcmp(dir,path->GetCText())) Cd(dir);
		path->SetText(dir);
		delete[] dir;
	}
}

//! Change directory to path/to if to is relative, or just to if it is absolute. Also set the path field.
/*! \todo need sanity checking on to.
 */
void FileDialog::Cd(const char *to)
{
	if (!to) return;
	DBG cerr <<"... cd to:"<<(to?to:"Go to where???")<<endl;

	char *pth=NULL;
	if (to[0]!='/') {
		pth=path->GetText();
		if (pth[strlen(pth)-1]!='/') appendstr(pth,"/");
		appendstr(pth,to);
	} else {
		pth=newstr(to);
	}
	if (!getDirectory(pth)) {
		path->SetText(pth);
	}
	delete[] pth;
}

//! Go up in the file hierarchy, and set path text.
void FileDialog::GoUp()
{
	if (!strcmp("/",path->GetCText())) return; // if on "/" can't go up
	char *pth=path->GetText();
	char *lastslash=strrchr(pth,'/');
	if (!lastslash) {
		char *ppth=getcwd(NULL,0);
		makestr(pth,ppth);
		free(ppth);
	}
	if (lastslash-pth==(int)strlen(pth)-1) { // remove trailing '/'
		pth[strlen(pth)-1]='\0';
		lastslash=strrchr(pth,'/');
	}
	if (!lastslash) { delete[] pth; return; }
	*lastslash='\0';
	if (pth[0]=='\0') strcpy(pth,"/");
	path->SetText(pth);
	getDirectory(pth);
	delete[] pth;
}

//! Cancel if ESC.
int FileDialog::CharInput(unsigned int ch,const char *buffer,int len,unsigned int state,const LaxKeyboard *d)
{
	if (ch==LAX_Esc) {
		return !closeWindow();//esc will propagate
	}
	return anXWindow::CharInput(ch,buffer,len,state,d);
}



} // namespace Laxkit







