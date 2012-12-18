/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_SegmentMerger_
#define _lucene_index_SegmentMerger_


CL_CLASS_DEF(store,Directory)
#include "CLucene/store/_RAMDirectory.h"
#include "_SegmentMergeInfo.h"
#include "_SegmentMergeQueue.h"
#include "IndexReader.h"
#include "_TermInfosWriter.h"
#include "Terms.h"
#include "MergePolicy.h"

CL_NS_DEF(index)
class DefaultSkipListWriter;
/**
* The SegmentMerger class combines two or more Segments, represented by an IndexReader ({@link #add},
* into a single Segment.  After adding the appropriate readers, call the merge method to combine the 
* segments.
*<P> 
* If the compoundFile flag is set, then the segments will be merged into a compound file.
*   
* 
* @see #merge
* @see #add
*/
class SegmentMerger:LUCENE_BASE {
  CL_NS(util)::ValueArray<uint8_t> payloadBuffer;
	
	//Directory of the segment
	CL_NS(store)::Directory* directory;     
	//name of the new segment
  std::string segment;
	//Set of IndexReaders
	CL_NS(util)::CLVector<IndexReader*,CL_NS(util)::Deletor::Object<IndexReader> > readers;
	//Field Infos for t	he FieldInfo instances of all fields
	FieldInfos* fieldInfos;

  int32_t mergedDocs;

  // Whether we should merge doc stores (stored fields and
  // vectors files).  When all segments we are merging
  // already share the same doc store files, we don't need
  // to merge the doc stores.
  bool mergeDocStores;

  /** Maximum number of contiguous documents to bulk-copy
  when merging stored fields */
  static int32_t MAX_RAW_MERGE_DOCS;

	//The queue that holds SegmentMergeInfo instances
	SegmentMergeQueue* queue;
	//IndexOutput to the new Frequency File
	CL_NS(store)::IndexOutput* freqOutput;
  //IndexOutput to the new Prox File
	CL_NS(store)::IndexOutput* proxOutput;
	//Writes Terminfos that have been merged
	TermInfosWriter* termInfosWriter;
	TermInfo termInfo; //(new) minimize consing

  int32_t termIndexInterval;
	int32_t skipInterval;
  int32_t maxSkipLevels;
  DefaultSkipListWriter* skipListWriter;

public:
  static const uint8_t NORMS_HEADER[]; 
  static const int NORMS_HEADER_length;

	/**
	* 
	* @param dir The Directory to merge the other segments into
	* @param name The name of the new segment
	* @param compoundFile true if the new segment should use a compoundFile
	*/
  SegmentMerger( IndexWriter* writer, const char* name, MergePolicy::OneMerge* merge );

  SegmentMerger(IndexWriter* writer, std::string name, MergePolicy::OneMerge* merge);

  void init();

	//Destructor
	~SegmentMerger();
	
	/**
	* Add an IndexReader to the collection of readers that are to be merged
	* @param reader
	*/
	void add(IndexReader* reader);
	
	/**
	* 
	* @param i The index of the reader to return
	* @return The ith reader to be merged
	*/
	IndexReader* segmentReader(const int32_t i);
	
  /**
   * Merges the readers specified by the {@link #add} method
   * into the directory passed to the constructor.
   * @param mergeDocStores if false, we will not merge the
   * stored fields nor vectors files
   * @return The number of documents that were merged
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
	int32_t merge(bool mergeDocStores);
	/**
	* close all IndexReaders that have been added.
	* Should not be called before merge().
	* @throws IOException
	*/
	void closeReaders();

  
  class CheckAbort {
  private:
    float_t workCount;
    MergePolicy::OneMerge* merge;
    CL_NS(store)::Directory* dir;
  public:
    CheckAbort(MergePolicy::OneMerge* merge, CL_NS(store)::Directory* dir);

    /**
     * Records the fact that roughly units amount of work
     * have been done since this method was last called.
     * When adding time-consuming code into SegmentMerger,
     * you should test different values for units to ensure
     * that the time in between calls to merge.checkAborted
     * is up to ~ 1 second.
     */
    void work(float_t units);
  };
	
private:
  CheckAbort* checkAbort;

	void addIndexed(IndexReader* reader, FieldInfos* fieldInfos, StringArrayWithDeletor& names, 
		bool storeTermVectors, bool storePositionWithTermVector,
		bool storeOffsetWithTermVector, bool storePayloads);

	/**
	* Merge the fields of all segments 
	* @return The number of documents in all of the readers
  * @throws CorruptIndexException if the index is corrupt
  * @throws IOException if there is a low-level IO error
	*/
	int32_t mergeFields();

	/**
	* Merge the TermVectors from each of the segments into the new one.
	* @throws IOException
	*/
  	void mergeVectors();

	/** Merge the terms of all segments */
	void mergeTerms();

	/** Merges all TermInfos into a single segment */
	void mergeTermInfos();

	/** Merge one term found in one or more segments. The array <code>smis</code>
	*  contains segments that are positioned at the same term. <code>N</code>
	*  is the number of cells in the array actually occupied.
	*
	* @param smis array of segments
	* @param n number of cells in the array actually occupied
  * @throws CorruptIndexException if the index is corrupt
  * @throws IOException if there is a low-level IO error
	*/
	int32_t mergeTermInfo( SegmentMergeInfo** smis, int32_t n);
	    
	/** Process postings from multiple segments all positioned on the
	*  same term. Writes out merged entries into freqOutput and
	*  the proxOutput streams.
	*
	* @param smis array of segments
	* @param n number of cells in the array actually occupied
	* @return number of documents across all segments where this term was found
  * @throws CorruptIndexException if the index is corrupt
  * @throws IOException if there is a low-level IO error
	*/
	int32_t appendPostings(SegmentMergeInfo** smis, int32_t n);

	//Merges the norms for all fields 
	void mergeNorms();

	void createCompoundFile(const char* filename, std::vector<std::string>* files=NULL);
	friend class IndexWriter; //allow IndexWriter to use createCompoundFile
};
CL_NS_END
#endif
