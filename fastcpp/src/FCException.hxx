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


#ifndef _FCEXCEPTION_HXX_
#define _FCEXCEPTION_HXX_

#include <exception>
#include <iostream>

namespace FastCPP
{
   class FCException : public std::exception
   {
      public:
	 FCException(const std::string &what,
		     const std::string &file,
		     int line);
	 virtual ~FCException() throw();
	 std::string& what();
      protected:
	 std::string mWhat;

	 friend std::ostream& operator<<(std::ostream& strm, const FCException& e);
   };
};

#endif //_FCEXCEPTION_HXX_
