/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "MergePolicy.h"
#include "_SegmentInfos.h"
#include "IndexWriter.h"
#include "CLucene/store/Directory.h"
#include <assert.h>

CL_NS_USE(util)
CL_NS_USE(store)
CL_NS_DEF(index)

#define MESSAGE(msg) if ( writer != NULL && writer->getInfoStream() != NULL ) message(msg)

const int32_t LogMergePolicy::DEFAULT_MAX_MERGE_DOCS = LUCENE_INT32_MAX_SHOULDBE;

MergePolicy::OneMerge::OneMerge(SegmentInfos* segments, bool _useCompoundFile):
  useCompoundFile(_useCompoundFile)
{
  if (0 == segments->size())
    _CLTHROWA(CL_ERR_Runtime,"segments must include at least one segment");
  this->segments = segments;
  this->info = NULL;
  this->segmentsClone = NULL;
  this->mergeGen = 0;
  this->maxNumSegmentsOptimize = 0;
  aborted = mergeDocStores = optimize = increfDone = registerDone = isExternal = false;
}
MergePolicy::OneMerge::~OneMerge(){
  _CLDELETE(this->segmentsClone);

  while ( this->segments->size() > 0 ){
    this->segments->remove(0,true);//don't delete...
  }
  _CLDELETE(this->segments);//and finally delete the segments object itself
}

const char* MergePolicy::OneMerge::getClassName(){
  return "MergePolicy::OneMerge";
}
const char* MergePolicy::OneMerge::getObjectName() const{
  return getClassName();
}
void MergePolicy::OneMerge::setException(CLuceneError& error) {
  SCOPED_LOCK_MUTEX(THIS_LOCK)
  this->error.set(error.number(),error.what());
}

const CLuceneError& MergePolicy::OneMerge::getException(){
  SCOPED_LOCK_MUTEX(THIS_LOCK)
  return error;
}

void MergePolicy::OneMerge::abort() {
  SCOPED_LOCK_MUTEX(THIS_LOCK)
  aborted = true;
}

bool MergePolicy::OneMerge::isAborted() {
  SCOPED_LOCK_MUTEX(THIS_LOCK)
  return aborted;
}

void MergePolicy::OneMerge::checkAborted(CL_NS(store)::Directory* dir){
  SCOPED_LOCK_MUTEX(THIS_LOCK)
  if (aborted)
    _CLTHROWA(CL_ERR_MergeAborted, (string("merge is aborted: ") + segString(dir)).c_str() );
}

std::string MergePolicy::OneMerge::segString(CL_NS(store)::Directory* dir) const{
  std::string b;
  const int32_t numSegments = segments->size();
  for(int32_t i=0;i<numSegments;i++) {
    if (i > 0) b.append(" ");
    b.append(segments->info(i)->segString(dir));
  }
  if (info != NULL)
    b.append(" into ").append(info->name);
  if (optimize)
    b.append(" [optimize]");
  return b;
}


MergePolicy::MergeSpecification::MergeSpecification(){
  merges = _CLNEW CLArrayList<OneMerge*>;
}
MergePolicy::MergeSpecification::~MergeSpecification(){
  _CLDELETE(merges);
}
void MergePolicy::MergeSpecification::add(OneMerge* merge) {
  merges->push_back(merge);
}

std::string MergePolicy::MergeSpecification::segString(CL_NS(store)::Directory* dir) {
  std::string b = "MergeSpec:\n";
  int32_t count = merges->size();
  for(int32_t i=0;i<count;i++){
    b.append("  ");
    b.append(Misc::toString(1 + i));
    b.append(": ");
    b.append((*merges)[i]->segString(dir));
  }
  return b;
}



const float_t LogMergePolicy::LEVEL_LOG_SPAN = 0.75;

void LogMergePolicy::message(const string& message) {
  if (writer != NULL){
  	string msg = "LMP: " + message;
    writer->message( msg );
  }
}
int32_t LogMergePolicy::getMergeFactor(){
  return mergeFactor;
}
bool LogMergePolicy::isOptimized(SegmentInfos* infos, IndexWriter* writer, int32_t maxNumSegments, std::vector<SegmentInfo*>& segmentsToOptimize){
  const int32_t numSegments = infos->size();
  int32_t numToOptimize = 0;
  SegmentInfo* optimizeInfo = NULL;
  for(int32_t i=0;i<numSegments && numToOptimize <= maxNumSegments;i++) {
    SegmentInfo* info = infos->info(i);
    vector<SegmentInfo*>::iterator itr = segmentsToOptimize.begin();
    while ( itr != segmentsToOptimize.end() ){
      if ( *itr == info ) {
        numToOptimize++;
        optimizeInfo = info;
      }
      itr++;
    }
  }

  return numToOptimize <= maxNumSegments &&
    (numToOptimize != 1 || isOptimized(writer, optimizeInfo));
}

bool LogMergePolicy::isOptimized(IndexWriter* writer, SegmentInfo* info){
  return !info->hasDeletions() &&
    !info->hasSeparateNorms() &&
    info->dir == writer->getDirectory() &&
    info->getUseCompoundFile() == _useCompoundFile;
}

LogMergePolicy::LogMergePolicy(){
  this->maxMergeDocs = DEFAULT_MAX_MERGE_DOCS;
  this->mergeFactor = DEFAULT_MERGE_FACTOR;
  this->_useCompoundFile = true;
  this->_useCompoundDocStore = true;
  this->writer = NULL;
  this->minMergeSize = this->maxMergeSize = 0;
}

void LogMergePolicy::setMergeFactor(int32_t mergeFactor) {
  if (mergeFactor < 2)
    _CLTHROWA(CL_ERR_IllegalArgument, "mergeFactor cannot be less than 2");
  this->mergeFactor = mergeFactor;
}

bool LogMergePolicy::useCompoundFile(SegmentInfos* /*infos*/, SegmentInfo* /*info*/) {
  return _useCompoundFile;
}

void LogMergePolicy::setUseCompoundFile(bool useCompoundFile) {
  this->_useCompoundFile = useCompoundFile;
}

bool LogMergePolicy::getUseCompoundFile() {
  return _useCompoundFile;
}

bool LogMergePolicy::useCompoundDocStore(SegmentInfos* /*infos*/) {
  return _useCompoundDocStore;
}

void LogMergePolicy::setUseCompoundDocStore(bool useCompoundDocStore) {
  this->_useCompoundDocStore = useCompoundDocStore;
}

bool LogMergePolicy::getUseCompoundDocStore() {
  return _useCompoundDocStore;
}

void LogMergePolicy::close() {}


MergePolicy::MergeSpecification* LogMergePolicy::findMergesForOptimize(SegmentInfos* infos, IndexWriter* writer, int32_t maxNumSegments, vector<SegmentInfo*>& segmentsToOptimize) {
  MergeSpecification* spec = NULL;

  assert (maxNumSegments > 0);

  if (!isOptimized(infos, writer, maxNumSegments, segmentsToOptimize)) {

    // Find the newest (rightmost) segment that needs to
    // be optimized (other segments may have been flushed
    // since optimize started):
    int32_t last = infos->size();
    while(last > 0) {
      const SegmentInfo* info = infos->info(--last);

      vector<SegmentInfo*>::iterator itr = segmentsToOptimize.begin();
      bool containsInfo = false;
      while (itr != segmentsToOptimize.end() ){
        if ( *itr == info ){
          containsInfo = true;
          break;
        }
        itr++;
      }
      if (containsInfo) {
        last++;
        break;
      }
    }

    if (last > 0) {
      spec = _CLNEW MergeSpecification();

      // First, enroll all "full" merges (size
      // mergeFactor) to potentially be run concurrently:
      while (last - maxNumSegments + 1 >= mergeFactor) {
        SegmentInfos* range = _CLNEW SegmentInfos;
        infos->range(last-mergeFactor, last, *range);
        spec->add(_CLNEW OneMerge(range, _useCompoundFile));
        last -= mergeFactor;
      }

      // Only if there are no full merges pending do we
      // add a final partial (< mergeFactor segments) merge:
      if (0 == spec->merges->size()) {
        if (maxNumSegments == 1) {

          // Since we must optimize down to 1 segment, the
          // choice is simple:
          if (last > 1 || !isOptimized(writer, infos->info(0))){
            SegmentInfos* range = _CLNEW SegmentInfos;
            infos->range(0, last, *range);
            spec->add(_CLNEW OneMerge(range, _useCompoundFile));
          }
        } else if (last > maxNumSegments) {

          // Take care to pick a partial merge that is
          // least cost, but does not make the index too
          // lopsided.  If we always just picked the
          // partial tail then we could produce a highly
          // lopsided index over time:

          // We must merge this many segments to leave
          // maxNumSegments in the index (from when
          // optimize was first kicked off):
          const int32_t finalMergeSize = last - maxNumSegments + 1;

          // Consider all possible starting points:
          int64_t bestSize = 0;
          int32_t bestStart = 0;

          for(int32_t i=0;i<last-finalMergeSize+1;i++) {
            int64_t sumSize = 0;
            for(int32_t j=0;j<finalMergeSize;j++)
              sumSize += size(infos->info(j+i));
            if (i == 0 || (sumSize < 2*size(infos->info(i-1)) && sumSize < bestSize)) {
              bestStart = i;
              bestSize = sumSize;
            }
          }

          SegmentInfos* range = _CLNEW SegmentInfos;
          infos->range(bestStart, bestStart+finalMergeSize, *range);
          spec->add(_CLNEW OneMerge(range, _useCompoundFile));
        }
      }

    } else
      _CLDELETE(spec);
  } else
    _CLDELETE(spec);

  return spec;
}

MergePolicy::MergeSpecification* LogMergePolicy::findMerges(SegmentInfos* infos, IndexWriter* writer){

  const int32_t numSegments = infos->size();
  this->writer = writer;
  MESSAGE( string("findMerges: ") + Misc::toString(numSegments) + " segments");

  // Compute levels, which is just log (base mergeFactor)
  // of the size of each segment
  ValueArray<float_t> levels(numSegments);
  const float_t norm = log((float_t)mergeFactor);

  for(int32_t i=0;i<numSegments;i++) {
    SegmentInfo* info = infos->info(i);
    int64_t _size = size(info);

    // Floor tiny segments
    if (_size < 1)
      _size = 1;
    levels[i] = log((float_t)_size)/norm;
  }

  float_t levelFloor;
  if (minMergeSize <= 0)
    levelFloor = 0.0;
  else
    levelFloor = log((float_t)minMergeSize)/norm;

  // Now, we quantize the log values into levels.  The
  // first level is any segment whose log size is within
  // LEVEL_LOG_SPAN of the max size, or, who has such as
  // segment "to the right".  Then, we find the max of all
  // other segments and use that to define the next level
  // segment, etc.

  MergeSpecification* spec = NULL;

  int32_t start = 0;
  while(start < numSegments) {

    // Find max level of all segments not already
    // quantized.
    float_t maxLevel = levels[start];
    for(int32_t i=1+start;i<numSegments;i++) {
      const float_t level = levels[i];
      if (level > maxLevel)
        maxLevel = level;
    }

    // Now search backwards for the rightmost segment that
    // falls into this level:
    float_t levelBottom;
    if (maxLevel < levelFloor)
      // All remaining segments fall into the min level
      levelBottom = -1.0F;
    else {
      levelBottom = maxLevel - LEVEL_LOG_SPAN;

      // Force a boundary at the level floor
      if (levelBottom < levelFloor && maxLevel >= levelFloor)
        levelBottom = levelFloor;
    }

    int32_t upto = numSegments-1;
    while(upto >= start) {
      if (levels[upto] >= levelBottom) {
        break;
      }
      upto--;
    }
    MESSAGE(string("  level ") + Misc::toString(levelBottom) + " to " + Misc::toString(maxLevel) + ": " + Misc::toString(1+upto-start) + " segments");

    // Finally, record all merges that are viable at this level:
    int32_t end = start + mergeFactor;
    while(end <= 1+upto) {
      bool anyTooLarge = false;
      for(int32_t i=start;i<end;i++) {
        SegmentInfo* info = infos->info(i);
        anyTooLarge |= (size(info) >= maxMergeSize || info->docCount >= maxMergeDocs);
      }

      if (!anyTooLarge) {
        if (spec == NULL)
          spec = _CLNEW MergeSpecification();
        MESSAGE( string("    ") + Misc::toString(start) + " to " + Misc::toString(end) + ": add this merge");
        SegmentInfos* range = _CLNEW SegmentInfos;
        infos->range(start, end, *range);
        spec->add(_CLNEW OneMerge(range, _useCompoundFile));
      } else
        MESSAGE( string("    ") + Misc::toString(start) + " to " + Misc::toString(end) + ": contains segment over maxMergeSize or maxMergeDocs; skipping");

      start = end;
      end = start + mergeFactor;
    }

    start = 1+upto;
  }

  return spec;
}

void LogMergePolicy::setMaxMergeDocs(int32_t maxMergeDocs) {
  this->maxMergeDocs = maxMergeDocs;
}

int32_t LogMergePolicy::getMaxMergeDocs() {
  return maxMergeDocs;
}

const char* LogMergePolicy::getClassName(){
  return "LogMergePolicy";
}
const char* LogMergePolicy::getObjectName() const{
  return getClassName();
}
bool LogMergePolicy::instanceOf(const char* other) const{
  const char* t = this->getObjectName();
  if ( t==other || strcmp( t, other )==0 ){
		return true;
  }
  t = getClassName();
  if ( t==other || strcmp( t, other )==0 ){
    return true;
  }
  return false;
}




LogDocMergePolicy::LogDocMergePolicy() {
  minMergeSize = DEFAULT_MIN_MERGE_DOCS;

  // maxMergeSize is never used by LogDocMergePolicy; set
  // it to Long.MAX_VALUE to disable it
  maxMergeSize = LUCENE_INT64_MAX_SHOULDBE;
}

void LogDocMergePolicy::setMinMergeDocs(int32_t minMergeDocs) {
  minMergeSize = minMergeDocs;
}

int32_t LogDocMergePolicy::getMinMergeDocs() {
  return (int32_t)minMergeSize;
}

int64_t LogDocMergePolicy::size(SegmentInfo* info) {
  return info->docCount;
}

const char* LogDocMergePolicy::getClassName(){
  return "LogDocMergePolicy";
}
const char* LogDocMergePolicy::getObjectName() const{
  return getClassName();
}




int64_t LogByteSizeMergePolicy::size(SegmentInfo* info) {
  return info->sizeInBytes();
}

/** Default minimum segment size.  @see setMinMergeMB */
const float_t LogByteSizeMergePolicy::DEFAULT_MIN_MERGE_MB = 1.6;

/** Default maximum segment size.  A segment of this size
 *  or larger will never be merged.  @see setMaxMergeMB */
const float_t LogByteSizeMergePolicy::DEFAULT_MAX_MERGE_MB = (float_t)LUCENE_INT64_MAX_SHOULDBE;

LogByteSizeMergePolicy::LogByteSizeMergePolicy() {
  minMergeSize = (int64_t) (DEFAULT_MIN_MERGE_MB*1024*1024);
  maxMergeSize = (uint64_t) (DEFAULT_MAX_MERGE_MB); //*1024*1024
}

/** <p>Determines the largest segment (measured by total
 *  byte size of the segment's files, in MB) that may be
 *  merged with other segments.  Small values (e.g., less
 *  than 50 MB) are best for interactive indexing, as this
 *  limits the length of pauses while indexing to a few
 *  seconds.  Larger values are best for batched indexing
 *  and speedier searches.</p>
 *
 *  <p>Note that {@link #setMaxMergeDocs} is also
 *  used to check whether a segment is too large for
 *  merging (it's either or).</p>*/
void LogByteSizeMergePolicy::setMaxMergeMB(float_t mb) {
  maxMergeSize = (uint64_t) (mb*1024*1024);
}

/** Returns the largest segment (meaured by total byte
 *  size of the segment's files, in MB) that may be merged
 *  with other segments.
 *  @see #setMaxMergeMB */
float_t LogByteSizeMergePolicy::getMaxMergeMB() {
  return ((float_t) maxMergeSize)/1024/1024;
}

/** Sets the minimum size for the lowest level segments.
 * Any segments below this size are considered to be on
 * the same level (even if they vary drastically in size)
 * and will be merged whenever there are mergeFactor of
 * them.  This effectively truncates the "long tail" of
 * small segments that would otherwise be created into a
 * single level.  If you set this too large, it could
 * greatly increase the merging cost during indexing (if
 * you flush many small segments). */
void LogByteSizeMergePolicy::setMinMergeMB(float_t mb) {
  minMergeSize = (int64_t) (mb*1024*1024);
}

/** Get the minimum size for a segment to remain
 *  un-merged.
 *  @see #setMinMergeMB **/
float_t LogByteSizeMergePolicy::getMinMergeMB() {
  return ((float_t) minMergeSize)/1024/1024;
}

const char* LogByteSizeMergePolicy::getClassName(){
  return "LogByteSizeMergePolicy";
}
const char* LogByteSizeMergePolicy::getObjectName() const{
  return getClassName();
}


CL_NS_END
