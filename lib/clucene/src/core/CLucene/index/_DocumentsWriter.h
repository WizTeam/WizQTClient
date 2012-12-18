/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_DocumentsWriter_
#define _lucene_index_DocumentsWriter_

#include "CLucene/store/IndexInput.h"
#include "CLucene/config/_threads.h"
#include "CLucene/util/Array.h"
#include "CLucene/store/_RAMDirectory.h"
#include "_TermInfo.h"

CL_CLASS_DEF(analysis,Analyzer)
CL_CLASS_DEF(analysis,Token)
CL_CLASS_DEF(analysis,TokenStream)
CL_CLASS_DEF(document,Field)
CL_CLASS_DEF(store,IndexOutput)
CL_CLASS_DEF(document,Document)
CL_CLASS_DEF(util,StringReader)

CL_NS_DEF(index)

class DocumentsWriter;
class DefaultSkipListWriter;
class FieldInfos;
class FieldsWriter;
class FieldInfos;
class IndexWriter;
class TermInfo;
class TermInfosWriter;
class Term;
class FieldInfo;
class Term_Compare;
class Term_Equals;

/** Used only internally to DW to call abort "up the stack" */
class AbortException{
public:
  CLuceneError err;
  AbortException(CLuceneError& _err, DocumentsWriter* docWriter);
};

/**
 * This class accepts multiple added documents and directly
 * writes a single segment file.  It does this more
 * efficiently than creating a single segment per document
 * (with DocumentWriter) and doing standard merges on those
 * segments.
 *
 * When a document is added, its stored fields (if any) and
 * term vectors (if any) are immediately written to the
 * Directory (ie these do not consume RAM).  The freq/prox
 * postings are accumulated into a Postings hash table keyed
 * by term.  Each entry in this hash table holds a separate
 * uint8_t stream (allocated as incrementally growing slices
 * into large shared uint8_t[] arrays) for freq and prox, that
 * contains the postings data for multiple documents.  If
 * vectors are enabled, each unique term for each document
 * also allocates a PostingVector instance to similarly
 * track the offsets & positions uint8_t stream.
 *
 * Once the Postings hash is full (ie is consuming the
 * allowed RAM) or the number of added docs is large enough
 * (in the case we are flushing by doc count instead of RAM
 * usage), we create a real segment and flush it to disk and
 * reset the Postings hash.
 *
 * In adding a document we first organize all of its fields
 * by field name.  We then process field by field, and
 * record the Posting hash per-field.  After each field we
 * flush its term vectors.  When it's time to flush the full
 * segment we first sort the fields by name, and then go
 * field by field and sorts its postings.
 *
 *
 * Threads:
 *
 * Multiple threads are allowed into addDocument at once.
 * There is an initial synchronized call to getThreadState
 * which allocates a ThreadState for this thread.  The same
 * thread will get the same ThreadState over time (thread
 * affinity) so that if there are consistent patterns (for
 * example each thread is indexing a different content
 * source) then we make better use of RAM.  Then
 * processDocument is called on that ThreadState without
 * synchronization (most of the "heavy lifting" is in this
 * call).  Finally the synchronized "finishDocument" is
 * called to flush changes to the directory.
 *
 * Each ThreadState instance has its own Posting hash. Once
 * we're using too much RAM, we flush all Posting hashes to
 * a segment by merging the docIDs in the posting lists for
 * the same term across multiple thread states (see
 * writeSegment and appendPostings).
 *
 * When flush is called by IndexWriter, or, we flush
 * internally when autoCommit=false, we forcefully idle all
 * threads and flush only once they are all idle.  This
 * means you can call flush with a given thread even while
 * other threads are actively adding/deleting documents.
 *
 *
 * Exceptions:
 *
 * Because this class directly updates in-memory posting
 * lists, and flushes stored fields and term vectors
 * directly to files in the directory, there are certain
 * limited times when an exception can corrupt this state.
 * For example, a disk full while flushing stored fields
 * leaves this file in a corrupt state.  Or, an OOM
 * exception while appending to the in-memory posting lists
 * can corrupt that posting list.  We call such exceptions
 * "aborting exceptions".  In these cases we must call
 * abort() to discard all docs added since the last flush.
 *
 * All other exceptions ("non-aborting exceptions") can
 * still partially update the index structures.  These
 * updates are consistent, but, they represent only a part
 * of the document seen up until the exception was hit.
 * When this happens, we immediately mark the document as
 * deleted so that the document is always atomically ("all
 * or none") added to the index.
 */
class DocumentsWriter {
public:

  // Number of documents a delete term applies to.
  class Num {
  private:
  	int32_t num;
  public:
    Num(int32_t num) {
      this->num = num;
    }
    int32_t getNum() {
      return num;
    }
    void setNum(int32_t num) {
      // Only record the new number if it's greater than the
      // current one.  This is important because if multiple
      // threads are replacing the same doc at nearly the
      // same time, it's possible that one thread that got a
      // higher docID is scheduled before the other
      // threads.
      if (num > this->num)
        this->num = num;
    }
  };
  typedef CL_NS(util)::CLHashMap<Term*,Num*, Term_Compare,Term_Equals,
    CL_NS(util)::Deletor::Object<Term>, CL_NS(util)::Deletor::Object<Num> > TermNumMapType;

private:
  IndexWriter* writer;
  CL_NS(store)::Directory* directory;
  DEFINE_MUTEX(THIS_LOCK)
  DEFINE_CONDITION(THIS_WAIT_CONDITION)

  FieldInfos* fieldInfos; // All fields we've seen
  CL_NS(store)::IndexOutput *tvx, *tvf, *tvd;              // To write term vectors
  FieldsWriter* fieldsWriter;              // To write stored fields

  std::string segment;                         // Current segment we are working on
  std::string docStoreSegment;                 // Current doc-store segment we are writing
  int32_t docStoreOffset;                     // Current starting doc-store offset of current segment

  int32_t nextDocID;                          // Next docID to be added
  int32_t numDocsInRAM;                       // # docs buffered in RAM
  int32_t numDocsInStore;                     // # docs written to doc stores
  int32_t nextWriteDocID;                     // Next docID to be written

  std::ostream* infoStream;

  // Currently used only for deleting a doc on hitting an non-aborting exception
  std::vector<int32_t> bufferedDeleteDocIDs;

  // The max number of delete terms that can be buffered before
  // they must be flushed to disk.
  int32_t maxBufferedDeleteTerms;

  // How much RAM we can use before flushing.  This is 0 if
  // we are flushing by doc count instead.
  int64_t ramBufferSize;

  // Flush @ this number of docs.  If rarmBufferSize is
  // non-zero we will flush by RAM usage instead.
  int32_t maxBufferedDocs;

  bool closed;

  // Coarse estimates used to measure RAM usage of buffered deletes
  static int32_t OBJECT_HEADER_BYTES;
  static int32_t OBJECT_POINTER_BYTES;    // TODO: should be 8 on 64-bit platform
  static int32_t BYTES_PER_CHAR;
  static int32_t BYTES_PER_INT;


  // This Hashmap buffers delete terms in ram before they
  // are applied.  The key is delete term; the value is
  // number of buffered documents the term applies to.
  TermNumMapType* bufferedDeleteTerms;
  int32_t numBufferedDeleteTerms;


  /* Simple StringReader that can be reset to a new string;
   * we use this when tokenizing the string value from a
   * Field. */
  typedef CL_NS(util)::StringReader ReusableStringReader;

  class ByteBlockPool;
  class CharBlockPool;
	class FieldMergeState;

  /* IndexInput that knows how to read the byte slices written
   * by Posting and PostingVector.  We read the bytes in
   * each slice until we hit the end of that slice at which
   * point we read the forwarding address of the next slice
   * and then jump to it.*/
  class ByteSliceReader: public CL_NS(store)::IndexInput {
    ByteBlockPool* pool;
    int32_t bufferUpto;
    const uint8_t* buffer;
    int32_t limit;
    int32_t level;

    int32_t upto;
    int32_t bufferOffset;
    int32_t endIndex;
  public:
    ByteSliceReader();
    virtual ~ByteSliceReader();
    void init(ByteBlockPool* pool, int32_t startIndex, int32_t endIndex);

    uint8_t readByte();
    int64_t writeTo(CL_NS(store)::IndexOutput* out);
    void nextSlice();
    void readBytes(uint8_t* b, const int32_t len);
    int64_t getFilePointer() const;
    int64_t length() const;
    void seek(const int64_t pos);
    void close();

	  IndexInput* clone() const;
	  const char* getDirectoryType() const;
	  const char* getObjectName() const;
	  static const char* getClassName();

    friend class FieldMergeState;
  };


  struct PostingVector; //predefine...

  /* Used to track postings for a single term.  One of these
   * exists per unique term seen since the last flush. */
  struct Posting {
    int32_t textStart;                                  // Address into char[] blocks where our text is stored
    int32_t docFreq;                                    // # times this term occurs in the current doc
    int32_t freqStart;                                  // Address of first uint8_t[] slice for freq
    int32_t freqUpto;                                   // Next write address for freq
    int32_t proxStart;                                  // Address of first uint8_t[] slice
    int32_t proxUpto;                                   // Next write address for prox
    int32_t lastDocID;                                  // Last docID where this term occurred
    int32_t lastDocCode;                                // Code for prior doc
    int32_t lastPosition;                               // Last position where this term occurred
    PostingVector* vector;                           // Corresponding PostingVector instance
  };

  /* Used to track data for term vectors.  One of these
   * exists per unique term seen in each field in the
   * document. */
  struct PostingVector {
    Posting* p;                                      // Corresponding Posting instance for this term
    int32_t lastOffset;                                 // Last offset we saw
    int32_t offsetStart;                                // Address of first slice for offsets
    int32_t offsetUpto;                                 // Next write address for offsets
    int32_t posStart;                                   // Address of first slice for positions
    int32_t posUpto;                                    // Next write address for positions
  };


  /* Stores norms, buffered in RAM, until they are flushed
   * to a partial segment. */
  class BufferedNorms {
  public:
    CL_NS(store)::RAMOutputStream out;
    int32_t upto;

    BufferedNorms();
    void add(float_t norm);
    void reset();
    void fill(int32_t docID);
  };


  // Used only when infoStream != null
  int64_t segmentSize(const std::string& segmentName);

  static const int32_t POINTER_NUM_BYTE;
  static const int32_t INT_NUM_BYTE;
  static const int32_t CHAR_NUM_BYTE;

  // Holds free pool of Posting instances
  CL_NS(util)::ObjectArray<Posting> postingsFreeListDW;
  int32_t postingsFreeCountDW;
  int32_t postingsAllocCountDW;

  typedef CL_NS(util)::CLArrayList<TCHAR*, CL_NS(util)::Deletor::tcArray> FreeCharBlocksType;
  FreeCharBlocksType freeCharBlocks;

  /* We have three pools of RAM: Postings, uint8_t blocks
   * (holds freq/prox posting data) and char blocks (holds
   * characters in the term).  Different docs require
   * varying amount of storage from these three classes.
   * For example, docs with many unique single-occurrence
   * short terms will use up the Postings RAM and hardly any
   * of the other two.  Whereas docs with very large terms
   * will use alot of char blocks RAM and relatively less of
   * the other two.  This method just frees allocations from
   * the pools once we are over-budget, which balances the
   * pools to match the current docs. */
  void balanceRAM();

  std::vector<std::string>* _files;                      // Cached list of files we've created
  std::vector<std::string>* _abortedFiles;               // List of files that were written before last abort()

  bool allThreadsIdle();

  bool hasNorms;                       // Whether any norms were seen since last flush

  DefaultSkipListWriter* skipListWriter;

  bool currentFieldStorePayloads;

  /** Creates a segment from all Postings in the Postings
   *  hashes across all ThreadStates & FieldDatas. */
  void writeSegment(std::vector<std::string>& flushedFiles);

  /** Returns the name of the file with this extension, on
   *  the current segment we are working on. */
  std::string segmentFileName(const std::string& extension);
  std::string segmentFileName(const char* extension);

  TermInfo termInfo; // minimize consing


  /** Reset after a flush */
  void resetPostingsData();

  static const uint8_t defaultNorm; ///=Similarity::encodeNorm(1.0f)

  bool timeToFlushDeletes();

  // Buffer a term in bufferedDeleteTerms, which records the
  // current number of documents buffered in ram so that the
  // delete term will be applied to those documents as well
  // as the disk segments.
  void addDeleteTerm(Term* term, int32_t docCount);

  // Buffer a specific docID for deletion.  Currently only
  // used when we hit a exception when adding a document
  void addDeleteDocID(int32_t docId);

  typedef CL_NS(util)::CLArrayList<uint8_t*, CL_NS(util)::Deletor::vArray<uint8_t> > FreeByteBlocksType;
  FreeByteBlocksType freeByteBlocks;


  /** Per-thread state.  We keep a separate Posting hash and
    *  other state for each thread and then merge postings *
    *  hashes from all threads when writing the segment. */
  class ThreadState {
  public:
    /** Holds data associated with a single field, including
      *  the Postings hash.  A document may have many *
      *  occurrences for a given field name; we gather all *
      *  such occurrences here (in docFields) so that we can
      *  * process the entire field at once. */
    class FieldData: public CL_NS(util)::Comparable {
    private:
      ThreadState* threadState;

      int32_t fieldCount;
	  CL_NS(util)::ValueArray<CL_NS(document)::Field*> docFields;

      FieldData* next;

      bool postingsCompacted;

      CL_NS(util)::ValueArray<Posting*> postingsHash;
      int32_t postingsHashSize;
      int32_t postingsHashHalfSize;
      int32_t postingsHashMask;

      int32_t postingsVectorsUpto;
	    DocumentsWriter* _parent;

      int32_t offsetEnd;
      CL_NS(analysis)::Token* localToken;

      int32_t offsetStartCode;
      int32_t offsetStart;

      ByteSliceReader* vectorSliceReader;

      void initPostingArrays();

      /** Only called when term vectors are enabled.  This
      *  is called the first time we see a given term for
      *  each * document, to allocate a PostingVector
      *  instance that * is used to record data needed to
      *  write the posting * vectors. */
      PostingVector* addNewVector();

      /** This is the hotspot of indexing: it's called once
      *  for every term of every document.  Its job is to *
      *  update the postings uint8_t stream (Postings hash) *
      *  based on the occurence of a single term. */
      void addPosition(CL_NS(analysis)::Token* token);

      /** Called when postings hash is too small (> 50%
      *  occupied) or too large (< 20% occupied). */
      void rehashPostings(int32_t newSize);

      /** Called once per field per document if term vectors
      *  are enabled, to write the vectors to *
      *  RAMOutputStream, which is then quickly flushed to
      *  the real term vectors files in the Directory. */
      void writeVectors(FieldInfo* fieldInfo);

      void compactPostings();

    public:
      int32_t numPostings;
      FieldInfo* fieldInfo;
      int32_t lastGen;
      int32_t position;
      int32_t length;
      int32_t offset;
      float_t boost;
      bool doNorms;
      bool doVectors;
      bool doVectorPositions;
      bool doVectorOffsets;
      void resetPostingArrays();

      FieldData(DocumentsWriter* _parent, ThreadState* __threadState, FieldInfo* fieldInfo);
      ~FieldData();

      /** So Arrays.sort can sort us. */
      int32_t compareTo(const void* o);

      /** Collapse the hash table & sort in-place. */
	    CL_NS(util)::ValueArray<Posting*>* sortPostings();

      /** Process all occurrences of one field in the document. */
      void processField(CL_NS(analysis)::Analyzer* analyzer);

      /* Invert one occurrence of one field in the document */
      void invertField(CL_NS(document)::Field* field, CL_NS(analysis)::Analyzer* analyzer, int32_t maxFieldLength);

      static bool sort(FieldData*, FieldData*);

	    const char* getObjectName() const;
      static const char* getClassName();
      int32_t compareTo(lucene::util::NamedObject *);
      friend class ThreadState;
      friend class FieldMergeState;
    };

  private:
    CL_NS(util)::ValueArray<Posting*> postingsFreeListTS;           // Free Posting instances
    int32_t postingsFreeCountTS;

    CL_NS(util)::ValueArray<int64_t> vectorFieldPointers;
    CL_NS(util)::ValueArray<int32_t> vectorFieldNumbers;

    int32_t numStoredFields;                  // How many stored fields in current doc
    float_t docBoost;                       // Boost for current doc

    CL_NS(util)::ValueArray<FieldData*> fieldDataArray;           // Fields touched by current doc
    int32_t numFieldData;                     // How many fields in current doc
    int32_t numVectorFields;                  // How many vector fields in current doc

    CL_NS(util)::ValueArray<FieldData*> fieldDataHash;            // Hash FieldData instances by field name
    int32_t fieldDataHashMask;
    TCHAR* maxTermPrefix;                 // Non-null prefix of a too-large term if this
                                          // doc has one

    int32_t fieldGen;

    CL_NS(util)::ObjectArray<PostingVector> postingsVectors;
    int32_t maxPostingsVectors;

    // Used to read a string value for a field
    ReusableStringReader* stringReader;


    ByteBlockPool* postingsPool;
    ByteBlockPool* vectorsPool;
    CharBlockPool* charPool;

    // Current posting we are working on
    Posting* p;
    PostingVector* vector;

    //writeFreqByte...
    uint8_t* freq;
    int32_t freqUpto;

    //writeProxByte...
    uint8_t* prox;
    int32_t proxUpto;

    //writeOffsetByte...
    uint8_t* offsets;
    int32_t offsetUpto;

    //writePosByte...
    uint8_t* pos;
    int32_t posUpto;


    /** Do in-place sort of Posting array */
    void doPostingSort(Posting** postings, int32_t numPosting);

    void quickSort(Posting** postings, int32_t lo, int32_t hi);

    /** Do in-place sort of PostingVector array */
    void doVectorSort(CL_NS(util)::ArrayBase<PostingVector*>& postings, int32_t numPosting);

    void quickSort(CL_NS(util)::ArrayBase<PostingVector*>& postings, int32_t lo, int32_t hi);

    // USE ONLY FOR DEBUGGING!
    /*
      public String getPostingText() {
      char[] text = charPool.buffers[p.textStart >> CHAR_BLOCK_SHIFT];
      int32_t upto = p.textStart & CHAR_BLOCK_MASK;
      while(text[upto] != CLUCENE_END_OF_WORD)
      upto++;
      return new String(text, p.textStart, upto-(p.textStart & BYTE_BLOCK_MASK));
      }
    */

    /** Test whether the text for current Posting p equals
      *  current tokenText. */
    bool postingEquals(const TCHAR* tokenText, int32_t tokenTextLen);

    /** Compares term text for two Posting instance and
      *  returns -1 if p1 < p2; 1 if p1 > p2; else 0.
      */
    int32_t comparePostings(Posting* p1, Posting* p2);


  public:
    bool isIdle;                // Whether we are in use
    CL_NS(store)::RAMOutputStream* tvfLocal;    // Term vectors for one doc
    CL_NS(store)::RAMOutputStream* fdtLocal;    // Stored fields for one doc
    FieldsWriter* localFieldsWriter;       // Fields for one doc
    int32_t numThreads;                   // Number of threads that use this instance
    int32_t numAllFieldData;
    CL_NS(util)::ValueArray<FieldData*> allFieldDataArray; // All FieldData instances
    bool doFlushAfter;
    int32_t docID;                            // docID we are now working on

    DocumentsWriter* _parent;

    ThreadState(DocumentsWriter* _parent);
    virtual ~ThreadState();

    /** Initializes shared state for this new document */
    void init(CL_NS(document)::Document* doc, int32_t docID);

    /** Tokenizes the fields of a document into Postings */
    void processDocument(CL_NS(analysis)::Analyzer* analyzer);

    /** If there are fields we've seen but did not see again
      *  in the last run, then free them up.  Also reduce
      *  postings hash size. */
    void trimFields();

    /** Clear the postings hash and return objects back to
      *  shared pool */
    void resetPostings();

    /** Move all per-document state that was accumulated in
      *  the ThreadState into the "real" stores. */
    void writeDocument();

    /** Write vInt into freq stream of current Posting */
    void writeFreqVInt(int32_t i);

    /** Write vInt into prox stream of current Posting */
    void writeProxVInt(int32_t i);

    /** Write uint8_t into freq stream of current Posting */
    void writeFreqByte(uint8_t b);

    /** Write uint8_t into prox stream of current Posting */
    void writeProxByte(uint8_t b);

    /** Currently only used to copy a payload into the prox
      *  stream. */
    void writeProxBytes(uint8_t* b, int32_t offset, int32_t len);

    /** Write vInt into offsets stream of current
      *  PostingVector */
    void writeOffsetVInt(int32_t i);

    /** Write uint8_t into offsets stream of current
      *  PostingVector */
    void writeOffsetByte(uint8_t b);

    /** Write vInt into pos stream of current
      *  PostingVector */
    void writePosVInt(int32_t i);

    /** Write uint8_t into pos stream of current
      *  PostingVector */
    void writePosByte(uint8_t b);

    friend class FieldMergeState;
  };

  /* Class that Posting and PostingVector use to write uint8_t
   * streams into shared fixed-size uint8_t[] arrays.  The idea
   * is to allocate slices of increasing lengths For
   * example, the first slice is 5 bytes, the next slice is
   * 14, etc.  We start by writing our bytes into the first
   * 5 bytes.  When we hit the end of the slice, we allocate
   * the next slice and then write the address of the new
   * slice into the last 4 bytes of the previous slice (the
   * "forwarding address").
   *
   * Each slice is filled with 0's initially, and we mark
   * the end with a non-zero uint8_t.  This way the methods
   * that are writing into the slice don't need to record
   * its length and instead allocate a new slice once they
   * hit a non-zero uint8_t. */
  template<typename T>
  class BlockPool {
  protected:
    bool trackAllocations;

    int32_t numBuffer;

    int32_t bufferUpto;           // Which buffer we are upto
    int32_t blockSize;

    DocumentsWriter* parent;
  public:
    CL_NS(util)::ValueArray< T* > buffers;
    int32_t tOffset;          // Current head offset
    int32_t tUpto;             // Where we are in head buffer
    T* buffer;                              // Current head buffer

    virtual T* getNewBlock(bool trackAllocations) = 0;

    BlockPool(DocumentsWriter* _parent, int32_t _blockSize, bool trackAllocations):
      buffers(CL_NS(util)::ValueArray<T*>(10))
    {
	    this->blockSize = _blockSize;
      this->parent = _parent;
      bufferUpto = -1;
      tUpto = blockSize;
      tOffset = -blockSize;
      buffer = NULL;
      numBuffer = 0;
      this->trackAllocations = trackAllocations;
      buffer = NULL;
    }
	  virtual ~BlockPool(){
		  buffers.deleteValues();
	  }

    virtual void reset() = 0;

    void nextBuffer() {
      if (1+bufferUpto == buffers.length) {
      	//expand the number of buffers
        buffers.resize( (int32_t)(buffers.length * 1.5));
      }
      buffer = buffers.values[1+bufferUpto] = getNewBlock(trackAllocations);
      bufferUpto++;

      tUpto = 0;
      tOffset += blockSize;
    }

		friend class DocumentsWriter;
    friend class DocumentsWriter::ThreadState;
    friend class DocumentsWriter::ThreadState::FieldData;
    friend class DocumentsWriter::FieldMergeState;
    friend class DocumentsWriter::ByteSliceReader;
  };

  class CharBlockPool: public BlockPool<TCHAR>{
  public:
    CharBlockPool(DocumentsWriter* _parent);
    virtual ~CharBlockPool();
    TCHAR* getNewBlock(bool trackAllocations);
    void reset();
    friend class DocumentsWriter::FieldMergeState;
  };
  class ByteBlockPool: public BlockPool<uint8_t>{
  public:
    ByteBlockPool( bool _trackAllocations, DocumentsWriter* _parent);
    virtual ~ByteBlockPool();
    uint8_t* getNewBlock(bool trackAllocations);
    int32_t newSlice(const int32_t size);
    int32_t allocSlice(uint8_t* slice, const int32_t upto);
    void reset();

    friend class DocumentsWriter::ThreadState;
  };



  // Max # ThreadState instances; if there are more threads
  // than this they share ThreadStates
  static const int32_t MAX_THREAD_STATE;
  CL_NS(util)::ValueArray<ThreadState*> threadStates;
  CL_NS(util)::CLHashMap<_LUCENE_THREADID_TYPE, ThreadState*,
    CL_NS (util)::CLuceneThreadIdCompare,CL_NS (util)::CLuceneThreadIdCompare,
    CL_NS (util)::Deletor::ConstNullVal<_LUCENE_THREADID_TYPE>,
    CL_NS (util)::Deletor::Object<ThreadState> > threadBindings;
  int32_t numWaiting;
  CL_NS(util)::ValueArray<ThreadState*> waitingThreadStates;
  int32_t pauseThreads;                       // Non-zero when we need all threads to
                                                  // pause (eg to flush)
  bool flushPending;                   // True when a thread has decided to flush
  bool bufferIsFull;                   // True when it's time to write segment
  int32_t abortCount;                         // Non-zero while abort is pending or running

  CL_NS(util)::ObjectArray<BufferedNorms> norms;   // Holds norms until we flush

  /** Does the synchronized work to finish/flush the
   * inverted document. */
  void finishDocument(ThreadState* state);


  /** Used to merge the postings from multiple ThreadStates
   * when creating a segment */
  class FieldMergeState {
  private:
    ThreadState::FieldData* field;
    CL_NS(util)::ValueArray<Posting*>* postings;

    Posting* p;
    TCHAR* text;
    int32_t textOffset;

    int32_t postingUpto;

    ByteSliceReader freq;
    ByteSliceReader prox;

    int32_t docID;
    int32_t termFreq;
  public:
    FieldMergeState();
    ~FieldMergeState();
    bool nextTerm();
    bool nextDoc();

    friend class DocumentsWriter;
  };


public:
  DocumentsWriter(CL_NS(store)::Directory* directory, IndexWriter* writer);
  ~DocumentsWriter();

  /** If non-null, various details of indexing are printed
   *  here. */
  void setInfoStream(std::ostream* infoStream);

  /** Set how much RAM we can use before flushing. */
  void setRAMBufferSizeMB(float_t mb);

  float_t getRAMBufferSizeMB();

  /** Set max buffered docs, which means we will flush by
   *  doc count instead of by RAM usage. */
  void setMaxBufferedDocs(int32_t count);

  int32_t getMaxBufferedDocs();

  /** Get current segment name we are writing. */
  std::string getSegment();

  /** Returns how many docs are currently buffered in RAM. */
  int32_t getNumDocsInRAM();

  /** Returns the current doc store segment we are writing
   *  to.  This will be the same as segment when autoCommit
   *  * is true. */
  const std::string& getDocStoreSegment();

  /** Returns the doc offset into the shared doc store for
   *  the current buffered docs. */
  int32_t getDocStoreOffset();

  /** Closes the current open doc stores an returns the doc
   *  store segment name.  This returns a blank string if there are
   *  no buffered documents. */
  std::string closeDocStore();

  const std::vector<std::string>* abortedFiles();

  /* Returns list of files in use by this instance,
   * including any flushed segments. */
  const std::vector<std::string>& files();

  void setAborting();

  /** Called if we hit an exception when adding docs,
   *  flushing, etc.  This resets our state, discarding any
   *  docs added since last flush.  If ae is non-null, it
   *  contains the root cause exception (which we re-throw
   *  after we are done aborting). */
  void abort(AbortException* ae);

  // Returns true if an abort is in progress
  bool pauseAllThreads();

  void resumeAllThreads();

  std::vector<std::string> newFiles;

  /** Flush all pending docs to a new segment */
  int32_t flush(bool closeDocStore);

  /** Build compound file for the segment we just flushed */
  void createCompoundFile(const std::string& segment);

  /** Set flushPending if it is not already set and returns
   *  whether it was set. This is used by IndexWriter to *
   *  trigger a single flush even when multiple threads are
   *  * trying to do so. */
  bool setFlushPending();

  void clearFlushPending();

  /** Write norms in the "true" segment format.  This is
  *  called only during commit, to create the .nrm file. */
  void writeNorms(const std::string& segmentName, int32_t totalNumDoc);

  int32_t compareText(const TCHAR* text1, const TCHAR* text2);

  /* Walk through all unique text tokens (Posting
   * instances) found in this field and serialize them
   * into a single RAM segment. */
  void appendPostings(CL_NS(util)::ArrayBase<ThreadState::FieldData*>* fields,
                      TermInfosWriter* termsOut,
                      CL_NS(store)::IndexOutput* freqOut,
                      CL_NS(store)::IndexOutput* proxOut);

  void close();

  /** Returns a free (idle) ThreadState that may be used for
   * indexing this one document.  This call also pauses if a
   * flush is pending.  If delTerm is non-null then we
   * buffer this deleted term after the thread state has
   * been acquired. */
  ThreadState* getThreadState(CL_NS(document)::Document* doc, Term* delTerm);

  /** Returns true if the caller (IndexWriter) should now
   * flush. */
  bool addDocument(CL_NS(document)::Document* doc, CL_NS(analysis)::Analyzer* analyzer);

  bool updateDocument(Term* t, CL_NS(document)::Document* doc, CL_NS(analysis)::Analyzer* analyzer);

  bool updateDocument(CL_NS(document)::Document* doc, CL_NS(analysis)::Analyzer* analyzer, Term* delTerm);

  int32_t getNumBufferedDeleteTerms();

  const TermNumMapType& getBufferedDeleteTerms();

  const std::vector<int32_t>* getBufferedDeleteDocIDs();

  // Reset buffered deletes.
  void clearBufferedDeletes();

  bool bufferDeleteTerms(const CL_NS(util)::ArrayBase<Term*>* terms);

  bool bufferDeleteTerm(Term* term);

  void setMaxBufferedDeleteTerms(int32_t maxBufferedDeleteTerms);

  int32_t getMaxBufferedDeleteTerms();

  bool hasDeletes();

  int64_t getRAMUsed();

  int64_t numBytesAlloc;
  int64_t numBytesUsed;

  /* Used only when writing norms to fill in default norm
   * value into the holes in docID stream for those docs
   * that didn't have this field. */
  static void fillBytes(CL_NS(store)::IndexOutput* out, uint8_t b, int32_t numBytes);

  uint8_t* copyByteBuffer;

  /** Copy numBytes from srcIn to destIn */
  void copyBytes(CL_NS(store)::IndexInput* srcIn, CL_NS(store)::IndexOutput* destIn, int64_t numBytes);


  // Size of each slice.  These arrays should be at most 16
  // elements.  First array is just a compact way to encode
  // X+1 with a max.  Second array is the length of each
  // slice, ie first slice is 5 bytes, next slice is 14
  // bytes, etc.
  static const int32_t nextLevelArray[10];
  static const int32_t levelSizeArray[10];

  // Why + 5*POINTER_NUM_BYTE below?
  //   1: Posting has "vector" field which is a pointer
  //   2: Posting is referenced by postingsFreeList array
  //   3,4,5: Posting is referenced by postings hash, which
  //          targets 25-50% fill factor; approximate this
  //          as 3X # pointers
  //TODO: estimate this accurately for C++!
  static const int32_t POSTING_NUM_BYTE; /// = OBJECT_HEADER_BYTES + 9*INT_NUM_BYTE + 5*POINTER_NUM_BYTE;

  /* Allocate more Postings from shared pool */
  void getPostings(CL_NS(util)::ValueArray<Posting*>& postings);
  void recyclePostings(CL_NS(util)::ValueArray<Posting*>& postings, int32_t numPostings);

  /* Initial chunks size of the shared uint8_t[] blocks used to
     store postings data */
  static const int32_t BYTE_BLOCK_SHIFT;
  static const int32_t BYTE_BLOCK_SIZE;
  static const int32_t BYTE_BLOCK_MASK;
  static const int32_t BYTE_BLOCK_NOT_MASK;

  /* Allocate another uint8_t[] from the shared pool */
  uint8_t* getByteBlock(bool trackAllocations);

  /* Return a uint8_t[] to the pool */
  void recycleBlocks(CL_NS(util)::ArrayBase<uint8_t*>& blocks, int32_t start, int32_t end);

  /* Initial chunk size of the shared char[] blocks used to
     store term text */
  static const int32_t CHAR_BLOCK_SHIFT;
  static const int32_t CHAR_BLOCK_SIZE;
  static const int32_t CHAR_BLOCK_MASK;

  static const int32_t MAX_TERM_LENGTH;

  /* Allocate another char[] from the shared pool */
  TCHAR* getCharBlock();

  /* Return a char[] to the pool */
  void recycleBlocks(CL_NS(util)::ArrayBase<TCHAR*>& blocks, int32_t start, int32_t numBlocks);

  std::string toMB(int64_t v);


};

#define CLUCENE_END_OF_WORD 0x0

CL_NS_END
#endif
