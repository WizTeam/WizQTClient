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
#include "TokenSources.h"

#include "CLucene/util/VoidList.h"
#include "CLucene/util/CLStreams.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/index/TermVector.h"
#include "CLucene/document/Document.h"

CL_NS_DEF2(search,highlight)
CL_NS_USE(analysis)
CL_NS_USE(index)
CL_NS_USE(util)


///Compares the Token for their order
class TokenOrderCompare:LUCENE_BASE, public CL_NS(util)::Compare::_base //<Token*>
{
public:
	bool operator()( Token* t1, Token* t2 ) const{
	if(t1->startOffset()>t2->startOffset())
        return false;
    if(t1->startOffset()<t2->startOffset())
        return true;
	return true;
}
};

TokenSources::TokenSources(void)
{
}

TokenSources::~TokenSources(void)
{
}

TokenStream* TokenSources::getAnyTokenStream(IndexReader* reader,int32_t docId, TCHAR* field, Analyzer* analyzer)
{
	TokenStream* ts=NULL;

	TermFreqVector* tfv=reader->getTermFreqVector(docId,field);
	if(tfv!=NULL)
	{
		TermPositionVector* tmp = tfv->__asTermPositionVector();
		if ( tmp != NULL )
		    ts=getTokenStream(tmp);
	}
	//No token info stored so fall back to analyzing raw content
	if(ts==NULL)
	{
		ts=getTokenStream(reader,docId,field,analyzer);
	}
	return ts;
}


TokenStream* TokenSources::getTokenStream(TermPositionVector* tpv)
{
    //assumes the worst and makes no assumptions about token position sequences.
    return getTokenStream(tpv,false);   
}

TokenStream* TokenSources::getTokenStream(TermPositionVector* tpv, bool tokenPositionsGuaranteedContiguous)
{
    //an object used to iterate across an array of tokens
    /*class StoredTokenStream extends TokenStream
    {
        Token tokens[];
        int32_t currentToken=0;
        StoredTokenStream(Token tokens[])
        {
            this.tokens=tokens;
        }
        public Token next()
        {
            if(currentToken>=tokens.length)
            {
                return NULL;
            }
            return tokens[currentToken++];
        }            
    }     */   
    //code to reconstruct the original sequence of Tokens
    const ArrayBase<const TCHAR*>* terms = tpv->getTerms();
    const ArrayBase<int32_t>* freq=tpv->getTermFrequencies();

    size_t totalTokens=0;
	for (int32_t i = 0; i < freq->length; i++)
		totalTokens+=freq->values[i];

    Token** tokensInOriginalOrder=NULL;
	CLSetList<Token*,TokenOrderCompare>* unsortedTokens = NULL;
    for (int32_t t = 0; t < freq->length; t++)
    {
        const ArrayBase<TermVectorOffsetInfo*>* offsets=tpv->getOffsets(t);
        if(offsets==NULL)
            return NULL;
        
        const ArrayBase<int32_t>* pos=NULL;
        if(tokenPositionsGuaranteedContiguous)
        {
            //try get the token position info to speed up assembly of tokens into sorted sequence
            pos=tpv->getTermPositions(t);
        }

		if ( tokensInOriginalOrder != NULL )
			tokensInOriginalOrder = _CL_NEWARRAY(Token*, totalTokens+1);

        if(pos==NULL)
        {	
            //tokens NOT stored with positions or not guaranteed contiguous - must add to list and sort later
            if(unsortedTokens==NULL)
                unsortedTokens=_CLNEW CLSetList<Token*,TokenOrderCompare>(false);
            for (int32_t tp=0; tp < offsets->length; tp++)
            {
                unsortedTokens->insert(_CLNEW Token(terms->values[t],
                    (*offsets)[tp]->getStartOffset(),
                    (*offsets)[tp]->getEndOffset()));
            }
        }
        else
        {
            //We have positions stored and a guarantee that the token position information is contiguous
            
            // This may be fast BUT wont work if Tokenizers used which create >1 token in same position or
            // creates jumps in position numbers - this code would fail under those circumstances
            
            //tokens stored with positions - can use this to index straight into sorted array
            for (int32_t tp = 0; tp < pos->length; tp++)
            {
                tokensInOriginalOrder[(*pos)[tp]]=_CLNEW Token(terms->values[t],
                        (*offsets)[tp]->getStartOffset(),
						(*offsets)[tp]->getEndOffset());
            }                
        }
    }
    //If the field has been stored without position data we must perform a sort        
    if(unsortedTokens!=NULL)
    {
		if ( totalTokens<unsortedTokens->size() ){
			_CLDELETE_ARRAY(tokensInOriginalOrder);
			tokensInOriginalOrder = _CL_NEWARRAY(Token*,unsortedTokens->size()+1);
		}
		//the list has already sorted our items //todo:check that this is true...
		unsortedTokens->toArray_nullTerminated(tokensInOriginalOrder);
		
		return _CLNEW StoredTokenStream(tokensInOriginalOrder,unsortedTokens->size());
    }else
		return _CLNEW StoredTokenStream(tokensInOriginalOrder,totalTokens);
}

TokenStream* TokenSources::getTokenStream(IndexReader* reader,int32_t docId, TCHAR* field)
{
	TermFreqVector* tfv=reader->getTermFreqVector(docId,field);
	if(tfv==NULL)
	{
		TCHAR buf[250];
		_sntprintf(buf,250,_T("%s in doc #%d does not have any term position data stored"),field,docId);
		_CLTHROWT(CL_ERR_IllegalArgument,buf);
		return NULL;
	}

	TermPositionVector* tmp = NULL;
	tmp = tfv->__asTermPositionVector();

	if ( tmp != NULL ){
	    return getTokenStream(tmp);	        
	}else{
		TCHAR buf[250];
		_sntprintf(buf,250,_T("%s in doc #%d does not have any term position data stored"),field,docId);
		_CLTHROWT(CL_ERR_IllegalArgument,buf);
		return NULL;
	}
}

//convenience method
TokenStream* TokenSources::getTokenStream(IndexReader* reader,int32_t docId, TCHAR* field,Analyzer* analyzer)
{
	CL_NS(document)::Document doc;
	reader->document(docId, doc);
	const TCHAR* contents=doc.get(field);
	if(contents==NULL)
	{
		TCHAR buf[250];
		_sntprintf(buf,250,_T("Field %s in document #%d is not stored and cannot be analyzed"),field,docId);
		_CLTHROWT(CL_ERR_IllegalArgument,buf);
		return NULL;
	}
    return analyzer->tokenStream(field,_CLNEW StringReader(contents));
}

TokenSources::StoredTokenStream::StoredTokenStream(CL_NS(analysis)::Token** tokens, size_t len)
{
	currentToken = 0;
    this->tokens=tokens;
	this->length = len;
}
CL_NS(analysis)::Token* TokenSources::StoredTokenStream::next(CL_NS(analysis)::Token* token)
{
    if(currentToken>=length)
    {
        return NULL;
    }
	Token* t = tokens[currentToken++];

	token->set(t->termBuffer(),t->startOffset(),t->endOffset(),t->type());;
    return token;
}
void TokenSources::StoredTokenStream::close(){
	
}

CL_NS_END2
