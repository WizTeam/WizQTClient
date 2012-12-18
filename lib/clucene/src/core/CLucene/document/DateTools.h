/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_document_DateTools_
#define _lucene_document_DateTools_

#ifdef _CL_TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if defined(_CL_HAVE_SYS_TIME_H)
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef _CL_HAVE_SYS_TIMEB_H
# include <sys/timeb.h>
#endif

CL_NS_DEF(document)

class CLUCENE_EXPORT DateTools {
private:
  static void strCatDate(TCHAR* buf, int zeroes, int value);
public:

  enum Resolution {
          NO_RESOLUTION,
          YEAR_FORMAT,		// yyyy
          MONTH_FORMAT,		// yyyyMM
          DAY_FORMAT,			// yyyyMMdd
          HOUR_FORMAT,		// yyyyMMddHH
          MINUTE_FORMAT,		// yyyyMMddHHmm
          SECOND_FORMAT,		// yyyyMMddHHmmss
          MILLISECOND_FORMAT	// yyyyMMddHHmmssSSS
  };

  /**
  * Converts a millisecond time to a string suitable for indexing.
  *
  * @param time the date expressed as milliseconds since January 1, 1970, 00:00:00 GMT
  * @param resolution the desired resolution, see {@link #Resolution}
  * @return a string in format <code>yyyyMMddHHmmssSSS</code> or shorter,
  *  depeding on <code>resolution</code>; using UTC as timezone
  */
  static TCHAR* timeToString(const int64_t time, Resolution resolution = MILLISECOND_FORMAT);

  static void timeToString(const int64_t time, Resolution resolution, TCHAR* buf, size_t bufLength);

  /**
  * Converts a string produced by <code>timeToString</code> or
  * <code>dateToString</code> back to a time, represented as the
  * number of milliseconds since January 1, 1970, 00:00:00 GMT.
  *
  * @param dateString the date string to be converted
  * @return the number of milliseconds since January 1, 1970, 00:00:00 GMT
  * @throws ParseException if <code>dateString</code> is not in the
  *  expected format
  */
  static int64_t stringToTime(const TCHAR* dateString);

  static tm* stringToDate(const TCHAR* dateString);

  /****

  *   CLucene specific methods

  *****/

  /**
  * Returns a 64bit time value based on the parameters passed
  */
  static int64_t getTime(unsigned short year, uint8_t month, uint8_t mday, uint8_t hours = 0,
      uint8_t minutes = 0, uint8_t seconds = 0, unsigned short ms = 0);

  /**
  * Returns a 64bit time value which is inclusive of the whole last day.
  */
  static int64_t timeMakeInclusive(const int64_t time);

  inline static int64_t getDifferenceFromGMT();

  static TCHAR* getISOFormat(const int64_t time);
  static TCHAR* getISOFormat(unsigned short year, uint8_t month, uint8_t mday, uint8_t hours = 0,
      uint8_t minutes = 0, uint8_t seconds = 0, unsigned short ms = 0);

  virtual ~DateTools();
	
};
CL_NS_END
#endif
