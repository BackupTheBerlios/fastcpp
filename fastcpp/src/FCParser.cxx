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

#include <assert.h>
#include <iostream>
#include <fstream>

#include "FastCPP.hxx"
#include "FCException.hxx"
#include "FCParser.hxx"

using namespace std;
using namespace FastCPP;

#define PAGE_OUT  "page<< \""
#define PAGE_OUT_END  "\" <<endl;\n"

#define PAGE_START \
"#include <iostream>\n"\
"\n"\
"#include <fastcpp/FastCPP.hxx>\n"\
"#include <fastcpp/FCException.hxx>\n"\
"#include <fastcpp/FCSession.hxx>\n"\
"#include <fastcpp/FCPage.hxx>\n"\
"#include <fastcpp/FCRequest.hxx>\n"\
"#include <fastcpp/Mutex.hxx>\n"\
"#include <fastcpp/Lock.hxx>\n"\
"\n"\
"using namespace FastCPP;\n"\
"using namespace std;\n"

#define PAGE_CLASS_START \
"\n"\
"class ThisPage :public FCPage\n"\
"{\n"\
"public:\n"\
"ThisPage(FCRequest &r, const string &pagename):FCPage(r,pagename)"\
"{}\n"
#define PAGE_SERVICE_START \
"      void service()\n"\
"      {\n"


#define PAGE_SERVICE_END \
"\n"\
"      }\n"\
"};\n"


#define PAGE_END \
"\n"\
"extern \"C\"\n"\
"{\n"\
"   void FCService(FCRequest &request)\n"\
"   {\n"\
"      ThisPage thispage(request, request.FileName());\n"\
"      thispage.Run();\n"\
"   }\n"\
"};\n"

#define POWER_TEXT "\n//Powered by FastCPP.\n"

#define ParserThrow(what) \
	    string str = what; \
            str += " IN "; \
	    str += mFileName; \
	    str += ",line :"; \
	    str += IntToChars(line); \
	    str += ",column:"; \
	    str += IntToChars(column); \
	    FCThrow(str);


int FCParser::MaxIncludeLevel = 6;
FCParser::FCParser(const string &filename, int i):
   mFileName(filename),
   line(0),
   column(0),
   lastTagLine(0),
   lastTagColumn(0),
   level(i),
   mState(HTML)
{
   if (level >= MaxIncludeLevel)
   {
      FCThrow("The Include level is Too MAX!");
   }
}

void FCParser::Parse()
{
   ifstream inFile(mFileName.c_str());
   string s;

   const char *buf = NULL;

   if(!inFile.is_open())
   {
      string what="Can't open the source : ";
      what += mFileName;
      FCThrow(what);
   }
   
   while(getline(inFile,s))
   {
      buf = s.c_str();
      line++;
      column = 0;
     parser_start:
      
      switch(mState)
      {
	 case HTML:
	    buf = ParseHtml(buf);
	    break;
	 case CODE:
	    buf = ParseCode(buf);
	    break;
	 case INC:
	    buf = ParseInc(buf);	   
	    break;	
	 case LINK:
	    buf = ParseLink(buf);	   
	    break; 
	 case COMP:
	    buf = ParseComp(buf);	   
	    break;     
	 case GLOBAL:
	    buf = ParseGlobal(buf);	   
	    break;    
	 case INCLUDE:
	    buf = ParseInclude(buf);	  
	    break;
	 case COMMENT:
	    buf = ParseComment(buf);
	    break;
	 case CLASS:
	    buf = ParseClass(buf);
	    break;	    
	 default:
	    ParserThrow("Unknow File Format!");
      }
      if (buf && *buf !='\0')
      {
	 goto parser_start;
      }   
   
   }

   if (mState != HTML)
   {
      // one tag no closed,may be CODE or COMMENT ,or GLOBAL
      string what = "One tag no closed ";
      what += mFileName;
      what += ",line: ";
      what += IntToChars(lastTagLine);
      what += ",colnum: ";
      what += IntToChars(lastTagColumn);
      FCThrow(what);
   }
   
}

char *FCParser::IntToChars(int val)
{
   static char buf[12];
   memset(buf,0,sizeof(buf));

   if (val == 0)
   {
      buf[0] = '0';
      buf[1] = 0;
      return buf;
   }

   bool neg = false;
   
   int value = val;
   if (value < 0)
   {
      value = -value;
      neg = true;
   }

   int c = 0;
   int v = value;
   while (v /= 10)
   {
      ++c;
   }

   if (neg)
   {
      ++c;
   }

   buf[c+1] = 0;
   
   v = value;
   while (v)
   {
      buf[c--] = '0' + v%10;
      v /= 10;
   }

   if (neg)
   {
      buf[0] = '-';
   }

   return buf;
}

const char *
FCParser::GetMacroData(const char *buf,
		       string &s,
		       const char *errText)
{
   while (*buf)
   {
      if (*buf == '\"')
      {
	 if ((buf = SkipChar(buf)))
	 {
	    while(*buf && *buf != '\"')
	    {
	       s += *buf;	       
	       buf = SkipChar(buf);
	    }
	    if (*buf)
	    {	       
	       buf = SkipChar(buf); 
	       while(*buf && strncmp(buf,"%>", 2) != 0)
	       {
		  buf = SkipChar(buf);  
	       }
	       if (*buf)
	       {
		  buf = SkipChar(buf, 2);
		  mState = HTML;
		  break;
	       }
	    }	    
	 }
	 ParserThrow(errText);
      }
      buf = SkipChar(buf);
   }
   return buf;
}

const char *
FCParser::SkipChar(const char *buf, int step)
{
   if (!buf)
   {
      return 0;
   }
   buf += step;
   column += step;
   return buf;
}

const char *
FCParser::ParseHtml(const char *buf)
{
   string s;

   while(*buf)
   {
      if (strncmp(buf,"<%",2) == 0)
      {
	 lastTagLine = line;
	 lastTagColumn = column;
	 if (!s.empty())
	 {
	    mCode += PAGE_OUT;
	    s += PAGE_OUT_END;
	    mCode += s;
	 }

	 if((buf = SkipChar(buf, 2)))
	 {
	    // update mSate
	    mState = CODE;
	    if (*buf == '@')
	    { 
	       if (( buf = SkipChar(buf)))
	       {
		  
		  if (strncmp(buf,"include", 7) == 0)
		  {
		     buf = SkipChar(buf, 7);
		     mState = INCLUDE;
		  }else if (strncmp(buf,"inc", 3) == 0)
		  {
		     buf = SkipChar(buf, 3);
		     mState = INC;
		  }else if (strncmp(buf,"link", 4) == 0)
		  {
		     buf = SkipChar(buf, 4);
		     mState = LINK;
		  }else if (strncmp(buf,"comp", 4) == 0)
		  {
		     buf = SkipChar(buf, 4);
		     mState = COMP;
		  } else if (strncmp(buf,"class",5) == 0)
		  {
		     buf = SkipChar(buf, 5);
		     mState = CLASS;   
		  } else
		  {
		     //unknown tag
		     mState = -1;
		  }
	       
	       }
	    } 
	    else if (*buf == '!')
	    {
	       if (( buf = SkipChar(buf)))
	       {
		  if (strncmp(buf,"--", 2) == 0)
		  {
		     buf = SkipChar(buf, 2);
		     mState = COMMENT;
		  }
		  else
		  {
		     mState = GLOBAL;
		  }
	       }
	    }	   
	 }
	 return buf;
      }
      if (*buf == '%')
      {
	 s += "%%";
      }
      else if (*buf == '\"')
      {
	 s += "\\\"";
      } 
      else if (*buf == '\'')
      {
	 s += "\\\'";
      }
      else
      {
	 s+= *buf;
      }
      buf = SkipChar(buf);
   }
   if (!s.empty())
   {
      mCode += PAGE_OUT;
      s += PAGE_OUT_END;
      mCode += s;
   }
   return buf;
}
const char *
FCParser::ParseCode(const char *buf)
{

   string lineInfo("#line ");
   lineInfo += IntToChars(line);
   lineInfo += " \"";
   lineInfo += mFileName;   
   lineInfo += "\"\n";

   string s;

   while (*buf)
   {
      if (strncmp(buf,"%>", 2) == 0)
      {
	 buf = SkipChar(buf, 2);
	 mState = HTML;
	 if (!s.empty())
	 {
	    mCode += lineInfo;
	    mCode += s;
	    mCode += '\n';
	 }
	 return buf;
      }
      s += *buf;
      buf = SkipChar(buf);
   }
   if (!s.empty())
   {
      mCode += lineInfo;
      mCode += s;
      mCode += '\n';
   }
   return buf;
}

const char *
FCParser::ParseClass(const char *buf)
{

   string lineInfo("#line ");
   lineInfo += IntToChars(line);
   lineInfo += " \"";
   lineInfo += mFileName;   
   lineInfo += "\"\n";

   string s;

   while (*buf)
   {
      if (strncmp(buf,"%>", 2) == 0)
      {
	 buf = SkipChar(buf, 2);
	 mState = HTML;
	 if (!s.empty())
	 {
	    mClass += lineInfo;
	    mClass += s;
	    mClass += '\n';
	 }
	 return buf;
      }
      s += *buf;
      buf = SkipChar(buf);
   }
   if (!s.empty())
   {
      mClass += lineInfo;
      mClass += s;
      mClass += '\n';
   }
   return buf;
}


const char *
FCParser::ParseInc(const char *buf)
{
   string s(" ");
   buf = GetMacroData(buf, s, "<%@inc Format Error.");
   mInc += s;
   return buf;
}

const char *
FCParser::ParseLink(const char *buf)
{
   string s(" ");
   buf = GetMacroData(buf, s, "<%@link Format Error.");
   mLink += s;
   return buf;
}

const char *
FCParser::ParseComp(const char *buf)
{
   string s(" ");
   buf = GetMacroData(buf, s, "<%@comp Format Error.");
   mComp += s;
   return buf;
}
const char *
FCParser::ParseGlobal(const char *buf)
{

   string lineInfo("#line ");
   lineInfo += IntToChars(line);
   lineInfo += " \"";
   lineInfo += mFileName;   
   lineInfo += "\"\n";


   string s;

   while(*buf)
   {
      if (strncmp(buf,"%>",2) == 0)
      {
	 buf = SkipChar(buf,2);
	 mState = HTML;

	 if (!s.empty())
	 {
	    mGlobal += lineInfo;
	    mGlobal += s;
	    mGlobal += '\n';
	 }

	 if (!s.empty())
	 {
	    mGlobal += s;
	    mGlobal += "\n";
	 }
	 return buf;
      }
      s+= *buf;
      buf = SkipChar(buf);
   }
   if (!s.empty())
   {
      mGlobal += lineInfo;
      mGlobal += s;
      mGlobal += '\n';
   }
   return buf;
}
const char *
FCParser::ParseInclude(const char *buf)
{
   string s;
   buf = GetMacroData(buf, s, "<%@include Format Error.");
   // get this file's path
   string path;
#ifdef WIN32
   path = mFileName.substr(0,mFileName.rfind('\\')+1);
#else
   path = mFileName.substr(0,mFileName.rfind('/')+1);
#endif
   string newFile = path;
   newFile +=s;

   try
   {
      FCParser p(newFile,level+1); 
      p.Parse();
   
      mCode += p.mCode;
      mInc += p.mInc;
      mLink += p.mLink;
      mComp += p.mComp;
      mGlobal += p.mGlobal;  
      mClass += p.mClass;
   }
   catch(FCException &e)
   {
      string s("Include Error:");
      s+= e.what();
      ParserThrow(s);
   }
}
const char *
FCParser::ParseComment(const char *buf)
{
   while(*buf)
   {
      if (strncmp(buf,"--%>",4) == 0)
      {
	 buf = SkipChar(buf,4);
	 mState = HTML;
	 break;
      }
      buf = SkipChar(buf);
   }
   return buf;
}

ostream& 
FastCPP::operator<<(std::ostream& strm,
		    const FCParser& p)
{
   strm<<"// This page created from "<<p.mFileName<<endl;
   strm<<PAGE_START<<endl;

   strm<<p.mGlobal<<endl;

   strm<<PAGE_CLASS_START<<endl;
   strm<<p.mClass<<endl;
   strm<<PAGE_SERVICE_START<<endl;
   strm<<p.mCode<<endl;
   strm<<PAGE_SERVICE_END<<endl;

   strm<<PAGE_END<<endl;
   strm<<POWER_TEXT<<endl;
   return strm;
}
