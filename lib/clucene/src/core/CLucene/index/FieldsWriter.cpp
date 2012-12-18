/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_FieldsWriter.h"

//#include "CLucene/util/VoidMap.h"
#include "CLucene/util/CLStreams.h"
#include "CLucene/util/Misc.h"
#include "CLucene/store/Directory.h"
#include "CLucene/store/_RAMDirectory.h"
#include "CLucene/store/IndexOutput.h"
#include "CLucene/document/Document.h"
#include "CLucene/document/Field.h"
#include "_FieldInfos.h"
#include "_FieldsReader.h"
#include <sstream>

CL_NS_USE(store)
CL_NS_USE(util)
CL_NS_USE(document)
CL_NS_DEF(index)

FieldsWriter::FieldsWriter(Directory* d, const char* segment, FieldInfos* fn):
	fieldInfos(fn)
{
//Func - Constructor
//Pre  - d contains a valid reference to a directory
//       segment != NULL and contains the name of the segment
//Post - fn contains a valid reference toa a FieldInfos

	CND_PRECONDITION(segment != NULL,"segment is NULL");

  fieldsStream = d->createOutput ( Misc::segmentname(segment,".fdt").c_str() );

	CND_CONDITION(fieldsStream != NULL,"fieldsStream is NULL");

  indexStream = d->createOutput( Misc::segmentname(segment,".fdx").c_str() );

	CND_CONDITION(indexStream != NULL,"indexStream is NULL");

	doClose = true;
}

FieldsWriter::FieldsWriter(CL_NS(store)::IndexOutput* fdx, CL_NS(store)::IndexOutput* fdt, FieldInfos* fn):
	fieldInfos(fn)
{
	fieldsStream = fdt;
	CND_CONDITION(fieldsStream != NULL,"fieldsStream is NULL");
	indexStream = fdx;
	CND_CONDITION(fieldsStream != NULL,"fieldsStream is NULL");
	doClose = false;
}

FieldsWriter::~FieldsWriter(){
//Func - Destructor
//Pre  - true
//Post - Instance has been destroyed

	close();
}

void FieldsWriter::close() {
//Func - Closes all streams and frees all resources
//Pre  - true
//Post - All streams have been closed all resources have been freed

	if (! doClose )
		return;

	//Check if fieldsStream is valid
	if (fieldsStream){
		//Close fieldsStream
		fieldsStream->close();
		_CLDELETE( fieldsStream );
	}

	//Check if indexStream is valid
	if (indexStream){
		//Close indexStream
		indexStream->close();
		_CLDELETE( indexStream );
	}
}

void FieldsWriter::addDocument(Document* doc) {
//Func - Adds a document
//Pre  - doc contains a valid reference to a Document
//       indexStream != NULL
//       fieldsStream != NULL
//Post - The document doc has been added

	CND_PRECONDITION(indexStream != NULL,"indexStream is NULL");
	CND_PRECONDITION(fieldsStream != NULL,"fieldsStream is NULL");

	indexStream->writeLong(fieldsStream->getFilePointer());

	int32_t storedCount = 0;
  {
    const Document::FieldsType& fields = *doc->getFields();
    for ( Document::FieldsType::const_iterator itr = fields.begin() ; itr != fields.end() ; itr++ ){
		  Field* field = *itr;
		  if (field->isStored())
			  storedCount++;
	  }
	  fieldsStream->writeVInt(storedCount);
  }
  {
	  const Document::FieldsType& fields = *doc->getFields();
    for ( Document::FieldsType::const_iterator itr = fields.begin() ; itr != fields.end() ; itr++ ){
		  Field* field = *itr;
		  if (field->isStored()) {
			  writeField(fieldInfos->fieldInfo(field->name()), field);
		  }
	  }
  }
}

void FieldsWriter::writeField(FieldInfo* fi, CL_NS(document)::Field* field)
{
	// if the field as an instanceof FieldsReader.FieldForMerge, we're in merge mode
	// and field.binaryValue() already returns the compressed value for a field
	// with isCompressed()==true, so we disable compression in that case
	bool disableCompression = (field->instanceOf(FieldsReader::FieldForMerge::getClassName()));

	fieldsStream->writeVInt(fi->number);
	uint8_t bits = 0;
	if (field->isTokenized())
		bits |= FieldsWriter::FIELD_IS_TOKENIZED;
	if (field->isBinary())
		bits |= FieldsWriter::FIELD_IS_BINARY;
	if (field->isCompressed())
		bits |= FieldsWriter::FIELD_IS_COMPRESSED;

	fieldsStream->writeByte(bits);

	if ( field->isCompressed() ){
    // compression is enabled for the current field
    CL_NS(util)::ValueArray<uint8_t> dataB;
    const CL_NS(util)::ValueArray<uint8_t>* data = &dataB;

    if (disableCompression) {
      // optimized case for merging, the data
      // is already compressed
      data = field->binaryValue();
    } else {
      // check if it is a binary field
      if (field->isBinary()) {
        compress(*field->binaryValue(), dataB);
      }else if ( field->stringValue() == NULL ){ //we must be using readerValue
        CND_PRECONDITION(!field->isIndexed(), "Cannot store reader if it is indexed too")
        Reader* r = field->readerValue();

        int32_t sz = r->size();
        if ( sz < 0 )
          sz = 10000000; //todo: we should warn the developer here....

        //read the entire string
        const TCHAR* rv = NULL;
        int64_t rl = r->read(rv, sz, 1);
        if ( rl > LUCENE_INT32_MAX_SHOULDBE )
          _CLTHROWA(CL_ERR_Runtime,"Field length too long");
        else if ( rl < 0 )
          rl = 0;

        string str = lucene_wcstoutf8string(rv, rl);
        CL_NS(util)::ValueArray<uint8_t> utfstr;
        utfstr.length = str.length();
        utfstr.values = (uint8_t*)str.c_str();
        compress(utfstr, dataB);
        utfstr.values = NULL;
      }else if ( field->stringValue() != NULL ){
        string str = lucene_wcstoutf8string(field->stringValue(), LUCENE_INT32_MAX_SHOULDBE);
        CL_NS(util)::ValueArray<uint8_t> utfstr;
        utfstr.length = str.length();
        utfstr.values = (uint8_t*)str.c_str();
        compress(utfstr, dataB);
        utfstr.values = NULL;
      }
    }
    fieldsStream->writeVInt(data->length);
    fieldsStream->writeBytes(data->values, data->length);

	}else{

		//FEATURE: this problem in Java Lucene too, if using Reader, data is not stored.
		//todo: this is a logic bug...
		//if the field is stored, and indexed, and is using a reader the field wont get indexed
		//
		//if we could write zero prefixed vints (therefore static length), then we could
		//write a reader directly to the field indexoutput and then go back and write the data
		//length. however this is not supported in lucene yet...
		//if this is ever implemented, then it would make sense to also be able to combine the
		//FieldsWriter and DocumentWriter::invertDocument process, and use a streamfilter to
		//write the field data while the documentwrite analyses the document! how cool would
		//that be! it would cut out all these buffers!!!

		// compression is disabled for the current field
		if (field->isBinary()) {
			const CL_NS(util)::ValueArray<uint8_t>* data = field->binaryValue();
      fieldsStream->writeVInt(data->length);
      fieldsStream->writeBytes(data->values, data->length);

		}else if ( field->stringValue() == NULL ){ //we must be using readerValue
			CND_PRECONDITION(!field->isIndexed(), "Cannot store reader if it is indexed too")
			Reader* r = field->readerValue();

			int32_t sz = r->size();
			if ( sz < 0 )
				sz = 10000000; //todo: we should warn the developer here....

			//read the entire string
			const TCHAR* rv;
			int64_t rl = r->read(rv, sz, 1);
			if ( rl > LUCENE_INT32_MAX_SHOULDBE )
				_CLTHROWA(CL_ERR_Runtime,"Field length too long");
			else if ( rl < 0 )
				rl = 0;

			fieldsStream->writeString( rv, (int32_t)rl);
		}else if ( field->stringValue() != NULL ){
			fieldsStream->writeString(field->stringValue(),_tcslen(field->stringValue()));
		}else
			_CLTHROWA(CL_ERR_Runtime, "No values are set for the field");
	}
}

void FieldsWriter::flushDocument(int32_t numStoredFields, CL_NS(store)::RAMOutputStream* buffer) {
	indexStream->writeLong(fieldsStream->getFilePointer());
	fieldsStream->writeVInt(numStoredFields);
	buffer->writeTo(fieldsStream);
}

void FieldsWriter::flush() {
  indexStream->flush();
  fieldsStream->flush();
}

void FieldsWriter::addRawDocuments(CL_NS(store)::IndexInput* stream, const int32_t* lengths, const int32_t numDocs) {
	int64_t position = fieldsStream->getFilePointer();
	const int64_t start = position;
	for(int32_t i=0;i<numDocs;i++) {
		indexStream->writeLong(position);
		position += lengths[i];
	}
	fieldsStream->copyBytes(stream, position-start);
	CND_CONDITION(fieldsStream->getFilePointer() == position,"fieldsStream->getFilePointer() != position");
}

void FieldsWriter::compress(const CL_NS(util)::ValueArray<uint8_t>& input, CL_NS(util)::ValueArray<uint8_t>& output){
  stringstream out;
  string err;
  if ( ! Misc::deflate(input.values, input.length, out, err) ){
    _CLTHROWA(CL_ERR_IO, err.c_str());
  }

  // get length of file:
  out.seekg (0, ios::end);
  size_t length = out.tellg();
  out.seekg (0, ios::beg);

  output.resize(length);
  out.read((char*)output.values,length);
}

CL_NS_END
