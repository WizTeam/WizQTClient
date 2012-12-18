/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_SegmentTermEnum_
#define _lucene_index_SegmentTermEnum_


//#include "Terms.h"
//#include "FieldInfos.h"
//#include "TermInfo.h"

CL_NS_DEF(index)

/**
 * SegmentTermEnum is an enumeration of all Terms and TermInfos
 */
class SegmentTermEnum:public TermEnum{
private:
	Term* _term;            ///points to the current Term in the enumeration
	TermInfo* termInfo;     ///points to the TermInfo matching the current Term in the enumeration

	bool isIndex;           ///Indicates if the Segment is a an index
	bool isClone;           ///Indicates if SegmentTermEnum is an orignal instance or
	                        ///a clone of another SegmentTermEnum

	TCHAR* buffer;			///The buffer that contains the data read from the Term Infos File
	uint32_t bufferLength;	///Length of the buffer

	int32_t format;
	int32_t formatM1SkipInterval;

	CL_NS(store)::IndexInput* input;    ///The IndexInput that reads from the Term Infos File
	FieldInfos* fieldInfos;	///contains the Field Infos for the segment
	int64_t size;			///The size of the enumeration
	int64_t position;		///The position of the current (term) in the enumeration
	int64_t indexPointer;
	Term* prev;				///The previous current
	int32_t indexInterval;
	int32_t skipInterval;
	int32_t maxSkipLevels;

	friend class TermInfosReader;
	friend class SegmentTermDocs;
protected:

	/**
	 * Constructor.
	 * The instance is created by cloning all properties of clone
	 */
	SegmentTermEnum( const SegmentTermEnum& clone);

public:
	///Constructor
	SegmentTermEnum(CL_NS(store)::IndexInput* i, FieldInfos* fis, const bool isi );

	///Destructor
	~SegmentTermEnum();

	/**
	 * Moves the current of the set to the next in the set
	 */
	bool next();

	/**
	 * Returns the current term. 
	 */
	Term* term(bool pointer=true);

    /**
	 * Scan for Term term without allocating new Terms
	 */
	void scanTo(const Term *term);

	/**
	 * Closes the enumeration to further activity, freeing resources.
	 */
	void close();

	/**
	 * Returns the document frequency of the current term in the set
	 */
	int32_t docFreq() const;

	/**
	 * Repositions term and termInfo within the enumeration
	 */
	void seek(const int64_t pointer, const int32_t p, Term* t, TermInfo* ti);
	
	/**
	 * Returns a clone of the current termInfo
	 */
	TermInfo* getTermInfo()const;

	/**
	 * Retrieves a clone of termInfo through the reference ti
	 */
	void getTermInfo(TermInfo* ti)const;

	/**
	 * Returns the freqPointer from the current TermInfo in the enumeration.
	 */
	int64_t freqPointer() const;

	/**
	 * Returns the proxPointer from the current TermInfo in the enumeration.
	 */
	int64_t proxPointer() const;

    /**
	 * Returns a clone of this instance
	 */
	SegmentTermEnum* clone() const;

	const char* getObjectName() const;
	static const char* getClassName();

private:
	/**
	 * Reads the next term in the enumeration
	 */
	Term* readTerm(Term* reuse);
   /** 
	 * Instantiate a buffer of length length+1
   * TODO: deprecate this...
	 */
	void growBuffer(const uint32_t length, bool force_copy);

};
CL_NS_END
#endif
