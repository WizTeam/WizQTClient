/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_util_VoidList_
#define _lucene_util_VoidList_

#include "Equators.h"
#include "CLucene/LuceneThreads.h"

CL_NS_DEF(util)

/**
* A template to encapsulate various list type classes
* @internal
*/
template<typename _kt,typename _base,typename _valueDeletor>
class CLUCENE_INLINE_EXPORT __CLList:public _base,LUCENE_BASE {
private:
	bool dv;
protected:
	typedef _base base;
public:
	typedef typename _base::const_iterator const_iterator;
	typedef typename _base::iterator iterator;

	virtual ~__CLList(){
		clear();
	}

	__CLList ( const bool deleteValue ):
		dv(deleteValue)
	{
	}

	void setDoDelete(bool val){ dv=val; }

	//sets array to the contents of this array.
	//array must be size
	void toArray(_kt* into) const{
		int i=0;
		for ( const_iterator itr=base::begin();itr!=base::end();itr++ ){
			into[i] = *itr;
			i++;
		}
	}

	//sets array to the contents of this array, terminating with a NULL pointer
	//array must be size+1
	void toArray_nullTerminated(_kt* into) const{
		int i=0;
		for ( const_iterator itr=base::begin();itr!=base::end();itr++ ){
			into[i] = *itr;
			i++;
		}
		into[i] = NULL;
	}

	void set(size_t i, _kt val) {
    if ( dv && i < base::size() )
			_valueDeletor::doDelete((*this)[i]);
    if ( i+1 > base::size() ) base::resize(i+1);
		(*this)[i] = val;
	}

	//todo: check this
	void delete_back(){
		if ( base::size() > 0 ){
			iterator itr = base::end();
			if ( itr != base::begin())
				itr --;
			_kt key = *itr;
			base::erase(itr);
			if ( dv )
				_valueDeletor::doDelete(key);
		}
	}

	void delete_front(){
		if ( base::size() > 0 ){
			iterator itr = base::begin();
			_kt key = *itr;
			base::erase(itr);
			if ( dv )
				_valueDeletor::doDelete(key);
		}
	}

	void clear(){
		if ( dv ){
			iterator itr = base::begin();
			while ( itr != base::end() ){
				_valueDeletor::doDelete(*itr);
				++itr;
			}
		}
		base::clear();
	}

	void remove(size_t i, bool dontDelete=false){
	    if ( i < base::size() ){
		  iterator itr=base::begin();
		  itr+=i;
		  _kt key = *itr;
		  base::erase( itr );
		  if ( dv && !dontDelete )
			  _valueDeletor::doDelete(key);
	    }
	}
	void remove(iterator itr, bool dontDelete=false){
		_kt key = *itr;
		base::erase( itr );
		if ( dv && !dontDelete )
			_valueDeletor::doDelete(key);
	}
};



//growable arrays of Objects (like a collection or list)
//a list, so can contain duplicates
//it grows in chunks... todo: check jlucene for initial size of array, and growfactors
template<typename _kt, typename _valueDeletor=CL_NS(util)::Deletor::Dummy>
class CLUCENE_INLINE_EXPORT CLVector:public __CLList<_kt, CL_NS_STD(vector)<_kt> , _valueDeletor>
{
public:
	CLVector ( const bool deleteValue=true ):
	  __CLList<_kt, CL_NS_STD(vector)<_kt> , _valueDeletor>(deleteValue)
	{
	}
};

//An array-backed implementation of the List interface
//a list, so can contain duplicates
//*** a very simple list - use <valarray>
//(This class is roughly equivalent to Vector, except that it is unsynchronized.)
#define CLArrayList CLVector
#define CLHashSet CLHashList
#define CLList CLVector

//implementation of the List interface, provides access to the first and last list elements in O(1)
//no comparator is required... and so can contain duplicates
//a simple list with no comparator
//*** a very simple list - use <list>
#ifdef LUCENE_DISABLE_HASHING
   #define CLHashList CLSetList
#else

template<typename _kt,
	typename _Comparator=CL_NS(util)::Compare::TChar,
	typename _valueDeletor=CL_NS(util)::Deletor::Dummy>
class CLUCENE_INLINE_EXPORT CLHashList:public __CLList<_kt, CL_NS_HASHING(_CL_HASH_SET)<_kt,_Comparator> , _valueDeletor>
{
public:
	CLHashList ( const bool deleteValue=true ):
	  __CLList<_kt, CL_NS_HASHING(_CL_HASH_SET)<_kt,_Comparator> , _valueDeletor>(deleteValue)
	{
	}
};
#endif

template<typename _kt, typename _valueDeletor=CL_NS(util)::Deletor::Dummy>
class CLUCENE_INLINE_EXPORT CLLinkedList:public __CLList<_kt, CL_NS_STD(list)<_kt> , _valueDeletor>
{
public:
	CLLinkedList ( const bool deleteValue=true ):
	  __CLList<_kt, CL_NS_STD(list)<_kt> , _valueDeletor>(deleteValue)
	{
	}
};
template<typename _kt,
	typename _Comparator=CL_NS(util)::Compare::TChar,
	typename _valueDeletor=CL_NS(util)::Deletor::Dummy>
class CLUCENE_INLINE_EXPORT CLSetList:public __CLList<_kt, CL_NS_STD(set)<_kt,_Comparator> , _valueDeletor>
{
public:
	CLSetList ( const bool deleteValue=true ):
	  __CLList<_kt, CL_NS_STD(set)<_kt,_Comparator> , _valueDeletor>(deleteValue)
	{
	}
};

CL_NS_END
#endif
