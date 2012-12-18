/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_analysis_standard_StandardAnalyzer
#define _lucene_analysis_standard_StandardAnalyzer

CL_CLASS_DEF(util,BufferedReader)
#include "CLucene/analysis/AnalysisHeader.h"

CL_NS_DEF2(analysis,standard)

/**
* Filters {@link lucene::analysis::standard::StandardTokenizer} with {@link lucene::analysis::standard::StandardFilter}, 
* {@link lucene::analysis::LowerCaseFilter} and {@link lucene::analysis::StopFilter}, using a list of English stop words.
*
*/
	class CLUCENE_EXPORT StandardAnalyzer : public Analyzer 
	{
	private:
		CLTCSetList* stopSet;
        int32_t maxTokenLength;

        class SavedStreams;
	public:
        /** Default maximum allowed token length */
        LUCENE_STATIC_CONSTANT(int32_t, DEFAULT_MAX_TOKEN_LENGTH = 255);

		/** Builds an analyzer.*/
		StandardAnalyzer();

		/** Builds an analyzer with the given stop words. */
		StandardAnalyzer( const TCHAR** stopWords);

		/** Builds an analyzer with the stop words from the given file.
		* @see WordlistLoader#getWordSet(File)
		*/
		StandardAnalyzer(const char* stopwordsFile, const char* enc = NULL);

		/** Builds an analyzer with the stop words from the given reader.
		* @see WordlistLoader#getWordSet(Reader)
		*/
		StandardAnalyzer(CL_NS(util)::Reader* stopwordsReader, const bool _bDeleteReader = false);

		virtual ~StandardAnalyzer();

        /**
        * Constructs a StandardTokenizer filtered by a 
        * StandardFilter, a LowerCaseFilter and a StopFilter.
        */
        TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);

        TokenStream* reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);

        /**
        * Set maximum allowed token length.  If a token is seen
        * that exceeds this length then it is discarded.  This
        * setting only takes effect the next time tokenStream or
        * reusableTokenStream is called.
        */
        void setMaxTokenLength(const int32_t length) {
            maxTokenLength = length;
        }

        /**
        * @see #setMaxTokenLength
        */
        int getMaxTokenLength() const {
            return maxTokenLength;
        }
	};
CL_NS_END2
#endif
