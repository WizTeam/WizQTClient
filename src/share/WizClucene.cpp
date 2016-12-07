#include "WizClucene.h"

#include "WizMisc.h"
#ifdef _T
#undef _T
#endif

#include "CLucene.h"
#include "CLucene/analysis/standard/StandardTokenizer.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"
#include "CLucene/analysis/standard/StandardFilter.h"

#include <map>
#include <deque>
#include <fstream>
#include <cassert>

#include <QStringList>
#include <QDebug>

/*
beacuase lucene default LUCENE_MAX_WORD_LEN = 128 to tokenize field,
change it to 1 make lucene::analysis::standard::StandardTokenizer class
have ability to parse CJK as normal words, please refer
to CLConfig.h for detailed info.
*/

/* FIXME: wchar_t on windows occupy 2 bytes (utf-16),
and wchar_t on unix occupy 4 bytes (ucs4), we should
use different tokenizer to analysis unicode chars
or convert utf-16 to ucs4 on windows before tokenize
*/
#if __SIZEOF_WCHAR_T__ == 4 || __WCHAR_MAX__ > 0x10000
typedef lucene::analysis::standard::StandardTokenizer WizTokenizer;
#endif

/*
 * code port from Wiz for windows
 */

class CJKTokenizer3 : public lucene::analysis::Tokenizer
{
public:
        CJKTokenizer3(lucene::util::Reader* in)
                : bufferIndex(0)
                , dataLen(0)
                , ioBuffer(new TCHAR[LUCENE_IO_BUFFER_SIZE])
                , input(in)
                , offset(0)
        {
        }

private:
        int bufferIndex;
        int dataLen;
        const TCHAR* ioBuffer;
        lucene::util::Reader* input;
        int offset;

#define _CJK                        (  (ch>=0x3040 && ch<=0x318f) || \
        (ch>=0x3300 && ch<=0x337f) || \
        (ch>=0x3400 && ch<=0x3d2d) || \
        (ch>=0x4e00 && ch<=0x9fff) || \
        (ch>=0xf900 && ch<=0xfaff) || \
        (ch>=0xac00 && ch<=0xd7af) ) //korean

public:
        lucene::analysis::Token* next(lucene::analysis::Token* token)
        {
                TCHAR ch = 0;

                while (true)
                {
                        offset++;
                        //
                        if (bufferIndex >= dataLen)
                        {
                                dataLen = input->read(ioBuffer, 1, LUCENE_IO_BUFFER_SIZE);
                                bufferIndex = 0;
                        }
                        //
                        if (dataLen == -1)
                        {
                                return NULL;
                        }
                        else
                        {
                                ch = ioBuffer[bufferIndex++];
                        }

                        if (_CJK)
                        {
                                break;
                        }

                }
                TCHAR currentWord[2] = {ch, 0};
                token->set(currentWord, offset - 1, offset);
                return token;
        }
};


const wchar_t* tokenTypeSingle = L"single";
const wchar_t* tokenTypeDouble = L"double";

class CJKTokenizer: public lucene::analysis::Tokenizer
{
private:
	/** word offset, used to imply which character(in ) is parsed */
	int32_t m_offset;

	/** the index used only for ioBuffer */
	int32_t m_bufferIndex;

	/** data length */
	int32_t m_dataLen;

	/**
	* character buffer, store the characters which are used to compose <br>
	* the returned Token
	*/
    TCHAR m_buffer[LUCENE_MAX_WORD_LEN + 1];

	/**
	* I/O buffer, used to store the content of the input(one of the <br>
	* members of Tokenizer)
	*/
	const TCHAR* m_ioBuffer;

	/** word type: single=>ASCII  double=>non-ASCII word=>default */
	const TCHAR* m_tokenType;

	/**
	* tag: previous character is a cached double-byte character  "C1C2C3C4"
	* ----(set the C1 isTokened) C1C2 "C2C3C4" ----(set the C2 isTokened)
	* C1C2 C2C3 "C3C4" ----(set the C3 isTokened) "C1C2 C2C3 C3C4"
	*/
	bool m_preIsTokened;


	bool m_ignoreSurrogates;

public:
    CJKTokenizer(lucene::util::Reader* in)
		: Tokenizer(in)
	{
        m_tokenType = lucene::analysis::Token::getDefaultType();
		m_offset = 0;
		m_bufferIndex = 0;
		m_dataLen = 0;
		m_preIsTokened = false;
		m_ignoreSurrogates = true;
	}

    lucene::analysis::Token* next(lucene::analysis::Token* token)
	{
		while (1)
		{
			int retTokenLength = 0;
            lucene::analysis::Token* ret = next1(token, retTokenLength);
			if (!ret)
                return NULL;

			if (retTokenLength > 0)
				return ret;
		}
	}

    lucene::analysis::Token* next1(lucene::analysis::Token* token, int& retTokenLength)
	{
		int32_t length = 0;
		retTokenLength = 0;

		/** the position used to create Token */
		int32_t start = m_offset;

		while (true) {
			/** current character */
            TCHAR c;
            int charlen = 1;

			m_offset++;

			if (m_bufferIndex >= m_dataLen) {
				m_dataLen = input->read(m_ioBuffer, 1, LUCENE_IO_BUFFER_SIZE);
				m_bufferIndex = 0;
			}

			if (m_dataLen == -1) {
				if (length > 0) {
					if (m_preIsTokened == true) {
						length = 0;
						m_preIsTokened = false;
					}

					break;
				} else {
					return NULL;
				}
			} else {
				//get current character
				c = m_ioBuffer[m_bufferIndex++];
			}

			//to support surrogates, we'll need to convert the incoming utf16 into
			//ucs4(c variable). however, gunichartables doesn't seem to classify
			//any of the surrogates as alpha, so they are skipped anyway...
			//so for now we just convert to ucs4 so that we dont corrupt the input.
            if ( c >= 0xd800 || c <= 0xdfff ){
                TCHAR c2 = m_ioBuffer[m_bufferIndex];
				if ( c2 >= 0xdc00 && c2 <= 0xdfff ){
					m_bufferIndex++;
					m_offset++;
					charlen=2;

					c = (((c & 0x03ffL) << 10) | ((c2 & 0x03ffL) <<  0)) + 0x00010000L;
				}
			}

			//if the current character is ASCII or Extend ASCII
			if ((c <= 0xFF) //is BASIC_LATIN
                || (c>=0xFF00 && c<=0xFFEF) //ascii >0x74 cast to unsigned...
				) {
					if (c >= 0xFF00) {
						//todo: test this... only happens on platforms where char is signed, i think...
						/** convert  HALFWIDTH_AND_FULLWIDTH_FORMS to BASIC_LATIN */
						c -= 0xFEE0;
					}

					////不是CJK////
					// if the current character is a letter or "_" "+" "#"
                    if (iswalnum(c) || ((c == '_') || (c == '+') || (c == '#')) ) {
						if (length == 0) {
							// "javaC1C2C3C4linux" <br>
							//      ^--: the current character begin to token the ASCII
							// letter
							start = m_offset - 1;
						} else if (m_tokenType == tokenTypeDouble) {
							// "javaC1C2C3C4linux" <br>
							//              ^--: the previous non-ASCII
							// : the current character
							m_offset-=charlen;
							m_bufferIndex-=charlen;
							m_tokenType = tokenTypeSingle;

							if (m_preIsTokened == true) {
								// there is only one non-ASCII has been stored
								length = 0;
								m_preIsTokened = false;

								break;
							} else {
								break;
							}
						}

						// store the LowerCase(c) in the buffer
                        m_buffer[length++] = towlower((TCHAR)c);
						m_tokenType = tokenTypeSingle;

						// break the procedure if buffer overflowed!
						if (length == LUCENE_MAX_WORD_LEN) {
							break;
						}
					} else if (length > 0) {
						if (m_preIsTokened == true) {
							length = 0;
							m_preIsTokened = false;
						} else {
							break;
						}
					}
			} 
			else 
			{
				// CJK
				// non-ASCII letter, eg."C1C2C3C4"
                if ( iswalpha(c) || (!m_ignoreSurrogates && c>=0x10000) )
				{
					if (length == 0) {
                        start = m_offset - 1;
                        if ( c < 0x00010000L )
                            m_buffer[length++] = (TCHAR)c;
                        else{
                            TCHAR ucs4 = c - 0x00010000L;
                            m_buffer[length++] = (TCHAR)((ucs4 >> 10) & 0x3ff) | 0xd800;
                            m_buffer[length++] = (TCHAR)((ucs4 >>  0) & 0x3ff) | 0xdc00;
                        }

						m_tokenType = tokenTypeDouble;
					} else {
						if (m_tokenType == tokenTypeSingle) {
							m_offset-=charlen;
							m_bufferIndex-=charlen;

							//return the previous ASCII characters
							break;
						} else {
							if ( c < 0x00010000L )
								m_buffer[length++] = (TCHAR)c;
							else{
                                TCHAR ucs4 = c - 0x00010000L;
								m_buffer[length++] = (TCHAR)((ucs4 >> 10) & 0x3ff) | 0xd800;
								m_buffer[length++] = (TCHAR)((ucs4 >>  0) & 0x3ff) | 0xdc00;
							}
                            m_tokenType = tokenTypeDouble;

							if (length >= 2) {
								m_offset-=charlen;
								m_bufferIndex-=charlen;
								m_preIsTokened = true;

								break;
							}
						}
					}
				} 
				else if (length > 0) 
				{
					if (m_preIsTokened == true) 
					{
						// empty the buffer
						length = 0;
						m_preIsTokened = false;
					}
					else 
					{
						break;
					}
				}
			}
		}

        m_buffer[length]=L'\0';
		token->set(m_buffer,start, start+length, m_tokenType);
		retTokenLength = length;
		return token;
	}

	bool getIgnoreSurrogates()
	{
		return m_ignoreSurrogates;
    }

	void setIgnoreSurrogates(bool ignoreSurrogates)
	{
		this->m_ignoreSurrogates = ignoreSurrogates;
	}
};


class LanguageBasedAnalyzer: public lucene::analysis::Analyzer
{
    wchar_t lang[100];
    bool stem;

public:
    LanguageBasedAnalyzer(const wchar_t* language=NULL, bool stem=true)
    {
        if (language == NULL)
            memset(lang, 0, 100);
        else
            wcsncpy(lang, language,100);

        this->stem = stem;
    }

    void setLanguage(const wchar_t* language)
    {
        wcsncpy(lang, language, 100);
    }

    void setStem(bool stem)
    {
        this->stem = stem;
    }

    lucene::analysis::TokenStream* tokenStream(const wchar_t* fieldName, lucene::util::Reader* reader)
    {
        lucene::analysis::TokenStream* ret = NULL;
        if (wcscmp(lang, L"cjk")==0)
        {
            ret = new CJKTokenizer(reader);
        }
        else if (wcscmp(lang, L"cjk3")==0)
        {
            ret = new CJKTokenizer3(reader);
        }
        else
        {
            lucene::util::BufferedReader* bufferedReader = reader->__asBufferedReader();
            if (bufferedReader == NULL)
                ret =  new lucene::analysis::standard::StandardTokenizer(new lucene::util::FilteredBufferedReader(reader, false), true);
            else
                ret = new lucene::analysis::standard::StandardTokenizer(bufferedReader);

            ret = new lucene::analysis::standard::StandardFilter(ret,true);

            if ( stem ) //hmm... this could be configured seperately from stem
                ret = new lucene::analysis::ISOLatin1AccentFilter(ret, true); //todo: this should really only be applied to latin languages...

            //lower case after the latin1 filter
            ret = new lucene::analysis::LowerCaseFilter(ret,true);
        }

        return ret;
    }
};


static std::vector<std::wstring> _arrayLog;

void TOLOG(const wchar_t* lpsz)
{
	if(_arrayLog.size() > 10)
		_arrayLog.clear();
	//
	_arrayLog.push_back(lpsz);
}


void WizPathRemoveBackslash(std::wstring& strPath)
{
    if (strPath.empty()) {
        return;
    }

    wchar_t ch = strPath[strPath.length() - 1];
    if (ch == '\\' || ch == '/') {
        strPath.erase(strPath.length() - 1);
    }
}

std::string WizW2A(const std::wstring& str)
{
    int bufferSize = str.length() * 2 + 1;
	char* szBuffer = new char[bufferSize];

    wcstombs(szBuffer, str.c_str(), bufferSize);

	std::string ret = szBuffer;
	delete [] szBuffer;
	return ret;
}

#define WIZTOOLS_FTS_MAX_FILE_LENGTH		(5 * 1024 * 1024)


struct WIZFTSDATA
{
        lucene::index::IndexWriter* writer;
        LanguageBasedAnalyzer an;
        LanguageBasedAnalyzer an3;

        WIZFTSDATA() : writer(NULL)
        {
                an.setLanguage(L"cjk");
                an3.setLanguage(L"cjk3");
        }

        ~WIZFTSDATA()
        {
                if (writer)
                {
                        writer->close();
                        delete writer;
                }
        }
};

//struct WIZFTSDATA
//{
//	lucene::index::IndexWriter* writer;
//	LanguageBasedAnalyzer an;
//
//    WIZFTSDATA()
//    {
//        writer = NULL;
//        an.setLanguage(L"cjk");
//    }
//
//	~WIZFTSDATA()
//    {
//        if (writer) {
//			writer->close();
//			delete writer;
//		}
//	}
//};

bool WizCluceneSearch::beginUpdateDocument(const wchar_t* lpszIndexPath, void** ppHandle)
{
    std::wstring strIndexPath(lpszIndexPath);
    WizPathRemoveBackslash(strIndexPath);
    std::string strIndexPathA = WizW2A(strIndexPath);

    try {
        WIZFTSDATA* pData = new WIZFTSDATA();

        if (lucene::index::IndexReader::indexExists(strIndexPathA.c_str()) ) {
			//open exists
            if ( lucene::index::IndexReader::isLocked(strIndexPathA.c_str()) ) {
                TOLOG(L"Index was locked... unlocking it.");
				lucene::index::IndexReader::unlock(strIndexPathA.c_str());
			}
			pData->writer = _CLNEW lucene::index::IndexWriter(strIndexPathA.c_str(), &pData->an, false);
        } else {
            //create
			pData->writer = _CLNEW lucene::index::IndexWriter(strIndexPathA.c_str(),&pData->an, true);
        }

        if (pData->writer) {
			pData->writer->setMaxFieldLength(WIZTOOLS_FTS_MAX_FILE_LENGTH);
			*ppHandle = pData;
            return true;
        } else {
			delete pData;
            TOLOG(L"Failed to get writer!");
            return false;
        }

    } catch (CLuceneError& e) {
        TOLOG(L"Indexing exception in WizToolsFTSUpdateDocument");
		TOLOG(e.twhat());
        return false;

    } catch (...) {
        TOLOG(L"Unknown exception in WizToolsFTSUpdateDocument");
        return false;
	}
}

bool WizCluceneSearch::endUpdateDocument(void* pHandle)
{
	WIZFTSDATA* pData = (WIZFTSDATA*)pHandle;
    if (!pData) {
        return false;
    }

    try {
        pData->writer->optimize();
        pData->writer->close();
		delete pData;
        return true;

    } catch (CLuceneError& e) {
        TOLOG(L"Indexing exception in WizFTSEndUpdateDocument3");
		TOLOG(e.twhat());
        return false;

    } catch (...) {
        TOLOG(L"Unknown exception in WizFTSEndUpdateDocument3");
        return false;
	}
}

bool WizCluceneSearch::updateDocument(void* pHandle,
                                       const wchar_t* lpszKbGUID,
                                       const wchar_t* lpszDocumentID,
                                       const wchar_t* lpszTitle,
                                       const wchar_t* lpszText)
{
    Q_UNUSED(lpszTitle);

    assert(pHandle && lpszKbGUID && lpszDocumentID && lpszTitle && lpszText);

    WIZFTSDATA* pData = (WIZFTSDATA*)pHandle;

    std::wstring strKbGUID(lpszKbGUID);
    std::wstring strDocumentID(lpszDocumentID);
    //std::wstring strTitle(lpszTitle);
    std::wstring strText(lpszText);

    try {

        {
            lucene::index::Term* term = _CLNEW lucene::index::Term(L"documentid", strDocumentID.c_str());
            if (term)
            {
                pData->writer->deleteDocuments(term);
            }
        }

        {
            lucene::index::Term* term2 = _CLNEW lucene::index::Term(L"documentid2", strDocumentID.c_str());
            if (term2)
            {
                pData->writer->deleteDocuments(term2);
            }
        }

    } catch (CLuceneError& e) {
        TOLOG(L"Indexing exception in deleteDocuments");
		TOLOG(e.twhat());
    } catch (...) {
        TOLOG(L"Unknown exception in deleteDocuments");
    }

    {
        lucene::document::Document doc;
        doc.add( *_CLNEW lucene::document::Field(L"documentid", strDocumentID.c_str(), lucene::document::Field::STORE_YES | lucene::document::Field::INDEX_UNTOKENIZED ) );
        doc.add( *_CLNEW lucene::document::Field(L"kbguid", strKbGUID.c_str(), lucene::document::Field::STORE_YES | lucene::document::Field::INDEX_UNTOKENIZED ) );
        doc.add( *_CLNEW lucene::document::Field(L"contents", strText.c_str(), lucene::document::Field::STORE_NO | lucene::document::Field::INDEX_TOKENIZED) );
        pData->writer->addDocument(&doc, &pData->an);
    }

    {
        lucene::document::Document doc;
        doc.add( *_CLNEW lucene::document::Field(L"documentid2", strDocumentID.c_str(), lucene::document::Field::STORE_YES | lucene::document::Field::INDEX_UNTOKENIZED ) );
        doc.add( *_CLNEW lucene::document::Field(L"kbguid", strKbGUID.c_str(), lucene::document::Field::STORE_YES | lucene::document::Field::INDEX_UNTOKENIZED ) );
        doc.add( *_CLNEW lucene::document::Field(L"contents", strText.c_str(), lucene::document::Field::STORE_NO | lucene::document::Field::INDEX_TOKENIZED) );
        pData->writer->addDocument(&doc, &pData->an3);
    }

    return true;
}

bool WizCluceneSearch::searchDocument(const wchar_t* lpszIndexPath,
                                       const wchar_t* lpszKeywords)
{
    std::wstring strIndexPath(lpszIndexPath);
    WizPathRemoveBackslash(strIndexPath);
    std::string strIndexPathA = WizW2A(strIndexPath);

    const wchar_t* arrAnalyzer[] = {
        L"cjk",
        L"cjk3",
        NULL
    };

    //const wchar_t* fields[] = {L"title", L"contents", NULL};

    std::map<std::string, std::string> mapGUIDOdd;
    std::map<std::string, std::string> mapGUIDEven;
    QStringList listKey = QString::fromWCharArray(lpszKeywords).split(getWizSearchSplitChar());
    if (listKey.isEmpty())
        return false;

    try {
        lucene::search::IndexSearcher searcher(strIndexPathA.c_str());

        for (int keyIndex = 0; keyIndex < listKey.count(); keyIndex++)
        {
            std::wstring strKeywords = listKey.at(keyIndex).toStdWString();

            std::map<std::string, std::string>& mapGUIDOld = keyIndex % 2 == 0 ? mapGUIDOdd : mapGUIDEven;
            std::map<std::string, std::string>& mapGUIDCur = keyIndex % 2 == 0 ? mapGUIDEven : mapGUIDOdd;

            for (int i = 0; ; i++)
            {
                if (!arrAnalyzer[i])
                    break;

                //for (int j = 0; ; j++) {
                //    if (!fields[j]) break;

                LanguageBasedAnalyzer analyzer(arrAnalyzer[i]);
                lucene::search::Query* query = lucene::queryParser::QueryParser::parse(strKeywords.c_str(), L"contents", &analyzer);

                qDebug() << "Search document contents";

                if (!query) {
                    continue;
                }

                lucene::search::Hits* hits = searcher.search(query);
                if (!hits) {
                    _CLDELETE(query);
                    continue;
                }

                for (size_t j = 0;j < hits->length(); j++ ) {
                    lucene::document::Document* doc = &hits->doc(j);
                    const TCHAR* kbid = doc->get(L"kbguid");
                    const TCHAR* docid = doc->get(L"documentid");
                    if (!docid) {
                        docid = doc->get(L"documentid2");
                    }

                    if (docid) {
                        std::string strDocumentGUID = ::WizW2A(docid);
                        std::string strDocumentKBGUID = ::WizW2A(kbid);
                        if (mapGUIDCur.find(strDocumentGUID) == mapGUIDCur.end()) {
                            if (keyIndex > 0 && mapGUIDOld.find(strDocumentGUID) != mapGUIDOld.end())
                            {
                                mapGUIDCur[strDocumentGUID] = strDocumentKBGUID;
                                qDebug() << "Document finded  guid: " << QString(strDocumentGUID.data());
                            }
                            else if (keyIndex == 0)
                            {
                                mapGUIDCur[strDocumentGUID] = strDocumentKBGUID;
                                qDebug() << "Document finded  guid: " << QString(strDocumentGUID.data());
                            }

                        }
                    }
                }

                _CLDELETE(query);
                //}
            }
            mapGUIDOld.clear();
        }

        std::map<std::string, std::string>& mapGUIDCur =
                (listKey.count() % 2 == 0) ?  mapGUIDOdd : mapGUIDEven;
        std::map<std::string, std::string>::iterator iterator;
        for (iterator = mapGUIDCur.begin(); iterator != mapGUIDCur.end(); iterator++)
        {
            std::string strDocumentGUID = iterator->first;
            std::string strDocumentKBGUID = iterator->second;
            onSearchProcess(strDocumentKBGUID, strDocumentGUID, "");
        }

        searcher.close();
        onSearchEnd();
        return true;

    } catch (CLuceneError& e) {
		TOLOG(e.twhat());
        return false;

    } catch (...) {
        return false;
	}
}

bool WizCluceneSearch::deleteDocument(const wchar_t* lpszIndexPath,
                                       const wchar_t* lpszDocumentID)
{
    std::wstring strIndexPath(lpszIndexPath);
    WizPathRemoveBackslash(strIndexPath);
    std::string strIndexPathA = WizW2A(strIndexPath);
    std::wstring strWizDocumentID(lpszDocumentID);

    try {
        bool ret = lucene::index::IndexReader::indexExists(strIndexPathA.c_str());
        if (!ret) {
            return false;
        }
		
        lucene::index::IndexReader* reader = lucene::index::IndexReader::open(strIndexPathA.c_str());
        if (!reader) {
            TOLOG(L"Failed to get index reader!");
            return false;
        }

        {
            lucene::index::Term* term = _CLNEW lucene::index::Term(L"documentid", strWizDocumentID.c_str());
            if (term)
            {
                reader->deleteDocuments(term);
            }
        }

        {
            lucene::index::Term* term = _CLNEW lucene::index::Term(L"documentid2", strWizDocumentID.c_str());
            if (term)
            {
                reader->deleteDocuments(term);
            }
        }

        reader->close();
        _CLDELETE(reader);
        return true;

    } catch (CLuceneError& e) {
        TOLOG(L"Indexing exception in WizToolsFTSDeleteDocument");
		TOLOG(e.twhat());
        return false;

    } catch (...) {
        TOLOG(L"Unknown exception in WizToolsFTSDeleteDocument");
        return false;
	}
}


#ifdef WIZ_FTS_ENABLE_TEST

void _testCJK(const char* astr, bool ignoreSurrogates = true) 
{
	lucene::util::SimpleInputStreamReader r(new lucene::util::AStringReader(astr), lucene::util::SimpleInputStreamReader::UTF8);

	TCHAR szBuffer[1024] = {0};
    //::MultiByteToWideChar(CP_UTF8, 0, astr, -1, szBuffer, 1024);
    mbstowcs(szBuffer, astr, 1024);

	CJKTokenizer* tokenizer = _CLNEW CJKTokenizer(&r);
	tokenizer->setIgnoreSurrogates(ignoreSurrogates);
	int pos = 0;
    lucene::analysis::Token tok;

	while (1) {
		if (tokenizer->next(&tok) == NULL)
			break;
        const TCHAR* buffer = tok.termBuffer();
		pos++;
	}
	//
	if (tokenizer->next(&tok))
		assert(false);

	_CLDELETE(tokenizer);
}


void testCJK()
{
    const wchar_t* szTest = L"flash本地摄像头录像自动上传保存到服务器";
    char szBuffer[1024] = {0};
    wcstombs(szBuffer, szTest, 1024);
	_testCJK(szBuffer);
}



void testLanguageBasedAnalyzer() {
	LanguageBasedAnalyzer a;
	CL_NS(util)::StringReader reader("he abhorred accentueren");
	reader.setMinBufSize(50);
	lucene::analysis::TokenStream* ts;
    lucene::analysis::Token t;

	//test with english
	a.setLanguage("English");
	a.setStem(false);
	ts = a.tokenStream("contents", &reader);

	//CLUCENE_ASSERT(ts->next(&t) != NULL);
	//CLUCENE_ASSERT(_tcscmp(t.termBuffer(), "he") == 0);
	//CLUCENE_ASSERT(ts->next(&t) != NULL);
	//CLUCENE_ASSERT(_tcscmp(t.termBuffer(), "abhorred") == 0);
	//_CLDELETE(ts);

	//now test with dutch
	reader.reset(0);
	a.setLanguage("Dutch");
	a.setStem(true);
	ts = a.tokenStream("contents", &reader);

	//CLUCENE_ASSERT(ts->next(&t) != NULL);
	//CLUCENE_ASSERT(_tcscmp(t.termBuffer(), "he") == 0);
	//CLUCENE_ASSERT(ts->next(&t) != NULL);
	//CLUCENE_ASSERT(_tcscmp(t.termBuffer(), "abhorred") == 0);
	//CLUCENE_ASSERT(ts->next(&t) != NULL);
	//CLUCENE_ASSERT(_tcscmp(t.termBuffer(), "accentuer") == 0);
	//_CLDELETE(ts);
}

int WizTest()
{
    const wchar_t* lpszIndexPath = "/Users/Albert/test/";
	WIZFTSDATA* pData = new WIZFTSDATA();
	//
	try
	{
		std::wstring strIndexPath(lpszIndexPath);
		WizPathRemoveBackslash(strIndexPath);
		//
		std::string strIndexPathA = WizW2A(strIndexPath);
		//
		if (lucene::index::IndexReader::indexExists(strIndexPathA.c_str()) )
		{
			//open exists
			if ( lucene::index::IndexReader::isLocked(strIndexPathA.c_str()) )
			{
				TOLOG("Index was locked... unlocking it.");
				lucene::index::IndexReader::unlock(strIndexPathA.c_str());
			}
			pData->writer = _CLNEW lucene::index::IndexWriter(strIndexPathA.c_str(), &pData->an, false);
		}
		else
		{
			//create
			pData->writer = _CLNEW lucene::index::IndexWriter(strIndexPathA.c_str(),&pData->an, true);
		}
		//
		if (pData->writer)
		{
			pData->writer->setMaxFieldLength(WIZTOOLS_FTS_MAX_FILE_LENGTH);
		}
		else
		{
			delete pData;
			TOLOG("Failed to get writer!");
            return false;
		}
	}
	catch (CLuceneError& e)
	{
		TOLOG("Indexing exception in WizToolsFTSUpdateDocument");
		TOLOG(e.twhat());
        return false;
	}
	catch (...)
	{
		TOLOG("Unknown exception in WizToolsFTSUpdateDocument");
        return false;
	}

	/////////////////////////////////////////////////////////
	if (!pData)
        return false;
	//
	std::wstring strDocumentID(L"123");
	std::wstring strWizDocumentID(L"123");
	std::wstring strKbGUID(L"123");
	//
	std::wstring strURL(L"123");
	std::wstring strText(L"Connection Point");
	//
	try
	{
		{
			lucene::index::Term* term = _CLNEW lucene::index::Term("wizdocumentid", strWizDocumentID.c_str());
			if (term)
			{
				pData->writer->deleteDocuments(term);
			}
		}
		//
		{
			lucene::index::Term* term2 = _CLNEW lucene::index::Term("wizdocumentid_2", strWizDocumentID.c_str());
			if (term2)
			{
				pData->writer->deleteDocuments(term2);
			}
		}
	}
	catch (CLuceneError& e)
	{
		TOLOG("Indexing exception in deleteDocuments");
		TOLOG(e.twhat());
	}
	catch (...)
	{
		TOLOG("Unknown exception in deleteDocuments");
	}

	//
	{
		lucene::document::Document doc;
		doc.add( *_CLNEW lucene::document::Field("wizdocumentid", strWizDocumentID.c_str(), lucene::document::Field::STORE_YES | lucene::document::Field::INDEX_UNTOKENIZED ) );
		doc.add( *_CLNEW lucene::document::Field("documentid", strDocumentID.c_str(), lucene::document::Field::STORE_YES | lucene::document::Field::INDEX_UNTOKENIZED ) );
		doc.add( *_CLNEW lucene::document::Field("kbguid", strKbGUID.c_str(), lucene::document::Field::STORE_YES | lucene::document::Field::INDEX_UNTOKENIZED ) );
		doc.add( *_CLNEW lucene::document::Field("url", strURL.c_str(), lucene::document::Field::STORE_YES | lucene::document::Field::INDEX_UNTOKENIZED ) );
		//
        doc.add( *_CLNEW lucene::document::Field("contents", strText.c_str(), lucene::document::Field::STORE_NO | lucene::document::Field::INDEX_TOKENIZED) );
		//
		pData->writer->addDocument(&doc, &pData->an);
	}
	//
	//
	//
	//
#ifdef WIZ_FTS_ENABLE_CJK2	
	{
		lucene::document::Document doc;
		doc.add( *_CLNEW lucene::document::Field("wizdocumentid_2", strWizDocumentID.c_str(), lucene::document::Field::STORE_YES | lucene::document::Field::INDEX_UNTOKENIZED ) );
		doc.add( *_CLNEW lucene::document::Field("documentid", strDocumentID.c_str(), lucene::document::Field::STORE_YES | lucene::document::Field::INDEX_UNTOKENIZED ) );
		doc.add( *_CLNEW lucene::document::Field("kbguid", strKbGUID.c_str(), lucene::document::Field::STORE_YES | lucene::document::Field::INDEX_UNTOKENIZED ) );
		doc.add( *_CLNEW lucene::document::Field("url", strURL.c_str(), lucene::document::Field::STORE_YES | lucene::document::Field::INDEX_UNTOKENIZED ) );
		//
		doc.add( *_CLNEW lucene::document::Field("contents", strText.c_str(), lucene::document::Field::STORE_NO | lucene::document::Field::INDEX_TOKENIZED) );
		//
		pData->writer->addDocument(&doc, &pData->an2);
	}
#endif
	
	if (!pData)
        return false;
	//
	try
	{
		pData->writer->optimize();
		pData->writer->close();
		//
		delete pData;
	}
	catch (CLuceneError& e)
	{
		TOLOG("Indexing exception in WizFTSEndUpdateDocument3");
		TOLOG(e.twhat());
        return false;
	}
	catch (...)
	{
		TOLOG("Unknown exception in WizFTSEndUpdateDocument3");
        return false;
	}

	///////////////////////////////////////////////////////
	try
	{
		std::wstring strIndexPath(lpszIndexPath);
		WizPathRemoveBackslash(strIndexPath);
		//
		std::string strIndexPathA = WizW2A(strIndexPath);
		//
		std::wstring strKeywords(L"Connection");
		//
		lucene::search::IndexSearcher s(strIndexPathA.c_str());
		//
		std::set<std::string> setGUIDs;
		//
        const TCHAR* arrAnalyzer[] = {
#ifdef WIZ_FTS_ENABLE_CJK2	
			"cjk2",
#endif
			"cjk", 
			NULL
		};
		//
		for (int i = 0; ; i++)
		{
			if (!arrAnalyzer[i])
				break;
			//
			LanguageBasedAnalyzer analyzer(arrAnalyzer[i]);
			lucene::search::Query* q = lucene::queryParser::QueryParser::parse(strKeywords.c_str(), "contents", &analyzer);
			if (q)
			{
				lucene::search::Hits* h = s.search(q);
				if (h)
				{
					for (size_t i = 0;i < h->length(); i++ )
					{
						lucene::document::Document* doc = &h->doc(i);
						//
						const TCHAR* url = doc->get("url");
						const TCHAR* docid = doc->get("documentid");
						const TCHAR* kbid = doc->get("kbguid");
						//
						if (docid)
						{
							std::string strDocumentGUID = ::WizW2A(docid);
							if (setGUIDs.find(strDocumentGUID) == setGUIDs.end())
							{
								setGUIDs.insert(strDocumentGUID);
							}
						}
					}
					//
					_CLDELETE(h);
				}
				_CLDELETE(q);
			}
		}
		//
		s.close();
		//
        return true;
	}
	catch (CLuceneError& e)
	{
		TOLOG(e.twhat());
        return false;
	}
	catch (...)
	{
        return false;
	}


	return 0;
}

std::wstring readAllFromReader(lucene::util::Reader* reader)
{
    std::deque<std::wstring> arr;
    //
    size_t len = 0;
    TCHAR szLine[1024] = {0};
    while (reader->readLine(szLine, 1024) > 0)
    {
        arr.push_back(szLine);
        //
        len += wcslen(szLine) + 1;
    }
    //
    std::wstring ret;
    ret.reserve(len + 1);
    //
    for (std::deque<std::wstring>::const_iterator it = arr.begin();
        it != arr.end();
        it++)
    {
        ret.append(*it);
        ret.append(L"\n");
    }
    //
    return ret;
}


#endif
