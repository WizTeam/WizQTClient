/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_store_RAMDirectory_
#define _lucene_store_RAMDirectory_


#include "CLucene/util/VoidMap.h"
#include "Directory.h"
CL_CLASS_DEF(store,RAMFile)

CL_NS_DEF(store)

	/**
	* A memory-resident {@link Directory} implementation.  Locking
	* implementation is by default the {@link SingleInstanceLockFactory}
	* but can be changed with {@link #setLockFactory}.
	*
	*/
	class CLUCENE_EXPORT RAMDirectory:public Directory{
	protected:
		typedef CL_NS(util)::CLHashMap<char*,RAMFile*,
				CL_NS(util)::Compare::Char, CL_NS(util)::Equals::Char,
				CL_NS(util)::Deletor::acArray , CL_NS(util)::Deletor::Object<RAMFile> > FileMap;
		/// Removes an existing file in the directory.
		virtual bool doDeleteFile(const char* name);

		/**
		* Creates a new <code>RAMDirectory</code> instance from a different
		* <code>Directory</code> implementation.  This can be used to load
		* a disk-based index into memory.
		* <P>
		* This should be used only with indices that can fit into memory.
		*
		* @param dir a <code>Directory</code> value
		* @exception IOException if an error occurs
		*/
		void _copyFromDir(Directory* dir, bool closeDir);
		FileMap* files; // unlike the java Hashtable, FileMap is not synchronized, and all access must be protected by a lock
	public:
		int64_t sizeInBytes; //todo

	  DEFINE_MUTABLE_MUTEX(files_mutex) // mutable: const methods must also be able to synchronize properly

		/// Returns a null terminated array of strings, one for each file in the directory.
		bool list(std::vector<std::string>* names) const;

      /** Constructs an empty {@link Directory}. */
		RAMDirectory();

	  ///Destructor - only call this if you are sure the directory
	  ///is not being used anymore. Otherwise use the ref-counting
	  ///facilities of dir->close
		virtual ~RAMDirectory();

		RAMDirectory(Directory* dir);

	  /**
	   * Creates a new <code>RAMDirectory</code> instance from the {@link FSDirectory}.
	   *
	   * @param dir a <code>String</code> specifying the full index directory path
	   */
		RAMDirectory(const char* dir);

		/// Returns true iff the named file exists in this directory.
		bool fileExists(const char* name) const;

		/// Returns the time the named file was last modified.
		int64_t fileModified(const char* name) const;

		/// Returns the length in bytes of a file in the directory.
		int64_t fileLength(const char* name) const;

		/// Removes an existing file in the directory.
		virtual void renameFile(const char* from, const char* to);

		/** Set the modified time of an existing file to now. */
		void touchFile(const char* name);

		/// Creates a new, empty file in the directory with the given name.
		///	Returns a stream writing this file.
		virtual IndexOutput* createOutput(const char* name);

		/// Returns a stream reading an existing file.
		bool openInput(const char* name, IndexInput*& ret, CLuceneError& error, int32_t bufferSize = -1);

		virtual void close();

    std::string toString() const;

		static const char* getClassName();
		const char* getObjectName() const;
	};
CL_NS_END
#endif
