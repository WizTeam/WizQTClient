/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CLucene/store/Directory.h"
#include "CLucene/store/IndexOutput.h"
#include "CLucene/util/Misc.h"
#include "Term.h"
#include "_TermInfo.h"
#include "IndexWriter.h"
#include "_FieldInfos.h"
#include "_TermInfosWriter.h"
#include <assert.h>

CL_NS_USE(util)
CL_NS_USE(store)
CL_NS_DEF(index)

	TermInfosWriter::TermInfosWriter(Directory* directory, const char* segment, FieldInfos* fis, int32_t interval):
        fieldInfos(fis){
    //Func - Constructor
    //Pre  - directory contains a valid reference to a Directory
    //       segment != NULL
    //       fis contains a valid reference to a reference FieldInfos
    //Post - The instance has been created

    CND_PRECONDITION(segment != NULL, "segment is NULL");
    //Initialize instance
    initialise(directory,segment,interval, false);

		other = _CLNEW TermInfosWriter(directory, segment,fieldInfos, interval, true);

		CND_CONDITION(other != NULL, "other is NULL");

		other->other = this;
	}

  TermInfosWriter::TermInfosWriter(Directory* directory, const char* segment, FieldInfos* fis, int32_t interval, bool isIndex):
	    fieldInfos(fis){
    //Func - Constructor
    //Pre  - directory contains a valid reference to a Directory
    //       segment != NULL
    //       fis contains a valid reference to a reference FieldInfos
    //       isIndex is true or false
    //Post - The instance has been created

      CND_PRECONDITION(segment != NULL, "segment is NULL");
      initialise(directory,segment,interval,isIndex);
  }

  void TermInfosWriter::initialise(Directory* directory, const char* segment, int32_t interval, bool IsIndex){
    //Func - Helps constructors to initialize Instance
    //Pre  - directory contains a valid reference to a Directory
    //       segment != NULL
    //       fis contains a valid reference to a reference FieldInfos
    //Post - The instance has been initialized


    maxSkipLevels = 10;
    lastTermTextLength = 0;
    lastFieldNumber = -1;

    lastTi  = _CLNEW TermInfo();

    CND_CONDITION(lastTi != NULL, "Could not allocate memory for lastTi");

    lastIndexPointer = 0;
    size             = 0;
    isIndex          = IsIndex;
    indexInterval = interval;
    skipInterval = TermInfosWriter::DEFAULT_TERMDOCS_SKIP_INTERVAL;

    output = directory->createOutput( Misc::segmentname(segment, (isIndex ? ".tii" : ".tis")).c_str() );

    output->writeInt(FORMAT);                      // write format
    output->writeLong(0);                          // leave space for size
    output->writeInt(indexInterval);// write indexInterval
    output->writeInt(skipInterval); // write skipInterval

    output->writeInt(maxSkipLevels);              // write maxSkipLevels

    //Set other to NULL by Default
    other = NULL;
  }

	TermInfosWriter::~TermInfosWriter(){
    //Func - Destructor
    //Pre  - true
    //Post - de instance has been destroyed

		close();
	}

  void TermInfosWriter::add(Term* term, TermInfo* ti){
    const size_t length = term->textLength();
    if ( termTextBuffer.values == NULL || termTextBuffer.length < length ){
      termTextBuffer.resize( (int32_t)(length*1.25) );
    }
    _tcsncpy(termTextBuffer.values, term->text(), length);

    add(fieldInfos->fieldNumber(term->field()), termTextBuffer.values, length, ti);
  }

  // Currently used only by assert statement
  int32_t TermInfosWriter::compareToLastTerm(int32_t fieldNumber, const TCHAR* termText, int32_t length) {
    int32_t pos = 0;

    if (lastFieldNumber != fieldNumber) {
      const int32_t cmp = _tcscmp(fieldInfos->fieldName(lastFieldNumber), fieldInfos->fieldName(fieldNumber));
      // If there is a field named "" (empty string) then we
      // will get 0 on this comparison, yet, it's "OK".  But
      // it's not OK if two different field numbers map to
      // the same name.
      if (cmp != 0 || lastFieldNumber != -1)
        return cmp;
    }

    //TODO: is this just a _tcsncmp???
    while(pos < length && pos < lastTermTextLength) {
      const TCHAR c1 = lastTermText[pos];
      const TCHAR c2 = termText[pos];
      if (c1 < c2)
        return -1;
      else if (c1 > c2)
        return 1;
      pos++;
    }

    if (pos < lastTermTextLength)
      // Last term was longer
      return 1;
    else if (pos < length)
      // Last term was shorter
      return -1;
    else
      return 0;
  }

	void TermInfosWriter::add(int32_t fieldNumber, const TCHAR* termText, int32_t termTextLength, const TermInfo* ti) {
	//Func - Writes a Term and TermInfo to the outputstream
	//Pre  - Term must be lexicographically greater than all previous Terms added.
    //       Pointers of TermInfo ti (freqPointer and proxPointer) must be positive and greater than all previous.

// TODO: This is a hack. If _ASCII is defined, Misc::toString(const TCHAR*, int) will cause linking errors,
//       at least on VS. Needs a prettier fix no doubt... ISH 2009-11-08
#ifdef _ASCII
        assert(compareToLastTerm(fieldNumber, termText, termTextLength) < 0 ||
            (isIndex && termTextLength == 0 && lastTermTextLength == 0));
#else
    CND_PRECONDITION(compareToLastTerm(fieldNumber, termText, termTextLength) < 0 ||
      (isIndex && termTextLength == 0 && lastTermTextLength == 0),
      (string("Terms are out of order: field=") + Misc::toString(fieldInfos->fieldName(fieldNumber)) +
      " (number " + Misc::toString(fieldNumber) + ")" +
      " lastField=" + Misc::toString(fieldInfos->fieldName(lastFieldNumber)) +
      " (number " + Misc::toString(lastFieldNumber) + ")" +
      " text=" + Misc::toString(termText, termTextLength) +
      " lastText=" + Misc::toString(lastTermText.values, lastTermTextLength)
      ).c_str() );
#endif

    CND_PRECONDITION(ti->freqPointer >= lastTi->freqPointer, ("freqPointer out of order (" + Misc::toString(ti->freqPointer) + " < " + Misc::toString(lastTi->freqPointer) + ")").c_str());
    CND_PRECONDITION(ti->proxPointer >= lastTi->proxPointer, ("proxPointer out of order (" + Misc::toString(ti->proxPointer) + " < " + Misc::toString(lastTi->proxPointer) + ")").c_str());

		if (!isIndex && size % indexInterval == 0){
      //add an index term
      other->add(lastFieldNumber, lastTermText.values, lastTermTextLength, lastTi);                      // add an index term
		}

		//write term
		writeTerm(fieldNumber, termText, termTextLength);
		// write doc freq
		output->writeVInt(ti->docFreq);
		//write pointers
		output->writeVLong(ti->freqPointer - lastTi->freqPointer);
		output->writeVLong(ti->proxPointer - lastTi->proxPointer);
		if (ti->docFreq >= skipInterval) {
			output->writeVInt(ti->skipOffset);
		}

		if (isIndex){
			output->writeVLong(other->output->getFilePointer() - lastIndexPointer);
			lastIndexPointer = other->output->getFilePointer(); // write pointer
		}
    if (lastTermText.length < termTextLength || lastTermText.length == 0){
      lastTermText.resize( (int32_t)cl_max(10.0,termTextLength*1.25) );
    }
    if ( termText != NULL )
      _tcsncpy(lastTermText.values,termText,termTextLength);
    else
      lastTermText.values[0] = 0;

    lastTermTextLength = termTextLength;
    lastFieldNumber = fieldNumber;

		lastTi->set(ti);
		size++;
	}

	void TermInfosWriter::close() {
    //Func - Closes the TermInfosWriter
	//Pre  - true
	//Post - The TermInfosWriter has been closed

		if (output){
			//write size at start
		    output->seek(4);          // write size after format
		    output->writeLong(size);
		    output->close();
		   _CLDELETE(output);

		   if (!isIndex){
			   if(other){
			      other->close();
			      _CLDELETE( other );
          }
        }
        _CLDELETE(lastTi);
		   }
	}

  void TermInfosWriter::writeTerm(int32_t fieldNumber, const TCHAR* termText, int32_t termTextLength){

    // Compute prefix in common with last term:
    int32_t start = 0;
    const int32_t limit = termTextLength < lastTermTextLength ? termTextLength : lastTermTextLength;
    while(start < limit) {
      if (termText[start] != lastTermText.values[start])
        break;
      start++;
    }

    int32_t length = termTextLength - start;

    output->writeVInt(start);                     // write shared prefix length
    output->writeVInt(length);                  // write delta length
    output->writeChars(termText+start, length);  // write delta chars
    output->writeVInt(fieldNumber); // write field num
  }



CL_NS_END
