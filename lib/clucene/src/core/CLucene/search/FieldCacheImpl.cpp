/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_FieldCacheImpl.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/index/Term.h"
#include "CLucene/index/Terms.h"
#include "CLucene/util/_StringIntern.h"
#include "CLucene/util/Misc.h"
#include "Sort.h"

CL_NS_USE(util)
CL_NS_USE(index)
CL_NS_DEF(search)

///the type that is stored in the field cache. can't use a typedef because
///the decorated name would become too long
class fieldcacheCacheReaderType: public CL_NS(util)::CLHashMap<FieldCacheImpl::FileEntry*,
	FieldCacheAuto*,
	FieldCacheImpl::FileEntry::Compare,
	FieldCacheImpl::FileEntry::Equals,
	CL_NS(util)::Deletor::Object<FieldCacheImpl::FileEntry>,
	CL_NS(util)::Deletor::Object<FieldCacheAuto> >{
public:
    fieldcacheCacheReaderType(){
		setDeleteKey(false);
		setDeleteValue(false);
	}
	~fieldcacheCacheReaderType(){
		iterator itr = begin();
		while ( itr != end() ){
			FieldCacheImpl::FileEntry* f = itr->first;
			if ( f->getType() != SortField::AUTO )
				_CLDELETE( itr->second );
			_CLDELETE( f );
			++itr;
		}
		clear();
	}
};

//note: typename gets too long if using cacheReaderType as a typename
class fieldcacheCacheType: public CL_NS(util)::CLHashMap<
	CL_NS(index)::IndexReader*,
	fieldcacheCacheReaderType*,
	CL_NS(util)::Compare::Void<CL_NS(index)::IndexReader>,
	CL_NS(util)::Equals::Void<CL_NS(index)::IndexReader>,
	CL_NS(util)::Deletor::Object<CL_NS(index)::IndexReader>,
	CL_NS(util)::Deletor::Object<fieldcacheCacheReaderType> >{
public:
	fieldcacheCacheType ( const bool deleteKey, const bool deleteValue)
	{
	    setDeleteKey(deleteKey);
	    setDeleteValue(deleteValue);
	}
	virtual ~fieldcacheCacheType(){

	}
};

FieldCache::StringIndex::StringIndex (int32_t* values, TCHAR** lookup, int count) {
    this->count = count;
	this->order = values;
	this->lookup = lookup;
}

FieldCache::StringIndex::~StringIndex(){
    _CLDELETE_ARRAY(order);

    for ( int i=0;i<count;i++ )
        _CLDELETE_CARRAY(lookup[i]);
    _CLDELETE_ARRAY(lookup);
}

FieldCacheImpl::FieldCacheImpl()
{
    cache = _CLNEW fieldcacheCacheType(false,true);
}
FieldCacheImpl::~FieldCacheImpl(){
    cache->clear();
    _CLDELETE(cache);
}

FieldCacheImpl::FileEntry::FileEntry (const TCHAR* field, int32_t type) {
   this->field = CLStringIntern::intern(field);
   this->type = type;
   this->custom = NULL;
   this->_hashCode = 0;
 }

 /** Creates one of these objects for a custom comparator. */
 FieldCacheImpl::FileEntry::FileEntry (const TCHAR* field, SortComparatorSource* custom) {
   this->field = CLStringIntern::intern(field);
   this->type = SortField::CUSTOM;
   this->custom = custom;
   this->_hashCode = 0;
 }
 FieldCacheImpl::FileEntry::~FileEntry(){
   CLStringIntern::unintern(field);
 }

 size_t FieldCacheImpl::FileEntry::hashCode(){
 	if ( _hashCode == 0 ){
    //todo: cache hashcode?
     size_t ret = Misc::thashCode(field);
     if ( custom != NULL )
         ret = ret ^ custom->hashCode();
     ret = ret ^ (type*7); //type with a seed
	     _hashCode = ret;
    }
     return _hashCode;
 }
 int32_t FieldCacheImpl::FileEntry::compareTo(const FieldCacheImpl::FileEntry* other) const{
     if ( other->field == this->field ){
         if ( other->type == this->type ){
            if ( other->custom == NULL ){
                if ( this->custom == NULL )
                    return 0; //both null
                else
                    return 1;
            }else if ( this->custom == NULL )
                return -1;
            else if ( other->custom < this->custom )
                return -1;
            else if ( other->custom > this->custom )
                return 1;
            else
                return 0;
         }else if ( other->type > this->type )
             return 1;
         else
             return -1;

     }else
         return _tcscmp(other->field,this->field);
 }

 /** Two of these are equal iff they reference the same field and type. */
 /*bool FieldCacheImpl::FileEntry::equals (FileEntry* other) {
      if (other->field == field && other->type == type) {
       if (other->custom == NULL) {
         if (custom == NULL)
          return true;
       } else if (other->custom->equals (custom)) {
         return true;
       }
     }
 }*/

 /** Composes a hashcode based on the field and type. */
 /*size_t FieldCacheImpl::FileEntry::hashCode() {
   return field->hashCode() ^ type ^ (custom==NULL ? 0 : custom->hashCode());
 }*/





  /** See if an object is in the cache. */
  FieldCacheAuto* FieldCacheImpl::lookup (IndexReader* reader, const TCHAR* field, int32_t type) {
    FieldCacheAuto* ret = NULL;
    FileEntry* entry = _CLNEW FileEntry (field, type);
    {
    	SCOPED_LOCK_MUTEX(THIS_LOCK)
      	fieldcacheCacheReaderType* readerCache = cache->get(reader);
      	if (readerCache != NULL)
          ret = readerCache->get (entry);
      	_CLDELETE(entry);
	}
    return ret;
  }


  /** See if a custom object is in the cache. */
  FieldCacheAuto* FieldCacheImpl::lookup (IndexReader* reader, const TCHAR* field, SortComparatorSource* comparer) {
    FieldCacheAuto* ret = NULL;
    FileEntry* entry = _CLNEW FileEntry (field, comparer);
    {
    	SCOPED_LOCK_MUTEX(THIS_LOCK)
      	fieldcacheCacheReaderType* readerCache = cache->get(reader);
      	if (readerCache != NULL)
        	ret = readerCache->get (entry);
      	_CLDELETE(entry);
}
    return ret;
  }

	void FieldCacheImpl::closeCallback(CL_NS(index)::IndexReader* reader, void* fieldCacheImpl){
		FieldCacheImpl* fci = (FieldCacheImpl*)fieldCacheImpl;
    	SCOPED_LOCK_MUTEX(fci->THIS_LOCK)
		fci->cache->remove(reader);
	}

  /** Put an object into the cache. */
  void FieldCacheImpl::store (IndexReader* reader, const TCHAR* field, int32_t type, FieldCacheAuto* value) {
    FileEntry* entry = _CLNEW FileEntry (field, type);
    {
    	SCOPED_LOCK_MUTEX(THIS_LOCK)
	  fieldcacheCacheReaderType* readerCache = cache->get(reader);
	  if (readerCache == NULL) {
	    readerCache = _CLNEW fieldcacheCacheReaderType;
	    cache->put(reader,readerCache);
	    reader->addCloseCallback(closeCallback, this);
	  }
	  readerCache->put (entry, value);
	 //this is supposed to return the previous value, but it needs to be deleted!!!
    }
  }

  /** Put a custom object into the cache. */
  void FieldCacheImpl::store (IndexReader* reader, const TCHAR* field, SortComparatorSource* comparer, FieldCacheAuto* value) {
    FileEntry* entry = _CLNEW FileEntry (field, comparer);
    {
      SCOPED_LOCK_MUTEX(THIS_LOCK)
      fieldcacheCacheReaderType* readerCache = cache->get(reader);
      if (readerCache == NULL) {
        readerCache = _CLNEW fieldcacheCacheReaderType;
        cache->put(reader, readerCache);
		reader->addCloseCallback(FieldCacheImpl::closeCallback, this);
      }
      readerCache->put(entry, value);
	  //this is supposed to return the previous value, but it needs to be deleted!!!
	}
  }





 // inherit javadocs
 FieldCacheAuto* FieldCacheImpl::getInts (IndexReader* reader, const TCHAR* field) {
    field = CLStringIntern::intern(field);
    FieldCacheAuto* ret = lookup (reader, field, SortField::INT);
    if (ret == NULL) {
      int32_t retLen = reader->maxDoc();
      int32_t* retArray = _CL_NEWARRAY(int32_t,retLen);
	    memset(retArray,0,sizeof(int32_t)*retLen);
      if (retLen > 0) {
        TermDocs* termDocs = reader->termDocs();

	    Term* term = _CLNEW Term (field, LUCENE_BLANK_STRING, false);
      TermEnum* termEnum = reader->terms (term);
	    _CLDECDELETE(term);
      try {
          if (termEnum->term(false) == NULL) {
			      _CLTHROWA(CL_ERR_Runtime,"no terms in field"); //todo: add detailed error:  + field);
          }
          do {
            Term* term = termEnum->term(false);
            if (term->field() != field)
				      break;

            int32_t termval = _ttoi(term->text());
            termDocs->seek (termEnum);
            while (termDocs->next()) {
              retArray[termDocs->doc()] = termval;
            }
          } while (termEnum->next());
        } _CLFINALLY(
          termDocs->close();
          _CLDELETE(termDocs);
          termEnum->close();
          _CLDELETE(termEnum);
        )
      }

      FieldCacheAuto* fa = _CLNEW FieldCacheAuto(retLen,FieldCacheAuto::INT_ARRAY);
      fa->intArray = retArray;

      store (reader, field, SortField::INT, fa);
	    CLStringIntern::unintern(field);
      return fa;
    }
	  CLStringIntern::unintern(field);
    return ret;
  }

  // inherit javadocs
  FieldCacheAuto* FieldCacheImpl::getFloats (IndexReader* reader, const TCHAR* field){
	field = CLStringIntern::intern(field);
    FieldCacheAuto* ret = lookup (reader, field, SortField::FLOAT);
    if (ret == NULL) {
	  int32_t retLen = reader->maxDoc();
      float_t* retArray = _CL_NEWARRAY(float_t,retLen);
	  memset(retArray,0,sizeof(float_t)*retLen);
      if (retLen > 0) {
        TermDocs* termDocs = reader->termDocs();

		Term* term = _CLNEW Term (field, LUCENE_BLANK_STRING, false);
        TermEnum* termEnum = reader->terms (term);
		_CLDECDELETE(term);

        try {
          if (termEnum->term(false) == NULL) {
            _CLTHROWA(CL_ERR_Runtime,"no terms in field "); //todo: make richer error + field);
          }
          do {
            Term* term = termEnum->term(false);
            if (term->field() != field)
				break;

            float_t termval = _tcstod(term->text(),NULL);
            termDocs->seek (termEnum);
            while (termDocs->next()) {
              retArray[termDocs->doc()] = termval;
            }
          } while (termEnum->next());
        } _CLFINALLY(
          termDocs->close();
          _CLDELETE(termDocs);
          termEnum->close();
          _CLDELETE(termEnum);
        )
      }

	  FieldCacheAuto* fa = _CLNEW FieldCacheAuto(retLen,FieldCacheAuto::FLOAT_ARRAY);
	  fa->floatArray = retArray;

      store (reader, field, SortField::FLOAT, fa);
	  CLStringIntern::unintern(field);
      return fa;
    }
	CLStringIntern::unintern(field);
    return ret;
  }


  // inherit javadocs
  FieldCacheAuto* FieldCacheImpl::getStrings (IndexReader* reader, const TCHAR* field){
   //todo: this is not really used, i think?
	field = CLStringIntern::intern(field);
    FieldCacheAuto* ret = lookup (reader, field, SortField::STRING);
    if (ret == NULL) {
	  int32_t retLen = reader->maxDoc();
      TCHAR** retArray = _CL_NEWARRAY(TCHAR*,retLen+1);
      memset(retArray,0,sizeof(TCHAR*)*(retLen+1));
      if (retLen > 0) {
        TermDocs* termDocs = reader->termDocs();

		    Term* term = _CLNEW Term (field, LUCENE_BLANK_STRING, false);
        TermEnum* termEnum = reader->terms (term);
		    _CLDECDELETE(term);

        try {
          if (termEnum->term(false) == NULL) {
            _CLTHROWA(CL_ERR_Runtime,"no terms in field "); //todo: extend to + field);
          }
          do {
            Term* term = termEnum->term(false);
            if (term->field() != field)
				break;
            const TCHAR* termval = term->text();
            termDocs->seek (termEnum);
            while (termDocs->next()) {
              retArray[termDocs->doc()] = STRDUP_TtoT(termval); //todo: any better way of doing this???
            }
          } while (termEnum->next());
        } _CLFINALLY(
		  retArray[retLen]=NULL;
          termDocs->close();
          _CLDELETE(termDocs);
          termEnum->close();
          _CLDELETE(termEnum);
        )
      }
	    FieldCacheAuto* fa = _CLNEW FieldCacheAuto(retLen,FieldCacheAuto::STRING_ARRAY);
	    fa->stringArray = retArray;
	    fa->ownContents=true;
      store (reader, field, SortField::STRING, fa);
	    CLStringIntern::unintern(field);
      return fa;
    }
	  CLStringIntern::unintern(field);
    return ret;
  }

  // inherit javadocs
  FieldCacheAuto* FieldCacheImpl::getStringIndex (IndexReader* reader, const TCHAR* field){
	  field = CLStringIntern::intern(field);
    FieldCacheAuto* ret = lookup (reader, field, STRING_INDEX);
    int32_t t = 0;  // current term number
    if (ret == NULL) {
	    int32_t retLen = reader->maxDoc();
      int32_t* retArray = _CL_NEWARRAY(int32_t,retLen);
	    memset(retArray,0,sizeof(int32_t)*retLen);

      TCHAR** mterms = _CL_NEWARRAY(TCHAR*,retLen+2);
      mterms[0]=NULL;
      if ( retLen > 0 ) {
        TermDocs* termDocs = reader->termDocs();

		    Term* term = _CLNEW Term (field, LUCENE_BLANK_STRING, false);
        TermEnum* termEnum = reader->terms (term);
		    _CLDECDELETE(term);


		    CND_PRECONDITION(t+1 <= retLen, "t out of bounds");

        // an entry for documents that have no terms in this field
        // should a document with no terms be at top or bottom?
        // this puts them at the top - if it is changed, FieldDocSortedHitQueue
        // needs to change as well.
        mterms[t++] = NULL;

        try {
          if (termEnum->term(false) == NULL) {
            _CLTHROWA(CL_ERR_Runtime,"no terms in field"); //todo: make rich message " + field);
          }
          do {
            Term* term = termEnum->term(false);
            if (term->field() != field)
			        break;

            // store term text
            // we expect that there is at most one term per document
            if (t >= retLen+1)
			        _CLTHROWA(CL_ERR_Runtime,"there are more terms than documents in field"); //todo: rich error \"" + field + "\"");
            mterms[t] = STRDUP_TtoT(term->text());

            termDocs->seek (termEnum);
            while (termDocs->next()) {
              retArray[termDocs->doc()] = t;
            }

            t++;
          } while (termEnum->next());
		      CND_PRECONDITION(t<retLen+2,"t out of bounds");
		      mterms[t] = NULL;
        } _CLFINALLY(
          termDocs->close();
          _CLDELETE(termDocs);
          termEnum->close();
          _CLDELETE(termEnum);
        );

        if (t == 0) {
          // if there are no terms, make the term array
          // have a single NULL entry
          _CLDELETE_ARRAY(mterms);
          mterms = _CL_NEWARRAY(TCHAR*,1); //todo: delete old mterms?
          mterms[0]=NULL;
          } else if (t < retLen) { //todo: check, was mterms.length
          // if there are less terms than documents,
          // trim off the dead array space
          //const TCHAR** terms = _CL_NEWARRAY(TCHAR,t);
          //System.arraycopy (mterms, 0, terms, 0, t);
          //mterms = terms;

          //we simply shorten the length of the array...
        
        }
      }
      FieldCache::StringIndex* value = _CLNEW FieldCache::StringIndex (retArray, mterms,t);
  	  
	    FieldCacheAuto* fa = _CLNEW FieldCacheAuto(retLen,FieldCacheAuto::STRING_INDEX);
	    fa->stringIndex = value;
	    fa->ownContents=true;
      store (reader, field, STRING_INDEX, fa);
      CLStringIntern::unintern(field);
      return fa;
    }
    CLStringIntern::unintern(field);
    return ret;
  }

  // inherit javadocs
  FieldCacheAuto* FieldCacheImpl::getAuto (IndexReader* reader, const TCHAR* field) {
	  field = CLStringIntern::intern(field);
    FieldCacheAuto* ret = lookup (reader, field, SortField::AUTO);
    if (ret == NULL) {
	    Term* term = _CLNEW Term (field, LUCENE_BLANK_STRING, false);
      TermEnum* enumerator = reader->terms (term);
	    _CLDECDELETE(term);

      try {
        Term* term = enumerator->term(false);
        if (term == NULL) {
          _CLTHROWA(CL_ERR_Runtime,"no terms in field - cannot determine sort type"); //todo: make rich error: " + field + "
        }
        if (term->field() == field) {
          const TCHAR* termtext = term->text();
		      size_t termTextLen = term->textLength();

		      bool isint=true;
		      for ( size_t i=0;i<termTextLen;i++ ){
			      if ( _tcschr(_T("0123456789 +-"),termtext[i]) == NULL ){
				    isint = false;
				    break;
			      }
		      }
		      if ( isint )
			      ret = getInts (reader, field);
		      else{
			      bool isfloat=true;

			      int32_t searchLen = termTextLen;
			      if ( termtext[termTextLen-1] == 'f' )
				      searchLen--;
			      for ( int32_t i=0;i<searchLen;i++ ){
				      if ( _tcschr(_T("0123456789 Ee.+-"),termtext[i]) == NULL ){
					    isfloat = false;
					    break;
				      }
			      }
			      if ( isfloat )
				      ret = getFloats (reader, field);
			      else{
				      ret = getStringIndex (reader, field);
			      }
		      }

          if (ret != NULL) {
			      store (reader, field, SortField::AUTO, ret);
          }
        } else {
          _CLTHROWA (CL_ERR_Runtime,"field does not appear to be indexed"); //todo: make rich error: \"" + field + "\"
        }
      } _CLFINALLY( enumerator->close(); _CLDELETE(enumerator) );

    }
	  CLStringIntern::unintern(field);
    return ret;
  }


  // inherit javadocs
  FieldCacheAuto* FieldCacheImpl::getCustom (IndexReader* reader, const TCHAR* field, SortComparator* comparator){
	  field = CLStringIntern::intern(field);

    FieldCacheAuto* ret = lookup (reader, field, comparator);
    if (ret == NULL) {
	    int32_t retLen = reader->maxDoc();
      Comparable** retArray = _CL_NEWARRAY(Comparable*,retLen);
	    memset(retArray,0,sizeof(Comparable*)*retLen);
      if (retLen > 0) {
        TermDocs* termDocs = reader->termDocs();
        TermEnum* termEnum = reader->terms ();

        try {
          if (termEnum->term(false) == NULL) {
            _CLTHROWA(CL_ERR_Runtime,"no terms in field "); //todo: make rich error + field);
          }
          do {
            Term* term = termEnum->term(false);
            if (term->field() != field)
				    break;
            Comparable* termval = comparator->getComparable (term->text());
            termDocs->seek (termEnum);
            while (termDocs->next()) {
              retArray[termDocs->doc()] = termval;
            }
          } while (termEnum->next());
        } _CLFINALLY (
          termDocs->close();
          _CLDELETE(termDocs);
          termEnum->close();
          _CLDELETE(termEnum);
        );
      }

      FieldCacheAuto* fa = _CLNEW FieldCacheAuto(retLen,FieldCacheAuto::COMPARABLE_ARRAY);
      fa->comparableArray = retArray;
      fa->ownContents=true;
      store (reader, field, SortField::CUSTOM, fa);
      CLStringIntern::unintern(field);
      return fa;
    }
	  CLStringIntern::unintern(field);
    return ret;
  }


CL_NS_END
