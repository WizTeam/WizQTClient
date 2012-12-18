/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_SegmentHeader.h"
#include "_SegmentTermEnum.h"

#include "Terms.h"
#include "_FieldInfos.h"
#include "Term.h"
#include "_TermInfo.h"
#include "_TermInfosWriter.h"

CL_NS_USE(store)
CL_NS_DEF(index)

	SegmentTermEnum::SegmentTermEnum(IndexInput* i, FieldInfos* fis, const bool isi):
		fieldInfos(fis){
	//Func - Constructor
	//Pre  - i holds a reference to an instance of IndexInput
	//       fis holds a reference to an instance of FieldInfos
	//       isi
	//Post - An instance of SegmentTermEnum has been created
		input		 = i;
		position     = -1;
		//Instantiate a Term with empty field, empty text and which is interned (see term.h what interned means)
	    _term         = _CLNEW Term;
		isIndex      = isi;
		termInfo     = _CLNEW TermInfo();
		indexPointer = 0;
		buffer       = NULL;
		bufferLength = 0;
		prev         = NULL;
		formatM1SkipInterval = 0;
		maxSkipLevels = 1;
		
		//Set isClone to false as the instance is not clone of another instance
		isClone      = false;


		int32_t firstInt = input->readInt();
    if (firstInt >= 0) {
         // original-format file, without explicit format version number
         format = 0;
         size = firstInt;

         // back-compatible settings
         indexInterval = 128;
         skipInterval = LUCENE_INT32_MAX_SHOULDBE; // switch off skipTo optimization

      } else {
         // we have a format version number
         format = firstInt;

         // check that it is a format we can understand
         if (format < TermInfosWriter::FORMAT){
            TCHAR err[30];
            _sntprintf(err,30,_T("Unknown format version: %d"), format);
            _CLTHROWT(CL_ERR_CorruptIndex,err);
         }

         size = input->readLong();                    // read the size
         
         if(format == -1){
            if (!isIndex) {
               indexInterval = input->readInt();
               formatM1SkipInterval = input->readInt();
            }
            // switch off skipTo optimization for file format prior to 1.4rc2 in order to avoid a bug in 
            // skipTo implementation of these versions
            skipInterval = LUCENE_INT32_MAX_SHOULDBE;
         }else{
            indexInterval = input->readInt();
            skipInterval = input->readInt();
            if ( format == -3 ) {
		// this new format introduces multi-level skipping
            	maxSkipLevels = input->readInt();
            }
         }
      }
	}

	SegmentTermEnum::SegmentTermEnum(const SegmentTermEnum& clone):
		fieldInfos(clone.fieldInfos)
	{
	//Func - Constructor
	//       The instance is created by cloning all properties of clone
	//Pre  - clone holds a valid reference to SegmentTermEnum
	//Post - An instance of SegmentTermEnum with the same properties as clone
		
		input		 = clone.input->clone();
		//Copy the postion from the clone
		position     = clone.position;

        if ( clone._term != NULL ){
			_term         = _CLNEW Term;
			_term->set(clone._term,clone._term->text());
		}else
			_term = NULL;
		isIndex      = clone.isIndex;
		termInfo     = _CLNEW TermInfo(clone.termInfo);
		indexPointer = clone.indexPointer;
		buffer       = clone.buffer==NULL?NULL:(TCHAR*)malloc(sizeof(TCHAR) * (clone.bufferLength+1));
		bufferLength = clone.bufferLength;
		prev         = clone.prev==NULL?NULL:_CLNEW Term(clone.prev->field(),clone.prev->text(),false);
		size         = clone.size;

      format       = clone.format;
      indexInterval= clone.indexInterval;
      skipInterval = clone.skipInterval;
      formatM1SkipInterval = clone.formatM1SkipInterval;
      maxSkipLevels = clone.maxSkipLevels;
      
		//Set isClone to true as this instance is a clone of another instance
		isClone      = true;

		//Copy the contents of buffer of clone to the buffer of this instance
		if ( clone.buffer != NULL )
			memcpy(buffer,clone.buffer,bufferLength * sizeof(TCHAR));
	}

	SegmentTermEnum::~SegmentTermEnum(){
	//Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed. If this instance was a clone
	//       then the inputstream is closed and deleted too.

        //todo: revisit this... close() should clean up most of everything.

		//Finalize prev
		_CLDECDELETE(prev );
		//Finalize term
		_CLDECDELETE( _term );
		

		//Delete the buffer if necessary
		if ( buffer != NULL ) free(buffer);
		//Delete termInfo if necessary
		_CLDELETE(termInfo);

		//Check if this instance is a clone
		if ( isClone ){
			//Close the inputstream
			input->close();
			//delete the inputstream
			_CLDELETE(input);
			}
	}

	const char* SegmentTermEnum::getObjectName() const{ return getClassName(); }
	const char* SegmentTermEnum::getClassName(){ return "SegmentTermEnum"; }

	bool SegmentTermEnum::next(){
	//Func - Moves the current of the set to the next in the set
	//Pre  - true
	//Post - If the end has been reached NULL is returned otherwise the term has
	//       become the next Term in the enumeration

		//Increase position by and and check if the end has been reached
		if (position++ >= size-1) {
			//delete term
			_CLDECDELETE(_term);
			return false;
		}

		//delete the previous enumerated term
		Term* tmp=NULL;
		if ( prev != NULL ){
			if ( _LUCENE_ATOMIC_INT_GET(prev->__cl_refcount) > 1 ){
				_CLDECDELETE(prev); //todo: tune other places try and delete its term 
			}else
				tmp = prev; //we are going to re-use this term
		}
		//prev becomes the current enumerated term
		prev = _term;
		//term becomes the next term read from inputStream input
		_term = readTerm(tmp);

		//Read docFreq, the number of documents which contain the term.
		termInfo->docFreq = input->readVInt();
		//Read freqPointer, a pointer into the TermFreqs file (.frq)
		termInfo->freqPointer += input->readVLong();
		
		//Read proxPointer, a pointer into the TermPosition file (.prx).
		termInfo->proxPointer += input->readVLong();

      if(format == -1){
         //  just read skipOffset in order to increment  file pointer; 
         // value is never used since skipTo is switched off
         if (!isIndex) {
            if (termInfo->docFreq > formatM1SkipInterval) {
               termInfo->skipOffset = input->readVInt(); 
            }
         }
      }else{
         if (termInfo->docFreq >= skipInterval) 
            termInfo->skipOffset = input->readVInt();
      }

		//Check if the enumeration is an index
		if (isIndex)
			//read index pointer
			indexPointer += input->readVLong();

		return true;
	}

	Term* SegmentTermEnum::term(bool pointer) {
		if ( pointer )
			return _CL_POINTER(_term);
		else
			return _term;
	}

	void SegmentTermEnum::scanTo(const Term *term){
	//Func - Scan for Term without allocating new Terms
	//Pre  - term != NULL
	//Post - The iterator term has been moved to the position where Term is expected to be
	//       in the enumeration
		while ( term->compareTo(this->_term) > 0 && next()) 
		{
		}
	}

	void SegmentTermEnum::close() {
	//Func - Closes the enumeration to further activity, freeing resources.
	//Pre  - true
	//Post - The inputStream input has been closed

			input->close();
	}

	int32_t SegmentTermEnum::docFreq() const {
	//Func - Returns the document frequency of the current term in the set
	//Pre  - termInfo != NULL
	//       next() must have been called once
	//Post  - The document frequency of the current enumerated term has been returned

		return termInfo->docFreq;
	}

	void SegmentTermEnum::seek(const int64_t pointer, const int32_t p, Term* t, TermInfo* ti) {
	//Func - Repositions term and termInfo within the enumeration
	//Pre  - pointer >= 0
	//       p >= 0 and contains the new position within the enumeration
	//       t is a valid reference to a Term and is the new current term in the enumeration
	//       ti is a valid reference to a TermInfo and is corresponding TermInfo form the new
	//       current Term
	//Post - term and terminfo have been repositioned within the enumeration

		//Reset the IndexInput input to pointer
		input->seek(pointer);
		//Assign the new position
		position = p;

		//finalize the current term
		if ( _term == NULL || _LUCENE_ATOMIC_INT_GET(_term->__cl_refcount) > 1 ){
			_CLDECDELETE(_term);
			//Get a pointer from t and increase the reference counter of t
			_term = _CLNEW Term; //cannot use reference, because TermInfosReader uses non ref-counted array
		}
		_term->set(t,t->text());

		//finalize prev
		_CLDECDELETE(prev);

		//Change the current termInfo so it matches the new current term
		termInfo->set(ti);

		//Have the buffer grown if needed
		if ( bufferLength <= _term->textLength() )
			growBuffer(_term->textLength(), true );		  // copy term text into buffer
		else
			_tcsncpy(buffer,_term->text(),bufferLength); //just copy the buffer
	}

	TermInfo* SegmentTermEnum::getTermInfo()const {
	//Func - Returns a clone of the current termInfo
	//Pre  - termInfo != NULL
	//       next() must have been called once
	//Post - A clone of the current termInfo has been returned

		return _CLNEW TermInfo(*termInfo); //clone
	}

	void SegmentTermEnum::getTermInfo(TermInfo* ti)const {
	//Func - Retrieves a clone of termInfo through the reference ti
	//Pre  - ti contains a valid reference to TermInfo
	//       termInfo != NULL
	//       next() must have been called once
	//Post - ti contains a clone of termInfo

		ti->set(termInfo);
	}

	int64_t SegmentTermEnum::freqPointer()const {
	//Func - Returns the freqpointer of the current termInfo
	//Pre  - termInfo != NULL
	//       next() must have been called once
	//Post - The freqpointer of the current termInfo has been returned

		return termInfo->freqPointer;
	}

	int64_t SegmentTermEnum::proxPointer()const {
	//Func - Returns the proxPointer of the current termInfo
	//Pre  - termInfo != NULL
	//       next() must have been called once
	//Post - the proxPointer of the current termInfo has been returned

		return termInfo->proxPointer;
	}

	SegmentTermEnum* SegmentTermEnum::clone() const {
	//Func - Returns a clone of this instance
	//Pre  - true
	//Post - An clone of this instance has been returned

		return _CLNEW SegmentTermEnum(*this);
	}

	Term* SegmentTermEnum::readTerm(Term* reuse) {
	//Func - Reads the next term in the enumeration
	//Pre  - true
	//Post - The next Term in the enumeration has been read and returned

		//Read the start position from the inputStream input
		int32_t start = input->readVInt();
		//Read the length of term in the inputStream input
		int32_t length = input->readVInt();

		//Calculated the total lenght of bytes that buffer must be to contain the current
		//chars in buffer and the new ones yet to be read
		uint32_t totalLength = start + length;

		if (static_cast<uint32_t>(bufferLength) < totalLength+1)
			growBuffer(totalLength, false); //dont copy the buffer over.

		//Read a length number of characters into the buffer from position start in the inputStream input
		input->readChars(buffer, start, length);
		//Null terminate the string
		buffer[totalLength] = 0;

		//Return a new Term	
		int32_t field = input->readVInt();
		const TCHAR* fieldname = fieldInfos->fieldName(field);
		if ( reuse == NULL )
			reuse = _CLNEW Term;

		reuse->set(fieldname, buffer, false);
		return reuse;
	}

	void SegmentTermEnum::growBuffer(const uint32_t length, bool force_copy) {
	//Func - Instantiate a buffer of length length+1
	//Pre  - length > 0
	//Post - pre(buffer) has been deleted with its contents. A new buffer
	//       has been allocated of length length+1 and the text of term has been copied
	//       to buffer
		//todo: we could guess that we will need to re-grow this
		//buffer a few times...so start off with a reasonable grow
		//value...
		if ( bufferLength > length )
			return;

        //Store the new bufferLength
		if ( length - bufferLength < 8 )
			bufferLength = length+8;
		else
			bufferLength = length+1;

		bool copy = buffer==NULL;

		//Instantiate the new buffer + 1 is needed for terminator '\0'
		if ( buffer == NULL )
			buffer = (TCHAR*)malloc(sizeof(TCHAR) * (bufferLength+1));
		else
			buffer = (TCHAR*)realloc(buffer, sizeof(TCHAR) * (bufferLength+1));

		if ( copy || force_copy){
			//Copy the text of term into buffer
			_tcsncpy(buffer,_term->text(),bufferLength);
		}
	}

CL_NS_END
