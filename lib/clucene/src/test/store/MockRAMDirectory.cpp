/*------------------------------------------------------------------------------
* Copyright (C) 2003-2010 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#include "MockRAMDirectory.h"

#include <iostream>
#include <string.h>
#include "CLucene/_ApiHeader.h"

CL_NS_DEF(store)

MockRAMDirectory::MockRAMDirectory() :
	RAMDirectory(),
	noDeleteOpenFile(true), maxSize(0) {
	// empty
}

MockRAMDirectory::MockRAMDirectory(const char* dir) :
	RAMDirectory(dir),
	noDeleteOpenFile(true), maxSize(0) {
	// empty
}

MockRAMDirectory::MockRAMDirectory(Directory* dir) :
	RAMDirectory(dir),
	noDeleteOpenFile(true), maxSize(0) {
	// empty
}

MockRAMDirectory::~MockRAMDirectory() {
	while (!failures.empty()) {
		delete failures.back();
		failures.pop_back();
	}
}

IndexOutput* MockRAMDirectory::createOutput(const char* name) {
	MockRAMFile* file = new MockRAMFile(this);

	{
		SCOPED_LOCK_MUTEX(openFiles_mutex);

		if (noDeleteOpenFile && openFiles.find(name) != openFiles.end()) {
			char buffer[200];
			_snprintf(buffer, 200, "MockRAMDirectory: file %s is still open: cannot overwrite", name);
			_CLTHROWA(CL_ERR_IO, buffer);
		}
	}

	SCOPED_LOCK_MUTEX(files_mutex);

	MockRAMFile* existing = static_cast<MockRAMFile*>(files->get((char*)name));

	if (existing != NULL && strcmp(name, "segments.gen") != 0) {
		char buffer[200];
		_snprintf(buffer, 200, "MockRAMDirectory: file %s already exist", name);
		_CLTHROWA(CL_ERR_IO, buffer);
	} else {
		if (existing != NULL) {
            SCOPED_LOCK_MUTEX(THIS_LOCK);
			sizeInBytes -= existing->getSizeInBytes();
			existing->setDirectory(NULL);
		}

	    files->put(STRDUP_AtoA(name), file);
	}

	return _CLNEW MockRAMOutputStream(this, file);
}

bool MockRAMDirectory::openInput(const char* name, IndexInput*& ret, CLuceneError& error, int32_t buffferSize) {
	SCOPED_LOCK_MUTEX(files_mutex);
	MockRAMFile* file = static_cast<MockRAMFile*>(files->get((char*)name));

	if (file == NULL) {
		char buffer[200];
		_snprintf(buffer, 200, "MockRAMDirectory: file %s not found", name);
		error.set(CL_ERR_IO, buffer);
		return false;
	} else {
		SCOPED_LOCK_MUTEX(openFiles_mutex);

		if (openFiles.find(name) != openFiles.end()) {
			++openFiles[name];
		} else {
			openFiles.insert(std::make_pair<std::string, int32_t>(name, 1));
		}
	}

	ret = _CLNEW MockRAMInputStream(this, name, file);
	return true;
}

void MockRAMDirectory::close() {
	SCOPED_LOCK_MUTEX(openFiles_mutex);
	if (noDeleteOpenFile && !openFiles.empty()) {
		char buffer[200];
		_snprintf(buffer, 200, "MockRAMDirectory: cannot close: there are still open files: %d", (int)openFiles.size());
		_CLTHROWA(CL_ERR_IO, buffer);
	}
}

bool MockRAMDirectory::deleteFile(const char* name, const bool throwError) {
	SCOPED_LOCK_MUTEX(openFiles_mutex);
	if (noDeleteOpenFile && openFiles.find(name) != openFiles.end() && throwError) {
		char buffer[200];
		_snprintf(buffer, 200, "MockRAMDirectory: file %s is still open: cannot delete", name);
		_CLTHROWA(CL_ERR_IO, buffer);
	}

	return RAMDirectory::deleteFile(name, throwError);
}

void MockRAMDirectory::setNoDeleteOpenFile(bool value) {
	noDeleteOpenFile = value;
}

void MockRAMDirectory::setMaxUsedSize(int64_t value) {
	maxUsedSize = value;
}

int64_t MockRAMDirectory::getMaxUsedSize(void) const {
	return maxUsedSize;
}

void MockRAMDirectory::resetMaxUsedSize(void) {
	maxUsedSize = getRecomputedActualSizeInBytes();
}

void MockRAMDirectory::setMaxSizeInBytes(int64_t value) {
	maxSize = value;
}

int64_t MockRAMDirectory::getMaxSizeInBytes() const {
	return maxSize;
}

void MockRAMDirectory::setRandomIOExceptionRate(float_t rate, int32_t seed) {
	srand(seed);
	randomIOExceptionRate = rate;
}

float_t MockRAMDirectory::getRandomIOExceptionRate(void) const {
	return randomIOExceptionRate;
}

bool MockRAMDirectory::getNoDeleteOpenFile() const {
	return noDeleteOpenFile;
}

std::map<std::string, int32_t>& MockRAMDirectory::getOpenFiles() {
	return openFiles;
}

void MockRAMDirectory::maybeThrowIOException(void) {
	if (randomIOExceptionRate > 0.0) {
		// don't use low bits from rand()
		// (see http://en.wikipedia.org/wiki/Linear_congruential_generator#Advantages_and_disadvantages_of_LCGs)
		int32_t number = ((rand() >> 4) % 1000);
		if (number < randomIOExceptionRate * 1000) {
			char buffer[200];
			_snprintf(buffer, 200, "MockRAMDirectory: a random IOException");
			_CLTHROWA(CL_ERR_IO, buffer);
		}
	}
}

int64_t MockRAMDirectory::getRecomputedSizeInBytes(void) {
	SCOPED_LOCK_MUTEX(files_mutex);
	int64_t size = 0;
	RAMDirectory::FileMap::iterator it = files->begin();

	while (it != files->end()) {
		size += (*it).second->getSizeInBytes();
		it++;
	}

	return size;
}

int64_t MockRAMDirectory::getRecomputedActualSizeInBytes(void) {
	SCOPED_LOCK_MUTEX(files_mutex);
	int64_t size = 0;
	RAMDirectory::FileMap::iterator it = files->begin();

	while (it != files->end()) {
		size += (*it).second->getLength();
		it++;
	}

	return size;
}

void MockRAMDirectory::failOn(Failure* fail) {
	failures.push_back(fail);
}

void MockRAMDirectory::maybeThrowDeterministicException(void) {
	std::vector<Failure*>::iterator it = failures.begin();

	while (it != failures.end()) {
		(*it)->eval(this);
		it++;
	}
}

//
// MockRAMOutputStream
//

MockRAMOutputStream::MockRAMOutputStream() :
	RAMOutputStream(),
	dir(NULL),
	first(true) {
	// empty
}

MockRAMOutputStream::MockRAMOutputStream(MockRAMDirectory* d, MockRAMFile* f) :
	RAMOutputStream(f),
	dir(d),
	first(true) {
	// empty
}

void MockRAMOutputStream::close() {
	RAMOutputStream::close();

	int64_t size = dir->getRecomputedActualSizeInBytes();

	if (size > dir->getMaxUsedSize()) {
		dir->setMaxUsedSize(size);
	}
}

void MockRAMOutputStream::flush() {
	dir->maybeThrowDeterministicException();
	RAMOutputStream::flush();
}

void MockRAMOutputStream::writeByte(const uint8_t b) {
	singleByte[0] = b;
	writeBytes(singleByte, 1);
}

void MockRAMOutputStream::writeBytes(const uint8_t* b, const int32_t length) {
	int64_t freeSpace = dir->getMaxSizeInBytes() - dir->sizeInBytes;
	int64_t realUsage = 0;

	// Enforce disk full
	if (dir->getMaxSizeInBytes() != 0 && freeSpace < length) {
		// Compute the real disk free. This will greatly slow
	 	// down our test but makes it more accurate:
	 	realUsage = dir->getRecomputedActualSizeInBytes();
		freeSpace = dir->getMaxSizeInBytes() - realUsage;
	}

	if (dir->getMaxSizeInBytes() != 0 && freeSpace <= length) {
		if (freeSpace > 0 && freeSpace < length) {
		 	realUsage += freeSpace;
			RAMOutputStream::writeBytes(b, freeSpace);
		}

		if (realUsage > dir->getMaxUsedSize()) {
			dir->setMaxUsedSize(realUsage);
		}

		char buffer[200];
		_snprintf(buffer, 200, "MockRAMOutputStream: fake disk full at %d bytes", (int)dir->getRecomputedActualSizeInBytes());
		_CLTHROWA(CL_ERR_IO, buffer);
	} else {
		RAMOutputStream::writeBytes(b, length);
	}

	dir->maybeThrowDeterministicException();

	if (first) {
		// Maybe throw random exception; only do this on first
		// write to a new file:
		first = false;
		dir->maybeThrowIOException();
	}
}

int64_t MockRAMOutputStream::length() const {
	return RAMOutputStream::length();
}

//
// MockRAMInputStream
//

MockRAMInputStream::MockRAMInputStream(const MockRAMInputStream& clone) :
	RAMInputStream(clone),
	isClone(true) {
	dir = clone.dir;
	name = clone.name;
}

MockRAMInputStream::MockRAMInputStream(MockRAMDirectory* d, const char* n, MockRAMFile* f) :
	RAMInputStream(f),
	dir(d),
	name(n),
	isClone(false) {
	// empty
}

void MockRAMInputStream::close() {
	RAMInputStream::close();

	if (!isClone) {
		SCOPED_LOCK_MUTEX(openFiles_mutex);
		int v = dir->getOpenFiles()[name.c_str()];

		if (v == 1) {
			dir->getOpenFiles().erase(name.c_str());
		} else {
			--dir->getOpenFiles()[name.c_str()];
		}
	}
}

CL_NS_END
