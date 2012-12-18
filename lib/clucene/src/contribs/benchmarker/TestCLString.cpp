/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "stdafx.h"

using namespace lucene::util;
using namespace lucene::analysis;
using namespace lucene::document;
using namespace lucene::index;
using namespace lucene::store;

int BenchmarkDocumentWriter(Timer* timerCase)
{
	RAMDirectory ram;
	SimpleAnalyzer an;
	IndexWriter* ndx = _CLNEW IndexWriter(&ram, &an, true);
   ndx->setMaxFieldLength(0x7FFFFFFF);

   char fname[1024];
	strcpy(fname, clucene_data_location);
   strcat(fname, "reuters-21578/feldman-cia-worldfactbook-data.txt");
	
	timerCase->start();
	for ( int i=0;i<10;i++ ){
  
		FileReader* reader = _CLNEW FileReader(fname, "ASCII");
		Document doc;
		doc.add(*_CLNEW Field(_T("contents"),reader, Field::STORE_YES | Field::INDEX_TOKENIZED));
		
		ndx->addDocument(&doc);
	}
	ndx->close();
	timerCase->stop();

   ram.close();
	_CLDELETE(ndx);
	return 0;
}

int BenchmarkTermDocs(Timer* timerCase){
	IndexReader* reader = IndexReader::open("index");
	timerCase->start();
	TermEnum* en = reader->terms();
	while (en->next()){
		Term* term = en->term();
		_CLDECDELETE(term);
	}
	en->close();
	_CLDELETE(en);
	timerCase->stop();
	reader->close();
	_CLDELETE(reader);
	return 0;
}
