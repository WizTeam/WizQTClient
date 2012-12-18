/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"

#include "FSDirectory.h"
#include "_MMapIndexInput.h"
#include "CLucene/util/Misc.h"

#include <fcntl.h>
#ifdef _CL_HAVE_IO_H
	#include <io.h>
#endif
#ifdef _CL_HAVE_SYS_STAT_H
	#include <sys/stat.h>
#endif
#ifdef _CL_HAVE_UNISTD_H
	#include <unistd.h>
#endif
#ifdef _CL_HAVE_DIRECT_H
	#include <direct.h>
#endif
#ifdef _CL_HAVE_SYS_MMAN_H
	#include <sys/mman.h>
#endif
#ifdef _CL_HAVE_WINERROR_H
	#include <winerror.h>
#endif
#include <errno.h>

#if defined(_CL_HAVE_FUNCTION_MAPVIEWOFFILE)
    typedef int HANDLE;
	
    #define GENERIC_READ                     (0x80000000L)
    #define FILE_SHARE_READ                 0x00000001  
    #define OPEN_EXISTING       3
    #define PAGE_READONLY          0x02     
    #define SECTION_MAP_READ    0x0004
    #define FILE_MAP_READ       SECTION_MAP_READ

	typedef struct  _SECURITY_ATTRIBUTES
    {
        _cl_dword_t nLength;
        void* lpSecurityDescriptor;
        bool bInheritHandle;
    }	SECURITY_ATTRIBUTES;
	
	extern "C" __declspec(dllimport) _cl_dword_t __stdcall GetFileSize( HANDLE hFile, _cl_dword_t* lpFileSizeHigh );
    
	extern "C" __declspec(dllimport) bool __stdcall UnmapViewOfFile( void* lpBaseAddress );

    extern "C" __declspec(dllimport) bool __stdcall CloseHandle( HANDLE hObject );
    extern "C" __declspec(dllimport) HANDLE __stdcall CreateFileA(
		const char* lpFileName,
		_cl_dword_t dwDesiredAccess,
		_cl_dword_t dwShareMode,
		SECURITY_ATTRIBUTES* lpSecurityAttributes,
		_cl_dword_t dwCreationDisposition,
		_cl_dword_t dwFlagsAndAttributes,
		HANDLE hTemplateFile
    );
    extern "C" __declspec(dllimport) HANDLE __stdcall CreateFileMappingA(
        HANDLE hFile,
        SECURITY_ATTRIBUTES* lpFileMappingAttributes,
        _cl_dword_t flProtect,
        _cl_dword_t dwMaximumSizeHigh,
        _cl_dword_t dwMaximumSizeLow,
        const char* lpName
    );
    extern "C" __declspec(dllimport) void* __stdcall MapViewOfFile(
        HANDLE hFileMappingObject,
        _cl_dword_t dwDesiredAccess,
        _cl_dword_t dwFileOffsetHigh,
        _cl_dword_t dwFileOffsetLow,
        _cl_dword_t dwNumberOfBytesToMap
    );
    extern "C" __declspec(dllimport) _cl_dword_t __stdcall GetLastError();
#endif


CL_NS_DEF(store)
CL_NS_USE(util)

    class MMapIndexInput::Internal: LUCENE_BASE{
	public:
		uint8_t* data;
		int64_t pos;
#if defined(_CL_HAVE_FUNCTION_MAPVIEWOFFILE)
		HANDLE mmaphandle;
		HANDLE fhandle;
#elif defined(_CL_HAVE_FUNCTION_MMAP)
		int fhandle;
#else
        #error no mmap implementation set
#endif
		bool isClone;
		int64_t _length;
		
		Internal():
    		data(NULL),
    		pos(0),
    		isClone(false),
    		_length(0)
    	{
    	}
        ~Internal(){
        }
    };

	MMapIndexInput::MMapIndexInput(Internal* __internal):
	    _internal(__internal)
	{
  }
  
  bool MMapIndexInput::open(const char* path, IndexInput*& ret, CLuceneError& error, int32_t __bufferSize )    {

	//Func - Constructor.
	//       Opens the file named path
	//Pre  - path != NULL
	//Post - if the file could not be opened  an exception is thrown.

	  CND_PRECONDITION(path != NULL, "path is NULL");

    Internal* _internal = _CLNEW Internal;

#if defined(_CL_HAVE_FUNCTION_MAPVIEWOFFILE)
	  _internal->mmaphandle = NULL;
	  _internal->fhandle = CreateFileA(path,GENERIC_READ,FILE_SHARE_READ, 0,OPEN_EXISTING,0,0);
	  
	  //Check if a valid fhandle was retrieved
	  if (_internal->fhandle < 0){
		_cl_dword_t err = GetLastError();
        if ( err == ERROR_FILE_NOT_FOUND )
        error.set(CL_ERR_IO, "File does not exist");
        else if ( err == ERROR_ACCESS_DENIED )
        error.set(CL_ERR_IO, "File Access denied");
        else if ( err == ERROR_TOO_MANY_OPEN_FILES )
        error.set(CL_ERR_IO, "Too many open files");
		else
          error.set(CL_ERR_IO, "Could not open file");
	  }

	  _cl_dword_t dummy=0;
	  _internal->_length = GetFileSize(_internal->fhandle, &dummy);

	  if ( _internal->_length > 0 ){
			_internal->mmaphandle = CreateFileMappingA(_internal->fhandle,NULL,PAGE_READONLY,0,0,NULL);
			if ( _internal->mmaphandle != NULL ){
				void* address = MapViewOfFile(_internal->mmaphandle,FILE_MAP_READ,0,0,0);
				if ( address != NULL ){
					_internal->data = (uint8_t*)address;
          ret = _CLNEW MMapIndexInput(_internal);
          return true;
				}
			}
			
			//failure:
			int errnum = GetLastError(); 
			
			CloseHandle(_internal->mmaphandle);
	
			char* lpMsgBuf=strerror(errnum);
			size_t len = strlen(lpMsgBuf)+80;
			char* errstr = _CL_NEWARRAY(char, len); 
			cl_sprintf(errstr, len, "MMapIndexInput::MMapIndexInput failed with error %d: %s", errnum, lpMsgBuf); 
	
	    error.set(CL_ERR_IO, errstr);
			_CLDELETE_CaARRAY(errstr);
	  }

#else //_CL_HAVE_FUNCTION_MAPVIEWOFFILE
     _internal->fhandle = ::_cl_open (path, _O_BINARY | O_RDONLY | _O_RANDOM, _S_IREAD);
  	 if (_internal->fhandle < 0){
	    error.set(CL_ERR_IO, strerror(errno));
  	 }else{
		// stat it
		struct stat sb;
		if (::fstat (_internal->fhandle, &sb)){
	    error.set(CL_ERR_IO, strerror(errno));
		}else{
			// get length from stat
			_internal->_length = sb.st_size;
			
			// mmap the file
			void* address = ::mmap(0, _internal->_length, PROT_READ, MAP_SHARED, _internal->fhandle, 0);
			if (address == MAP_FAILED){
				error.set(CL_ERR_IO, strerror(errno));
			}else{
				_internal->data = (uint8_t*)address;
        ret = _CLNEW MMapIndexInput(_internal);
        return true;
			}
		}
  	 }
#endif

    _CLDELETE(_internal);
    return false;
  }

  MMapIndexInput::MMapIndexInput(const MMapIndexInput& clone): IndexInput(clone){
  //Func - Constructor
  //       Uses clone for its initialization
  //Pre  - clone is a valide instance of FSIndexInput
  //Post - The instance has been created and initialized by clone
        _internal = _CLNEW Internal;
        
#if defined(_CL_HAVE_FUNCTION_MAPVIEWOFFILE)
	  _internal->mmaphandle = NULL;
	  _internal->fhandle = NULL;
#endif

	  _internal->data = clone._internal->data;
	  _internal->pos = clone._internal->pos;

	  //clone the file length
	  _internal->_length  = clone._internal->_length;
	  //Keep in mind that this instance is a clone
	  _internal->isClone = true;
  }

  uint8_t MMapIndexInput::readByte(){
	  return *(_internal->data+(_internal->pos++));
  }

  void MMapIndexInput::readBytes(uint8_t* b, const int32_t len){
	memcpy(b, _internal->data+_internal->pos, len);
	_internal->pos+=len;
  }
  int32_t MMapIndexInput::readVInt(){
	  uint8_t b = *(_internal->data+(_internal->pos++));
	  int32_t i = b & 0x7F;
	  for (int shift = 7; (b & 0x80) != 0; shift += 7) {
	    b = *(_internal->data+(_internal->pos++));
	    i |= (b & 0x7F) << shift;
	  }
	  return i;
  }
  int64_t MMapIndexInput::getFilePointer() const{
	return _internal->pos;
  }
  void MMapIndexInput::seek(const int64_t pos){
	  this->_internal->pos=pos;
  }
  int64_t MMapIndexInput::length() const{ return _internal->_length; }

  MMapIndexInput::~MMapIndexInput(){
  //Func - Destructor
  //Pre  - True
  //Post - The file for which this instance is responsible has been closed.
  //       The instance has been destroyed

	  close();
	  _CLDELETE(_internal);
  }

  IndexInput* MMapIndexInput::clone() const
  {
    return _CLNEW MMapIndexInput(*this);
  }
  void MMapIndexInput::close()  {
	if ( !_internal->isClone ){
#if defined(_CL_HAVE_FUNCTION_MAPVIEWOFFILE)
		if ( _internal->data != NULL ){
			if ( ! UnmapViewOfFile(_internal->data) ){
				CND_PRECONDITION( false, "UnmapViewOfFile(data) failed"); //todo: change to rich error
			}
		}

		if ( _internal->mmaphandle != NULL ){
			if ( ! CloseHandle(_internal->mmaphandle) ){
				CND_PRECONDITION( false, "CloseHandle(mmaphandle) failed");
			}
		}
		if ( _internal->fhandle != NULL ){
			if ( !CloseHandle(_internal->fhandle) ){
				CND_PRECONDITION( false, "CloseHandle(fhandle) failed");
			}
		}
		_internal->mmaphandle = NULL;
		_internal->fhandle = NULL;
#else
		if ( _internal->data != NULL )
	  		::munmap(_internal->data, _internal->_length);
	  	if ( _internal->fhandle > 0 )
	  		::close(_internal->fhandle);
	  	_internal->fhandle = 0;
#endif
	}
	_internal->data = NULL;
	_internal->pos = 0;
  }


CL_NS_END
