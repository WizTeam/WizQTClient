/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
//#include "CLucene/util/_ThreadLocal.h"
#include "CLucene/util/Equators.h"

void testError ( CuTest *tc )
{
	const char* msg = "test";
	CLuceneError err ( 0,msg,false );
	CLuceneError err2 = err;
	CuAssert ( tc,_T ( "Error did not copy properly" ),err.what() !=err2.what() );
	CuAssert ( tc,_T ( "Error values did not correspond" ),strcmp ( err.what(),err2.what() ) ==0 );

	IndexReader* reader = NULL;
	try
	{
		RAMDirectory dir; _CL_LDECREF(& dir);
		reader = IndexReader::open ( &dir,true );
	}
	catch ( CLuceneError& )
	{
		_CLDELETE ( reader );
	}
	catch ( ... )
	{
		_CLDELETE ( reader );
		CuAssert ( tc,_T ( "Error did not catch properly" ),false );
	}
}

/*
typedef CL_NS(util)::ThreadLocal<char*, CL_NS(util)::Deletor::acArray> tlTest;
struct Data{
	tlTest* tl;
	CuTest *tc;
};
_LUCENE_THREAD_FUNC ( threadLocalTest, arg )
{
	Data* data = (Data*)arg;
	CuTest *tc = data->tc;
	tlTest* tl = data->tl;

	char* val = tl->get();

	CLUCENE_ASSERT(val == NULL);

	tl->set(STRDUP_AtoA("test"));
	tl->setNull();

	val = _CL_NEWARRAY(char, 50);
	_snprintf(val, 50, "hello from thread %d", (int)_LUCENE_CURRTHREADID);

	tl->set(val);

	CLUCENE_ASSERT(tl->get() != NULL);

	//wait a bit until thread local deleted our data...
	Misc::sleep(1000);

	CLUCENE_ASSERT(tl->get() == NULL);
}
void testThreadLocal ( CuTest *tc )
{

	const int threadsCount = 10;

	//read using multiple threads...
	_LUCENE_THREADID_TYPE threads[threadsCount];

	Data data;
	data.tc = tc;
	data.tl = _CLNEW tlTest;

	int i;
	for ( i=0;i<threadsCount;i++ )
		threads[i] = _LUCENE_THREAD_CREATE ( &threadLocalTest, &data );

	CL_NS ( util ) ::Misc::sleep ( 500 );

	_CLDELETE (data.tl);

	for ( i=0;i<threadsCount;i++ )
		_LUCENE_THREAD_JOIN ( threads[i] );
}
*/
CuSuite *testdebug ( void )
{
	CuSuite *suite = CuSuiteNew ( _T ( "CLucene Debug Test" ) );

	SUITE_ADD_TEST ( suite, testError );
	//SUITE_ADD_TEST ( suite, testThreadLocal );

	return suite;
}
// EOF
