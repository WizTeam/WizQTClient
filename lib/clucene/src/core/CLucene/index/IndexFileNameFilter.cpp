/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_IndexFileNameFilter.h"
#include "_IndexFileNames.h"

CL_NS_DEF(index)

FilenameFilter::~FilenameFilter(){
}

IndexFileNameFilter* IndexFileNameFilter::_singleton = NULL;
IndexFileNameFilter* IndexFileNameFilter::singleton(){
  if ( _singleton == NULL )
    _singleton = _CLNEW IndexFileNameFilter();
  return _singleton;
}

void IndexFileNameFilter::_shutdown(){
  _CLDELETE(_singleton);
}

IndexFileNameFilter::IndexFileNameFilter() {
	size_t i;
	for ( i = 0; i < IndexFileNames::INDEX_EXTENSIONS().length; ++i) {
	  extensions.insert(IndexFileNames::INDEX_EXTENSIONS()[i]);
	}
	for ( i = 0; i < IndexFileNames::INDEX_EXTENSIONS_IN_COMPOUND_FILE().length; ++i) {
	  extensionsInCFS.insert(IndexFileNames::INDEX_EXTENSIONS_IN_COMPOUND_FILE()[i]);
	}
}
IndexFileNameFilter::~IndexFileNameFilter(){
}
bool IndexFileNameFilter::accept(const char* /*dir*/, const char* name) const {
	string _name(name);
	size_t i = _name.find_last_of('.');
	if (i != string::npos) {
	  const char* extension = name + 1 + i;
	  char* tmp;
	  if (extensions.find(extension) != extensions.end()) {
		return true;
	  }

	  size_t l = _name.length();
	  if (*extension == 'f' &&
				 strtol(extension+1, &tmp,10)>= 0 && tmp == (extension+l) ) { //check for f001
		return true;
	  } else if (*extension == 's' &&
				 strtol(extension+1, &tmp,10)>= 0 && tmp == (extension+l)) {
		return true;
	  }
	} else {
	  if ( strcmp(name, IndexFileNames::DELETABLE) == 0 ) return true;
	  else if ( strncmp(name, IndexFileNames::SEGMENTS, strlen(IndexFileNames::SEGMENTS)) == 0 ) return true;
	}
	return false;
}

bool IndexFileNameFilter::isCFSFile(const char* name) const {
	string _name(name);
	size_t i = _name.find_last_of('.');
	if (i != string::npos) {
	  const char* extension = name + 1 + i;
	  char* tmp;
	  if (extensionsInCFS.find(extension) != extensionsInCFS.end() ) {
		return true;
	  }
	  size_t l = _name.length();
	  if (*extension == 'f' &&
				 strtol(extension+1, &tmp,10)>= 0 && tmp == (extension+l)) {
		return true;
	  }
	}
	return false;
}

const IndexFileNameFilter* IndexFileNameFilter::getFilter() {
	return singleton();
}
CL_NS_END
