/**
 * Copyright 2002-2004 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _lucene_search_highlight_highlighter_
#define _lucene_search_highlight_highlighter_



CL_CLASS_DEF(util, StringBuffer)
//#include "CLucene/util/VoidList.h"
CL_CLASS_DEF2(search,highlight,Formatter)
CL_CLASS_DEF2(search,highlight,Encoder)
CL_CLASS_DEF2(search,highlight,HighlightScorer)
CL_CLASS_DEF2(search,highlight,Fragmenter)
CL_CLASS_DEF2(search,highlight,TextFragment)
CL_CLASS_DEF(analysis, TokenStream)
CL_CLASS_DEF(analysis, Analyzer)

//#include "HighlightScorer.h"
//#include "SimpleFragmenter.h"
//#include "TextFragment.h"

CL_NS_DEF2(search,highlight)

/**
* Class used to markup highlighted terms found in the best sections of a
* text, using configurable {@link Fragmenter}, {@link Scorer}, {@link Formatter},
* and tokenizers. 	  
* {@link Encoder} and tokenizers.
*/
class CLUCENE_CONTRIBS_EXPORT Highlighter :LUCENE_BASE
{
private:
	int32_t maxDocBytesToAnalyze;

	Formatter * _formatter;
	bool delete_formatter;
	
	Encoder* _encoder;
	bool delete_encoder;

	Fragmenter * _textFragmenter;
	bool delete_textFragmenter;

	HighlightScorer * _fragmentScorer;
	bool delete_fragmentScorer;

	/** Improves readability of a score-sorted list of TextFragments by merging any fragments 
	 * that were contiguous in the original text into one larger fragment with the correct order.
	 * This will leave a "null" in the array entry for the lesser scored fragment. 
	 * 
	 * @param frag An array of document fragments in descending score
	 */
	void _mergeContiguousFragments(TextFragment** frag, int32_t fragsLen);
	
public:
	LUCENE_STATIC_CONSTANT(int32_t, DEFAULT_MAX_DOC_BYTES_TO_ANALYZE=50*1024);

	/**
	 * Constructs a Highlighter object with the provided scorer. The HighlightScorer object is owned
	 * by the Highlighter object, and it will freed in the destructor.
	 */
	Highlighter(HighlightScorer * fragmentScorer);

	Highlighter(Formatter * formatter, HighlightScorer * fragmentScorer);

	Highlighter(Formatter * formatter, Encoder* encoder, HighlightScorer * fragmentScorer);


	/**
	 * Destructor for Highlighter. It deletes the owned HighlightScorer, formatter and textFragmenter.
	 */
	~Highlighter();

	/**
	 * Highlights chosen terms in a text, extracting the most relevant section.
	 * The document text is analysed in chunks to record hit statistics
	 * across the document. After accumulating stats, the fragment with the highest score
	 * is returned
	 *
	 * @param tokenStream   a stream of tokens identified in the text parameter, including offset information. 
	 * This is typically produced by an analyzer re-parsing a document's 
	 * text. Some work may be done on retrieving TokenStreams more efficently 
	 * by adding support for storing original text position data in the Lucene
	 * index but this support is not currently available (as of Lucene 1.4 rc2).  
	 * @param text text to highlight terms in
	 *
	 * @return highlighted text fragment or null if no terms found
	 */
	TCHAR* getBestFragment(CL_NS(analysis)::TokenStream * tokenStream, const TCHAR* text);

	/**
	 * Highlights chosen terms in a text, extracting the most relevant section.
	 * This is a convenience method that calls
	 * {@link #getBestFragment(TokenStream, const TCHAR*)}
	 *
	 * @param analyzer   the analyzer that will be used to split <code>text</code>
	 * into chunks  
	 * @param text text to highlight terms in
	 * @param fieldName Name of field used to influence analyzer's tokenization policy 
	 *
	 * @return highlighted text fragment or null if no terms found
	 */
	TCHAR* getBestFragment(CL_NS(analysis)::Analyzer* analyzer, const TCHAR* fieldName, const TCHAR* text);

	/**
	 * Highlights chosen terms in a text, extracting the most relevant sections.
	 * This is a convenience method that calls
	 * {@link #getBestFragments(TokenStream, const TCHAR*, int)}
	 *
	 * @param analyzer   the analyzer that will be used to split <code>text</code>
	 * into chunks  
	 * @param text        	text to highlight terms in
	 * @param maxNumFragments  the maximum number of fragments.
	 *
	 * @return highlighted text fragments (between 0 and maxNumFragments number of fragments)
	 */
	TCHAR** getBestFragments(
		CL_NS(analysis)::Analyzer* analyzer,	
		const TCHAR* text,
		int32_t maxNumFragments);

	/**
	 * Highlights chosen terms in a text, extracting the most relevant sections.
	 * The document text is analysed in chunks to record hit statistics
	 * across the document. After accumulating stats, the fragments with the highest scores
	 * are returned as an array of strings in order of score (contiguous fragments are merged into 
	 * one in their original order to improve readability)
	 *
	 * @param text        	text to highlight terms in
	 * @param maxNumFragments  the maximum number of fragments.
	 *
	 * @return highlighted text fragments (between 0 and maxNumFragments number of fragments)
	 */
	 TCHAR** getBestFragments(
		CL_NS(analysis)::TokenStream * tokenStream,	
		const TCHAR* text,
		int32_t maxNumFragments);

	/**
    * Low level api to get the most relevant (formatted) sections of the document.
  	* This method has been made public to allow visibility of score information held in TextFragment objects.
  	* Thanks to Jason Calabrese for help in redefining the interface.
    * @param tokenStream
    * @param text
    * @param maxNumFragments
    * @param mergeContiguousFragments
    */
	TextFragment** getBestTextFragments(
		CL_NS(util)::StringBuffer* writeTo,
		CL_NS(analysis)::TokenStream * tokenStream,	
		const TCHAR* text,
		bool mergeContiguousFragments,
		int32_t maxNumFragments);

	/**
	 * Highlights terms in the  text , extracting the most relevant sections
	 * and concatenating the chosen fragments with a separator (typically "...").
	 * The document text is analysed in chunks to record hit statistics
	 * across the document. After accumulating stats, the fragments with the highest scores
	 * are returned in order as "separator" delimited strings.
	 *
	 * @param text        text to highlight terms in
	 * @param maxNumFragments  the maximum number of fragments.
	 * @param separator  the separator used to intersperse the document fragments (typically "...")
	 *
	 * @return highlighted text
	 */
	TCHAR* getBestFragments(
		CL_NS(analysis)::TokenStream * tokenStream,	
		const TCHAR* text,
		int32_t maxNumFragments,
		const TCHAR* separator);

	/**
	 * @return the maximum number of bytes to be tokenized per doc 
	 */
	int32_t getMaxDocBytesToAnalyze();

	/**
	 * @param byteCount the maximum number of bytes to be tokenized per doc
	 * (This can improve performance with large documents)
	 */
	void setMaxDocBytesToAnalyze(int32_t byteCount);

	/**
	 */
	Fragmenter * getTextFragmenter();

	/**
	 * @param fragmenter
	 */
	void setTextFragmenter(Fragmenter * fragmenter);

	/**
	 * @return Object used to score each text fragment 
	 */
	HighlightScorer * getFragmentScorer();

	/**
	 * @param HighlightScorer
	 */
	void setFragmentScorer(HighlightScorer * scorer);
	
    Encoder* getEncoder();
    void setEncoder(Encoder* encoder);
};


CL_NS_END2

#endif

