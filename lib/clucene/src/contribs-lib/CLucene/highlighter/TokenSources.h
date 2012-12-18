/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#ifndef _lucene_search_highlight_tokensources_
#define _lucene_search_highlight_tokensources_

#include "CLucene/analysis/AnalysisHeader.h"
CL_CLASS_DEF(index, IndexReader)
CL_CLASS_DEF(index, TermPositionVector)
//#include "CLucene/index/IndexReader.h"
//#include "CLucene/index/TermVector.h"

CL_NS_DEF2(search,highlight)

class CLUCENE_CONTRIBS_EXPORT TokenSources: LUCENE_BASE
{
	//an object used to iterate across an array of tokens
	class StoredTokenStream:public CL_NS(analysis)::TokenStream
    {
	public:
        CL_NS(analysis)::Token** tokens;
		size_t length;
        int32_t currentToken;
        StoredTokenStream(CL_NS(analysis)::Token** tokens, size_t len);
		CL_NS(analysis)::Token* next(CL_NS(analysis)::Token* token);
		void close();
    };
public:
	TokenSources(void);
	~TokenSources(void);

	/**
     * A convenience method that tries a number of approaches to getting a token stream.
     * The cost of finding there are no termVectors in the index is minimal (1000 invocations still 
     * registers 0 ms). So this "lazy" (flexible?) approach to coding is probably acceptable
     * @param reader
     * @param docId
     * @param field
     * @param analyzer
     * @return null if field not stored correctly 
     * @throws IOException
     */
	static CL_NS(analysis)::TokenStream* getAnyTokenStream(CL_NS(index)::IndexReader* reader,int32_t docId, TCHAR* field, CL_NS(analysis)::Analyzer* analyzer);
    
    static CL_NS(analysis)::TokenStream* getTokenStream(CL_NS(index)::TermPositionVector* tpv);

    /**
     * Low level api.
     * Returns a token stream or null if no offset info available in index.
     * This can be used to feed the highlighter with a pre-parsed token stream 
     * 
     * In my tests the speeds to recreate 1000 token streams using this method are:
     * - with TermVector offset only data stored - 420  milliseconds 
     * - with TermVector offset AND position data stored - 271 milliseconds
     *  (nb timings for TermVector with position data are based on a tokenizer with contiguous
     *  positions - no overlaps or gaps)
     * The cost of not using TermPositionVector to store
     * pre-parsed content and using an analyzer to re-parse the original content: 
     * - reanalyzing the original content - 980 milliseconds
     * 
     * The re-analyze timings will typically vary depending on -
     * 	1) The complexity of the analyzer code (timings above were using a 
     * 	   stemmer/lowercaser/stopword combo)
     *  2) The  number of other fields (Lucene reads ALL fields off the disk 
     *     when accessing just one document field - can cost dear!)
     *  3) Use of compression on field storage - could be faster cos of compression (less disk IO)
     *     or slower (more CPU burn) depending on the content.
     *
     * @param tpv
     * @param tokenPositionsGuaranteedContiguous true if the token position numbers have no overlaps or gaps. If looking
     * to eek out the last drops of performance, set to true. If in doubt, set to false.
     */
    static CL_NS(analysis)::TokenStream* getTokenStream(CL_NS(index)::TermPositionVector* tpv, bool tokenPositionsGuaranteedContiguous);

	static CL_NS(analysis)::TokenStream* getTokenStream(CL_NS(index)::IndexReader* reader,int32_t docId, TCHAR* field);

    //convenience method
	static CL_NS(analysis)::TokenStream* getTokenStream(CL_NS(index)::IndexReader* reader,int32_t docId, TCHAR* field,CL_NS(analysis)::Analyzer* analyzer);
};

CL_NS_END2
#endif
