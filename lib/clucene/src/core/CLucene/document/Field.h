/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_document_Field_
#define _lucene_document_Field_

#include "CLucene/util/Array.h"
#include "CLucene/util/Equators.h"
/*
Fieldable reading:
https://issues.apache.org/jira/browse/LUCENE-1219?page=com.atlassian.jira.plugin.system.issuetabpanels:comment- tabpanel&focusedCommentId=12578199#action_12578199
http://lucene.markmail.org/message/ioi4f6z24cbd5bdm?q=Fieldable#query:Fieldable+page:1+mid:fxmvzb6up7zve7k4+state:results

TODO: - Solve some inconsistencies between CL and JL - mainly in the constructors area.
	  - Write some more tests to make sure we conform with JL - mainly in the tokenizing and omitNorms area
	  - Is there a bug in JL when calling setOmitNorms after a Tokenized field was created?
*/

CL_CLASS_DEF(util,Reader)
CL_CLASS_DEF(analysis,TokenStream)

CL_NS_DEF(document)

/**
A field is a section of a Document.  Each field has two parts, a name and a
value.  Values may be free text, provided as a String or as a Reader, or they
may be atomic keywords, which are not further processed.  Such keywords may
be used to represent dates, urls, etc.  Fields are optionally stored in the
index, so that they may be returned with hits on the document.
*/
class CLUCENE_EXPORT Field : public CL_NS(util)::NamedObject{
public:
	enum Store{ 
		/** Store the original field value in the index. This is useful for short texts
		* like a document's title which should be displayed with the results. The
		* value is stored in its original form, i.e. no analyzer is used before it is
		* stored.
		*/
		STORE_YES=1,

		/** Do not store the field value in the index. */
		STORE_NO=2,

		/** Store the original field value in the index in a compressed form. This is
    * useful for long documents and for binary valued fields.
    */
    STORE_COMPRESS=4
	};

	enum Index{ 
		/** Do not index the field value. This field can thus not be searched,
		* but one can still access its contents provided it is
		* {@link Field::Store stored}. */
		INDEX_NO=16, 

		/** Index the field's value so it can be searched. An Analyzer will be used
		* to tokenize and possibly further normalize the text before its
		* terms will be stored in the index. This is useful for common text.
		*/
		INDEX_TOKENIZED=32, 

		/** Index the field's value without using an Analyzer, so it can be searched.
		* As no analyzer is used the value will be stored as a single term. This is
		* useful for unique Ids like product numbers.
		*/
		INDEX_UNTOKENIZED=64, 

		/** Index the field's value without an Analyzer, and disable
		* the storing of norms.  No norms means that index-time boosting
		* and field length normalization will be disabled.  The benefit is
		* less memory usage as norms take up one byte per indexed field
		* for every document in the index.
		* Note that once you index a given field <i>with</i> norms enabled,
		* disabling norms will have no effect.  In other words, for NO_NORMS
		* to have the above described effect on a field, all instances of that
		* field must be indexed with NO_NORMS from the beginning.
		*/
		INDEX_NONORMS=128
	};

	enum TermVector{
		/** Do not store term vectors. */
		TERMVECTOR_NO=256, 

		/** Store the term vectors of each document. A term vector is a list
		* of the document's terms and their number of occurences in that document. */
		TERMVECTOR_YES=512,

		/**
		* Store the term vector + token position information
		* 
		* @see #YES
		*/ 
		TERMVECTOR_WITH_POSITIONS = TERMVECTOR_YES | 1024,

		/**
		* Store the term vector + Token offset information
		* 
		* @see #YES
		*/ 
		TERMVECTOR_WITH_OFFSETS = TERMVECTOR_YES | 2048,

		/**
		* Store the term vector + Token position and offset information
		* 
		* @see #YES
		* @see #WITH_POSITIONS
		* @see #WITH_OFFSETS
		*/ 
		TERMVECTOR_WITH_POSITIONS_OFFSETS = TERMVECTOR_WITH_OFFSETS | TERMVECTOR_WITH_POSITIONS
	};

	bool lazy;

	enum ValueType {
		VALUE_NONE = 0,
		VALUE_STRING = 1,
		VALUE_READER = 2,
		VALUE_BINARY = 4,
		VALUE_TOKENSTREAM = 8
	};

	/**
	* TCHAR value constructor of Field.
	* @memory Set duplicateValue to false to save on memory allocations when possible
	*/
	Field(const TCHAR* name, const TCHAR* value, int _config, const bool duplicateValue = true);

	/**
	* Reader* constructor of Field.
	* @memory consumes reader
	*/
	Field(const TCHAR* name, CL_NS(util)::Reader* reader, int _config);

	/**
	* Binary constructor of Field.
	* @memory Set duplicateValue to false to save on memory allocations when possible
	*/
  Field(const TCHAR* name, CL_NS(util)::ValueArray<uint8_t>* data, int _config, const bool duplicateValue = true);

	Field(const TCHAR* name, int _config); ///<No value, for lazy loading support
  virtual ~Field();

	/**  The name of the field (e.g., "date", "subject", "title", "body", etc.)
	*	as an interned string. */
	const TCHAR* name() const; ///<returns reference

	/** The value of the field as a String, or null.  If null, the Reader value
	* or binary value is used.  Exactly one of stringValue(), readerValue() and
	* binaryValue() must be set. */
	virtual const TCHAR* stringValue(); ///<returns reference

	/** The value of the field as a reader, or null.  If null, the String value
	* or binary value is used.  Exactly one of stringValue(), readerValue() and
	* binaryValue() must be set. */
	virtual CL_NS(util)::Reader* readerValue();

	/** The value of the field as a String, or null.  If null, the String value
	* or Reader value is used.  Exactly one of stringValue(), readerValue() and
	* binaryValue() must be set. */
	virtual const CL_NS(util)::ValueArray<uint8_t>* binaryValue();

	/** The value of the field as a TokesStream, or null.  If null, the Reader value,
	* String value, or binary value is used. Exactly one of stringValue(), 
	* readerValue(), binaryValue(), and tokenStreamValue() must be set. */
	virtual CL_NS(analysis)::TokenStream* tokenStreamValue();

	//  True if the value of the field is to be stored in the index for return
	//	with search hits.  It is an error for this to be true if a field is
	//	Reader-valued. 
	bool isStored() const;

	//  True if the value of the field is to be indexed, so that it may be
	//	searched on. 
	bool isIndexed() const;

	// True if the value of the field should be tokenized as text prior to
	//	indexing.  Un-tokenized fields are indexed as a single word and may not be
	//	Reader-valued.
	bool isTokenized() const;
	
	/** True if the value of the field is stored and compressed within the index 
	*/
	bool isCompressed() const;

	/** True if the term or terms used to index this field are stored as a term
	*  vector, available from {@link IndexReader#getTermFreqVector(int32_t,TCHAR*)}.
	*  These methods do not provide access to the original content of the field,
	*  only to terms used to index it. If the original content must be
	*  preserved, use the <code>stored</code> attribute instead.
	*
	* @see IndexReader#getTermFreqVector(int32_t, String)
	*/
	bool isTermVectorStored() const;

	/**
	* True iff terms are stored as term vector together with their offsets 
	* (start and end positon in source text).
	*/
	bool isStoreOffsetWithTermVector() const;
	  
	/**
	* True iff terms are stored as term vector together with their token positions.
	*/
	bool isStorePositionWithTermVector() const;

	/** Returns the boost factor for hits for this field.
	*
	* <p>The default value is 1.0.
	*
	* <p>Note: this value is not stored directly with the document in the index.
	* Documents returned from {@link IndexReader#document(int)} and
	* {@link Hits#doc(int)} may thus not have the same value present as when
	* this field was indexed.
	*
	* @see #setBoost(float_t)
	*/
	float_t getBoost() const;
      
	/** Sets the boost factor hits on this field.  This value will be
	* multiplied into the score of all hits on this this field of this
	* document.
	*
	* <p>The boost is multiplied by {@link Document#getBoost()} of the document
	* containing this field.  If a document has multiple fields with the same
	* name, all such values are multiplied together.  This product is then
	* multipled by the value {@link Similarity#lengthNorm(String,int)}, and
	* rounded by {@link Similarity#encodeNorm(float_t)} before it is stored in the
	* index.  One should attempt to ensure that this product does not overflow
	* the range of that encoding.
	*
	* @see Document#setBoost(float_t)
	* @see Similarity#lengthNorm(String, int)
	* @see Similarity#encodeNorm(float_t)
	*/
	void setBoost(const float_t value);

	/** True if the value of the filed is stored as binary */
	bool isBinary() const;
	
	/** True if norms are omitted for this indexed field */
	bool getOmitNorms() const;

	/** Expert:
	*
	* If set, omit normalization factors associated with this indexed field.
	* This effectively disables indexing boosts and length normalization for this field.
	*/
	void setOmitNorms(const bool omitNorms);

	/**
	* Indicates whether a Field is Lazy or not.  The semantics of Lazy loading are such that if a Field is lazily loaded, retrieving
	* it's values via {@link #stringValue()} or {@link #binaryValue()} is only valid as long as the {@link org.apache.lucene.index.IndexReader} that
	* retrieved the {@link Document} is still open.
	*  
	* @return true if this field can be loaded lazily
	*/
	bool isLazy() const;

	/** Prints a Field for human consumption. */
	TCHAR* toString();

	/** <p>Expert: change the value of this field.  This can
	*  be used during indexing to re-use a single Field
	*  instance to improve indexing speed by avoiding GC cost
	*  of new'ing and reclaiming Field instances.  Typically
	*  a single {@link Document} instance is re-used as
	*  well.  This helps most on small documents.</p>
	* 
	*  <p>Note that you should only use this method after the
	*  Field has been consumed (ie, the {@link Document}
	*  containing this Field has been added to the index).
	*  Also, each Field instance should only be used once
	*  within a single {@link Document} instance.  See <a
	*  href="http://wiki.apache.org/lucene-java/ImproveIndexingSpeed">ImproveIndexingSpeed</a>
	*  for details.</p>
	*
	* @memory Caller is responsible for releasing value if duplicateValue == false */
	void setValue(TCHAR* value, const bool duplicateValue = true);

	/** Expert: change the value of this field.  See <a href="#setValue(TCHAR*)">setValue(TCHAR*)</a>. */
	void setValue(CL_NS(util)::Reader* value);

	/** Expert: change the value of this field.  See <a href="#setValue(TCHAR*)">setValue(TCHAR*)</a>. */
	void setValue(CL_NS(util)::ValueArray<uint8_t>* value) ;

	/** Expert: change the value of this field.  See <a href="#setValue(TCHAR*)">setValue(TCHAR*)</a>. */
	void setValue(CL_NS(analysis)::TokenStream* value);

	virtual const char* getObjectName() const;
	static const char* getClassName();

protected:
	/**
	* Set configs using XOR. This resets all the settings
	* For example, to use term vectors with positions and offsets do:
	* object->setConfig(TERMVECTOR_WITH_POSITIONS | TERMVECTOR_WITH_OFFSETS);
	*/
	void setConfig(const uint32_t _config);

	void _resetValue();

	void* fieldsData;
	ValueType valueType;

	const TCHAR* _name;
	uint32_t config;
	float_t boost;
};
CL_NS_END
#endif
