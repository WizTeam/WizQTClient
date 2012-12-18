/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"

#include "_IndexFileNames.h"
#include "_SegmentInfos.h"
#include "CLucene/util/Misc.h"


CL_NS_DEF(index)

	const char* IndexFileNames::SEGMENTS = "segments";
	const char* IndexFileNames::SEGMENTS_GEN = "segments.gen";
	const char* IndexFileNames::DELETABLE = "deletable";
	const char* IndexFileNames::NORMS_EXTENSION = "nrm";
	const char* IndexFileNames::FREQ_EXTENSION = "frq";
	const char* IndexFileNames::PROX_EXTENSION = "prx";
	const char* IndexFileNames::TERMS_EXTENSION = "tis";
	const char* IndexFileNames::TERMS_INDEX_EXTENSION = "tii";
	const char* IndexFileNames::FIELDS_INDEX_EXTENSION = "fdx";
	const char* IndexFileNames::FIELDS_EXTENSION = "fdt";
	const char* IndexFileNames::VECTORS_FIELDS_EXTENSION = "tvf";
	const char* IndexFileNames::VECTORS_DOCUMENTS_EXTENSION = "tvd";
	const char* IndexFileNames::VECTORS_INDEX_EXTENSION = "tvx";
	const char* IndexFileNames::COMPOUND_FILE_EXTENSION = "cfs";
	const char* IndexFileNames::COMPOUND_FILE_STORE_EXTENSION = "cfx";
	const char* IndexFileNames::DELETES_EXTENSION = "del";
	const char* IndexFileNames::FIELD_INFOS_EXTENSION = "fnm";
	const char* IndexFileNames::PLAIN_NORMS_EXTENSION = "f";
	const char* IndexFileNames::SEPARATE_NORMS_EXTENSION = "s";
	const char* IndexFileNames::GEN_EXTENSION = "gen";
  
	const char* IndexFileNames_INDEX_EXTENSIONS_s[] =
		{
			IndexFileNames::COMPOUND_FILE_EXTENSION,
			IndexFileNames::FIELD_INFOS_EXTENSION,
			IndexFileNames::FIELDS_INDEX_EXTENSION,
			IndexFileNames::FIELDS_EXTENSION,
			IndexFileNames::TERMS_INDEX_EXTENSION,
			IndexFileNames::TERMS_EXTENSION,
			IndexFileNames::FREQ_EXTENSION,
			IndexFileNames::PROX_EXTENSION,
			IndexFileNames::DELETES_EXTENSION,
			IndexFileNames::VECTORS_INDEX_EXTENSION,
			IndexFileNames::VECTORS_DOCUMENTS_EXTENSION,
			IndexFileNames::VECTORS_FIELDS_EXTENSION,
			IndexFileNames::GEN_EXTENSION,
			IndexFileNames::NORMS_EXTENSION,
			IndexFileNames::COMPOUND_FILE_STORE_EXTENSION
		};
  
	CL_NS(util)::ConstValueArray<const char*> IndexFileNames::_INDEX_EXTENSIONS;
  CL_NS(util)::ConstValueArray<const char*>& IndexFileNames::INDEX_EXTENSIONS(){
    if ( _INDEX_EXTENSIONS.length == 0 ){
      _INDEX_EXTENSIONS.values = IndexFileNames_INDEX_EXTENSIONS_s;
      _INDEX_EXTENSIONS.length = 15;
    }
    return _INDEX_EXTENSIONS;
  }

	const char* IndexFileNames_INDEX_EXTENSIONS_IN_COMPOUND_FILE_s[] = {
		IndexFileNames::FIELD_INFOS_EXTENSION,
		IndexFileNames::FIELDS_INDEX_EXTENSION,
		IndexFileNames::FIELDS_EXTENSION,
		IndexFileNames::TERMS_INDEX_EXTENSION,
		IndexFileNames::TERMS_EXTENSION,
		IndexFileNames::FREQ_EXTENSION,
		IndexFileNames::PROX_EXTENSION,
		IndexFileNames::VECTORS_INDEX_EXTENSION,
		IndexFileNames::VECTORS_DOCUMENTS_EXTENSION,
		IndexFileNames::VECTORS_FIELDS_EXTENSION,
		IndexFileNames::NORMS_EXTENSION
	};
	CL_NS(util)::ConstValueArray<const char*> IndexFileNames::_INDEX_EXTENSIONS_IN_COMPOUND_FILE;
  CL_NS(util)::ConstValueArray<const char*>& IndexFileNames::INDEX_EXTENSIONS_IN_COMPOUND_FILE(){
    if ( _INDEX_EXTENSIONS_IN_COMPOUND_FILE.length == 0 ){
      _INDEX_EXTENSIONS_IN_COMPOUND_FILE.values = IndexFileNames_INDEX_EXTENSIONS_IN_COMPOUND_FILE_s;
      _INDEX_EXTENSIONS_IN_COMPOUND_FILE.length = 11;
    }
    return _INDEX_EXTENSIONS_IN_COMPOUND_FILE;
  }

	const char* IndexFileNames_STORE_INDEX_EXTENSIONS_s[] = {
		IndexFileNames::VECTORS_INDEX_EXTENSION,
		IndexFileNames::VECTORS_FIELDS_EXTENSION,
		IndexFileNames::VECTORS_DOCUMENTS_EXTENSION,
		IndexFileNames::FIELDS_INDEX_EXTENSION,
		IndexFileNames::FIELDS_EXTENSION
	};
	CL_NS(util)::ConstValueArray<const char*> IndexFileNames::_STORE_INDEX_EXTENSIONS;
  CL_NS(util)::ConstValueArray<const char*>& IndexFileNames::STORE_INDEX_EXTENSIONS(){
    if ( _STORE_INDEX_EXTENSIONS.length == 0 ){
      _STORE_INDEX_EXTENSIONS.values = IndexFileNames_STORE_INDEX_EXTENSIONS_s;
      _STORE_INDEX_EXTENSIONS.length = 5;
    }
    return _STORE_INDEX_EXTENSIONS;
  }
	
	const char* IndexFileNames_NON_STORE_INDEX_EXTENSIONS_s[] = {
		IndexFileNames::FIELD_INFOS_EXTENSION,
		IndexFileNames::FREQ_EXTENSION,
		IndexFileNames::PROX_EXTENSION,
		IndexFileNames::TERMS_EXTENSION,
		IndexFileNames::TERMS_INDEX_EXTENSION,
		IndexFileNames::NORMS_EXTENSION
	};
	CL_NS(util)::ConstValueArray<const char*> IndexFileNames::_NON_STORE_INDEX_EXTENSIONS;
  CL_NS(util)::ConstValueArray<const char*>& IndexFileNames::NON_STORE_INDEX_EXTENSIONS(){
    if ( _NON_STORE_INDEX_EXTENSIONS.length == 0 ){
      _NON_STORE_INDEX_EXTENSIONS.values = IndexFileNames_NON_STORE_INDEX_EXTENSIONS_s;
      _NON_STORE_INDEX_EXTENSIONS.length = 6;
    }
    return _NON_STORE_INDEX_EXTENSIONS;
  }

	const char* IndexFileNames_COMPOUND_EXTENSIONS_s[] = {
		IndexFileNames::FIELD_INFOS_EXTENSION,
		IndexFileNames::FREQ_EXTENSION,
		IndexFileNames::PROX_EXTENSION,
		IndexFileNames::FIELDS_INDEX_EXTENSION,
		IndexFileNames::FIELDS_EXTENSION,
		IndexFileNames::TERMS_INDEX_EXTENSION,
		IndexFileNames::TERMS_EXTENSION
	};
	CL_NS(util)::ConstValueArray<const char*> IndexFileNames::_COMPOUND_EXTENSIONS;
  CL_NS(util)::ConstValueArray<const char*>& IndexFileNames::COMPOUND_EXTENSIONS(){
    if ( _COMPOUND_EXTENSIONS.length == 0 ){
      _COMPOUND_EXTENSIONS.values = IndexFileNames_COMPOUND_EXTENSIONS_s;
      _COMPOUND_EXTENSIONS.length = 7;
    }
    return _COMPOUND_EXTENSIONS;
  }

	const char* IndexFileNames_VECTOR_EXTENSIONS_s[] = {
		IndexFileNames::VECTORS_INDEX_EXTENSION,
		IndexFileNames::VECTORS_DOCUMENTS_EXTENSION,
		IndexFileNames::VECTORS_FIELDS_EXTENSION
	};
	CL_NS(util)::ConstValueArray<const char*> IndexFileNames::_VECTOR_EXTENSIONS;
  CL_NS(util)::ConstValueArray<const char*>& IndexFileNames::VECTOR_EXTENSIONS(){
    if ( _VECTOR_EXTENSIONS.length == 0 ){
      _VECTOR_EXTENSIONS.values = IndexFileNames_VECTOR_EXTENSIONS_s;
      _VECTOR_EXTENSIONS.length = 3;
    }
    return _VECTOR_EXTENSIONS;
  }

	string IndexFileNames::fileNameFromGeneration( const char* base, const char* extension, int64_t gen ) {
		if ( gen == SegmentInfo::NO ) {
			return "";
		} else if ( gen == SegmentInfo::WITHOUT_GEN ) {
			return string(base) + extension;
		} else {
      char buf[(sizeof(unsigned long) << 3) + 1];
      CL_NS(util)::Misc::longToBase( gen, 36, buf );
      return string(base) + "_" + buf + extension;
		}
	}
	
	bool IndexFileNames::isDocStoreFile( const char* fileName ) {
		
		const char* p = strchr( fileName, (int)'.' );
		
		if ( p != NULL && strcmp( p+1, COMPOUND_FILE_STORE_EXTENSION ) == 0 ) {
			return true;
		}
		for ( int32_t i = 0; i < STORE_INDEX_EXTENSIONS().length; i++ ) {
			if ( p != NULL && strcmp( p+1, STORE_INDEX_EXTENSIONS()[i] ) == 0 ) {
				return true;
			}
		}
		return false;
	}

CL_NS_END
