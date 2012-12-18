/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_store_Directory
#define _lucene_store_Directory

#include "CLucene/util/Equators.h"
#include "CLucene/LuceneThreads.h"
#include <string>
#include <vector>

CL_CLASS_DEF(store,Lock)
CL_CLASS_DEF(store,IndexInput)
CL_CLASS_DEF(store,IndexOutput)
CL_CLASS_DEF(store,LockFactory)
CL_CLASS_DEF(store,LuceneLock)

CL_NS_DEF(store)

   /** A Directory is a flat list of files.  Files may be written once, when they
   * are created.  Once a file is created it may only be opened for read, or
   * deleted.  Random access is permitted both when reading and writing.
   *
   * <p> Direct i/o is not used directly, but rather all i/o is
   * through this API.  This permits things such as: <ul>
   * <li> implementation of RAM-based indices;
   * <li> implementation indices stored in a database, via a database;
   * <li> implementation of an index as a single file;
   * </ul>
   *
   */
class CLUCENE_EXPORT Directory: LUCENE_REFBASE, public CL_NS(util)::NamedObject {
	protected:
		LockFactory* lockFactory;

		Directory();
		// Removes an existing file in the directory.
		virtual bool doDeleteFile(const char* name) = 0;
	public:
		DEFINE_MUTEX(THIS_LOCK)

		virtual ~Directory();

		// Returns an null terminated array of strings, one for each file in the directory.
		char** list() const;
		virtual bool list(std::vector<std::string>* names) const = 0; //todo: deprecate this...
		bool list(std::vector<std::string>& names) const;

		// Returns true iff a file with the given name exists.
		virtual bool fileExists(const char* name) const = 0;

		// Returns the time the named file was last modified.
		virtual int64_t fileModified(const char* name) const = 0;

		// Returns the length of a file in the directory.
		virtual int64_t fileLength(const char* name) const = 0;

		// An advanced overload to avoid throwing an error. if result is false, error is filled with the reason
		virtual bool openInput(const char* name, IndexInput*& ret, CLuceneError& error, int32_t bufferSize = -1) = 0;

		// Returns a stream reading an existing file.
		IndexInput* openInput(const char* name, int32_t bufferSize=-1);

		/// Set the modified time of an existing file to now. */
		virtual void touchFile(const char* name) = 0;

		// Removes an existing file in the directory.
		virtual bool deleteFile(const char* name, const bool throwError=true);

		// Renames an existing file in the directory.
		//	If a file already exists with the new name, then it is replaced.
		//	This replacement should be atomic.
		virtual void renameFile(const char* from, const char* to) = 0;

		// Creates a new, empty file in the directory with the given name.
		//	Returns a stream writing this file.
		virtual IndexOutput* createOutput(const char* name) = 0;

		// Construct a {@link Lock}.
		// @param name the name of the lock file
		virtual LuceneLock* makeLock(const char* name);

		virtual void clearLock(const char* name);

		// Closes the store.
		virtual void close() = 0;

    virtual std::string toString() const = 0;

		void setLockFactory( LockFactory* lockFactory );

		LockFactory* getLockFactory();

    virtual std::string getLockID();
	};
CL_NS_END
#endif
