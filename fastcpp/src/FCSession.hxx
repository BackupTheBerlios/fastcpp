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

#ifndef _FCSESSION_HXX_
#define _FCSESSION_HXX_

#include <map>
#include <string>
#include "Mutex.hxx"
#include "Lock.hxx"

using namespace std;


namespace FastCPP
{
   class FCSession 
   {
      public:
	 virtual ~FCSession(){}
   };
   // basic session implement
   template <typename T>
   class FCSessionImpl : public FCSession
   {
      public:
	 FCSessionImpl(const T &_t):t(_t)
	 {}
	 const T & get()
	 {
	    return t;
	 }
	 T t;
   };

   typedef FCSessionImpl<string> FCString;
   typedef FCSessionImpl<int> FCInt;
   typedef FCSessionImpl<long> FCLong;
   typedef FCSessionImpl<bool> FCnBool;
   typedef FCSessionImpl<double> FCDouble;
   typedef FCSessionImpl<char> FCChar;
   typedef FCSessionImpl<float> FCFloat;


   typedef map<string, FCSession*> SessionMaps;
   class FCSessions
   {
      public:
	 FCSessions(const string &s,bool use = true);
 	 ~FCSessions();
	 bool isUse()
	 {
	    Lock lock(mMutex);
	    return mUse;
	 }
	 void inUse()
	 {
	    Lock lock(mMutex);
	    mUse = true;
	 }
	 void outUse()
	 {
	    Lock lock(mMutex);
	    mUse = false;
	 }
	 bool isTimeOut()
	 {
	    Lock lock(mMutex);
	    int now = time(NULL);
	    if (now >= expire)
	    {
	       return true;
	    }
	    return false;
	 }
	 static int DefaultExpire;
	 static int SessionIDLen;
	 Mutex mMutex;
	 SessionMaps  mSessions;
	 int expire;
	 bool mUse;
	 string mId;
   };
   
   typedef map<string, FCSessions *> SessionsMaps;
};

#endif // _FCPAGE_HXX_
