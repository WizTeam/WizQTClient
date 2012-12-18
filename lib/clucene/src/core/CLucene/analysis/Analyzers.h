/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_analysis_Analyzers_
#define _lucene_analysis_Analyzers_

#include "CLucene/util/VoidList.h"
#include "CLucene/util/VoidMap.h"
#include "CLucene/util/CLStreams.h"
#include "AnalysisHeader.h"

CL_NS_DEF(analysis)

/** An abstract base class for simple, character-oriented tokenizers.*/
class CLUCENE_EXPORT CharTokenizer:public Tokenizer {
private:
	int32_t offset, bufferIndex, dataLen;
	TCHAR buffer[LUCENE_MAX_WORD_LEN+1];
	const TCHAR* ioBuffer;
protected:
    
    /** Returns true iff a character should be included in a token.  This
    * tokenizer generates as tokens adjacent sequences of characters which
    * satisfy this predicate.  Characters for which this is false are used to
    * define token boundaries and are not included in tokens. */
	virtual bool isTokenChar(const TCHAR c) const = 0;

    /** Called on each token character to normalize it before it is added to the
    * token.  The default implementation does nothing.  Subclasses may use this
    * to, e.g., lowercase tokens. */
   	virtual TCHAR normalize(const TCHAR c) const;

public:
	CharTokenizer(CL_NS(util)::Reader* in);
	Token* next(Token* token);
	void reset(CL_NS(util)::Reader* input);

	virtual ~CharTokenizer();
};


/** A LetterTokenizer is a tokenizer that divides text at non-letters.  That's
to say, it defines tokens as maximal strings of adjacent letters, as defined
by java.lang.Character.isLetter() predicate.

Note: this does a decent job for most European languages, but does a terrible
job for some Asian languages, where words are not separated by spaces. */
class CLUCENE_EXPORT LetterTokenizer:public CharTokenizer {
public:
	// Construct a new LetterTokenizer. 
	LetterTokenizer(CL_NS(util)::Reader* in);
    virtual ~LetterTokenizer();
protected:
    /** Collects only characters which satisfy _istalpha.*/
	bool isTokenChar(const TCHAR c) const;
};



/**
* LowerCaseTokenizer performs the function of LetterTokenizer
* and LowerCaseFilter together.  It divides text at non-letters and converts
* them to lower case.  While it is functionally equivalent to the combination
* of LetterTokenizer and LowerCaseFilter, there is a performance advantage
* to doing the two tasks at once, hence this (redundant) implementation.
* <P>
* Note: this does a decent job for most European languages, but does a terrible
* job for some Asian languages, where words are not separated by spaces.
*/
class CLUCENE_EXPORT LowerCaseTokenizer:public LetterTokenizer {
public:
	/** Construct a new LowerCaseTokenizer. */
	LowerCaseTokenizer(CL_NS(util)::Reader* in);
    virtual ~LowerCaseTokenizer();
protected:
	/** Collects only characters which satisfy _totlower. */
	TCHAR normalize(const TCHAR chr) const;
};


/** A WhitespaceTokenizer is a tokenizer that divides text at whitespace.
 * Adjacent sequences of non-Whitespace characters form tokens. */
class CLUCENE_EXPORT WhitespaceTokenizer: public CharTokenizer {
public:
	/** Construct a new WhitespaceTokenizer. */ 
	WhitespaceTokenizer(CL_NS(util)::Reader* in);
	virtual ~WhitespaceTokenizer();
protected:
	/** Collects only characters which do not satisfy _istspace.*/
	bool isTokenChar(const TCHAR c) const;
};


/** An Analyzer that uses WhitespaceTokenizer. */
class CLUCENE_EXPORT WhitespaceAnalyzer: public Analyzer {
public:
    WhitespaceAnalyzer();
    TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
	TokenStream* reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
    virtual ~WhitespaceAnalyzer();
};

/** An Analyzer that filters LetterTokenizer with LowerCaseFilter. */
class CLUCENE_EXPORT SimpleAnalyzer: public Analyzer {
public:
    SimpleAnalyzer();
	TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
	TokenStream* reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);

	virtual ~SimpleAnalyzer();
};



/**
* Normalizes token text to lower case.
*/
class CLUCENE_EXPORT LowerCaseFilter: public TokenFilter {
public:
	LowerCaseFilter(TokenStream* in, bool deleteTokenStream);
	virtual ~LowerCaseFilter();
	Token* next(Token* token);
};


/**
 * Removes stop words from a token stream.
 */
class CLUCENE_EXPORT StopFilter: public TokenFilter {
private:
	//bvk: i found this to work faster with a non-hash table. the number of items
	//in the stop table is not like to make it worth having hashing.
	//ish: implement a radix/patricia tree for this?
	CLTCSetList* stopWords;
	bool deleteStopTable;

	bool enablePositionIncrements;
	const bool ignoreCase;
public:
	static bool ENABLE_POSITION_INCREMENTS_DEFAULT;

	// Constructs a filter which removes words from the input
	//	TokenStream that are named in the array of words. 
	StopFilter(TokenStream* in, bool deleteTokenStream, const TCHAR** _stopWords, const bool _ignoreCase = false);

	virtual ~StopFilter();

	/** Constructs a filter which removes words from the input
	*	TokenStream that are named in the CLSetList.
	*/
	StopFilter(TokenStream* in, bool deleteTokenStream, CLTCSetList* stopTable, bool _deleteStopTable=false);
	
	/**
	* Builds a Hashtable from an array of stop words, appropriate for passing
	* into the StopFilter constructor.  This permits this table construction to
	* be cached once when an Analyzer is constructed. 
	* Note: the stopWords list must be a static list because the strings are not copied
	*/
	static void fillStopTable(CLTCSetList* stopTable,
                                      const TCHAR** stopWords, const bool _ignoreCase = false);

	/**
	* Returns the next input Token whose termText() is not a stop word.
	*/ 
	Token* next(Token* token);


	/**
	* @see #setEnablePositionIncrementsDefault(boolean). 
	*/
	static bool getEnablePositionIncrementsDefault();

	/**
	* Set the default position increments behavior of every StopFilter created from now on.
	* <p>
	* Note: behavior of a single StopFilter instance can be modified 
	* with {@link #setEnablePositionIncrements(boolean)}.
	* This static method allows control over behavior of classes using StopFilters internally, 
	* for example {@link lucene::analysis::standard::StandardAnalyzer StandardAnalyzer}. 
	* <p>
	* Default : false.
	* @see #setEnablePositionIncrements(boolean).
	*/
	static void setEnablePositionIncrementsDefault(const bool defaultValue);

	/**
	* @see #setEnablePositionIncrements(boolean). 
	*/
	bool getEnablePositionIncrements() const;

	/**
	* Set to <code>true</code> to make <b>this</b> StopFilter enable position increments to result tokens.
	* <p>
	* When set, when a token is stopped (omitted), the position increment of 
	* the following token is incremented.  
	* <p>
	* Default: see {@link #setEnablePositionIncrementsDefault(boolean)}.
	*/
	void setEnablePositionIncrements(const bool enable);

};

/**
 * Loader for text files that represent a list of stopwords.
 *
 */
class CLUCENE_EXPORT WordlistLoader {
public:
	/**
	* Loads a text file and adds every line as an entry to a HashSet (omitting
	* leading and trailing whitespace). Every line of the file should contain only
	* one word. The words need to be in lowercase if you make use of an
	* Analyzer which uses LowerCaseFilter (like StandardAnalyzer).
	*
	* @param wordfile File containing the wordlist
	* @return A HashSet with the file's words
	*/
	static CLTCSetList* getWordSet(const char* wordfilePath, const char* enc = NULL, CLTCSetList* stopTable = NULL);

	/**
	* Reads lines from a Reader and adds every line as an entry to a HashSet (omitting
	* leading and trailing whitespace). Every line of the Reader should contain only
	* one word. The words need to be in lowercase if you make use of an
	* Analyzer which uses LowerCaseFilter (like StandardAnalyzer).
	*
	* @param reader Reader containing the wordlist
	* @return A HashSet with the reader's words
	*/
	static CLTCSetList* getWordSet(CL_NS(util)::Reader* reader, CLTCSetList* stopTable = NULL, const bool bDeleteReader = false);
};


/** Filters LetterTokenizer with LowerCaseFilter and StopFilter. */
class CLUCENE_EXPORT StopAnalyzer: public Analyzer {
	CLTCSetList* stopTable;
    class SavedStreams;

public:
    /** Builds an analyzer which removes words in ENGLISH_STOP_WORDS. */
    StopAnalyzer();
    virtual ~StopAnalyzer();
    
    /** Builds an analyzer which removes words in the provided array. */
    StopAnalyzer( const TCHAR** stopWords );

	/** Builds an analyzer with the stop words from the given file.
	* @see WordlistLoader#getWordSet(File)
	*/
	StopAnalyzer(const char* stopwordsFile, const char* enc = NULL);

	/** Builds an analyzer with the stop words from the given reader.
	* @see WordlistLoader#getWordSet(Reader)
	*/
	StopAnalyzer(CL_NS(util)::Reader* stopwordsReader, const bool _bDeleteReader = false);

    /** Filters LowerCaseTokenizer with StopFilter. */
    TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
    TokenStream* reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);

	/** An array containing some common English words that are not usually useful
    for searching. */
    static const TCHAR* ENGLISH_STOP_WORDS[];
};



/**
 * This analyzer is used to facilitate scenarios where different
 * fields require different analysis techniques.  Use {@link #addAnalyzer}
 * to add a non-default analyzer on a field name basis.
 * 
 * <p>Example usage:
 * 
 * <pre>
 *   PerFieldAnalyzerWrapper* aWrapper =
 *      new PerFieldAnalyzerWrapper(new StandardAnalyzer());
 *   aWrapper.addAnalyzer("firstname", new KeywordAnalyzer());
 *   aWrapper.addAnalyzer("lastname", new KeywordAnalyzer());
 * </pre>
 * 
 * <p>In this example, StandardAnalyzer will be used for all fields except "firstname"
 * and "lastname", for which KeywordAnalyzer will be used.
 * 
 * <p>A PerFieldAnalyzerWrapper can be used like any other analyzer, for both indexing
 * and query parsing.
 */
class CLUCENE_EXPORT PerFieldAnalyzerWrapper : public Analyzer {
private:
    Analyzer* defaultAnalyzer;
    
    typedef CL_NS(util)::CLHashMap<TCHAR*, Analyzer*, CL_NS(util)::Compare::TChar,
    	CL_NS(util)::Equals::TChar, CL_NS(util)::Deletor::tcArray,CL_NS(util)::Deletor::Void<Analyzer> > AnalyzerMapType;
    AnalyzerMapType* analyzerMap;
public:
    /**
    * Constructs with default analyzer.
    *
    * @param defaultAnalyzer Any fields not specifically
    * defined to use a different analyzer will use the one provided here.
    */
    PerFieldAnalyzerWrapper(Analyzer* defaultAnalyzer);
    virtual ~PerFieldAnalyzerWrapper();
    
    /**
    * Defines an analyzer to use for the specified field.
    *
    * @param fieldName field name requiring a non-default analyzer
    * @param analyzer non-default analyzer to use for field
    */
    void addAnalyzer(const TCHAR* fieldName, Analyzer* analyzer);
    TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);

	TokenStream* reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);

	/** Return the positionIncrementGap from the analyzer assigned to fieldName */
	int32_t getPositionIncrementGap(const TCHAR* fieldName);
};


/**
 * A filter that replaces accented characters in the ISO Latin 1 character set 
 * (ISO-8859-1) by their unaccented equivalent. The case will not be altered.
 * <p>
 * For instance, '&agrave;' will be replaced by 'a'.
 * <p>
 */
class CLUCENE_EXPORT ISOLatin1AccentFilter: public TokenFilter {
public:
	ISOLatin1AccentFilter(TokenStream* input, bool deleteTs);
	
	/**
	 * To replace accented characters in a String by unaccented equivalents.
	 */
	Token* next(Token* token);
	virtual ~ISOLatin1AccentFilter();
};


/**
 * Emits the entire input as a single token.
 */
class CLUCENE_EXPORT KeywordTokenizer: public Tokenizer {
private:
  LUCENE_STATIC_CONSTANT(int, DEFAULT_BUFFER_SIZE = 256);
  bool done;
  int bufferSize;
public:
    KeywordTokenizer(CL_NS(util)::Reader* input, int bufferSize=-1);
    Token* next(Token* token);
	void reset(CL_NS(util)::Reader* input);

	virtual ~KeywordTokenizer();
};

/**
 * "Tokenizes" the entire stream as a single token. This is useful
 * for data like zip codes, ids, and some product names.
 */
class CLUCENE_EXPORT KeywordAnalyzer: public Analyzer {
public:
	KeywordAnalyzer();
    virtual ~KeywordAnalyzer();
    TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
	TokenStream* reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
};

    
/**
 * Removes words that are too long and too short from the stream.
 *
 */
class CLUCENE_EXPORT LengthFilter: public TokenFilter {
private:
    size_t _min;
    size_t _max;
public:
    /**
    * Build a filter that removes words that are too long or too
    * short from the text.
    */
    LengthFilter(TokenStream* in, const size_t _min, const size_t _max);
    
    /**
    * Returns the next input Token whose termText() is the right len
    */
    Token* next(Token* token);
};


CL_NS_END
#endif
