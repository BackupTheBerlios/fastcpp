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

#ifndef _FCTHREAD_HXX_
#define _FCTHREAD_HXX_
#include <pthread.h>
#include "FastCPP.hxx"
#include "FCException.hxx"
#include "Mutex.hxx"
#include "Lock.hxx"

namespace FastCPP
{
   class FCThread
   {
      public:
	 FCThread():
	    mShutDown(false) 
	 {	    
	 }
	 ~FCThread()
	 {
	    ShutDown();
	    Join();
	 }
	 void Run()
	 {  
	    // spawn the thread
	    if ( int retval = pthread_create( &mId, 0,FCThread::ThreadFunc , this) )
	    {
	       FCThrow("Failed to spawn thread");
	    }	 
	 }
	 void Join()
	 {
	    void* stat;
	    pthread_join( mId , &stat );
	 }
	 bool IsShutDown()
	 {
	    Lock lock(mShutdownMutex);
	    return mShutDown;
	 }
      
	 void ShutDown()
	 {
	    Lock lock(mShutdownMutex);
	    mShutDown = true;
	 }
	 virtual void thread() = 0;
	 static void *ThreadFunc(void  *t)
	 {
	    FCThread *thr = static_cast<FCThread *>(t);
	    thr->thread();	    
	 }
      protected:
	 bool mShutDown;
	 Mutex mShutdownMutex;
	 pthread_t mId;
   };
};
#endif
