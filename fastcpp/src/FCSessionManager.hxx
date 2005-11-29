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
// manager  session
#ifndef _FCSESSIONMANAGER_HXX_
#define _FCSESSIONMANAGER_HXX_

#include "FastCPP.hxx"
#include "FCException.hxx"
#include "FCThread.hxx"
#include "Lock.hxx"
#include "Mutex.hxx"

namespace FastCPP
{
   class FCSessionManager : public FCThread
   {
      public:
	 void thread();
   };
};
#endif // _FCSESSIONMANAGER_HXX_
