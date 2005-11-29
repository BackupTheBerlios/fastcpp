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

#include "Mutex.hxx"

using namespace FastCPP;

Mutex::Mutex()
{
#ifndef WIN32
   pthread_mutex_init(&mMutex,NULL);
#else
   InitializeCriticalSection(&mMutex);
#endif
}

Mutex::~Mutex()
{
#ifndef WIN32
   pthread_mutex_destroy(&mMutex);
#else
   DeleteCriticalSection(&mMutex);
#endif
}

void Mutex::Lock()
{
#ifndef WIN32
   pthread_mutex_lock(&mMutex);
#else
   EnterCriticalSection(&mMutex);
#endif
}

void Mutex::UnLock()
{
#ifndef WIN32
   pthread_mutex_unlock(&mMutex);
#else
   LeaveCriticalSection(&mMutex);
#endif
}
