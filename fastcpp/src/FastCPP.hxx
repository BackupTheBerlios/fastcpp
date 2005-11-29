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

#ifndef _FASTCPP_HXX_
#define _FASTCPP_HXX_

#include <string>
#include <map>
#include <list>

#include "Mutex.hxx"
#include "FCPageFile.hxx"
#include "FCSession.hxx"


#define FCThrow(what) throw FCException(what,__FILE__,__LINE__)

extern FastCPP::Mutex _RPageMutex;
extern FastCPP::Mutex _WLruMutex;
extern FastCPP::Mutex _WRemovePageMutex;

extern FastCPP::Mutex _RSessionMutex;

//typedef map<std::string,FastCPP::FCPageFile*> PageFiles;

//extern PageFiles  _pages;
extern FastCPP::PageCaches _pages;

extern list<FastCPP::FCPageFile*>  _removePages;

extern FastCPP::SessionsMaps _sessionsMaps;

void GenUUID(char *buf,int len);
time_t GetFileMTime(const std::string &file);

#define PIPE_RD	0
#define PIPE_WR	1

int FCExec(const std::string &prog,
	   const std::string &params,
	   int *out,
	   int *in,
	   int *err);

#endif //_FASTCPP_HXX_
