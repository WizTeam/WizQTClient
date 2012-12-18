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

#include "CLucene/_ApiHeader.h"
#include "Highlighter.h"
#include "TokenGroup.h"
#include "Encoder.h"
#include "Scorer.h"
#include "Formatter.h"
#include "HighlightScorer.h"
#include "Fragmenter.h"
#include "TextFragment.h"
#include "SimpleFragmenter.h"
#include "SimpleHTMLFormatter.h"
#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/util/PriorityQueue.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/CLStreams.h"

CL_NS_DEF2(search,highlight)
CL_NS_USE(analysis)
CL_NS_USE(util)

	class FragmentQueue : public CL_NS(util)::PriorityQueue<TextFragment*, CL_NS(util)::Deletor::Object<TextFragment> >
	{
	public:
		FragmentQueue(int32_t size)
		{
			initialize(size, true);
		}

	protected:
		bool lessThan(TextFragment * fragA, TextFragment * fragB)
		{
			if (fragA->getScore() == fragB->getScore())
				return fragA->getFragNum() > fragB->getFragNum();
			else
				return fragA->getScore() < fragB->getScore();
		}
	};


	Highlighter::Highlighter(HighlightScorer * fragmentScorer):
		delete_textFragmenter(true),
		delete_fragmentScorer(false),
		delete_formatter(true),
		delete_encoder(true)
	{
		maxDocBytesToAnalyze = DEFAULT_MAX_DOC_BYTES_TO_ANALYZE;
		
		_textFragmenter = _CLNEW SimpleFragmenter();
		_fragmentScorer = fragmentScorer;
		_formatter = _CLNEW SimpleHTMLFormatter();
		_encoder = _CLNEW DefaultEncoder();
	}

	Highlighter::Highlighter(Formatter * formatter, HighlightScorer * fragmentScorer):
		delete_textFragmenter(true),
		delete_fragmentScorer(false),
		delete_formatter(false),
		delete_encoder(true)
	{
		maxDocBytesToAnalyze = DEFAULT_MAX_DOC_BYTES_TO_ANALYZE;
		
		_textFragmenter = _CLNEW SimpleFragmenter();
		_fragmentScorer = fragmentScorer;
		_formatter = formatter;
		_encoder = _CLNEW DefaultEncoder();
	}

	Highlighter::Highlighter(Formatter * formatter, Encoder* encoder, HighlightScorer * fragmentScorer):
		delete_textFragmenter(true),
		delete_fragmentScorer(false),
		delete_formatter(false),
		delete_encoder(false)
	{
		maxDocBytesToAnalyze = DEFAULT_MAX_DOC_BYTES_TO_ANALYZE;
		_textFragmenter = _CLNEW SimpleFragmenter();
		_fragmentScorer = fragmentScorer;
		_formatter = formatter;
		_encoder = encoder;
	}

	Highlighter::~Highlighter()
	{
		if ( delete_textFragmenter )
			_CLDELETE ( _textFragmenter );

		if ( delete_fragmentScorer )
			_CLDELETE(_fragmentScorer);

		if( delete_formatter )
			_CLDELETE(_formatter);

		if ( delete_encoder )
			_CLDELETE(_encoder);
	}

	TCHAR* Highlighter::getBestFragment(TokenStream * tokenStream, const TCHAR* text)
	{
		TCHAR** results = getBestFragments(tokenStream,text, 1);
		TCHAR* result = 0;

		if (results[0] != NULL )
			result = stringDuplicate(results[0]);

		_CLDELETE_CARRAY_ALL(results);

		return result;
	}

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
  	* @return highlighted text fragment or NULL if no terms found
  	*/
  	TCHAR* Highlighter::getBestFragment(Analyzer* analyzer, const TCHAR* fieldName, const TCHAR* text)
  	{
  	    TokenStream* tokenStream = analyzer->tokenStream(fieldName, _CLNEW StringReader(text));
  	    return getBestFragment(tokenStream, text);
  	}

	TCHAR** Highlighter::getBestFragments(
		TokenStream * tokenStream,	
		const TCHAR* text,
		int32_t maxNumFragments)
	{
		maxNumFragments = cl_max((int32_t)1, maxNumFragments); //sanity check
		
		StringBuffer buffer;
		TextFragment** frags = getBestTextFragments(&buffer,tokenStream,text, true,maxNumFragments);

		//Get text
		std::vector<TCHAR*> fragTexts;
		for (uint32_t i=0; frags[i]!=NULL; i++)
		{
			TextFragment* f = frags[i];
			if ((f != NULL) && (f->getScore() > 0))
			{
				 fragTexts.push_back(f->toString(&buffer));
			}
			_CLDELETE(f);
		}

		_CLDELETE_ARRAY(frags);

        size_t l = fragTexts.size();
		TCHAR** ret = _CL_NEWARRAY(TCHAR*,l+1);
		for ( size_t j=0;j<l;j++ )
		    ret[j] = fragTexts[j];
		ret[l] = NULL;

		return ret;
	}

	TCHAR* Highlighter::getBestFragments(
		TokenStream * tokenStream,	
		const TCHAR* text,
		int32_t maxNumFragments,
		const TCHAR* separator)
	{
		TCHAR** sections = getBestFragments(tokenStream,text, maxNumFragments);
		StringBuffer result;

		for (int32_t i = 0; sections[i]!=NULL; i++)
		{
			if (i > 0)
			{
				result.append(separator);
			}
			result.append(sections[i]);
		}

		_CLDELETE_CARRAY_ALL(sections);
		return result.toString();
	}

	TextFragment** Highlighter::getBestTextFragments(
		StringBuffer* writeTo,
		TokenStream * tokenStream,	
		const TCHAR* text,
		bool mergeContiguousFragments,
		int32_t maxNumFragments)
	{
		CLArrayList<TextFragment*> docFrags(false);
		TextFragment* currentFrag = _CLNEW TextFragment(writeTo->length(), docFrags.size());
		_fragmentScorer->startFragment(currentFrag);
		docFrags.push_back(currentFrag);

		FragmentQueue fragQueue(maxNumFragments);

		try
		{
			int32_t startOffset;
			int32_t endOffset;
			int32_t lastEndOffset = 0;
			_textFragmenter->start(text);
			TCHAR substringBuffer[LUCENE_MAX_WORD_LEN];

			TokenGroup* tokenGroup=_CLNEW TokenGroup();

			TCHAR buffer[LUCENE_MAX_FIELD_LEN+1];
			Token token;
			while ( tokenStream->next(&token) )
			{
				if((tokenGroup->getNumTokens()>0)&&(tokenGroup->isDistinct(&token))){
					//the current token is distinct from previous tokens -
					// markup the cached token group info
					 startOffset = tokenGroup->getStartOffset();
					 endOffset = tokenGroup->getEndOffset();

					 _tcsncpy(substringBuffer,text+startOffset,endOffset-startOffset);
					 substringBuffer[endOffset-startOffset]=_T('\0');

					 TCHAR* encoded = _encoder->encodeText(substringBuffer);
					 TCHAR* markedUpText=_formatter->highlightTerm(encoded, tokenGroup);
					 _CLDELETE_CARRAY(encoded);

					 //store any whitespace etc from between this and last group
					 if (startOffset > lastEndOffset){
						 int len = startOffset-lastEndOffset;
						 if ( len > LUCENE_MAX_FIELD_LEN )
							 len = LUCENE_MAX_FIELD_LEN;
						 _tcsncpy(buffer,text+lastEndOffset,len);
						 buffer[len]=_T('\0');

						 TCHAR* encoded = _encoder->encodeText(buffer);
						 writeTo->append(encoded);
						 _CLDELETE_CARRAY(encoded);
					 }
					 writeTo->append(markedUpText);
					 lastEndOffset=endOffset;
					 tokenGroup->clear();
					 _CLDELETE_CARRAY(markedUpText);

					//check if current token marks the start of a new fragment
					if (_textFragmenter->isNewFragment(&token))
					{
						currentFrag->setScore(_fragmentScorer->getFragmentScore());
						//record stats for a new fragment
						currentFrag->setTextEndPos( writeTo->length() );
						currentFrag =_CLNEW TextFragment(writeTo->length(), docFrags.size());
						_fragmentScorer->startFragment(currentFrag);
						docFrags.push_back(currentFrag);
					}
				}

				// does query contain current token?
				float_t score=_fragmentScorer->getTokenScore(&token);			
				//TCHAR* highlightedTerm = _formatter->highlightTerm(&substringBuffer, token->termText(), score, startOffset);
				//newText->append(highlightedTerm);
				//_CLDELETE_CARRAY(highlightedTerm);
				//_CLDELETE(token);

				tokenGroup->addToken(&token,_fragmentScorer->getTokenScore(&token));

				if(lastEndOffset>maxDocBytesToAnalyze)
				{
					break;
				}
			}
			currentFrag->setScore(_fragmentScorer->getFragmentScore());

			if(tokenGroup->getNumTokens()>0)
  	        {
  	            //flush the accumulated text (same code as in above loop)
  	            startOffset = tokenGroup->getStartOffset();
  	            endOffset = tokenGroup->getEndOffset();

				_tcsncpy(substringBuffer,text+startOffset,endOffset-startOffset);
				substringBuffer[endOffset-startOffset]=_T('\0');

				TCHAR* encoded = _encoder->encodeText(substringBuffer);
        TCHAR* markedUpText=_formatter->highlightTerm(encoded, tokenGroup);
				_CLDELETE_CARRAY(encoded);

  	            //store any whitespace etc from between this and last group
				if (startOffset > lastEndOffset){
					int len = startOffset-lastEndOffset;
					if ( len > LUCENE_MAX_FIELD_LEN )
						len = LUCENE_MAX_FIELD_LEN;
					_tcsncpy(buffer,text+lastEndOffset,len);
					buffer[len]=_T('\0');

					TCHAR* encoded = _encoder->encodeText(buffer);
  					writeTo->append(encoded);
					_CLDELETE_CARRAY(encoded);
				}
  	            writeTo->append(markedUpText);
  	            lastEndOffset=endOffset;

				_CLDELETE_CARRAY(markedUpText);
  	        }

			// append text after end of last token
			//if (lastEndOffset < (int32_t)_tcslen(text))
			//newText->append(text+lastEndOffset);

			currentFrag->setTextEndPos(writeTo->length());

			//sort the most relevant sections of the text
			while (docFrags.size() > 0) {
			//for (TextFragmentList::iterator i = docFrags.begin(); i != docFrags.end(); i++)
			//{
				currentFrag = (TextFragment*) docFrags[0];
				docFrags.remove(0);

				//If you are running with a version of Lucene before 11th Sept 03
				// you do not have PriorityQueue.insert() - so uncomment the code below					

				/*if (currentFrag->getScore() >= minScore)
				{
					fragQueue.put(currentFrag);
					if (fragQueue.size() > maxNumFragments)
					{ // if hit queue overfull
						_CLLDELETE(fragQueue.pop()); // remove lowest in hit queue
						minScore = ((TextFragment *) fragQueue.top())->getScore(); // reset minScore
					}


				} else {
					_CLDELETE(currentFrag);
				}*/

				//The above code caused a problem as a result of Christoph Goller's 11th Sept 03
				//fix to PriorityQueue. The correct method to use here is the new "insert" method
				// USE ABOVE CODE IF THIS DOES NOT COMPILE!
				if ( !fragQueue.insert(currentFrag) )
					_CLDELETE(currentFrag);

				//todo: check this
			}

			//return the most relevant fragments
			int32_t fragsLen = fragQueue.size();
			TextFragment** frags = _CL_NEWARRAY(TextFragment*,fragsLen+1);
			for ( int32_t i=0;i<fragsLen;i++ )
				frags[i] = fragQueue.pop();
			frags[fragsLen]=NULL;

			//merge any contiguous fragments to improve readability
  	        if(mergeContiguousFragments)
  	        {
  	            _mergeContiguousFragments(frags,fragsLen);
  	            CLArrayList<TextFragment*> fragTexts;
  	            for (int32_t i = 0; i < fragsLen; i++)
  	            {
					TextFragment* tf = frags[i];
  	                if ((tf != NULL) && (tf->getScore() > 0))
  						fragTexts.push_back(tf);
  	                else
						_CLDELETE(tf);
  	            }
				_CLDELETE_ARRAY(frags);
				frags = _CL_NEWARRAY(TextFragment*,fragTexts.size()+1);
				fragTexts.toArray_nullTerminated(frags);
  	        }

			_CLDELETE(tokenGroup);
			//_CLDELETE(newText);
			return frags;

		}
		_CLFINALLY(
			if (tokenStream)
			{
				try
				{
					tokenStream->close();
				}
				catch (...)
				{
				}
			}
		)
	}


	void Highlighter::_mergeContiguousFragments(TextFragment** frag, int32_t fragsLen)
	{
		bool mergingStillBeingDone;
		if ( frag[0] != NULL )
			do
			{
				mergingStillBeingDone = false; //initialise loop control flag
				//for each fragment, scan other frags looking for contiguous blocks
				for (int32_t i=0; i<fragsLen; i++)
				{
					if (frag[i] == NULL)
					{
						continue;
					}
					//merge any contiguous blocks 
					for (int32_t x=0; x<fragsLen; x++)
					{
					   if ( x==i )
					      continue; //bug 1072183. don't try and merge with self

						if (frag[x] == NULL)
							continue;
						if (frag[i] == NULL)
							break;

						TextFragment * frag1 = NULL;
						TextFragment * frag2 = NULL;
						int32_t frag1Num = 0;
						int32_t frag2Num = 0;
						int32_t bestScoringFragNum;
						int32_t worstScoringFragNum;
						//if blocks are contiguous....
						if (frag[i]->follows(frag[x]))
						{
							frag1 = frag[x];
							frag1Num = x;
							frag2 = frag[i];
							frag2Num = i;
						}
						else if (frag[x]->follows(frag[i]))
						{
							frag1 = frag[i];
							frag1Num = i;
							frag2 = frag[x];
							frag2Num = x;
						}
						//merging required..
						if (frag1 != NULL)
						{
							if (frag1->getScore() > frag2->getScore())
							{
								bestScoringFragNum = frag1Num;
								worstScoringFragNum = frag2Num;
							}
							else
							{
								bestScoringFragNum = frag2Num;
								worstScoringFragNum = frag1Num;
							}
							frag1->merge(frag2);
							frag[worstScoringFragNum]= NULL;
							mergingStillBeingDone = true;
							frag[bestScoringFragNum]=frag1;
							_CLDELETE(frag2);
						}
					}
				}
			}
			while (mergingStillBeingDone);
	}

	int32_t Highlighter::getMaxDocBytesToAnalyze()
	{
		return maxDocBytesToAnalyze;
	}

	void Highlighter::setMaxDocBytesToAnalyze(int32_t byteCount)
	{
		maxDocBytesToAnalyze = byteCount;
	}

	Fragmenter * Highlighter::getTextFragmenter()
	{
		return _textFragmenter;
	}

	void Highlighter::setTextFragmenter(Fragmenter * fragmenter)
	{
		if ( delete_textFragmenter ){
			_CLDELETE(_textFragmenter);
			delete_textFragmenter = false;
		}
		_textFragmenter = fragmenter;
	}

	HighlightScorer * Highlighter::getFragmentScorer()
	{
		return _fragmentScorer;
	}


	void Highlighter::setFragmentScorer(HighlightScorer * scorer)
	{
		if ( delete_fragmentScorer ){
			delete_fragmentScorer = false;
			_CLDELETE(scorer);
		}
		_fragmentScorer = scorer;
	}

	
    Encoder* Highlighter::getEncoder()
    {
        return _encoder;
    }
    void Highlighter::setEncoder(Encoder* encoder)
    {
		if ( delete_encoder ){
			_CLDELETE(encoder);
			delete_encoder = false;
		}
        this->_encoder = encoder;
    }



CL_NS_END2
