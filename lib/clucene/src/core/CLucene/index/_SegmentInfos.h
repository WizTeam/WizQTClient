/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_SegmentInfos_
#define _lucene_index_SegmentInfos_


//#include "IndexReader.h"
#include "CLucene/util/Misc.h"
#include "_IndexFileNames.h"
CL_CLASS_DEF(store,Directory)
CL_CLASS_DEF(store,IndexInput)
CL_CLASS_DEF(store,IndexOutput)

CL_NS_DEF(index)

	class SegmentInfo :LUCENE_BASE{
	public:

		LUCENE_STATIC_CONSTANT(int32_t, NO = -1);			// e.g. no norms; no deletes;
		LUCENE_STATIC_CONSTANT(int32_t, YES = 1);			// e.g. have norms; have deletes;
		LUCENE_STATIC_CONSTANT(int32_t, CHECK_DIR = 0);		// e.g. must check dir to see if there are norms/deletions
		LUCENE_STATIC_CONSTANT(int32_t, WITHOUT_GEN = 0);	// a file name that has no GEN in it.

    std::string name;									// unique name in dir
		int32_t docCount;							// number of docs in seg
		CL_NS(store)::Directory* dir;				// where segment resides

	private:
		bool preLockless;						  // true if this is a segments file written before
                                                  // lock-less commits (2.1)

		int64_t delGen;                            // current generation of del file; NO if there
                                                  // are no deletes; CHECK_DIR if it's a pre-2.1 segment
                                                  // (and we must check filesystem); YES or higher if
                                                  // there are deletes at generation N

    CL_NS(util)::ValueArray<int64_t> normGen;     // current generation of each field's norm file.
                                                  // If this array is null, for lockLess this means no
                                                  // separate norms.  For preLockLess this means we must
                                                  // check filesystem. If this array is not null, its
                                                  // values mean: NO says this field has no separate
                                                  // norms; CHECK_DIR says it is a preLockLess segment and
                                                  // filesystem must be checked; >= YES says this field
                                                  // has separate norms with the specified generation

		int8_t isCompoundFile;					  // NO if it is not; YES if it is; CHECK_DIR if it's
                                                  // pre-2.1 (ie, must check file system to see
                                                  // if <name>.cfs and <name>.nrm exist)

		bool hasSingleNormFile;					  // true if this segment maintains norms in a single file;
                                                  // false otherwise
                                                  // this is currently false for segments populated by DocumentWriter
                                                  // and true for newly created merged segments (both
                                                  // compound and non compound).

	private:

    std::vector<std::string> _files;                               // cached list of files that this segment uses
                                                  // in the Directory

		int64_t _sizeInBytes;					  // total byte size of all of our files (computed on demand)

		int32_t docStoreOffset;					  // if this segment shares stored fields & vectors, this
                                                  // offset is where in that file this segment's docs begin
    std::string docStoreSegment;					  // name used to derive fields/vectors file we share with
                                                  // other segments
												  // This string is being interned. There might be a way around this,
												  // and if found, this would greatly improve perfomance.

		bool docStoreIsCompoundFile;			  // whether doc store files are stored in compound file (*.cfx)

		/* Called whenever any change is made that affects which
		* files this segment has. */
		void clearFiles();

    void addIfExists(std::vector<std::string>& files, const std::string& fileName);

	public:
		SegmentInfo(const char* _name, const int32_t _docCount, CL_NS(store)::Directory* _dir,
			bool _isCompoundFile=SegmentInfo::CHECK_DIR,
      bool _hasSingleNormFile=false,
			int32_t _docStoreOffset = -1,
      const char* _docStoreSegment = NULL,
      bool _docStoreIsCompoundFile = false);

		/**
		* Construct a new SegmentInfo instance by reading a
		* previously saved SegmentInfo from input.
		*
		* @param dir directory to load from
		* @param format format of the segments info file
		* @param input input handle to read segment info from
		*/
		SegmentInfo(CL_NS(store)::Directory* dir, int32_t format, CL_NS(store)::IndexInput* input);

		~SegmentInfo();

		void setNumFields(const int32_t numFields);
    int64_t sizeInBytes();
		bool hasDeletions() const;

		void advanceDelGen();
		void clearDelGen();

		SegmentInfo* clone ();

    std::string getDelFileName() const;

		/**
		* Returns true if this field for this segment has saved a separate norms file (_<segment>_N.sX).
		*
		* @param fieldNumber the field index to check
		*/
		bool hasSeparateNorms(const int32_t fieldNumber) const;

		/**
		* Returns true if any fields in this segment have separate norms.
		*/
		bool hasSeparateNorms() const;

		/**
		* Get the file name for the norms file for this field.
		*
		* @param number field index
		*/
    std::string getNormFileName(const int32_t number) const;

		/**
		* Increment the generation count for the norms file for
		* this field.
		*
		* @param fieldIndex field whose norm file will be rewritten
		*/
		void advanceNormGen(const int32_t fieldIndex);

		/**
		* Mark whether this segment is stored as a compound file.
		*
		* @param isCompoundFile true if this is a compound file;
		* else, false
		*/
		void setUseCompoundFile(const bool isCompoundFile);

		/**
		* Returns true if this segment is stored as a compound
		* file; else, false.
		*/
		bool getUseCompoundFile() const;

    /*
    * Return all files referenced by this SegmentInfo.  The
    * returns List is a locally cached List so you should not
    * modify it.
    */
    const std::vector<std::string>& files();

		/**
		* Copy everything from src SegmentInfo into our instance.
		*/
		void reset(const SegmentInfo* src);

		/**
		* Save this segment's info.
		*/
		void write(CL_NS(store)::IndexOutput* output);

		int32_t getDocStoreOffset() const;

		bool getDocStoreIsCompoundFile() const;

		void setDocStoreIsCompoundFile(const bool v);

		/**
		* Returns a reference to docStoreSegment
		*/
    const std::string& getDocStoreSegment() const;

		void setDocStoreOffset(const int32_t offset);

		/** We consider another SegmentInfo instance equal if it
		*  has the same dir and same name. */
		bool equals(const SegmentInfo* obj);

		///Gets the Directory where the segment resides
		CL_NS(store)::Directory* getDir() const{ return dir; } //todo: since dir is public, consider removing this function

	    friend class SegmentReader;

	    /** Used for debugging */
	    std::string segString(CL_NS(store)::Directory* dir);
	};

	typedef CL_NS(util)::CLVector<SegmentInfo*,CL_NS(util)::Deletor::Object<SegmentInfo> > segmentInfosType;
  //SegmentInfos manages a list of SegmentInfo instances
  //Each SegmentInfo contains information about a segment in a directory.
  //
  //The active segments in the index are stored in the segment info file.
  //An index only has a single file in this format, and it is named "segments".
  //This lists each segment by name, and also contains the size of each segment.
  //The format of the file segments is defined as follows:
  //
  //                                        SegCount
  //Segments --> SegCount, <SegName, SegSize>
  //
  //SegCount, SegSize --> UInt32
  //
  //SegName --> String
  //
  //SegName is the name of the segment, and is used as the file name prefix
  //for all of the files that compose the segment's index.
  //
  //SegSize is the number of documents contained in the segment index.
  //
  //Note:
  //At http://jakarta.apache.org/lucene/docs/fileformats.html the definition
  //of all file formats can be found. Note that java lucene currently
  //defines Segments as follows:
  //
  //Segments --> Format, Version, SegCount, <SegName, SegSize>SegCount
  //
  //Format, SegCount, SegSize --> UInt32
  //
  //Format and Version have not been implemented yet

	class IndexReader;

	class SegmentInfos: LUCENE_BASE {
	public:
	  DEFINE_MUTEX(THIS_LOCK)

		/** The file format version, a negative number. */
		/* Works since counter, the old 1st entry, is always >= 0 */
		LUCENE_STATIC_CONSTANT(int32_t,FORMAT=-1);

		/** This format adds details used for lockless commits.  It differs
		* slightly from the previous format in that file names
		* are never re-used (write once).  Instead, each file is
		* written to the next generation.  For example,
		* segments_1, segments_2, etc.  This allows us to not use
		* a commit lock.  See <a
		* href="http://lucene.apache.org/java/docs/fileformats.html">file
		* formats</a> for details.
		*/
		LUCENE_STATIC_CONSTANT(int32_t,FORMAT_LOCKLESS=-2);

		/** This format adds a "hasSingleNormFile" flag into each segment info.
		* See <a href="http://issues.apache.org/jira/browse/LUCENE-756">LUCENE-756</a>
		* for details.
		*/
		LUCENE_STATIC_CONSTANT(int32_t,FORMAT_SINGLE_NORM_FILE=-3);

		/** This format allows multiple segments to share a single
		* vectors and stored fields file. */
		LUCENE_STATIC_CONSTANT(int32_t,FORMAT_SHARED_DOC_STORE=-4);

	private:
		/* This must always point to the most recent file format. */
		LUCENE_STATIC_CONSTANT(int32_t,CURRENT_FORMAT=FORMAT_SHARED_DOC_STORE);

	public:
		int32_t counter;  // used to name new segments

		/**
		* counts how often the index has been changed by adding or deleting docs.
		* starting with the current time in milliseconds forces to create unique version numbers.
		*/
		int64_t version;

	private:
		int64_t generation;					// generation of the "segments_N" for the next commit
		int64_t lastGeneration;				// generation of the "segments_N" file we last successfully read
											// or wrote; this is normally the same as generation except if
											// there was an IOException that had interrupted a commit

		/**
		* If non-null, information about loading segments_N files
		* will be printed here.  @see #setInfoStream.
		*/
		static std::ostream* infoStream;

		LUCENE_STATIC_CONSTANT(int32_t,defaultGenFileRetryCount=10);
		LUCENE_STATIC_CONSTANT(int32_t,defaultGenFileRetryPauseMsec=50);
		LUCENE_STATIC_CONSTANT(int32_t,defaultGenLookaheadCount=10);

		segmentInfosType infos;

		friend class IndexWriter; //allow IndexWriter to use counter

    static void message(const char* _message, ...);

  public:
      SegmentInfos(bool deleteMembers=true, int32_t reserveCount=0);
      ~SegmentInfos();

		//Returns a reference to the i-th SegmentInfo in the list.
		SegmentInfo* info(int32_t i) const;

		/**
		* Get the generation (N) of the current segments_N file
		* from a list of files.
		*
		* @param files -- array of file names to check
		*/
    static int64_t getCurrentSegmentGeneration( std::vector<std::string>& files );

		/**
		* Get the generation (N) of the current segments_N file
		* in the directory.
		*
		* @param directory -- directory to search for the latest segments_N file
		*/
		static int64_t getCurrentSegmentGeneration( const CL_NS(store)::Directory* directory );

		/**
		* Get the filename of the current segments_N file
		* from a list of files.
		*
		* @param files -- array of file names to check
		*/
    static std::string getCurrentSegmentFileName( std::vector<std::string>& files );

		/**
		* Get the filename of the current segments_N file
		* in the directory.
		*
		* @param directory -- directory to search for the latest segments_N file
		*/
		static std::string getCurrentSegmentFileName( CL_NS(store)::Directory* directory );

		/**
		* Get the segments_N filename in use by this segment infos.
		*/
		std::string getCurrentSegmentFileName();

		/**
		* Parse the generation off the segments file name and
		* return it.
		*/
		static int64_t generationFromSegmentsFileName( const char* fileName );

		/**
		* Get the next segments_N filename that will be written.
		*/
		std::string getNextSegmentFileName();

		/* public vector-like operations */
		//delete and clears objects 'from' from to 'to'
		void clearto(size_t to, size_t end);
		//count of segment infos
		int32_t size() const;
		/** add a segment info
    * @param pos position to add the info at. -1 for last position
    */
		void add(SegmentInfo* info, int32_t pos=-1);
		SegmentInfo* elementAt(int32_t pos);
		void setElementAt(SegmentInfo* si, int32_t pos);
		void clear();

		void insert(SegmentInfos* infos, bool takeMemory);
		void insert(SegmentInfo* info);
		int32_t indexOf(const SegmentInfo* info) const;
		void range(size_t from, size_t to, SegmentInfos& ret) const;
    void remove(size_t index, bool dontDelete=false);

		/**
		* Read a particular segmentFileName.  Note that this may
		* throw an IOException if a commit is in process.
		*
		* @param directory -- directory containing the segments file
		* @param segmentFileName -- segment file to load
		* @throws CorruptIndexException if the index is corrupt
		* @throws IOException if there is a low-level IO error
		*/
		void read(CL_NS(store)::Directory* directory, const char* segmentFileName);

		/**
		* This version of read uses the retry logic (for lock-less
		* commits) to find the right segments file to load.
		* @throws CorruptIndexException if the index is corrupt
		* @throws IOException if there is a low-level IO error
		*/
		void read(CL_NS(store)::Directory* directory);

		//Writes a new segments file based upon the SegmentInfo instances it manages
		//note: still does not support lock-less writes (still pre-2.1 format)
        void write(CL_NS(store)::Directory* directory);

		/**
		* Returns a copy of this instance, also copying each
		* SegmentInfo.
		*/
		SegmentInfos* clone() const;

		/**
		* version number when this SegmentInfos was generated.
		*/
		int64_t getVersion() const;
		int64_t getGeneration() const;
		int64_t getLastGeneration() const;

		/**
		* Current version number from segments file.
		* @throws CorruptIndexException if the index is corrupt
		* @throws IOException if there is a low-level IO error
		*/
		static int64_t readCurrentVersion(CL_NS(store)::Directory* directory);


    /** If non-null, information about retries when loading
    * the segments file will be printed to this.
    */
    static void setInfoStream(std::ostream* infoStream);

    /**
    * @see #setInfoStream
    */
    static std::ostream* getInfoStream();

		/**
		* Advanced: set how many times to try loading the
		* segments.gen file contents to determine current segment
		* generation.  This file is only referenced when the
		* primary method (listing the directory) fails.
		*/
		//static void setDefaultGenFileRetryCount(const int32_t count);
		/**
		* @see #setDefaultGenFileRetryCount
		*/
		static int32_t getDefaultGenFileRetryCount();

		/**
		* Advanced: set how many milliseconds to pause in between
		* attempts to load the segments.gen file.
		*/
		//static void setDefaultGenFileRetryPauseMsec(const int32_t msec);
		/**
		* @see #setDefaultGenFileRetryPauseMsec
		*/
		static int32_t getDefaultGenFileRetryPauseMsec();

		/**
		* Advanced: set how many times to try incrementing the
		* gen when loading the segments file.  This only runs if
		* the primary (listing directory) and secondary (opening
		* segments.gen file) methods fail to find the segments
		* file.
		*/
		//static void setDefaultGenLookaheadCount(const int32_t count);
		/**
		* @see #setDefaultGenLookaheadCount
		*/
		static int32_t getDefaultGenLookahedCount();

    class _FindSegmentsFile: LUCENE_BASE{
    protected:
      const char* fileDirectory;
      CL_NS(store)::Directory* directory;

      void doRun();
      virtual bool tryDoBody(const char* segmentFileName, CLuceneError& ret_err) = 0;
    };

		/**
		* Utility class for executing code that needs to do
		* something with the current segments file.  This is
		* necessary with lock-less commits because from the time
		* you locate the current segments file name, until you
		* actually open it, read its contents, or check modified
		* time, etc., it could have been deleted due to a writer
		* commit finishing.
		*/
    template<typename RET>
		class FindSegmentsFile: public _FindSegmentsFile{
    protected:
      virtual RET doBody(const char* segmentFileName) = 0;
      RET result;

      //catch only IO errors, return true on success...
      bool tryDoBody(const char* segmentFileName, CLuceneError& ret_err){
        try{
          result = doBody(segmentFileName);
          return true;
        } catch (CLuceneError& err) {
          result = 0;
          ret_err.set(err.number(),err.what());
        }
        return false;
      }
    public:
    		FindSegmentsFile( CL_NS(store)::Directory* dir ){
	        this->directory = dir;
          this->fileDirectory = NULL;
          this->result = 0;
        }
    		FindSegmentsFile( const char* dir ){
	        this->directory = NULL;
          this->fileDirectory = dir;
          this->result = 0;
        }
        ~FindSegmentsFile(){
        }

        RET run(){
          doRun();
          return result;
        };
    	};
    	//friend class SegmentInfos::FindSegmentsFile;

    	class FindSegmentsVersion: public FindSegmentsFile<int64_t> {
    	public:
    		FindSegmentsVersion( CL_NS(store)::Directory* dir );
    		FindSegmentsVersion( const char* dir );
    		int64_t doBody( const char* segmentFileName );
    	};
    	friend class SegmentInfos::FindSegmentsVersion;

		class FindSegmentsRead: public FindSegmentsFile<bool> {
      	  SegmentInfos* _this;
    	public:
		  FindSegmentsRead( CL_NS(store)::Directory* dir, SegmentInfos* _this );
    	  FindSegmentsRead( const char* dir, SegmentInfos* _this );
    	  bool doBody( const char* segmentFileName );
    	};
    	friend class SegmentInfos::FindSegmentsRead;
  };
CL_NS_END
#endif
