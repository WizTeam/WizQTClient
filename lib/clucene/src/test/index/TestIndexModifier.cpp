/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
#include "CLucene/index/IndexModifier.h"
#include <sstream>

CL_NS_USE(store)
CL_NS_USE(index)
CL_NS_USE(document)
CL_NS_USE2(analysis,standard)

void IndexModifierExceptionTest(CuTest *tc)
{
    class LockedLock : public LuceneLock
    {
    public:
        LockedLock() : LuceneLock() {};
        virtual bool obtain() {return obtain(0);};
        virtual void release() {};
        virtual bool isLocked() {return true;};
        bool obtain(int64_t lockWaitTimeout) {return false;};
        virtual std::string toString() {return "LockedLock";};
        virtual const char* getObjectName() const {return "LockedLock";};
    };

    class LockedDirectory : public RAMDirectory
    {
    public:
        bool    errOn;

        LockedDirectory() : RAMDirectory(), errOn(false) {};

        // this simulates locking problem, only if errOn is true
        LuceneLock* makeLock(const char* name) {
            if (errOn)
                return _CLNEW LockedLock();
            else
                return RAMDirectory::makeLock(name);
        };
    };

    LockedDirectory     directory;
    StandardAnalyzer    analyzer;
    Document            doc;
    IndexModifier *     pIm = NULL;

    try
    {
        doc.add(* _CLNEW Field(_T("text"), _T("Document content"), Field::STORE_YES | Field::INDEX_TOKENIZED));
        pIm = _CLNEW IndexModifier(&directory, &analyzer, true);

        pIm->addDocument(&doc);
        pIm->deleteDocument(0);
    }
    catch (CLuceneError & err)
    {
        CuFail(tc, _T("Exception thrown upon startup"));
        return;
    }

    try
    {
        // switch on locking timeout simulation
        directory.errOn = true;

        // throws lock timeout exception
        pIm->addDocument(&doc);
        CuFail(tc, _T("Exception was not thrown during addDocument"));
    }
    catch (CLuceneError & err)
    {
    }

    // this produces Access Violation exception
    try {
        _CLLDELETE(pIm);
    } catch (...)
    {
        CuFail(tc, _T("Exception thrown upon deletion"));
    }
}

class bulk_modification {
public:
	void modify_index(CuTest *tc, IndexModifier& ndx);
};

class incremental_modification {
public:
	void modify_index(CuTest *tc, IndexModifier& ndx);
};

template<typename modification>
class IMinsertDelete_tester : public modification {
public:
	void invoke(Directory& storage, CuTest *tc);
};

void bulk_modification::modify_index(CuTest *tc, IndexModifier& ndx){
	std::basic_stringstream<TCHAR> field;
	for ( int i=0;i<1000;i++ ){
		field.str(_T(""));
		field << _T("fielddata") << i;

		Document doc;
		
		doc.add (
			*_CLNEW Field(
				_T("field0"),
				field.str().c_str(),
				Field::STORE_YES | Field::INDEX_UNTOKENIZED
			)
		);
		ndx.addDocument(&doc);
	}
	for ( int i=0;i<1000;i+=2 ){
		field.str(_T(""));
		field << _T("fielddata") << i;

		Term deleted(
			_T("field0"),
			field.str().c_str(),
			true
		);
		CLUCENE_ASSERT(ndx.deleteDocuments(&deleted) > 0);
	}
}

void incremental_modification::modify_index(CuTest *tc, IndexModifier& ndx){
	std::basic_stringstream<TCHAR> field;
	for ( int i=0;i<1000;i++ ){
		field.str(_T(""));
		field << _T("fielddata") << i;

		Document doc;
		
		doc.add (
			*_CLNEW Field(
				_T("field0"),
				field.str().c_str(),
				Field::STORE_YES | Field::INDEX_UNTOKENIZED
			)
		);
		ndx.addDocument(&doc);
		if ( 0 == i % 2 ) {
			Term deleted(
				_T("field0"),
				field.str().c_str(),
				true
			);
			CLUCENE_ASSERT(ndx.deleteDocuments(&deleted) > 0);
		}
	}
}

template<typename modification>
void IMinsertDelete_tester<modification>::invoke(
	Directory& storage,
	CuTest *tc
){
	SimpleAnalyzer a;

	IndexModifier ndx2(&storage,&a,true);
	ndx2.close();
	IndexModifier ndx(&storage,&a,false);

	ndx.setUseCompoundFile(false);
	ndx.setMergeFactor(2);

	this->modify_index(tc, ndx);

	ndx.optimize();
	ndx.close();

	//test the ram loading
	RAMDirectory ram2(&storage);
	IndexReader* reader2 = IndexReader::open(&ram2);
	Term* term = _CLNEW Term(_T("field0"),_T("fielddata1"));
	TermDocs* en = reader2->termDocs(term);
	CLUCENE_ASSERT(en->next());
	_CLDELETE(en);
	_CLDECDELETE(term);
	term = _CLNEW Term(_T("field0"),_T("fielddata0"));
	en = reader2->termDocs(term);
	CLUCENE_ASSERT(!en->next());
	_CLDELETE(en);
	_CLDECDELETE(term);
	_CLDELETE(reader2);
}

void testIMinsertDelete(CuTest *tc){
	char fsdir[CL_MAX_PATH];
	_snprintf(fsdir,CL_MAX_PATH,"%s/%s",cl_tempDir, "test.search");
	RAMDirectory ram;
	FSDirectory* disk = FSDirectory::getDirectory(fsdir);
	IMinsertDelete_tester<bulk_modification>().invoke(ram, tc);
	IMinsertDelete_tester<incremental_modification>().invoke(ram, tc);
	IMinsertDelete_tester<bulk_modification>().invoke(*disk, tc);
	IMinsertDelete_tester<incremental_modification>().invoke(*disk, tc);
	disk->close();
	_CLDECDELETE(disk);
}

CuSuite *testIndexModifier(void)
{
	CuSuite *suite = CuSuiteNew(_T("CLucene IndexModifier Test"));
	SUITE_ADD_TEST(suite, IndexModifierExceptionTest);
	SUITE_ADD_TEST(suite, testIMinsertDelete);

  return suite; 
}
