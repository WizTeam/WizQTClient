/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_document_FieldSelector_
#define _lucene_document_FieldSelector_

#include "CLucene/util/Equators.h"
#include "CLucene/util/Array.h"
#include "CLucene/util/VoidMap.h"

CL_NS_DEF(document)

/**
 * Similar to a {@link java.io.FileFilter}, the FieldSelector allows one to make decisions about
 * what Fields get loaded on a {@link Document} by {@link org.apache.lucene.index.IndexReader#document(int,org.apache.lucene.document.FieldSelector)}
 *
 **/
class CLUCENE_EXPORT FieldSelector :LUCENE_BASE {
public:

  /**
  *  Provides information about what should be done with this Field
  *
  **/
  enum FieldSelectorResult {
    /**
    * Load this {@link Field} every time the {@link Document} is loaded, reading in the data as it is encounterd.
    *  {@link Document#getField(String)} and {@link Document#getField(String)} should not return null.
    *<p/>
    * {@link Document#add(Field)} should be called by the Reader.
    */
    LOAD = 0,

    /**
    * Lazily load this {@link Field}.  This means the {@link Field} is valid, but it may not actually contain its data until
    * invoked.  {@link Document#getField(String)} SHOULD NOT BE USED.  {@link Document#getField(String)} is safe to use and should
    * return a valid instance of a {@link Field}.
    *<p/>
    * {@link Document#add(Field)} should be called by the Reader.
    */
    LAZY_LOAD = 1,

    /**
    * Do not load the {@link Field}.  {@link Document#getField(String)} and {@link Document#getField(String)} should return null.
    * {@link Document#add(Field)} is not called.
    * <p/>
    * {@link Document#add(Field)} should not be called by the Reader.
    */
    NO_LOAD = 2,

    /**
    * Load this field as in the {@link #LOAD} case, but immediately return from {@link Field} loading for the {@link Document}.  Thus, the
    * Document may not have its complete set of Fields.  {@link Document#getField(String)} and {@link Document#getField(String)} should
    * both be valid for this {@link Field}
    * <p/>
    * {@link Document#add(Field)} should be called by the Reader.
    */
    LOAD_AND_BREAK = 3,

    /**
    * Behaves much like {@link #LOAD} but does not uncompress any compressed data.  This is used for internal purposes.
    * {@link Document#getField(String)} and {@link Document#getField(String)} should not return null.
    * <p/>
    * {@link Document#add(Field)} should be called by the Reader.
    */
    LOAD_FOR_MERGE = 4,

    /** Expert:  Load the size of this {@link Field} rather than its value.
    * Size is measured as number of bytes required to store the field == bytes for a binary or any compressed value, and 2*chars for a String value.
    * The size is stored as a binary value, represented as an int in a byte[], with the higher order byte first in [0]
    */
    SIZE = 5,

    /** Expert: Like {@link #SIZE} but immediately break from the field loading loop, i.e., stop loading further fields, after the size is loaded 		*/
    SIZE_AND_BREAK = 6
  };

	virtual ~FieldSelector();

	/**
	*
	* @param fieldName the field to accept or reject
	* @return an instance of {@link FieldSelectorResult}
	* if the {@link Field} named <code>fieldName</code> should be loaded.
	*/
	virtual FieldSelectorResult accept(const TCHAR* fieldName) const = 0;

};

/**
 * Load the First field and break.
 * <p/>
 * See {@link FieldSelectorResult#LOAD_AND_BREAK}
 */
class CLUCENE_EXPORT LoadFirstFieldSelector :public FieldSelector {
public:
	~LoadFirstFieldSelector();

	FieldSelectorResult accept(const TCHAR* fieldName) const;
};

/**
 * A FieldSelector based on a Map of field names to FieldSelectorResults
 *
 * @author Chuck Williams
 */
class CLUCENE_EXPORT MapFieldSelector: public FieldSelector {
public:
  typedef CL_NS(util)::CLHashMap<TCHAR*, FieldSelectorResult,
    CL_NS(util)::Compare::TChar,
    CL_NS(util)::Equals::TChar,
    CL_NS(util)::Deletor::tcArray,
    CL_NS(util)::Deletor::DummyInt32> FieldSelectionsType;
  FieldSelectionsType* fieldSelections;

  virtual ~MapFieldSelector();

  MapFieldSelector(std::vector<const TCHAR*>& fieldSelections);

  /** Create a a MapFieldSelector
   */
  MapFieldSelector();

  /** Create a a MapFieldSelector
   * @param fields fields to LOAD.  All other fields are NO_LOAD.
   */
  MapFieldSelector(CL_NS(util)::ArrayBase<const TCHAR*>& fields);

  /** Load field according to its associated value in fieldSelections
   * @param field a field name
   * @return the fieldSelections value that field maps to or NO_LOAD if none.
   */
  FieldSelectorResult accept(const TCHAR* field) const;

  void add(const TCHAR*, FieldSelector::FieldSelectorResult action=FieldSelector::LOAD);
};

CL_NS_END
#endif
