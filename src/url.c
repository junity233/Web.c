/* -*- encoding: utf-8; -*- */
/* -*- c-mode -*- */
/* File-name:    <url.c> */
/* Author:       <Xsoda> */
/* Create:       <Friday December 20 12:38:02 2013> */
/* Time-stamp:   <Wednesday December 25, 10:10:57 2013> */
/* Mail:         <Xsoda@Live.com> */

#include "url.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// static char *strndup(const char *str, int n)
// {
//    char *dst;
//    if (!str) return NULL;
//    if (n < 0) n = strlen(str);
//    if (n == 0) return NULL;
//    if ((dst = (char *)malloc(n + 1)) == NULL)
//       return NULL;
//    memcpy(dst, str, n);
//    dst[n] = 0;
//    return dst;
// }

void parse_query(url_field_t *url, const char *query)
{
   char *chr;
   chr = strchr(query, '=');
   while (chr)
   {
      if (url->query)
         url->query = realloc(url->query, (url->query_num + 1) * sizeof(*url->query));
      else
         url->query = malloc(sizeof(*url->query));
      url->query[url->query_num].name = strndup(query, chr - query);
      query = chr + 1;
      chr = strchr(query, '&');
      if (chr)
      {
         url->query[url->query_num].value = strndup(query, chr - query);
         url->query_num++;
         query = chr + 1;
         chr = strchr(query, '=');
      }
      else
      {
         url->query[url->query_num].value = strndup(query, strlen(query));
         url->query_num++;
         break;
      }
   }
}
url_field_t *url_parse (const char *str)
{
   const char *pch;
   char *query;
   url_field_t *url;
   query = NULL;
   if ((url = (url_field_t *)malloc(sizeof(url_field_t))) == NULL)
      return NULL;
   memset(url, 0, sizeof(url_field_t));
   if (str && str[0])
   {
      pch = strchr(str, '?');
      if (pch)
      {
         url->path = strndup(str, pch - str);
         str = pch + 1;
         pch = strchr(str, '#');
         if (pch)
         {
            query = strndup(str, pch - str);
            str = pch + 1;
            url->fragment = strndup(str, strlen(str));
         }
         else
         {
            query = strndup(str, strlen(str));
            str = str + strlen(str);
         }
         parse_query(url, query);

         free(query);
      }
      else
      {
         pch = strchr(str, '#');
         if (pch)
         {
            url->path = strndup(str, pch - str);
            str = pch + 1;
            url->fragment = strndup(str, strlen(str));
            str = str + strlen(str);
         }
         else
         {
            url->path = strndup(str, strlen(str));
            str = str + strlen(str);
         }
      }
      
   }
   else
   {
      url_free(url);
      return NULL;
   }
   return url;
}

void url_free(url_field_t *url)
{
   if (!url) return;
   if (url->path) free(url->path);
   if (url->query)
   {
      int i;
      for (i = 0; i < url->query_num; i++)
      {
         free(url->query[i].name);
         free(url->query[i].value);
      }
      free(url->query);
   }
   if (url->fragment) free(url->fragment);
   free(url);
}

#if 0
int main()
{
   url_field_t *url = url_parse("schema://usr:pwd@localhost:port/path?a=b&c=d&e=f#foo");
   url_field_print(url);
   url_free(url);
}
#endif

