
#ifndef _lucene_index_IndexFileNames_
#define _lucene_index_IndexFileNames_

#include "CLucene/util/Array.h"

CL_NS_DEF(index)

class CLUCENE_EXPORT IndexFileNames {
	static CL_NS(util)::ConstValueArray<const char*> _INDEX_EXTENSIONS;
	static CL_NS(util)::ConstValueArray<const char*> _INDEX_EXTENSIONS_IN_COMPOUND_FILE;
	static CL_NS(util)::ConstValueArray<const char*> _STORE_INDEX_EXTENSIONS;
	static CL_NS(util)::ConstValueArray<const char*> _NON_STORE_INDEX_EXTENSIONS;
	static CL_NS(util)::ConstValueArray<const char*> _COMPOUND_EXTENSIONS;
	static CL_NS(util)::ConstValueArray<const char*> _VECTOR_EXTENSIONS;
public:
	static const char* SEGMENTS;
	static const char* SEGMENTS_GEN;
	static const char* DELETABLE;
	static const char* NORMS_EXTENSION;
	static const char* FREQ_EXTENSION;
	static const char* PROX_EXTENSION;
	static const char* TERMS_EXTENSION;
	static const char* TERMS_INDEX_EXTENSION;
	static const char* FIELDS_INDEX_EXTENSION;
	static const char* FIELDS_EXTENSION;
	static const char* VECTORS_FIELDS_EXTENSION;
	static const char* VECTORS_DOCUMENTS_EXTENSION;
	static const char* VECTORS_INDEX_EXTENSION;
	static const char* COMPOUND_FILE_EXTENSION;
	static const char* COMPOUND_FILE_STORE_EXTENSION;
	static const char* DELETES_EXTENSION;
	static const char* FIELD_INFOS_EXTENSION;
	static const char* PLAIN_NORMS_EXTENSION;
	static const char* SEPARATE_NORMS_EXTENSION;
	static const char* GEN_EXTENSION;
	
	LUCENE_STATIC_CONSTANT(int32_t,COMPOUND_EXTENSIONS_LENGTH=7);
	LUCENE_STATIC_CONSTANT(int32_t,VECTOR_EXTENSIONS_LENGTH=3);
	LUCENE_STATIC_CONSTANT(int32_t,STORE_INDEX_EXTENSIONS_LENGTH=5);
	
	static CL_NS(util)::ConstValueArray<const char*>& INDEX_EXTENSIONS();
	static CL_NS(util)::ConstValueArray<const char*>& INDEX_EXTENSIONS_IN_COMPOUND_FILE();
	static CL_NS(util)::ConstValueArray<const char*>& STORE_INDEX_EXTENSIONS();
	static CL_NS(util)::ConstValueArray<const char*>& NON_STORE_INDEX_EXTENSIONS();
	static CL_NS(util)::ConstValueArray<const char*>& COMPOUND_EXTENSIONS();
	static CL_NS(util)::ConstValueArray<const char*>& VECTOR_EXTENSIONS();
	
  static std::string fileNameFromGeneration( const char* base, const char* extension, int64_t gen );
	static bool isDocStoreFile( const char* fileName );
	
};

CL_NS_END
#endif
