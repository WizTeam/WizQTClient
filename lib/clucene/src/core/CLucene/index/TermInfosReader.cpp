/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"

#include "Term.h"
#include "Terms.h"
#include "CLucene/util/Misc.h"
#include "CLucene/store/Directory.h"
#include "CLucene/store/IndexInput.h"

#include "_TermInfo.h"
#include "_FieldInfos.h"
#include "_SegmentTermEnum.h"
#include "_FieldInfos.h"
#include "_TermInfo.h"
#include "_TermInfosWriter.h"
#include "_TermInfosReader.h"

CL_NS_USE(store)
CL_NS_USE(util)
CL_NS_DEF(index)


  TermInfosReader::TermInfosReader(Directory* dir, const char* seg, FieldInfos* fis, const int32_t readBufferSize):
      directory (dir),fieldInfos (fis), indexTerms(NULL), indexInfos(NULL), indexPointers(NULL), indexDivisor(1)
  {
  //Func - Constructor.
  //       Reads the TermInfos file (.tis) and eventually the Term Info Index file (.tii)
  //Pre  - dir is a reference to a valid Directory
  //       Fis contains a valid reference to an FieldInfos instance
  //       seg != NULL and contains the name of the segment
  //Post - An instance has been created and the index named seg has been read. (Remember
  //       a segment is nothing more then an independently readable index)

      CND_PRECONDITION(seg != NULL, "seg is NULL");

	  //Initialize the name of the segment
      segment    =  seg;

      //Create a filname fo a Term Info File
	  string tisFile = Misc::segmentname(segment,".tis");
	  string tiiFile = Misc::segmentname(segment,".tii");
	  bool success = false;
    origEnum = indexEnum = NULL;
    _size = indexTermsLength = totalIndexInterval = 0;

	  try {
		  //Create an SegmentTermEnum for storing all the terms read of the segment
		  origEnum = _CLNEW SegmentTermEnum( directory->openInput( tisFile.c_str(), readBufferSize ), fieldInfos, false);
		  _size =  origEnum->size;
		  totalIndexInterval = origEnum->indexInterval;
		  indexEnum = _CLNEW SegmentTermEnum( directory->openInput( tiiFile.c_str(), readBufferSize ), fieldInfos, true);

		  //Check if enumerator points to a valid instance
		  CND_CONDITION(origEnum != NULL, "No memory could be allocated for orig enumerator");
		  CND_CONDITION(indexEnum != NULL, "No memory could be allocated for index enumerator");

		  success = true;
	  } _CLFINALLY({
		  // With lock-less commits, it's entirely possible (and
		  // fine) to hit a FileNotFound exception above. In
		  // this case, we want to explicitly close any subset
		  // of things that were opened so that we don't have to
		  // wait for a GC to do so.
		  if (!success) {
			  close();
		  }
	  });

  }

  TermInfosReader::~TermInfosReader(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

      //Close the TermInfosReader to be absolutly sure that enumerator has been closed
	  //and the arrays indexTerms, indexPointers and indexInfos and  their elements
	  //have been destroyed
      close();
  }
  int32_t TermInfosReader::getSkipInterval() const {
    return origEnum->skipInterval;
  }

  int32_t TermInfosReader::getMaxSkipLevels() const {
    return origEnum->maxSkipLevels;
  }

  void TermInfosReader::setIndexDivisor(const int32_t _indexDivisor) {
	  if (indexDivisor < 1)
		  _CLTHROWA(CL_ERR_IllegalArgument, "indexDivisor must be > 0");

	  if (indexTerms != NULL)
		  _CLTHROWA(CL_ERR_IllegalArgument, "index terms are already loaded");

	  this->indexDivisor = _indexDivisor;
	  totalIndexInterval = origEnum->indexInterval * _indexDivisor;
  }

  int32_t TermInfosReader::getIndexDivisor() const { return indexDivisor; }
  void TermInfosReader::close() {

	  //Check if indexTerms and indexInfos exist
     if (indexTerms && indexInfos){
          //Iterate through arrays indexTerms and indexPointer to
	      //destroy their elements
#ifdef _DEBUG
         for ( int32_t i=0; i<indexTermsLength;++i ){
            indexTerms[i].__cl_refcount--;
         }
#endif
         //Delete the arrays
         delete [] indexTerms;
         _CLDELETE_ARRAY(indexInfos);
     }

      //Delete the arrays
      _CLDELETE_ARRAY(indexPointers);

      if (origEnum != NULL){
        origEnum->close();

	    //Get a pointer to IndexInput used by the enumeration but
	    //instantiated in the constructor by directory.open( tisFile )
        IndexInput *is = origEnum->input;

        //Delete the enumuration enumerator
        _CLDELETE(origEnum);

        //Delete the IndexInput
        _CLDELETE(is);
      }

      if (indexEnum != NULL){
        indexEnum->close();

	    //Get a pointer to IndexInput used by the enumeration but
	    //instantiated in the constructor by directory.open( tiiFile )
        IndexInput *is = indexEnum->input;

        //Delete the enumuration enumerator
        _CLDELETE(indexEnum);

        //Delete the IndexInput
        _CLDELETE(is);
      }
	  enumerators.setNull();
  }

  int64_t TermInfosReader::size() const{
  //Func - Return the size of the enumeration of TermInfos
  //Pre  - true
  //Post - size has been returened

      return _size;
  }


  Term* TermInfosReader::get(const int32_t position) {
  //Func - Returns the nth term in the set
  //Pre  - position > = 0
  //Post - The n-th term in the set has been returned

	  //Check if the size is 0 because then there are no terms
      if (_size == 0)
          return NULL;

	  SegmentTermEnum* enumerator = getEnum();

	  if (
	      enumerator != NULL //an enumeration exists
	      && enumerator->term(false) != NULL // term is at or past current
	      && position >= enumerator->position
		  && position < (enumerator->position + totalIndexInterval)
	     )
	  {
		  return scanEnum(position);			  // can avoid seek
	  }

    //random-access: must seek
    seekEnum(position / totalIndexInterval);

	//Get the Term at position
    return scanEnum(position);
  }

  SegmentTermEnum* TermInfosReader::getEnum(){
    SegmentTermEnum* termEnum = enumerators.get();
    if (termEnum == NULL){
      termEnum = terms();
      enumerators.set(termEnum);
    }
    return termEnum;
  }

  TermInfo* TermInfosReader::get(const Term* term){
  //Func - Returns a TermInfo for a term
  //Pre  - term holds a valid reference to term
  //Post - if term can be found its TermInfo has been returned otherwise NULL

    //If the size of the enumeration is 0 then no Terms have been read
	if (_size == 0)
		return NULL;

    ensureIndexIsRead();

    // optimize sequential access: first try scanning cached enum w/o seeking
    SegmentTermEnum* enumerator = getEnum();

    // optimize sequential access: first try scanning cached enumerator w/o seeking
    if (
	      //the current term of the enumeration enumerator is not at the end AND
      	enumerator->term(false) != NULL	 &&
      	(
            //there exists a previous current called prev and term is positioned after this prev OR
            ( enumerator->prev != NULL && term->compareTo(enumerator->prev) > 0) ||
            //term is positioned at the same position as the current of enumerator or at a higher position
            term->compareTo(enumerator->term(false)) >= 0 )
      	)
     {

		//Calculate the offset for the position
		int32_t _enumOffset = (int32_t)(enumerator->position/totalIndexInterval)+1;

		// but before end of block
		if (
			//the length of indexTerms (the number of terms in enumerator) equals
			//_enum_offset OR
			indexTermsLength == _enumOffset	 ||
			//term is positioned in front of term found at _enumOffset in indexTerms
			term->compareTo(&indexTerms[_enumOffset]) < 0){

			//no need to seek, retrieve the TermInfo for term
			return scanEnum(term);
        }
    }

    //Reposition current term in the enumeration
    seekEnum(getIndexOffset(term));
	//Return the TermInfo for term
    return scanEnum(term);
  }


  int64_t TermInfosReader::getPosition(const Term* term) {
  //Func - Returns the position of a Term in the set
  //Pre  - term holds a valid reference to a Term
  //       enumerator != NULL
  //Post - If term was found then its position is returned otherwise -1

	  //if the enumeration is empty then return -1
	  if (_size == 0)
		  return -1;

	  ensureIndexIsRead();

      //Retrieve the indexOffset for term
      int32_t indexOffset = getIndexOffset(term);
      seekEnum(indexOffset);

	  SegmentTermEnum* enumerator = getEnum();

      while(term->compareTo(enumerator->term(false)) > 0 && enumerator->next()) {}

	  if ( term->equals(enumerator->term(false)) ){
          return enumerator->position;
	  }else
          return -1;
  }

  SegmentTermEnum* TermInfosReader::terms(const Term* term) {
  //Func - Returns an enumeration of terms starting at or after the named term.
  //       If term is null then enumerator is set to the beginning
  //Pre  - term holds a valid reference to a Term
  //       enumerator != NULL
  //Post - An enumeration of terms starting at or after the named term has been returned

	  SegmentTermEnum* enumerator = NULL;
	  if ( term != NULL ){
		//Seek enumerator to term; delete the new TermInfo that's returned.
		TermInfo* ti = get(term);
		_CLLDELETE(ti);
		enumerator = getEnum();
	  }else
	    enumerator = origEnum;

      //Clone the entire enumeration
      SegmentTermEnum* cln = enumerator->clone();

      //Check if cln points to a valid instance
      CND_CONDITION(cln != NULL,"cln is NULL");

      return cln;
  }


  void TermInfosReader::ensureIndexIsRead() {
  //Func - Reads the term info index file or .tti file.
  //       This file contains every IndexInterval-th entry from the .tis file,
  //       along with its location in the "tis" file. This is designed to be read entirely
  //       into memory and used to provide random access to the "tis" file.
  //Pre  - indexTerms    = NULL
  //       indexInfos    = NULL
  //       indexPointers = NULL
  //Post - The term info index file has been read into memory

    SCOPED_LOCK_MUTEX(THIS_LOCK)

	  if ( indexTerms != NULL )
		  return;

      try {
          indexTermsLength = (size_t)indexEnum->size;

		      //Instantiate an block of Term's,so that each one doesn't have to be new'd
          indexTerms    = new Term[indexTermsLength];
          CND_CONDITION(indexTerms != NULL,"No memory could be allocated for indexTerms");//Check if is indexTerms is a valid array

		  //Instantiate an big block of TermInfo's, so that each one doesn't have to be new'd
          indexInfos    = _CL_NEWARRAY(TermInfo,indexTermsLength);
          CND_CONDITION(indexInfos != NULL,"No memory could be allocated for indexInfos"); //Check if is indexInfos is a valid array

          //Instantiate an array indexPointers that contains pointers to the term info index file
          indexPointers = _CL_NEWARRAY(int64_t,indexTermsLength);
          CND_CONDITION(indexPointers != NULL,"No memory could be allocated for indexPointers");//Check if is indexPointers is a valid array

		  //Iterate through the terms of indexEnum
          for (int32_t i = 0; indexEnum->next(); ++i){
              indexTerms[i].set(indexEnum->term(false),indexEnum->term(false)->text());
              indexEnum->getTermInfo(&indexInfos[i]);
              indexPointers[i] = indexEnum->indexPointer;

			        for (int32_t j = 1; j < indexDivisor; j++)
				        if (!indexEnum->next())
					        break;
          }
    }_CLFINALLY(
          indexEnum->close();
		  //Close and delete the IndexInput is. The close is done by the destructor.
          _CLDELETE( indexEnum->input );
          _CLDELETE( indexEnum );
    );
  }


  int32_t TermInfosReader::getIndexOffset(const Term* term){
  //Func - Returns the offset of the greatest index entry which is less than or equal to term.
  //Pre  - term holds a reference to a valid term
  //       indexTerms != NULL
  //Post - The new offset has been returned

      //Check if is indexTerms is a valid array
      CND_PRECONDITION(indexTerms != NULL,"indexTerms is NULL");

      int32_t lo = 0;
      int32_t hi = indexTermsLength - 1;
	  int32_t mid;
	  int32_t delta;

      while (hi >= lo) {
          //Start in the middle betwee hi and lo
          mid = (lo + hi) >> 1;

          //Check if is indexTerms[mid] is a valid instance of Term
          CND_PRECONDITION(&indexTerms[mid] != NULL,"indexTerms[mid] is NULL");
          CND_PRECONDITION(mid < indexTermsLength,"mid >= indexTermsLength");

		  //Determine if term is before mid or after mid
          delta = term->compareTo(&indexTerms[mid]);
          if (delta < 0){
              //Calculate the new hi
              hi = mid - 1;
          }else if (delta > 0){
                  //Calculate the new lo
                  lo = mid + 1;
			  }else{
                  //term has been found so return its position
                  return mid;
          }
     }
     // the new starting offset
     return hi;
  }

  void TermInfosReader::seekEnum(const int32_t indexOffset) {
  //Func - Reposition the current Term and TermInfo to indexOffset
  //Pre  - indexOffset >= 0
  //       indexTerms    != NULL
  //       indexInfos    != NULL
  //       indexPointers != NULL
  //Post - The current Term and Terminfo have been repositioned to indexOffset

      CND_PRECONDITION(indexOffset >= 0, "indexOffset contains a negative number");
      CND_PRECONDITION(indexTerms != NULL,    "indexTerms is NULL");
      CND_PRECONDITION(indexInfos != NULL,    "indexInfos is NULL");
      CND_PRECONDITION(indexPointers != NULL, "indexPointers is NULL");

	  SegmentTermEnum* enumerator =  getEnum();
	  enumerator->seek(
          indexPointers[indexOffset],
		  (indexOffset * totalIndexInterval) - 1,
          &indexTerms[indexOffset],
		  &indexInfos[indexOffset]
	      );
  }


  TermInfo* TermInfosReader::scanEnum(const Term* term) {
  //Func - Scans the Enumeration of terms for term and returns the corresponding TermInfo instance if found.
  //       The search is started from the current term.
  //Pre  - term contains a valid reference to a Term
  //       enumerator != NULL
  //Post - if term has been found the corresponding TermInfo has been returned otherwise NULL
  //       has been returned

      SegmentTermEnum* enumerator = getEnum();
	  enumerator->scanTo(term);

      //Check if the at the position the Term term can be found
	  if (enumerator->term(false) != NULL && term->equals(enumerator->term(false)) ){
		  //Return the TermInfo instance about term
          return enumerator->getTermInfo();
     }else{
          //term was not found so no TermInfo can be returned
          return NULL;
     }
  }

  Term* TermInfosReader::scanEnum(const int32_t position) {
  //Func - Scans the enumeration to the requested position and returns the
  //       Term located at that position
  //Pre  - position > = 0
  //       enumerator != NULL
  //Post - The Term at the requested position has been returned

      SegmentTermEnum* enumerator = getEnum();

	  //As long the position of the enumeration enumerator is smaller than the requested one
      while(enumerator->position < position){
		  //Move the current of enumerator to the next
		  if (!enumerator->next()){
			  //If there is no next it means that the requested position was to big
              return NULL;
          }
	  }

	  //Return the Term a the requested position
	  return enumerator->term();
  }

CL_NS_END
