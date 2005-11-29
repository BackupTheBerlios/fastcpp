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

#include <stdlib.h>

#include <httpd.h>
#include <http_config.h>
#include <http_protocol.h>
#include <exception>
extern int InitFastCPP();

extern int doRequest(request_rec *r);
extern "C"
{
   static int mod_fastcpp_method_handler (request_rec *r)
   {   
      static int i = 0;
      if (!i)
      {
	 InitFastCPP();
	 i++;
      }
      if (strcmp("fastcpp-script",r->handler))
	 return DECLINED;

      if (r->finfo.filetype == 0) 
      {
	 return HTTP_NOT_FOUND;
      }
      return doRequest(r);   
   }

   static void mod_fastcpp_register_hooks (apr_pool_t *p)
   {	  
      ap_hook_handler(mod_fastcpp_method_handler, NULL, NULL, APR_HOOK_LAST);
   }
   module AP_MODULE_DECLARE_DATA fastcpp_module =
   {	
      STANDARD20_MODULE_STUFF, // standard stuff; no need to mess with this
      NULL, // create per-directory configuration structures - we do not.
      NULL, // merge per-directory - no need to merge if we are not creating anything.
      NULL, // create per-server configuration structures.
      NULL, // merge per-server - hrm - examples I have been reading don't bother with this for trivial cases.
      NULL, // configuration directive handlers
      mod_fastcpp_register_hooks, // request handlers
   };
};
