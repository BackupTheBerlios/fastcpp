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

#ifndef _FCPARSER_HXX_
#define _FCPARSER_HXX_
#include <iostream>

namespace FastCPP
{
   class FCPageFile;
   class FCParser
   {
      public:
	 enum
	 {
	    HTML,
	    CODE,
	    INC,
	    LINK,
	    COMP,
	    GLOBAL,
	    INCLUDE,
	    COMMENT,
	    CLASS
	 };

	 FCParser(const std::string &filename,int i = 0); // The filename should be an absolute path.
	 void Parse();
	 static char *IntToChars(int val);
	 static int MaxIncludeLevel ;
      protected:
	 const char *GetMacroData(const char *buf,
				  std::string &s,
				  const char *errText);
	 const char *SkipChar(const char *buf, int step = 1);

	 const char *ParseClass(const char *buf);
	 const char *ParseHtml(const char *buf);
	 const char *ParseCode(const char *buf);
	 const char *ParseInc(const char *buf);
	 const char *ParseLink(const char *buf);
	 const char *ParseComp(const char *buf);
	 const char *ParseGlobal(const char *buf);
	 const char *ParseInclude(const char *buf);	
	 const char *ParseComment(const char *buf);
      protected:
	 std::string mInc; // include path
	 std::string mLink; // lib path
	 std::string mComp; // compile param
	 std::string mGlobal; // global str
	 std::string mCode; // html and cpp code
	 std::string mClass;
	 std::string mFileName;
	 int line;
	 int column;
	 int lastTagLine;
	 int lastTagColumn;
	 int level;
	 int  mState;
	 friend std::ostream& operator<<(std::ostream& strm, const FCParser& p);
	 friend class FCPageFile;
	 
	 
   };   
};

#endif //_FCPARSER_HXX_
