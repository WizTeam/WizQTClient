/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "IndexDeletionPolicy.h"

CL_NS_DEF(index)
	
IndexDeletionPolicy::~IndexDeletionPolicy(){
}
IndexCommitPoint::~IndexCommitPoint(){
}



KeepOnlyLastCommitDeletionPolicy::~KeepOnlyLastCommitDeletionPolicy(){
}

void KeepOnlyLastCommitDeletionPolicy::onInit(std::vector<IndexCommitPoint*>& commits) {
  // Note that commits.size() should normally be 1:
  onCommit(commits);
}

void KeepOnlyLastCommitDeletionPolicy::onCommit(std::vector<IndexCommitPoint*>& commits) {
  // Note that commits.size() should normally be 2 (if not
  // called by onInit above):
  size_t size = commits.size();
  for(size_t i=0;i<size-1;i++) {
    commits[i]->deleteCommitPoint();
  }
}

const char* KeepOnlyLastCommitDeletionPolicy::getClassName(){
	return "KeepOnlyLastCommitDeletionPolicy";
}
const char* KeepOnlyLastCommitDeletionPolicy::getObjectName() const{
	return getClassName();
}



/** A {@link IndexDeletionPolicy} that wraps around any other
 *  {@link IndexDeletionPolicy} and adds the ability to hold and
 *  later release a single "snapshot" of an index.  While
 *  the snapshot is held, the {@link IndexWriter} will not
 *  remove any files associated with it even if the index is
 *  otherwise being actively, arbitrarily changed.  Because
 *  we wrap another arbitrary {@link IndexDeletionPolicy}, this
 *  gives you the freedom to continue using whatever {@link
 *  IndexDeletionPolicy} you would normally want to use with your
 *  index.

class SnapshotDeletionPolicy: public IndexDeletionPolicy {
private:
  IndexCommitPoint lastCommit;
  IndexDeletionPolicy primary;
  IndexCommitPoint snapshot;
  DEFINE_MUTEX(SnapshotDeletionPolicy_LOCK)

  class MyCommitPoint: public IndexCommitPoint {
    IndexCommitPoint cp;
  public:
    MyCommitPoint(IndexCommitPoint cp) {
      this.cp = cp;
    }
    String getSegmentsFileName() {
      return cp.getSegmentsFileName();
    }
    Collection getFileNames() throws IOException {
      return cp.getFileNames();
    }
    void deleteCommitPoint() {
      synchronized(SnapshotDeletionPolicy_LOCK) {
        // Suppress the delete request if this commit point is
        // our current snapshot.
        if (snapshot != cp)
          cp->deleteCommitPoint();
      }
    }
  }

  List wrapCommits(List commits) {
    final int count = commits.size();
    List myCommits = new ArrayList(count);
    for(int i=0;i<count;i++)
      myCommits.add(new MyCommitPoint((IndexCommitPoint) commits.get(i)));
    return myCommits;
  }
public:
  SnapshotDeletionPolicy(IndexDeletionPolicy primary) {
    this.primary = primary;
  }

  void onInit(List commits) throws IOException {
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    primary.onInit(wrapCommits(commits));
    lastCommit = (IndexCommitPoint) commits.get(commits.size()-1);
  }

  void onCommit(List commits) throws IOException {
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    primary.onCommit(wrapCommits(commits));
    lastCommit = (IndexCommitPoint) commits.get(commits.size()-1);
  }

  ** Take a snapshot of the most recent commit to the
   *  index.  You must call release() to free this snapshot.
   *  Note that while the snapshot is held, the files it
   *  references will not be deleted, which will consume
   *  additional disk space in your index. If you take a
   *  snapshot at a particularly bad time (say just before
   *  you call optimize()) then in the worst case this could
   *  consume an extra 1X of your total index size, until
   *  you release the snapshot. *
  IndexCommitPoint snapshot() {
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    if (snapshot == null)
      snapshot = lastCommit;
    else
      throw new IllegalStateException("snapshot is already set; please call release() first");
    return snapshot;
  }

  ** Release the currently held snapshot. *
  void release() {
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    if (snapshot != null)
      snapshot = null;
    else
      throw new IllegalStateException("snapshot was not set; please call snapshot() first");
  }
}
*/


CL_NS_END

