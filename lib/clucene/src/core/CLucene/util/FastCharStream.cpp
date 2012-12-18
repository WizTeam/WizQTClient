/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_FastCharStream.h"

#include "CLucene/util/CLStreams.h"

CL_NS_DEF(util)

const int32_t FastCharStream::maxRewindSize = LUCENE_MAX_WORD_LEN*2;

  FastCharStream::FastCharStream(BufferedReader* reader):
    pos(0),
    rewindPos(0),
	resetPos(0),
	col(1),
	line(1),
	input(reader)
  {
    input->setMinBufSize(maxRewindSize);
  }
  FastCharStream::~FastCharStream(){
  }

  void FastCharStream::reset()
  {
      pos = 0;
      rewindPos = 0;
      resetPos = 0;
      col = 1;
      line = 1;
      input->setMinBufSize(maxRewindSize);	
  }

  void FastCharStream::readChar(TCHAR &c) {
	try{
		int32_t r = input->read();
		if ( r == -1 )
			input = NULL;
		c = r;
	}catch(CLuceneError& err){
		if ( err.number() == CL_ERR_IO )
			input = 0;
		throw err;
	}
  }
  int FastCharStream::GetNext()
  {
    if (input == 0 ) // end of file
    {
      _CLTHROWA(CL_ERR_IO,"warning : FileReader.GetNext : Read TCHAR over EOS.");
    }
    // this is rather inefficient
    // implementing the functions from the java version of
    // charstream will be much more efficient.
	++pos;
    TCHAR ch;
    readChar(ch);

    if (input == NULL) { // eof
        return -1;
    }
    if (rewindPos == 0) {
        col += 1;
        if(ch == '\n') {
            line++;
            col = 1;
        }
    } else {
        rewindPos--;
    }
    return ch;
  }

  void FastCharStream::UnGet(){
//      printf("UnGet \n");
    if (input == 0) 
		return;
    if ( pos == 0 ) {
      _CLTHROWA(CL_ERR_IO,"error : No character can be UnGet");
    }
    rewindPos++;

	input->reset(pos-1);
    pos--;
  }

  int FastCharStream::Peek() {
    int c = GetNext();
    UnGet();
    return c;
  }

  bool FastCharStream::Eos() const {
	return input==NULL;
  }

  int32_t FastCharStream::Column() const {
	return col;
  }

  int32_t FastCharStream::Line() const {
	return line;
  }
CL_NS_END
