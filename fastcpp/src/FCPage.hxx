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
 *  Foundation, Inc.,51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#ifndef _FCPAGE_HXX_
#define _FCPAGE_HXX_
#include <map>
#include "FCSession.hxx"
#include "Mutex.hxx"

using namespace std;

namespace FastCPP
{
   class FCRequest;
   class FCPage 
   {
      public:
	 FCPage(FCRequest &r, const string &pagename);
	 virtual ~FCPage();
	 void Run();
	 virtual void service() = 0;

      protected:
	 FCRequest &page;
	 const string &mPageName;
   };
};

#endif // _FCPAGE_HXX_
