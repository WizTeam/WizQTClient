/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <cctype>
#include <string.h>
#include <algorithm>

#include "CLucene/StdHeader.h"
#include "CLucene/_clucene-config.h"

#include "CLucene.h"
#include "CLucene/util/CLStreams.h"
#include "CLucene/util/dirent.h"
#include "CLucene/config/repl_tchar.h"
#include "CLucene/util/Misc.h"
#include "CLucene/util/StringBuffer.h"

using namespace std;
using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;

void FileDocument(const char* f, Document* doc){

    // Add the path of the file as a field named "path".  Use an indexed and stored field, so
    // that the index stores the path, and so that the path is searchable.
    TCHAR tf[CL_MAX_DIR];
    STRCPY_AtoT(tf,f,CL_MAX_DIR);
    doc->add( *_CLNEW Field(_T("path"), tf, Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );

    // Add the last modified date of the file a field named "modified". Again, we make it
    // searchable, but no attempt is made to tokenize the field into words.
    //doc->add( *_CLNEW Field(_T("modified"), DateTools::timeToString(f->lastModified()), Field::STORE_YES | Field::INDEX_NO));

    // Add the contents of the file a field named "contents".  This time we use a tokenized
	// field so that the text can be searched for words in it.

    // Here we read the data without any encoding. If you want to use special encoding
    // see the contrib/jstreams - they contain various types of stream readers
    FILE* fh = fopen(f,"r");
	if ( fh != NULL ){
		StringBuffer str;
		char abuf[1024];
		TCHAR tbuf[1024];
		size_t r;
		do{
			r = fread(abuf,1,1023,fh);
			abuf[r]=0;
			STRCPY_AtoT(tbuf,abuf,r);
			tbuf[r]=0;
			str.append(tbuf);
		}while(r>0);
		fclose(fh);

		doc->add( *_CLNEW Field(_T("contents"), str.getBuffer(), Field::STORE_YES | Field::INDEX_TOKENIZED) );
	}
}

void indexDocs(IndexWriter* writer, const char* directory) {
    vector<string> files;
    std::sort(files.begin(),files.end());
    Misc::listFiles(directory,files,true);
    vector<string>::iterator itr = files.begin();
    
    // Re-use the document object
    Document doc;
    int i=0;
    while ( itr != files.end() ){
        const char* path = itr->c_str();
        printf( "adding file %d: %s\n", ++i, path );

        doc.clear();
        FileDocument( path, &doc );
        writer->addDocument( &doc );
        ++itr;
    }
}
void IndexFiles(const char* path, const char* target, const bool clearIndex){
	IndexWriter* writer = NULL;
	lucene::analysis::WhitespaceAnalyzer an;
	
	if ( !clearIndex && IndexReader::indexExists(target) ){
		if ( IndexReader::isLocked(target) ){
			printf("Index was locked... unlocking it.\n");
			IndexReader::unlock(target);
		}

		writer = _CLNEW IndexWriter( target, &an, false);
	}else{
		writer = _CLNEW IndexWriter( target ,&an, true);
	}

    //writer->setInfoStream(&std::cout);

    // We can tell the writer to flush at certain occasions
    //writer->setRAMBufferSizeMB(0.5);
    //writer->setMaxBufferedDocs(3);

    // To bypass a possible exception (we have no idea what we will be indexing...)
    writer->setMaxFieldLength(0x7FFFFFFFL); // LUCENE_INT32_MAX_SHOULDBE
    
    // Turn this off to make indexing faster; we'll turn it on later before optimizing
    writer->setUseCompoundFile(false);

	uint64_t str = Misc::currentTimeMillis();

	indexDocs(writer, path);
	
    // Make the index use as little files as possible, and optimize it
    writer->setUseCompoundFile(true);
    writer->optimize();
	
    // Close and clean up
    writer->close();
	_CLLDELETE(writer);

	printf("Indexing took: %d ms.\n\n", (int32_t)(Misc::currentTimeMillis() - str));
}
