/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_SkipListReader_
#define _lucene_index_SkipListReader_

#include "CLucene/store/IndexInput.h"
#include "CLucene/util/Array.h"

CL_NS_DEF(index)

/**
 * This abstract class reads skip lists with multiple levels.
 *
 * See {@link MultiLevelSkipListWriter} for the information about the encoding
 * of the multi level skip lists.
 *
 * Subclasses must implement the abstract method {@link #readSkipData(int, IndexInput)}
 * which defines the actual format of the skip data.
 */
class MultiLevelSkipListReader : LUCENE_BASE {
protected:
	// number of levels in this skip list
	int32_t numberOfSkipLevels;

	// the maximum number of skip levels possible for this index
	int32_t maxNumberOfSkipLevels;
private:
	// Expert: defines the number of top skip levels to buffer in memory.
	// Reducing this number results in less memory usage, but possibly
	// slower performance due to more random I/Os.
	// Please notice that the space each level occupies is limited by
	// the skipInterval. The top level can not contain more than
	// skipLevel entries, the second top level can not contain more
	// than skipLevel^2 entries and so forth.
	int32_t numberOfLevelsToBuffer;

	int32_t docCount;
	bool haveSkipped;

	CL_NS(util)::ObjectArray<CL_NS(store)::IndexInput> skipStream;		// skipStream for each level
	int64_t* skipPointer;			// the start pointer of each skip level
	int32_t* skipInterval;         // skipInterval of each level
	int32_t* numSkipped;				// number of docs skipped per level

	int32_t* skipDoc;					// doc id of current skip entry per level
	int32_t lastDoc;                // doc id of last read skip entry with docId <= target
	int64_t* childPointer;			// child pointer of current skip entry per level
	int64_t lastChildPointer;		// childPointer of last read skip entry with docId <= target

	bool inputIsBuffered;

public:
  /**
  * @memory consumes _skipStream
  */
	MultiLevelSkipListReader(CL_NS(store)::IndexInput* _skipStream, const int32_t maxSkipLevels, const int32_t _skipInterval);
	virtual ~MultiLevelSkipListReader();

	/** Returns the id of the doc to which the last call of {@link #skipTo(int)}
	*  has skipped.  */
	int32_t getDoc() const;

	/** Skips entries to the first beyond the current whose document number is
	*  greater than or equal to <i>target</i>. Returns the current doc count.
	*/
	int32_t skipTo(const int32_t target);

private:
	bool loadNextSkip(const int32_t level);

protected:
	/** Seeks the skip entry on the given level */
	virtual void seekChild(const int32_t level);

	void close();

	/** initializes the reader */
	void init(const int64_t _skipPointer, const int32_t df);

private:
	/** Loads the skip levels  */
	void loadSkipLevels();

protected:
	/**
	* Subclasses must implement the actual skip data encoding in this method.
	*
	* @param level the level skip data shall be read from
	* @param skipStream the skip stream to read from
	*/
	virtual int32_t readSkipData(const int32_t level, CL_NS(store)::IndexInput* skipStream) = 0;

	/** Copies the values of the last read skip entry on this level */
	virtual void setLastSkipData(const int32_t level);

protected:
	/** used to buffer the top skip levels */
	class SkipBuffer : public CL_NS(store)::IndexInput {
	private:
		uint8_t* data;
		int64_t pointer;
		int32_t pos;
		size_t _datalength;

	public:
		SkipBuffer(CL_NS(store)::IndexInput* input, const int32_t length);
		virtual ~SkipBuffer();

	private:
		void close();

		int64_t getFilePointer() const;

		int64_t length() const;

		uint8_t readByte();

		/* Make sure b is passed after the offset has been calculated into it, if necessary! */
		void readBytes(uint8_t* b, const int32_t len);

		void seek(const int64_t _pos);

		SkipBuffer(const SkipBuffer& other);
		CL_NS(store)::IndexInput* clone() const;

		const char* getDirectoryType() const;
		const char* getObjectName() const;
		static const char* getClassName();
	};
};


/**
 * Implements the skip list reader for the default posting list format
 * that stores positions and payloads.
 *
 */
class DefaultSkipListReader: public MultiLevelSkipListReader {
private:
	bool currentFieldStoresPayloads;
	int64_t* freqPointer;
	int64_t* proxPointer;
	int32_t* payloadLength;

	int64_t lastFreqPointer;
	int64_t lastProxPointer;
	int32_t lastPayloadLength;

public:
	DefaultSkipListReader(CL_NS(store)::IndexInput* _skipStream, const int32_t maxSkipLevels, const int32_t _skipInterval);
	virtual ~DefaultSkipListReader();

	void init(const int64_t _skipPointer, const int64_t freqBasePointer, const int64_t proxBasePointer, const int32_t df, const bool storesPayloads);

	/** Returns the freq pointer of the doc to which the last call of
	* {@link MultiLevelSkipListReader#skipTo(int)} has skipped.  */
	int64_t getFreqPointer() const;

	/** Returns the prox pointer of the doc to which the last call of
	* {@link MultiLevelSkipListReader#skipTo(int)} has skipped.  */
	int64_t getProxPointer() const;

	/** Returns the payload length of the payload stored just before
	* the doc to which the last call of {@link MultiLevelSkipListReader#skipTo(int)}
	* has skipped.  */
	int32_t getPayloadLength() const;

protected:
	void seekChild(const int32_t level);

	void setLastSkipData(const int32_t level);

	int32_t readSkipData(const int32_t level, CL_NS(store)::IndexInput* _skipStream);
};

CL_NS_END
#endif
