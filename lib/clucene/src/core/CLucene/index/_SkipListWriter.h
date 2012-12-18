/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_SkipListWriter_
#define _lucene_index_SkipListWriter_

#include "CLucene/store/IndexInput.h"
#include "CLucene/store/_RAMDirectory.h"
#include "CLucene/util/Array.h"

CL_NS_DEF(index)

/**
 * This abstract class writes skip lists with multiple levels.
 * 
 * Example for skipInterval = 3:
 *                                                     c            (skip level 2)
 *                 c                 c                 c            (skip level 1) 
 *     x     x     x     x     x     x     x     x     x     x      (skip level 0)
 * d d d d d d d d d d d d d d d d d d d d d d d d d d d d d d d d  (posting list)
 *     3     6     9     12    15    18    21    24    27    30     (df)
 * 
 * d - document
 * x - skip data
 * c - skip data with child pointer
 * 
 * Skip level i contains every skipInterval-th entry from skip level i-1.
 * Therefore the number of entries on level i is: floor(df / ((skipInterval ^ (i + 1))).
 * 
 * Each skip entry on a level i>0 contains a pointer to the corresponding skip entry in list i-1.
 * This guarantess a logarithmic amount of skips to find the target document.
 * 
 * While this class takes care of writing the different skip levels,
 * subclasses must define the actual format of the skip data.
 * 
 */
class MultiLevelSkipListWriter {
private:
  // the skip interval in the list with level = 0
  int32_t skipInterval;
  
  // for every skip level a different buffer is used 
  CL_NS(util)::ArrayBase<CL_NS(store)::RAMOutputStream*>* skipBuffer;


  /**
   * Writes the current skip data to the buffers. The current document frequency determines
   * the max level is skip data is to be written to. 
   * 
   * @param df the current document frequency 
   * @throws IOException
   */
  void bufferSkip(int32_t df);

  /**
   * Writes the buffered skip lists to the given output.
   * 
   * @param output the IndexOutput the skip lists shall be written to 
   * @return the pointer the skip list starts
   */
  int64_t writeSkip(CL_NS(store)::IndexOutput* output);

protected:
  // number of levels in this skip list
  int32_t numberOfSkipLevels;
  
  MultiLevelSkipListWriter(int32_t skipInterval, int32_t maxSkipLevels, int32_t df);
  virtual ~MultiLevelSkipListWriter();
  void init();

  void resetSkip();

  /**
   * Subclasses must implement the actual skip data encoding in this method.
   *  
   * @param level the level skip data shall be writting for
   * @param skipBuffer the skip buffer to write to
   */
  virtual void writeSkipData(int32_t level, CL_NS(store)::IndexOutput* skipBuffer) = 0;

  friend class SegmentMerger;
  friend class DocumentsWriter;
};

/**
 * Implements the skip list writer for the default posting list format
 * that stores positions and payloads.
 *
 */
class DefaultSkipListWriter: public MultiLevelSkipListWriter {
private:
  int32_t* lastSkipDoc;
  int32_t* lastSkipPayloadLength;
  int64_t* lastSkipFreqPointer;
  int64_t* lastSkipProxPointer;
  
  CL_NS(store)::IndexOutput* freqOutput;
  CL_NS(store)::IndexOutput* proxOutput;

  int32_t curDoc;
  bool curStorePayloads;
  int32_t curPayloadLength;
  int64_t curFreqPointer;
  int64_t curProxPointer;
  
  /**
   * Sets the values for the current skip data. 
   */
  void setSkipData(int32_t doc, bool storePayloads, int32_t payloadLength);

protected:
  void resetSkip();
  
  void writeSkipData(int32_t level, CL_NS(store)::IndexOutput* skipBuffer);
public:
	
  DefaultSkipListWriter(int32_t skipInterval, int32_t numberOfSkipLevels, int32_t docCount, 
    CL_NS(store)::IndexOutput* freqOutput, CL_NS(store)::IndexOutput* proxOutput);
  ~DefaultSkipListWriter();
  
  friend class SegmentMerger;
  friend class DocumentsWriter;
};


CL_NS_END
#endif
