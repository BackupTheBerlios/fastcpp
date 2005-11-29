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

#ifndef _FCREQUEST_HXX_
#define _FCREQUEST_HXX_
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "FCSession.hxx"

using namespace std;

struct request_rec;

namespace FastCPP
{
   class FCPage;


   class FCRequest: public std::streambuf,
		    public std::ostream
   {
      public:
	 enum
	 {
	    GET,
	    POST
	 };	 
	 FCRequest(request_rec *r);
	 ~FCRequest();
	 const string &FileName()
	 {
	    return mFileName;
	 }

	 FCSession *GetSession(const string &name);

	 void SetSession(const string &name, 
			 FCSession *session);

	 const string &GetCookie(const string &name);
	 void SetCookie(const string &name, 
			const string &value,
			const string &path = "",
			const string &domain = "",
			int expire = 0);
	 const string &GetParam(const string &name);
	 static string Unencoded(const string &data);
	 void SetContentType(const string &text);
	 void Redirect (const string &u);
	 void Include (const string &u);
	 void CheckRedirect();
	 void SetCode(int code = 200);
	 int GetCode()
	 {
	    return mCode;
	 }
	 void Init();
	 int GetMethod()
	 {
	    return mMethod;
	 }
	 const string &GetSessionID()
	 {
	    return mSessionID;
	 }
      private:

	 int sync ();
	 int overflow (int c);

	 void ProcessCookie();
	 void ProcessForm();
	 void ProcessData(const char *query, 
			  char delim, 
			  char sep);
	 int readData (char *buf_data,
		       unsigned int bufbytes);

      protected:	 
	 typedef map<string,string> StringMaps;
	 typedef map<string,FCSession *> SessionMaps;
	 StringMaps mParams;
	 StringMaps mCookies;
	 FCSessions *mSessions;
	 
	 int mMethod;
	 string mSessionID;
	 string mPagePath;
	 string mFileName;
	 request_rec *req;

	 string mContentType;
	 int mCode;
	 bool mCreateSession;
	 bool mRedirect;
      private:
	 enum { Size=4095 }; 
	 char buffer[Size+1];
	 bool isInit;
	 friend class FCPage;
   };
};
#endif //_FCREQUEST_HXX_
