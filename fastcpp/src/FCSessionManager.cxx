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

#include "FastCPP.hxx"
#include "FCException.hxx"
#include "FCThread.hxx"
#include "Lock.hxx"
#include "Mutex.hxx"

#include "FCSessionManager.hxx"
#include "FCSession.hxx"

using namespace FastCPP;

void FCSessionManager::thread()
{
   while(!IsShutDown())
   {
      {
	 Lock lock(_RSessionMutex);
      
	 for (SessionsMaps::iterator sit = _sessionsMaps.begin();
	      sit != _sessionsMaps.end();
	      sit ++)
	 {
	    if (sit->second->isTimeOut() && !sit->second->isUse())
	    {
	       _sessionsMaps.erase(sit);
	       delete sit->second;
	       break; // delete one session
	    }
	 }
      }
      sleep(5);
   }
}
