/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_util_Misc_H
#define _lucene_util_Misc_H

#include <vector>

CL_NS_DEF(util)
  /** A class containing various functions.
  */
  class CLUCENE_SHARED_EXPORT Misc{
    static void zerr(int ret, std::string& err);
  public:
    static uint64_t currentTimeMillis();
    static const TCHAR* replace_all( const TCHAR* val, const TCHAR* srch, const TCHAR* repl );
    static bool dir_Exists(const char* path);
    static int64_t file_Size(const char* path);
    static int64_t filelength(int handle);
    static void sleep(const int ms);

    /**
     * Unlinks the given file, waits until dir_Exists is false. It makes maxAttempts
     * attempts to remove the file. If maxAttemps is less than 0 then unlimited
     * count of attempts is done. 
     * Returns 1 if deleted and dir_Exists returns false
     * Returns 0 if deleted and dir_Exists returns still true
     * Returns -1 if file can not be deleted.
     */
    static int32_t file_Unlink(const char* path, int32_t maxAttempts = -1);

    static size_t ahashCode(const char* str);
		static size_t ahashCode(const char* str, size_t len);

    static TCHAR* join ( const TCHAR* a, const TCHAR* b, const TCHAR* c=NULL, const TCHAR* d=NULL,const TCHAR* e=NULL,const TCHAR* f=NULL );
    static char* ajoin ( const char* a, const char* b, const char* c=NULL, const char* d=NULL,const char* e=NULL,const char* f=NULL );

    static bool priv_isDotDir( const TCHAR* name );
		//Creates a filename by concatenating Segment with ext and x
    static std::string segmentname(const char* segment, const char* ext, const int32_t x=-1 );
		//Creates a filename in buffer by concatenating Segment with ext and x
		static void segmentname(char* buffer,int32_t bufferLen, const char* Segment, const char* ext, const int32_t x=-1);

   /**
   * Compares two strings, character by character, and returns the
   * first position where the two strings differ from one another.
   *
   * @param s1 The first string to compare
   * @param s1Len The length of the first string to compare
   * @param s2 The second string to compare
   * @param s2Len The length of the second string to compare
   * @return The first position where the two strings differ.
   */
	static int32_t stringDifference(const TCHAR* s1, const int32_t s1Len, const TCHAR* s2, const int32_t s2Len);

	// In-place trimming for strings and words ("te st" will be returned by stringTrim, while wordTrim will return "te")
	// This is by design only meant for use with on-memory strings, and calling it like stringTrim(_T("test")) will
	// be errorneous
	static TCHAR* stringTrim(TCHAR* s);
	static TCHAR* wordTrim(TCHAR* s);

	static size_t longToBase( int64_t value, int32_t base, char* to ); //< length of to should be at least ((sizeof(unsigned long) << 3) + 1). returns actual length used
	static int64_t base36ToLong( const char* value );

  static std::string toString(const int32_t value);
  static std::string toString(const int64_t value);
  static std::string toString(const _LUCENE_THREADID_TYPE value);
  static std::string toString(const bool value);
  static std::string toString(const float_t value);
  static std::string toString(const TCHAR* s, int32_t len=-1);

	#ifdef _UCS2
    static size_t whashCode(const wchar_t* str);
		static size_t whashCode(const wchar_t* str, size_t len);
		#define thashCode whashCode

    static char* _wideToChar(const wchar_t* s);
    static wchar_t* _charToWide(const char* s);

    static void _cpycharToWide(const char* s, wchar_t* d, size_t len);
    static void _cpywideToChar(const wchar_t* s, char* d, size_t len);
	#else
		#define thashCode ahashCode
	#endif

		/** List all files in dir.
		* @param bool fullPath True to return entire path
		*/
		static bool listFiles(const char* dir, std::vector<std::string>& files, bool fullPath=false);

    /** uncompress the source stream into the dest stream.
    * Default CHUNK size is 1k
    */
    static bool inflate(const uint8_t* source, size_t sourcelen, std::ostream& dest, std::string& err, int CHUNK=-1);
    /** compress the source stream into the dest stream.
    * Default CHUNK size is 1k
    * Default level is Z_BEST_COMPRESSION
    */
    static bool deflate(const uint8_t* source, size_t sourcelen, std::ostream& dest, std::string& err, int CHUNK=-1, int level=-1);
  };

CL_NS_END
#endif
