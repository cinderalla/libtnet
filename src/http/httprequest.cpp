#include "httprequest.h"

#include <string.h>
#include <stdlib.h>
#include <vector>

extern "C"
{
#include "http_parser.h"    
}

#include "httputil.h"
#include "stringutil.h"
#include "log.h"

using namespace std;

namespace tnet
{

    HttpRequest::HttpRequest()
    {
        majorVersion = 1;
        minorVersion = 1;
    }
   
    HttpRequest::~HttpRequest()
    {
    } 

    void HttpRequest::clear()
    {
        url.clear();
        schema.clear();
        host.clear();
        path.clear();
        query.clear();
        body.clear();
        
        headers.clear();
        params.clear();
        
        majorVersion = 1;
        minorVersion = 1;
        method = HTTP_GET;        
        port = 80;
    }

    void HttpRequest::parseUrl()
    {
        struct http_parser_url u;
        if(http_parser_parse_url(url.c_str(), url.size(), 0, &u) != 0)
        {
            LOG_ERROR("parseurl error %s", url.c_str());
            return;    
        }

        if(u.field_set & (1 << UF_SCHEMA))
        {
            schema = url.substr(u.field_data[UF_SCHEMA].off, u.field_data[UF_SCHEMA].len);    
        }
        
        if(u.field_set & (1 << UF_HOST))
        {
            host = url.substr(u.field_data[UF_HOST].off, u.field_data[UF_HOST].len);    
        }

        if(u.field_set & (1 << UF_PORT))
        {
            port = u.port;    
        }
        else
        {
            if(strcasecmp(schema.c_str(), "https") == 0 || strcasecmp(schema.c_str(), "wss") == 0)
            {
                port = 443;    
            }
            else
            {
                port = 80; 
            }   
        }

        if(u.field_set & (1 << UF_PATH))
        {
            path = url.substr(u.field_data[UF_PATH].off, u.field_data[UF_PATH].len);    
        }

        if(u.field_set & (1 << UF_QUERY))
        {
            query = url.substr(u.field_data[UF_QUERY].off, u.field_data[UF_QUERY].len);    
        } 
        
    }

    void HttpRequest::parseQuery()
    {
        static string sep1 = "&";
        static string sep2 = "=";

        vector<string> args = StringUtil::split(query, sep1);
        string key;
        string value;
        for(size_t i = 0; i < args.size(); ++i)
        {
            vector<string> p = StringUtil::split(args[i], sep2);
            if(p.size() == 2)
            {
                key = p[0];
                value = p[1]; 
            }    
            else if(p.size() == 1)
            {
                key = p[0];
                value = "";    
            }
            else
            {
                //invalid, ignore
                continue;    
            }

            params.insert(make_pair(HttpUtil::unescape(key), HttpUtil::unescape(value)));
        }
           
    }
}
