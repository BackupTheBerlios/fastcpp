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

#include <errno.h>
#include <dlfcn.h>
#include <fstream>
#include <sys/wait.h>

#include "FastCPP.hxx"
#include "Lock.hxx"
#include "FCException.hxx"
#include "FCPageFile.hxx"
#include "FCParser.hxx"

#define TMPPATH "/tmp/"

using namespace FastCPP;
using namespace std;

#define DefaultCompiler "g++"
#define DefaultCompilParam " -O2 -shared -I/usr/include -L/usr/lib "

LruList FCPageFile::mLruLists;

FCPageFile::FCPageFile(const string &n,LruList &list):
   LruList::BaseLruItem(list),
   mCount(0),
   FCService(0),
   mHandle(0),
   mtime(0),
   mCXXFile(TMPPATH),
   mSOFile(TMPPATH),
   mPageName(n)
{
   // gen temp cxx file name
   // gen temp so file name
   char uuid[24];
   GenUUID(uuid, sizeof(uuid));	    
   mCXXFile+= uuid;
   mCXXFile+= ".cxx";

   mSOFile+= uuid;
   mSOFile+= ".so";
}

FCPageFile::~FCPageFile()
{
   if (mHandle)
   {
      dlclose(mHandle);
      mHandle = 0;
      FCService = 0;
      unlink(mSOFile.c_str());
   }
}

bool 
FCPageFile::IsIdle()
{
   Lock lock(mMutex);
   return mCount == 0;
}

void 
FCPageFile::ToSource(const FCParser &p)
{
   ofstream inFile(mCXXFile.c_str());
   inFile<<p;
   inFile.close();
   // get comp, link, inc, link 
   mCompileStr = DefaultCompilParam;
   mCompileStr += p.mInc;
   mCompileStr += p.mLink;
   mCompileStr += p.mComp;
   
   mCompileStr += " ";
   mCompileStr += mCXXFile;
   mCompileStr += " -o ";
   mCompileStr += mSOFile;


}
#define BUFSIZE 1024
void 
FCPageFile::Compile()
{
   // exec compiler   

   int perr[2];

   char buffer[BUFSIZE];
   string compiler(DefaultCompiler);
   string stderrStr;
   pipe(perr);

   pid_t pid = FCExec(compiler,mCompileStr,NULL, NULL, perr);

   close(perr[PIPE_WR]);

   memset(buffer, 0, sizeof(buffer));
   int n;
   while ( (n = read(perr[PIPE_RD], buffer, BUFSIZE -1)) > 0 )
   {	   
      stderrStr += buffer;
      memset(buffer, 0, sizeof(buffer));
   }

   close(perr[PIPE_RD]);

   int status;
   waitpid(pid,&status,0);
   
   if (unlink(mCXXFile.c_str()) < 0)
   {
      // report error!      
   }
   
   if (status != 0)
   {
      //Compile Error!
      FCThrow(stderrStr);
   }
}

void 
FCPageFile::LoadSO()
{
   if (!(mHandle = dlopen(mSOFile.c_str(),RTLD_NOW)))
   {
      string s("Can't load the so file:");
      s += mSOFile;
      s += ",";
      s += dlerror();
      FCThrow(s);
   }

   if(!(FCService = (FCServiceFunc)dlsym(mHandle, "FCService")))
   {
      string s("Can't load the FCService@");
      s += mSOFile;
      FCThrow(s);
   }
}

time_t 
FCPageFile::MTime() const
{
   return mtime;
}
void 
FCPageFile::MTime(const time_t &t)
{
   mtime = t;
}
void 
FCPageFile::AddRef()
{
   Lock lock(mMutex);
   mCount++;
}
void 
FCPageFile::DecRef()
{
   Lock lock(mMutex);
   mCount--; 
}
