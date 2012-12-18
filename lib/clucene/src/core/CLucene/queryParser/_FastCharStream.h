/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_queryParser_FastCharStream_
#define _lucene_queryParser_FastCharStream_

CL_CLASS_DEF(util,Reader)

CL_NS_DEF(queryParser)

class CharStream;

/** An efficient implementation of JavaCC's CharStream interface.  <p>Note that
 * this does not do line-number counting, but instead keeps track of the
 * character position of the token in the input, as required by Lucene's {@link
 * org.apache.lucene.analysis.Token} API. */ 
class FastCharStream : public CharStream {
protected:
	TCHAR* buffer;
	size_t _bufferSize; // To keep track of the buffer size in memory

	int32_t bufferLength;				  // end of valid chars
	int32_t bufferPosition;				  // next char to read

	int32_t tokenStart;					  // offset in buffer
	int32_t bufferStart;				  // position in file of buffer

	CL_NS(util)::Reader* input;			  // source of chars
	bool _ownsReader;	// Should we delete the reader once done with it, or in destructor?

  
public:
	/** Constructs from a Reader. */
	FastCharStream(CL_NS(util)::Reader* r, bool ownsReader = false);
	virtual ~FastCharStream();

	TCHAR readChar();

private:
	void refill();

public:
	void backup(const int32_t amount);

	/*@memory Caller is responsible for deleting the returned string */
	TCHAR* GetImage();

	/*@memory Caller is responsible for deleting the returned string */
	TCHAR* GetSuffix(const int32_t len);

	void Done();

	TCHAR BeginToken();

	int32_t getColumn() const;
	int32_t getLine() const;
	int32_t getEndColumn() const;
	int32_t getEndLine() const;
	int32_t getBeginColumn() const;
	int32_t getBeginLine() const;
};
CL_NS_END
#endif
