/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "Misc.h"
#include <assert.h>
#include <iostream>
#include <map>

#if defined(_CL_HAVE_SYS_TIME_H)
# include <sys/time.h>
#elif defined(_CL_HAVE_TIME_H)
# include <time.h>
#endif
#ifdef _CL_HAVE_SYS_TIMEB_H
	#include <sys/timeb.h>
#endif

#if defined(_CL_HAVE_SYS_STAT_H)
    #include <sys/stat.h>
#endif
#ifdef _CL_HAVE_STRINGS_H
	#include <strings.h>
#endif
#ifdef _CL_HAVE_UNISTD_H
	#include <unistd.h>
#endif

#include <cctype>
#include <limits.h>
#include "CLucene/util/dirent.h" //if we have dirent, then the native one will be used

//for zlib...
#include "zlib.h"
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif
#define CL_Z_DEFAULT_CHUNK 1024


#ifdef _CL_HAVE_FUNCTION_SLEEP
	//don't ignore windows.h... breaks mingw32 in some cases. Define Sleep instead
	extern "C" __declspec(dllimport) void __stdcall Sleep(_cl_dword_t);
#endif

CL_NS_DEF(util)

size_t Misc::ahashCode(const char* str){
	// Compute the hash code using a local variable to be reentrant.
	size_t hashCode = 0;
	while ( *str != 0 )
		hashCode = hashCode * 31 + *str++;
	return hashCode;
}
size_t Misc::ahashCode(const char* str, size_t len){
	// Compute the hash code using a local variable to be reentrant.
	size_t hashCode = 0;
	for (size_t i = 0; i<len; i++)
		hashCode = hashCode * 31 + *str++;
	return hashCode;
}

int64_t Misc::filelength(int filehandle)
{
    struct cl_stat_t info;
    if (fileHandleStat(filehandle, &info) == -1)
 	 		return -1;
    return info.st_size;
}

//this is global...
void Misc::sleep(const int ms){
    #if defined(_CL_HAVE_FUNCTION_USLEEP)
        usleep(ms*1000);//expects microseconds
    #elif defined(SLEEPFUNCTION)
	    SLEEPFUNCTION(ms);
	#else
	    #error no sleep function???
	#endif
}


#ifdef _UCS2
size_t Misc::whashCode(const wchar_t* str){
	// Compute the hash code using a local variable to be reentrant.
	size_t hashCode = 0;
	while ( *str != 0 )
		hashCode = hashCode * 31 + *str++;
	return hashCode;
}
size_t Misc::whashCode(const wchar_t* str, size_t len){
	// Compute the hash code using a local variable to be reentrant.
	size_t hashCode = 0;
	for (size_t i = 0; i<len; i++)
		hashCode = hashCode * 31 + *str++;
	return hashCode;
}

std::string Misc::toString(const TCHAR* s, int32_t len){
  if ( s == NULL || len == 0 ) return "";
  if ( len < 0 ) len = _tcslen(s);
  char* buf = _CL_NEWARRAY(char,len+1);
  STRCPY_WtoA(buf,s,len+1);
  string ret = buf;
  _CLDELETE_CaARRAY(buf);
  return ret;
}
//static
char* Misc::_wideToChar(const wchar_t* s){
   size_t len = _tcslen(s);
   char* msg=_CL_NEWARRAY(char,len+1);
   _cpywideToChar( s,msg,len+1 );
   return msg;
}
wchar_t* Misc::_charToWide(const char* s){
   size_t len = strlen(s);
   wchar_t* msg = _CL_NEWARRAY(wchar_t,len+1);
   _cpycharToWide(s,msg,len+1);
   return msg;
}

void Misc::_cpywideToChar(const wchar_t* s, char* d, size_t len){
    size_t sLen = wcslen(s);
    for ( uint32_t i=0;i<len&&i<sLen+1;i++ )
        d[i] = LUCENE_OOR_CHAR(s[i]);
}
void Misc::_cpycharToWide(const char* s, wchar_t* d, size_t len){
    size_t sLen = strlen(s);
    for ( uint32_t i=0;i<len&&i<sLen+1;i++ )
      d[i] = s[i];
}
#endif

//static
uint64_t Misc::currentTimeMillis() {
#ifndef _CL_HAVE_FUNCTION_GETTIMEOFDAY
    struct _timeb tstruct;
    _ftime(&tstruct);

    return (((uint64_t) tstruct.time) * 1000) + tstruct.millitm;
#else
    struct timeval tstruct;
    if (gettimeofday(&tstruct, NULL) < 0) {
			return 0;
    }

    return (((uint64_t) tstruct.tv_sec) * 1000) + tstruct.tv_usec / 1000;
#endif
}

//static
const TCHAR* Misc::replace_all( const TCHAR* val, const TCHAR* srch, const TCHAR* repl )
{
int32_t cnt = 0;
size_t repLen = _tcslen(repl);
size_t srchLen = _tcslen(srch);
size_t srcLen = _tcslen(val);

const TCHAR* pos = val;
while( (pos = _tcsstr(pos+1, srch)) != NULL ) {
    ++cnt;
}

size_t lenNew = (srcLen - (srchLen * cnt)) + (repLen * cnt);
TCHAR* ret = _CL_NEWARRAY(TCHAR,lenNew+1);
ret[lenNew] = 0;
if ( cnt == 0 ){
    _tcscpy(ret,val);
    return ret;
}

TCHAR* cur = ret; //position of return buffer
const TCHAR* lst = val; //position of value buffer
pos = val; //searched position of value buffer
while( (pos = _tcsstr(pos+1,srch)) != NULL ) {
    _tcsncpy(cur,lst,pos-lst); //copy till current
    cur += (pos-lst);
    lst = pos; //move val position

    _tcscpy( cur,repl); //copy replace
    cur += repLen; //move return buffer position
    lst += srchLen; //move last value buffer position
}
_tcscpy(cur, lst ); //copy rest of buffer

return ret;
}

//static
bool Misc::dir_Exists(const char* path){
	if ( !path || !*path )
		return false;
	struct cl_stat_t buf;
	int32_t ret = fileStat(path,&buf);
	return ( ret == 0);
}

int64_t Misc::file_Size(const char* path){
	struct cl_stat_t buf;
	if ( fileStat(path,&buf) == 0 )
		return buf.st_size;
	else
		return -1;
}

int Misc::file_Unlink(const char* path, int32_t maxAttempts )
{
    int32_t i;

    if( ! path || ! * path )
        return -1;

    if( maxAttempts == 0 )
        maxAttempts = 1;

    while( maxAttempts != 0 )
    {
        if( _unlink( path ) != 0 )
            return -1;

        i = 0;
        while( i < 100 )
        {
            if( ! Misc::dir_Exists( path ) )
                return 1;

            if( ++i > 50 )      // if it still doesn't show up, then we do some sleeping for the last 50ms
                _LUCENE_SLEEP( 1 );
        }

        if( maxAttempts > 0 )
            maxAttempts--;
    }

    return 0;
}


//static
TCHAR* Misc::join ( const TCHAR* a, const TCHAR* b, const TCHAR* c, const TCHAR* d,const TCHAR* e,const TCHAR* f ) {
#define LEN(x) (x == NULL ? 0 : _tcslen(x))
const size_t totalLen =
    LEN(a) + LEN(b) + LEN(c) + LEN(d) + LEN(e) + LEN(f)
    + sizeof(TCHAR); /* Space for terminator. */

TCHAR* buf = _CL_NEWARRAY(TCHAR,totalLen);
buf[0]=0;
if ( a != NULL) _tcscat(buf,a);
if ( b != NULL) _tcscat(buf,b);
if ( c != NULL) _tcscat(buf,c);
if ( d != NULL) _tcscat(buf,d);
if ( e != NULL) _tcscat(buf,e);
if ( f != NULL) _tcscat(buf,f);
return buf;
}

char* Misc::ajoin ( const char* a, const char* b, const char* c, const char* d,const char* e,const char* f ) {
  #define aLEN(x) (x == NULL ? 0 : strlen(x))
  const size_t totalLen =
    aLEN(a) + aLEN(b) + aLEN(c) + aLEN(d) + aLEN(e) + aLEN(f)
    + sizeof(char); /* Space for terminator. */

  char* buf = _CL_NEWARRAY(char,totalLen);
  buf[0]=0;
  if ( a != NULL) strcat(buf,a);
  if ( b != NULL) strcat(buf,b);
  if ( c != NULL) strcat(buf,c);
  if ( d != NULL) strcat(buf,d);
  if ( e != NULL) strcat(buf,e);
  if ( f != NULL) strcat(buf,f);
return buf;
}

//static
bool Misc::priv_isDotDir( const TCHAR* name )
{
if( name[0] == '\0' ) {
    return (false);
}
if( name[0] == '.' && name[1] == '\0' ) {
    return (true);
}
if( name[1] == '\0' ) {
    return (false);
}
if( name[0] == '.' && name[1] == '.' && name[2] == '\0' ) {
    return (true);
}

return (false);
}

//internal static function shared for clucene
string Misc::segmentname( const char* segment, const char* ext, const int32_t x ){
//Func -  Returns an allocated buffer in which it creates a filename by
//       concatenating segment with ext and x
//Pre    ext != NULL and holds the extension
//       x contains a number
//Post - A buffer has been instantiated an when x = -1 buffer contains the concatenation of
//       segment and ext otherwise buffer contains the contentation of segment, ext and x

	CND_PRECONDITION(ext != NULL, "ext is NULL");


  if ( x!=-1 ){
    char buf[30];
    _snprintf(buf,10,"%d",x);
    return string(segment) + ext + buf;
  }else{
    return string(segment) + ext;
  }
}
void Misc::segmentname(char* buffer,int32_t bufferLen, const char* Segment, const char* ext, const int32_t x){
//Func - Static Method
//       Creates a filename in buffer by concatenating Segment with ext and x
//Pre  - buffer  != NULL
//       Segment != NULL and holds the name of the segment
//       ext     != NULL and holds the extension
//       x contains a number
//Post - When x = -1 buffer contains the concatenation of Segment and ext otherwise
//       buffer contains the contentation of Segment, ext and x

	CND_PRECONDITION(buffer  != NULL, "buffer is NULL");
	CND_PRECONDITION(Segment != NULL, "Segment is NULL");
	CND_PRECONDITION(ext     != NULL, "ext is NULL");

	if ( x== -1 )
		_snprintf(buffer,bufferLen,"%s%s", Segment,ext );
	else
		_snprintf(buffer,bufferLen,"%s%s%d", Segment,ext,x );
}


//static
int32_t Misc::stringDifference(const TCHAR* s1, const int32_t len1, const TCHAR* s2, const int32_t len2) {
	int32_t len = len1 < len2 ? len1 : len2;
	for (int32_t i = 0; i < len; i++)
		if ( s1[i] != s2[i])
			return i;
	return len;
}

TCHAR* Misc::stringTrim(TCHAR* text) {
	size_t j, i;
	size_t len = _tcslen(text);

	for ( i=0;i<len;i++ ){ // find the first non-space character and store it as i
		if ( ! _istspace(text[i]) )
			break;
	}
	for ( j=len-1; j > i; --j ){ // find the last non-space character and store it as j
		if ( ! _istspace(text[j]) ) {
			break;
		}
	}

	if (i==0 && j==len-1) // prevent unnecessary copy
		return text;

	if (i==0)
		text[j+1]=0;
	else {
		j++;
		_tcsncpy(text, text+i, j-i);
		text[j-i] = 0;
	}

	return text;
}

TCHAR* Misc::wordTrim(TCHAR* text) {
	size_t j, i;
	size_t len = _tcslen(text);

	for ( i=0;i<len;i++ ){ // find the first non-space character and store it as i
		if ( ! _istspace(text[i]) )
			break;
	}
	for ( j=i; j < len; j++ ){ // find the last non-space character and store it as j
		if ( _istspace(text[j]) ) {
			break;
		}
	}

	if (i == 0 && j==len)
		return text;

	if (i==j) // empty string
		return NULL;

	if (i == 0) {
		text[j] = 0;
		return text;
	} else {
		_tcsncpy(text, text+i, j-i);
		text[j-i] = 0;
	}

	return text;
}

size_t Misc::longToBase( int64_t value, int32_t base, char* retval ) {
    static char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char buf[(sizeof(unsigned long) << 3) + 1];
    char *ptr, *end;

    ptr = end = buf + sizeof(buf) - 1;

    *ptr = '\0';
    do {
            *--ptr = digits[ value % base ];
            value /= base;
    } while ( ptr > buf && value );

    memcpy( retval, ptr, end - ptr );
    retval[end-ptr] = 0;

    return end-ptr;
}

int64_t Misc::base36ToLong( const char* value ) {
	char* ptr = (char*)value;
	int64_t lval = 0;

	while ( *ptr != '\0' ) {
		lval = isdigit(*ptr) ? ( 36 * lval ) + ( *ptr - '0' ) : ( 36 * lval ) + ( *ptr - 'a' + 10 );
		ptr++;
	}

	return lval;
}

bool Misc::listFiles(const char* directory, std::vector<std::string>& files, bool fullPath){
  //clear old files
  DIR* dir = opendir(directory);
  if ( dir == NULL ) return false;
  struct dirent* fl = readdir(dir);
  struct cl_stat_t buf;
	string path;
  while ( fl != NULL ){
  	path = string(directory) + "/" + fl->d_name;
		int32_t ret = fileStat(path.c_str(),&buf);
		if ( ret==0 && !(buf.st_mode & S_IFDIR) ) {
			if ( (strcmp(fl->d_name, ".")) && (strcmp(fl->d_name, "..")) ) {
				if ( fullPath ){
					files.push_back(path);
				}else{
					files.push_back(fl->d_name);
				}
			}
		}
	  fl = readdir(dir);
  }
  closedir(dir);
  return true;
}


std::string Misc::toString(const bool value){
  return value ? "true" : "false";
}
std::string Misc::toString(_LUCENE_THREADID_TYPE value){
  static int32_t nextindex = 0;
  static std::map<_LUCENE_THREADID_TYPE, int32_t> ids;
  if (ids.find(value) == ids.end()) {
    ids[value] = nextindex++;
  }
  return toString(ids[value]);
}
std::string Misc::toString(const int32_t value){
  char buf[20];
  TCHAR tbuf[20];
  _i64tot(value, tbuf, 10);
  STRCPY_TtoA(buf,tbuf,20);
  return buf;
}
std::string Misc::toString(const int64_t value){
  char buf[20];
  TCHAR tbuf[20];
  _i64tot(value, tbuf, 10);
  STRCPY_TtoA(buf,tbuf,20);
  return buf;
}
std::string Misc::toString(const float_t value){
  char buf[20];
  _snprintf(buf,20,"%0.2f",(double)value);
  return buf;
}

void Misc::zerr(int ret, string& err)
{
  switch (ret) {
  case Z_ERRNO:
    err = "error occurred while reading or writing from the zlib streams";
    break;
  case Z_STREAM_ERROR:
    err = "invalid compression level";
    break;
  case Z_DATA_ERROR:
    err = "invalid or incomplete deflate data";
    break;
  case Z_MEM_ERROR:
    err = "out of memory";
    break;
  case Z_VERSION_ERROR:
    err ="zlib version mismatch";
  }
}



/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
bool Misc::deflate(const uint8_t* in, size_t inlen, std::ostream& dest, string& err, int CHUNK, int level)
{
  int ret, flush;
  unsigned have;
  z_stream strm;
  if ( level == -1 ) level = Z_BEST_COMPRESSION;
  if ( CHUNK == -1 ) CHUNK = CL_Z_DEFAULT_CHUNK;
  uint8_t* out = (uint8_t*)malloc(CHUNK);

  /* allocate deflate state */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  ret = deflateInit(&strm, level);
  if (ret != Z_OK){
    free(out);
    zerr(ret, err);
    return false;
  }

  /* compress until end of file */
  do {
    strm.avail_in = inlen;
    strm.next_in = (uint8_t*)in;
    flush = Z_FINISH;

    /* run deflate() on input until output buffer not full, finish
       compression if all of source has been read in */
    do {
      strm.avail_out = CHUNK;
      strm.next_out = out;
      ret = ::deflate(&strm, flush);  /* no bad return value */
      assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
      have = CHUNK - strm.avail_out;
      dest.write( (char*)out,have);
      if ( dest.fail() ) {
        (void)deflateEnd(&strm);
        free(out);
        zerr(Z_ERRNO, err);
        return false;
      }
    } while (strm.avail_out == 0);
    assert(strm.avail_in == 0);   /* all input will be used */

    /* done when last data in file processed */
  } while (flush != Z_FINISH);
  assert(ret == Z_STREAM_END);    /* stream will be complete */

  /* clean up and return */
  (void)deflateEnd(&strm);
  free(out);
  return true;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
bool Misc::inflate(const uint8_t* in, size_t inlen, std::ostream& dest, string& err, int CHUNK)
{
  int ret;
  unsigned have;
  z_stream strm;
  if ( CHUNK == -1 ) CHUNK = CL_Z_DEFAULT_CHUNK;
  uint8_t* out = (uint8_t*)malloc(CHUNK);

  /* allocate inflate state */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  ret = inflateInit(&strm);
  if (ret != Z_OK){
    free(out);
    zerr(ret, err);
    return false;
  }

  /* decompress until deflate stream ends or end of file */
  do {
    strm.avail_in = inlen;
    if (strm.avail_in == 0)
      break;
    strm.next_in = (uint8_t*)in;

    /* run inflate() on input until output buffer not full */
    do {
      strm.avail_out = CHUNK;
      strm.next_out = out;
      ret = ::inflate(&strm, Z_NO_FLUSH);
      assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
      switch (ret) {
      case Z_NEED_DICT:
        ret = Z_DATA_ERROR;   /* and fall through */
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
        (void)inflateEnd(&strm);
        free(out);
        zerr(ret, err);
        return false;
      }
      have = CHUNK - strm.avail_out;
      dest.write( (char*)out,have);
      if ( dest.fail() ) {
        (void)inflateEnd(&strm);
        free(out);
        zerr(Z_ERRNO, err);
        return false;
      }
    } while (strm.avail_out == 0);

    /* done when inflate() says it's done */
  } while (ret != Z_STREAM_END);

  /* clean up and return */
  (void)inflateEnd(&strm);
  free(out);
  if ( ret == Z_STREAM_END )
    return true;
  zerr(Z_DATA_ERROR, err);
  return false;
}

CL_NS_END
