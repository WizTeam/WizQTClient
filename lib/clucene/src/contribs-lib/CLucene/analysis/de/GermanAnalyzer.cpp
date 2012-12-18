/*------------------------------------------------------------------------------
* Copyright (C) 2003-2010 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CLucene/util/CLStreams.h"
#include "CLucene/analysis/Analyzers.h"
#include "CLucene/analysis/standard/StandardTokenizer.h"
#include "CLucene/analysis/standard/StandardFilter.h"
#include "CLucene/util/StringBuffer.h"
#include "GermanAnalyzer.h"
#include "GermanStemmer.h"
#include "GermanStemFilter.h"

CL_NS_USE(analysis)
CL_NS_USE2(analysis,de)
CL_NS_USE2(analysis,standard)

  const TCHAR GermanAnalyzer_DASZ[] = { 0x64, 0x61, 0xdf };
  const TCHAR GermanAnalyzer_FUER[] = { 0x66, 0xfc, 0x72 };
  const TCHAR* GermanAnalyzer_GERMAN_STOP_WORDS[] = {
    _T("einer"), _T("eine"), _T("eines"), _T("einem"), _T("einen"),
    _T("der"), _T("die"), _T("das"), _T("dass"), GermanAnalyzer_DASZ,
    _T("du"), _T("er"), _T("sie"), _T("es"),
    _T("was"), _T("wer"), _T("wie"), _T("wir"),
    _T("und"), _T("oder"), _T("ohne"), _T("mit"),
    _T("am"), _T("im"),_T("in"), _T("aus"), _T("auf"),
    _T("ist"), _T("sein"), _T("war"), _T("wird"),
    _T("ihr"), _T("ihre"), _T("ihres"),
    _T("als"), GermanAnalyzer_FUER, _T("von"), _T("mit"),
    _T("dich"), _T("dir"), _T("mich"), _T("mir"),
    _T("mein"), _T("sein"), _T("kein"),
    _T("durch"), _T("wegen"), _T("wird")
  };

  CL_NS(util)::ConstValueArray<const TCHAR*> GermanAnalyzer::GERMAN_STOP_WORDS( GermanAnalyzer_GERMAN_STOP_WORDS, 48 );

  class GermanAnalyzer::SavedStreams : public TokenStream {
  public:
      StandardTokenizer* tokenStream;
      TokenStream* filteredTokenStream;

      SavedStreams():tokenStream(NULL), filteredTokenStream(NULL)
      {
      }

      void close(){}
      Token* next(Token* token) {return NULL;}
  };

  GermanAnalyzer::GermanAnalyzer() {
    exclusionSet = NULL;
    stopSet = _CLNEW CLTCSetList;
    StopFilter::fillStopTable(stopSet, GERMAN_STOP_WORDS.values);
  }

  GermanAnalyzer::GermanAnalyzer(const TCHAR** stopwords) {
    exclusionSet = NULL;
    stopSet = _CLNEW CLTCSetList;
    StopFilter::fillStopTable(stopSet, stopwords);
  }

  GermanAnalyzer::GermanAnalyzer(CL_NS(analysis)::CLTCSetList* stopwords) {
    exclusionSet = NULL;
    stopSet = stopwords;
  }

  GermanAnalyzer::GermanAnalyzer(const char* stopwordsFile, const char* enc) {
    exclusionSet = NULL;
    stopSet = WordlistLoader::getWordSet(stopwordsFile, enc);
  }

  GermanAnalyzer::GermanAnalyzer(CL_NS(util)::Reader* stopwordsReader, const bool deleteReader) {
    exclusionSet = NULL;
    stopSet = WordlistLoader::getWordSet(stopwordsReader, NULL, deleteReader);
  }

  GermanAnalyzer::~GermanAnalyzer() {
    _CLLDELETE(stopSet);
    _CLLDELETE(exclusionSet);
  }

  void GermanAnalyzer::setStemExclusionTable(const TCHAR** exclusionlist) {
    if (exclusionSet != NULL) {
      exclusionSet->clear();
    } else {
      exclusionSet = _CLNEW CLTCSetList;
    }

    CL_NS(analysis)::StopFilter::fillStopTable(exclusionSet, exclusionlist);
  }

  void GermanAnalyzer::setStemExclusionTable(CL_NS(analysis)::CLTCSetList* exclusionlist) {
    if (exclusionSet != exclusionlist) {
      _CLLDELETE(exclusionSet);
      exclusionSet = exclusionlist;
    }
  }

  void GermanAnalyzer::setStemExclusionTable(const char* exclusionlistFile, const char* enc) {
    exclusionSet = WordlistLoader::getWordSet(exclusionlistFile, enc, exclusionSet);
  }

  void GermanAnalyzer::setStemExclusionTable(CL_NS(util)::Reader* exclusionlistReader, const bool deleteReader) {
    exclusionSet = WordlistLoader::getWordSet(exclusionlistReader, exclusionSet, deleteReader);
  }

  TokenStream* GermanAnalyzer::tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader) {
    TokenStream* result;
    CL_NS(util)::BufferedReader* bufferedReader = reader->__asBufferedReader();

    if ( bufferedReader == NULL )
      result = _CLNEW StandardTokenizer( _CLNEW CL_NS(util)::FilteredBufferedReader(reader, false), true );
    else
      result = _CLNEW StandardTokenizer(bufferedReader);

    result = _CLNEW StandardFilter(result, true);
    result = _CLNEW LowerCaseFilter(result, true);
    result = _CLNEW StopFilter(result, true, stopSet);
    result = _CLNEW GermanStemFilter(result, true, exclusionSet);

    return result;
  }

  TokenStream* GermanAnalyzer::reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader)
  {
    SavedStreams* streams = reinterpret_cast<SavedStreams*>(getPreviousTokenStream());

    if (streams == NULL) {
      streams = _CLNEW SavedStreams();
      CL_NS(util)::BufferedReader* bufferedReader = reader->__asBufferedReader();

      if ( bufferedReader == NULL )
        streams->tokenStream = _CLNEW StandardTokenizer( _CLNEW CL_NS(util)::FilteredBufferedReader(reader, false), true );
      else
        streams->tokenStream = _CLNEW StandardTokenizer(bufferedReader);

      streams->filteredTokenStream = _CLNEW StandardFilter(streams->tokenStream, true);
      streams->filteredTokenStream = _CLNEW LowerCaseFilter(streams->filteredTokenStream, true);
      streams->filteredTokenStream = _CLNEW StopFilter(streams->filteredTokenStream, true, stopSet);
      streams->filteredTokenStream = _CLNEW GermanStemFilter(streams->filteredTokenStream, true, exclusionSet);
      setPreviousTokenStream(streams);
    } else
      streams->tokenStream->reset(reader);

    return streams->filteredTokenStream;
  }
