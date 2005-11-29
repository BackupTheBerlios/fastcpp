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

#ifndef _MUTEX_HXX_
#define _MUTEX_HXX_
#ifndef WIN32
//Linux
#include <pthread.h>
#else
#include <windows.h>
#endif
namespace FastCPP
{
   class Mutex
   {
      public:
	 Mutex();
	 ~Mutex();
	 virtual void Lock();
	 virtual void UnLock();
      private:
#ifndef WIN32
	 pthread_mutex_t mMutex;
#else
	 CRITICAL_SECTION mMutex;
#endif
   };
	
};

#endif
