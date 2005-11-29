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

#ifndef _FCPAGEFILE_HXX_
#define _FCPAGEFILE_HXX_
#include <stdint.h>
#include <map>
#include "Mutex.hxx"
#include "FCRequest.hxx"
#include "LruList.hxx"

using namespace std;

namespace FastCPP
{
   class FCParser;
   class FCPageFile;
   // typedef pair<FCPageFile *,LruItem *> SingleCachePage;
   typedef map<string, FCPageFile *> PageCaches;
   typedef void (*FCServiceFunc)(FCRequest &);
   class FCPageFile : public LruList::BaseLruItem
   {
      public:
	 FCPageFile(const string &n, LruList &list);
	 ~FCPageFile();
	 bool IsIdle();
	 void ToSource(const FCParser &p);
	 void Compile();
	 void LoadSO();
	 time_t MTime() const;
	 void MTime(const time_t &t);
	 void AddRef();
	 void DecRef();
	 FCServiceFunc FCService;
	 //void (*FCService)(FCRequest &request);
	 const string &FileName()
	 {
	    return mPageName;
	 }
      public:
	 static LruList mLruLists;
      protected:
	 uint32_t mCount; // use count


	 void *mHandle;
	 time_t mtime;
	 Mutex mMutex;
	 string mCXXFile;
	 string mSOFile;
	 string mCompileStr;
	 string mPageName;
   };
};
#endif //_FCPAGEFILE_HXX_
