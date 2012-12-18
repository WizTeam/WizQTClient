/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "Term.h"
#include "_TermInfo.h"
#include "_SkipListWriter.h"
#include "_CompoundFile.h"
#include "CLucene/util/Misc.h"
#include "CLucene/store/IndexInput.h"
#include "CLucene/store/IndexOutput.h"

CL_NS_USE(store)
CL_NS_USE(util)
CL_NS_DEF(index)


class WriterFileEntry:LUCENE_BASE {
public:
	WriterFileEntry(){
		directoryOffset=0;
		dataOffset=0;
	}
	~WriterFileEntry(){
	}
	/** source file */
	char file[CL_MAX_PATH];

	/** temporary holder for the start of directory entry for this file */
	int64_t directoryOffset;

	/** temporary holder for the start of this file's data section */
	int64_t dataOffset;

};


/** Implementation of an IndexInput that reads from a portion of the
 *  compound file. The visibility is left as "package" *only* because
 *  this helps with testing since JUnit test cases in a different class
 *  can then access package fields of this class.
 */
class CSIndexInput:public CL_NS(store)::BufferedIndexInput {
private:
	CL_NS(store)::IndexInput* base;
	int64_t fileOffset;
	int64_t _length;
protected:
	/** Expert: implements buffer refill.  Reads uint8_ts from the current
	*  position in the input.
	* @param b the array to read uint8_ts into
	* @param length the number of uint8_ts to read
	*/
	void readInternal(uint8_t* /*b*/, const int32_t /*len*/);
	void seekInternal(const int64_t /*pos*/)
	{
	}

public:
	CSIndexInput(CL_NS(store)::IndexInput* base, const int64_t fileOffset, const int64_t length, const int32_t readBufferSize = CL_NS(store)::BufferedIndexInput::BUFFER_SIZE);
	CSIndexInput(const CSIndexInput& clone);
	~CSIndexInput();

	/** Closes the stream to futher operations. */
	void close();
	CL_NS(store)::IndexInput* clone() const;

	int64_t length() const { return _length; }

	const char* getDirectoryType() const{ return CompoundFileReader::getClassName(); }
  const char* getObjectName() const{ return getClassName(); }
  static const char* getClassName() { return "CSIndexInput"; }
};

class ReaderFileEntry:LUCENE_BASE {
public:
	int64_t offset;
	int64_t length;
	ReaderFileEntry(){
		offset=0;
		length=0;
	}
	~ReaderFileEntry(){
	}
};


CSIndexInput::CSIndexInput(CL_NS(store)::IndexInput* base, const int64_t fileOffset, const int64_t length, const int32_t _readBufferSize):BufferedIndexInput(_readBufferSize){
   this->base = base;
   this->fileOffset = fileOffset;
   this->_length = length;
}

void CSIndexInput::readInternal(uint8_t* b, const int32_t len)
{
   SCOPED_LOCK_MUTEX(base->THIS_LOCK)

   int64_t start = getFilePointer();
   if(start + len > _length)
      _CLTHROWA(CL_ERR_IO,"read past EOF");
   base->seek(fileOffset + start);
   base->readBytes(b, len, false);
}
CSIndexInput::~CSIndexInput(){
}
IndexInput* CSIndexInput::clone() const
{
	return _CLNEW CSIndexInput(*this);
}
CSIndexInput::CSIndexInput(const CSIndexInput& clone): BufferedIndexInput(clone){
   this->base = clone.base; //no need to clone this..
   this->fileOffset = clone.fileOffset;
   this->_length = clone._length;
}

void CSIndexInput::close(){
}



CompoundFileReader::CompoundFileReader(Directory* dir, const char* name, int32_t _readBufferSize):
	readBufferSize(_readBufferSize), directory(dir), stream(NULL), entries(_CLNEW EntriesType(true,true))
{
   fileName = STRDUP_AtoA(name);

   bool success = false;

   try {
      stream = dir->openInput(name, readBufferSize);

      // read the directory and init files
      int32_t count = stream->readVInt();
      ReaderFileEntry* entry = NULL;
      TCHAR tid[CL_MAX_PATH];
      for (int32_t i=0; i<count; i++) {
            int64_t offset = stream->readLong();
            stream->readString(tid,CL_MAX_PATH);
            char* aid = STRDUP_TtoA(tid);

            if (entry != NULL) {
               // set length of the previous entry
               entry->length = offset - entry->offset;
            }

            entry = _CLNEW ReaderFileEntry();
            entry->offset = offset;
            entries->put(aid, entry);
      }

      // set the length of the final entry
      if (entry != NULL) {
            entry->length = stream->length() - entry->offset;
      }

      success = true;

   }_CLFINALLY(
      if (! success && (stream != NULL)) {
            try {
               stream->close();
               _CLDELETE(stream);
			   } catch (CLuceneError& err){
                if ( err.number() != CL_ERR_IO )
                    throw err;
				   //else ignore
            }
      }
   )
}

CompoundFileReader::~CompoundFileReader(){
	close();
	_CLDELETE_CaARRAY(fileName);
	_CLDELETE(entries);
}

Directory* CompoundFileReader::getDirectory(){
   return directory;
}

const char* CompoundFileReader::getName() const{
   return fileName;
}
const char* CompoundFileReader::getClassName(){
  return "CompoundFileReader";
}
const char* CompoundFileReader::getObjectName() const{
  return getClassName();
}

void CompoundFileReader::close(){
  SCOPED_LOCK_MUTEX(THIS_LOCK)

  if (stream != NULL){
      entries->clear();
      stream->close();
      _CLDELETE(stream);
  }
}

bool CompoundFileReader::openInput(const char * id, CL_NS(store)::IndexInput *& ret, CLuceneError& error, int32_t bufferSize){
	SCOPED_LOCK_MUTEX(THIS_LOCK);

	if (stream == NULL){
		error.set(CL_ERR_IO,"Stream closed");
		return false;
	}

	const ReaderFileEntry* entry = entries->get((char*)id);
	if (entry == NULL){
		char buf[CL_MAX_PATH+26];
		cl_sprintf(buf, CL_MAX_PATH+26, "No sub-file with id %s found", id);
		error.set(CL_ERR_IO,buf);
		return false;
	}

	if (bufferSize < 1)
		bufferSize = readBufferSize;

	ret = _CLNEW CSIndexInput(stream, entry->offset, entry->length, bufferSize);
	return true;
}

bool CompoundFileReader::list(vector<string>* names) const{
  for ( EntriesType::const_iterator i=entries->begin();i!=entries->end();i++ ){
     names->push_back(i->first);
     ++i;
  }
  return true;
}

bool CompoundFileReader::fileExists(const char* name) const{
   return entries->exists((char*)name);
}

int64_t CompoundFileReader::fileModified(const char* name) const{
  return directory->fileModified(name);
}

void CompoundFileReader::touchFile(const char* name){
  directory->touchFile(name);
}

bool CompoundFileReader::doDeleteFile(const char* /*name*/){
   _CLTHROWA(CL_ERR_UnsupportedOperation,"UnsupportedOperationException: CompoundFileReader::doDeleteFile");
}

void CompoundFileReader::renameFile(const char* /*from*/, const char* /*to*/){
   _CLTHROWA(CL_ERR_UnsupportedOperation,"UnsupportedOperationException: CompoundFileReader::renameFile");
}

int64_t CompoundFileReader::fileLength(const char* name) const{
  ReaderFileEntry* e = entries->get((char*)name);
  if (e == NULL){
     char buf[CL_MAX_PATH + 30];
     strcpy(buf,"File ");
     strncat(buf,name,CL_MAX_PATH );
     strcat(buf," does not exist");
     _CLTHROWA(CL_ERR_IO,buf);
  }
  return e->length;
}
IndexOutput* CompoundFileReader::createOutput(const char* /*name*/){
   _CLTHROWA(CL_ERR_UnsupportedOperation,"UnsupportedOperationException: CompoundFileReader::createOutput");
}
LuceneLock* CompoundFileReader::makeLock(const char* /*name*/){
   _CLTHROWA(CL_ERR_UnsupportedOperation,"UnsupportedOperationException: CompoundFileReader::makeLock");
}

string CompoundFileReader::toString() const{
	return string("CompoundFileReader@") + fileName;
}



class CompoundFileWriter::Internal{
public:
	CL_NS(store)::Directory* directory;
	string fileName;

	CL_NS(util)::CLHashSet<char*,
		CL_NS(util)::Compare::Char,CL_NS(util)::Deletor::acArray> ids;

	typedef CL_NS(util)::CLLinkedList<WriterFileEntry*,
		CL_NS(util)::Deletor::Object<WriterFileEntry> > EntriesType;
	EntriesType* entries;

	bool merged;
  SegmentMerger::CheckAbort* checkAbort;

  Internal():
    ids(true),
    entries(_CLNEW EntriesType(true))
  {

  }
  ~Internal(){
	_CLDELETE(entries);
  }
};
CompoundFileWriter::CompoundFileWriter(Directory* dir, const char* name, SegmentMerger::CheckAbort* checkAbort){
  _internal = _CLNEW Internal;
  if (dir == NULL)
      _CLTHROWA(CL_ERR_NullPointer,"directory cannot be null");
  if (name == NULL)
      _CLTHROWA(CL_ERR_NullPointer,"name cannot be null");
  _internal->merged = false;
  _internal->checkAbort = checkAbort;
  _internal->directory = dir;
  _internal->fileName = name;
}

CompoundFileWriter::~CompoundFileWriter(){
  _CLDELETE(_internal);
}

Directory* CompoundFileWriter::getDirectory(){
  return _internal->directory;
}

/** Returns the name of the compound file. */
const char* CompoundFileWriter::getName() const{
  return _internal->fileName.c_str();
}

void CompoundFileWriter::addFile(const char* file){
  if (_internal->merged)
      _CLTHROWA(CL_ERR_IO,"Can't add extensions after merge has been called");

  if (file == NULL)
      _CLTHROWA(CL_ERR_NullPointer,"file cannot be null");

  if (_internal->ids.find((char*)file)!=_internal->ids.end()){
     char buf[CL_MAX_PATH + 30];
     strcpy(buf,"File ");
     strncat(buf,file,CL_MAX_PATH);
     strcat(buf," already added");
     _CLTHROWA(CL_ERR_IO,buf);
  }
  _internal->ids.insert(STRDUP_AtoA(file));

  WriterFileEntry* entry = _CLNEW WriterFileEntry();
  STRCPY_AtoA(entry->file,file,CL_MAX_PATH);
  _internal->entries->push_back(entry);
}

void CompoundFileWriter::close(){
  if (_internal->merged)
      _CLTHROWA(CL_ERR_IO,"Merge already performed");

  if (_internal->entries->size()==0) //isEmpty()
      _CLTHROWA(CL_ERR_IO,"No entries to merge have been defined");

  _internal->merged = true;

  // open the compound stream
  IndexOutput* os = NULL;
  try {
      os = _internal->directory->createOutput(_internal->fileName.c_str());

      // Write the number of entries
      os->writeVInt(_internal->entries->size());

      // Write the directory with all offsets at 0.
      // Remember the positions of directory entries so that we can
      // adjust the offsets later
      { //msvc6 for scope fix
		  for ( CLLinkedList<WriterFileEntry*>::iterator i=_internal->entries->begin();i!=_internal->entries->end();i++ ){
			  WriterFileEntry* fe = *i;
			  fe->directoryOffset = os->getFilePointer();
			  os->writeLong(0);    // for now
			  os->writeString(fe->file);
		  }
	  }

      // Open the files and copy their data into the stream.
      // Remember the locations of each file's data section.
      { //msvc6 for scope fix
      const int32_t bufferLength = 16384;
		  uint8_t buffer[bufferLength];
		  for ( CL_NS(util)::CLLinkedList<WriterFileEntry*>::iterator i=_internal->entries->begin();i!=_internal->entries->end();i++ ){
			  WriterFileEntry* fe = *i;
			  fe->dataOffset = os->getFilePointer();
			  copyFile(fe, os, buffer, bufferLength);
		  }
	  }

	  { //msvc6 for scope fix
		  // Write the data offsets into the directory of the compound stream
		  for ( CLLinkedList<WriterFileEntry*>::iterator i=_internal->entries->begin();
            i!=_internal->entries->end();i++ ){
			  WriterFileEntry* fe = *i;
			  os->seek(fe->directoryOffset);
			  os->writeLong(fe->dataOffset);
		  }
	  }


  } _CLFINALLY (
	  if (os != NULL) try { os->close(); _CLDELETE(os); } catch (...) { }
  );
}


void CompoundFileWriter::copyFile(WriterFileEntry* source, IndexOutput* os, uint8_t* buffer, int32_t bufferLength){
  IndexInput* is = NULL;
  try {
      int64_t startPtr = os->getFilePointer();

      is = _internal->directory->openInput(source->file);
      int64_t length = is->length();
      int64_t remainder = length;
      int32_t chunk = bufferLength;

      while(remainder > 0) {
          int32_t len = (int32_t)cl_min((int64_t)chunk, remainder);
          is->readBytes(buffer, len);
          os->writeBytes(buffer, len);
          remainder -= len;

          if (_internal->checkAbort != NULL)
            // Roughly every 2 MB we will check if
            // it's time to abort
            _internal->checkAbort->work(80);
      }

      // Verify that remainder is 0
      if (remainder != 0){
         TCHAR buf[CL_MAX_PATH+100];
         _sntprintf(buf,CL_MAX_PATH+100,_T("Non-zero remainder length after copying")
            _T(": %d (id: %s, length: %d, buffer size: %d)"),
            (int)remainder,source->file,(int)length,(int)chunk );
		    _CLTHROWT(CL_ERR_IO,buf);
      }

      // Verify that the output length diff is equal to original file
      int64_t endPtr = os->getFilePointer();
      int64_t diff = endPtr - startPtr;
      if (diff != length){
         TCHAR buf[100];
         _sntprintf(buf,100,_T("Difference in the output file offsets %d ")
            _T("does not match the original file length %d"),(int)diff,(int)length);
         _CLTHROWT(CL_ERR_IO,buf);
      }
  } _CLFINALLY (
     if (is != NULL){
        is->close();
        _CLDELETE(is);
     }
  );
}

CL_NS_END
