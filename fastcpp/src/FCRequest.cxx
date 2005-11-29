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
 *  Foundation, Inc.,  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <httpd.h>
#include <http_protocol.h>
#include <http_config.h>
#include <http_core.h>
#include <http_request.h>
#include <http_vhost.h>
#include <http_main.h>
#include <http_log.h>
#include <apr_strings.h>
#define APACHE2
#ifdef APACHE2
#include <http_connection.h>
#include <util_filter.h>
#endif
#include <string>

#include "FastCPP.hxx"
#include "FCException.hxx"
#include "FCRequest.hxx"
#include "FCSession.hxx"
#include "Lock.hxx"
#include "Mutex.hxx"
#include "FCParser.hxx"

using namespace std;
using namespace FastCPP;

int hextable[256];

#define FASTCPPID "FASTCPPID"

FCRequest::FCRequest(request_rec *r):
   std::ostream (this),
   req(r),
   mContentType("text/html"),
   mCode(200),
   mSessions(0),
   mMethod(GET),
   mCreateSession(false),
   mRedirect(false),
   isInit(false)
{
   mFileName = r->filename;
   SetContentType(mContentType);
   setp(buffer,buffer+Size);
   req->status = mCode;
}

FCRequest::~FCRequest()
{
   if (mSessions)
   {
      mSessions->outUse();
   }
}
static 
int LookUpChar(const char *buffer, 
	       char ch, 
	       int offset)
{
   const char * lookUp;
				
   for (lookUp = buffer + offset; * lookUp; lookUp++)
      if (* lookUp == ch)
	 return lookUp - buffer;
   return -1;
}
static int  
ReverseFindChar(const char *cstring,char ch)
{
   const char * lookUp = cstring;
   int nWhere = -1;
   while(*lookUp)
   {
      if(*lookUp == ch)
      {
	 nWhere = lookUp - cstring;
      }
      lookUp ++;
   }
	
   return nWhere;		
}
void 
FCRequest::Init()
{
   if(isInit)
   {
      return;
   }
   isInit = true;
   // parse cookie
   ProcessCookie();
   // parse params (get or post)
   ProcessForm();
   // get session list
   StringMaps::iterator it = mCookies.find(FASTCPPID);
   if (it != mCookies.end())
   {
      mSessionID = it->second;
      Lock lock(_RSessionMutex);
      
      SessionsMaps::iterator sit = _sessionsMaps.find(it->second);
      if (sit != _sessionsMaps.end())
      { 
	 mSessions = sit->second;	 
	 Lock lock(mSessions->mMutex);
	 mSessions->expire = time(NULL) + FCSessions::DefaultExpire;
      }     
   }
   
   int pos = ReverseFindChar(req->parsed_uri.path,'/');
   if(pos >= 0)
   {
      string s(req->parsed_uri.path, pos + 1);
      mPagePath = s;
   }
   
   if (mSessions)
   {
      mSessions->inUse();
   }	
}

void
FCRequest::ProcessForm()
{
   if(req->method == NULL)
   {
      return ;
   }
	
   // When METHOD has no contents, the default action is to process it as GET method
   if (strcasecmp("GET", req->method) == 0)
   {      
      mMethod = GET;
      if(req->args)
      {
	 ProcessData(req->args, '=', '&');
      }
   }
   else if (strcasecmp("POST", req->method) == 0)
   {
      mMethod = POST;
      char *post_data;
      int content_length = 0;
      
#ifdef APACHE2		
      const char *tmp_data = apr_table_get(req->headers_in,"Content-length"); 
#else
      const char *tmp_data = ap_table_get(req->headers_in,"Content-length");
#endif
      if (!tmp_data)
	 return ;

      content_length = atoi(tmp_data);
		
		
      post_data = new char[content_length + 1];
      
      if(readData(post_data,content_length) != OK)
	 return ;
      ProcessData(post_data,'=', '&');
   }
}												
void 
FCRequest::ProcessCookie()
{
   size_t position;
   const char *cookies,*aux,*tmpcookies;
   const char *pcookies;
#ifdef APACHE2
   pcookies = apr_table_get(req->headers_in,"Cookie");
#else
   pcookies = ap_table_get(req->headers_in,"Cookie");
#endif
   if(!pcookies)
      return ;

   const string &s = Unencoded(pcookies);
   cookies = s.c_str();
   if(s.empty())
   {
      return;
   }
   aux = cookies;
   tmpcookies = cookies;
   
   while (cookies && *cookies != '\0') {

      position = 0;
		
      while (*aux++ != '=')
      {
	 position++;
      }

      string name(cookies, position);
      position = 0;
      cookies = aux;

      if ((strchr(cookies, ';')) == NULL) {
	 aux = NULL;
	 position = strlen(cookies);
      }
      else {
	 while (*aux++ != ';') 
	 {
	    position++;
	 }
	 // Eliminate the blank space after ';'
	 aux++;
      }
      string value(cookies, position);
      mCookies[name] = value;
      cookies = aux;
   }
}
												
void 
FCRequest::ProcessData(const char *query, 
		       char delim, 
		       char sep)
{
	
   register size_t position = 0, total_len = 0, i = 0;
   const char *aux;


   if (!query)
      return ;

   total_len = strlen(query);
   string s = Unencoded(query);
   query = s.c_str();
   aux = query;


   while (query && *query != '\0') 
   {

      position = 0;
		
      // Scans the string for the next 'delim' character
      while (*aux &&(*aux != delim)) {
	 position++;
	 aux++;
	 i++;
      }
      if (*aux) {
	 aux++;
	 i++;
      }		

      string name(query, position);
	
      query = aux;
      position = 0;
      while (*aux && (*aux != sep)) {
	 if ((*aux == '%') && (i + 2 <= total_len)) {
	    if (isalnum(aux[1]) && isalnum(aux[2])) {
	       aux += 2;
	       i += 2;
	       position++;
	    }
	 }
	 else			
	    position++;
				
	 aux++;
	 i++;
      }
	
      if (*aux) {
	 aux++;
	 i++;
      }
      
      if(position)
      {
	 string value(query, position);
	 mParams[name] = value;
      }
      else
      {
	 string value;
	 mParams[name] = value;
      }
	
      query = aux;
   }
}
int  
FCRequest::readData (char *buf_data,
		     unsigned int bufbytes)
{
#ifdef APACHE2
   apr_bucket_brigade *bb;
   conn_rec *c;
   int seen_eos;
   int rv;
   unsigned int dbpos = 0;


   c = req->connection;
   bb = apr_brigade_create(req->pool, c->bucket_alloc);
   seen_eos = 0;

   do {
      apr_bucket *bucket;

      rv = ap_get_brigade(req->input_filters, bb, AP_MODE_READBYTES,
			  APR_BLOCK_READ, HUGE_STRING_LEN);
       
      if (rv != APR_SUCCESS) {
	 return rv;
      }

      APR_BRIGADE_FOREACH(bucket, bb) {
	 const char *data;
	 apr_size_t len;

	 if (APR_BUCKET_IS_EOS(bucket)) {
	    seen_eos = 1;
	    break;
	 }

	 /* We can't do much with this. */
	 if (APR_BUCKET_IS_FLUSH(bucket)) {
	    continue;
	 }

 
	 /* read */
	 apr_bucket_read(bucket, &data, &len, APR_BLOCK_READ);
            
	 if ( dbpos < bufbytes) {
	    int cursize;

	    if ((dbpos + len) > bufbytes) {
	       cursize = bufbytes - dbpos;
	    }
	    else {
	       cursize = len;
	    }
	    memcpy(buf_data + dbpos, data, cursize);
	    dbpos += cursize;
	 }
      }
      apr_brigade_cleanup(bb);
   }
   while (!seen_eos);

   return APR_SUCCESS;
#else
   return OK;
#endif

}


int 
FCRequest::sync ()
{
   ap_rwrite(pbase(),pptr() - pbase(), req);
   setp(buffer, buffer+Size); 
   return 0;
}
int 
FCRequest::overflow (int c)
{
   sync();
   if (c != EOF) 
   {
      *pptr() = static_cast<unsigned char>(c);
      pbump(1);
   }
   return c;
}
static char hexmap[] = "0123456789abcdef";
string 
FCRequest::Unencoded(const string &data)
{
   string ret;

   const char* p = data.c_str();
   
   for (int i = 0; i < data.size(); ++i)
   {
      unsigned char c = *p++;
      if (c == '%')
      {
	 if ( i+2 < data.size())
	 {
	    char* high = strchr(hexmap, *p++);
	    char* low = strchr(hexmap, *p++);

	    if (high == 0 || low == 0)
	    {
	       return ret;
	    }
            
	    int highInt = high - hexmap;
	    int lowInt = low - hexmap;
	    ret += char(highInt<<4 | lowInt);
	    i += 2;
	 }
	 else
	 {
	    break;
	 }
      }
      else
      {
	 ret += c;
      }
   }
   return ret;
}
FCSession *
FCRequest::GetSession(const string &name)
{
   if (!mSessions)
      return 0;
   Lock lock(mSessions->mMutex);
   SessionMaps::iterator it = mSessions->mSessions.find(name);
   if (it != mSessions->mSessions.end())
   {
      return it->second;
   }
   return 0;
}

void
FCRequest::SetSession(const string &name, 
		      FCSession *session)
{  
   if (!mSessions)
   {
      //
      char id[FCSessions::SessionIDLen + 1];
      GenUUID(id, FCSessions::SessionIDLen);
      id[FCSessions::SessionIDLen] = 0;
      SetCookie(FASTCPPID, id);      
      mSessions = new FCSessions(id);
      mCreateSession = true;
      Lock lock(_RSessionMutex);
      _sessionsMaps[id] = mSessions;      
   }

   Lock lock(mSessions->mMutex);
   SessionMaps::iterator it = mSessions->mSessions.find(name);
   if (it != mSessions->mSessions.end())
   {
        delete it->second;
        mSessions->mSessions.erase(it);
   }
   if (session)
   {
       mSessions->mSessions[name] = session;
   }

}
	 
const string &
FCRequest::GetCookie(const string &name)
{
   static string s;
   StringMaps::const_iterator it = mCookies.find(name);
   if (it != mCookies.end())
   {
      return it->second;
   }
   return s;
}

void 
FCRequest::SetCookie(const string &name, 
		     const string &value, 
		     const string &path,
		     const string &domain,		     
		     int expire)
{
   string s = name;
   s +="=";   
   s += value;
   if(!path.empty())
   {
      s += ";path=";
      s += path;
   }
   if(!domain.empty())
   {
      s += ";domain=";
      s += domain;
   }
   if (expire >0)
   {
      s += ";expires=";
      s += FCParser::IntToChars(expire);      
   }
#ifdef APACHE2
   char *cookie = (char *)apr_palloc(req->pool,s.size() + 1);
#else
   char *cookie = (char *)ap_palloc(req->pool,s.size() + 1);
#endif
   memcpy(cookie, s.c_str(), s.size());
#ifdef APACHE2
   apr_table_add(req->err_headers_out, "Set-Cookie", cookie);
#else
   ap_table_add(req->err_headers_out, "Set-Cookie", cookie);
#endif
}

const string & 
FCRequest::GetParam(const string &name)
{
   static string s;
   StringMaps::const_iterator it = mParams.find(name);
   if (it != mParams.end())
   {
      return it->second;
   }
   return s;
}

   
void
FCRequest::SetContentType(const string &text)
{
   mContentType = text;
#ifdef APACHE2
   char *type = (char *)apr_palloc(req->pool,text.size()+1);
#else
  char *type = (char *)ap_palloc(req->pool,text.size()+1); 
#endif
   memcpy(type, text.c_str(), text.size());
   type[text.size()] = 0;
   req->content_type = type;
}

void
FCRequest::Redirect (const string &u)
{
#ifdef APACHE2
   char *url = (char *) apr_palloc(req->pool, u.size() + 1);
#else
   char *url = (char *) ap_palloc(req->pool, u.size() + 1);
#endif
   memcpy(url, u.c_str(), u.size());
#ifdef APACHE2   
   apr_table_add(req->err_headers_out, "Location", url);
#else
   ap_table_add(req->err_headers_out, "Location", url);
#endif
   mRedirect = true;
}

void  
FCRequest::Include (const string &u)
{
   return;
#ifdef APACHE2
   request_rec *rr;
   
   if(u.empty())
   {
      return;
   }

   rr = ap_sub_req_lookup_uri(u.c_str(), req, req->output_filters);

   rr->headers_in = req->headers_in;
   if (mCreateSession)
   {
      // the cookie doesn't  contains 'FASTCPPID'
      string s;
      s = "FASTCPPID=";
      s += mSessions->mId;
      
      int j = 0 ;

      const char *tmpCookie = apr_table_get(req->headers_in,"Cookie");
      if (tmpCookie)
      {
	 j = strlen(tmpCookie);
      }
      
      char *cookieStr = (char *) apr_palloc(req->pool,
					    j + s.size() +2);
      memset(cookieStr,0,j + s.size() +2);
      if(tmpCookie)
      {
	 memcpy(cookieStr,tmpCookie,j);
	 memcpy(cookieStr + j, ";",1);
	 j++;
      }
      memcpy(cookieStr + j, s.c_str(), s.size());
      apr_table_setn(rr->headers_in,"Cookie",cookieStr);		 
   }
 

   if (rr->status != HTTP_OK) {
      ap_destroy_sub_req(rr);
      return ;
   }
	
   /* No hardwired path info or query allowed */
	
   if ((rr->path_info && rr->path_info[0]) || rr->args) {
      ap_destroy_sub_req(rr);
      return ;
   }
   if (rr->finfo.filetype != APR_REG) {
      ap_destroy_sub_req(rr);
      return ;
   }
   ap_run_sub_req(rr);

   ap_destroy_sub_req(rr);
   return ;
#else

#endif
}

void  
FCRequest::CheckRedirect()
{
   if (mRedirect && mCode == 200)
   {  
      const char *location;
#ifdef APACHE2      
      location = apr_table_get(req->err_headers_out, "Location");
#else
      location = ap_table_get(req->err_headers_out, "Location");
#endif
      if (location && location[0] == '/' && req->status == 200)
      {
	 /* Internal redirect -- fake-up a pseudo-request */
	 req->status = HTTP_OK;

	 /* This redirect needs to be a GET no matter what the original
	  * method was.
	  */
	 req->method = apr_pstrdup(req->pool, "GET");
	 req->method_number = M_GET;

	 ap_internal_redirect_handler(location, req);
	 return;
      }
      else
      {
	 req->status = HTTP_MOVED_TEMPORARILY;
	 mCode = HTTP_MOVED_TEMPORARILY;
      }
   }
   return ;
}
void 
FCRequest::SetCode(int code)
{
   mCode = code;
   req->status = code;
}
