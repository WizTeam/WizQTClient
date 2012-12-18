/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_analysis_AnalysisHeader_
#define _lucene_analysis_AnalysisHeader_

#include "CLucene/index/Payload.h"
#include "CLucene/util/VoidList.h"
#include "CLucene/LuceneThreads.h"

CL_CLASS_DEF(util,Reader)
CL_NS_DEF(analysis)

typedef CL_NS(util)::CLSetList<TCHAR*, CL_NS(util)::Compare::TChar, CL_NS(util)::Deletor::tcArray> CLTCSetList;

/** A Token is an occurence of a term from the text of a field.  It consists of
  a term's text, the start and end offset of the term in the text of the field,
  and a type string.
  <p>
  The start and end offsets permit applications to re-associate a token with
  its source text, e.g., to display highlighted query terms in a document
  browser, or to show matching text fragments in a KWIC (KeyWord In Context)
  display, etc.
  <p>
  The type is an interned string, assigned by a lexical analyzer
  (a.k.a. tokenizer), naming the lexical or syntactic class that the token
  belongs to.  For example an end of sentence marker token might be implemented
  with type "eos".  The default token type is "word".
  <p>
  A Token can optionally have metadata (a.k.a. Payload) in the form of a variable
  length byte array. Use {@link lucene::index::TermPositions#getPayloadLength()} and
  {@link lucene::index::TermPositions#getPayload(byte[], int)} to retrieve the payloads from the index.

  <br><br>
  <p><font color="#FF0000">
  WARNING: The status of the <b>Payloads</b> feature is experimental.
  The APIs introduced here might change in the future and will not be
  supported anymore in such a case.</font>

  <br><br>

  <p>Tokenizers and filters should try to re-use a Token
  instance when possible for best performance, by
  implementing the {@link lucene::index::TokenStream#next(Token)} API.
  Failing that, to create a new Token you should first use
  one of the constructors that starts with null text.  Then
  you should call either {@link #termBuffer()} or {@link
  #resizeTermBuffer(int)} to retrieve the Token's
  termBuffer.  Fill in the characters of your term into this
  buffer, and finally call {@link #setTermLength(int)} to
  set the length of the term text.  See <a target="_top"
  href="https://issues.apache.org/jira/browse/LUCENE-969">LUCENE-969</a>
  for details.</p>

  @see Payload
*/
class CLUCENE_EXPORT Token:LUCENE_BASE{
private:
	int32_t _startOffset;				///< start in source text
	int32_t _endOffset;				  ///< end in source text
	const TCHAR* _type;				  ///< lexical type
	int32_t positionIncrement;
	size_t bufferTextLen;

	#ifndef LUCENE_TOKEN_WORD_LENGTH
	TCHAR* _buffer;				  ///< the text of the term
	#else
	TCHAR _buffer[LUCENE_TOKEN_WORD_LENGTH+1];				  ///< the text of the term
	#endif
	int32_t _termTextLen;                                         ///< the length of termText. Internal use only

	CL_NS(index)::Payload* payload;

public:
	static const TCHAR* getDefaultType();

	Token();
	virtual ~Token();

	/// Constructs a Token with the given text, start and end offsets, & type. 
	Token(const TCHAR* text, const int32_t start, const int32_t end, const TCHAR* typ=NULL);
	void set(const TCHAR* text, const int32_t start, const int32_t end, const TCHAR* typ=NULL);

	size_t bufferLength();
	void growBuffer(size_t size);
  TCHAR* resizeTermBuffer(size_t size);
	/** Set the position increment.  This determines the position of this
	* token relative to the previous Token in a TokenStream, used in
	* phrase searching.
	*
	* The default value is 1.
	*
	* Some common uses for this are:
	*
	* - Set it to zero to put multiple terms in the same position.  This is
	* useful if, e.g., a word has multiple stems.  Searches for phrases
	* including either stem will match.  In this case, all but the first stem's
	* increment should be set to zero: the increment of the first instance
	* should be one.  Repeating a token with an increment of zero can also be
	* used to boost the scores of matches on that token.
	*
	* - Set it to values greater than one to inhibit exact phrase matches.
	* If, for example, one does not want phrases to match across removed stop
	* words, then one could build a stop word filter that removes stop words and
	* also sets the increment to the number of stop words removed before each
	* non-stop word.  Then exact phrase queries will only match when the terms
	* occur with no intervening stop words.
	*/
	void setPositionIncrement(int32_t posIncr);
	int32_t getPositionIncrement() const;

	/** Returns the internal termBuffer character array which
	*  you can then directly alter.  If the array is too
	*  small for your token, use {@link
	*  #resizeTermBuffer(int)} to increase it.  After
	*  altering the buffer be sure to call {@link
	*  #setTermLength} to record the number of valid
	*  characters that were placed into the termBuffer. */
	TCHAR* termBuffer() const;
	size_t termLength(); //< Length of the the termBuffer. See #termBuffer

	_CL_DEPRECATED( termBuffer ) const TCHAR* termText() const; //< See #termBuffer()
	_CL_DEPRECATED( termLength ) size_t termTextLength(); //< See #termLength

	void resetTermTextLen(); //< Empties the termBuffer. See #termBuffer
	void setText(const TCHAR* txt, int32_t len=-1); //< Sets the termBuffer. See #termBuffer

	/**
	* Returns this Token's starting offset, the position of the first character
	* corresponding to this token in the source text.
	*
	* Note that the difference between endOffset() and startOffset() may not be
	* equal to termText.length(), as the term text may have been altered by a
	* stemmer or some other filter.
	*/
	int32_t startOffset() const;

	/** Set the starting offset.
	@see #startOffset() */
	void setStartOffset(const int32_t val);

  void setTermLength(int32_t);

	/**
	* Returns this Token's ending offset, one greater than the position of the
	* last character corresponding to this token in the source text.
	*/
	int32_t endOffset() const;

	/** Set the ending offset.
	@see #endOffset() */
	void setEndOffset(const int32_t val);

	/// Returns this Token's lexical type.  Defaults to "word".
	const TCHAR* type() const; ///<returns reference
	void setType(const TCHAR* val); ///<returns reference

	/**
	* Returns this Token's payload.
	*/
	CL_NS(index)::Payload* getPayload();

	/**
	* Sets this Token's payload.
	*/
	void setPayload(CL_NS(index)::Payload* payload);

	/** Resets the term text, payload, and positionIncrement to default.
	* Other fields such as startOffset, endOffset and the token type are
	* not reset since they are normally overwritten by the tokenizer. */
	void clear();

	TCHAR* toString() const;
};

/** A TokenStream enumerates the sequence of tokens, either from
fields of a document or from query text.
<p>
This is an abstract class.  Concrete subclasses are:
<ul>
<li>{@link Tokenizer}, a TokenStream
whose input is a Reader; and
<li>{@link TokenFilter}, a TokenStream
whose input is another TokenStream.
</ul>
NOTE: subclasses must override at least one of {@link
#next()} or {@link #next(Token)}.
*/
class CLUCENE_EXPORT TokenStream {
public:
	/** Returns the next token in the stream, or null at EOS.
	*  When possible, the input Token should be used as the
	*  returned Token (this gives fastest tokenization
	*  performance), but this is not required and a new Token
	*  may be returned (pass NULL for this).
	*  Callers may re-use a single Token instance for successive
	*  calls to this method.
	*  <p>
	*  This implicitly defines a "contract" between
	*  consumers (callers of this method) and
	*  producers (implementations of this method
	*  that are the source for tokens):
	*  <ul>
	*   <li>A consumer must fully consume the previously
	*       returned Token before calling this method again.</li>
	*   <li>A producer must call {@link Token#clear()}
	*       before setting the fields in it & returning it</li>
	*  </ul>
	*  Note that a {@link TokenFilter} is considered a consumer.
	*  @param result a Token that may or may not be used to return
	*  @return next token in the stream or null if end-of-stream was hit
	*/
	virtual Token* next(Token* token) = 0;

	/** This is for backwards compatibility only. You should pass the token you want to fill
	 * to next(), this will save a lot of object construction and destructions.
	 *  @deprecated. use next(token). Kept only to avoid breaking existing code.
	 */
	_CL_DEPRECATED(next(Token)) Token* next();

    /** Releases resources associated with this stream. */
	virtual void close() = 0;

  /** Resets this stream to the beginning. This is an
   *  optional operation, so subclasses may or may not
   *  implement this method. Reset() is not needed for
   *  the standard indexing process. However, if the Tokens
   *  of a TokenStream are intended to be consumed more than
   *  once, it is necessary to implement reset().
   */
  virtual void reset();

	virtual ~TokenStream();
};


/** An Analyzer builds TokenStreams, which analyze text.  It thus represents a
 *  policy for extracting index terms from text.
 *  <p>
 *  Typical implementations first build a Tokenizer, which breaks the stream of
 *  characters from the Reader into raw Tokens.  One or more TokenFilters may
 *  then be applied to the output of the Tokenizer.
 *  <p>
 *  WARNING: You must override one of the methods defined by this class in your
 *  subclass or the Analyzer will enter an infinite loop.
 */
class CLUCENE_EXPORT Analyzer{
public:
	Analyzer();

	/** Creates a TokenStream which tokenizes all the text in the provided
	Reader.  Default implementation forwards to tokenStream(Reader) for
	compatibility with older version.  Override to allow Analyzer to choose
	strategy based on document and/or field.  Must be able to handle null
	field name for backward compatibility. */
	virtual TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader)=0;

	/** Creates a TokenStream that is allowed to be re-used
	*  from the previous time that the same thread called
	*  this method.  Callers that do not need to use more
	*  than one TokenStream at the same time from this
	*  analyzer should use this method for better
	*  performance.
	*/
	virtual TokenStream* reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
private:

	DEFINE_MUTEX(THIS_LOCK)
	struct Internal;
	Internal* _internal;
protected:

	/** Used by Analyzers that implement reusableTokenStream
	*  to retrieve previously saved TokenStreams for re-use
	*  by the same thread. */
	TokenStream* getPreviousTokenStream();

	/** Used by Analyzers that implement reusableTokenStream
	*  to save a TokenStream for later re-use by the same
	*  thread. */
	void setPreviousTokenStream(TokenStream* obj);
public:
	/**
	* Invoked before indexing a Field instance if
	* terms have already been added to that field.  This allows custom
	* analyzers to place an automatic position increment gap between
	* Field instances using the same field name.  The default value
	* position increment gap is 0.  With a 0 position increment gap and
	* the typical default token position increment of 1, all terms in a field,
	* including across Field instances, are in successive positions, allowing
	* exact PhraseQuery matches, for instance, across Field instance boundaries.
	*
	* @param fieldName Field name being indexed.
	* @return position increment gap, added to the next token emitted from {@link #tokenStream(TCHAR*, Reader*)}
	*/
	virtual int32_t getPositionIncrementGap(const TCHAR* fieldName);

	virtual ~Analyzer();
};


/** A Tokenizer is a TokenStream whose input is a Reader.
<p>
This is an abstract class.
<p>
NOTE: subclasses must override at least one of {@link
#next()} or {@link #next(Token)}.
<p>
NOTE: subclasses overriding {@link #next(Token)} must
call {@link Token#clear()}.
*/
class CLUCENE_EXPORT Tokenizer:public TokenStream {
protected:
    /** The text source for this Tokenizer. */
	CL_NS(util)::Reader* input;

public:
    /** Construct a tokenizer with null input. */
	Tokenizer();
    /** Construct a token stream processing the given input. */
    Tokenizer(CL_NS(util)::Reader* _input);

    /** By default, closes the input Reader. */
	virtual void close();

	/** Expert: Reset the tokenizer to a new reader.  Typically, an
	*  analyzer (in its reusableTokenStream method) will use
	*  this to re-use a previously created tokenizer. */
	virtual void reset(CL_NS(util)::Reader* _input);

	virtual ~Tokenizer();
};

/** A TokenFilter is a TokenStream whose input is another token stream.
<p>
This is an abstract class.
*/
class CLUCENE_EXPORT TokenFilter:public TokenStream {
protected:
    /** The source of tokens for this filter. */
	TokenStream* input;
    /** If true then input will be deleted in the destructor */
	bool deleteTokenStream;

    /** Construct a token stream filtering the given input.
     *
     * @param in The TokenStream to filter from
     * @param deleteTS If true, input will be deleted in the destructor
    */
	TokenFilter(TokenStream* in, bool deleteTS=false);
	virtual ~TokenFilter();
public:
    /** Close the input TokenStream. */
	void close();
};

CL_NS_END
#endif
