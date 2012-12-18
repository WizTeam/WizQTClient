/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "Analyzers.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/Misc.h"
#include <assert.h>

CL_NS_USE(util)
CL_NS_DEF(analysis)

CharTokenizer::CharTokenizer(Reader* in) :
	Tokenizer(in),
	offset(0),
	bufferIndex(0),
	dataLen(0),
	ioBuffer(NULL)
{
	buffer[0]=0;
}
CharTokenizer::~CharTokenizer(){
}

TCHAR CharTokenizer::normalize(const TCHAR c) const
{
	return c;
}
Token* CharTokenizer::next(Token* token){
	int32_t length = 0;
	int32_t start = offset;
	while (true) {
		TCHAR c;
		offset++;
		if (bufferIndex >= dataLen) {
			dataLen = input->read(ioBuffer, 1, LUCENE_IO_BUFFER_SIZE );
			if (dataLen == -1)
				dataLen = 0;
			bufferIndex = 0;
		}
		if (dataLen <= 0 ) {
			if (length > 0)
				break;
			else
				return NULL;
		}else
			c = ioBuffer[bufferIndex++];
		if (isTokenChar(c)) {                       // if it's a token TCHAR

      if (length == 0)			  // start of token
        start = offset-1;

      buffer[length++] = normalize(c);          // buffer it, normalized

      if (length == LUCENE_MAX_WORD_LEN)		  // buffer overflow!
        break;

		} else if (length > 0)			  // at non-Letter w/ chars
			break;					  // return 'em
	}
	buffer[length]=0;
	token->set( buffer, start, start+length);
	return token;
}
void CharTokenizer::reset(CL_NS(util)::Reader* input)
{
	Tokenizer::reset(input);
	bufferIndex = 0;
	offset = 0;
	dataLen = 0;
}


LetterTokenizer::LetterTokenizer(CL_NS(util)::Reader* in):
    CharTokenizer(in) {
}

LetterTokenizer::~LetterTokenizer(){}

bool LetterTokenizer::isTokenChar(const TCHAR c) const {
	return _istalpha(c)!=0;
}

LowerCaseTokenizer::LowerCaseTokenizer(CL_NS(util)::Reader* in):
    LetterTokenizer(in) {
}

LowerCaseTokenizer::~LowerCaseTokenizer(){
}

TCHAR LowerCaseTokenizer::normalize(const TCHAR chr) const {
	return _totlower(chr);
}

WhitespaceTokenizer::WhitespaceTokenizer(CL_NS(util)::Reader* in):CharTokenizer(in) {
}
WhitespaceTokenizer::~WhitespaceTokenizer(){
}

bool WhitespaceTokenizer::isTokenChar(const TCHAR c)  const{
	return _istspace(c)==0; //(return true if NOT a space)
}

WhitespaceAnalyzer::WhitespaceAnalyzer(){
}
WhitespaceAnalyzer::~WhitespaceAnalyzer(){
}

TokenStream* WhitespaceAnalyzer::tokenStream(const TCHAR* /*fieldName*/, Reader* reader) {
	return _CLNEW WhitespaceTokenizer(reader);
}
TokenStream* WhitespaceAnalyzer::reusableTokenStream(const TCHAR* /*fieldName*/, CL_NS(util)::Reader* reader)
{
	Tokenizer* tokenizer = static_cast<Tokenizer*>(getPreviousTokenStream());
	if (tokenizer == NULL) {
		tokenizer = _CLNEW WhitespaceTokenizer(reader);
		setPreviousTokenStream(tokenizer);
	} else
		tokenizer->reset(reader);
	return tokenizer;
}

SimpleAnalyzer::SimpleAnalyzer(){
}
SimpleAnalyzer::~SimpleAnalyzer(){
}
TokenStream* SimpleAnalyzer::tokenStream(const TCHAR* /*fieldName*/, Reader* reader) {
	return _CLNEW LowerCaseTokenizer(reader);
}
TokenStream* SimpleAnalyzer::reusableTokenStream(const TCHAR* /*fieldName*/, CL_NS(util)::Reader* reader) {
	Tokenizer* tokenizer = static_cast<Tokenizer*>(getPreviousTokenStream());
	if (tokenizer == NULL) {
		tokenizer = _CLNEW LowerCaseTokenizer(reader);
		setPreviousTokenStream(tokenizer);
	} else
		tokenizer->reset(reader);
	return tokenizer;
}


LowerCaseFilter::LowerCaseFilter(TokenStream* in, bool deleteTokenStream):
    TokenFilter(in,deleteTokenStream) {
}
LowerCaseFilter::~LowerCaseFilter(){
}

Token* LowerCaseFilter::next(Token* t){
	if (input->next(t) == NULL)
		return NULL;
 	stringCaseFold( t->termBuffer() );
	return t;
}

bool StopFilter::ENABLE_POSITION_INCREMENTS_DEFAULT = false;


/** Constructs a filter which removes words from the input
*	TokenStream that are named in the CLSetList.
*/
StopFilter::StopFilter(TokenStream* in, bool deleteTokenStream, CLTCSetList* stopTable, bool _deleteStopTable):
	TokenFilter(in, deleteTokenStream),
	stopWords (stopTable),
	deleteStopTable(_deleteStopTable),
	enablePositionIncrements(ENABLE_POSITION_INCREMENTS_DEFAULT),
	ignoreCase(false)
{
}

StopFilter::StopFilter(TokenStream* in, bool deleteTokenStream, const TCHAR** _stopWords, const bool _ignoreCase):
	TokenFilter(in, deleteTokenStream),
	enablePositionIncrements(ENABLE_POSITION_INCREMENTS_DEFAULT),
	ignoreCase(_ignoreCase)
{
	deleteStopTable = true;
	stopWords = _CLNEW CLTCSetList(true);
	fillStopTable( stopWords,_stopWords, _ignoreCase );
}

StopFilter::~StopFilter(){
	if (deleteStopTable)
		_CLLDELETE(stopWords);
}
//static
bool StopFilter::getEnablePositionIncrementsDefault() {
	return ENABLE_POSITION_INCREMENTS_DEFAULT;
}
//static
void StopFilter::setEnablePositionIncrementsDefault(const bool defaultValue) {
	ENABLE_POSITION_INCREMENTS_DEFAULT = defaultValue;
}

bool StopFilter::getEnablePositionIncrements() const { return enablePositionIncrements; }
void StopFilter::setEnablePositionIncrements(const bool enable) { this->enablePositionIncrements = enable; }

void StopFilter::fillStopTable(CLTCSetList* stopTable, const TCHAR** stopWords, const bool _ignoreCase)
{
  TCHAR* tmp;
	if ( _ignoreCase ){
		for (int32_t i = 0; stopWords[i]!=NULL; i++){
      tmp = STRDUP_TtoT(stopWords[i]);
      stringCaseFold(tmp);
			stopTable->insert( tmp );
    }
	}else{
		for (int32_t i = 0; stopWords[i]!=NULL; i++){
      tmp = STRDUP_TtoT(stopWords[i]);
			stopTable->insert( tmp );
    }
  }
}

Token* StopFilter::next(Token* token) {
	// return the first non-stop word found
	int32_t skippedPositions = 0;
	while (input->next(token)){
		TCHAR* termText = token->termBuffer();
    if ( ignoreCase ){
      stringCaseFold(termText);
    }
		if (stopWords->find(termText)==stopWords->end()){
			if (enablePositionIncrements) {
				token->setPositionIncrement(token->getPositionIncrement() + skippedPositions);
			}
			return token;
		}
		skippedPositions += token->getPositionIncrement();
	}

	// reached EOS -- return nothing
	return NULL;
}

StopAnalyzer::StopAnalyzer(const char* stopwordsFile, const char* enc):
	stopTable(_CLNEW CLTCSetList(true))
{
	if ( enc == NULL )
		enc = "ASCII";
	WordlistLoader::getWordSet(stopwordsFile, enc, stopTable);
}

StopAnalyzer::StopAnalyzer(CL_NS(util)::Reader* stopwordsReader, const bool _bDeleteReader):
	stopTable(_CLNEW CLTCSetList(true))
{
	WordlistLoader::getWordSet(stopwordsReader, stopTable, _bDeleteReader);
}

StopAnalyzer::StopAnalyzer():
	stopTable(_CLNEW CLTCSetList(true))
{
	StopFilter::fillStopTable(stopTable,ENGLISH_STOP_WORDS);
}
class StopAnalyzer::SavedStreams : public TokenStream {
public:
    Tokenizer* source;
    TokenStream* result;

    SavedStreams():source(NULL), result(NULL) {}

    void close(){}
    Token* next(Token* token) {return NULL;}
};
StopAnalyzer::~StopAnalyzer()
{
    SavedStreams* t = reinterpret_cast<SavedStreams*>(this->getPreviousTokenStream());
    if (t) _CLDELETE(t->result);
    _CLDELETE(stopTable);
}
StopAnalyzer::StopAnalyzer( const TCHAR** stopWords):
	stopTable(_CLNEW CLTCSetList(true))
{
	StopFilter::fillStopTable(stopTable,stopWords);
}
TokenStream* StopAnalyzer::tokenStream(const TCHAR* /*fieldName*/, Reader* reader) {
	return _CLNEW StopFilter(_CLNEW LowerCaseTokenizer(reader),true, stopTable);
}

/** Filters LowerCaseTokenizer with StopFilter. */
TokenStream* StopAnalyzer::reusableTokenStream(const TCHAR* fieldName, Reader* reader) {
    SavedStreams* streams = reinterpret_cast<SavedStreams*>(getPreviousTokenStream());
    if (streams == NULL) {
        streams = _CLNEW SavedStreams();
        streams->source = _CLNEW LowerCaseTokenizer(reader);
        streams->result = _CLNEW StopFilter(streams->source, true, stopTable);
        setPreviousTokenStream(streams);
    } else
        streams->source->reset(reader);
    return streams->result;
}

const TCHAR* StopAnalyzer::ENGLISH_STOP_WORDS[]  = 
{
	_T("a"), _T("an"), _T("and"), _T("are"), _T("as"), _T("at"), _T("be"), _T("but"), _T("by"),
	_T("for"), _T("if"), _T("in"), _T("into"), _T("is"), _T("it"),
	_T("no"), _T("not"), _T("of"), _T("on"), _T("or"), _T("such"),
	_T("that"), _T("the"), _T("their"), _T("then"), _T("there"), _T("these"),
	_T("they"), _T("this"), _T("to"), _T("was"), _T("will"), _T("with"), NULL
};

PerFieldAnalyzerWrapper::PerFieldAnalyzerWrapper(Analyzer* defaultAnalyzer):
    analyzerMap(_CLNEW AnalyzerMapType(true,true))
{
    this->defaultAnalyzer = defaultAnalyzer;
}
PerFieldAnalyzerWrapper::~PerFieldAnalyzerWrapper(){
    analyzerMap->clear();
    _CLLDELETE(analyzerMap);
    _CLLDELETE(defaultAnalyzer);
}

void PerFieldAnalyzerWrapper::addAnalyzer(const TCHAR* fieldName, Analyzer* analyzer) {
    analyzerMap->put(STRDUP_TtoT(fieldName), analyzer);
}

TokenStream* PerFieldAnalyzerWrapper::tokenStream(const TCHAR* fieldName, Reader* reader) {
    Analyzer* analyzer = analyzerMap->get(const_cast<TCHAR*>(fieldName));
    if (analyzer == NULL) {
      analyzer = defaultAnalyzer;
    }

    return analyzer->tokenStream(fieldName, reader);
}

TokenStream* PerFieldAnalyzerWrapper::reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader) {
	Analyzer* analyzer = analyzerMap->get(const_cast<TCHAR*>(fieldName));
    if (analyzer == NULL){
		analyzer = defaultAnalyzer;
    }

	return analyzer->reusableTokenStream(fieldName, reader);
}

int32_t PerFieldAnalyzerWrapper::getPositionIncrementGap(const TCHAR* fieldName) {
	Analyzer* analyzer = analyzerMap->get(const_cast<TCHAR*>(fieldName));
	if (analyzer == NULL)
		analyzer = defaultAnalyzer;
	return analyzer->getPositionIncrementGap(fieldName);
}



ISOLatin1AccentFilter::ISOLatin1AccentFilter(TokenStream* input, bool deleteTs):
	TokenFilter(input,deleteTs)
{
}
ISOLatin1AccentFilter::~ISOLatin1AccentFilter(){
}
Token* ISOLatin1AccentFilter::next(Token* token){
	if ( input->next(token) != NULL ){
		int32_t l = token->termLength();
		const TCHAR* chars = token->termBuffer();
		bool doProcess = false;
		for (int32_t i = 0; i < l; ++i) {
			#ifdef _UCS2
			if ( chars[i] >= 0xC0 && chars[i] <= 0x178 ) {
			#else
			if ( (chars[i] >= 0xC0 && chars[i] <= 0xFF) || chars[i] < 0 ) {
			#endif
				doProcess = true;
				break;
			}

		}
		if ( !doProcess ) {
			return token;
		}

		StringBuffer output(l*2);
		for (int32_t j = 0; j < l; j++) {
			#ifdef _UCS2
			TCHAR c = chars[j];
			#else
			unsigned char c = chars[j];
			#endif
			switch (c) {
				case 0xC0 : // �
				case 0xC1 : // �
				case 0xC2 : // �
				case 0xC3 : // �
				case 0xC4 : // �
				case 0xC5 : // �
					output.appendChar('A');
					break;
				case 0xC6 : // �
					output.append(_T("AE"));
					break;
				case 0xC7 : // �
					output.appendChar('C');
					break;
				case 0xC8 : // �
				case 0xC9 : // �
				case 0xCA : // �
				case 0xCB : // �
					output.appendChar('E');
					break;
				case 0xCC : // �
				case 0xCD : // �
				case 0xCE : // �
				case 0xCF : // �
					output.appendChar('I');
					break;
				case 0xD0 : // �
					output.appendChar('D');
					break;
				case 0xD1 : // �
					output.appendChar('N');
					break;
				case 0xD2 : // �
				case 0xD3 : // �
				case 0xD4 : // �
				case 0xD5 : // �
				case 0xD6 : // �
				case 0xD8 : // �
					output.appendChar('O');
					break;
				case 0xDE : // �
					output.append(_T("TH"));
					break;
				case 0xD9 : // �
				case 0xDA : // �
				case 0xDB : // �
				case 0xDC : // �
					output.appendChar('U');
					break;
				case 0xDD : // �
					output.appendChar('Y');
					break;
				case 0xE0 : // �
				case 0xE1 : // �
				case 0xE2 : // �
				case 0xE3 : // �
				case 0xE4 : // �
				case 0xE5 : // �
					output.appendChar('a');
					break;
				case 0xE6 : // �
					output.append(_T("ae"));
					break;
				case 0xE7 : // �
					output.appendChar('c');
					break;
				case 0xE8 : // �
				case 0xE9 : // �
				case 0xEA : // �
				case 0xEB : // �
					output.appendChar('e');
					break;
				case 0xEC : // �
				case 0xED : // �
				case 0xEE : // �
				case 0xEF : // �
					output.appendChar('i');
					break;
				case 0xF0 : // �
					output.appendChar('d');
					break;
				case 0xF1 : // �
					output.appendChar('n');
					break;
				case 0xF2 : // �
				case 0xF3 : // �
				case 0xF4 : // �
				case 0xF5 : // �
				case 0xF6 : // �
				case 0xF8 : // �
					output.appendChar('o');
					break;
				case 0xDF : // �
					output.append(_T("ss"));
					break;
				case 0xFE : // �
					output.append(_T("th"));
					break;
				case 0xF9 : // �
				case 0xFA : // �
				case 0xFB : // �
				case 0xFC : // �
					output.appendChar('u');
					break;
				case 0xFD : // �
				case 0xFF : // �
					output.appendChar('y');
					break;

				#ifdef _UCS2
				case 0x152 : // �
					output.append(_T("OE"));
					break;
				case 0x153 : // �
					output.append(_T("oe"));
					break;
				case 0x178 : // �
					output.appendChar('Y');
					break;
				#endif
				default :
					output.appendChar(c);
					break;
			}
		}
		token->setText(output.getBuffer());
		return token;
	}
	return NULL;
}


KeywordAnalyzer::KeywordAnalyzer(){}
KeywordAnalyzer::~KeywordAnalyzer(){}
TokenStream* KeywordAnalyzer::tokenStream(const TCHAR* /*fieldName*/, CL_NS(util)::Reader* reader){
    return _CLNEW KeywordTokenizer(reader);
}
TokenStream* KeywordAnalyzer::reusableTokenStream(const TCHAR* /*fieldName*/, CL_NS(util)::Reader* reader)
{
	Tokenizer* tokenizer = static_cast<Tokenizer*>(getPreviousTokenStream());
	if (tokenizer == NULL) {
		tokenizer = _CLNEW KeywordTokenizer(reader);
		setPreviousTokenStream(tokenizer);
	} else
		tokenizer->reset(reader);
	return tokenizer;
}

KeywordTokenizer::KeywordTokenizer(CL_NS(util)::Reader* input, int bufferSize):
	Tokenizer(input)
{
  this->done = false;
  this->bufferSize = bufferSize;
  if ( bufferSize < 1 )
	  this->bufferSize = DEFAULT_BUFFER_SIZE;
}
KeywordTokenizer::~KeywordTokenizer(){
}

Token* KeywordTokenizer::next(Token* token){
  if (!done) {
    done = true;
    int32_t upto = 0;
    int32_t rd;

    token->clear();
    TCHAR* termBuffer=token->termBuffer();
    const TCHAR* readBuffer=NULL;

    while (true) {
      rd = input->read(readBuffer, 1, cl_min(bufferSize, token->bufferLength()-upto) );
      if (rd == -1)
		    break;
      if ( upto == token->bufferLength() ){
        termBuffer = token->resizeTermBuffer(token->bufferLength() + 8);
      }
	    _tcsncpy(termBuffer + upto, readBuffer, rd);
      upto += rd;
    }
    if ( token->bufferLength() < upto + 1 ){
      termBuffer=token->resizeTermBuffer(token->bufferLength() + 8);
    }
    termBuffer[upto]=0;
    token->setTermLength(upto);
    return token;
  }
  return NULL;
}
void KeywordTokenizer::reset(CL_NS(util)::Reader* input)
{
	Tokenizer::reset(input);
	this->done = false;
}


LengthFilter::LengthFilter(TokenStream* in, const size_t _min, const size_t _max):
    TokenFilter(in)
{
    this->_min = _min;
    this->_max = _max;
}

Token* LengthFilter::next(Token* token)
{
    // return the first non-stop word found
    while ( input->next(token) )
    {
        size_t len = token->termLength();
        if (len >= _min && len <= _max)
            return token;
        // note: else we ignore it but should we index each part of it?
    }
    // reached EOS -- return null
    return NULL;
}


CLTCSetList* WordlistLoader::getWordSet(const char* wordfilePath, const char* enc, CLTCSetList* stopTable)
{
	if ( enc == NULL )
		enc = "ASCII";
	CL_NS(util)::FileReader* reader = NULL;
	try {
		reader = _CLNEW CL_NS(util)::FileReader(wordfilePath, enc, LUCENE_DEFAULT_TOKEN_BUFFER_SIZE);
		stopTable = getWordSet(reader, stopTable);
	}
	_CLFINALLY (
		if (reader != NULL) {
			//reader->close();
			_CLLDELETE(reader);
		}
	);
	return stopTable;
}

//static
CLTCSetList* WordlistLoader::getWordSet(CL_NS(util)::Reader* reader, CLTCSetList* stopTable, const bool bDeleteReader)
{
	if (!stopTable)
		stopTable = _CLNEW CLTCSetList(true);

	TCHAR* word = NULL;
	try {
		word = _CL_NEWARRAY(TCHAR, LUCENE_DEFAULT_TOKEN_BUFFER_SIZE);
		while (reader->readLine(word, LUCENE_DEFAULT_TOKEN_BUFFER_SIZE) > 0) {
			stopTable->insert( STRDUP_TtoT(CL_NS(util)::Misc::wordTrim(word)));
		}
	}
	_CLFINALLY (
		if (bDeleteReader && reader != NULL) {
			//reader->close();
			_CLDELETE(reader);
		}
		_CLDELETE_ARRAY(word);
	);
	return stopTable;
}


CL_NS_END
