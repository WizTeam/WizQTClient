/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"

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
#include <errno.h>

#include <assert.h>

#include "FSDirectory.h"
#include "LockFactory.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/index/IndexWriter.h"
#include "CLucene/util/Misc.h"
#include "CLucene/util/_MD5Digester.h"

#ifdef LUCENE_FS_MMAP
    #include "_MMapIndexInput.h"
#endif

CL_NS_DEF(store)
CL_NS_USE(util)

   /** This cache of directories ensures that there is a unique Directory
   * instance per path, so that synchronization on the Directory can be used to
   * synchronize access between readers and writers.
   */
	static CL_NS(util)::CLHashMap<const char*,FSDirectory*,CL_NS(util)::Compare::Char,CL_NS(util)::Equals::Char> DIRECTORIES(false,false);
	STATIC_DEFINE_MUTEX(DIRECTORIES_LOCK)

	bool FSDirectory::disableLocks=false;

	class FSDirectory::FSIndexInput:public BufferedIndexInput {
		/**
		* We used a shared handle between all the fsindexinput clones.
		* This reduces number of file handles we need, and it means
		* we dont have to use file tell (which is slow) before doing
		* a read.
    * TODO: get rid of this and dup/fctnl or something like that...
		*/
		class SharedHandle: LUCENE_REFBASE{
		public:
			int32_t fhandle;
			int64_t _length;
			int64_t _fpos;
			DEFINE_MUTEX(*SHARED_LOCK)
			char path[CL_MAX_DIR]; //todo: this is only used for cloning, better to get information from the fhandle
			SharedHandle(const char* path);
			~SharedHandle();
		};
		SharedHandle* handle;
		int64_t _pos;
		FSIndexInput(SharedHandle* handle, int32_t __bufferSize):
			BufferedIndexInput(__bufferSize)
		{
			this->_pos = 0;
			this->handle = handle;
		};
	protected:
		FSIndexInput(const FSIndexInput& clone);
	public:
		static bool open(const char* path, IndexInput*& ret, CLuceneError& error, int32_t bufferSize=-1);
		~FSIndexInput();

		IndexInput* clone() const;
		void close();
		int64_t length() const { return handle->_length; }

		const char* getDirectoryType() const{ return FSDirectory::getClassName(); }
    const char* getObjectName() const{ return getClassName(); }
    static const char* getClassName() { return "FSIndexInput"; }
	protected:
		// Random-access methods
		void seekInternal(const int64_t position);
		// IndexInput methods
		void readInternal(uint8_t* b, const int32_t len);
	};

	class FSDirectory::FSIndexOutput: public BufferedIndexOutput {
	private:
		int32_t fhandle;
	protected:
		// output methods:
		void flushBuffer(const uint8_t* b, const int32_t size);
	public:
		FSIndexOutput(const char* path, int filemode);
		~FSIndexOutput();

		// output methods:
		void close();

		// Random-access methods
		void seek(const int64_t pos);
		int64_t length() const;
	};

	bool FSDirectory::FSIndexInput::open(const char* path, IndexInput*& ret, CLuceneError& error, int32_t __bufferSize )    {
	//Func - Constructor.
	//       Opens the file named path
	//Pre  - path != NULL
	//Post - if the file could not be opened  an exception is thrown.

	  CND_PRECONDITION(path != NULL, "path is NULL");

	  if ( __bufferSize == -1 )
		  __bufferSize = CL_NS(store)::BufferedIndexOutput::BUFFER_SIZE;
	  SharedHandle* handle = _CLNEW SharedHandle(path);

	  //Open the file
	  handle->fhandle  = ::_cl_open(path, _O_BINARY | O_RDONLY | _O_RANDOM, _S_IREAD );

	  //Check if a valid handle was retrieved
	  if (handle->fhandle >= 0){
		  //Store the file length
		  handle->_length = fileSize(handle->fhandle);
		  if ( handle->_length == -1 )
	  		error.set( CL_ERR_IO,"fileStat error" );
		  else{
			  handle->_fpos = 0;
			  ret = _CLNEW FSIndexInput(handle, __bufferSize);
			  return true;
		  }
	  }else{
		  int err = errno;
      if ( err == ENOENT )
	      error.set(CL_ERR_IO, "File does not exist");
      else if ( err == EACCES )
        error.set(CL_ERR_IO, "File Access denied");
      else if ( err == EMFILE )
        error.set(CL_ERR_IO, "Too many open files");
      else
      	error.set(CL_ERR_IO, "Could not open file");
	  }
#ifndef _CL_DISABLE_MULTITHREADING
    delete handle->SHARED_LOCK;
#endif
	  _CLDECDELETE(handle);
	  return false;
  }

  FSDirectory::FSIndexInput::FSIndexInput(const FSIndexInput& other): BufferedIndexInput(other){
  //Func - Constructor
  //       Uses clone for its initialization
  //Pre  - clone is a valide instance of FSIndexInput
  //Post - The instance has been created and initialized by clone
	  if ( other.handle == NULL )
		  _CLTHROWA(CL_ERR_NullPointer, "other handle is null");

	  SCOPED_LOCK_MUTEX(*other.handle->SHARED_LOCK)
	  handle = _CL_POINTER(other.handle);
	  _pos = other.handle->_fpos; //note where we are currently...
  }

  FSDirectory::FSIndexInput::SharedHandle::SharedHandle(const char* path){
  	fhandle = 0;
    _length = 0;
    _fpos = 0;
    strcpy(this->path,path);

#ifndef _CL_DISABLE_MULTITHREADING
	  SHARED_LOCK = new _LUCENE_THREADMUTEX;
#endif
  }
  FSDirectory::FSIndexInput::SharedHandle::~SharedHandle() {
    if ( fhandle >= 0 ){
      if ( ::_close(fhandle) != 0 )
        _CLTHROWA(CL_ERR_IO, "File IO Close error");
      else
        fhandle = -1;
    }
  }

  FSDirectory::FSIndexInput::~FSIndexInput(){
  //Func - Destructor
  //Pre  - True
  //Post - The file for which this instance is responsible has been closed.
  //       The instance has been destroyed

	  FSIndexInput::close();
  }

  IndexInput* FSDirectory::FSIndexInput::clone() const
  {
    return _CLNEW FSDirectory::FSIndexInput(*this);
  }
  void FSDirectory::FSIndexInput::close()  {
	BufferedIndexInput::close();
#ifndef _CL_DISABLE_MULTITHREADING
	if ( handle != NULL ){
		//here we have a bit of a problem... we need to lock the handle to ensure that we can
		//safely delete the handle... but if we delete the handle, then the scoped unlock,
		//won't be able to unlock the mutex...

		//take a reference of the lock object...
		_LUCENE_THREADMUTEX* mutex = handle->SHARED_LOCK;
		//lock the mutex
		mutex->lock();

		//determine if we are about to delete the handle...
		bool dounlock = ( _LUCENE_ATOMIC_INT_GET(handle->__cl_refcount) > 1 );

    //decdelete (deletes if refcount is down to 0
		_CLDECDELETE(handle);

		//printf("handle=%d\n", handle->__cl_refcount);
		if ( dounlock ){
			mutex->unlock();
		}else{
			delete mutex;
		}
	}
#else
	_CLDECDELETE(handle);
#endif
  }

  void FSDirectory::FSIndexInput::seekInternal(const int64_t position)  {
	CND_PRECONDITION(position>=0 &&position<handle->_length,"Seeking out of range")
	_pos = position;
  }

/** IndexInput methods */
void FSDirectory::FSIndexInput::readInternal(uint8_t* b, const int32_t len) {
	CND_PRECONDITION(handle!=NULL,"shared file handle has closed");
	CND_PRECONDITION(handle->fhandle>=0,"file is not open");
	SCOPED_LOCK_MUTEX(*handle->SHARED_LOCK)

	if ( handle->_fpos != _pos ){
		if ( fileSeek(handle->fhandle,_pos,SEEK_SET) != _pos ){
			_CLTHROWA( CL_ERR_IO, "File IO Seek error");
		}
		handle->_fpos = _pos;
	}

	bufferLength = _read(handle->fhandle,b,len); // 2004.10.31:SF 1037836
	if (bufferLength == 0){
		_CLTHROWA(CL_ERR_IO, "read past EOF");
	}
	if (bufferLength == -1){
		//if (EINTR == errno) we could do something else... but we have
		//to guarantee some return, or throw EOF

		_CLTHROWA(CL_ERR_IO, "read error");
	}
	_pos+=bufferLength;
	handle->_fpos=_pos;
}

  FSDirectory::FSIndexOutput::FSIndexOutput(const char* path, int filemode){
	//O_BINARY - Opens file in binary (untranslated) mode
	//O_CREAT - Creates and opens new file for writing. Has no effect if file specified by filename exists
	//O_RANDOM - Specifies that caching is optimized for, but not restricted to, random access from disk.
	//O_WRONLY - Opens file for writing only;
    if ( filemode <= 0 ){
      filemode = 0644;
    }
	  if ( Misc::dir_Exists(path) )
	    fhandle = _cl_open( path, _O_BINARY | O_RDWR | _O_RANDOM | O_TRUNC, filemode);
	  else // added by JBP
	    fhandle = _cl_open( path, _O_BINARY | O_RDWR | _O_RANDOM | O_CREAT, filemode);

	  if ( fhandle < 0 ){
      int err = errno;
      if ( err == ENOENT )
	      _CLTHROWA(CL_ERR_IO, "File does not exist");
      else if ( err == EACCES )
          _CLTHROWA(CL_ERR_IO, "File Access denied");
      else if ( err == EMFILE )
          _CLTHROWA(CL_ERR_IO, "Too many open files");
    }
  }
  FSDirectory::FSIndexOutput::~FSIndexOutput(){
	if ( fhandle >= 0 ){
	  try {
        FSIndexOutput::close();
	  }catch(CLuceneError& err){
	    //ignore IO errors...
	    if ( err.number() != CL_ERR_IO )
	        throw;
	  }
	}
  }

  /** output methods: */
  void FSDirectory::FSIndexOutput::flushBuffer(const uint8_t* b, const int32_t size) {
	  CND_PRECONDITION(fhandle>=0,"file is not open");
      if ( size > 0 && _write(fhandle,b,size) != size )
        _CLTHROWA(CL_ERR_IO, "File IO Write error");
  }
  void FSDirectory::FSIndexOutput::close() {
    try{
      BufferedIndexOutput::close();
    }catch(CLuceneError& err){
	    //ignore IO errors...
	    if ( err.number() != CL_ERR_IO )
	        throw;
    }

    if ( ::_close(fhandle) != 0 )
      _CLTHROWA(CL_ERR_IO, "File IO Close error");
    else
      fhandle = -1; //-1 now indicates closed
  }

  void FSDirectory::FSIndexOutput::seek(const int64_t pos) {
    CND_PRECONDITION(fhandle>=0,"file is not open");
    BufferedIndexOutput::seek(pos);
	int64_t ret = fileSeek(fhandle,pos,SEEK_SET);
	if ( ret != pos ){
      _CLTHROWA(CL_ERR_IO, "File IO Seek error");
	}
  }
  int64_t FSDirectory::FSIndexOutput::length() const {
	  CND_PRECONDITION(fhandle>=0,"file is not open");
	  return fileSize(fhandle);
  }


	const char* FSDirectory::LOCK_DIR=NULL;
	const char* FSDirectory::getLockDir(){
		#ifdef LUCENE_LOCK_DIR
		LOCK_DIR = LUCENE_LOCK_DIR;
		#else
			#ifdef LUCENE_LOCK_DIR_ENV_1
			if ( LOCK_DIR == NULL )
				LOCK_DIR = getenv(LUCENE_LOCK_DIR_ENV_1);
			#endif
			#ifdef LUCENE_LOCK_DIR_ENV_2
			if ( LOCK_DIR == NULL )
				LOCK_DIR = getenv(LUCENE_LOCK_DIR_ENV_2);
			#endif
			#ifdef LUCENE_LOCK_DIR_ENV_FALLBACK
			if ( LOCK_DIR == NULL )
				LOCK_DIR=LUCENE_LOCK_DIR_ENV_FALLBACK;
			#endif
			if ( LOCK_DIR == NULL )
				_CLTHROWA(CL_ERR_IO, "Couldn't get determine lock dir");
		#endif

		return LOCK_DIR;
	}

  FSDirectory::FSDirectory():
   Directory(),
   refCount(0),
   useMMap(LUCENE_USE_MMAP)
  {
    filemode = 0644;
    this->lockFactory = NULL;
  }

  void FSDirectory::init(const char* _path, LockFactory* lockFactory)
  {
    directory = _path;
    bool doClearLockID = false;

    if ( lockFactory == NULL ) {
    	if ( disableLocks ) {
    		lockFactory = NoLockFactory::getNoLockFactory();
    	} else {
    		lockFactory = _CLNEW FSLockFactory( directory.c_str(), this->filemode );
    		doClearLockID = true;
    	}
    }

    setLockFactory( lockFactory );

    if ( doClearLockID ) {
    	lockFactory->setLockPrefix(NULL);
    }

    if (!Misc::dir_Exists(directory.c_str())){
      char* err = _CL_NEWARRAY(char,19+directory.length()+1); //19: len of " is not a directory"
      strcpy(err,directory.c_str());
      strcat(err," is not a directory");
      _CLTHROWA_DEL(CL_ERR_IO, err );
    }
  }


  void FSDirectory::create(){
    SCOPED_LOCK_MUTEX(THIS_LOCK)

	  //clear old files
	  vector<string> files;
	  Misc::listFiles(directory.c_str(), files, false);
	  vector<string>::iterator itr = files.begin();
	  while ( itr != files.end() ){
	  	if ( CL_NS(index)::IndexReader::isLuceneFile(itr->c_str()) ){
        if ( _unlink( (directory + PATH_DELIMITERA + *itr).c_str() ) == -1 ) {
				  _CLTHROWA(CL_ERR_IO, "Couldn't delete file "); //todo: make richer error
				}
	  	}
	  	itr++;
	  }
    lockFactory->clearLock( CL_NS(index)::IndexWriter::WRITE_LOCK_NAME );

  }

  void FSDirectory::priv_getFN(char* buffer, const char* name) const{
      buffer[0] = 0;
      strcpy(buffer,directory.c_str());
      strcat(buffer, PATH_DELIMITERA );
      strcat(buffer,name);
  }

  FSDirectory::~FSDirectory(){
  }

  void FSDirectory::setFileMode(int mode){
    this->filemode = mode;
  }
  int FSDirectory::getFileMode(){
    return this->filemode;
  }
  void FSDirectory::setUseMMap(bool value){ useMMap = value; }
  bool FSDirectory::getUseMMap() const{ return useMMap; }
  const char* FSDirectory::getClassName(){
    return "FSDirectory";
  }
  const char* FSDirectory::getObjectName() const{
    return getClassName();
  }

  void FSDirectory::setDisableLocks(bool doDisableLocks) { disableLocks = doDisableLocks; }
  bool FSDirectory::getDisableLocks() { return disableLocks; }


  bool FSDirectory::list(vector<string>* names) const{ //todo: fix this, ugly!!!
    CND_PRECONDITION(!directory.empty(),"directory is not open");
    return Misc::listFiles(directory.c_str(), *names, false);
  }

  bool FSDirectory::fileExists(const char* name) const {
	  CND_PRECONDITION(directory[0]!=0,"directory is not open");
    char fl[CL_MAX_DIR];
    priv_getFN(fl, name);
    return Misc::dir_Exists( fl );
  }

  const char* FSDirectory::getDirName() const{
    return directory.c_str();
  }

  FSDirectory* FSDirectory::getDirectory(const char* file, bool create, LockFactory* lockFactory){
    FSDirectory* dir = getDirectory(file, (LockFactory*)NULL);

    // This is now deprecated (creation should only be done
    // by IndexWriter):
    if (create) {
      dir->create();
    }

    return dir;
  }
  //static
  FSDirectory* FSDirectory::getDirectory(const char* _file, LockFactory* lockFactory){
    FSDirectory* dir = NULL;
	{
		if ( !_file || !*_file )
			_CLTHROWA(CL_ERR_IO,"Invalid directory");
    
    char buf[CL_MAX_PATH];
  	char* file = _realpath(_file,buf);//set a realpath so that if we change directory, we can still function
  	if ( !file || !*file ){
  		strncpy(buf, _file, CL_MAX_PATH);
      file = buf;
  	}
    
    struct cl_stat_t fstat;
		if ( fileStat(file,&fstat) == 0 && !(fstat.st_mode & S_IFDIR) ){
	      char tmp[1024];
	      _snprintf(tmp,1024,"%s not a directory", file);
	      _CLTHROWA(CL_ERR_IO,tmp);
		}

    if ( fileStat(file,&fstat) != 0 ) {
	  	//todo: should construct directory using _mkdirs... have to write replacement
      if ( _mkdir(file) == -1 ){
        string err = "Couldn't create directory: ";
        err += string(file);
			  _CLTHROWA(CL_ERR_IO, err.c_str() );
      }
		}


		SCOPED_LOCK_MUTEX(DIRECTORIES_LOCK)
		dir = DIRECTORIES.get(file);
		if ( dir == NULL  ){
      dir = _CLNEW FSDirectory();
      dir->init(_file,lockFactory);
			DIRECTORIES.put( dir->directory.c_str(), dir);
		} else {
			if ( lockFactory != NULL && lockFactory != dir->getLockFactory() ) {
				_CLTHROWA(CL_ERR_IO,"Directory was previously created with a different LockFactory instance, please pass NULL as the lockFactory instance and use setLockFactory to change it");
			}
		}

		{
			SCOPED_LOCK_MUTEX(dir->THIS_LOCK)
			dir->refCount++;
		}
	}

    return _CL_POINTER(dir); // TODO: Isn't this a double ref increment?
  }

  int64_t FSDirectory::fileModified(const char* name) const {
	CND_PRECONDITION(directory[0]!=0,"directory is not open");
    struct cl_stat_t buf;
    char buffer[CL_MAX_DIR];
    priv_getFN(buffer,name);
    if (fileStat( buffer, &buf ) == -1 )
      return 0;
    else
      return buf.st_mtime;
  }

  //static
  int64_t FSDirectory::fileModified(const char* dir, const char* name){
    struct cl_stat_t buf;
    char buffer[CL_MAX_DIR];
	_snprintf(buffer,CL_MAX_DIR,"%s%s%s",dir,PATH_DELIMITERA,name);
    fileStat( buffer, &buf );
    return buf.st_mtime;
  }

  void FSDirectory::touchFile(const char* name){
	  CND_PRECONDITION(directory[0]!=0,"directory is not open");
    char buffer[CL_MAX_DIR];
    _snprintf(buffer,CL_MAX_DIR,"%s%s%s",directory.c_str(),PATH_DELIMITERA,name);

    int32_t r = _cl_open(buffer, O_RDWR, this->filemode);
    if ( r < 0 )
      _CLTHROWA(CL_ERR_IO,"IO Error while touching file");
    ::_close(r);
  }

  int64_t FSDirectory::fileLength(const char* name) const {
	  CND_PRECONDITION(directory[0]!=0,"directory is not open");
    struct cl_stat_t buf;
    char buffer[CL_MAX_DIR];
    priv_getFN(buffer,name);
    if ( fileStat( buffer, &buf ) == -1 )
      return 0;
    else
      return buf.st_size;
  }

  bool FSDirectory::openInput(const char * name, IndexInput *& ret, CLuceneError& error, int32_t bufferSize)
  {
	CND_PRECONDITION(directory[0]!=0,"directory is not open")
    char fl[CL_MAX_DIR];
    priv_getFN(fl, name);
#ifdef LUCENE_FS_MMAP
	//todo: do some tests here... like if the file
	//is >2gb, then some system cannot mmap the file
	//also some file systems mmap will fail?? could detect here too
	if ( useMMap && Misc::file_Size(fl) < LUCENE_INT32_MAX_SHOULDBE ) //todo: would this be bigger on 64bit systems?. i suppose it would be...test first
		return MMapIndexInput::open( fl, ret, error, bufferSize );
	else
#endif
	return FSIndexInput::open( fl, ret, error, bufferSize );
  }

  void FSDirectory::close(){
    SCOPED_LOCK_MUTEX(DIRECTORIES_LOCK)
    {
	    THIS_LOCK.lock();

	    CND_PRECONDITION(directory[0]!=0,"directory is not open");

	    if (--refCount <= 0 ) {//refcount starts at 1
	        Directory* dir = DIRECTORIES.get(getDirName());
	        if(dir){
	            DIRECTORIES.remove( getDirName() ); //this will be removed in ~FSDirectory
	            _CLDECDELETE(dir);
              //NOTE: Don't unlock the mutex, since it has been destroyed now...
	            return;
	        }
	    }
	    THIS_LOCK.unlock();  
	  }
   }

   /**
   * So we can do some byte-to-hexchar conversion below
   */
	char HEX_DIGITS[] =
	{'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

	char* FSDirectory::getLockPrefix() const{
		char dirName[CL_MAX_PATH]; // name to be hashed
		if ( _realpath(directory.c_str(),dirName) == NULL ){
			_CLTHROWA(CL_ERR_Runtime,"Invalid directory path");
		}

		//to make a compatible name with jlucene, we need to make some changes...
		if ( dirName[1] == ':' )
			dirName[0] = (char)_totupper((char)dirName[0]);

		char* smd5 = MD5String(dirName);

		char* ret=_CL_NEWARRAY(char,32+7+1); //32=2*16, 7=strlen("lucene-")
		strcpy(ret,"lucene-");
		strcat(ret,smd5);

		_CLDELETE_CaARRAY(smd5);

	    return ret;
  }

  bool FSDirectory::doDeleteFile(const char* name)  {
	CND_PRECONDITION(directory[0]!=0,"directory is not open");
    char fl[CL_MAX_DIR];
    priv_getFN(fl, name);
	return _unlink(fl) != -1;
  }

  void FSDirectory::renameFile(const char* from, const char* to){
	CND_PRECONDITION(directory[0]!=0,"directory is not open");
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    char old[CL_MAX_DIR];
    priv_getFN(old, from);

    char nu[CL_MAX_DIR];
    priv_getFN(nu, to);

    /* This is not atomic.  If the program crashes between the call to
    delete() and the call to renameTo() then we're screwed, but I've
    been unable to figure out how else to do this... */

    if ( Misc::dir_Exists(nu) ){
      //we run this sequence of unlinking an arbitary 100 times
      //on some platforms (namely windows), there can be a
      //delay between unlink and dir_exists==false
        if( Misc::file_Unlink( nu ) == -1 ) {
    	    char* err = _CL_NEWARRAY(char,16+strlen(to)+1); //16: len of "couldn't delete "
    		strcpy(err,"couldn't delete ");
    		strcat(err,to);
            _CLTHROWA_DEL(CL_ERR_IO, err );
        }
    }
    if ( _rename(old,nu) != 0 ){
       //todo: jlucene has some extra rename code - if the rename fails, it copies
       //the whole file to the new file... might want to implement that if renaming
       //fails on some platforms
        char buffer[20+CL_MAX_PATH+CL_MAX_PATH];
        strcpy(buffer,"couldn't rename ");
        strcat(buffer,from);
        strcat(buffer," to ");
        strcat(buffer,nu);
      _CLTHROWA(CL_ERR_IO, buffer );
    }
  }

  IndexOutput* FSDirectory::createOutput(const char* name) {
	CND_PRECONDITION(directory[0]!=0,"directory is not open");
    char fl[CL_MAX_DIR];
    priv_getFN(fl, name);
	  if ( Misc::dir_Exists(fl) ){
          if ( Misc::file_Unlink( fl, 1 ) == -1 ) {
			  char tmp[1024];
			  strcpy(tmp, "Cannot overwrite: ");
			  strcat(tmp, name);
			  _CLTHROWA(CL_ERR_IO, tmp);
		  }
          assert( ! Misc::dir_Exists(fl) );
	  }
    return _CLNEW FSIndexOutput( fl, this->filemode );
  }

  string FSDirectory::toString() const{
	  return string("FSDirectory@") + this->directory;
  }

CL_NS_END
