/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "IndexWriter.h"
#include "IndexReader.h"
#include "CLucene/document/Document.h"
#include "CLucene/store/Directory.h"
#include "CLucene/search/Similarity.h"
#include "CLucene/util/Misc.h"

#include "CLucene/store/_Lock.h"
#include "CLucene/store/_RAMDirectory.h"
#include "CLucene/store/FSDirectory.h"
#include "CLucene/util/Array.h"
#include "CLucene/util/PriorityQueue.h"
#include "_DocumentsWriter.h"
#include "_TermInfo.h"
#include "_SegmentInfos.h"
#include "_SegmentMerger.h"
#include "_SegmentHeader.h"
#include "CLucene/search/Similarity.h"
#include "CLucene/index/MergePolicy.h"
#include "MergePolicy.h"
#include "MergeScheduler.h"
#include "_IndexFileDeleter.h"
#include "_Term.h"
#include <assert.h>
#include <algorithm>
#include <iostream>

CL_NS_USE(store)
CL_NS_USE(util)
CL_NS_USE(document)
CL_NS_USE(analysis)
CL_NS_USE(search)
CL_NS_DEF(index)

int64_t IndexWriter::WRITE_LOCK_TIMEOUT = 1000;
const char* IndexWriter::WRITE_LOCK_NAME = "write.lock";
std::ostream* IndexWriter::defaultInfoStream = NULL;

const int32_t IndexWriter::MERGE_READ_BUFFER_SIZE = 4096;
const int32_t IndexWriter::DISABLE_AUTO_FLUSH = -1;
const int32_t IndexWriter::DEFAULT_MAX_BUFFERED_DOCS = DISABLE_AUTO_FLUSH;
const float_t IndexWriter::DEFAULT_RAM_BUFFER_SIZE_MB = 16.0;
const int32_t IndexWriter::DEFAULT_MAX_BUFFERED_DELETE_TERMS = DISABLE_AUTO_FLUSH;
const int32_t IndexWriter::DEFAULT_MAX_MERGE_DOCS = LogDocMergePolicy::DEFAULT_MAX_MERGE_DOCS;
const int32_t IndexWriter::DEFAULT_MERGE_FACTOR = LogMergePolicy::DEFAULT_MERGE_FACTOR;

DEFINE_MUTEX(IndexWriter::MESSAGE_ID_LOCK)
int32_t IndexWriter::MESSAGE_ID = 0;
const int32_t IndexWriter::MAX_TERM_LENGTH = DocumentsWriter::MAX_TERM_LENGTH;

class IndexWriter::Internal{
public:
  IndexWriter* _this;
  Internal(IndexWriter* _this){
    this->_this = _this;
  }
  // Apply buffered delete terms to the segment just flushed from ram
  // apply appropriately so that a delete term is only applied to
  // the documents buffered before it, not those buffered after it.
  void applyDeletesSelectively(const DocumentsWriter::TermNumMapType& deleteTerms,
    const std::vector<int32_t>& deleteIds,IndexReader* reader);

  // Apply buffered delete terms to this reader.
  void applyDeletes(const DocumentsWriter::TermNumMapType& deleteTerms, IndexReader* reader);
};

void IndexWriter::deinit(bool releaseWriteLock) throw() {
  if (writeLock != NULL && releaseWriteLock) {
    writeLock->release(); // release write lock
    _CLLDELETE(writeLock);
  }
  _CLLDELETE(segmentInfos);
  _CLLDELETE(mergingSegments);
  _CLLDELETE(pendingMerges);
  _CLLDELETE(runningMerges);
  _CLLDELETE(mergeExceptions);
  _CLLDELETE(segmentsToOptimize);
  _CLLDELETE(mergeScheduler);
  _CLLDELETE(mergePolicy);
  _CLLDELETE(deleter);
  _CLLDELETE(docWriter);
  if (bOwnsDirectory) _CLLDECDELETE(directory);
  delete _internal;
}

IndexWriter::~IndexWriter(){
  deinit();
}

void IndexWriter::ensureOpen()   {
  if (closed) {
    _CLTHROWA(CL_ERR_AlreadyClosed, "this IndexWriter is closed");
  }
}

void IndexWriter::message(string message) {
  if (infoStream != NULL){
    (*infoStream) << string("IW ") << Misc::toString(messageID) << string(" [")
    						  << Misc::toString( _LUCENE_CURRTHREADID ) << string("]: ") << message << string("\n");
  }
}

void IndexWriter::setMessageID() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  if (infoStream != NULL && messageID == -1) {
    { SCOPED_LOCK_MUTEX(MESSAGE_ID_LOCK)
      messageID = MESSAGE_ID++;
    }
  }
}

LogMergePolicy* IndexWriter::getLogMergePolicy() const{
  if ( mergePolicy->instanceOf(LogMergePolicy::getClassName()) )
    return (LogMergePolicy*) mergePolicy;
  else
    _CLTHROWA(CL_ERR_IllegalArgument, "this method can only be called when the merge policy is the default LogMergePolicy");
}

bool IndexWriter::getUseCompoundFile() {
  return getLogMergePolicy()->getUseCompoundFile();
}


void IndexWriter::setUseCompoundFile(bool value) {
  getLogMergePolicy()->setUseCompoundFile(value);
  getLogMergePolicy()->setUseCompoundDocStore(value);
}

void IndexWriter::setSimilarity(Similarity* similarity) {
  ensureOpen();
  this->similarity = similarity;
}

Similarity* IndexWriter::getSimilarity() {
  ensureOpen();
  return this->similarity;
}


void IndexWriter::setTermIndexInterval(int32_t interval) {
  ensureOpen();
  this->termIndexInterval = interval;
}

int32_t IndexWriter::getTermIndexInterval() {
  ensureOpen();
  return termIndexInterval;
}

IndexWriter::IndexWriter(const char* path, Analyzer* a, bool create):bOwnsDirectory(true){
    init(FSDirectory::getDirectory(path, create), a, create, true, (IndexDeletionPolicy*)NULL, true);
}

IndexWriter::IndexWriter(Directory* d, Analyzer* a, bool create, bool closeDir):bOwnsDirectory(false){
  init(d, a, create, closeDir, NULL, true);
}

IndexWriter::IndexWriter(Directory* d, bool autoCommit, Analyzer* a, IndexDeletionPolicy* deletionPolicy, bool closeDirOnShutdown):bOwnsDirectory(false){
  init(d, a, closeDirOnShutdown, deletionPolicy, autoCommit);
}

IndexWriter::IndexWriter(Directory* d, bool autoCommit, Analyzer* a, bool create, IndexDeletionPolicy* deletionPolicy, bool closeDirOnShutdown):bOwnsDirectory(false){
  init(d, a, create, closeDirOnShutdown, deletionPolicy, autoCommit);
}

void IndexWriter::init(Directory* d, Analyzer* a, bool closeDir, IndexDeletionPolicy* deletionPolicy, bool autoCommit){
  if (IndexReader::indexExists(d)) {
    init(d, a, false, closeDir, deletionPolicy, autoCommit);
  } else {
    init(d, a, true, closeDir, deletionPolicy, autoCommit);
  }
}

void IndexWriter::init(Directory* d, Analyzer* a, const bool create, const bool closeDir,
                       IndexDeletionPolicy* deletionPolicy, const bool autoCommit){
  this->_internal = new Internal(this);
  this->termIndexInterval = IndexWriter::DEFAULT_TERM_INDEX_INTERVAL;
  this->mergeScheduler = _CLNEW SerialMergeScheduler(); //TODO: implement and use ConcurrentMergeScheduler
  this->mergingSegments = _CLNEW MergingSegmentsType;
  this->pendingMerges = _CLNEW PendingMergesType;
  this->runningMerges = _CLNEW RunningMergesType;
  this->mergeExceptions = _CLNEW MergeExceptionsType;
  this->segmentsToOptimize = _CLNEW SegmentsToOptimizeType;
  this->mergePolicy = _CLNEW LogByteSizeMergePolicy();
  this->localRollbackSegmentInfos = NULL;
  this->stopMerges = false;
  messageID = -1;
  maxFieldLength = FIELD_TRUNC_POLICY__WARN;
  infoStream = NULL;
  this->mergeFactor = this->minMergeDocs = this->maxMergeDocs = 0;
  this->commitLockTimeout =0;
  this->closeDir = closeDir;
  this->commitPending = this->closed = this->closing = false;
  directory = d;
  analyzer = a;
  this->infoStream = defaultInfoStream;
  setMessageID();
  this->writeLockTimeout = IndexWriter::WRITE_LOCK_TIMEOUT;
  this->similarity = Similarity::getDefault();
  this->hitOOM = false;
  this->autoCommit = true;
  this->segmentInfos = _CLNEW SegmentInfos;
  this->mergeGen = 0;
  this->rollbackSegmentInfos = NULL;
  this->deleter = NULL;
  this->docWriter = NULL;
  this->writeLock = NULL;

  if (create) {
    // Clear the write lock in case it's leftover:
    directory->clearLock(IndexWriter::WRITE_LOCK_NAME);
  }

  bool hasLock = false;
  try {
    writeLock = directory->makeLock(IndexWriter::WRITE_LOCK_NAME);
    hasLock = writeLock->obtain(writeLockTimeout);
    if (!hasLock) // obtain write lock
      _CLTHROWA(CL_ERR_LockObtainFailed, (string("Index locked for write: ") + writeLock->getObjectName()).c_str() );
  } catch (...) {
    deinit(hasLock);
    throw;
  }

  try {
    if (create) {
      // Try to read first.  This is to allow create
      // against an index that's currently open for
      // searching.  In this case we write the next
      // segments_N file with no segments:
      try {
        segmentInfos->read(directory);
        segmentInfos->clear();
      } catch (CLuceneError& e) {
        if ( e.number() != CL_ERR_IO ) throw e;
        // Likely this means it's a fresh directory
      }
      segmentInfos->write(directory);
    } else {
      segmentInfos->read(directory);
    }

    this->autoCommit = autoCommit;
    if (!autoCommit) {
      rollbackSegmentInfos = segmentInfos->clone();
    }else{
      rollbackSegmentInfos = NULL;
    }

    docWriter = _CLNEW DocumentsWriter(directory, this);
    docWriter->setInfoStream(infoStream);

    // Default deleter (for backwards compatibility) is
    // KeepOnlyLastCommitDeleter:
    deleter = _CLNEW IndexFileDeleter(directory,
                                   deletionPolicy == NULL ? _CLNEW KeepOnlyLastCommitDeletionPolicy() : deletionPolicy,
                                   segmentInfos, infoStream, docWriter);

    pushMaxBufferedDocs();

    if (infoStream != NULL) {
      message( string("init: create=") + (create ? "true" : "false") );
      messageState();
    }

  } catch (CLuceneError& e) {
    deinit(e.number() == CL_ERR_IO);
    throw e;
  }
}

void IndexWriter::setMergePolicy(MergePolicy* mp) {
  ensureOpen();
  if (mp == NULL)
    _CLTHROWA(CL_ERR_NullPointer, "MergePolicy must be non-NULL");

  if (mergePolicy != mp){
    mergePolicy->close();
    _CLDELETE(mergePolicy);
  }
  mergePolicy = mp;
  pushMaxBufferedDocs();
  if (infoStream != NULL)
    message(string("setMergePolicy ") + mp->getObjectName());
}

MergePolicy* IndexWriter::getMergePolicy() {
  ensureOpen();
  return mergePolicy;
}

void IndexWriter::setMergeScheduler(MergeScheduler* mergeScheduler) {
  ensureOpen();
  if (mergeScheduler == NULL)
    _CLTHROWA(CL_ERR_NullPointer, "MergeScheduler must be non-NULL");

  if (this->mergeScheduler != mergeScheduler) {
    finishMerges(true);
    this->mergeScheduler->close();
    _CLLDELETE(this->mergeScheduler)
  }
  this->mergeScheduler = mergeScheduler;
  if (infoStream != NULL)
    message( string("setMergeScheduler ") + mergeScheduler->getObjectName());
}

MergeScheduler* IndexWriter::getMergeScheduler() {
  ensureOpen();
  return mergeScheduler;
}

void IndexWriter::setMaxMergeDocs(int32_t maxMergeDocs) {
  getLogMergePolicy()->setMaxMergeDocs(maxMergeDocs);
}

int32_t IndexWriter::getMaxMergeDocs() const{
  return getLogMergePolicy()->getMaxMergeDocs();
}

void IndexWriter::setMaxFieldLength(int32_t maxFieldLength) {
  ensureOpen();
  this->maxFieldLength = maxFieldLength;
  if (infoStream != NULL)
    message( "setMaxFieldLength " + Misc::toString(maxFieldLength) );
}

int32_t IndexWriter::getMaxFieldLength() {
  ensureOpen();
  return maxFieldLength;
}

void IndexWriter::setMaxBufferedDocs(int32_t maxBufferedDocs) {
  ensureOpen();
  if (maxBufferedDocs != DISABLE_AUTO_FLUSH && maxBufferedDocs < 2)
    _CLTHROWA(CL_ERR_IllegalArgument,
        "maxBufferedDocs must at least be 2 when enabled");
  if (maxBufferedDocs == DISABLE_AUTO_FLUSH
      && (int32_t)getRAMBufferSizeMB() == DISABLE_AUTO_FLUSH)
    _CLTHROWA(CL_ERR_IllegalArgument,
        "at least one of ramBufferSize and maxBufferedDocs must be enabled");
  docWriter->setMaxBufferedDocs(maxBufferedDocs);
  pushMaxBufferedDocs();
  if (infoStream != NULL)
    message("setMaxBufferedDocs " + Misc::toString(maxBufferedDocs));
}

void IndexWriter::pushMaxBufferedDocs() {
  if (docWriter->getMaxBufferedDocs() != DISABLE_AUTO_FLUSH) {
    const MergePolicy* mp = mergePolicy;
    if (mp->instanceOf(LogDocMergePolicy::getClassName())) {
      LogDocMergePolicy* lmp = (LogDocMergePolicy*) mp;
      const int32_t maxBufferedDocs = docWriter->getMaxBufferedDocs();
      if (lmp->getMinMergeDocs() != maxBufferedDocs) {
        if (infoStream != NULL){
          message(string("now push maxBufferedDocs ") + Misc::toString(maxBufferedDocs) + " to LogDocMergePolicy");
        }
        lmp->setMinMergeDocs(maxBufferedDocs);
      }
    }
  }
}

int32_t IndexWriter::getMaxBufferedDocs() {
  ensureOpen();
  return docWriter->getMaxBufferedDocs();
}

void IndexWriter::setRAMBufferSizeMB(float_t mb) {
  if ( (int32_t)mb != DISABLE_AUTO_FLUSH && mb <= 0.0)
    _CLTHROWA(CL_ERR_IllegalArgument,
        "ramBufferSize should be > 0.0 MB when enabled");
  if (mb == DISABLE_AUTO_FLUSH && getMaxBufferedDocs() == DISABLE_AUTO_FLUSH)
    _CLTHROWA(CL_ERR_IllegalArgument,
        "at least one of ramBufferSize and maxBufferedDocs must be enabled");
  docWriter->setRAMBufferSizeMB(mb);
  if (infoStream != NULL){
    message(string("setRAMBufferSizeMB ") + Misc::toString(mb));
  }
}

float_t IndexWriter::getRAMBufferSizeMB() {
  return docWriter->getRAMBufferSizeMB();
}

void IndexWriter::setMaxBufferedDeleteTerms(int32_t maxBufferedDeleteTerms) {
  ensureOpen();
  if (maxBufferedDeleteTerms != DISABLE_AUTO_FLUSH
      && maxBufferedDeleteTerms < 1)
    _CLTHROWA(CL_ERR_IllegalArgument,
        "maxBufferedDeleteTerms must at least be 1 when enabled");
  docWriter->setMaxBufferedDeleteTerms(maxBufferedDeleteTerms);
  if (infoStream != NULL)
    message("setMaxBufferedDeleteTerms " + Misc::toString(maxBufferedDeleteTerms));
}

int32_t IndexWriter::getMaxBufferedDeleteTerms() {
  ensureOpen();
  return docWriter->getMaxBufferedDeleteTerms();
}

void IndexWriter::setMergeFactor(int32_t mergeFactor) {
  getLogMergePolicy()->setMergeFactor(mergeFactor);
}

int32_t IndexWriter::getMergeFactor() const {
  return getLogMergePolicy()->getMergeFactor();
}

void IndexWriter::setDefaultInfoStream(std::ostream* infoStream) {
  IndexWriter::defaultInfoStream = infoStream;
}

std::ostream* IndexWriter::getDefaultInfoStream() {
  return IndexWriter::defaultInfoStream;
}

//TODO: infoStream - unicode
void IndexWriter::setInfoStream(std::ostream* infoStream) {
  ensureOpen();
  this->infoStream = infoStream;
  setMessageID();
  docWriter->setInfoStream(infoStream);
  deleter->setInfoStream(infoStream);
  if (infoStream != NULL)
    messageState();
}

void IndexWriter::messageState() {
  message( string("setInfoStream: dir=") + directory->toString() +
          " autoCommit=" + (autoCommit?"true":"false" +
          string(" mergePolicy=") + mergePolicy->getObjectName() +
          " mergeScheduler=" + mergeScheduler->getObjectName() +
          " ramBufferSizeMB=" + Misc::toString(docWriter->getRAMBufferSizeMB()) +
          " maxBuffereDocs=" + Misc::toString(docWriter->getMaxBufferedDocs())) +
          " maxBuffereDeleteTerms=" + Misc::toString(docWriter->getMaxBufferedDeleteTerms()) +
          " maxFieldLength=" + Misc::toString(maxFieldLength) +
          " index=" + segString());
}

std::ostream* IndexWriter::getInfoStream() {
  ensureOpen();
  return infoStream;
}

void IndexWriter::setWriteLockTimeout(int64_t writeLockTimeout) {
  ensureOpen();
  this->writeLockTimeout = writeLockTimeout;
}

int64_t IndexWriter::getWriteLockTimeout() {
  ensureOpen();
  return writeLockTimeout;
}

void IndexWriter::setDefaultWriteLockTimeout(int64_t writeLockTimeout) {
  IndexWriter::WRITE_LOCK_TIMEOUT = writeLockTimeout;
}

int64_t IndexWriter::getDefaultWriteLockTimeout() {
  return IndexWriter::WRITE_LOCK_TIMEOUT;
}

void IndexWriter::close(bool waitForMerges) {
  bool doClose;

  // If any methods have hit OutOfMemoryError, then abort
  // on close, in case the internal state of IndexWriter
  // or DocumentsWriter is corrupt
  if (hitOOM)
    abort();

  { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
    // Ensure that only one thread actually gets to do the closing:
    if (!closing) {
      doClose = true;
      closing = true;
    } else
      doClose = false;
  }
  if (doClose)
    closeInternal(waitForMerges);
  else
    // Another thread beat us to it (is actually doing the
    // close), so we will block until that other thread
    // has finished closing
    waitForClose();
}

void IndexWriter::waitForClose() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  while(!closed && closing) {
    CONDITION_WAIT(THIS_LOCK, THIS_WAIT_CONDITION)
  }
}

void IndexWriter::closeInternal(bool waitForMerges) {
  try {
    if (infoStream != NULL)
      message(string("now flush at close"));

    docWriter->close();

    // Only allow a _CLNEW merge to be triggered if we are
    // going to wait for merges:
    flush(waitForMerges, true);

    if (waitForMerges)
      // Give merge scheduler last chance to run, in case
      // any pending merges are waiting:
      mergeScheduler->merge(this);

    mergePolicy->close();

    finishMerges(waitForMerges);

    mergeScheduler->close();

    { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
      if (commitPending) {
        bool success = false;
        try {
          segmentInfos->write(directory);         // now commit changes
          success = true;
        } _CLFINALLY (
          if (!success) {
            if (infoStream != NULL)
              message(string("hit exception committing segments file during close"));
            deletePartialSegmentsFile();
          }
        )
        if (infoStream != NULL)
          message("close: wrote segments file \"" + segmentInfos->getCurrentSegmentFileName() + "\"");

        deleter->checkpoint(segmentInfos, true);

        commitPending = false;
//        _CLDELETE(rollbackSegmentInfos);
      }
    _CLDELETE(rollbackSegmentInfos);


      if (infoStream != NULL)
        message("at close: " + segString());

      _CLDELETE(docWriter);
      deleter->close();
    }

    if (closeDir)
      directory->close();

    if (writeLock != NULL) {
      writeLock->release();                          // release write lock
      _CLDELETE(writeLock);
    }
    closed = true;
  } catch (std::bad_alloc&) {
    hitOOM = true;
    _CLTHROWA(CL_ERR_OutOfMemory,"Out of memory");
  } _CLFINALLY (
    { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
      if (!closed) {
        closing = false;
        if (infoStream != NULL)
          message(string("hit exception while closing"));
      }
      CONDITION_NOTIFYALL(THIS_WAIT_CONDITION)
    }
  )
}

bool IndexWriter::flushDocStores() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  const std::vector<std::string>& files = docWriter->files();

  bool useCompoundDocStore = false;

  if (files.size() > 0) {
    string docStoreSegment;

    bool success = false;
    try {
      docStoreSegment = docWriter->closeDocStore();
      success = true;
    } _CLFINALLY (
      if (!success) {
        if (infoStream != NULL)
          message(string("hit exception closing doc store segment"));
        docWriter->abort(NULL);
      }
    )

    useCompoundDocStore = mergePolicy->useCompoundDocStore(segmentInfos);

    if (useCompoundDocStore && !docStoreSegment.empty()) {
      // Now build compound doc store file

      success = false;

      const int32_t numSegments = segmentInfos->size();
      const string compoundFileName = docStoreSegment + "." + IndexFileNames::COMPOUND_FILE_STORE_EXTENSION;

      try {
        CompoundFileWriter cfsWriter(directory, compoundFileName.c_str());
        const size_t size = files.size();
        for(size_t i=0;i<size;++i)
          cfsWriter.addFile(files[i].c_str());

        // Perform the merge
        cfsWriter.close();

        for(int32_t i=0;i<numSegments;i++) {
          SegmentInfo* si = segmentInfos->info(i);
          if (si->getDocStoreOffset() != -1 &&
              si->getDocStoreSegment().compare(docStoreSegment)==0)
            si->setDocStoreIsCompoundFile(true);
        }
        checkpoint();
        success = true;
      } _CLFINALLY (
        if (!success) {

          if (infoStream != NULL)
            message("hit exception building compound file doc store for segment " + docStoreSegment);

          // Rollback to no compound file
          for(int32_t i=0;i<numSegments;i++) {
            SegmentInfo* si = segmentInfos->info(i);
            if (si->getDocStoreOffset() != -1 &&
                si->getDocStoreSegment().compare(docStoreSegment)==0 )
              si->setDocStoreIsCompoundFile(false);
          }
          deleter->deleteFile(compoundFileName.c_str());
          deletePartialSegmentsFile();
        }
      )

      deleter->checkpoint(segmentInfos, false);
    }
  }

  return useCompoundDocStore;
}

Directory* IndexWriter::getDirectory() {
  ensureOpen();
  return directory;
}

Analyzer* IndexWriter::getAnalyzer() {
  ensureOpen();
  return analyzer;
}

int32_t IndexWriter::docCount() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  ensureOpen();
  int32_t count = docWriter->getNumDocsInRAM();
  for (int32_t i = 0; i < segmentInfos->size(); i++) {
    SegmentInfo* si = segmentInfos->info(i);
    count += si->docCount;
  }
  return count;
}


void IndexWriter::addDocument(Document* doc, Analyzer* analyzer) {
  if ( analyzer == NULL ) analyzer = this->analyzer;
  ensureOpen();
  bool doFlush = false;
  bool success = false;
  try {
    try {
      doFlush = docWriter->addDocument(doc, analyzer);
      success = true;
    } _CLFINALLY (
      if (!success) {

        if (infoStream != NULL)
          message(string("hit exception adding document"));

        { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
          // If docWriter has some aborted files that were
          // never incref'd, then we clean them up here
          if (docWriter != NULL) {
            const std::vector<std::string>* files = docWriter->abortedFiles();
            if (files != NULL )
              deleter->deleteNewFiles(*files);
          }
        }
      }
    )
    if (doFlush)
      flush(true, false);
  } catch (std::bad_alloc&) {
    hitOOM = true;
    _CLTHROWA(CL_ERR_OutOfMemory,"Out of memory");
  }
}

void IndexWriter::deleteDocuments(Term* term) {
  ensureOpen();
  try {
    bool doFlush = docWriter->bufferDeleteTerm(term);
    if (doFlush)
      flush(true, false);
  } catch (std::bad_alloc&) {
    hitOOM = true;
    _CLTHROWA(CL_ERR_OutOfMemory,"Out of memory");
  }
}

void IndexWriter::deleteDocuments(const ArrayBase<Term*>* terms) {
  ensureOpen();
  try {
    bool doFlush = docWriter->bufferDeleteTerms(terms);
    if (doFlush)
      flush(true, false);
  } catch (std::bad_alloc&) {
    hitOOM = true;
    _CLTHROWA(CL_ERR_OutOfMemory,"Out of memory");
  }
}

void IndexWriter::updateDocument(Term* term, Document* doc) {
  ensureOpen();
  updateDocument(term, doc, getAnalyzer());
}

void IndexWriter::updateDocument(Term* term, Document* doc, Analyzer* analyzer)
{
  ensureOpen();
  try {
    bool doFlush = false;
    bool success = false;
    try {
      doFlush = docWriter->updateDocument(term, doc, analyzer);
      success = true;
    } _CLFINALLY (
      if (!success) {

        if (infoStream != NULL)
          message(string("hit exception updating document"));

        { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
          // If docWriter has some aborted files that were
          // never incref'd, then we clean them up here
          const std::vector<std::string>* files = docWriter->abortedFiles();
          if (files != NULL)
            deleter->deleteNewFiles(*files);
        }
      }
    )
    if (doFlush)
      flush(true, false);
  } catch (std::bad_alloc&) {
    hitOOM = true;
    _CLTHROWA(CL_ERR_OutOfMemory,"Out of memory");
  }
}

// for test purpose
int32_t IndexWriter::getSegmentCount(){
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  return segmentInfos->size();
}

// for test purpose
int32_t IndexWriter::getNumBufferedDocuments(){
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  return docWriter->getNumDocsInRAM();
}

// for test purpose
int32_t IndexWriter::getDocCount(int32_t i) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  if (i >= 0 && i < segmentInfos->size()) {
    return segmentInfos->info(i)->docCount;
  } else {
    return -1;
  }
}

string IndexWriter::newSegmentName() {
  // Cannot synchronize on IndexWriter because that causes
  // deadlock
  { SCOPED_LOCK_MUTEX(segmentInfos->THIS_LOCK)
    // Important to set commitPending so that the
    // segmentInfos is written on close.  Otherwise we
    // could close, re-open and re-return the same segment
    // name that was previously returned which can cause
    // problems at least with ConcurrentMergeScheduler.
    commitPending = true;

    char buf[10];
    Misc::longToBase(segmentInfos->counter++, 36,buf);
    return string("_") + buf;
  }
}

void IndexWriter::optimize(bool doWait) {
  optimize(1, doWait);
}

void IndexWriter::optimize(int32_t maxNumSegments, bool doWait) {
  ensureOpen();

  if (maxNumSegments < 1)
    _CLTHROWA(CL_ERR_IllegalArgument, "maxNumSegments must be >= 1; got " + maxNumSegments);

  if (infoStream != NULL)
    message("optimize: index now " + segString());

  flush();

  { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
    resetMergeExceptions();
    segmentsToOptimize->clear();
    const int32_t numSegments = segmentInfos->size();
    for(int32_t i=0;i<numSegments;i++)
      segmentsToOptimize->push_back(segmentInfos->info(i));

    // Now mark all pending & running merges as optimize
    // merge:
    PendingMergesType::iterator it = pendingMerges->begin();
    while(it != pendingMerges->end()) {
      MergePolicy::OneMerge* _merge = *it;
      _merge->optimize = true;
      _merge->maxNumSegmentsOptimize = maxNumSegments;

      it++;
    }

    RunningMergesType::iterator it2 = runningMerges->begin();
    while(it2 != runningMerges->end()) {
      MergePolicy::OneMerge* _merge = *it2;
      _merge->optimize = true;
      _merge->maxNumSegmentsOptimize = maxNumSegments;

      it2++;
    }
  }

  maybeMerge(maxNumSegments, true);

  if (doWait) {
    { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
      while(optimizeMergesPending()) {
        CONDITION_WAIT(THIS_LOCK, THIS_WAIT_CONDITION);

        if (mergeExceptions->size() > 0) {
          // Forward any exceptions in background merge
          // threads to the current thread:
          const int32_t size = mergeExceptions->size();
          for(int32_t i=0;i<size;i++) {
            MergePolicy::OneMerge* _merge = (*mergeExceptions)[0];
            if (_merge->optimize) {
              CLuceneError tmp(_merge->getException());
              CLuceneError err(tmp.number(),
                (string("background merge hit exception: ") + _merge->segString(directory) +  ":"  + tmp.what() ).c_str(), false );
              throw err;
            }
          }
        }
      }
    }
  }

  // NOTE: in the ConcurrentMergeScheduler case, when
  // doWait is false, we can return immediately while
  // background threads accomplish the optimization
}

bool IndexWriter::optimizeMergesPending() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  for(PendingMergesType::iterator it = pendingMerges->begin();
      it != pendingMerges->end(); it++){
    if ((*it)->optimize)
      return true;

    it++;
  }

  for(RunningMergesType::iterator it = runningMerges->begin();
      it != runningMerges->end(); it++){
    if ((*it)->optimize)
      return true;

    it++;
  }

  return false;
}

void IndexWriter::maybeMerge() {
  maybeMerge(false);
}

void IndexWriter::maybeMerge(bool optimize) {
  maybeMerge(1, optimize);
}

void IndexWriter::maybeMerge(int32_t maxNumSegmentsOptimize, bool optimize) {
  updatePendingMerges(maxNumSegmentsOptimize, optimize);
  mergeScheduler->merge(this);
}

void IndexWriter::updatePendingMerges(int32_t maxNumSegmentsOptimize, bool optimize){
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  assert (!optimize || maxNumSegmentsOptimize > 0);

  if (stopMerges)
    return;

  MergePolicy::MergeSpecification* spec;
  if (optimize) {
    spec = mergePolicy->findMergesForOptimize(segmentInfos, this, maxNumSegmentsOptimize, *segmentsToOptimize);

    if (spec != NULL) {
      const int32_t numMerges = spec->merges->size();
      for(int32_t i=0;i<numMerges;i++) {
        MergePolicy::OneMerge* _merge = (*spec->merges)[i];
        _merge->optimize = true;
        _merge->maxNumSegmentsOptimize = maxNumSegmentsOptimize;
      }
    }

  } else
    spec = mergePolicy->findMerges(segmentInfos, this);

  if (spec != NULL) {
    const int32_t numMerges = spec->merges->size();
    for(int32_t i=0;i<numMerges;i++)
      registerMerge((*spec->merges)[i]);
  }
  _CLDELETE(spec);
}

MergePolicy::OneMerge* IndexWriter::getNextMerge() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  if (pendingMerges->size() == 0)
    return NULL;
  else {
    // Advance the merge from pending to running
    MergePolicy::OneMerge* _merge = *pendingMerges->begin();
    pendingMerges->pop_front();
    runningMerges->insert(_merge);
    return _merge;
  }
}


void IndexWriter::startTransaction() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  if (infoStream != NULL)
    message(string("now start transaction"));

  CND_PRECONDITION(docWriter->getNumBufferedDeleteTerms() == 0,
         "calling startTransaction with buffered delete terms not supported");
  CND_PRECONDITION (docWriter->getNumDocsInRAM() == 0,
         "calling startTransaction with buffered documents not supported");

  localRollbackSegmentInfos = segmentInfos->clone();
  localAutoCommit = autoCommit;

  if (localAutoCommit) {

    if (infoStream != NULL)
      message(string("flush at startTransaction"));

    flush();
    // Turn off auto-commit during our local transaction:
    autoCommit = false;
  } else
    // We must "protect" our files at this point from
    // deletion in case we need to rollback:
    deleter->incRef(segmentInfos, false);
}

void IndexWriter::rollbackTransaction() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  if (infoStream != NULL)
    message(string("now rollback transaction"));

  // First restore autoCommit in case we hit an exception below:
  autoCommit = localAutoCommit;

  // Keep the same segmentInfos instance but replace all
  // of its SegmentInfo instances.  This is so the next
  // attempt to commit using this instance of IndexWriter
  // will always write to a _CLNEW generation ("write once").
  segmentInfos->clear();
  segmentInfos->insert(localRollbackSegmentInfos, true);
  _CLDELETE(localRollbackSegmentInfos);

  // Ask deleter to locate unreferenced files we had
  // created & remove them:
  deleter->checkpoint(segmentInfos, false);

  if (!autoCommit)
    // Remove the incRef we did in startTransaction:
    deleter->decRef(segmentInfos);

  deleter->refresh();
  finishMerges(false);
  stopMerges = false;
}

void IndexWriter::commitTransaction() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  if (infoStream != NULL)
    message(string("now commit transaction"));

  // First restore autoCommit in case we hit an exception below:
  autoCommit = localAutoCommit;

  bool success = false;
  try {
    checkpoint();
    success = true;
  } _CLFINALLY (
    if (!success) {
      if (infoStream != NULL)
        message(string("hit exception committing transaction"));

      rollbackTransaction();
    }
  )

  if (!autoCommit)
    // Remove the incRef we did in startTransaction.
    deleter->decRef(localRollbackSegmentInfos);

  _CLDELETE(localRollbackSegmentInfos);

  // Give deleter a chance to remove files now:
  deleter->checkpoint(segmentInfos, autoCommit);
}

void IndexWriter::abort() {
  ensureOpen();
  if (autoCommit)
    _CLTHROWA(CL_ERR_IllegalState,"abort() can only be called when IndexWriter was opened with autoCommit=false");

  bool doClose;
  { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
    // Ensure that only one thread actually gets to do the closing:
    if (!closing) {
      doClose = true;
      closing = true;
    } else
      doClose = false;
  }

  if (doClose) {

    finishMerges(false);

    // Must pre-close these two, in case they set
    // commitPending=true, so that we can then set it to
    // false before calling closeInternal
    mergePolicy->close();
    mergeScheduler->close();

    { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
      // Keep the same segmentInfos instance but replace all
      // of its SegmentInfo instances.  This is so the next
      // attempt to commit using this instance of IndexWriter
      // will always write to a _CLNEW generation ("write
      // once").
      segmentInfos->clear();
      segmentInfos->insert(rollbackSegmentInfos, false);

      docWriter->abort(NULL);

      // Ask deleter to locate unreferenced files & remove
      // them:
      deleter->checkpoint(segmentInfos, false);
      deleter->refresh();
    }

    commitPending = false;
    closeInternal(false);
  } else
    waitForClose();
}

void IndexWriter::finishMerges(bool waitForMerges) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  if (!waitForMerges) {

    stopMerges = true;

    // Abort all pending & running merges:
    for(PendingMergesType::iterator it = pendingMerges->begin();
        it != pendingMerges->end(); it++ ){
      MergePolicy::OneMerge* _merge = *it;
      if (infoStream != NULL)
        message("now abort pending merge " + _merge->segString(directory));
      _merge->abort();
      mergeFinish(_merge);

      it++;
    }
    pendingMerges->clear();

    for(RunningMergesType::iterator it = runningMerges->begin();
        it != runningMerges->end(); it++ ){
      MergePolicy::OneMerge* _merge = *it;
      if (infoStream != NULL)
        message("now abort running merge " + _merge->segString(directory));
      _merge->abort();

      it++;
    }

    // These merges periodically check whether they have
    // been aborted, and stop if so.  We wait here to make
    // sure they all stop.  It should not take very int64_t
    // because the merge threads periodically check if
    // they are aborted.
    while(runningMerges->size() > 0) {
      if (infoStream != NULL)
        message( string("now wait for ") + Misc::toString( (int32_t)runningMerges->size()) + " running merge to abort");
      CONDITION_WAIT(THIS_LOCK, THIS_WAIT_CONDITION)
    }

    assert (0 == mergingSegments->size());

    if (infoStream != NULL)
      message(string("all running merges have aborted"));

  } else {
    while(pendingMerges->size() > 0 || runningMerges->size() > 0) {
      CONDITION_WAIT(THIS_LOCK, THIS_WAIT_CONDITION)
    }
    assert (0 == mergingSegments->size());
  }
}

void IndexWriter::checkpoint() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  if (autoCommit) {
    segmentInfos->write(directory);
    commitPending = false;
    if (infoStream != NULL)
      message("checkpoint: wrote segments file \"" + segmentInfos->getCurrentSegmentFileName() + "\"");
  } else {
    commitPending = true;
  }
}

void IndexWriter::addIndexes(CL_NS(util)::ArrayBase<CL_NS(store)::Directory*>& dirs){

  ensureOpen();

  // Do not allow add docs or deletes while we are running:
  docWriter->pauseAllThreads();

  try {

    if (infoStream != NULL)
      message(string("flush at addIndexes"));
    flush();

    bool success = false;

    startTransaction();

    try {

      { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
        for (int32_t i = 0; i< dirs.length; i++) {
          SegmentInfos sis;	  // read infos from dir
          sis.read(dirs[i]);
          segmentInfos->insert(&sis,true);	  // add each info
        }
      }

      optimize();

      success = true;
    } _CLFINALLY (
      if (success) {
        commitTransaction();
      } else {
        rollbackTransaction();
      }
    )
  } catch (std::bad_alloc&) {
    hitOOM = true;
    _CLTHROWA(CL_ERR_OutOfMemory,"Out of memory");
  } _CLFINALLY (
    docWriter->resumeAllThreads();
  )
}

void IndexWriter::resetMergeExceptions() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  mergeExceptions->clear();
  mergeGen++;
}

void IndexWriter::addIndexesNoOptimize(CL_NS(util)::ArrayBase<CL_NS(store)::Directory*>& dirs)
{
  ensureOpen();

  // Do not allow add docs or deletes while we are running:
  docWriter->pauseAllThreads();

  try {
    if (infoStream != NULL)
      message(string("flush at addIndexesNoOptimize"));
    flush();

    bool success = false;

    startTransaction();

    try {

      { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
        for (int32_t i = 0; i< dirs.length; i++) {
          if (directory == dirs[i]) {
            // cannot add this index: segments may be deleted in merge before added
            _CLTHROWA(CL_ERR_IllegalArgument,"Cannot add this index to itself");
          }

          SegmentInfos sis; // read infos from dir
          sis.read(dirs[i]);
          segmentInfos->insert(&sis, true);
        }
      }

      maybeMerge();

      // If after merging there remain segments in the index
      // that are in a different directory, just copy these
      // over into our index.  This is necessary (before
      // finishing the transaction) to avoid leaving the
      // index in an unusable (inconsistent) state.
      copyExternalSegments();

      success = true;

    } _CLFINALLY (
      if (success) {
        commitTransaction();
      } else {
        rollbackTransaction();
      }
    )
  } catch (std::bad_alloc&) {
    hitOOM = true;
    _CLTHROWA(CL_ERR_OutOfMemory,"Out of memory");
  } _CLFINALLY (
    docWriter->resumeAllThreads();
  )
}

void IndexWriter::copyExternalSegments() {

  bool any = false;

  while(true) {
    SegmentInfo* info = NULL;
    MergePolicy::OneMerge* _merge = NULL;
    { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
      const int32_t numSegments = segmentInfos->size();
      for(int32_t i=0;i<numSegments;i++) {
        info = segmentInfos->info(i);
        if (info->dir != directory) {
          SegmentInfos* range = _CLNEW SegmentInfos;
          segmentInfos->range(i, 1+i, *range);
          _merge = _CLNEW MergePolicy::OneMerge(range, info->getUseCompoundFile());
          break;
        }
      }
    }

    if (_merge != NULL) {
      if (registerMerge(_merge)) {
        PendingMergesType::iterator p = std::find(pendingMerges->begin(),pendingMerges->end(), _merge);
        pendingMerges->remove(p,true);
        runningMerges->insert(_merge);
        any = true;
        merge(_merge);
      } else
        // This means there is a bug in the
        // MergeScheduler.  MergeSchedulers in general are
        // not allowed to run a merge involving segments
        // external to this IndexWriter's directory in the
        // background because this would put the index
        // into an inconsistent state (where segmentInfos
        // has been written with such external segments
        // that an IndexReader would fail to load).
        _CLTHROWA(CL_ERR_Merge, (string("segment \"") + info->name + " exists in external directory yet the MergeScheduler executed the merge in a separate thread").c_str() );
    } else
      // No more external segments
      break;
  }

  if (any)
    // Sometimes, on copying an external segment over,
    // more merges may become necessary:
    mergeScheduler->merge(this);
}

void IndexWriter::doAfterFlush(){
}

void IndexWriter::flush() {
  flush(true, false);
}

void IndexWriter::flush(bool triggerMerge, bool _flushDocStores) {
  ensureOpen();

  if (doFlush(_flushDocStores) && triggerMerge)
    maybeMerge();
}

bool IndexWriter::doFlush(bool _flushDocStores) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  // Make sure no threads are actively adding a document

  // Returns true if docWriter is currently aborting, in
  // which case we skip flushing this segment
  if (docWriter->pauseAllThreads()) {
    docWriter->resumeAllThreads();
    return false;
  }

  bool ret = false;
  try {

    SegmentInfo* newSegment = NULL;

    const int32_t numDocs = docWriter->getNumDocsInRAM();

    // Always flush docs if there are any
    bool flushDocs = numDocs > 0;

    // With autoCommit=true we always must flush the doc
    // stores when we flush
    _flushDocStores |= autoCommit;
    string docStoreSegment = docWriter->getDocStoreSegment();
    if (docStoreSegment.empty())
      _flushDocStores = false;

    // Always flush deletes if there are any delete terms.
    // TODO: when autoCommit=false we don't have to flush
    // deletes with every flushed segment; we can save
    // CPU/IO by buffering longer & flushing deletes only
    // when they are full or writer is being closed.  We
    // have to fix the "applyDeletesSelectively" logic to
    // apply to more than just the last flushed segment
    bool flushDeletes = docWriter->hasDeletes();

    if (infoStream != NULL) {
      message("  flush: segment=" + docWriter->getSegment() +
              " docStoreSegment=" + docWriter->getDocStoreSegment() +
              " docStoreOffset=" + Misc::toString(docWriter->getDocStoreOffset()) +
              " flushDocs=" + Misc::toString(flushDocs) +
              " flushDeletes=" + Misc::toString(flushDeletes) +
              " flushDocStores=" + Misc::toString(_flushDocStores) +
              " numDocs=" + Misc::toString(numDocs) +
              " numBufDelTerms=" + Misc::toString(docWriter->getNumBufferedDeleteTerms()) );
      message("  index before flush " + segString());
    }

    int32_t docStoreOffset = docWriter->getDocStoreOffset();

    // docStoreOffset should only be non-zero when
    // autoCommit == false
    assert (!autoCommit || 0 == docStoreOffset);

    bool docStoreIsCompoundFile = false;

    // Check if the doc stores must be separately flushed
    // because other segments, besides the one we are about
    // to flush, reference it
    if (_flushDocStores && (!flushDocs || !docWriter->getSegment().compare(docWriter->getDocStoreSegment())==0 )) {
      // We must separately flush the doc store
      if (infoStream != NULL)
        message("  flush shared docStore segment " + docStoreSegment);

      docStoreIsCompoundFile = flushDocStores();
      _flushDocStores = false;
    }

    string segment = docWriter->getSegment();

    // If we are flushing docs, segment must not be NULL:
    assert (!segment.empty() || !flushDocs);

    if (flushDocs || flushDeletes) {

      SegmentInfos* rollback = NULL;

      if (flushDeletes)
        rollback = segmentInfos->clone();

      bool success = false;

      try {
        if (flushDocs) {

          if (0 == docStoreOffset && _flushDocStores) {
            // This means we are flushing doc stores
            // with this segment, so it will not be shared
            // with other segments
            assert (!docStoreSegment.empty());
            assert (docStoreSegment.compare(segment)==0);
            docStoreOffset = -1;
            docStoreIsCompoundFile = false;
            docStoreSegment.clear();
          }

          int32_t flushedDocCount = docWriter->flush(_flushDocStores);

          newSegment = _CLNEW SegmentInfo(segment.c_str(),
                                       flushedDocCount,
                                       directory, false, true,
                                       docStoreOffset, docStoreSegment.c_str(),
                                       docStoreIsCompoundFile);
          segmentInfos->insert(newSegment);
        }

        if (flushDeletes)
          // we should be able to change this so we can
          // buffer deletes longer and then flush them to
          // multiple flushed segments, when
          // autoCommit=false
          applyDeletes(flushDocs);

        doAfterFlush();

        checkpoint();
        success = true;
      } _CLFINALLY (
        if (!success) {

          if (infoStream != NULL)
            message("hit exception flushing segment " + segment);

          if (flushDeletes) {

            // Carefully check if any partial .del files
            // should be removed:
            const int32_t size = rollback->size();
            for(int32_t i=0;i<size;i++) {
              const string newDelFileName = segmentInfos->info(i)->getDelFileName();
              const string delFileName = rollback->info(i)->getDelFileName();
              if ( !newDelFileName.empty() && newDelFileName.compare(delFileName)!=0 )
                deleter->deleteFile(newDelFileName.c_str());
            }

            // Fully replace the segmentInfos since flushed
            // deletes could have changed any of the
            // SegmentInfo instances:
            segmentInfos->clear();
            assert(false);//test me..
            segmentInfos->insert(rollback, false);

          } else {
            // Remove segment we added, if any:
            if ( newSegment != NULL &&
                segmentInfos->size() > 0 &&
                segmentInfos->info(segmentInfos->size()-1) == newSegment)
              segmentInfos->remove(segmentInfos->size()-1);
          }
          if (flushDocs)
            docWriter->abort(NULL);
          deletePartialSegmentsFile();
          deleter->checkpoint(segmentInfos, false);

          if (!segment.empty())
            deleter->refresh(segment.c_str());
        } else if (flushDeletes)
            _CLDELETE(rollback);
      )

      deleter->checkpoint(segmentInfos, autoCommit);

      if (flushDocs && mergePolicy->useCompoundFile(segmentInfos,
                                                   newSegment)) {
        success = false;
        try {
          docWriter->createCompoundFile(segment);
          newSegment->setUseCompoundFile(true);
          checkpoint();
          success = true;
        } _CLFINALLY (
          if (!success) {
            if (infoStream != NULL)
              message("hit exception creating compound file for newly flushed segment " + segment);
            newSegment->setUseCompoundFile(false);
            deleter->deleteFile( (segment + "." + IndexFileNames::COMPOUND_FILE_EXTENSION).c_str() );
            deletePartialSegmentsFile();
          }
        )

        deleter->checkpoint(segmentInfos, autoCommit);
      }

      ret = true;
    }

  } catch (std::bad_alloc&) {
    hitOOM = true;
    _CLTHROWA(CL_ERR_OutOfMemory,"Out of memory");
  } _CLFINALLY (
    docWriter->clearFlushPending();
    docWriter->resumeAllThreads();
  )
  return ret;
}

int64_t IndexWriter::ramSizeInBytes() {
  ensureOpen();
  return docWriter->getRAMUsed();
}

int32_t IndexWriter::numRamDocs() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  ensureOpen();
  return docWriter->getNumDocsInRAM();
}

int32_t IndexWriter::ensureContiguousMerge(MergePolicy::OneMerge* _merge) {

  int32_t first = segmentInfos->indexOf(_merge->segments->info(0));
  if (first == -1)
    _CLTHROWA(CL_ERR_Merge, (string("could not find segment ") + _merge->segments->info(0)->name + " in current segments").c_str());

  const int32_t numSegments = segmentInfos->size();

  const int32_t numSegmentsToMerge = _merge->segments->size();
  for(int32_t i=0;i<numSegmentsToMerge;i++) {
    const SegmentInfo* info = _merge->segments->info(i);

    if (first + i >= numSegments || !segmentInfos->info(first+i)->equals(info) ) {
      if (segmentInfos->indexOf(info) == -1)
        _CLTHROWA(CL_ERR_Merge, (string("MergePolicy selected a segment (") + info->name + ") that is not in the index").c_str());
      else
        _CLTHROWA(CL_ERR_Merge, (string("MergePolicy selected non-contiguous segments to merge (") + _merge->getObjectName() + " vs " + segString() + "), which IndexWriter (currently) cannot handle").c_str() );
    }
  }

  return first;
}

bool IndexWriter::commitMerge(MergePolicy::OneMerge* _merge) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  assert (_merge->registerDone);

  if (hitOOM)
    return false;

  if (infoStream != NULL)
    message("commitMerge: " + _merge->segString(directory));

  // If merge was explicitly aborted, or, if abort() or
  // rollbackTransaction() had been called since our merge
  // started (which results in an unqualified
  // deleter->refresh() call that will remove any index
  // file that current segments does not reference), we
  // abort this merge
  if (_merge->isAborted()) {
    if (infoStream != NULL)
      message("commitMerge: skipping merge " + _merge->segString(directory) + ": it was aborted");

    assert (_merge->increfDone);
    decrefMergeSegments(_merge);
    deleter->refresh(_merge->info->name.c_str());
    return false;
  }

  bool success = false;

  int32_t start;

  try {
    SegmentInfos* sourceSegmentsClone = _merge->segmentsClone;
    const SegmentInfos* sourceSegments = _merge->segments;

    start = ensureContiguousMerge(_merge);
    if (infoStream != NULL)
      message("commitMerge " + _merge->segString(directory));

    // Carefully merge deletes that occurred after we
    // started merging:

    BitVector* deletes = NULL;
    int32_t docUpto = 0;

    const int32_t numSegmentsToMerge = sourceSegments->size();
    for(int32_t i=0;i<numSegmentsToMerge;i++) {
      const SegmentInfo* previousInfo = sourceSegmentsClone->info(i);
      const SegmentInfo* currentInfo = sourceSegments->info(i);

      assert (currentInfo->docCount == previousInfo->docCount);

      const int32_t docCount = currentInfo->docCount;

      if (previousInfo->hasDeletions()) {

        // There were deletes on this segment when the merge
        // started.  The merge has collapsed away those
        // deletes, but, if _CLNEW deletes were flushed since
        // the merge started, we must now carefully keep any
        // newly flushed deletes but mapping them to the _CLNEW
        // docIDs.

        assert (currentInfo->hasDeletions());

        // Load deletes present @ start of merge, for this segment:
        BitVector previousDeletes(previousInfo->dir, previousInfo->getDelFileName().c_str());

        if (!currentInfo->getDelFileName().compare(previousInfo->getDelFileName())==0 ){
          // This means this segment has had new deletes
          // committed since we started the merge, so we
          // must merge them:
          if (deletes == NULL)
            deletes = _CLNEW BitVector(_merge->info->docCount);

          BitVector currentDeletes(currentInfo->dir, currentInfo->getDelFileName().c_str());
          for(int32_t j=0;j<docCount;j++) {
            if (previousDeletes.get(j))
              assert (currentDeletes.get(j));
            else {
              if (currentDeletes.get(j))
                deletes->set(docUpto);
              docUpto++;
            }
          }
        } else
          docUpto += docCount - previousDeletes.count();

      } else if (currentInfo->hasDeletions()) {
        // This segment had no deletes before but now it
        // does:
        if (deletes == NULL)
          deletes = _CLNEW BitVector(_merge->info->docCount);
        BitVector currentDeletes(directory, currentInfo->getDelFileName().c_str());

        for(int32_t j=0;j<docCount;j++) {
          if (currentDeletes.get(j))
            deletes->set(docUpto);
          docUpto++;
        }

      } else
        // No deletes before or after
        docUpto += currentInfo->docCount;

      _merge->checkAborted(directory);
    }

    if (deletes != NULL) {
      _merge->info->advanceDelGen();
      deletes->write(directory, _merge->info->getDelFileName().c_str() );
      _CLDELETE(deletes);
    }
    success = true;
  } _CLFINALLY (
    if (!success) {
      if (infoStream != NULL)
        message(string("hit exception creating merged deletes file"));
      deleter->refresh(_merge->info->name.c_str());
    }
  )

  // Simple optimization: if the doc store we are using
  // has been closed and is in now compound format (but
  // wasn't when we started), then we will switch to the
  // compound format as well:
  const string mergeDocStoreSegment = _merge->info->getDocStoreSegment();
  if ( !mergeDocStoreSegment.empty() && !_merge->info->getDocStoreIsCompoundFile()) {
    const int32_t size = segmentInfos->size();
    for(int32_t i=0;i<size;i++) {
      const SegmentInfo* info = segmentInfos->info(i);
      const string docStoreSegment = info->getDocStoreSegment();
      if ( !docStoreSegment.empty() &&
          docStoreSegment.compare(mergeDocStoreSegment)==0 &&
          info->getDocStoreIsCompoundFile()) {
        _merge->info->setDocStoreIsCompoundFile(true);
        break;
      }
    }
  }

  success = false;
  SegmentInfos* rollback = NULL;
  try {
    rollback = segmentInfos->clone();
    int32_t segmentssize = _merge->segments->size();
    for ( int32_t i=0;i<segmentssize;i++ ){
      segmentInfos->remove(start);
    }
    segmentInfos->add(_merge->info,start);
    checkpoint();
    success = true;
  } _CLFINALLY (
    if (!success && rollback != NULL) {
      if (infoStream != NULL)
        message(string("hit exception when checkpointing after merge"));
      segmentInfos->clear();
      segmentInfos->insert(rollback,true);
      deletePartialSegmentsFile();
      deleter->refresh(_merge->info->name.c_str());
    }
    _CLDELETE(rollback);
  )

  if (_merge->optimize)
    segmentsToOptimize->push_back(_merge->info);

  // Must checkpoint before decrefing so any newly
  // referenced files in the _CLNEW merge->info are incref'd
  // first:
  deleter->checkpoint(segmentInfos, autoCommit);

  decrefMergeSegments(_merge);

  return true;
}


void IndexWriter::decrefMergeSegments(MergePolicy::OneMerge* _merge) {
  const SegmentInfos* sourceSegmentsClone = _merge->segmentsClone;
  const int32_t numSegmentsToMerge = sourceSegmentsClone->size();
  assert (_merge->increfDone);
  _merge->increfDone = false;
  for(int32_t i=0;i<numSegmentsToMerge;i++) {
    SegmentInfo* previousInfo = sourceSegmentsClone->info(i);
    // Decref all files for this SegmentInfo (this
    // matches the incref in mergeInit):
    if (previousInfo->dir == directory)
      deleter->decRef(previousInfo->files());
  }
}

void IndexWriter::merge(MergePolicy::OneMerge* _merge)
{

  assert (_merge->registerDone);
  assert (!_merge->optimize || _merge->maxNumSegmentsOptimize > 0);

  bool success = false;

  try {
    try {
      try {
        mergeInit(_merge);

        if (infoStream != NULL)
          message("now merge\n  merge=" + _merge->segString(directory) + "\n  index=" + segString());

        mergeMiddle(_merge);
        success = true;
      } catch (CLuceneError& e) {
        if ( e.number() != CL_ERR_MergeAborted ) throw e;
        _merge->setException(e);
        addMergeException(_merge);
        // We can ignore this exception, unless the merge
        // involves segments from external directories, in
        // which case we must throw it so, for example, the
        // rollbackTransaction code in addIndexes* is
        // executed.
        if (_merge->isExternal)
          throw e;
      }
    } _CLFINALLY (
      { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
        try {

          mergeFinish(_merge);

          if (!success) {
            if (infoStream != NULL)
              message(string("hit exception during merge"));
            addMergeException(_merge);
            if (_merge->info != NULL && segmentInfos->indexOf(_merge->info)==-1)
              deleter->refresh(_merge->info->name.c_str());
          }

          // This merge (and, generally, any change to the
          // segments) may now enable new merges, so we call
          // merge policy & update pending merges.
          if (success && !_merge->isAborted() && !closed && !closing)
            updatePendingMerges(_merge->maxNumSegmentsOptimize, _merge->optimize);

        } _CLFINALLY (
          RunningMergesType::iterator itr = runningMerges->find(_merge);
          if ( itr != runningMerges->end() ) runningMerges->remove( itr );
          // Optimize may be waiting on the final optimize
          // merge to finish; and finishMerges() may be
          // waiting for all merges to finish:
          CONDITION_NOTIFYALL(THIS_WAIT_CONDITION)
        )
      }
    )
  } catch (std::bad_alloc&) {
    hitOOM = true;
    _CLTHROWA(CL_ERR_OutOfMemory,"Out of memory");
  }
}

bool IndexWriter::registerMerge(MergePolicy::OneMerge* _merge) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  if (_merge->registerDone)
    return true;

  const int32_t count = _merge->segments->size();
  bool isExternal = false;
  for(int32_t i=0;i<count;i++) {
    SegmentInfo* info = _merge->segments->info(i);
    if (mergingSegments->find(info) != mergingSegments->end())
      return false;
    if (segmentInfos->indexOf(info) == -1)
      return false;
    if (info->dir != directory)
      isExternal = true;
  }

  pendingMerges->push_back(_merge);

  if (infoStream != NULL)
    message( string("add merge to pendingMerges: ") + _merge->segString(directory) + " [total " + Misc::toString((int32_t)pendingMerges->size()) + " pending]");

  _merge->mergeGen = mergeGen;
  _merge->isExternal = isExternal;

  // OK it does not conflict; now record that this merge
  // is running (while synchronized) to avoid race
  // condition where two conflicting merges from different
  // threads, start
  for(int32_t i=0;i<count;i++)
    mergingSegments->insert(mergingSegments->end(),_merge->segments->info(i));

  // Merge is now registered
  _merge->registerDone = true;
  return true;
}

void IndexWriter::mergeInit(MergePolicy::OneMerge* _merge) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  bool success = false;
  try {
    _mergeInit(_merge);
    success = true;
  } _CLFINALLY (
    if (!success) {
      mergeFinish(_merge);
      RunningMergesType::iterator itr = runningMerges->find(_merge);
      if ( itr != runningMerges->end() ) runningMerges->remove(itr);
    }
  )
}

void IndexWriter::_mergeInit(MergePolicy::OneMerge* _merge) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  assert (testPoint("startMergeInit"));

  assert (_merge->registerDone);

  if (_merge->info != NULL)
    // mergeInit already done
    return;

  if (_merge->isAborted())
    return;

  const SegmentInfos* sourceSegments = _merge->segments;
  const int32_t end = sourceSegments->size();

  ensureContiguousMerge(_merge);

  // Check whether this merge will allow us to skip
  // merging the doc stores (stored field & vectors).
  // This is a very substantial optimization (saves tons
  // of IO) that can only be applied with
  // autoCommit=false.

  Directory* lastDir = directory;
  string lastDocStoreSegment;
  int32_t next = -1;

  bool mergeDocStores = false;
  bool doFlushDocStore = false;
  const string currentDocStoreSegment = docWriter->getDocStoreSegment();

  // Test each segment to be merged: check if we need to
  // flush/merge doc stores
  for (int32_t i = 0; i < end; i++) {
    SegmentInfo* si = sourceSegments->info(i);

    // If it has deletions we must merge the doc stores
    if (si->hasDeletions())
      mergeDocStores = true;

    // If it has its own (private) doc stores we must
    // merge the doc stores
    if (-1 == si->getDocStoreOffset())
      mergeDocStores = true;

    // If it has a different doc store segment than
    // previous segments, we must merge the doc stores
    string docStoreSegment = si->getDocStoreSegment();
    if (docStoreSegment.empty())
      mergeDocStores = true;
    else if (lastDocStoreSegment.empty())
      lastDocStoreSegment = docStoreSegment;
    else if (!lastDocStoreSegment.compare(docStoreSegment)==0 )
      mergeDocStores = true;

    // Segments' docScoreOffsets must be in-order,
    // contiguous.  For the default merge policy now
    // this will always be the case but for an arbitrary
    // merge policy this may not be the case
    if (-1 == next)
      next = si->getDocStoreOffset() + si->docCount;
    else if (next != si->getDocStoreOffset())
      mergeDocStores = true;
    else
      next = si->getDocStoreOffset() + si->docCount;

    // If the segment comes from a different directory
    // we must merge
    if (lastDir != si->dir)
      mergeDocStores = true;

    // If the segment is referencing the current "live"
    // doc store outputs then we must merge
    if (si->getDocStoreOffset() != -1 && !currentDocStoreSegment.empty() && si->getDocStoreSegment().compare(currentDocStoreSegment)==0 )
      doFlushDocStore = true;
  }

  int32_t docStoreOffset;
  string docStoreSegment;
  bool docStoreIsCompoundFile;

  if (mergeDocStores) {
    docStoreOffset = -1;
    docStoreSegment.clear();
    docStoreIsCompoundFile = false;
  } else {
    SegmentInfo* si = sourceSegments->info(0);
    docStoreOffset = si->getDocStoreOffset();
    docStoreSegment = si->getDocStoreSegment();
    docStoreIsCompoundFile = si->getDocStoreIsCompoundFile();
  }

  if (mergeDocStores && doFlushDocStore) {
    // SegmentMerger intends to merge the doc stores
    // (stored fields, vectors), and at least one of the
    // segments to be merged refers to the currently
    // live doc stores.

    // TODO: if we know we are about to merge away these
    // newly flushed doc store files then we should not
    // make compound file out of them...
    if (infoStream != NULL)
      message(string("flush at merge"));
    flush(false, true);
  }

  // We must take a full copy at this point so that we can
  // properly merge deletes in commitMerge()
  _merge->segmentsClone = _merge->segments->clone();

  for (int32_t i = 0; i < end; i++) {
    SegmentInfo* si = _merge->segmentsClone->info(i);

    // IncRef all files for this segment info to make sure
    // they are not removed while we are trying to merge->
    if (si->dir == directory)
      deleter->incRef(si->files());
  }

  _merge->increfDone = true;

  _merge->mergeDocStores = mergeDocStores;

  // Bind a _CLNEW segment name here so even with
  // ConcurrentMergePolicy we keep deterministic segment
  // names.
  _merge->info = _CLNEW SegmentInfo(newSegmentName().c_str(), 0,
                               directory, false, true,
                               docStoreOffset,
                               docStoreSegment.c_str(),
                               docStoreIsCompoundFile);
  // Also enroll the merged segment into mergingSegments;
  // this prevents it from getting selected for a merge
  // after our merge is done but while we are building the
  // CFS:
  mergingSegments->insert(_merge->info);
}

void IndexWriter::mergeFinish(MergePolicy::OneMerge* _merge) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  if (_merge->increfDone)
    decrefMergeSegments(_merge);

  assert (_merge->registerDone);

  const SegmentInfos* sourceSegments = _merge->segments;
  const int32_t end = sourceSegments->size();
  for(int32_t i=0;i<end;i++){//todo: use iterator
    MergingSegmentsType::iterator itr = mergingSegments->find(sourceSegments->info(i));
    if ( itr != mergingSegments->end() ) mergingSegments->remove(itr);
  }
  MergingSegmentsType::iterator itr = mergingSegments->find(_merge->info);
  if ( itr != mergingSegments->end() ) mergingSegments->remove(itr);
  _merge->registerDone = false;
}

int32_t IndexWriter::mergeMiddle(MergePolicy::OneMerge* _merge) {

  _merge->checkAborted(directory);

  const string mergedName = _merge->info->name;

  int32_t mergedDocCount = 0;

  const SegmentInfos* sourceSegments = _merge->segments;
  SegmentInfos* sourceSegmentsClone = _merge->segmentsClone;
  const int32_t numSegments = sourceSegments->size();

  if (infoStream != NULL)
    message("merging " + _merge->segString(directory));

  SegmentMerger merger (this, mergedName.c_str(), _merge) ;

  // This is try/finally to make sure merger's readers are
  // closed:

  bool success = false;

  try {
    int32_t totDocCount = 0;

    for (int32_t i = 0; i < numSegments; i++) {
      SegmentInfo* si = sourceSegmentsClone->info(i);
      IndexReader* reader = SegmentReader::get(si, MERGE_READ_BUFFER_SIZE, _merge->mergeDocStores); // no need to set deleter (yet)
      merger.add(reader);
      totDocCount += reader->numDocs();
    }
    if (infoStream != NULL) {
      message(string("merge: total ")+ Misc::toString(totDocCount)+" docs");
    }

    _merge->checkAborted(directory);

    mergedDocCount = _merge->info->docCount = merger.merge(_merge->mergeDocStores);

    assert (mergedDocCount == totDocCount);

    success = true;

  } _CLFINALLY (
    // close readers before we attempt to delete
    // now-obsolete segments
    merger.closeReaders();
    if (!success) {
      if (infoStream != NULL)
        message("hit exception during merge; now refresh deleter on segment " + mergedName);
      { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
        addMergeException(_merge);
        deleter->refresh(mergedName.c_str());
      }
    }
  )

  if (!commitMerge(_merge))
    // commitMerge will return false if this merge was aborted
    return 0;

  if (_merge->useCompoundFile) {

    success = false;
    bool skip = false;
    const string compoundFileName = mergedName + "." + IndexFileNames::COMPOUND_FILE_EXTENSION;

    try {
      try {
        merger.createCompoundFile(compoundFileName.c_str());
        success = true;
      } catch (CLuceneError& ioe) {
        if ( ioe.number() != CL_ERR_IO ) throw ioe;

        { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
          if (segmentInfos->indexOf(_merge->info) == -1) {
            // If another merge kicked in and merged our
            // _CLNEW segment away while we were trying to
            // build the compound file, we can hit a
            // FileNotFoundException and possibly
            // IOException over NFS.  We can tell this has
            // happened because our SegmentInfo is no
            // longer in the segments; if this has
            // happened it is safe to ignore the exception
            // & skip finishing/committing our compound
            // file creating.
            if (infoStream != NULL)
              message("hit exception creating compound file; ignoring it because our info (segment " + _merge->info->name + ") has been merged away");
            skip = true;
          } else
            throw ioe;
        }
      }
    } _CLFINALLY (
      if (!success) {
        if (infoStream != NULL)
          message(string("hit exception creating compound file during merge: skip=") + Misc::toString(skip));

        { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
          if (!skip)
            addMergeException(_merge);
          deleter->deleteFile(compoundFileName.c_str());
        }
      }
    )

    if (!skip) {

      { SCOPED_LOCK_MUTEX(this->THIS_LOCK)
        if (skip || segmentInfos->indexOf(_merge->info) == -1 || _merge->isAborted()) {
          // Our segment (committed in non-compound
          // format) got merged away while we were
          // building the compound format.
          deleter->deleteFile(compoundFileName.c_str());
        } else {
          success = false;
          try {
            _merge->info->setUseCompoundFile(true);
            checkpoint();
            success = true;
          } _CLFINALLY (
            if (!success) {
              if (infoStream != NULL)
                message(string("hit exception checkpointing compound file during merge"));

              // Must rollback:
              addMergeException(_merge);
              _merge->info->setUseCompoundFile(false);
              deletePartialSegmentsFile();
              deleter->deleteFile(compoundFileName.c_str());
            }
          )

          // Give deleter a chance to remove files now.
          deleter->checkpoint(segmentInfos, autoCommit);
        }
      }
    }
  }

  return mergedDocCount;
}

void IndexWriter::addMergeException(MergePolicy::OneMerge* _merge) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  if ( mergeGen == _merge->mergeGen ){
    MergeExceptionsType::iterator itr = mergeExceptions->begin();
    while ( itr != mergeExceptions->end() ){
      MergePolicy::OneMerge* x = *itr;
      if ( x == _merge ){
        return;
      }
    }
  }
  mergeExceptions->push_back(_merge);
}

void IndexWriter::deletePartialSegmentsFile()  {
  if (segmentInfos->getLastGeneration() != segmentInfos->getGeneration()) {
    string segmentFileName = IndexFileNames::fileNameFromGeneration(IndexFileNames::SEGMENTS,
                                                                   "",
                                                                   segmentInfos->getGeneration());
    if (infoStream != NULL)
      message("now delete partial segments file \"" + segmentFileName + "\"");

    deleter->deleteFile(segmentFileName.c_str());
  }
}


void IndexWriter::applyDeletes(bool flushedNewSegment) {
  const DocumentsWriter::TermNumMapType& bufferedDeleteTerms = docWriter->getBufferedDeleteTerms();
  const vector<int32_t>* bufferedDeleteDocIDs = docWriter->getBufferedDeleteDocIDs();

  if (infoStream != NULL)
    message( string("flush ") + Misc::toString(docWriter->getNumBufferedDeleteTerms()) +
          " buffered deleted terms and " + Misc::toString((int32_t)bufferedDeleteDocIDs->size()) +
          " deleted docIDs on " + Misc::toString((int32_t)segmentInfos->size()) + " segments.");

  if (flushedNewSegment) {
    IndexReader* reader = NULL;
    try {
      // Open readers w/o opening the stored fields /
      // vectors because these files may still be held
      // open for writing by docWriter
      reader = SegmentReader::get(segmentInfos->info(segmentInfos->size() - 1), false);

      // Apply delete terms to the segment just flushed from ram
      // apply appropriately so that a delete term is only applied to
      // the documents buffered before it, not those buffered after it.
      _internal->applyDeletesSelectively(bufferedDeleteTerms, *bufferedDeleteDocIDs, reader);
    } _CLFINALLY (
      if (reader != NULL) {
        try {
          reader->doCommit();
        } _CLFINALLY (
          reader->doClose();
          _CLLDELETE(reader);
        )
      }
    )
  }

  int32_t infosEnd = segmentInfos->size();
  if (flushedNewSegment) {
    infosEnd--;
  }

  for (int32_t i = 0; i < infosEnd; i++) {
    IndexReader* reader = NULL;
    try {
      reader = SegmentReader::get(segmentInfos->info(i), false);

      // Apply delete terms to disk segments
      // except the one just flushed from ram.
      _internal->applyDeletes(bufferedDeleteTerms, reader);
    } _CLFINALLY (
      if (reader != NULL) {
        try {
          reader->doCommit();
        } _CLFINALLY (
          reader->doClose();
        )
      }
    )
  }

  // Clean up bufferedDeleteTerms.
  docWriter->clearBufferedDeletes();
}


int32_t IndexWriter::getBufferedDeleteTermsSize() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  return docWriter->getBufferedDeleteTerms().size();
}

int32_t IndexWriter::getNumBufferedDeleteTerms() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  return docWriter->getNumBufferedDeleteTerms();
}

void IndexWriter::Internal::applyDeletesSelectively(const DocumentsWriter::TermNumMapType& deleteTerms,
  const vector<int32_t>& deleteIds, IndexReader* reader)
{
  DocumentsWriter::TermNumMapType::const_iterator iter = deleteTerms.begin();
  while (iter != deleteTerms.end() ) {
    Term* term = iter->first;
    TermDocs* docs = reader->termDocs(term);
    if (docs != NULL) {
      int32_t num = iter->second->getNum();
      try {
        while (docs->next()) {
          int32_t doc = docs->doc();
          if (doc >= num) {
            break;
          }
          reader->deleteDocument(doc);
        }
      } _CLFINALLY (
        docs->close();
        _CLDELETE(docs);
      )
    }

    iter++;
  }

  if (deleteIds.size() > 0) {
    vector<int32_t>::const_iterator iter2 = deleteIds.begin();
    while (iter2 != deleteIds.end()){
      reader->deleteDocument(*iter2);
      ++iter2;
    }
  }
}

void IndexWriter::Internal::applyDeletes(const DocumentsWriter::TermNumMapType& deleteTerms, IndexReader* reader)
{
  DocumentsWriter::TermNumMapType::const_iterator iter = deleteTerms.begin();
  while (iter != deleteTerms.end()) {
    reader->deleteDocuments(iter->first);
    iter++;
  }
}

SegmentInfo* IndexWriter::newestSegment() {
  return segmentInfos->info(segmentInfos->size()-1);
}

string IndexWriter::segString() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  std::string buffer;
  for(int32_t i = 0; i < segmentInfos->size(); i++) {
    if (i > 0) {
      buffer+= " ";
    }
    buffer+= segmentInfos->info(i)->segString(directory);
  }

  return buffer;
}

bool IndexWriter::testPoint(const char* name) {
  return true;
}

CL_NS_END
