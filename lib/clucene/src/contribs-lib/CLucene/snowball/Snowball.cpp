/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "SnowballAnalyzer.h"
#include "SnowballFilter.h"
#include "CLucene/util/Misc.h"
#include "CLucene/util/CLStreams.h"
#include "CLucene/analysis/Analyzers.h"
#include "CLucene/analysis/standard/StandardTokenizer.h"
#include "CLucene/analysis/standard/StandardFilter.h"

CL_NS_USE(analysis)
CL_NS_USE(util)
CL_NS_USE2(analysis,standard)

CL_NS_DEF2(analysis,snowball)

  /** Builds the named analyzer with no stop words. */
  SnowballAnalyzer::SnowballAnalyzer(const TCHAR* language) {
    this->language = STRDUP_TtoT(language);
	stopSet = NULL;
  }

  SnowballAnalyzer::~SnowballAnalyzer(){
	  _CLDELETE_CARRAY(language);
	  if ( stopSet != NULL )
		  _CLDELETE(stopSet);
  }

  /** Builds the named analyzer with the given stop words.
  */
  SnowballAnalyzer::SnowballAnalyzer(const TCHAR* language, const TCHAR** stopWords) {
    this->language = STRDUP_TtoT(language);

    stopSet = _CLNEW CLTCSetList(true);
	StopFilter::fillStopTable(stopSet,stopWords);
  }

  TokenStream* SnowballAnalyzer::tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader) {
	 return this->tokenStream(fieldName,reader,false);
  }

  /** Constructs a {@link StandardTokenizer} filtered by a {@link
      StandardFilter}, a {@link LowerCaseFilter} and a {@link StopFilter}. */
  TokenStream* SnowballAnalyzer::tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader, bool deleteReader) {
		BufferedReader* bufferedReader = reader->__asBufferedReader();
		TokenStream* result;

		if ( bufferedReader == NULL )
			result =  _CLNEW StandardTokenizer( _CLNEW FilteredBufferedReader(reader, deleteReader), true );
		else
			result = _CLNEW StandardTokenizer(bufferedReader, deleteReader);

	 result = _CLNEW StandardFilter(result, true);
    result = _CLNEW CL_NS(analysis)::LowerCaseFilter(result, true);
    if (stopSet != NULL)
      result = _CLNEW CL_NS(analysis)::StopFilter(result, true, stopSet);
    result = _CLNEW SnowballFilter(result, language, true);
    return result;
  }
  
  
  
  
  
  
  
    /** Construct the named stemming filter.
   *
   * @param in the input tokens to stem
   * @param name the name of a stemmer
   */
	SnowballFilter::SnowballFilter(TokenStream* in, const TCHAR* language, bool deleteTS):
		TokenFilter(in,deleteTS)
	{
		TCHAR tlang[50];
		char lang[50];
		_tcsncpy(tlang,language,50);
		_tcslwr(tlang);

		STRCPY_TtoA(lang,tlang,50);
		stemmer = sb_stemmer_new(lang, NULL); //use utf8 encoding

		if ( stemmer == NULL ){
			_CLTHROWA(CL_ERR_IllegalArgument, "language not available for stemming\n"); //todo: richer error
		}
    }

	SnowballFilter::~SnowballFilter(){
		sb_stemmer_delete(stemmer);
	}

  /** Returns the next input Token, after being stemmed */
  Token* SnowballFilter::next(Token* token){
    if (input->next(token) == NULL)
      return NULL;

	unsigned char uctext[LUCENE_MAX_WORD_LEN];
	TCHAR tchartext[LUCENE_MAX_WORD_LEN];

#ifdef _UCS2
	char utf8text[LUCENE_MAX_WORD_LEN];

	size_t len = lucene_wcstoutf8(utf8text,token->termBuffer(),LUCENE_MAX_WORD_LEN);
	memcpy(uctext,utf8text,len);
	uctext[len]='\0';
#else
	const char* tmp = token->termText();
	int len = token->termTextLength();
	for (int i=0;i<len+1;i++)
		uctext[i]=tmp[i];
#endif

    const sb_symbol* stemmed = sb_stemmer_stem(stemmer, uctext, len);
	if ( stemmed == NULL )
		_CLTHROWA(CL_ERR_Runtime,"Out of memory");

	int stemmedLen=sb_stemmer_length(stemmer);

#ifdef _UCS2
	memcpy(utf8text,stemmed,stemmedLen);
	utf8text[stemmedLen]=0;
	lucene_utf8towcs(tchartext,utf8text,LUCENE_MAX_WORD_LEN);
#else
	for (int i=0;i<stemmedLen+1;i++)
		tchartext[i]=stemmed[i];
#endif
	token->set(tchartext,token->startOffset(), token->endOffset(), token->type());
	return token;
  }


CL_NS_END2
