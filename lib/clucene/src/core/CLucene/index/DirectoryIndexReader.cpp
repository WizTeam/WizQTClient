/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "DirectoryIndexReader.h"
#include "_IndexFileDeleter.h"
#include "IndexDeletionPolicy.h"
#include "_MultiSegmentReader.h"
#include "_SegmentHeader.h"
#include "IndexWriter.h"
#include "CLucene/store/Directory.h"
#include "CLucene/store/Lock.h"
#include "_SegmentInfos.h"

CL_NS_USE(store)
CL_NS_USE(util)

CL_NS_DEF(index)


  void DirectoryIndexReader::doClose() {
    if(closeDirectory && _directory){
        _directory->close();
    }
    _CLDECDELETE(_directory);
  }

  void DirectoryIndexReader::doCommit() {
    if(hasChanges){
      if (segmentInfos != NULL) {

        // Default deleter (for backwards compatibility) is
        // KeepOnlyLastCommitDeleter:
        IndexFileDeleter deleter(_directory,
                                                         deletionPolicy == NULL ? _CLNEW KeepOnlyLastCommitDeletionPolicy() : deletionPolicy,
                                                         segmentInfos, NULL, NULL);

        // Checkpoint the state we are about to change, in
        // case we have to roll back:
        startCommit();

        bool success = false;
        try {
          commitChanges();
          segmentInfos->write(_directory);
          success = true;
        } _CLFINALLY (

          if (!success) {

            // Rollback changes that were made to
            // SegmentInfos but failed to get [fully]
            // committed.  This way this reader instance
            // remains consistent (matched to what's
            // actually in the index):
            rollbackCommit();

            // Recompute deletable files & remove them (so
            // partially written .del files, etc, are
            // removed):
            deleter.refresh();
          }
        )

        // Have the deleter remove any now unreferenced
        // files due to this commit:
        deleter.checkpoint(segmentInfos, true);

        if (writeLock != NULL) {
          writeLock->release();  // release write lock
          _CLDELETE(writeLock);
        }
      }
      else
        commitChanges();
    }
    hasChanges = false;
  }

  void DirectoryIndexReader::acquireWriteLock() {
    if (segmentInfos != NULL) {
      ensureOpen();
      if (stale)
        _CLTHROWA(CL_ERR_StaleReader, "IndexReader out of date and no longer valid for delete, undelete, or setNorm operations");

      if (writeLock == NULL) {
        LuceneLock* writeLock = _directory->makeLock(IndexWriter::WRITE_LOCK_NAME);
        if (!writeLock->obtain(IndexWriter::WRITE_LOCK_TIMEOUT)) { // obtain write lock
          string message = string("Index locked for write: ") + writeLock->getObjectName();
          _CLDELETE(writeLock);
          _CLTHROWA(CL_ERR_LockObtainFailed, message.c_str());
        }
        this->writeLock = writeLock;

        // we have to check whether index has changed since this reader was opened.
        // if so, this reader is no longer valid for deletion
        if (SegmentInfos::readCurrentVersion(_directory) > segmentInfos->getVersion()) {
          stale = true;
          this->writeLock->release();
          _CLDELETE(writeLock);
          _CLTHROWA(CL_ERR_StaleReader, "IndexReader out of date and no longer valid for delete, undelete, or setNorm operations");
        }
      }
    }
  }

  void DirectoryIndexReader::init(Directory* __directory, SegmentInfos* segmentInfos, bool closeDirectory) {
    this->deletionPolicy = NULL;
    this->stale = false;
    this->writeLock = NULL;
    this->rollbackSegmentInfos = NULL;
    this->_directory = _CL_POINTER(__directory);
    this->segmentInfos = segmentInfos;
    this->closeDirectory = closeDirectory;
  }

  DirectoryIndexReader::DirectoryIndexReader():
    IndexReader()
  {
  }
  DirectoryIndexReader::~DirectoryIndexReader(){
    try {
      if (writeLock != NULL) {
        writeLock->release();                        // release write lock
        writeLock = NULL;
      }
    }catch(...){
    }
     _CLDELETE(segmentInfos);
     _CLDELETE(rollbackSegmentInfos);
  }
  DirectoryIndexReader::DirectoryIndexReader(Directory* __directory, SegmentInfos* segmentInfos, bool closeDirectory):
    IndexReader()
  {
    init(__directory, segmentInfos, closeDirectory);
  }

  class DirectoryIndexReader::FindSegmentsFile_Open: public SegmentInfos::FindSegmentsFile<DirectoryIndexReader*>{
    bool closeDirectory;
    IndexDeletionPolicy* deletionPolicy;
	protected:
    DirectoryIndexReader* doBody(const char* segmentFileName) {

      SegmentInfos* infos = _CLNEW SegmentInfos;
      infos->read(directory, segmentFileName);

      DirectoryIndexReader* reader;

      if (infos->size() == 1) {          // index is optimized
        reader = SegmentReader::get(infos, infos->info(0), closeDirectory);
      } else {
        reader = _CLNEW MultiSegmentReader(directory, infos, closeDirectory);
      }
      reader->setDeletionPolicy(deletionPolicy);
      return reader;
    }
  public:
    FindSegmentsFile_Open( bool closeDirectory, IndexDeletionPolicy* deletionPolicy,
        CL_NS(store)::Directory* dir ):
      SegmentInfos::FindSegmentsFile<DirectoryIndexReader*>(dir)
    {
      this->closeDirectory = closeDirectory;
      this->deletionPolicy = deletionPolicy;
    }
  };

  DirectoryIndexReader* DirectoryIndexReader::open(Directory* __directory, bool closeDirectory, IndexDeletionPolicy* deletionPolicy) {
    DirectoryIndexReader::FindSegmentsFile_Open runner(closeDirectory, deletionPolicy, __directory);
    return runner.run();
  }


  class DirectoryIndexReader::FindSegmentsFile_Reopen: public SegmentInfos::FindSegmentsFile<DirectoryIndexReader*>{
    bool closeDirectory;
    IndexDeletionPolicy* deletionPolicy;
    DirectoryIndexReader* _this;
	protected:
    DirectoryIndexReader* doBody(const char* segmentFileName) {
      SegmentInfos* infos = _CLNEW SegmentInfos();
      infos->read(directory, segmentFileName);

      DirectoryIndexReader* newReader = _this->doReopen(infos);

      if (_this != newReader) {
        newReader->init(directory, infos, closeDirectory);
        newReader->deletionPolicy = deletionPolicy;
      }

      return newReader;
    }
  public:
    FindSegmentsFile_Reopen( bool closeDirectory, IndexDeletionPolicy* deletionPolicy,
        CL_NS(store)::Directory* dir, DirectoryIndexReader* _this ):
      SegmentInfos::FindSegmentsFile<DirectoryIndexReader*>(dir)
    {
      this->closeDirectory = closeDirectory;
      this->deletionPolicy = deletionPolicy;
      this->_this = _this;
    }
  };

  IndexReader* DirectoryIndexReader::reopen(){
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    ensureOpen();

    if (this->hasChanges || this->isCurrent()) {
      // the index hasn't changed - nothing to do here
      return this;
    }
    FindSegmentsFile_Reopen runner(closeDirectory, deletionPolicy, _directory, this);
    IndexReader* ret = runner.run();

    //disown this memory...
    this->writeLock = NULL;
    this->_directory = NULL;
    this->deletionPolicy = NULL;

    return ret;
  }

  void DirectoryIndexReader::setDeletionPolicy(IndexDeletionPolicy* deletionPolicy) {
    this->deletionPolicy = deletionPolicy;
  }

  /** Returns the directory this index resides in.
   */
  Directory* DirectoryIndexReader::directory() {
    ensureOpen();
    return _directory;
  }

  /**
   * Version number when this IndexReader was opened.
   */
  int64_t DirectoryIndexReader::getVersion() {
    ensureOpen();
    return segmentInfos->getVersion();
  }

  /**
   * Check whether this IndexReader is still using the
   * current (i.e., most recently committed) version of the
   * index.  If a writer has committed any changes to the
   * index since this reader was opened, this will return
   * <code>false</code>, in which case you must open a _CLNEW
   * IndexReader in order to see the changes.  See the
   * description of the <a href="IndexWriter.html#autoCommit"><code>autoCommit</code></a>
   * flag which controls when the {@link IndexWriter}
   * actually commits changes to the index.
   *
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  bool DirectoryIndexReader::isCurrent(){
    ensureOpen();
    return SegmentInfos::readCurrentVersion(_directory) == segmentInfos->getVersion();
  }

  /**
   * Checks is the index is optimized (if it has a single segment and no deletions)
   * @return <code>true</code> if the index is optimized; <code>false</code> otherwise
   */
  bool DirectoryIndexReader::isOptimized() {
    ensureOpen();
    return segmentInfos->size() == 1 && hasDeletions() == false;
  }

  /**
   * Should internally checkpoint state that will change
   * during commit so that we can rollback if necessary.
   */
  void DirectoryIndexReader::startCommit() {
    if (segmentInfos != NULL) {
      rollbackSegmentInfos = segmentInfos->clone();
    }
    rollbackHasChanges = hasChanges;
  }

  /**
   * Rolls back state to just before the commit (this is
   * called by commit() if there is some exception while
   * committing).
   */
  void DirectoryIndexReader::rollbackCommit() {
    if (segmentInfos != NULL) {
      for(int32_t i=0;i<segmentInfos->size();i++) {
        // Rollback each segmentInfo.  Because the
        // SegmentReader holds a reference to the
        // SegmentInfo we can't [easily] just replace
        // segmentInfos, so we reset it in place instead:
        segmentInfos->info(i)->reset(rollbackSegmentInfos->info(i));
      }
      _CLDELETE(rollbackSegmentInfos);
    }

    hasChanges = rollbackHasChanges;
  }

CL_NS_END
