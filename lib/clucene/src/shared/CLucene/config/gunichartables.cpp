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
 * http://cvs.gnome.org/viewcvs/glib/glib/guniprop.c?view=log
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
typedef unsigned short guint16;
typedef          short gint16;
typedef          char  gchar;
typedef unsigned char  guchar;

/* These are the possible character classifications.
 * See http://www.unicode.org/Public/UNIDATA/UnicodeData.txt
   or http://www.unicode.org/Public/UNIDATA/UCD.html.
      
   todo: i think there is a new version of the unicode, which we should use.
   data is licensed like this: http://www.unicode.org/copyright.html... not sure but looks apache compatible
 */
typedef enum
{
  G_UNICODE_CONTROL,
  G_UNICODE_FORMAT,
  G_UNICODE_UNASSIGNED,
  G_UNICODE_PRIVATE_USE,
  G_UNICODE_SURROGATE,
  G_UNICODE_LOWERCASE_LETTER,
  G_UNICODE_MODIFIER_LETTER,
  G_UNICODE_OTHER_LETTER,
  G_UNICODE_TITLECASE_LETTER,
  G_UNICODE_UPPERCASE_LETTER,
  G_UNICODE_COMBINING_MARK,
  G_UNICODE_ENCLOSING_MARK,
  G_UNICODE_NON_SPACING_MARK,
  G_UNICODE_DECIMAL_NUMBER,
  G_UNICODE_LETTER_NUMBER,
  G_UNICODE_OTHER_NUMBER,
  G_UNICODE_CONNECT_PUNCTUATION,
  G_UNICODE_DASH_PUNCTUATION,
  G_UNICODE_CLOSE_PUNCTUATION,
  G_UNICODE_FINAL_PUNCTUATION,
  G_UNICODE_INITIAL_PUNCTUATION,
  G_UNICODE_OTHER_PUNCTUATION,
  G_UNICODE_OPEN_PUNCTUATION,
  G_UNICODE_CURRENCY_SYMBOL,
  G_UNICODE_MODIFIER_SYMBOL,
  G_UNICODE_MATH_SYMBOL,
  G_UNICODE_OTHER_SYMBOL,
  G_UNICODE_LINE_SEPARATOR,
  G_UNICODE_PARAGRAPH_SEPARATOR,
  G_UNICODE_SPACE_SEPARATOR
} GUnicodeType;


#include "_gunichartables.h"

#define ATTR_TABLE(Page) (((Page) <= G_UNICODE_LAST_PAGE_PART1) \
                          ? attr_table_part1[Page] \
                          : attr_table_part2[(Page) - 0xe00])

#define ATTTABLE(Page, Char) \
  ((ATTR_TABLE(Page) == G_UNICODE_MAX_TABLE_INDEX) ? 0 : (attr_data[ATTR_TABLE(Page)][Char]))


#define TTYPE_PART1(Page, Char) \
  ((type_table_part1[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (type_table_part1[Page] - G_UNICODE_MAX_TABLE_INDEX) \
   : (type_data[type_table_part1[Page]][Char]))

#define TTYPE_PART2(Page, Char) \
  ((type_table_part2[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (type_table_part2[Page] - G_UNICODE_MAX_TABLE_INDEX) \
   : (type_data[type_table_part2[Page]][Char]))

#define TYPE(Char) \
  (((Char) <= G_UNICODE_LAST_CHAR_PART1) \
   ? TTYPE_PART1 ((Char) >> 8, (Char) & 0xff) \
   : (((Char) >= 0xe0000 && (Char) <= G_UNICODE_LAST_CHAR) \
      ? TTYPE_PART2 (((Char) - 0xe0000) >> 8, (Char) & 0xff) \
      : G_UNICODE_UNASSIGNED))

/* Count the number of elements in an array. The array must be defined
 * as such; using this with a dynamically allocated array will give
 * incorrect results.
 */
#define G_N_ELEMENTS(arr)		(sizeof (arr) / sizeof ((arr)[0]))




#if defined(LUCENE_USE_INTERNAL_CHAR_FUNCTIONS)
#ifdef _LUCENE_PRAGMA_WARNINGS
 #pragma message ("===== Note: using internal character function for compatibility =====")
#else
 #warning "===== Note: using internal character function for compatibility ====="
#endif

bool cl_isletter(gunichar c)
{
    int t = TYPE (c);
    switch(t)
    {
      case G_UNICODE_LOWERCASE_LETTER: return true;
      case G_UNICODE_TITLECASE_LETTER: return true;
      case G_UNICODE_UPPERCASE_LETTER: return true;
      case G_UNICODE_MODIFIER_LETTER: return true;
      case G_UNICODE_OTHER_LETTER: return true;
      default: return false;
    }
}

bool cl_isalnum(gunichar c)
{
    int t = TYPE (c);
    switch(t)
    {
      case G_UNICODE_LOWERCASE_LETTER: return true;
      case G_UNICODE_TITLECASE_LETTER: return true;
      case G_UNICODE_UPPERCASE_LETTER: return true;
      case G_UNICODE_MODIFIER_LETTER: return true;
      case G_UNICODE_OTHER_LETTER: return true;
      case G_UNICODE_DECIMAL_NUMBER: return true;
      case G_UNICODE_LETTER_NUMBER: return true;
      case G_UNICODE_OTHER_NUMBER: return true;
      default: return false;
    }
}

bool cl_isdigit(gunichar c)
{
    int t = TYPE (c);
    switch(t)
    {
      case G_UNICODE_DECIMAL_NUMBER: return true;
      case G_UNICODE_LETTER_NUMBER: return true;
      case G_UNICODE_OTHER_NUMBER: return true;
      default: return false;
    }
}

/**
 * cl_isspace:
 * @c: a Unicode character
 *
 * Determines whether a character is a space, tab, or line separator
 * (newline, carriage return, etc.).  Given some UTF-8 text, obtain a
 * character value with lucene_utf8towc().
 *
 * (Note: don't use this to do word breaking; you have to use
 * Pango or equivalent to get word breaking right, the algorithm
 * is fairly complex.)
 *
 * Return value: %TRUE if @c is a punctuation character
 **/
bool cl_isspace (gunichar c)
{
  switch (c)
  {
      /* special-case these since Unicode thinks they are not spaces */
    case '\t':
    case '\n':
    case '\r':
    case '\f':
      return true;

    default:
    {
     int t = TYPE ((gunichar)c);
     return (t == G_UNICODE_SPACE_SEPARATOR || t == G_UNICODE_LINE_SEPARATOR
             || t == G_UNICODE_PARAGRAPH_SEPARATOR);
    }
  }
}



/**
 * cl_tolower:
 * @c: a Unicode character.
 *
 * Converts a character to lower case.
 *
 * Return value: the result of converting @c to lower case.
 *               If @c is not an upperlower or titlecase character,
 *               or has no lowercase equivalent @c is returned unchanged.
 **/
TCHAR cl_tolower (TCHAR ch)
{
  gunichar c=ch;
  int t = TYPE ((gunichar)c);
  if (t == G_UNICODE_UPPERCASE_LETTER)
  {
      gunichar val = ATTTABLE (c >> 8, c & 0xff);
      if (val >= 0x1000000)
      {
        const gchar *p = special_case_table + val - 0x1000000;
		wchar_t ret=0;
		lucene_utf8towc(ret,p);
#ifdef _UCS2
		return ret;
#else
        return LUCENE_OOR_CHAR(ret);
#endif
      }else
        return val ? val : c;
  }else if (t == G_UNICODE_TITLECASE_LETTER){
      unsigned int i;
      for (i = 0; i < G_N_ELEMENTS (title_table); ++i)
      {
        if (title_table[i][0] == c)
          return title_table[i][2];
      }
  }
  return c;
}

/**
 * cl_toupper:
 * @c: a Unicode character
 * 
 * Converts a character to uppercase.
 * 
 * Return value: the result of converting @c to uppercase.
 *               If @c is not an lowercase or titlecase character,
 *               or has no upper case equivalent @c is returned unchanged.
 **/
TCHAR cl_toupper (TCHAR ch)
{
  gunichar c=ch;
  int t = TYPE (c);
  if (t == G_UNICODE_LOWERCASE_LETTER)
    {
      gunichar val = ATTTABLE (c >> 8, c & 0xff);
      if (val >= 0x1000000)
	{
	  const gchar *p = special_case_table + val - 0x1000000;
	  
	  wchar_t ret=0;
	  lucene_utf8towc(ret,p);
#ifdef _UCS2
	  return ret;
#else
      return LUCENE_OOR_CHAR(ret);
#endif
	  //return lucene_utf8towc (p);
	}
      else
	return val ? val : c;
    }
  else if (t == G_UNICODE_TITLECASE_LETTER)
    {
      unsigned int i;
      for (i = 0; i < G_N_ELEMENTS (title_table); ++i)
	{
	  if (title_table[i][0] == c)
	    return title_table[i][1];
	}
    }
  return c;
}



/**
 * cl_tcasefold:
 * @str: a unicode string
 *
 * Converts a string into a form that is independent of case. The
 * result will not correspond to any particular case, but can be
 * compared for equality or ordered with the results of calling
 * cl_tcasefold() on other strings.
 *
 * Note that calling cl_tcasefold() followed by g_utf8_collate() is
 * only an approximation to the correct linguistic case insensitive
 * ordering, though it is a fairly good one. Getting this exactly
 * right would require a more sophisticated collation function that
 * takes case sensitivity into account. GLib does not currently
 * provide such a function.
 *
 * Return value: a newly allocated string, that is a
 *   case independent form of @str.
 **/
TCHAR cl_tcasefold(const TCHAR ch){
    int start = 0;
    int end = G_N_ELEMENTS (casefold_table);
    
	if (ch >= casefold_table[start].ch &&
        ch <= casefold_table[end - 1].ch)
    {
        while (1)
        {
            int half = (start + end) / 2;
            if (ch == casefold_table[half].ch)
            {
				   wchar_t ret=0;
				   lucene_utf8towc(ret,casefold_table[half].data);

               #ifdef _UCS2
		           return ret;
               #else
                   return LUCENE_OOR_CHAR(ret);
               #endif
            }else if (half == start){
                break;
            }else if (ch > casefold_table[half].ch){
                start = half;
            }else{
                end = half;
            }
        }
    }
    return cl_tolower(ch);
    
}


//this function was not taken from gnome
TCHAR* cl_tcscasefold( TCHAR * str, int len ) //len default is -1
{
    TCHAR *p = str;
    while ((len < 0 || p < str + len) && *p)
    {
        *p = cl_tcasefold(*p);
		p++;
    }
    return str;
}
//this function was not taken from gnome
int cl_tcscasefoldcmp(const TCHAR * dst, const TCHAR * src){
    TCHAR f,l;
    
    do{
        f = cl_tcasefold( (*(dst++)) );
        l = cl_tcasefold( (*(src++)) );
    } while ( (f) && (f == l) );
    
    return (int)(f - l);
}

#endif
