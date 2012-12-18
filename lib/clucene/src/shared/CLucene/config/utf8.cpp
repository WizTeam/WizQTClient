/*
 * Copyright (C) 1999 Tom Tromey
 * Copyright (C) 2000 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *
 ************************************************
 * Also licensed with permission from Tom Tromey
 * and Owen Taylor under the Apache license.
 * Original location:
 * http://cvs.gnome.org/viewcvs/glib/glib/gutf8.c?rev=1.50&view=log
 ************************************************
 *
 * Copyright 2003-2006 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 #include "CLucene/_SharedHeader.h"

typedef unsigned long  gunichar;
typedef unsigned char  guchar;

#define UTF8_COMPUTE(Char, Mask, Len)					      \
  if (Char < 128)							      \
    {									      \
      Len = 1;								      \
      Mask = 0x7f;							      \
    }									      \
  else if ((Char & 0xe0) == 0xc0)					      \
    {									      \
      Len = 2;								      \
      Mask = 0x1f;							      \
    }									      \
  else if ((Char & 0xf0) == 0xe0)					      \
    {									      \
      Len = 3;								      \
      Mask = 0x0f;							      \
    }									      \
  else if ((Char & 0xf8) == 0xf0)					      \
    {									      \
      Len = 4;								      \
      Mask = 0x07;							      \
    }									      \
  else if ((Char & 0xfc) == 0xf8)					      \
    {									      \
      Len = 5;								      \
      Mask = 0x03;							      \
    }									      \
  else if ((Char & 0xfe) == 0xfc)					      \
    {									      \
      Len = 6;								      \
      Mask = 0x01;							      \
    }									      \
  else									      \
    Len = -1;

/*#define UTF8_LENGTH(Char)              \
  ((Char) < 0x80 ? 1 :                 \
   ((Char) < 0x800 ? 2 :               \
    ((Char) < 0x10000 ? 3 :            \
     ((Char) < 0x200000 ? 4 :          \
      ((Char) < 0x4000000 ? 5 : 6)))))*/


#define UTF8_GET(Result, Chars, Count, Mask, Len)			      \
  (Result) = (Chars)[0] & (Mask);					      \
  for ((Count) = 1; (Count) < (Len); ++(Count))				      \
    {									      \
      if (((Chars)[(Count)] & 0xc0) != 0x80)				      \
		{								      \
			(Result) = -1;						      \
			break;							      \
		}								      \
      (Result) <<= 6;							      \
      (Result) |= ((Chars)[(Count)] & 0x3f);				      \
    }


/**
 * lucene_wctoutf8:
 * @c: a ISO10646 character code
 * @outbuf: output buffer, must have at least 6 bytes of space.
 *       If %NULL, the length will be computed and returned
 *       and nothing will be written to @outbuf.
 *
 * Converts a single character to UTF-8.
 *
 * Return value: number of bytes written
 **/
size_t	lucene_wctoutf8(char * outbuf, const wchar_t ch)
{
  gunichar c = ch;
  guchar len = 0;
  int first;
  int i;

  if (c < 0x80)
    {
      first = 0;
      len = 1;
    }
  else if (c < 0x800)
    {
      first = 0xc0;
      len = 2;
    }
  else if (c < 0x10000)
    {
      first = 0xe0;
      len = 3;
    }
   else if (c < 0x200000)
    {
      first = 0xf0;
      len = 4;
    }
  else if (c < 0x4000000)
    {
      first = 0xf8;
      len = 5;
    }
  else
    {
      first = 0xfc;
      len = 6;
    }

  if (outbuf)
  {
	for (i = len - 1; i > 0; --i)
	{
		outbuf[i] = (char)((c & 0x3f) | 0x80);
		c >>= 6;
	}
	outbuf[0] = (char)(c | first);
  }

  return len;
}


/**
 * lucene_utf8towc:
 * @p: a pointer to Unicode character encoded as UTF-8
 *
 * Converts a sequence of bytes encoded as UTF-8 to a Unicode character.
 * If @p does not point to a valid UTF-8 encoded character, results are
 * undefined. If you are not sure that the bytes are complete
 * valid Unicode characters, you should use lucene_utf8towc_validated()
 * instead.
 *
 * Return value: the number of p consumed for the character, or 0 on error
 **/
size_t lucene_utf8towc(wchar_t& pwc, const char *p)
{
  int i, mask = 0;
  int result;
  unsigned char c = (unsigned char) *p;
  int len=0;

  UTF8_COMPUTE (c, mask, len);
  if (len == -1)
    return 0;
  UTF8_GET (result, p, i, mask, len);

  pwc = result;
  return len;
}


//this function was not taken from gnome
size_t lucene_wcstoutf8(char * result, const wchar_t * str, size_t result_length){
  char *p=result;
  int i = 0;

  while (p < result + result_length-1 && str[i] != 0)
    p += lucene_wctoutf8(p,str[i++]);

  *p = '\0';

  return p-result;
}
//this function was not taken from gnome
size_t lucene_utf8towcs(wchar_t * result, const char * str, size_t result_length){
  char *sp = const_cast<char*>(str);
  wchar_t *rp = result;

  while (rp < result + result_length && *sp!=0){
    size_t r = lucene_utf8towc(*rp,sp);
    if ( r == 0 )
      return 0;
    sp += r;
    rp++;
  }

  size_t ret = sp-str;
  if ( ret < result_length )
	*rp = '\0';

  return ret;
}
//get the number of bytes that make up the utf8 character.
//this function was not taken from gnome
size_t lucene_utf8charlen(const unsigned char c)
{
  int mask = 0;
  int len=0;

  UTF8_COMPUTE (c, mask, len);
  return len;
}
#ifndef _ASCII
//convert unicode string to a utf8 string
std::string lucene_wcstoutf8string(const wchar_t* str, size_t strlen){
  size_t i = 0;
  std::string result;
  char p[6];

  while (i < strlen && str[i] != 0){
    result.append(p, lucene_wctoutf8(p,str[i++]));
  }

  return result;
}
#endif
