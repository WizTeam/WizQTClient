/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CJKAnalyzer.h"
#include "CLucene/util/CLStreams.h"

CL_NS_DEF2(analysis,cjk)
CL_NS_USE(analysis)
CL_NS_USE(util)


const TCHAR* CJKTokenizer::tokenTypeSingle = _T("single");
const TCHAR* CJKTokenizer::tokenTypeDouble = _T("double");

CJKTokenizer::CJKTokenizer(Reader* in):
	Tokenizer(in)
{
	tokenType = Token::getDefaultType();
	offset = 0;
	bufferIndex = 0;
	dataLen = 0;
	preIsTokened = false;
	ignoreSurrogates = true;
}

CL_NS(analysis)::Token* CJKTokenizer::next(Token* token){
    /** how many character(s) has been stored in buffer */
    int32_t length = 0;

    /** the position used to create Token */
    int32_t start = offset;

    while (true) {
        /** current character */
        clunichar c;
	int charlen = 1;

        offset++;

        if (bufferIndex >= dataLen) {
            dataLen = input->read(ioBuffer, 1, LUCENE_IO_BUFFER_SIZE);
            bufferIndex = 0;
        }

        if (dataLen == -1) {
            if (length > 0) {
                if (preIsTokened == true) {
                    length = 0;
                    preIsTokened = false;
                }

                break;
            } else {
                return NULL;
            }
        } else {
            //get current character
            c = ioBuffer[bufferIndex++];
        }

		//to support surrogates, we'll need to convert the incoming utf16 into
		//ucs4(c variable). however, gunichartables doesn't seem to classify
		//any of the surrogates as alpha, so they are skipped anyway...
		//so for now we just convert to ucs4 so that we dont corrupt the input.
		if ( c >= 0xd800 || c <= 0xdfff ){
			clunichar c2 = ioBuffer[bufferIndex];
			if ( c2 >= 0xdc00 && c2 <= 0xdfff ){
				bufferIndex++;
				offset++;
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

            // if the current character is a letter or "_" "+" "#"
			if (_istalnum(c) || ((c == '_') || (c == '+') || (c == '#')) ) {
                if (length == 0) {
                    // "javaC1C2C3C4linux" <br>
                    //      ^--: the current character begin to token the ASCII
                    // letter
                    start = offset - 1;
                } else if (tokenType == tokenTypeDouble) {
                    // "javaC1C2C3C4linux" <br>
                    //              ^--: the previous non-ASCII
                    // : the current character
                    offset-=charlen;
                    bufferIndex-=charlen;
                    tokenType = tokenTypeSingle;

                    if (preIsTokened == true) {
                        // there is only one non-ASCII has been stored
                        length = 0;
                        preIsTokened = false;

                        break;
                    } else {
                        break;
                    }
                }

                // store the LowerCase(c) in the buffer
                buffer[length++] = _totlower((TCHAR)c);
				tokenType = tokenTypeSingle;

                // break the procedure if buffer overflowed!
                if (length == LUCENE_MAX_WORD_LEN) {
                    break;
                }
            } else if (length > 0) {
                if (preIsTokened == true) {
                    length = 0;
                    preIsTokened = false;
                } else {
                    break;
                }
            }
        } else {
            // non-ASCII letter, eg."C1C2C3C4"
			if ( _istalpha(c) || (!ignoreSurrogates && c>=0x10000) ) {
                if (length == 0) {
                    start = offset - 1;
                    
					if ( c < 0x00010000L )
						buffer[length++] = (TCHAR)c;
					else{
						clunichar ucs4 = c - 0x00010000L;
						buffer[length++] = (TCHAR)((ucs4 >> 10) & 0x3ff) | 0xd800;
						buffer[length++] = (TCHAR)((ucs4 >>  0) & 0x3ff) | 0xdc00;
					}

                    tokenType = tokenTypeDouble;
                } else {
                    if (tokenType == tokenTypeSingle) {
                        offset-=charlen;
                        bufferIndex-=charlen;

                        //return the previous ASCII characters
                        break;
                    } else {
						if ( c < 0x00010000L )
							buffer[length++] = (TCHAR)c;
						else{
							clunichar ucs4 = c - 0x00010000L;
							buffer[length++] = (TCHAR)((ucs4 >> 10) & 0x3ff) | 0xd800;
							buffer[length++] = (TCHAR)((ucs4 >>  0) & 0x3ff) | 0xdc00;
						}
						tokenType = tokenTypeDouble;

                        if (length >= 2) {
                            offset-=charlen;
                            bufferIndex-=charlen;
                            preIsTokened = true;

                            break;
                        }
                    }
                }
            } else if (length > 0) {
                if (preIsTokened == true) {
                    // empty the buffer
                    length = 0;
                    preIsTokened = false;
                } else {
                    break;
                }
            }
        }
    }

	buffer[length]='\0';
	token->set(buffer,start, start+length, tokenType);
	return token;
}

CL_NS_END2
