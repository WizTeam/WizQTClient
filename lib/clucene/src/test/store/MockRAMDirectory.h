/*------------------------------------------------------------------------------
* Copyright (C) 2003-2010 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#ifndef _lucene_store_MockRAMDirectory_
#define _lucene_store_MockRAMDirectory_

#include "CLucene.h"
#include "CLucene/_clucene-config.h"
#include <string>
#include <map>
#include <vector>

#include "CLucene/store/RAMDirectory.h"
#include "CLucene/store/_RAMDirectory.h"

CL_NS_DEF(store)

/**
 * This is a subclass of RAMDirectory that adds methods
 * intented to be used only by unit tests.
 */ 
class MockRAMDirectory : public RAMDirectory {
public:

	/**
	 * Objects that represent fail-able conditions. Objects of a derived
	 * class are created and registered with the mock directory. After
	 * register, each object will be invoked once for each first write
 	 * of a file, giving the object a chance to throw an IOException.
	 */ 
	class Failure {
	public:
		Failure() { doFail = false; }

		void eval(MockRAMDirectory* dir) {}
		/**
		 * reset should set the state of the failure to its default
		 * (freshly constructed) state. Reset is convenient for tests
		 * that want to create one failure object and then reuse it in
		 * multiple cases. This, combined with the fact that Failure
		 * subclasses are often anonymous classes makes reset difficult to
		 * do otherwise.
		 *
		 * A typical example of use is
		 * Failure* failure = new Failure();
		 * ...
		 * mock.failOn(failure->reset())
		 */ 
		Failure* reset() { return this; }
	void setDoFail() { doFail = true; };
		void clearDoFail() { doFail = false; }

	private:
		bool doFail;
	};

	MockRAMDirectory();
	MockRAMDirectory(const char* dir);
	MockRAMDirectory(Directory* dir);
	virtual ~MockRAMDirectory();
	virtual IndexOutput* createOutput(const char* name);
	virtual bool openInput(const char* name, IndexInput*& ret, CLuceneError& error, int32_t bufferSize = -1);
	virtual void close();
	virtual bool deleteFile(const char* name, const bool throwError=true);

	/**
	 * Emulate windows whereby deleting an open file is not
	 * allowed (raise IOException).
	 */
	void setNoDeleteOpenFile(bool value);
	bool getNoDeleteOpenFile() const;

	void setMaxUsedSize(int64_t value);
	int64_t getMaxUsedSize() const;
	void resetMaxUsedSize();

	void setMaxSizeInBytes(int64_t value);
	int64_t getMaxSizeInBytes() const;

	/**
	 * If 0.0, no exceptions will be thrown. Else this should
	 * be a double 0.0 - 1.0. We will randomly throw an
	 * IOException on the first write to an OutputStream based
	 * on this probability.
	 */ 
	void setRandomIOExceptionRate(float_t rate, int32_t seed);
	float_t getRandomIOExceptionRate() const;

	void maybeThrowIOException();

	/** Provided for testing purposes. Use sizeInBytes() instead. */ 
	int64_t getRecomputedSizeInBytes();
 	/**
	 * Like getRecomputedSizeInBytes(), but, uses actual file
	 * lengths rather than buffer allocations (which are
	 * quantized up to nearest
	 * RAMOutputStream.BUFFER_SIZE (now 1024) bytes.
	 */ 
	int64_t getRecomputedActualSizeInBytes();

	void failOn(Failure* fail);
	void maybeThrowDeterministicException();

	std::map<std::string, int32_t>& getOpenFiles();

	DEFINE_MUTABLE_MUTEX(openFiles_mutex);

private:
	std::map<std::string, int32_t> openFiles;
	std::vector<Failure*> failures;
	bool noDeleteOpenFile;
	int64_t maxUsedSize;
	int64_t maxSize;
	float_t randomIOExceptionRate;
};

/**
 * Subclass of RAMFile to access RAMFile::directory without making it public in RAMFile.
 */
class MockRAMFile : public RAMFile {

public:

	MockRAMFile(RAMDirectory* directory) : RAMFile(directory) {
		// empty
	}

	virtual ~MockRAMFile() {
		// empty
	}

	void setDirectory(RAMDirectory* value) {
		directory = value;
	}
};

class MockRAMOutputStream : public RAMOutputStream {

public:
	MockRAMOutputStream();
	MockRAMOutputStream(MockRAMDirectory* d, MockRAMFile* f);
	virtual void close(void);
	virtual void flush(void);
	virtual void writeByte(const uint8_t b);
	virtual void writeBytes(const uint8_t* b, const int32_t length);
	virtual int64_t length() const;

private:
	MockRAMDirectory* dir;
	bool first;
	uint8_t singleByte[1];
};

class MockRAMInputStream : public RAMInputStream {

public:
	MockRAMInputStream(const MockRAMInputStream& clone);
	MockRAMInputStream(MockRAMDirectory* d, const char* n, MockRAMFile* f);
	void close(void);

	DEFINE_MUTABLE_MUTEX(openFiles_mutex);

private:
	MockRAMDirectory* dir;
	std::string name;
	bool isClone;
};


CL_NS_END

#endif // _lucene_store_MockRAMDirectory_
