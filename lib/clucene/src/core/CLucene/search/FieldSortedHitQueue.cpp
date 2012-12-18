/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "FieldSortedHitQueue.h"
#include "_FieldDocSortedHitQueue.h"
#include "_FieldCacheImpl.h"
#include "Compare.h"
#include "CLucene/index/IndexReader.h"

CL_NS_USE(util)
CL_NS_USE(index)
CL_NS_DEF(search)


//note: typename gets too long if using cacheReaderType as a typename
class hitqueueCacheType: public CL_NS(util)::CLHashMap<CL_NS(index)::IndexReader*, 
		hitqueueCacheReaderType*, 
		CL_NS(util)::Compare::Void<CL_NS(index)::IndexReader>,
		CL_NS(util)::Equals::Void<CL_NS(index)::IndexReader>,
		CL_NS(util)::Deletor::Object<CL_NS(index)::IndexReader>, 
		CL_NS(util)::Deletor::Object<hitqueueCacheReaderType> >{
public:
    hitqueueCacheType(bool deleteKey, bool deleteValue){
	    setDeleteKey(deleteKey);
	    setDeleteValue(deleteValue);
    }
    ~hitqueueCacheType(){
        clear();
    }
};


///the type that is stored in the field cache. can't use a typedef because
///the decorated name would become too long
class hitqueueCacheReaderType: public CL_NS(util)::CLHashMap<FieldCacheImpl::FileEntry*,
    ScoreDocComparator*, 
    FieldCacheImpl::FileEntry::Compare,
    FieldCacheImpl::FileEntry::Equals,
    CL_NS(util)::Deletor::Object<FieldCacheImpl::FileEntry>,
    CL_NS(util)::Deletor::Object<ScoreDocComparator> >{

public:
    hitqueueCacheReaderType(bool deleteValue){
	    setDeleteKey(true);
	    setDeleteValue(deleteValue);
    }
    ~hitqueueCacheReaderType(){
        clear();
    }
};

hitqueueCacheType* FieldSortedHitQueue::Comparators = _CLNEW hitqueueCacheType(false,true);
DEFINE_MUTEX(FieldSortedHitQueue::Comparators_LOCK)

void FieldSortedHitQueue::_shutdown(){
	Comparators->clear();
	_CLDELETE(Comparators);
}

FieldSortedHitQueue::FieldSortedHitQueue (IndexReader* reader, SortField** _fields, int32_t size):
	fieldsLen(0),
	maxscore(1.0f)
{
	while ( _fields[fieldsLen] != 0 )
		fieldsLen++;

	comparators = _CL_NEWARRAY(ScoreDocComparator*,fieldsLen+1);
	SortField** tmp = _CL_NEWARRAY(SortField*,fieldsLen+1);
	for (int32_t i=0; i<fieldsLen; ++i) {
		const TCHAR* fieldname = _fields[i]->getField();
		//todo: fields[i].getLocale(), not implemented
		comparators[i] = getCachedComparator (reader, fieldname, _fields[i]->getType(), _fields[i]->getFactory());
		tmp[i] = _CLNEW SortField (fieldname, comparators[i]->sortType(), _fields[i]->getReverse());
	}
	comparatorsLen = fieldsLen;
	comparators[fieldsLen]=NULL;
	tmp[fieldsLen] = NULL;
	this->fields = tmp;

	initialize(size,true);
}


bool FieldSortedHitQueue::lessThan (FieldDoc* docA, FieldDoc* docB) {
    // keep track of maximum score
    if (docA->scoreDoc.score > maxscore) maxscore = docA->scoreDoc.score;
    if (docB->scoreDoc.score > maxscore) maxscore = docB->scoreDoc.score;

    // run comparators
    int32_t c = 0;
	for ( int32_t i=0; c==0 && i<comparatorsLen; ++i ) {
		c = (fields[i]->getReverse()) ? comparators[i]->compare (&docB->scoreDoc, &docA->scoreDoc) : 
			comparators[i]->compare (&docA->scoreDoc, &docB->scoreDoc);
    }
    // avoid random sort order that could lead to duplicates (bug #31241):
    if (c == 0)
      return docA->scoreDoc.doc > docB->scoreDoc.doc;
    return c > 0;
}


//static
ScoreDocComparator* FieldSortedHitQueue::comparatorString (IndexReader* reader, const TCHAR* field) {
	//const TCHAR* field = CLStringIntern::intern(fieldname);
	FieldCacheAuto* fa = FieldCache::DEFAULT()->getStringIndex (reader, field);
	//CLStringIntern::unintern(field);

	CND_PRECONDITION(fa->contentType==FieldCacheAuto::STRING_INDEX,"Content type is incorrect");
	fa->ownContents = false;
    return _CLNEW ScoreDocComparators::String(fa->stringIndex, fa->contentLen);
}

//static 
ScoreDocComparator* FieldSortedHitQueue::comparatorInt (IndexReader* reader, const TCHAR* field){
    //const TCHAR* field = CLStringIntern::intern(fieldname);
    FieldCacheAuto* fa =  FieldCache::DEFAULT()->getInts (reader, field);
	//CLStringIntern::unintern(field);

	CND_PRECONDITION(fa->contentType==FieldCacheAuto::INT_ARRAY,"Content type is incorrect");
    return _CLNEW ScoreDocComparators::Int32(fa->intArray, fa->contentLen);
  }

//static
 ScoreDocComparator* FieldSortedHitQueue::comparatorFloat (IndexReader* reader, const TCHAR* field) {
	//const TCHAR* field = CLStringIntern::intern(fieldname);
    FieldCacheAuto* fa = FieldCache::DEFAULT()->getFloats (reader, field);
	//CLStringIntern::unintern(field);

	CND_PRECONDITION(fa->contentType==FieldCacheAuto::FLOAT_ARRAY,"Content type is incorrect");
	return _CLNEW ScoreDocComparators::Float (fa->floatArray, fa->contentLen);
  }
//static
  ScoreDocComparator* FieldSortedHitQueue::comparatorAuto (IndexReader* reader, const TCHAR* field){
	//const TCHAR* field = CLStringIntern::intern(fieldname);
    FieldCacheAuto* fa =  FieldCache::DEFAULT()->getAuto (reader, field);
	//CLStringIntern::unintern(field);

    if (fa->contentType == FieldCacheAuto::STRING_INDEX ) {
      return comparatorString (reader, field);
    } else if (fa->contentType == FieldCacheAuto::INT_ARRAY) {
      return comparatorInt (reader, field);
    } else if (fa->contentType == FieldCacheAuto::FLOAT_ARRAY) {
      return comparatorFloat (reader, field);
    } else if (fa->contentType == FieldCacheAuto::STRING_ARRAY) {
      return comparatorString (reader, field);
    } else {
      _CLTHROWA(CL_ERR_Runtime, "unknown data type in field"); //todo: rich error information: '"+field+"'");
    }
  }


  //todo: Locale locale, not implemented yet
  ScoreDocComparator* FieldSortedHitQueue::getCachedComparator (IndexReader* reader, const TCHAR* fieldname, int32_t type, SortComparatorSource* factory){ 
	if (type == SortField::DOC) 
		return ScoreDocComparator::INDEXORDER();
	if (type == SortField::DOCSCORE) 
		return ScoreDocComparator::RELEVANCE();
    ScoreDocComparator* comparator = lookup (reader, fieldname, type, factory);
    if (comparator == NULL) {
      switch (type) {
		case SortField::AUTO:
          comparator = comparatorAuto (reader, fieldname);
          break;
		case SortField::INT:
          comparator = comparatorInt (reader, fieldname);
          break;
		case SortField::FLOAT:
          comparator = comparatorFloat (reader, fieldname);
          break;
		case SortField::STRING:
          //if (locale != NULL) 
		//	  comparator = comparatorStringLocale (reader, fieldname, locale);
          //else 
			  comparator = comparatorString (reader, fieldname);
          break;
		case SortField::CUSTOM:
          comparator = factory->newComparator (reader, fieldname);
          break;
        default:
          _CLTHROWA(CL_ERR_Runtime,"unknown field type");
		  //todo: extend error
			//throw _CLNEW RuntimeException ("unknown field type: "+type);
      }
      store (reader, fieldname, type, factory, comparator);
    }
	return comparator;
  }
  
  
  FieldDoc* FieldSortedHitQueue::fillFields (FieldDoc* doc) const{
    int32_t n = comparatorsLen;
    Comparable** fields = _CL_NEWARRAY(Comparable*,n+1);
    for (int32_t i=0; i<n; ++i)
		fields[i] = comparators[i]->sortValue(&doc->scoreDoc);
	fields[n]=NULL;
    doc->fields = fields;
    if (maxscore > 1.0f) 
        doc->scoreDoc.score /= maxscore;   // normalize scores
    return doc;
  }

  ScoreDocComparator* FieldSortedHitQueue::lookup (IndexReader* reader, const TCHAR* field, int32_t type, SortComparatorSource* factory) {
    ScoreDocComparator* sdc = NULL;
    FieldCacheImpl::FileEntry* entry = (factory != NULL)
	  ? _CLNEW FieldCacheImpl::FileEntry (field, factory)
      : _CLNEW FieldCacheImpl::FileEntry (field, type);
	
	{
		SCOPED_LOCK_MUTEX(Comparators_LOCK)
		hitqueueCacheReaderType* readerCache = Comparators->get(reader);
		if (readerCache == NULL){
			_CLDELETE(entry);
			return NULL;
		}
		
		sdc = readerCache->get (entry);
		_CLDELETE(entry);
	}
	return sdc;
  }

	void FieldSortedHitQueue::closeCallback(CL_NS(index)::IndexReader* reader, void*){
		SCOPED_LOCK_MUTEX(Comparators_LOCK)
		Comparators->remove(reader);
	}
	
  //static
  void FieldSortedHitQueue::store (IndexReader* reader, const TCHAR* field, int32_t type, SortComparatorSource* factory, ScoreDocComparator* value) {
	FieldCacheImpl::FileEntry* entry = (factory != NULL)
		? _CLNEW FieldCacheImpl::FileEntry (field, factory)
		: _CLNEW FieldCacheImpl::FileEntry (field, type);

	{
		SCOPED_LOCK_MUTEX(Comparators_LOCK)
		hitqueueCacheReaderType* readerCache = Comparators->get(reader);
		if (readerCache == NULL) {
			readerCache = _CLNEW hitqueueCacheReaderType(true);
			Comparators->put(reader,readerCache);
			reader->addCloseCallback(FieldSortedHitQueue::closeCallback,NULL);
		}
		readerCache->put (entry, value);
		//return NULL; //supposed to return previous value...
	}
  }

FieldSortedHitQueue::~FieldSortedHitQueue(){
	_CLDELETE_ARRAY(comparators);
    if ( fields != NULL ){
       for ( int i=0;fields[i]!=NULL;i++ )
           _CLDELETE(fields[i]);
       _CLDELETE_ARRAY(fields);
    }
}
CL_NS_END
