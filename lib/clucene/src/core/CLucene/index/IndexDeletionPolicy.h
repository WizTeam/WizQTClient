/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_IndexDeletionPolicy_
#define _lucene_index_IndexDeletionPolicy_

#include <vector>
#include "CLucene/util/Equators.h"

CL_NS_DEF(index)


class CLUCENE_EXPORT IndexCommitPoint {
public:
  virtual ~IndexCommitPoint();
  /**
   * Get the segments file (<code>segments_N</code>) associated
   * with this commit point.
   */
  virtual std::string getSegmentsFileName() = 0;

  /**
   * Returns all index files referenced by this commit point.
   */
  virtual const std::vector<std::string>& getFileNames() = 0;

  /**
   * Delete this commit point.
   * <p>
   * Upon calling this, the writer is notified that this commit
   * point should be deleted.
   * <p>
   * Decision that a commit-point should be deleted is taken by the {@link IndexDeletionPolicy} in effect
   * and therefore this should only be called by its {@link IndexDeletionPolicy#onInit onInit()} or
   * {@link IndexDeletionPolicy#onCommit onCommit()} methods.
  */
  virtual void deleteCommitPoint() = 0;
};

/**
 * <p>Expert: policy for deletion of stale {@link IndexCommitPoint index commits}.
 *
 * <p>Implement this interface, and pass it to one
 * of the {@link IndexWriter} or {@link IndexReader}
 * constructors, to customize when older
 * {@link IndexCommitPoint point-in-time commits}
 * are deleted from the index directory.  The default deletion policy
 * is {@link KeepOnlyLastCommitDeletionPolicy}, which always
 * removes old commits as soon as a new commit is done (this
 * matches the behavior before 2.2).</p>
 *
 * <p>One expected use case for this (and the reason why it
 * was first created) is to work around problems with an
 * index directory accessed via filesystems like NFS because
 * NFS does not provide the "delete on last close" semantics
 * that Lucene's "point in time" search normally relies on.
 * By implementing a custom deletion policy, such as "a
 * commit is only removed once it has been stale for more
 * than X minutes", you can give your readers time to
 * refresh to the new commit before {@link IndexWriter}
 * removes the old commits.  Note that doing so will
 * increase the storage requirements of the index.  See <a
 * target="top"
 * href="http://issues.apache.org/jira/browse/LUCENE-710">LUCENE-710</a>
 * for details.</p>
 */
class CLUCENE_EXPORT IndexDeletionPolicy: public CL_NS(util)::NamedObject{
public:
  virtual ~IndexDeletionPolicy();

  /**
   * <p>This is called once when a writer is first
   * instantiated to give the policy a chance to remove old
   * commit points.</p>
   *
   * <p>The writer locates all index commits present in the
   * index directory and calls this method.  The policy may
   * choose to delete some of the commit points, doing so by
   * calling method {@link IndexCommitPoint#delete delete()}
   * of {@link IndexCommitPoint}.</p>
   *
   * <p><u>Note:</u> the last CommitPoint is the most recent one,
   * i.e. the "front index state". Be careful not to delete it,
   * unless you know for sure what you are doing, and unless
   * you can afford to lose the index content while doing that.
   *
   * @param commits List of current
   * {@link IndexCommitPoint point-in-time commits},
   *  sorted by age (the 0th one is the oldest commit).
   */
  virtual void onInit(std::vector<IndexCommitPoint*>& commits) = 0;

  /**
   * <p>This is called each time the writer completed a commit.
   * This gives the policy a chance to remove old commit points
   * with each commit.</p>
   *
   * <p>The policy may now choose to delete old commit points
   * by calling method {@link IndexCommitPoint#delete delete()}
   * of {@link IndexCommitPoint}.</p>
   *
   * <p>If writer has <code>autoCommit = true</code> then
   * this method will in general be called many times during
   * one instance of {@link IndexWriter}.  If
   * <code>autoCommit = false</code> then this method is
   * only called once when {@link IndexWriter#close} is
   * called, or not at all if the {@link IndexWriter#abort}
   * is called.
   *
   * <p><u>Note:</u> the last CommitPoint is the most recent one,
   * i.e. the "front index state". Be careful not to delete it,
   * unless you know for sure what you are doing, and unless
   * you can afford to lose the index content while doing that.
   *
   * @param commits List of {@link IndexCommitPoint},
   *  sorted by age (the 0th one is the oldest commit).
   */
  virtual void onCommit(std::vector<IndexCommitPoint*>& commits) = 0;
};




/**
 * This {@link IndexDeletionPolicy} implementation that
 * keeps only the most recent commit and immediately removes
 * all prior commits after a new commit is done.  This is
 * the default deletion policy.
 */

class CLUCENE_EXPORT KeepOnlyLastCommitDeletionPolicy: public IndexDeletionPolicy {
public:
  virtual ~KeepOnlyLastCommitDeletionPolicy();
  /**
   * Deletes all commits except the most recent one.
   */
  void onInit(std::vector<IndexCommitPoint*>& commits);

  /**
   * Deletes all commits except the most recent one.
   */
  void onCommit(std::vector<IndexCommitPoint*>& commits);

	static const char* getClassName();
	const char* getObjectName() const;
};

CL_NS_END
#endif
