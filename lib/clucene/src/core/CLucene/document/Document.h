/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_document_Document_
#define _lucene_document_Document_

#include "CLucene/util/VoidList.h"
#include "Field.h"

///todo: jlucene has change from using DocumentFieldList/Enumeration
///to using a java List... do we want to do this too?
CL_NS_DEF(document)

class DocumentFieldEnumeration;

/** Documents are the unit of indexing and search.
*
* A Document is a set of fields.  Each field has a name and a textual value.
* A field may be {@link Field#isStored() stored} with the document, in which
* case it is returned with search hits on the document.  Thus each document
* should typically contain one or more stored fields which uniquely identify
* it.
*
* <p>Note that fields which are <i>not</i> {@link Field#isStored() stored} are
* <i>not</i> available in documents retrieved from the index, e.g. with {@link
* Hits#doc(int32_t, Document*)}, {@link Searcher#doc(int32_t, Document*)} or {@link
* IndexReader#document(int32_t, Document*)}.
*/
class CLUCENE_EXPORT Document:LUCENE_BASE {
public:
  typedef CL_NS(util)::CLArrayList<Field*,CL_NS(util)::Deletor::Object<Field> > FieldsType;
private:
	FieldsType* _fields;
	float_t boost;
public:
	/** Constructs a new document with no fields. */
	Document();

	~Document();

	/** Sets a boost factor for hits on any field of this document.  This value
	* will be multiplied into the score of all hits on this document.
	*
	* <p>The default value is 1.0.
	* 
	* <p>Values are multiplied into the value of {@link Field#getBoost()} of
	* each field in this document.  Thus, this method in effect sets a default
	* boost for the fields of this document.
	*
	* @see Field#setBoost(float_t)
	*/
	void setBoost(const float_t boost);
  
	/** Returns, at indexing time, the boost factor as set by {@link #setBoost(float_t)}. 
	*
	* <p>Note that once a document is indexed this value is no longer available
	* from the index.  At search time, for retrieved documents, this method always 
	* returns 1. This however does not mean that the boost value set at  indexing 
	* time was ignored - it was just combined with other indexing time factors and 
	* stored elsewhere, for better indexing and search performance. (For more 
	* information see the "norm(t,d)" part of the scoring formula in 
	* {@link Similarity}.)
	*
	* @see #setBoost(float_t)
	*/
	float_t getBoost() const;

	/**
	* <p>Adds a field to a document.  Several fields may be added with
	* the same name.  In this case, if the fields are indexed, their text is
	* treated as though appended for the purposes of search.</p>
	* <p> Note that add like the removeField(s) methods only makes sense 
	* prior to adding a document to an index. These methods cannot
	* be used to change the content of an existing index! In order to achieve this,
	* a document has to be deleted from an index and a new changed version of that
	* document has to be added.</p>
	*
	*/
	void add(Field& field);

	/**
	* <p>Removes field with the specified name from the document.
	* If multiple fields exist with this name, this method removes the first field that has been added.
	* If there is no field with the specified name, the document remains unchanged.</p>
	* <p> Note that the removeField(s) methods like the add method only make sense 
	* prior to adding a document to an index. These methods cannot
	* be used to change the content of an existing index! In order to achieve this,
	* a document has to be deleted from an index and a new changed version of that
	* document has to be added.</p>
	* Note: name is case sensitive
	*/
	void removeField(const TCHAR* name);

	/**
	* <p>Removes all fields with the given name from the document.
	* If there is no field with the specified name, the document remains unchanged.</p>
	* <p> Note that the removeField(s) methods like the add method only make sense 
	* prior to adding a document to an index. These methods cannot
	* be used to change the content of an existing index! In order to achieve this,
	* a document has to be deleted from an index and a new changed version of that
	* document has to be added.</p>
	* Note: name is case sensitive
	*/
	void removeFields(const TCHAR* name);

	/** Returns a field with the given name if any exist in this document, or
	* null.  If multiple fields exists with this name, this method returns the
	* first value added. 
	* Note: name is case sensitive
	* Do not use this method with lazy loaded fields.
	*/
	Field* getField(const TCHAR* name) const;
	
	/** Returns the string value of the field with the given name if any exist in
	* this document, or null.  If multiple fields exist with this name, this
	* method returns the first value added. If only binary fields with this name
	* exist, returns null.
	* Note: name is case sensitive
	*/
	const TCHAR* get(const TCHAR* field) const;

  /** Returns an Enumeration of all the fields in a document.
  * @deprecated use {@link #getFields()} instead
  */
  _CL_DEPRECATED(  getFields() ) DocumentFieldEnumeration* fields();

  /** Returns a List of all the fields in a document.
   * <p>Note that fields which are <i>not</i> {@link Field#isStored() stored} are
   * <i>not</i> available in documents retrieved from the index, e.g. with {@link
   * Hits#doc(int)}, {@link Searcher#doc(int)} or {@link IndexReader#document(int)}.
   */
  const FieldsType* getFields() const;

  /**
   * Returns an array of {@link Field}s with the given name.
   * This method can return <code>null</code>.
   *
   * @param name the name of the field
   * @return a <code>Field[]</code> array or <code>null</code>
   */
  void getFields(const TCHAR* name, std::vector<Field*>& ret);

	/** Prints the fields of a document for human consumption. */
	TCHAR* toString() const;
        

	/**
	* Returns an array of values of the field specified as the method parameter.
	* This method can return <code>null</code>.
	* Note: name is case sensitive
	*
	* @param name the name of the field
	* @return a <code>TCHAR**</code> of field values or <code>null</code>
	*/
	TCHAR** getValues(const TCHAR* name);
	
	/**
	* Empties out the document so that it can be reused
	*/
	void clear();
};


class CLUCENE_EXPORT DocumentFieldEnumeration :LUCENE_BASE{
private:
  struct Internal;
  Internal* _internal;
public:
  DocumentFieldEnumeration(Document::FieldsType::iterator itr, Document::FieldsType::iterator end);
  ~DocumentFieldEnumeration();
  bool hasMoreElements() const;
  Field* nextElement();
};
CL_NS_END
#endif
