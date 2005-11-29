/*  Copyright  2005  jinti jinti.shen@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <pthread.h>
#include <httpd.h>
#include <map>
#include <list>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>

#include <sys/stat.h>

#include <httpd.h>
#include <http_protocol.h>
#include <http_config.h>
#include <http_core.h>
#include <http_request.h>
#include <http_vhost.h>
#include <http_main.h>
#include <http_log.h>
#define APACHE2
#ifdef APACHE2
#include <http_connection.h>
#include <util_filter.h>
#endif

#include "FastCPP.hxx"
#include "FCException.hxx"
#include "Lock.hxx"
#include "FCPageFile.hxx"
#include "FCParser.hxx"

#include "FCManager.hxx"
#include "FCSessionManager.hxx"

FastCPP::Mutex _RPageMutex;
FastCPP::Mutex _WLruMutex;
FastCPP::Mutex _WRemovePageMutex;

FastCPP::Mutex _RSessionMutex;
FastCPP::Mutex _WSessionMutex;

using namespace std;
using namespace FastCPP;

static int MaxCachePages =  128;

//PageFiles  _pages;
list<FCPageFile*>  _removePages;

PageCaches _pages;

SessionsMaps _sessionsMaps;
FCManager manager;
FCSessionManager sessionManager;
static int ii = 0;
static bool test = false;

void ShowMsg(request_rec *r, 
	     const string &title,
	     const string &msg,
	     int code);

int doRequest(request_rec *r)
{
   FCRequest request(r);
   try
   {
      request.Init();

      FCPageFile *file = 0;

      bool reBuild = true;
      {
	 Lock lock(_RPageMutex);

	 PageCaches::iterator it = _pages.find(request.FileName());
	 if (it != _pages.end())
	 {
	    file = it->second;
	    reBuild = false;
	    time_t t = GetFileMTime(request.FileName());
//	    request<<"t:"<<t<<",mt:"<<file->MTime();
	    if (t != file->MTime())
	    {
	       reBuild = true;
	    }	               
	 }
      }
	
      if (reBuild)
      {
	 FCPageFile *tmpfile = NULL;
	 FCPageFile  *tail = NULL;
	 {
	    Lock lock(_WLruMutex);
	    tmpfile = new FCPageFile(request.FileName(),
				     FCPageFile::mLruLists);
	    
	       
	    if (FCPageFile::mLruLists.size() > MaxCachePages)
	    {
	       //remove last page
	       tail = dynamic_cast<FCPageFile *>(FCPageFile::mLruLists.tail());
	       if (tail)
	       {
		  Lock lock(_WRemovePageMutex);			   
		  _removePages.push_back(tail);
		  _pages.erase(tail->FileName());
	       }
	    }
	 }

	    
	 try
	 {
	    tmpfile->MTime(GetFileMTime(request.FileName()));

	    FCParser p(request.FileName());
	    p.Parse();

	    tmpfile->ToSource(p);
	    tmpfile->Compile();
	    tmpfile->LoadSO();
	 }catch(FCException &e)
	 {
	    // show message
	    delete tmpfile;
	    ShowMsg(r,"Parser Error!",
		    e.what(),
		    500);
	    return OK;
	 }
	 catch(...)
	 {
	    // show message
	    delete tmpfile;
	    ShowMsg(r,"Parser Error!",
		    ":( UnKnown Error!",
		    500);
	    return OK;
	 } 
	    
	 {
	    Lock lock(_WLruMutex);
	    if (file)
	    {
	       // when update one pags's exist pagefile ,
	       // then remove the pagefile from pages,and put into removePages.
	       Lock lock(_WRemovePageMutex);			   
	       _removePages.push_back(file);
	       _pages.erase(file->FileName());
	    }
      
	 }
	 Lock lock(_WLruMutex);	    
	 file = tmpfile;
	 _pages[request.FileName()] = file;
      }
      else
      {
	 Lock lock(_WLruMutex);
	 FCPageFile::mLruLists.update(file);
      }
      try
      { 
	 file->AddRef();
	 file->FCService(request);
	 file->DecRef();
	 request.CheckRedirect();
	 return OK;
      }
      catch(FCException &e)
      {
	 file->DecRef();
	 ShowMsg(r,"Run FCService Error!",
		 e.what(),
		 500);
	 return OK;
      }
      catch(...)
      {
	 file->DecRef();
	 ShowMsg(r,"Run FCService UnKnown Error!",
		 ":( UnKnown Error!",
		 500);
	 return OK;
      }
   }
   catch(FCException &e)
   {
      ShowMsg(r,"FastCPP Error!",
	      e.what(),
	      500);
      return OK; 
   }
   catch(...)
   {
      return 500;
   }
   return OK;
}

void SetMaxCachePages(int size)
{
   MaxCachePages = size;
}
time_t GetFileMTime(const string &file)
{
   struct stat info;
   if (lstat(file.c_str(),&info) == 0)
   {
      return info.st_mtime;
   }
   return 0;
    
}
void GenUUID(char *buf,int len)
{
   char table[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJLMOPQRSTUVWXYZ";
   static  unsigned int code = getpid();
   
   if(code > UINT_MAX)
   {
      code = 0;
   }
   code ++;
   srand(code);

   for (int i = 0; i < len; i++) 
   { 
      buf[i] = table[rand()%(sizeof(table)-1)];
   }

   buf[len] = '\0';   
}
#define SHELL_PATH "/bin/sh"

int FCExec(const string& prog,
	   const string& param,
	   int *pout,
	   int *pin,
	   int *perr)
{
   pid_t newpid;
   const char *newargs[4];
   string s = prog;
   s += " ";
   s += param;

   newargs[0] = SHELL_PATH;
   newargs[1] = "-c";
   newargs[2] = s.c_str();
   newargs[3] = NULL;

   if ((newpid = fork()) < 0)
   {
      return errno;
   }
   else if (newpid == 0) 
   {
      if (pout)
      {
	 close(0);	/* stdin */
       
	 dup2(pout[PIPE_RD], 0);
	 close(pout[PIPE_RD]); close(pout[PIPE_WR]);
      }

      if (pin)
      {
	 close(1);	/* stdout */
	 dup2(pin[PIPE_WR], 1);

	 close(pin[PIPE_RD]); close(pin[PIPE_WR]);
      }

      if (perr)
      {
	 close(2);	/* stderr */
	 dup2(perr[PIPE_WR], 2);

	 close(perr[PIPE_RD]); close(perr[PIPE_WR]);
      }
      execv(SHELL_PATH,(char* const*)newargs);
      
      return -1;
   }
   return newpid;
}

void ShowMsg(request_rec *r, 
	     const string &title,
	     const string &msg,
	     int code)
{
   string s("<html><head><title>");
   s += title;
   s += "</title>";
   s += "</head><body><h1>";
   s += title;
   s += "</h1><H3><pre>";
   s += msg;
   s += "</pre></H3><hr><H3><address>Powered by FastCPP </address></H3></body></html>";
   ap_rwrite(s.c_str(),s.size(), r);
}

int InitFastCPP()
{
   manager.Run();
   sessionManager.Run(); 
}

