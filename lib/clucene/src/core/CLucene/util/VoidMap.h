/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_util_VoidMap_
#define _lucene_util_VoidMap_

#include "Equators.h"
#include "CLucene/LuceneThreads.h"

#if defined(_CL_HAVE_TR1_UNORDERED_MAP) && defined(_CL_HAVE_TR1_UNORDERED_SET)
	#include <tr1/unordered_map>
	#include <tr1/unordered_set>
#elif defined(_CL_HAVE_HASH_MAP) && defined(_CL_HAVE_HASH_SET)
	//hashing is all or nothing!
	#include <hash_map>
	#include <hash_set>
#elif defined(_CL_HAVE_EXT_HASH_MAP) && defined(_CL_HAVE_EXT_HASH_SET)
	#include <ext/hash_map>
	#include <ext/hash_set>
#elif !defined(LUCENE_DISABLE_HASHING)
	#define LUCENE_DISABLE_HASHING
#endif

CL_NS_DEF(util)

/**
* A template to encapsulate various map type classes
* @internal
*/
template<typename _kt, typename _vt,
	typename _base,
	typename _KeyDeletor=CL_NS(util)::Deletor::Dummy,
	typename _ValueDeletor=CL_NS(util)::Deletor::Dummy>
class CLUCENE_INLINE_EXPORT __CLMap:public _base,LUCENE_BASE {
protected:
	bool dk;
	bool dv;
	typedef _base base;
public:
	typedef typename _base::iterator iterator;
	typedef typename _base::const_iterator const_iterator;
	typedef CL_NS_STD(pair)<_kt, _vt> _pair;

	///Default constructor for the __CLMap
	__CLMap ():
		dk(true),
		dv(true)
	{
	}

	///Deconstructor for the __CLMap
	~__CLMap (){
		clear();
	}

	void setDeleteKey(bool val){ dk = val; }
	void setDeleteValue(bool val){ dv = val; }

	///Construct the VoidMap and set the deleteTypes to the specified values
	///\param deleteKey if true then the key variable is deleted when an object is deleted
	///\param keyDelType delete the key variable using the specified type
	///\param deleteValue if true then the value variable is deleted when an object is deleted
	///\param valueDelType delete the value variable using the specified type
	/*__CLMap ( const bool deleteKey, const bool deleteValue ):
		dk(deleteKey),
		dv(deleteValue)
	{
	}*/

	///checks to see if the specified key exists
	///\param k the key to check for
	///\returns true if the key exists
	bool exists(_kt k)const{
		const_iterator itr = base::find(k);
		bool ret = itr!=base::end();
		return ret;
	}

	///using a non-const key, get a non-const value
	_vt get( _kt k) const {
		const_iterator itr = base::find(k);
		if ( itr==base::end() )
			return (_vt)NULL;
		else
			return itr->second;
	}
	/*
	///using a non-const key, get the actual key
	_kt getKey( _kt k) const {
		const_iterator itr = base::find(k);
		if ( itr==base::end() )
			return NULL;
		else
			return itr->first;
	}*/

	void removeitr (iterator itr, const bool dontDeleteKey = false, const bool dontDeleteValue = false){
	  if ( itr == base::end() ) return;

		//delete key&val first. This prevents potential loops (deleting object removes itself)
		_kt key = itr->first;
		_vt val = itr->second;
		base::erase(itr);

		//keys & vals need to be deleted after erase, because the hashvalue is still needed
		if ( dk && !dontDeleteKey )
			_KeyDeletor::doDelete(key);
		if ( dv && !dontDeleteValue )
			_ValueDeletor::doDelete(val);
	}
	///delete and optionally delete the specified key and associated value
	void remove(_kt key, const bool dontDeleteKey = false, const bool dontDeleteValue = false){
		iterator itr = base::find(key);
		if ( itr!=base::end() )
			removeitr(itr,dontDeleteKey,dontDeleteValue);
	}

	///clear all keys and values in the map
	void clear(){
		if ( dk || dv ){
			iterator itr = base::begin();
			while ( itr!=base::end() ){
				#ifdef _CL_HAVE_EXT_HASH_MAP
				removeitr(itr);
				itr = base::begin();

				#else
				if ( dk )
					_KeyDeletor::doDelete(itr->first);
				if ( dv )
					_ValueDeletor::doDelete(itr->second);
				++itr;

				#endif
			}
		}
		base::clear();
	}
};

// makes no guarantees as to the order of the map
// cannot contain duplicate keys; each key can map to at most one value
#define CLHashtable CLHashMap

#if defined(LUCENE_DISABLE_HASHING)

 //a CLSet with CLHashMap traits
template<typename _kt, typename _vt,
	typename _Compare,
	typename _EqualDummy,
	typename _KeyDeletor=CL_NS(util)::Deletor::Dummy,
	typename _ValueDeletor=CL_NS(util)::Deletor::Dummy>
class CLUCENE_INLINE_EXPORT CLHashMap:public __CLMap<_kt,_vt,
	CL_NS_STD(map)<_kt,_vt, _Compare>,
	_KeyDeletor,_ValueDeletor>
{
	typedef typename CL_NS_STD(map)<_kt,_vt,_Compare> _base;
	typedef __CLMap<_kt, _vt, CL_NS_STD(map)<_kt,_vt, _Compare>,
		_KeyDeletor,_ValueDeletor> _this;
public:
	CLHashMap ( const bool deleteKey=false, const bool deleteValue=false )
	{
		_this::setDeleteKey(deleteKey);
		_this::setDeleteValue(deleteValue);
	}

	///put the specified pair into the map. remove any old items first
	///\param k the key
	///\param v the value
	virtual void put(_kt k,_vt v){
		//todo: check if this is always right!
		//must should look through code, for
		//cases where map is not unique!!!
		if ( _this::dk || _this::dv )
			_this::remove(k);

		(*this)[k] = v;;
	}

};
#elif defined(_CL_HAVE_EXT_HASH_MAP)
 //ext/hash_map syntax
//HashMap  class is roughly equivalent to Hashtable, except that it is unsynchronized
template<typename _kt, typename _vt,
	typename _Hasher,
	typename _Equals,
	typename _KeyDeletor=CL_NS(util)::Deletor::Dummy,
	typename _ValueDeletor=CL_NS(util)::Deletor::Dummy>
class CLUCENE_INLINE_EXPORT CLHashMap:public __CLMap<_kt,_vt,
	CL_NS_HASHING(_CL_HASH_MAP)<_kt,_vt, _Hasher,_Equals>,
	_KeyDeletor,_ValueDeletor>
{
	typedef __CLMap<_kt,_vt, CL_NS_HASHING(_CL_HASH_MAP)<_kt,_vt, _Hasher,_Equals>,
		_KeyDeletor,_ValueDeletor> _this;
public:
	CLHashMap ( const bool deleteKey=false, const bool deleteValue=false )
	{
		_this::setDeleteKey(deleteKey);
		_this::setDeleteValue(deleteValue);
	}
	///put the specified pair into the map. remove any old items first
	///\param k the key
	///\param v the value
	virtual void put(_kt k,_vt v){
		//todo: check if this is always right!
		//must should look through code, for
		//cases where map is not unique!!!
		if ( _this::dk || _this::dv )
			_this::remove(k);

		(*this)[k] = v;;
	}

};

#elif defined(_CL_HAVE_HASH_MAP)

//HashMap  class is roughly equivalent to Hashtable, except that it is unsynchronized
template<typename _kt, typename _vt,
	typename _Hasher,
	typename _Equals,
	typename _KeyDeletor=CL_NS(util)::Deletor::Dummy,
	typename _ValueDeletor=CL_NS(util)::Deletor::Dummy>
class CLUCENE_INLINE_EXPORT CLHashMap:public __CLMap<_kt,_vt,
	CL_NS_HASHING(_CL_HASH_MAP)<_kt,_vt, _Hasher>,
	_KeyDeletor,_ValueDeletor>
{
	typedef __CLMap<_kt,_vt, CL_NS_HASHING(_CL_HASH_MAP)<_kt,_vt, _Hasher>,
		_KeyDeletor,_ValueDeletor> _this;
public:
	CLHashMap ( const bool deleteKey=false, const bool deleteValue=false )
	{
		_this::setDeleteKey(deleteKey);
		_this::setDeleteValue(deleteValue);
	}
	///put the specified pair into the map. remove any old items first
	///\param k the key
	///\param v the value
	virtual void put(_kt k,_vt v){
		//todo: check if this is always right!
		//must should look through code, for
		//cases where map is not unique!!!
		if ( _this::dk || _this::dv )
			_this::remove(k);

		(*this)[k] = v;;
	}

};
#endif

//A collection that contains no duplicates
//does not guarantee that the order will remain constant over time
template<typename _kt, typename _vt,
	typename _Compare,
	typename _KeyDeletor=CL_NS(util)::Deletor::Dummy,
	typename _ValueDeletor=CL_NS(util)::Deletor::Dummy>
class CLUCENE_INLINE_EXPORT CLSet:public __CLMap<_kt,_vt,
	CL_NS_STD(map)<_kt,_vt, _Compare>,
	_KeyDeletor,_ValueDeletor>
{
	typedef typename CL_NS_STD(map)<_kt,_vt,_Compare> _base;
	typedef __CLMap<_kt, _vt, CL_NS_STD(map)<_kt,_vt, _Compare>,
		_KeyDeletor,_ValueDeletor> _this;
public:
	CLSet ( const bool deleteKey=false, const bool deleteValue=false )
	{
		_this::setDeleteKey(deleteKey);
		_this::setDeleteValue(deleteValue);
	}
	///put the specified pair into the map. remove any old items first
	///\param k the key
	///\param v the value
	virtual void put(_kt k,_vt v){
		//todo: check if this is always right!
		//must should look through code, for
		//cases where map is not unique!!!
		if ( _this::dk || _this::dv )
			_this::remove(k);

		(*this)[k] = v;;
	}

};


//A collection that can contains duplicates
template<typename _kt, typename _vt,
	typename _Compare,
	typename _KeyDeletor=CL_NS(util)::Deletor::Dummy,
	typename _ValueDeletor=CL_NS(util)::Deletor::Dummy>
class CLUCENE_INLINE_EXPORT CLMultiMap:public __CLMap<_kt,_vt,
	CL_NS_STD(multimap)<_kt,_vt>,
	_KeyDeletor,_ValueDeletor>
{
	typedef typename CL_NS_STD(multimap)<_kt,_vt> _base;
	typedef __CLMap<_kt, _vt, CL_NS_STD(multimap)<_kt,_vt>,
		_KeyDeletor,_ValueDeletor> _this;
public:
	CLMultiMap ( const bool deleteKey=false, const bool deleteValue=false )
	{
		_this::setDeleteKey(deleteKey);
		_this::setDeleteValue(deleteValue);
	}

	///put the specified pair into the map. remove any old items first
	///\param k the key
	///\param v the value
	void put(_kt k,_vt v){
		//todo: check if this is always right!
		//must should look through code, for
		//cases where map is not unique!!!
		if ( _this::dk || _this::dv )
			_this::remove(k);

	}
};


//*** need to create a class that allows duplicates - use <set>
//#define CLSet __CLMap
CL_NS_END

#endif
