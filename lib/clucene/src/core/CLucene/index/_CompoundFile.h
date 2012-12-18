/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_compoundfile_h
#define _lucene_index_compoundfile_h


CL_CLASS_DEF(store,Lock)
#include "CLucene/store/Directory.h"
#include "CLucene/store/IndexInput.h"
#include "_SegmentMerger.h"

CL_NS_DEF(index)

class WriterFileEntry;
class ReaderFileEntry;

/**
 * Class for accessing a compound stream.
 * This class implements a directory, but is limited to only read operations.
 * Directory methods that would normally modify data throw an exception.
 *
 */
class CompoundFileReader: public CL_NS(store)::Directory {
private:
    int32_t readBufferSize;

	// Base info
	CL_NS(store)::Directory* directory;
	char* fileName;

	CL_NS(store)::IndexInput* stream;

    typedef CL_NS(util)::CLHashMap<char*,ReaderFileEntry*,
		CL_NS(util)::Compare::Char,
		CL_NS(util)::Equals::Char,
		CL_NS(util)::Deletor::acArray,
		CL_NS(util)::Deletor::Object<ReaderFileEntry> > EntriesType;
	EntriesType* entries;
protected:
	/** Removes an existing file in the directory-> */
	bool doDeleteFile(const char* name);

public:
	CompoundFileReader(CL_NS(store)::Directory* dir, const char* name, int32_t _readBufferSize=CL_NS(store)::BufferedIndexInput::BUFFER_SIZE);
	virtual ~CompoundFileReader();
	CL_NS(store)::Directory* getDirectory();
	const char* getName() const;

	void close();
	bool openInput(const char * name, CL_NS(store)::IndexInput *& ret, CLuceneError& error, int32_t bufferSize=0);

	/** Returns an array of strings, one for each file in the directory-> */
	bool list(std::vector<std::string>* names) const;
	/** Returns true iff a file with the given name exists. */
	bool fileExists(const char* name) const;
	/** Returns the time the named file was last modified. */
	int64_t fileModified(const char* name) const;
	/** Set the modified time of an existing file to now. */
	void touchFile(const char* name);
	/** Renames an existing file in the directory->
	If a file already exists with the new name, then it is replaced.
	This replacement should be atomic. */
	void renameFile(const char* from, const char* to);
	/** Returns the length of a file in the directory.
     * @throws IOException if the file does not exist */
    int64_t fileLength(const char* name) const;
	/** Not implemented
     * @throws UnsupportedOperationException */
    CL_NS(store)::IndexOutput* createOutput(const char* name);
	/** Not implemented
     * @throws UnsupportedOperationException */
	CL_NS(store)::LuceneLock* makeLock(const char* name);

	std::string toString() const;

	static const char* getClassName();
	const char* getObjectName() const;
};


/**
 * Combines multiple files into a single compound file.
 * The file format:<br>
 * <ul>
 *     <li>VInt fileCount</li>
 *     <li>{Directory}
 *         fileCount entries with the following structure:</li>
 *         <ul>
 *             <li>int64_t dataOffset</li>
 *             <li>UTFString extension</li>
 *         </ul>
 *     <li>{File Data}
 *         fileCount entries with the raw data of the corresponding file</li>
 * </ul>
 *
 * The fileCount integer indicates how many files are contained in this compound
 * file. The {directory} that follows has that many entries. Each directory entry
 * contains an encoding identifier, an int64_t pointer to the start of this file's
 * data section, and a UTF String with that file's extension.
 *
 */
class CompoundFileWriter:LUCENE_BASE {
  class Internal;
  Internal* _internal;

	/** Copy the contents of the file with specified extension into the
	*  provided output stream. Use the provided buffer for moving data
	*  to reduce memory allocation.
	*/
	void copyFile(WriterFileEntry* source, CL_NS(store)::IndexOutput* os, uint8_t* buffer, int32_t bufferLength);
public:
	/** Create the compound stream in the specified file. The file name is the
	*  entire name (no extensions are added).
	*/
	CompoundFileWriter(CL_NS(store)::Directory* dir, const char* name, SegmentMerger::CheckAbort* checkAbort = NULL);
	~CompoundFileWriter();
	/** Returns the directory of the compound file. */
	CL_NS(store)::Directory* getDirectory();
	const char* getName() const ;
	/** Add a source stream. <code>file</code> is the string by which the
	*  sub-stream will be known in the compound stream.
	*
	*  @throws IllegalStateException if this writer is closed
	*  @throws NullPointerException if <code>file</code> is null
	*  @throws IllegalArgumentException if a file with the same name
	*   has been added already
	*/
	void addFile(const char* file);
	/** Merge files with the extensions added up to now.
	*  All files with these extensions are combined sequentially into the
	*  compound stream. After successful merge, the source files
    *  @throws IllegalStateException if close() had been called before or
    *   if no file has been added to this object
	*  are deleted.
	*/
	void close();
};

CL_NS_END
#endif
