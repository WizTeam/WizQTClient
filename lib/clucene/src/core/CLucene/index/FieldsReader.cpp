/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"

#include <assert.h>
#include "CLucene/util/Misc.h"
#include "CLucene/util/_StringIntern.h"
#include "CLucene/util/CLStreams.h"
#include "CLucene/store/Directory.h"
#include "CLucene/store/IndexInput.h"
#include "CLucene/document/Document.h"
#include "CLucene/document/FieldSelector.h"
#include "_FieldInfos.h"
#include "_FieldsWriter.h"
#include "_FieldsReader.h"
#include "CLucene/analysis/AnalysisHeader.h"
#include <sstream>

CL_NS_USE(store)
CL_NS_USE(document)
CL_NS_USE(util)
CL_NS_DEF(index)

FieldsReader::FieldsReader(Directory* d, const char* segment, FieldInfos* fn, int32_t _readBufferSize, int32_t _docStoreOffset, int32_t size):
	fieldInfos(fn), cloneableFieldsStream(NULL), fieldsStream(NULL), indexStream(NULL),
        numTotalDocs(0),_size(0), closed(false),docStoreOffset(0)
{
//Func - Constructor
//Pre  - d contains a valid reference to a Directory
//       segment != NULL
//       fn contains a valid reference to a FieldInfos
//Post - The instance has been created

	CND_PRECONDITION(segment != NULL, "segment != NULL");

	bool success = false;

	try {
		cloneableFieldsStream = d->openInput( Misc::segmentname(segment,".fdt").c_str(), _readBufferSize );
		fieldsStream = cloneableFieldsStream->clone();

		indexStream = d->openInput( Misc::segmentname(segment,".fdx").c_str(), _readBufferSize );

		if (_docStoreOffset != -1) {
			// We read only a slice out of this shared fields file
			this->docStoreOffset = _docStoreOffset;
			this->_size = size;

			// Verify the file is long enough to hold all of our
			// docs
			CND_CONDITION(((int32_t) (indexStream->length() / 8)) >= size + this->docStoreOffset,
				"the file is not long enough to hold all of our docs");
		} else {
			this->docStoreOffset = 0;
			this->_size = (int32_t) (indexStream->length() >> 3);
		}

		//_size = (int32_t)indexStream->length()/8;

		numTotalDocs = (int32_t) (indexStream->length() >> 3);
		success = true;
	} _CLFINALLY ({
		// With lock-less commits, it's entirely possible (and
		// fine) to hit a FileNotFound exception above. In
		// this case, we want to explicitly close any subset
		// of things that were opened so that we don't have to
		// wait for a GC to do so.
		if (!success) {
			close();
		}
	});
}

FieldsReader::~FieldsReader(){
//Func - Destructor
//Pre  - true
//Post - The instance has been destroyed

	close();
}

void FieldsReader::ensureOpen() {
	if (closed) {
		_CLTHROWA(CL_ERR_IllegalState, "this FieldsReader is closed");
	}
}

void FieldsReader::close() {
	if (!closed) {
		if (fieldsStream){
			fieldsStream->close();
			_CLDELETE(fieldsStream);
		}
		if (cloneableFieldsStream){
			cloneableFieldsStream->close();
			_CLDELETE(cloneableFieldsStream);
		}
		if(indexStream){
			indexStream->close();
			_CLDELETE(indexStream);
		}
		/*
		CL_NS(store)::IndexInput* localFieldsStream = fieldsStreamTL.get();
		if (localFieldsStream != NULL) {
			localFieldsStream->close();
			fieldsStreamTL->set(NULL);
		}*/
		closed = true;
	}
}

int32_t FieldsReader::size() const{
	return _size;
}

bool FieldsReader::doc(int32_t n, Document& doc, const CL_NS(document)::FieldSelector* fieldSelector) {
  if ( (n + docStoreOffset) * 8L > indexStream->length() )
      return false;
	indexStream->seek((n + docStoreOffset) * 8L);
	int64_t position = indexStream->readLong();
	fieldsStream->seek(position);

	int32_t numFields = fieldsStream->readVInt();
	for (int32_t i = 0; i < numFields; i++) {
		const int32_t fieldNumber = fieldsStream->readVInt();
		FieldInfo* fi = fieldInfos->fieldInfo(fieldNumber);
    if ( fi == NULL ) _CLTHROWA(CL_ERR_IO, "Field stream is invalid");

		FieldSelector::FieldSelectorResult acceptField = (fieldSelector == NULL) ?	FieldSelector::LOAD : fieldSelector->accept(fi->name);

		uint8_t bits = fieldsStream->readByte();
		CND_CONDITION(bits <= FieldsWriter::FIELD_IS_COMPRESSED + FieldsWriter::FIELD_IS_TOKENIZED + FieldsWriter::FIELD_IS_BINARY,
			"invalid field bits");

		const bool compressed = (bits & FieldsWriter::FIELD_IS_COMPRESSED) != 0;
		const bool tokenize = (bits & FieldsWriter::FIELD_IS_TOKENIZED) != 0;
		const bool binary = (bits & FieldsWriter::FIELD_IS_BINARY) != 0;

		//TODO: Find an alternative approach here if this list continues to grow beyond the
		//list of 5 or 6 currently here.  See Lucene 762 for discussion
		if (acceptField == FieldSelector::LOAD) {
			addField(doc, fi, binary, compressed, tokenize);
		}
		else if (acceptField == FieldSelector::LOAD_FOR_MERGE) {
			addFieldForMerge(doc, fi, binary, compressed, tokenize);
		}
		else if (acceptField == FieldSelector::LOAD_AND_BREAK){
			addField(doc, fi, binary, compressed, tokenize);
			break;//Get out of this loop
		}
		else if (acceptField == FieldSelector::LAZY_LOAD) {
			addFieldLazy(doc, fi, binary, compressed, tokenize);
		}
		else if (acceptField == FieldSelector::SIZE){
			skipField(binary, compressed, addFieldSize(doc, fi, binary, compressed));
		}
		else if (acceptField == FieldSelector::SIZE_AND_BREAK){
			addFieldSize(doc, fi, binary, compressed);
			break;
		}else {
			skipField(binary, compressed);
		}
	}
	return true;
}

CL_NS(store)::IndexInput* FieldsReader::rawDocs(int32_t* lengths, const int32_t startDocID, const int32_t numDocs) {
	indexStream->seek((docStoreOffset+startDocID) * 8L);
	int64_t startOffset = indexStream->readLong();
	int64_t lastOffset = startOffset;
	int32_t count = 0;
	while (count < numDocs) {
		int64_t offset;
		const int32_t docID = docStoreOffset + startDocID + count + 1;
		CND_CONDITION( docID <= numTotalDocs, "invalid docID");
		if (docID < numTotalDocs)
			offset = indexStream->readLong();
		else
			offset = fieldsStream->length();
		lengths[count++] = static_cast<int32_t>(offset-lastOffset);
		lastOffset = offset;
	}

	fieldsStream->seek(startOffset);

	return fieldsStream;
}

void FieldsReader::skipField(const bool binary, const bool compressed) {
	skipField(binary, compressed, fieldsStream->readVInt());
}

void FieldsReader::skipField(const bool binary, const bool compressed, const int32_t toRead) {
	if (binary || compressed) {
		int64_t pointer = fieldsStream->getFilePointer();
		fieldsStream->seek(pointer + toRead);
	} else {
		//We need to skip chars.  This will slow us down, but still better
		fieldsStream->skipChars(toRead);
	}
}

void FieldsReader::addFieldLazy(CL_NS(document)::Document& doc, const FieldInfo* fi, const bool binary,
								const bool compressed, const bool tokenize) {
	if (binary) {
		int32_t toRead = fieldsStream->readVInt();
		int64_t pointer = fieldsStream->getFilePointer();
		if (compressed) {
			doc.add(*_CLNEW LazyField(this, fi->name, Field::STORE_COMPRESS, toRead, pointer));
		} else {
			doc.add(*_CLNEW LazyField(this, fi->name, Field::STORE_YES, toRead, pointer));
		}
		//Need to move the pointer ahead by toRead positions
		fieldsStream->seek(pointer + toRead);
	} else {
		LazyField* f = NULL;
		if (compressed) {
			int32_t toRead = fieldsStream->readVInt();
			int64_t pointer = fieldsStream->getFilePointer();
			f = _CLNEW LazyField(this, fi->name, Field::STORE_COMPRESS, toRead, pointer);
			//skip over the part that we aren't loading
			fieldsStream->seek(pointer + toRead);
			f->setOmitNorms(fi->omitNorms);
		} else {
			int32_t length = fieldsStream->readVInt();
			int64_t pointer = fieldsStream->getFilePointer();
			//Skip ahead of where we are by the length of what is stored
			fieldsStream->skipChars(length);
			f = _CLNEW LazyField(this, fi->name, Field::STORE_YES | getIndexType(fi, tokenize) | getTermVectorType(fi), length, pointer);
			f->setOmitNorms(fi->omitNorms);
		}
		doc.add(*f);
	}
}

// in merge mode we don't uncompress the data of a compressed field
void FieldsReader::addFieldForMerge(CL_NS(document)::Document& doc, const FieldInfo* fi, const bool binary, const bool compressed, const bool tokenize) {
	void* data;
	Field::ValueType v;

	if ( binary || compressed) {
		int32_t toRead = fieldsStream->readVInt();
    CL_NS(util)::ValueArray<uint8_t> b(toRead);
    fieldsStream->readBytes(b.values,toRead);
		v = Field::VALUE_BINARY;
    data = b.takeArray();
	} else {
		data = fieldsStream->readString();
		v = Field::VALUE_STRING;
	}

	doc.add(*_CLNEW FieldForMerge(data, v, fi, binary, compressed, tokenize));
}

void FieldsReader::addField(CL_NS(document)::Document& doc, const FieldInfo* fi, const bool binary, const bool compressed, const bool tokenize) {

	//we have a binary stored field, and it may be compressed
	if (binary) {
		const int32_t toRead = fieldsStream->readVInt();
    ValueArray<uint8_t>* b = _CLNEW ValueArray<uint8_t>(toRead);
    fieldsStream->readBytes(b->values,toRead);
		if (compressed) {
			// we still do not support compressed fields
      ValueArray<uint8_t>* data = _CLNEW ValueArray<uint8_t>;
      try{
        uncompress(*b, *data);
      }catch(CLuceneError& err){
        _CLDELETE(data);
        _CLDELETE(b);
        throw err;
      }
      _CLDELETE(b);
			doc.add(* _CLNEW Field(fi->name, data, Field::STORE_COMPRESS, false));
    }else{
			doc.add(* _CLNEW Field(fi->name, b, Field::STORE_YES, false));
    }
    //no need to clean up, Field consumes b
	} else {
		uint8_t bits = 0;
		bits |= getIndexType(fi, tokenize);
		bits |= getTermVectorType(fi);

		Field* f = NULL;
		if (compressed) {
      bits |= Field::STORE_COMPRESS;
      const int32_t toRead = fieldsStream->readVInt();
      ValueArray<uint8_t>* b = _CLNEW ValueArray<uint8_t>(toRead);
      fieldsStream->readBytes(b->values,toRead);
      ValueArray<uint8_t> data;
      try{
        uncompress(*b, data);
      }_CLFINALLY( _CLDELETE(b) )

#ifndef _ASCII
      //convert to utf8
      TCHAR* result = _CL_NEWARRAY(TCHAR, data.length);
      size_t l = lucene_utf8towcs(result, (const char*)data.values, data.length);
      result[l] = 0;

      //if we were a bit too pesimistic with the size, then shrink the memory...
      if ( l < data.length/2 ){
        TCHAR* tmp = result;
        result = STRDUP_TtoT(result);
        _CLDELETE_LCARRAY(tmp);
      }

      f = _CLNEW Field(fi->name,      // field name
        result, // uncompress the value and add as string
        bits, false);
#else
      f = _CLNEW Field(fi->name,      // field name
        reinterpret_cast<char*>(data.values), // uncompress the value and add as string
        bits, false);
#endif
      f->setOmitNorms(fi->omitNorms);
		} else {
			bits |= Field::STORE_YES;
      TCHAR* str = fieldsStream->readString();
			f = _CLNEW Field(fi->name,     // name
				str, // read value
				bits, false);
			f->setOmitNorms(fi->omitNorms);
		}
		doc.add(*f);
	}
}

int32_t FieldsReader::addFieldSize(CL_NS(document)::Document& doc, const FieldInfo* fi, const bool binary, const bool compressed) {
	const int32_t size = fieldsStream->readVInt();
	const uint32_t bytesize = binary || compressed ? size : 2*size;
	ValueArray<uint8_t>* sizebytes = _CLNEW ValueArray<uint8_t>(4);
  sizebytes->values[0] = (uint8_t) (bytesize>>24);
	sizebytes->values[1] = (uint8_t) (bytesize>>16);
	sizebytes->values[2] = (uint8_t) (bytesize>> 8);
	sizebytes->values[3] = (uint8_t)  bytesize      ;
	doc.add(*_CLNEW Field(fi->name, sizebytes, Field::STORE_YES, false));
	return size;
}

CL_NS(document)::Field::TermVector FieldsReader::getTermVectorType(const FieldInfo* fi) {
	if (fi->storeTermVector) {
		if (fi->storeOffsetWithTermVector) {
			if (fi->storePositionWithTermVector) {
				return Field::TERMVECTOR_WITH_POSITIONS_OFFSETS;
			} else {
				return Field::TERMVECTOR_WITH_OFFSETS;
			}
		} else if (fi->storePositionWithTermVector) {
			return Field::TERMVECTOR_WITH_POSITIONS;
		} else {
			return Field::TERMVECTOR_YES;
		}
	} else {
		return Field::TERMVECTOR_NO ;
	}
}

CL_NS(document)::Field::Index FieldsReader::getIndexType(const FieldInfo* fi, const bool tokenize) {
	if (fi->isIndexed && tokenize)
		return Field::INDEX_TOKENIZED;
	else if (fi->isIndexed && !tokenize)
		return Field::INDEX_UNTOKENIZED;
	else
		return Field::INDEX_NO;
}


FieldsReader::LazyField::LazyField(FieldsReader* _parent, const TCHAR* _name,
								   int config, const int32_t _toRead, const int64_t _pointer)
: Field(_name, config), parent(_parent) {
	// todo: need to allow for auto setting Field::INDEX_NO | Field::TERMVECTOR_NO so only Store is required
	this->toRead = _toRead;
	this->pointer = _pointer;
	lazy = true;
}
FieldsReader::LazyField::~LazyField(){
}

CL_NS(store)::IndexInput* FieldsReader::LazyField::getFieldStream(){
	CL_NS(store)::IndexInput* localFieldsStream = parent->fieldsStreamTL.get();
	if (localFieldsStream == NULL) {
		localFieldsStream = parent->cloneableFieldsStream->clone();
		parent->fieldsStreamTL.set(localFieldsStream);
	}
	return localFieldsStream;
}

const ValueArray<uint8_t>* FieldsReader::LazyField::binaryValue(){
	parent->ensureOpen();
	if (fieldsData == NULL) {
		ValueArray<uint8_t>* b = _CLNEW ValueArray<uint8_t>(toRead);
		CL_NS(store)::IndexInput* localFieldsStream = getFieldStream();

		//Throw this IO Exception since IndexREader.document does so anyway, so probably not that big of a change for people
    //since they are already handling this exception when getting the document
    try {
      localFieldsStream->seek(pointer);
      localFieldsStream->readBytes(b->values, toRead);
      if (isCompressed() == true) {
        ValueArray<uint8_t>* data = _CLNEW ValueArray<uint8_t>;
        try{
          uncompress(*b, *data);
        }catch (CLuceneError& err){
          _CLDELETE(data);
          _CLDELETE(b);
          throw err;
        }
        _CLDELETE(b);
        fieldsData = data;
      } else {
        fieldsData = b;
      }
		  valueType = VALUE_BINARY;

    }catch(CLuceneError& err){
      if ( err.number() != CL_ERR_IO ) throw err;
      _CLTHROWA(CL_ERR_FieldReader, err.what());
    }

	}
	return static_cast<ValueArray<uint8_t>*>(fieldsData);
}

CL_NS(util)::Reader* FieldsReader::LazyField::readerValue(){
	parent->ensureOpen();
	return (valueType & VALUE_READER) ? static_cast<CL_NS(util)::Reader*>(fieldsData) : NULL;
}


CL_NS(analysis)::TokenStream* FieldsReader::LazyField::tokenStreamValue(){
	parent->ensureOpen();
	return (valueType & VALUE_TOKENSTREAM) ? static_cast<CL_NS(analysis)::TokenStream*>(fieldsData) : NULL;
}


/** The value of the field as a String, or null.  If null, the Reader value,
* binary value, or TokenStream value is used.  Exactly one of stringValue(),
* readerValue(), binaryValue(), and tokenStreamValue() must be set. */
const TCHAR* FieldsReader::LazyField::stringValue() {
	parent->ensureOpen();
	if (fieldsData == NULL) {
		CL_NS(store)::IndexInput* localFieldsStream = getFieldStream();
		localFieldsStream->seek(pointer);
		if (isCompressed()) {
      ValueArray<uint8_t> b(toRead);
      ValueArray<uint8_t> uncompressed;
			localFieldsStream->readBytes(b.values, toRead);
			_resetValue();
      uncompress(b, uncompressed); //no need to catch error, memory all in frame

#ifndef _ASCII
      TCHAR* str = _CL_NEWARRAY(TCHAR, uncompressed.length);
      size_t l = lucene_utf8towcs(str, (const char*)uncompressed.values, uncompressed.length);
      str[l] = 0;

      if ( l < uncompressed.length/2 ){
        //too pesimistic with size...
        fieldsData = STRDUP_TtoT(str);
        _CLDELETE_LCARRAY(str);
      }else{
        fieldsData = str;
      }
#else
      fieldsData = uncompressed.values;
#endif
        } else {
			//read in chars b/c we already know the length we need to read
			TCHAR* chars = _CL_NEWARRAY(TCHAR, toRead+1);
			localFieldsStream->readChars(chars, 0, toRead);
			chars[toRead] = _T('\0');
			_resetValue();
			fieldsData = chars;
		}
		valueType = VALUE_STRING;
	}
	return static_cast<const TCHAR*>(fieldsData); //instanceof String ? (String) fieldsData : null;
}

int64_t FieldsReader::LazyField::getPointer() const {
	parent->ensureOpen();
	return pointer;
}

void FieldsReader::LazyField::setPointer(const int64_t _pointer) {
	parent->ensureOpen();
	this->pointer = _pointer;
}

int32_t FieldsReader::LazyField::getToRead() const {
	parent->ensureOpen();
	return toRead;
}

void FieldsReader::LazyField::setToRead(const int32_t _toRead) {
	parent->ensureOpen();
	this->toRead = _toRead;
}

const TCHAR* FieldsReader::FieldForMerge::stringValue() const {
	return (valueType & VALUE_STRING) ? static_cast<TCHAR*>(fieldsData) : NULL;
}

CL_NS(util)::Reader* FieldsReader::FieldForMerge::readerValue() const {
	// not needed for merge
	return NULL;
}

const CL_NS(util)::ValueArray<uint8_t>* FieldsReader::FieldForMerge::binaryValue(){
	return (valueType & VALUE_BINARY) ? static_cast<CL_NS(util)::ValueArray<uint8_t>*>(fieldsData) : NULL;
}

CL_NS(analysis)::TokenStream* FieldsReader::FieldForMerge::tokenStreamValue() const {
	// not needed for merge
	return NULL;
}

FieldsReader::FieldForMerge::FieldForMerge(void* _value, ValueType _type, const FieldInfo* fi, const bool binary, const bool compressed, const bool tokenize) : Field(fi->name, 0) {

	uint32_t bits = STORE_YES;

	this->fieldsData = _value;
	this->valueType = _type;

	if (tokenize) bits |= INDEX_TOKENIZED;
	if (compressed) bits |= STORE_COMPRESS;

	if (fi->isIndexed && !tokenize) bits |= INDEX_UNTOKENIZED;
	if (fi->omitNorms) bits |= INDEX_NONORMS;
	if (fi->storeOffsetWithTermVector) bits |= TERMVECTOR_WITH_OFFSETS;
	if (fi->storePositionWithTermVector) bits |= TERMVECTOR_WITH_POSITIONS;
	if (fi->storeTermVector) bits |= TERMVECTOR_YES;

	setConfig(bits);
}
FieldsReader::FieldForMerge::~FieldForMerge(){
}
const char* FieldsReader::FieldForMerge::getClassName(){
  return "FieldsReader::FieldForMerge";
}
const char* FieldsReader::FieldForMerge::getObjectName() const{
  return getClassName();
}

void FieldsReader::uncompress(const CL_NS(util)::ValueArray<uint8_t>& input, CL_NS(util)::ValueArray<uint8_t>& output){
  stringstream out;
  string err;
  if ( ! Misc::inflate(input.values, input.length, out, err) ){
    _CLTHROWA(CL_ERR_IO, err.c_str());
  }

  // get length of file:
  out.seekg (0, ios::end);
  size_t length = out.tellg();
  out.seekg (0, ios::beg);

  output.resize(length+1);
  out.read((char*)output.values,length);
  output.values[length] = 0;//null-terminate in case we want to use it as utf8
}

CL_NS_END
