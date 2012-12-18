/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_MergeScheduler_
#define _lucene_index_MergeScheduler_

#include "CLucene/util/Equators.h"
#include "CLucene/LuceneThreads.h"
CL_NS_DEF(index)

class IndexWriter;

/** Expert: {@link IndexWriter} uses an instance
 *  implementing this interface to execute the merges
 *  selected by a {@link MergePolicy}.  The default
 *  MergeScheduler is {@link ConcurrentMergeScheduler}.
 * <p><b>NOTE:</b> This API is new and still experimental
 * (subject to change suddenly in the next release)</p>
*/
class CLUCENE_EXPORT MergeScheduler: public CL_NS(util)::NamedObject {
public:
  /** Run the merges provided by {@link IndexWriter#getNextMerge()}. */
  virtual void merge(IndexWriter* writer) = 0;

  /** Close this MergeScheduler. */
  virtual void close() = 0;
};

/** A {@link MergeScheduler} that simply does each merge
 *  sequentially, using the current thread. */
class CLUCENE_EXPORT SerialMergeScheduler: public MergeScheduler {
public:
  DEFINE_MUTEX(THIS_LOCK)

  /** Just do the merges in sequence. We do this
   * "synchronized" so that even if the application is using
   * multiple threads, only one merge may run at a time. */
  void merge(IndexWriter* writer);
  void close();

  const char* getObjectName() const;
  static const char* getClassName();
};


CL_NS_END
#endif
