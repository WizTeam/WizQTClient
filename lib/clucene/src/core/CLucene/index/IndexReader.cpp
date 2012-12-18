/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "IndexReader.h"
#include "IndexWriter.h"

#include "CLucene/store/Directory.h"
#include "CLucene/store/FSDirectory.h"
#include "CLucene/store/_Lock.h"
#include "CLucene/document/Document.h"
#include "CLucene/search/Similarity.h"
#include "CLucene/util/Misc.h"
#include "_SegmentInfos.h"
#include "_SegmentHeader.h"
#include "MultiReader.h"
#include "Terms.h"
#include <assert.h>

CL_NS_USE(util)
CL_NS_USE(store)
CL_NS_DEF(index)


  class IndexReaderFindSegmentsFile: public SegmentInfos::FindSegmentsFile<uint64_t>{
  public:
		IndexReaderFindSegmentsFile( CL_NS(store)::Directory* dir ):
			SegmentInfos::FindSegmentsFile<uint64_t>(dir){
		}
		IndexReaderFindSegmentsFile( const char* dir ):
			SegmentInfos::FindSegmentsFile<uint64_t>(dir){
		}
    uint64_t doBody(const char* segmentFileName){
      return directory->fileModified(segmentFileName);
    }
  };

	class CloseCallbackCompare:public CL_NS(util)::Compare::_base{
	public:
		bool operator()( IndexReader::CloseCallback t1, IndexReader::CloseCallback t2 ) const{
			return t1 > t2;
		}
		static void doDelete(IndexReader::CloseCallback /*dummy*/){
		}
	};


  class IndexReader::Internal: LUCENE_BASE{
  public:
    /**
    * @deprecated will be deleted when IndexReader(Directory) is deleted
    * @see #directory()
    */
    CL_NS(store)::Directory* directory;

    typedef CL_NS(util)::CLSet<IndexReader::CloseCallback, void*,
      CloseCallbackCompare,
      CloseCallbackCompare> CloseCallbackMap;
    CloseCallbackMap closeCallbacks;

    Internal(Directory* directory, IndexReader* _this)
    {
      if ( directory != NULL )
        this->directory = _CL_POINTER(directory);
      else
        this->directory = NULL;
      _this->closed = false;
      _this->hasChanges = false;
    }
    ~Internal(){
    }
  };

  IndexReader::IndexReader(Directory* dir){
  //Constructor.
  //Func - Creates an instance of IndexReader
  //Pre  - true
  //Post - An instance has been created with writeLock = NULL
    _internal = _CLNEW Internal(dir, this);
  }

  IndexReader::IndexReader(){
  //Constructor.
  //Func - Creates an instance of IndexReader
  //Pre  - true
  //Post - An instance has been created with writeLock = NULL
    _internal = _CLNEW Internal(NULL, this);
  }

  IndexReader::~IndexReader(){
  //Func - Destructor
  //       Destroys the instance and releases the writeLock if needed
  //Pre  - true
  //Post - The instance has been destroyed if pre(writeLock) exists is has been released
  	_CLLDELETE(_internal);
  }

  IndexReader* IndexReader::open(const char* path, bool closeDirectoryOnCleanup, IndexDeletionPolicy* deletionPolicy){
  //Func - Static method.
  //       Returns an IndexReader reading the index in an FSDirectory in the named path.
  //Pre  - path != NULL and contains the path of the index for which an IndexReader must be
  //       instantiated
  //       closeDir indicates if the directory needs to be closed
  //Post - An IndexReader has been returned that reads tnhe index located at path

	  CND_PRECONDITION(path != NULL, "path is NULL");
	   Directory* dir = FSDirectory::getDirectory(path);
     IndexReader* reader = open(dir,closeDirectoryOnCleanup,deletionPolicy);
     //because fsdirectory will now have a refcount of 1 more than
     //if the reader had been opened with a directory object,
     //we need to do a refdec
     _CLDECDELETE(dir);
     return reader;
  }

  IndexReader* IndexReader::open( Directory* directory, bool closeDirectory, IndexDeletionPolicy* deletionPolicy){
  //Func - Static method.
  //       Returns an IndexReader reading the index in an FSDirectory in the named path.
  //Pre  - directory represents a directory
  //       closeDir indicates if the directory needs to be closed
  //Post - An IndexReader has been returned that reads the index located at directory

       return DirectoryIndexReader::open(directory, closeDirectory, deletionPolicy);
  }

  IndexReader* IndexReader::reopen(){
	  _CLTHROWA(CL_ERR_UnsupportedOperation, "This reader does not support reopen().");
  }
  CL_NS(store)::Directory* IndexReader::directory() {
    ensureOpen();
    if (NULL != _internal->directory) {
      return _internal->directory;
    } else {
	  _CLTHROWA(CL_ERR_UnsupportedOperation, "This reader does not support this method.");
    }
  }

  void IndexReader::ensureOpen(){
  }

  void IndexReader::acquireWriteLock(){
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    /* NOOP */
  }

  CL_NS(document)::Document* IndexReader::document(const int32_t n){
    CL_NS(document)::Document* ret = _CLNEW CL_NS(document)::Document;
    if (!document(n,*ret) )
        _CLDELETE(ret);
    return ret;
  }

  uint64_t IndexReader::lastModified(const char* directory2) {
  //Func - Static method
  //       Returns the time the index in the named directory was last modified.
  //Pre  - directory != NULL and contains the path name of the directory to check
  //Post - The last modified time of the index has been returned

    CND_PRECONDITION(directory2 != NULL, "directory is NULL");

	  IndexReaderFindSegmentsFile runner(directory2);
	  return (uint64_t)runner.run();
  }

  int64_t IndexReader::getCurrentVersion(Directory* directory) {
		return SegmentInfos::readCurrentVersion(directory);
  }


   int64_t IndexReader::getCurrentVersion(const char* directory){
      Directory* dir = FSDirectory::getDirectory(directory);
      int64_t version = getCurrentVersion(dir);
      dir->close();
      _CLDECDELETE(dir);
      return version;
   }
    int64_t IndexReader::getVersion() {
          _CLTHROWA(CL_ERR_UnsupportedOperation, "This reader does not support this method.");
    }

  void IndexReader::setTermInfosIndexDivisor(int32_t /*indexDivisor*/) {
    _CLTHROWA(CL_ERR_UnsupportedOperation, "This reader does not support this method.");
  }

  int32_t IndexReader::getTermInfosIndexDivisor() {
    _CLTHROWA(CL_ERR_UnsupportedOperation, "This reader does not support this method.");
  }

  bool IndexReader::isCurrent() {
    _CLTHROWA(CL_ERR_UnsupportedOperation, "This reader does not support this method.");
  }
  bool IndexReader::isOptimized() {
    _CLTHROWA(CL_ERR_UnsupportedOperation, "This reader does not support this method.");
  }

  uint64_t IndexReader::lastModified(Directory* directory2) {
  //Func - Static method
  //       Returns the time the index in this directory was last modified.
  //Pre  - directory contains a valid reference
  //Post - The last modified time of the index has been returned
    IndexReaderFindSegmentsFile runner(directory2);
    return (uint64_t)runner.run();
  }

  void IndexReader::setNorm(int32_t doc, const TCHAR* field, uint8_t value){
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    this->ensureOpen();
    this->acquireWriteLock();
    this->hasChanges = true;
    this->doSetNorm(doc, field, value);
  }


  void IndexReader::setNorm(int32_t doc, const TCHAR* field, float_t value){
     ensureOpen();
     setNorm(doc, field, CL_NS(search)::Similarity::encodeNorm(value));
  }

  bool IndexReader::indexExists(const char* directory){
  //Func - Static method
  //       Checks if an index exists in the named directory
  //Pre  - directory != NULL
  //Post - Returns true if an index exists at the specified directory->
  //       If the directory does not exist or if there is no index in it.
  //       false is returned.
    std::vector<std::string> files;
    Misc::listFiles(directory, files);
    return SegmentInfos::getCurrentSegmentGeneration(files) != -1;
  }

  bool IndexReader::indexExists(const Directory* directory){
  //Func - Static method
  //       Checks if an index exists in the directory
  //Pre  - directory is a valid reference
  //Post - Returns true if an index exists at the specified directory->
  //       If the directory does not exist or if there is no index in it.
  //       false is returned.

    return SegmentInfos::getCurrentSegmentGeneration(directory) != -1;
  }

  TermDocs* IndexReader::termDocs(Term* term) {
  //Func - Returns an enumeration of all the documents which contain
  //       term. For each document, the document number, the frequency of
  //       the term in that document is also provided, for use in search scoring.
  //       Thus, this method implements the mapping:
  //
  //       Term => <docNum, freq>*
  //	   The enumeration is ordered by document number.  Each document number
  //       is greater than all that precede it in the enumeration.
  //Pre  - term != NULL
  //Post - A reference to TermDocs containing an enumeration of all found documents
  //       has been returned

      CND_PRECONDITION(term != NULL, "term is NULL");

      ensureOpen();
      //Reference an instantiated TermDocs instance
      TermDocs* _termDocs = termDocs();
      //Seek all documents containing term
      _termDocs->seek(term);
      //return the enumaration
      return _termDocs;
  }

  TermPositions* IndexReader::termPositions(Term* term){
  //Func - Returns an enumeration of all the documents which contain  term. For each
  //       document, in addition to the document number and frequency of the term in
  //       that document, a list of all of the ordinal positions of the term in the document
  //       is available.  Thus, this method implements the mapping:
  //
  //       Term => <docNum, freq,<pos 1, pos 2, ...pos freq-1>>*
  //
  //       This positional information faciliates phrase and proximity searching.
  //       The enumeration is ordered by document number.  Each document number is greater than
  //       all that precede it in the enumeration.
  //Pre  - term != NULL
  //Post - A reference to TermPositions containing an enumeration of all found documents
  //       has been returned

      CND_PRECONDITION(term != NULL, "term is NULL");

      ensureOpen();
      //Reference an instantiated termPositions instance
      TermPositions* _termPositions = termPositions();
	  //Seek all documents containing term
      _termPositions->seek(term);
	  //return the enumeration
      return _termPositions;
  }

  bool IndexReader::document(int32_t n, CL_NS(document)::Document* doc){
  	return document(n, *doc);
  }
  bool IndexReader::document(int32_t n, CL_NS(document)::Document& doc){
    ensureOpen();
    return document(n, doc, NULL);
  }

  void IndexReader::deleteDoc(const int32_t docNum){
    deleteDocument(docNum);
  }
  int32_t IndexReader::deleteTerm(Term* term){
    return deleteDocuments(term);
  }

  void IndexReader::deleteDocument(const int32_t docNum) {
  //Func - Deletes the document numbered docNum.  Once a document is deleted it will not appear
  //       in TermDocs or TermPostitions enumerations. Attempts to read its field with the document
  //       method will result in an error.  The presence of this document may still be reflected in
  //       the docFreq statistic, though this will be corrected eventually as the index is further modified.
  //Pre  - docNum >= 0
  //Post - If successful the document identified by docNum has been deleted. If no writelock
  //       could be obtained an exception has been thrown stating that the index was locked or has no write access

	  SCOPED_LOCK_MUTEX(THIS_LOCK)

     CND_PRECONDITION(docNum >= 0, "docNum is negative");

    ensureOpen();
    acquireWriteLock();

	  //Have the document identified by docNum deleted
    hasChanges = true;
    doDelete(docNum);
  }

  void IndexReader::flush() {
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    ensureOpen();
    commit();
  }

   void IndexReader::commit(){
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    if(hasChanges){
      doCommit();
    }
    hasChanges = false;
  }


  void IndexReader::undeleteAll(){
     SCOPED_LOCK_MUTEX(THIS_LOCK)
    ensureOpen();
    acquireWriteLock();
    hasChanges = true;
    doUndeleteAll();
  }

  int32_t IndexReader::deleteDocuments(Term* term) {
  //Func - Deletes all documents containing term. This is useful if one uses a
  //       document field to hold a unique ID string for the document.  Then to delete such
  //       a document, one merely constructs a term with the appropriate field and the unique
  //       ID string as its text and passes it to this method.
  //Pre  - term != NULL
  //Post - All documents containing term have been deleted. The number of deleted documents
  //       has been returned

      CND_PRECONDITION(term != NULL, "term is NULL");
      ensureOpen();

	  //Search for the documents contain term
      TermDocs* docs = termDocs(term);

	  //Check if documents have been found
	  if ( docs == NULL ){
          return 0;
	  }

	  //initialize
	  int32_t Counter = 0;
      try {
		  //iterate through the found documents
          while (docs->next()) {
			  //Delete the document
              deleteDocument(docs->doc());
              ++Counter;
          }
      }_CLFINALLY(
		  //Close the enumeration
          docs->close();
          );

    //Delete the enumeration of found documents
    _CLDELETE( docs );

	//Return the number of deleted documents
    return Counter;
  }


  void IndexReader::close() {
  //Func - Closes files associated with this index and also saves any new deletions to disk.
  //       No other methods should be called after this has been called.
  //Pre  - true
  //Post - All files associated with this index have been deleted and new deletions have been
  //       saved to disk
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    if ( !closed ){
      Internal::CloseCallbackMap::iterator iter = _internal->closeCallbacks.begin();
      for ( ;iter!=_internal->closeCallbacks.end();iter++){
        CloseCallback callback = *iter->first;
        callback(this,iter->second);
      }
      commit();
      doClose();
    }
	closed = true;
  }

  bool IndexReader::isLocked(Directory* directory) {
  //Func - Static method
  //       Checks if the index in the directory is currently locked.
  //Pre  - directory is a valid reference to a directory to check for a lock
  //Post - Returns true if the index in the named directory is locked otherwise false

	  //Check the existence of the file write.lock and return true when it does and false
	  //when it doesn't
     LuceneLock* l = directory->makeLock("write.lock");
     bool ret = l->isLocked();
     _CLDELETE(l);
     return ret;
  }

  bool IndexReader::isLocked(const char* directory) {
  //Func - Static method
  //       Checks if the index in the named directory is currently locked.
  //Pre  - directory != NULL and contains the directory to check for a lock
  //Post - Returns true if the index in the named directory is locked otherwise false

      CND_PRECONDITION(directory != NULL, "directory is NULL");

      Directory* dir = FSDirectory::getDirectory(directory);
      bool ret = isLocked(dir);
      dir->close();
      _CLDECDELETE(dir);

	  return ret;
  }

bool IndexReader::hasNorms(const TCHAR* field) {
	// backward compatible implementation.
	// SegmentReader has an efficient implementation.
  ensureOpen();
	return norms(field) != NULL;
}

void IndexReader::unlock(const char* path){
	FSDirectory* dir = FSDirectory::getDirectory(path);
	unlock(dir);
	dir->close();
	_CLDECDELETE(dir);
}
  void IndexReader::unlock(Directory* directory){
  //Func - Static method
  //       Forcibly unlocks the index in the named directory->
  //       Caution: this should only be used by failure recovery code,
  //       when it is known that no other process nor thread is in fact
  //       currently accessing this index.
  //Pre  - directory is a valid reference to a directory
  //Post - The directory has been forcibly unlocked
      LuceneLock* lock = directory->makeLock("write.lock");
      lock->release();
      _CLDELETE(lock);
  }

bool IndexReader::isLuceneFile(const char* filename){
	if ( !filename )
		return false;
	size_t len = strlen(filename);
	if ( len < 6 ) //need at least x.frx
		return false;
	const char* ext=filename+len;
	while(*ext != '.' && ext!=filename)
		ext--;

	if ( strcmp(ext, ".cfs") == 0 )
		return true;
	else if ( strcmp(ext, ".fnm") == 0 )
		return true;
	else if ( strcmp(ext, ".fdx") == 0 )
		return true;
	else if ( strcmp(ext, ".fdt") == 0 )
		return true;
	else if ( strcmp(ext, ".tii") == 0 )
		return true;
	else if ( strcmp(ext, ".tis") == 0 )
		return true;
	else if ( strcmp(ext, ".frq") == 0 )
		return true;
	else if ( strcmp(ext, ".prx") == 0 )
		return true;
	else if ( strcmp(ext, ".del") == 0 )
		return true;
	else if ( strcmp(ext, ".tvx") == 0 )
		return true;
	else if ( strcmp(ext, ".tvd") == 0 )
		return true;
	else if ( strcmp(ext, ".tvf") == 0 )
		return true;
	else if ( strcmp(ext, ".tvp") == 0 )
		return true;

	else if ( strcmp(filename, "segments") == 0 )
		return true;
	else if ( strcmp(filename, "segments.new") == 0 )
		return true;
	else if ( strcmp(filename, "deletable") == 0 )
		return true;

	else if ( strncmp(ext,".f",2)==0 ){
		const char* n = ext+2;
		if ( *n && _istdigit(*n) )
			return true;
	}

	return false;
}

CL_NS(store)::Directory* IndexReader::getDirectory() {
    return directory();
}

	void IndexReader::addCloseCallback(CloseCallback callback, void* parameter){
		_internal->closeCallbacks.put(callback, parameter);
	}

CL_NS_END
