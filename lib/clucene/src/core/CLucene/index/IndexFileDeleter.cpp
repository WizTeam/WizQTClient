#include "CLucene/_ApiHeader.h"
#include "_IndexFileDeleter.h"
#include "_IndexFileNameFilter.h"
#include "_DocumentsWriter.h"
#include "_SegmentHeader.h"
#include "CLucene/store/Directory.h"
#include "CLucene/LuceneThreads.h"
#include <algorithm>
#include <assert.h>
#include <iostream>

CL_NS_USE(store)
CL_NS_USE(util)
CL_NS_DEF(index)

bool IndexFileDeleter::VERBOSE_REF_COUNTS = false;

IndexFileDeleter::CommitPoint::CommitPoint(IndexFileDeleter* _this, SegmentInfos* segmentInfos){
  this->_this = _this;
  this->deleted = false;
  this->gen = 0;
	segmentsFileName = segmentInfos->getCurrentSegmentFileName();
	int32_t size = segmentInfos->size();
	files.push_back(segmentsFileName);
	gen = segmentInfos->getGeneration();
	for(int32_t i=0;i<size;i++) {
	  SegmentInfo* segmentInfo = segmentInfos->info(i);
	  if (segmentInfo->dir == _this->directory) {
      const vector<string>& ff = segmentInfo->files();
      files.insert(files.end(),ff.begin(), ff.end());
	  }
	}

}
IndexFileDeleter::CommitPoint::~CommitPoint(){
}

/**
* Get the segments_N file for this commit point32_t.
*/
std::string IndexFileDeleter::CommitPoint::getSegmentsFileName() {
	return segmentsFileName;
}
bool IndexFileDeleter::CommitPoint::sort(IndexCommitPoint* elem1, IndexCommitPoint* elem2){
  if (((CommitPoint*)elem1)->gen < ((CommitPoint*)elem2)->gen)
    return true;
  return false;
}

const std::vector<std::string>& IndexFileDeleter::CommitPoint::getFileNames() {
	return files;
}

/**
* Called only be the deletion policy, to remove this
* commit point32_t from the index.
*/
void IndexFileDeleter::CommitPoint::deleteCommitPoint() {
	if (!deleted) {
	  deleted = true;
	  _this->commitsToDelete.push_back(this);
	}
}

const char* IndexFileDeleter::CommitPoint::getClassName(){
  return "IndexFileDeleter::CommitPoint";
}
const char* IndexFileDeleter::CommitPoint::getObjectName() const{
  return getClassName();
}
int32_t IndexFileDeleter::CommitPoint::compareTo(NamedObject* obj) {
  if ( obj->getObjectName() != CommitPoint::getClassName() )
    return -1;

	CommitPoint* commit = (CommitPoint*) obj;
	if (gen < commit->gen) {
	  return -1;
	} else if (gen > commit->gen) {
	  return 1;
	} else {
	  return 0;
	}
}

void IndexFileDeleter::setInfoStream(std::ostream* infoStream) {
	this->infoStream = infoStream;
	if (infoStream != NULL){
		string msg = string("setInfoStream deletionPolicy=") + policy->getObjectName();
	  message( msg );
	}
}

void IndexFileDeleter::message(string message) {
	(*infoStream) << string("IFD [") << Misc::toString( _LUCENE_CURRTHREADID ) << string("]: ") << message << string("\n");
}


IndexFileDeleter::~IndexFileDeleter(){
  _CLDELETE(policy);
  commitsToDelete.clear();
  commits.clear();
  refCounts.clear();
}
IndexFileDeleter::IndexFileDeleter(Directory* directory, IndexDeletionPolicy* policy,
  SegmentInfos* segmentInfos, std::ostream* infoStream, DocumentsWriter* docWriter):
  refCounts( RefCountsType(true,true) ), commits(CommitsType(true))
{
	this->docWriter = docWriter;
	this->infoStream = infoStream;

	if (infoStream != NULL)
	  message( string("init: current segments file is \"") + segmentInfos->getCurrentSegmentFileName() + "\"; deletionPolicy=" + policy->getObjectName());

	this->policy = policy;
	this->directory = directory;
  CommitPoint* currentCommitPoint = NULL;

	// First pass: walk the files and initialize our ref
	// counts:
	int64_t currentGen = segmentInfos->getGeneration();
	const IndexFileNameFilter* filter = IndexFileNameFilter::getFilter();

	vector<string> files;
  if ( !directory->list(&files) )
	  _CLTHROWA(CL_ERR_IO, (string("cannot read directory ") + directory->toString() + ": list() returned NULL").c_str());


	for(size_t i=0;i<files.size();i++) {

	  string& fileName = files.at(i);

    if (filter->accept(NULL, fileName.c_str()) && !fileName.compare(IndexFileNames::SEGMENTS_GEN) == 0) {

	    // Add this file to refCounts with initial count 0:
	    getRefCount(fileName.c_str());

	    if ( strncmp(fileName.c_str(), IndexFileNames::SEGMENTS, strlen(IndexFileNames::SEGMENTS)) == 0 ) {

	      // This is a commit (segments or segments_N), and
	      // it's valid (<= the max gen).  Load it, then
	      // incref all files it refers to:
	      if (SegmentInfos::generationFromSegmentsFileName(fileName.c_str()) <= currentGen) {
	        if (infoStream != NULL) {
	          message("init: load commit \"" + fileName + "\"");
	        }
	        SegmentInfos sis;
          bool failed = false;
	        try {
	          sis.read(directory, fileName.c_str());
	        } catch (CLuceneError& e) {
            if ( e.number() != CL_ERR_IO ){
              throw e;
            }
	          // LUCENE-948: on NFS (and maybe others), if
	          // you have writers switching back and forth
	          // between machines, it's very likely that the
	          // dir listing will be stale and will claim a
	          // file segments_X exists when in fact it
	          // doesn't.  So, we catch this and handle it
	          // as if the file does not exist
	          if (infoStream != NULL) {
	            message("init: hit FileNotFoundException when loading commit \"" + fileName + "\"; skipping this commit point32_t");
	          }
	          failed = true;
	        }
	        if (!failed) {
	          CommitPoint* commitPoint = _CLNEW CommitPoint(this,&sis);
	          if (sis.getGeneration() == segmentInfos->getGeneration()) {
	            currentCommitPoint = commitPoint;
	          }
	          commits.push_back(commitPoint);
	          incRef(&sis, true);
	        }
	      }
	    }
	  }
	}

	if (currentCommitPoint == NULL) {
	  // We did not in fact see the segments_N file
	  // corresponding to the segmentInfos that was passed
	  // in.  Yet, it must exist, because our caller holds
	  // the write lock.  This can happen when the directory
	  // listing was stale (eg when index accessed via NFS
	  // client with stale directory listing cache).  So we
	  // try now to explicitly open this commit point32_t:
	  SegmentInfos sis;
	  try {
	    sis.read(directory, segmentInfos->getCurrentSegmentFileName().c_str());
	  } catch (CLuceneError& e) {
      if ( e.number() == CL_ERR_IO ){
	      _CLTHROWA(CL_ERR_CorruptIndex, "failed to locate current segments_N file");
      }
	  }
	  if (infoStream != NULL)
	    message("forced open of current segments file " + segmentInfos->getCurrentSegmentFileName());
	  currentCommitPoint = _CLNEW CommitPoint(this,&sis);
    commits.push_back(currentCommitPoint);
	  incRef(&sis, true);
	}

	// We keep commits list in sorted order (oldest to newest):
  	std::sort(commits.begin(), commits.end(), CommitPoint::sort);

	// Now delete anything with ref count at 0.  These are
	// presumably abandoned files eg due to crash of
	// IndexWriter.
  RefCountsType::iterator it = refCounts.begin();
	while(it != refCounts.end()) {
    char* fileName = it->first;
    RefCount* rc = it->second;
	  if (0 == rc->count) {
	    if (infoStream != NULL) {
	      message( string("init: removing unreferenced file \"") + fileName + "\"");
	    }
	    deleteFile(fileName);
	  }
    it++;
	}

	// Finally, give policy a chance to remove things on
	// startup:
	policy->onInit(commits);

	// It's OK for the onInit to remove the current commit
	// point; we just have to checkpoint our in-memory
	// SegmentInfos to protect those files that it uses:
	if (currentCommitPoint->deleted) {
	  checkpoint(segmentInfos, false);
	}

	deleteCommits();
}

/**
* Remove the CommitPoints in the commitsToDelete List by
* DecRef'ing all files from each segmentInfos->
*/
void IndexFileDeleter::deleteCommits() {

  int32_t size = commitsToDelete.size();

  if (size > 0) {

    // First decref all files that had been referred to by
    // the now-deleted commits:
    for(int32_t i=0;i<size;i++) {
      CommitPoint* commit = commitsToDelete[i];
      if (infoStream != NULL) {
        message("deleteCommits: now remove commit \"" + commit->getSegmentsFileName() + "\"");
      }
      decRef(commit->files);
    }
    commitsToDelete.clear();

    // Now compact commits to remove deleted ones (preserving the sort):
    size = commits.size();
    int32_t readFrom = 0;
    int32_t writeTo = 0;
    while(readFrom < size) {
      CommitPoint* commit = (CommitPoint*)commits[readFrom];
      if (!commit->deleted) {
        if (writeTo != readFrom) {
          commits.remove(readFrom,true);
          commits.remove(writeTo,false);//delete this one...
          if ( commits.size() == writeTo )
            commits.push_back(commit);
          else
            commits[writeTo] = commit;
        }
        writeTo++;
      }
      readFrom++;
    }

    while(size > writeTo) {
      commits.remove(size-1);
      size--;
    }
  }
}

/**
* Writer calls this when it has hit an error and had to
* roll back, to tell us that there may now be
* unreferenced files in the filesystem.  So we re-list
* the filesystem and delete such files.  If segmentName
* is non-NULL, we will only delete files corresponding to
* that segment.
*/
void IndexFileDeleter::refresh(const char* segmentName) {
  vector<string> files;
  if ( !directory->list(files) )
    _CLTHROWA(CL_ERR_IO, (string("cannot read directory ") + directory->toString() + ": list() returned NULL").c_str() );
  const IndexFileNameFilter* filter = IndexFileNameFilter::getFilter();
  string segmentPrefix1;
  string segmentPrefix2;
  if (segmentName != NULL) {
    segmentPrefix1 = string(segmentName) + ".";
    segmentPrefix2 = string(segmentName) + "_";
  }

  for(size_t i=0;i<files.size();i++) {
    string& fileName = files[i];
    if ( filter->accept(NULL, fileName.c_str()) &&
        ( (segmentName==NULL || fileName.compare(0,segmentPrefix1.length(),segmentPrefix1) == 0 || fileName.compare(0,segmentPrefix2.length(),segmentPrefix2)==0)
          && refCounts.find((char*)fileName.c_str())== refCounts.end() && fileName.compare(IndexFileNames::SEGMENTS_GEN)!=0) ){

      // Unreferenced file, so remove it
      if (infoStream != NULL) {
        message( string("refresh [prefix=") + segmentName + "]: removing newly created unreferenced file \"" + fileName + "\"");
      }
      deleteFile(fileName.c_str());
    }
  }
}

void IndexFileDeleter::refresh() {
  refresh(NULL);
}

void IndexFileDeleter::close() {
  deletePendingFiles();
}

void IndexFileDeleter::deletePendingFiles() {
  if (!deletable.empty()) {
    vector<string> oldDeletable;
    oldDeletable.insert(oldDeletable.end(),deletable.begin(),deletable.end());
    deletable.clear();

    int32_t size = oldDeletable.size();
    for(int32_t i=0;i<size;i++) {
      if (infoStream != NULL)
        message("delete pending file " + oldDeletable[i]);
      deleteFile(oldDeletable[i].c_str());
    }
  }
}

/**
* For definition of "check point32_t" see IndexWriter comments:
* "Clarification: Check Point32_ts (and commits)".
*
* Writer calls this when it has made a "consistent
* change" to the index, meaning new files are written to
* the index and the in-memory SegmentInfos have been
* modified to point32_t to those files.
*
* This may or may not be a commit (segments_N may or may
* not have been written).
*
* We simply incref the files referenced by the new
* SegmentInfos and decref the files we had previously
* seen (if any).
*
* If this is a commit, we also call the policy to give it
* a chance to remove other commits.  If any commits are
* removed, we decref their files as well.
*/
void IndexFileDeleter::checkpoint(SegmentInfos* segmentInfos, bool isCommit) {

  if (infoStream != NULL) {
    message(string("now checkpoint \"") + segmentInfos->getCurrentSegmentFileName() + "\" [" +
      Misc::toString(segmentInfos->size()) + " segments ; isCommit = " + Misc::toString(isCommit) + "]");
  }

  // Try again now to delete any previously un-deletable
  // files (because they were in use, on Windows):
  deletePendingFiles();

  // Incref the files:
  incRef(segmentInfos, isCommit);
  const vector<string>* docWriterFiles = NULL;
  if (docWriter != NULL) {
    docWriterFiles = &docWriter->files();
    if (!docWriterFiles->empty())
      incRef(*docWriterFiles);
    else
      docWriterFiles = NULL;
  }

  if (isCommit) {
    // Append to our commits list:
    commits.push_back(_CLNEW CommitPoint(this, segmentInfos));

    // Tell policy so it can remove commits:
    policy->onCommit(commits);

    // Decref files for commits that were deleted by the policy:
    deleteCommits();
  }

  // DecRef old files from the last checkpoint, if any:
  int32_t size = lastFiles.size();
  if (size > 0) {
    for(int32_t i=0;i<size;i++)
      decRef(lastFiles[i]);
    lastFiles.clear();
  }

  if (!isCommit) {
    // Save files so we can decr on next checkpoint/commit:
    size = segmentInfos->size();
    for(int32_t i=0;i<size;i++) {
      SegmentInfo* segmentInfo = segmentInfos->info(i);
      if (segmentInfo->dir == directory) {
        const vector<string>& files = segmentInfo->files();
        lastFiles.insert(lastFiles.end(), files.begin(), files.end());
      }
    }
  }
  if (docWriterFiles != NULL)
    lastFiles.insert(lastFiles.end(), docWriterFiles->begin(),docWriterFiles->end());
}

void IndexFileDeleter::incRef(SegmentInfos* segmentInfos, bool isCommit) {
  int32_t size = segmentInfos->size();
  for(int32_t i=0;i<size;i++) {
    SegmentInfo* segmentInfo = segmentInfos->info(i);
    if (segmentInfo->dir == directory) {
      incRef(segmentInfo->files());
    }
  }

  if (isCommit) {
    // Since this is a commit point32_t, also incref its
    // segments_N file:
    getRefCount(segmentInfos->getCurrentSegmentFileName().c_str())->IncRef();
  }
}

void IndexFileDeleter::incRef(const vector<string>& files) {
  int32_t size = files.size();
  for(int32_t i=0;i<size;i++) {
    const string& fileName = files[i];
    RefCount* rc = getRefCount(fileName.c_str());
    if (infoStream != NULL && VERBOSE_REF_COUNTS) {
      message(string("  IncRef \"") + fileName + "\": pre-incr count is " + Misc::toString((int32_t)rc->count));
    }
    rc->IncRef();
  }
}

void IndexFileDeleter::decRef(const vector<string>& files) {
  int32_t size = files.size();
  for(int32_t i=0;i<size;i++) {
    decRef(files[i]);
  }
}

void IndexFileDeleter::decRef(const string& fileName) {
  RefCount* rc = getRefCount(fileName.c_str());
  if (infoStream != NULL && VERBOSE_REF_COUNTS) {
    message(string("  DecRef \"") + fileName + "\": pre-decr count is " + Misc::toString((int32_t)rc->count));
  }
  if (0 == rc->DecRef()) {
    // This file is no int32_t64_ter referenced by any past
    // commit point32_ts nor by the in-memory SegmentInfos:
    deleteFile(fileName.c_str());
    refCounts.remove((char*)fileName.c_str());
  }
}

void IndexFileDeleter::decRef(SegmentInfos* segmentInfos) {
  int32_t size = segmentInfos->size();
  for(int32_t i=0;i<size;i++) {
    SegmentInfo* segmentInfo = segmentInfos->info(i);
    if (segmentInfo->dir == directory) {
      decRef(segmentInfo->files());
    }
  }
}

IndexFileDeleter::RefCount* IndexFileDeleter::getRefCount(const char* fileName) {
  RefCount* rc;
  RefCountsType::iterator itr = refCounts.find((char*)fileName);
  if (itr == refCounts.end()) {
    rc = _CLNEW RefCount();
    refCounts.put( STRDUP_AtoA(fileName), rc);
  } else {
    rc = itr->second;
  }
  return rc;
}

void IndexFileDeleter::deleteFiles(vector<string>& files) {
  int32_t size = files.size();
  for(int32_t i=0;i<size;i++)
    deleteFile(files[i].c_str());
}

/** Delets the specified files, but only if they are new
*  (have not yet been incref'd). */
void IndexFileDeleter::deleteNewFiles(const std::vector<std::string>& files) {
	int32_t size = files.size();
	for(int32_t i=0;i<size;i++)
	  if (refCounts.find((char*)files[i].c_str()) == refCounts.end())
	    deleteFile(files[i].c_str());
}

void IndexFileDeleter::deleteFile(const char* fileName)
{
	try {
	  if (infoStream != NULL) {
	    message(string("delete \"") + fileName + "\"");
	  }
	  directory->deleteFile(fileName);
	} catch (CLuceneError& e) {       // if delete fails
    if ( e.number() != CL_ERR_IO ){
      throw e;
    }
	  if (directory->fileExists(fileName)) {

	    // Some operating systems (e.g. Windows) don't
	    // permit a file to be deleted while it is opened
	    // for read (e.g. by another process or thread). So
	    // we assume that when a delete fails it is because
	    // the file is open in another process, and queue
	    // the file for subsequent deletion.

	    if (infoStream != NULL) {
	      message(string("IndexFileDeleter: unable to remove file \"") + fileName + "\": " + e.what() + "; Will re-try later.");
	    }
	    deletable.push_back(fileName);                  // add to deletable
	  }
	}
}

CL_NS_END
