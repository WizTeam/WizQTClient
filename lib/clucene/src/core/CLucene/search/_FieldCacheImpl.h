/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_FieldCacheImpl_
#define _lucene_search_FieldCacheImpl_

CL_CLASS_DEF(index,IndexReader)
CL_CLASS_DEF(search,SortComparator)
CL_CLASS_DEF(search,SortComparatorSource)
#include "FieldCache.h"
#include "CLucene/LuceneThreads.h"
CL_NS_DEF(search)

class fieldcacheCacheType;

/**
 * Expert: The default cache implementation, storing all values in memory.
 *
 */
class FieldCacheImpl: public FieldCache {
public:
  	DEFINE_MUTEX(THIS_LOCK)
  
	/** Expert: Every key in the internal cache is of this type. */
	class FileEntry:LUCENE_BASE {
		const TCHAR* field;        // which Field
		int32_t type;            // which SortField type
		SortComparatorSource* custom;       // which custom comparator
		size_t _hashCode;
	public:
		/** Creates one of these objects. */
		FileEntry (const TCHAR* field, int32_t type);
	   
		/** Creates one of these objects for a custom comparator. */
		FileEntry (const TCHAR* field, SortComparatorSource* custom);
		~FileEntry();

		int32_t getType() const{ return type; }
	   
		/** Two of these are equal iff they reference the same field and type. */
		bool equals (FileEntry* other) const;
	   
		/** Composes a hashcode based on the field and type. */
		size_t hashCode();

		int32_t compareTo(const FileEntry* other) const;

        class Compare:LUCENE_BASE, public CL_NS(util)::Compare::_base //<Term*>
		{
		public:
			bool operator()( FileEntry* f1, FileEntry* f2 ) const{
				return ( f1->compareTo(f2) < 0 );
			}
			size_t operator()( FileEntry* t ) const{
				return t->hashCode();
			}
		};
		class Equals:LUCENE_BASE, public CL_NS(util)::Compare::_base //<Term*>
		{
		public:
			bool operator()( FileEntry* f1, FileEntry* f2 ) const{
				return ( f1->compareTo(f2) == 0 );
			}
		};
	}; 

    FieldCacheImpl();
    virtual ~FieldCacheImpl();
private:
  /** The internal cache. Maps FileEntry to array of interpreted term values. **/
  //todo: make indexreader remove itself from here when the reader is shut
  fieldcacheCacheType* cache;
  
  /** See if an object is in the cache. */
  FieldCacheAuto* lookup (CL_NS(index)::IndexReader* reader, const TCHAR* field, int32_t type) ;
 
  /** See if a custom object is in the cache. */
  FieldCacheAuto* lookup (CL_NS(index)::IndexReader* reader, const TCHAR* field, SortComparatorSource* comparer);

  /** Put an object into the cache. */
  void store (CL_NS(index)::IndexReader* reader, const TCHAR* field, int32_t type, FieldCacheAuto* value);

  /** Put a custom object into the cache. */
  void store (CL_NS(index)::IndexReader* reader, const TCHAR* field, SortComparatorSource* comparer, FieldCacheAuto* value);
  
public:

  // inherit javadocs
  FieldCacheAuto* getInts (CL_NS(index)::IndexReader* reader, const TCHAR* field);

  // inherit javadocs
  FieldCacheAuto* getFloats (CL_NS(index)::IndexReader* reader, const TCHAR* field);

  // inherit javadocs
  FieldCacheAuto* getStrings (CL_NS(index)::IndexReader* reader, const TCHAR* field);

  // inherit javadocs
  FieldCacheAuto* getStringIndex (CL_NS(index)::IndexReader* reader, const TCHAR* field);

  // inherit javadocs
  FieldCacheAuto* getAuto (CL_NS(index)::IndexReader* reader, const TCHAR* field);

  // inherit javadocs
  FieldCacheAuto* getCustom (CL_NS(index)::IndexReader* reader, const TCHAR* field, SortComparator* comparator);


	/**
	* Callback for when IndexReader closes. This causes
	* any cache to be removed for the specified reader.
	*/
	static void closeCallback(CL_NS(index)::IndexReader* reader, void* fieldCacheImpl);
};


CL_NS_END
#endif
