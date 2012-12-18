/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "StandardAnalyzer.h"

////#include "CLucene/util/VoidMap.h"
#include "CLucene/util/CLStreams.h"
#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/analysis/Analyzers.h"
#include "StandardFilter.h"
#include "StandardTokenizer.h"

CL_NS_USE(util)
CL_NS_USE(analysis)

CL_NS_DEF2(analysis,standard)

	StandardAnalyzer::StandardAnalyzer():
		stopSet(_CLNEW CLTCSetList(true)), maxTokenLength(DEFAULT_MAX_TOKEN_LENGTH)
	{
      StopFilter::fillStopTable( stopSet,CL_NS(analysis)::StopAnalyzer::ENGLISH_STOP_WORDS);
	}

	StandardAnalyzer::StandardAnalyzer( const TCHAR** stopWords):
		stopSet(_CLNEW CLTCSetList(true)), maxTokenLength(DEFAULT_MAX_TOKEN_LENGTH)
	{
		StopFilter::fillStopTable( stopSet,stopWords );
	}

	StandardAnalyzer::StandardAnalyzer(const char* stopwordsFile, const char* enc):
		stopSet(_CLNEW CLTCSetList(true)), maxTokenLength(DEFAULT_MAX_TOKEN_LENGTH)
	{
		if ( enc == NULL )
			enc = "ASCII";
		WordlistLoader::getWordSet(stopwordsFile, enc, stopSet);
	}

	StandardAnalyzer::StandardAnalyzer(CL_NS(util)::Reader* stopwordsReader, const bool _bDeleteReader):
		stopSet(_CLNEW CLTCSetList(true)), maxTokenLength(DEFAULT_MAX_TOKEN_LENGTH)
	{
		WordlistLoader::getWordSet(stopwordsReader, stopSet, _bDeleteReader);
	}

        class StandardAnalyzer::SavedStreams : public TokenStream {
        public:
            StandardTokenizer* tokenStream;
            TokenStream* filteredTokenStream;

            SavedStreams():tokenStream(NULL), filteredTokenStream(NULL)
            {
            }

            void close(){}
            Token* next(Token* token) {return NULL;}
        };

	StandardAnalyzer::~StandardAnalyzer(){
        SavedStreams* t = reinterpret_cast<SavedStreams*>(this->getPreviousTokenStream());
        if (t) _CLDELETE(t->filteredTokenStream);
		_CLLDELETE(stopSet);
	}


	TokenStream* StandardAnalyzer::tokenStream(const TCHAR* /*fieldName*/, Reader* reader)
	{
		BufferedReader* bufferedReader = reader->__asBufferedReader();
		TokenStream* ret;

		if ( bufferedReader == NULL )
			ret = _CLNEW StandardTokenizer( _CLNEW FilteredBufferedReader(reader, false), true );
		else
			ret = _CLNEW StandardTokenizer(bufferedReader);
        //ret->setMaxTokenLength(maxTokenLength);
		ret = _CLNEW StandardFilter(ret,true);
		ret = _CLNEW LowerCaseFilter(ret,true);
		ret = _CLNEW StopFilter(ret,true, stopSet);
		return ret;
	}

    TokenStream* StandardAnalyzer::reusableTokenStream(const TCHAR* fieldName, Reader* reader){
        SavedStreams* streams = reinterpret_cast<SavedStreams*>(getPreviousTokenStream());
        if (streams == NULL) {
            streams = _CLNEW SavedStreams();
            setPreviousTokenStream(streams);

            BufferedReader* bufferedReader = reader->__asBufferedReader();
            if ( bufferedReader == NULL )
                streams->tokenStream = _CLNEW StandardTokenizer( _CLNEW FilteredBufferedReader(reader, false), true);
            else
                streams->tokenStream = _CLNEW StandardTokenizer(bufferedReader);

            streams->filteredTokenStream = _CLNEW StandardFilter(streams->tokenStream, true);
            streams->filteredTokenStream = _CLNEW LowerCaseFilter(streams->filteredTokenStream, true);
            streams->filteredTokenStream = _CLNEW StopFilter(streams->filteredTokenStream, true, stopSet);
        } else {
            streams->tokenStream->reset(reader);
        }
        //streams->tokenStream->setMaxTokenLength(maxTokenLength);

        return streams->filteredTokenStream;
    }
CL_NS_END2
