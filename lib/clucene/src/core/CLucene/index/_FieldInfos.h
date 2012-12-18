/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_FieldInfos_
#define _lucene_index_FieldInfos_

#include "CLucene/store/Directory.h"

CL_CLASS_DEF(document,Document)
CL_CLASS_DEF(document,Field)

CL_NS_DEF(index)

class FieldInfo :LUCENE_BASE{
  public:
	//name of the field
	const TCHAR*     name;

    //Is field indexed? true = yes false = no
	bool        isIndexed;

	//field number
	const int32_t number;

	// true if term vector for this field should be stored
	bool storeTermVector;
	bool storeOffsetWithTermVector;
	bool storePositionWithTermVector;

	bool omitNorms; // omit norms associated with indexed fields

	bool storePayloads; // whether this field stores payloads together with term positions

	//Func - Constructor
	//       Initialises FieldInfo.
	//       na holds the name of the field
	//       tk indicates whether this field is indexed or not
	//       nu indicates its number
	//Pre  - na != NULL and holds the name of the field
	//       tk is true or false
	//       number >= 0
	//Post - The FieldInfo instance has been created and initialized.
	//       name holds the duplicated string of na
	//       isIndexed = tk
	//       number = nu  
	FieldInfo(const TCHAR* fieldName, 
		const bool isIndexed, 
		const int32_t fieldNumber, 
		const bool storeTermVector,
		const bool storeOffsetWithTermVector,
		const bool storePositionWithTermVector,
		const bool omitNorms,
		const bool storePayloads);

    //Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed
	~FieldInfo();

	/* Clones this 
	* @memory - caller is responsible for deleting the returned object
	*/
	FieldInfo* clone();
};

/** Access to the Field Info file that describes document fields and whether or
 *  not they are indexed. Each segment has a separate Field Info file. Objects
 *  of this class are thread-safe for multiple readers, but only one thread can
 *  be adding documents at a time, with no other reader or writer threads
 *  accessing this object.
 */
class CLUCENE_EXPORT FieldInfos :LUCENE_BASE{
    //we now use internd field names, so we can use the voidCompare
	//to directly compare the strings
	typedef CL_NS(util)::CLHashMap<const TCHAR*,FieldInfo*,
	    CL_NS(util)::Compare::TChar,CL_NS(util)::Equals::TChar > defByName;
	defByName byName;
		
	CL_NS(util)::CLArrayList<FieldInfo*,CL_NS(util)::Deletor::Object<FieldInfo> > byNumber;
public:
	enum{
		IS_INDEXED = 0x1,
		STORE_TERMVECTOR = 0x2,
		STORE_POSITIONS_WITH_TERMVECTOR = 0x4,
		STORE_OFFSET_WITH_TERMVECTOR = 0x8,
		OMIT_NORMS = 0x10,
		STORE_PAYLOADS = 0x20
	};

	FieldInfos();
	~FieldInfos();

	/**
	* Construct a FieldInfos object using the directory and the name of the file
	* IndexInput
	* @param d The directory to open the IndexInput from
	* @param name The name of the file to open the IndexInput from in the Directory
	* @throws IOException
	*/
	FieldInfos(CL_NS(store)::Directory* d, const char* name);

	/**
	* Returns a deep clone of this FieldInfos instance.
	* @memory caller is responisble for deleting returned object
	*/
	FieldInfos* clone();

	/** Adds field info for a Document. */
	void add(const CL_NS(document)::Document* doc);

	/**
	* Add fields that are indexed. Whether they have termvectors has to be specified.
	* 
	* @param names The names of the fields. An array of TCHARs, last item has to be NULL
	* @param storeTermVectors Whether the fields store term vectors or not
	* @param storePositionWithTermVector treu if positions should be stored.
	* @param storeOffsetWithTermVector true if offsets should be stored
	*/
	void addIndexed(const TCHAR** names, const bool storeTermVectors, const bool storePositionWithTermVector, const bool storeOffsetWithTermVector);

	/**
	* Assumes the fields are not storing term vectors.
	* 
	* @param names The names of the fields
	* @param isIndexed Whether the fields are indexed or not
	* 
	* @see #add(TCHAR*, bool)
	*/
	void add(const TCHAR** names, const bool isIndexed, const bool storeTermVector=false,
              const bool storePositionWithTermVector=false, const bool storeOffsetWithTermVector=false,
			  const bool omitNorms=false, const bool storePayloads=false);

	// Merges in information from another FieldInfos. 
	void add(FieldInfos* other);
	
	/** If the field is not yet known, adds it. If it is known, checks to make
	*  sure that the isIndexed flag is the same as was given previously for this
	*  field. If not - marks it as being indexed.  Same goes for the TermVector
	* parameters.
	*
	* @param name The name of the field
	* @param isIndexed true if the field is indexed
	* @param storeTermVector true if the term vector should be stored
	* @param storePositionWithTermVector true if the term vector with positions should be stored
	* @param storeOffsetWithTermVector true if the term vector with offsets should be stored
	* @param omitNorms true if the norms for the indexed field should be omitted
	* @param storePayloads true if payloads should be stored for this field
	*/
	FieldInfo* add(const TCHAR* name, const bool isIndexed, const bool storeTermVector=false,
	          const bool storePositionWithTermVector=false, const bool storeOffsetWithTermVector=false, const bool omitNorms=false, const bool storePayloads=false);

	// was void
	FieldInfo* addInternal( const TCHAR* name,const bool isIndexed, const bool storeTermVector,
		const bool storePositionWithTermVector, const bool storeOffsetWithTermVector, const bool omitNorms, const bool storePayloads);

	int32_t fieldNumber(const TCHAR* fieldName)const;
	
	/**
	* Return the fieldinfo object referenced by the fieldNumber.
	* @param fieldNumber
	* @return the FieldInfo object or null when the given fieldNumber
	* doesn't exist.
	*/ 
	FieldInfo* fieldInfo(const TCHAR* fieldName) const;
	
	/**
	* Return the fieldName identified by its number.
	* 
	* @param fieldNumber
	* @return the fieldName or an empty string when the field
	* with the given number doesn't exist.
	*/  
	const TCHAR* fieldName(const int32_t fieldNumber)const;

	/**
	* Return the fieldinfo object referenced by the fieldNumber.
	* @param fieldNumber
	* @return the FieldInfo object or null when the given fieldNumber
	* doesn't exist.
	*/ 
	FieldInfo* fieldInfo(const int32_t fieldNumber) const;

	size_t size()const;
  	bool hasVectors() const;


	void write(CL_NS(store)::Directory* d, const char* name) const;
	void write(CL_NS(store)::IndexOutput* output) const;

private:
	void read(CL_NS(store)::IndexInput* input);

};
CL_NS_END
#endif
