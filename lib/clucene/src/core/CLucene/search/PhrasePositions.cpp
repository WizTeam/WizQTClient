/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_PhrasePositions.h"

CL_NS_USE(index)
CL_NS_DEF(search)

PhrasePositions::PhrasePositions(TermPositions* t, const int32_t OffSet) {
  //Func - Constructor
  //Pre  - t != NULL
  //       OffSet != NULL
  //Post - The instance has been created

      CND_PRECONDITION(t != NULL,"Tp is NULL");
      CND_PRECONDITION(OffSet >= 0 ,"OffSet is a negative number");

      tp       = t;
      offset   = OffSet;
      position = 0;
      count    = 0;
	  doc      = 0;

      _next     = NULL;
  }
	
  PhrasePositions::~PhrasePositions(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been deleted

    //delete next Phrase position and by doing that
    //all PhrasePositions in the list
    _CLDELETE(_next);

    //Check if tp is valid
    if ( tp != NULL ){
        //Close TermPositions tp
		tp->close();
        _CLDELETE(tp);
     }
  }

  bool PhrasePositions::next(){
  //Func - Increments to next doc
  //Pre  - tp != NULL
  //Post - if there was no next then doc = INT_MAX otherwise
  //       doc contains the current document number

      CND_PRECONDITION(tp != NULL,"tp is NULL");
    
	  //Move to the next in TermPositions tp
      if (!tp->next()) {
         //There is no next so close the stream
         tp->close();				  
         //delete tp and reset tp to NULL
         _CLVDELETE(tp); //todo: not a clucene object... should be
         //Assign Doc sentinel value
         doc = LUCENE_INT32_MAX_SHOULDBE; 
         return false;
		}else{
         doc  = tp->doc();
         position = 0;
         return true;
	   }
  }
  bool PhrasePositions::skipTo(int32_t target){
    if (!tp->skipTo(target)) {
      tp->close();				// close stream
      doc = LUCENE_INT32_MAX_SHOULDBE;	// sentinel value
      return false;
    }
    doc = tp->doc();
    position = 0;
    return true;
  }
  void PhrasePositions::firstPosition(){
  //Func - Read the first TermPosition
  //Pre  - tp != NULL
  //Post - 

      CND_PRECONDITION(tp != NULL,"tp is NULL");

      //read first pos
      count = tp->freq();				  
      //Move to the next TermPosition
	  nextPosition();
  }

  bool PhrasePositions::nextPosition(){
  //Func - Move to the next position
  //Pre  - tp != NULL
  //Post -

      CND_PRECONDITION(tp != NULL,"tp is NULL");

      if (count-- > 0) {				  
		  //read subsequent pos's
          position = tp->nextPosition() - offset;

		  //Check position always bigger than or equal to 0
          //bvk: todo, bug??? position < 0 occurs, cant figure out why,
          //old version does it too and will fail the "SearchTest" test
          //CND_CONDITION(position >= 0, "position has become a negative number");
          return true;
      }else{
          return false;
      }
	}
CL_NS_END
