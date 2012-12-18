/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_TermVector.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/store/IndexInput.h"
#include "CLucene/store/IndexOutput.h"
#include "_IndexFileNames.h"

CL_NS_USE(util)
CL_NS_DEF(index)

TermVectorsReader::TermVectorsReader(CL_NS(store)::Directory* d, const char* segment, FieldInfos* fieldInfos,
									 int32_t readBufferSize, int32_t docStoreOffset, int32_t size):
	fieldInfos(NULL), tvx(NULL), tvd(NULL), tvf(NULL), _size(0), docStoreOffset(0)
	{

	bool success = false;

	char fbuf[CL_MAX_NAME];
	strcpy(fbuf,segment);
	strcat(fbuf,".");
	char* fpbuf=fbuf+strlen(fbuf);

	strcpy(fpbuf,IndexFileNames::VECTORS_INDEX_EXTENSION);
	try {
		if (d->fileExists(fbuf)) {
			tvx = d->openInput(fbuf, readBufferSize);
			checkValidFormat(tvx);

			strcpy(fpbuf,IndexFileNames::VECTORS_DOCUMENTS_EXTENSION);
			tvd = d->openInput(fbuf, readBufferSize);
			tvdFormat = checkValidFormat(tvd);

			strcpy(fpbuf,IndexFileNames::VECTORS_FIELDS_EXTENSION);
			tvf = d->openInput(fbuf, readBufferSize);
			tvfFormat = checkValidFormat(tvf);
			if (-1 == docStoreOffset) {
				this->docStoreOffset = 0;
				this->_size = static_cast<int64_t>(tvx->length() >> 3);
			} else {
				this->docStoreOffset = docStoreOffset;
				this->_size = size;
				// Verify the file is long enough to hold all of our
				// docs
				CND_CONDITION( ((int64_t) (tvx->length() / 8)) >= size + docStoreOffset , "file is not long enough to hold all of our docs");
			}
		}

		this->fieldInfos = fieldInfos;
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

TermVectorsReader::TermVectorsReader(const TermVectorsReader& copy)
{
    tvx = copy.tvx->clone();
    tvd = copy.tvd->clone();
    tvf = copy.tvf->clone();

    tvdFormat = copy.tvdFormat;
    tvfFormat = copy.tvfFormat;
    _size = copy._size;
    fieldInfos = copy.fieldInfos;
	docStoreOffset = copy.docStoreOffset;
}
TermVectorsReader* TermVectorsReader::clone() const{
	if (tvx == NULL || tvd == NULL || tvf == NULL)
		return NULL;
	return _CLNEW TermVectorsReader(*this);
}

TermVectorsReader::~TermVectorsReader(){
	close();
}

int32_t TermVectorsReader::checkValidFormat(CL_NS(store)::IndexInput* in){
	int32_t format = in->readInt();
	if (format > TermVectorsReader::FORMAT_VERSION)
	{
		CL_NS(util)::StringBuffer err;
		err.append(_T("Incompatible format version: "));
		err.appendInt(format);
		err.append(_T(" expected "));
		err.appendInt(TermVectorsReader::FORMAT_VERSION);
		err.append(_T(" or less"));
		_CLTHROWT(CL_ERR_CorruptIndex,err.getBuffer());
	}
	return format;
}

void TermVectorsReader::close(){
	// make all effort to close up. Keep the first exception
  	// and throw it as a new one.
	// todo: why don't we trap the exception and at least make sure that
    // all streams that we can close are closed?
	CLuceneError keep;
	bool thrown = false;

	if (tvx != NULL){
		try{tvx->close();}
		catch(CLuceneError& err){
			if ( err.number() == CL_ERR_IO ){
				keep = err;
				thrown = true;
			}else
				throw err;
		}
		_CLDELETE(tvx);//delete even  if error thrown
	}
    if (tvd != NULL){
		try{tvd->close();}
		catch(CLuceneError& err){
			if ( err.number() == CL_ERR_IO ){
				keep = err;
				thrown = true;
			}else
				throw err;
		}
		_CLDELETE(tvd);
	}
    if (tvf != NULL){
		try{tvf->close();}
		catch(CLuceneError& err){
			if ( err.number() == CL_ERR_IO ){
				keep = err;
				thrown = true;
			}else
				throw err;
		}
		_CLDELETE(tvf);
	}

	if ( thrown )
		throw keep;
}

int64_t TermVectorsReader::size() const{
    return _size;
}

void TermVectorsReader::get(const int32_t docNum, const TCHAR* field, TermVectorMapper* mapper){
	if (tvx != NULL) {
		int32_t fieldNumber = fieldInfos->fieldNumber(field);
		//We need to account for the FORMAT_SIZE at when seeking in the tvx
		//We don't need to do this in other seeks because we already have the
		// file pointer
		//that was written in another file
        tvx->seek(((docNum + docStoreOffset) * 8L) + FORMAT_SIZE);
        int64_t position = tvx->readLong();

        tvd->seek(position);
        int32_t fieldCount = tvd->readVInt();
        // There are only a few fields per document. We opt for a full scan
        // rather then requiring that they be ordered. We need to read through
        // all of the fields anyway to get to the tvf pointers.
        int32_t number = 0;
        int32_t found = -1;
        for (int32_t i = 0; i < fieldCount; ++i) {
			if(tvdFormat == FORMAT_VERSION)
				number = tvd->readVInt();
			else
				number += tvd->readVInt();

          if (number == fieldNumber)
			  found = i;
        }

		// This field, although valid in the segment, was not found in this
		// document
		if (found != -1) {
          // Compute position in the tvf file
          position = 0;
          for (int32_t i = 0; i <= found; i++) // TODO: Was ++i, make sure its still good
            position += tvd->readVLong();

		  mapper->setDocumentNumber(docNum);
		  readTermVector(field, position, mapper);
      } else {
        // "Field not found"
      }
    } else {
      // "No tvx file"
	}
}

TermFreqVector* TermVectorsReader::get(const int32_t docNum, const TCHAR* field){
	// Check if no term vectors are available for this segment at all
	ParallelArrayTermVectorMapper* mapper = _CLNEW ParallelArrayTermVectorMapper();
	get(docNum, field, (TermVectorMapper*)mapper);

	TermFreqVector* ret = mapper->materializeVector();
	_CLLDELETE(mapper);
	return ret;
}


ArrayBase<TermFreqVector*>* TermVectorsReader::get(const int32_t docNum){
	ObjectArray<TermFreqVector>* result = NULL;
    // Check if no term vectors are available for this segment at all
    if (tvx != NULL) {
        //We need to offset by
		tvx->seek(((docNum + docStoreOffset) * 8L) + FORMAT_SIZE);
        int64_t position = tvx->readLong();

        tvd->seek(position);
        int32_t fieldCount = tvd->readVInt();

        // No fields are vectorized for this document
        if (fieldCount != 0) {
            int32_t number = 0;
            const TCHAR** fields = _CL_NEWARRAY(const TCHAR*,fieldCount+1);

			{ //msvc6 scope fix
				for (int32_t i = 0; i < fieldCount; ++i) {
					if(tvdFormat == FORMAT_VERSION)
						number = tvd->readVInt();
					else
						number += tvd->readVInt();
				    fields[i] = fieldInfos->fieldName(number);
				}
			}
			fields[fieldCount]=NULL;

		    // Compute position in the tvf file
		    position = 0;
		    int64_t* tvfPointers = _CL_NEWARRAY(int64_t,fieldCount);
			{ //msvc6 scope fix
				for (int32_t i = 0; i < fieldCount; ++i) {
				    position += tvd->readVLong();
				    tvfPointers[i] = position;
				}
			}

			result = (ObjectArray<TermFreqVector>*)readTermVectors(docNum, fields, tvfPointers, fieldCount);

            _CLDELETE_ARRAY(tvfPointers);
            _CLDELETE_ARRAY(fields);
        }
    } else {
			// "No tvx file"
	}
	return result;
}

void TermVectorsReader::get(const int32_t docNumber, TermVectorMapper* mapper) {
    // Check if no term vectors are available for this segment at all
    if (tvx != NULL) {
      //We need to offset by
      tvx->seek((docNumber * 8L) + FORMAT_SIZE);
      int64_t position = tvx->readLong();

      tvd->seek(position);
      int32_t fieldCount = tvd->readVInt();

      // No fields are vectorized for this document
      if (fieldCount != 0) {
        int32_t number = 0;
        const TCHAR** fields = _CL_NEWARRAY(const TCHAR*, fieldCount+1);

		{ //msvc6 scope fix
			for (int32_t i = 0; i < fieldCount; i++) {
				if(tvdFormat == FORMAT_VERSION)
					number = tvd->readVInt();
				else
					number += tvd->readVInt();

				fields[i] = fieldInfos->fieldName(number);
			}
		}
		fields[fieldCount]=NULL;

		// Compute position in the tvf file
		position = 0;
		int64_t* tvfPointers = _CL_NEWARRAY(int64_t,fieldCount);
		{ //msvc6 scope fix
			for (int32_t i = 0; i < fieldCount; i++) {
				position += tvd->readVLong();
				tvfPointers[i] = position;
			}
		}

        mapper->setDocumentNumber(docNumber);
        readTermVectors(fields, tvfPointers, fieldCount, mapper);

		_CLDELETE_ARRAY(tvfPointers);
		_CLDELETE_ARRAY(fields);
      }
    } else {
      // "No tvx file"
    }
  }

ObjectArray<SegmentTermVector>* TermVectorsReader::readTermVectors(const int32_t docNum,
										const TCHAR** fields, const int64_t* tvfPointers, const int32_t len){
	ObjectArray<SegmentTermVector>* res = _CLNEW CL_NS(util)::ObjectArray<SegmentTermVector>(len);
	ParallelArrayTermVectorMapper* mapper = _CLNEW ParallelArrayTermVectorMapper();
	for (int32_t i = 0; i < len; i++) {
		mapper->setDocumentNumber(docNum);
		readTermVector(fields[i], tvfPointers[i], mapper);
		res->values[i] = static_cast<SegmentTermVector*>(mapper->materializeVector());
		mapper->reset();
	}
	_CLLDELETE(mapper);
	return res;
}

void TermVectorsReader::readTermVectors(const TCHAR** fields, const int64_t* tvfPointers,
										const int32_t len, TermVectorMapper* mapper){
	for (int32_t i = 0; i < len; i++) {
		readTermVector(fields[i], tvfPointers[i], mapper);
	}
}

void TermVectorsReader::readTermVector(const TCHAR* field, const int64_t tvfPointer, TermVectorMapper* mapper){
	//Now read the data from specified position
    //We don't need to offset by the FORMAT here since the pointer already includes the offset
    tvf->seek(tvfPointer);

    int32_t numTerms = tvf->readVInt();
    // If no terms - return a constant empty termvector. However, this should never occur!
    if (numTerms == 0)
		return;

	bool storePositions;
    bool storeOffsets;

	if(tvfFormat == FORMAT_VERSION){
		uint8_t bits = tvf->readByte();
		storePositions = (bits & STORE_POSITIONS_WITH_TERMVECTOR) != 0;
		storeOffsets = (bits & STORE_OFFSET_WITH_TERMVECTOR) != 0;
	}
	else{
		tvf->readVInt();
		storePositions = false;
		storeOffsets = false;
	}
	mapper->setExpectations(field, numTerms, storeOffsets, storePositions);

  int32_t start = 0;
  int32_t deltaLength = 0;
  int32_t totalLength = 0;
  ValueArray<TCHAR> buffer(10); // init the buffer with a length of 10 character

  for (int32_t i = 0; i < numTerms; ++i) {
		start = tvf->readVInt();
		deltaLength = tvf->readVInt();
		totalLength = start + deltaLength;
    if (buffer.length < totalLength + 1) // increase buffer
		{
      buffer.resize(totalLength+1);
		}

		//read the term
    tvf->readChars(buffer.values, start, deltaLength);
    buffer.values[totalLength] = '\0'; //null terminate term

		//read the frequency
		int32_t freq = tvf->readVInt();
		ValueArray<int32_t>* positions = NULL;

		if (storePositions) { //read in the positions
			//does the mapper even care about positions?
			if (mapper->isIgnoringPositions() == false) {
				positions = _CLNEW ValueArray<int32_t>(freq);
				int32_t prevPosition = 0;
				for (int32_t j = 0; j < freq; j++)
				{
					positions->values[j] = prevPosition + tvf->readVInt();
					prevPosition = positions->values[j];
				}
			} else {
				//we need to skip over the positions.  Since these are VInts, I don't believe there is anyway to know for sure how far to skip
				//
				for (int32_t j = 0; j < freq; j++)
				{
					tvf->readVInt();
				}
			}
		}

		ArrayBase<TermVectorOffsetInfo*>* offsets = NULL;
		if (storeOffsets) {
			//does the mapper even care about offsets?
			if (mapper->isIgnoringOffsets() == false) {
				offsets = _CLNEW ObjectArray<TermVectorOffsetInfo>(freq);
				int32_t prevOffset = 0;
				for (int32_t j = 0; j < freq; j++) {
					int32_t startOffset = prevOffset + tvf->readVInt();
					int32_t endOffset = startOffset + tvf->readVInt();
					offsets->values[j] = _CLNEW TermVectorOffsetInfo(startOffset, endOffset);
					prevOffset = endOffset;
				}
			} else {
				for (int32_t j = 0; j < freq; j++){
					tvf->readVInt();
					tvf->readVInt();
				}
			}
		}
    mapper->map(buffer.values, totalLength, freq, offsets, positions);
	}
}

ObjectArray<TermVectorOffsetInfo>* TermVectorOffsetInfo_EMPTY_OFFSET_INFO = _CLNEW ObjectArray<TermVectorOffsetInfo>;

TermVectorOffsetInfo::TermVectorOffsetInfo() {
	startOffset = 0;
	endOffset=0;
}
TermVectorOffsetInfo::~TermVectorOffsetInfo() {
}

TermVectorOffsetInfo::TermVectorOffsetInfo(int32_t startOffset, int32_t endOffset) {
	this->endOffset = endOffset;
	this->startOffset = startOffset;
}

int32_t TermVectorOffsetInfo::getEndOffset() const{
	return endOffset;
}

void TermVectorOffsetInfo::setEndOffset(const int32_t _endOffset) {
	this->endOffset = _endOffset;
}

int32_t TermVectorOffsetInfo::getStartOffset() const{
	return startOffset;
}

void TermVectorOffsetInfo::setStartOffset(const int32_t _startOffset) {
	this->startOffset = _startOffset;
}

bool TermVectorOffsetInfo::equals(TermVectorOffsetInfo* termVectorOffsetInfo) {
	if (this == termVectorOffsetInfo)
		return true;

	if (endOffset != termVectorOffsetInfo->endOffset) return false;
	if (startOffset != termVectorOffsetInfo->startOffset) return false;

	return true;
}

size_t TermVectorOffsetInfo::hashCode() const{
	size_t result;
	result = startOffset;
	result = 29 * result + endOffset;
	return result;
}

TermVectorMapper::TermVectorMapper(){
	this->ignoringPositions = false;
	this->ignoringOffsets = false;
}

TermVectorMapper::TermVectorMapper(const bool _ignoringPositions, const bool _ignoringOffsets){
	this->ignoringPositions = _ignoringPositions;
	this->ignoringOffsets = _ignoringOffsets;
}

bool TermVectorMapper::isIgnoringPositions() const
{
	return ignoringPositions;
}

bool TermVectorMapper::isIgnoringOffsets() const
{
	return ignoringOffsets;
}

void TermVectorMapper::setDocumentNumber(const int32_t /*documentNumber*/)
{
    //default implementation does nothing...
}

ParallelArrayTermVectorMapper::ParallelArrayTermVectorMapper():
  terms(NULL),
  termFreqs(NULL),
  positions(NULL),
  offsets(NULL),
  currentPosition(0),
  field(NULL)
{
}
ParallelArrayTermVectorMapper::~ParallelArrayTermVectorMapper(){
	_CLDELETE_LCARRAY(field);
}

void ParallelArrayTermVectorMapper::setExpectations(const TCHAR* _field, const int32_t numTerms,
													const bool storeOffsets, const bool storePositions) {
	_CLDELETE_LCARRAY(field);
	this->field = STRDUP_TtoT(_field);

  terms = _CLNEW CL_NS(util)::TCharArray(numTerms);
	termFreqs = _CLNEW ValueArray<int32_t>(numTerms);

	this->storingOffsets = storeOffsets;
	this->storingPositions = storePositions;
	if(storePositions){
		positions = (ArrayBase< ArrayBase<int32_t>* >*)_CLNEW ObjectArray< ValueArray<int32_t> >(numTerms);
	}
	if(storeOffsets){
		offsets = _CLNEW ObjectArray< ArrayBase<TermVectorOffsetInfo*> >(numTerms);
	}
}

void ParallelArrayTermVectorMapper::map(const TCHAR* term, int32_t termLen, const int32_t frequency,
    ArrayBase<TermVectorOffsetInfo*>* _offsets,
    ArrayBase<int32_t>* _positions) {
	terms->values[currentPosition] = STRDUP_TtoT(term);

	termFreqs->values[currentPosition] = frequency;

	if (storingOffsets)
	{
		this->offsets->values[currentPosition] = _offsets;
	}
	if (storingPositions)
	{
		this->positions->values[currentPosition] = _positions;
	}
	currentPosition++;
}

TermFreqVector* ParallelArrayTermVectorMapper::materializeVector() {
	SegmentTermVector* tv = NULL;
	if (field != NULL && terms != NULL) {
		if (storingPositions || storingOffsets) {
			tv = _CLNEW SegmentTermPositionVector(field, terms, termFreqs, positions, offsets);
		} else {
			tv = _CLNEW SegmentTermVector(field, terms, termFreqs);
		}
	}
	return tv;
}

CL_NS_END
