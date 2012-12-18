#include "test.h"
#include "CLucene/store/Directory.h"
#include "CLucene/store/IndexInput.h"
#include "CLucene/util/BitSet.h"

CL_NS_USE(util)
CL_NS_USE(store)

/**
 * Compare two BitVectors.
 * This should really be an equals method on the BitVector itself.
 * @param bv One bit vector
 * @param compare The second to compare
 */
bool doCompare(BitSet& bv, BitSet& compare) {
    bool equal = true;
    for(int i=0;i<bv.size();i++) {
        // bits must be equal
        if(bv.get(i)!=compare.get(i)) {
            equal = false;
            break;
        }
    }
    return equal;
}

void doTestConstructOfSize(CuTest* tc, int n) {
    BitSet bv(n);
    CLUCENE_ASSERT(n==bv.size());
}

void testConstructSize(CuTest *tc) {
    doTestConstructOfSize(tc, 8);
    doTestConstructOfSize(tc, 20);
    doTestConstructOfSize(tc, 100);
    doTestConstructOfSize(tc, 1000);
}

void doTestGetSetVectorOfSize(CuTest* tc, int n) {
    BitSet bv(n);
    for(int i=0;i<bv.size();i++) {
        // ensure a set bit can be git'
        CLUCENE_ASSERT(!bv.get(i));
        bv.set(i);
        CLUCENE_ASSERT(bv.get(i));
    }
}

/**
 * Test the get() and set() methods on BitVectors of various sizes.
 * @throws Exception
 */
void testGetSet(CuTest* tc) {
    doTestGetSetVectorOfSize(tc, 8);
    doTestGetSetVectorOfSize(tc, 20);
    doTestGetSetVectorOfSize(tc, 100);
    doTestGetSetVectorOfSize(tc, 1000);
}

void doTestClearVectorOfSize(CuTest* tc, int n) {
    BitSet bv(n);
    for(int i=0;i<bv.size();i++) {
        // ensure a set bit is cleared
        CLUCENE_ASSERT(!bv.get(i));
        bv.set(i, true);
        CLUCENE_ASSERT(bv.get(i));
        bv.set(i, false);
        CLUCENE_ASSERT(!bv.get(i));
    }
}

/**
 * Test the clear() method on BitVectors of various sizes.
 * @throws Exception
 */
void testClear(CuTest* tc) {
    doTestClearVectorOfSize(tc, 8);
    doTestClearVectorOfSize(tc, 20);
    doTestClearVectorOfSize(tc, 100);
    doTestClearVectorOfSize(tc, 1000);
}

void doTestCountVectorOfSize(CuTest* tc, int n) {
    BitSet* bv = _CLNEW BitSet(n);
    // test count when incrementally setting bits
    for(int i=0;i<bv->size();i++) {
        CLUCENE_ASSERT(!bv->get(i));
        CLUCENE_ASSERT(i==bv->count());
        bv->set(i);
        CLUCENE_ASSERT(bv->get(i));
        CLUCENE_ASSERT(i+1==bv->count());
    }

    _CLLDELETE(bv);
    bv = _CLNEW BitSet(n);
    // test count when setting then clearing bits
    for(int i=0;i<bv->size();i++) {
        CLUCENE_ASSERT(!bv->get(i));
        CLUCENE_ASSERT(0==bv->count());
        bv->set(i, true);
        CLUCENE_ASSERT(bv->get(i));
        CLUCENE_ASSERT(1==bv->count());
        bv->set(i, false);
        CLUCENE_ASSERT(!bv->get(i));
        CLUCENE_ASSERT(0==bv->count());
    }

    _CLLDELETE(bv);
}

/**
 * Test the count() method on BitVectors of various sizes.
 * @throws Exception
 */
void testCount(CuTest* tc) {
    doTestCountVectorOfSize(tc, 8);
    doTestCountVectorOfSize(tc, 20);
    doTestCountVectorOfSize(tc, 100);
    doTestCountVectorOfSize(tc, 1000);
}

void doTestWriteRead(CuTest* tc, int n) {
    Directory* d = new RAMDirectory();

    BitSet bv(n);
    // test count when incrementally setting bits
    for(int i=0;i<bv.size();i++) {
        CLUCENE_ASSERT(!bv.get(i));
        CLUCENE_ASSERT(i==bv.count());
        bv.set(i);
        CLUCENE_ASSERT(bv.get(i));
        CLUCENE_ASSERT(i+1==bv.count());
        bv.write(d, "TESTBV");
        BitSet compare(d, "TESTBV");
        // compare bit vectors with bits set incrementally
        CLUCENE_ASSERT(doCompare(bv,compare));
    }

    _CLLDECDELETE( d );
}

/**
 * Test writing and construction to/from Directory.
 * @throws Exception
 */
void testWriteRead(CuTest* tc) {
    doTestWriteRead(tc, 8);
    doTestWriteRead(tc, 20);
    doTestWriteRead(tc, 100);
    doTestWriteRead(tc, 1000);
}

void doTestDgaps(CuTest* tc, int size, int count1, int count2) {
  Directory* d = _CLNEW RAMDirectory();
  BitSet* bv = _CLNEW BitSet(size);
  for (int i=0; i<count1; i++) {
    bv->set(i);
    CLUCENE_ASSERT(i+1==bv->count());
  }
  bv->write(d, "TESTBV");
  // gradually increase number of set bits
  for (int i=count1; i<count2; i++) {
    BitSet* bv2 = _CLNEW BitSet(d, "TESTBV");
    CLUCENE_ASSERT(doCompare(*bv,*bv2));
    _CLLDELETE(bv);
    bv = bv2;
    bv->set(i, true);
    CLUCENE_ASSERT(i+1==bv->count());
    bv->write(d, "TESTBV");
  }
  // now start decreasing number of set bits
  for (int i=count2-1; i>=count1; i--) {
    BitVector* bv2 = _CLNEW BitSet(d, "TESTBV");
    CLUCENE_ASSERT(doCompare(*bv,*bv2));
    _CLLDELETE(bv);
    bv = bv2;
    bv->set(i, false);
    CLUCENE_ASSERT(i==bv->count());
    bv->write(d, "TESTBV");
  }
  _CLLDELETE(bv);
  _CLLDECDELETE( d );
}

/**
 * Test r/w when size/count cause switching between bit-set and d-gaps file formats.  
 * @throws Exception
 */
void testDgaps(CuTest* tc) {
  doTestDgaps(tc, 1,0,1);
  doTestDgaps(tc, 10,0,1);
  doTestDgaps(tc, 100,0,1);
  doTestDgaps(tc, 202,0,1);
  doTestDgaps(tc, 1000,4,7);
  doTestDgaps(tc, 10000,40,43);
  doTestDgaps(tc, 100000,415,418);
  doTestDgaps(tc, 1000000,3123,3126);
}

void doTestBitAtEndOfBitSet(CuTest* tc, int size, int pos) {
  Directory* d = _CLNEW RAMDirectory();
  BitSet* bv = _CLNEW BitSet(size);

  bv->set(pos, true);
  bv->write(d, "TESTBV");
  _CLLDELETE(bv);
  bv = _CLNEW BitSet(d, "TESTBV");
  CLUCENE_ASSERT(bv->get(pos));
  _CLLDELETE( bv );
  _CLLDECDELETE( d );
}

void testBitAtEndOfBitSet(CuTest* tc) {
  doTestBitAtEndOfBitSet(tc, 202, 200);
}

void doTestNextSetBit(CuTest* tc, int nSize)
{
    BitSet  bv( nSize );
    int     nIdx;
    int     nExpectedIdx;

    // initialize bit set by setting every second bit starting with 0
    for( int32_t i = 0; i < bv.size(); i+=2 )
        bv.set(i);
    
    // iterate the bits
    nIdx = 0;
    nExpectedIdx = 0;
    while( (nIdx = bv.nextSetBit( nIdx )) != -1 )
    {
        assertEquals( nExpectedIdx, nIdx );
        nExpectedIdx += 2;
        nIdx++;
    }
}

/**
 * Test the nextSetBit() method on BitVectors of various sizes.
 * CLucene specific
 * @throws Exception
 */
void testNextSetBit(CuTest* tc)
{
    doTestNextSetBit(tc, 8);
    doTestNextSetBit(tc, 20);
    doTestNextSetBit(tc, 100);
}

CuSuite *testBitSet(void)
{
    CuSuite *suite = CuSuiteNew(_T("CLucene BitSet Test"));

    SUITE_ADD_TEST(suite, testConstructSize);
    SUITE_ADD_TEST(suite, testGetSet);
    SUITE_ADD_TEST(suite, testClear);
    SUITE_ADD_TEST(suite, testCount);
    SUITE_ADD_TEST(suite, testWriteRead);
    SUITE_ADD_TEST(suite, testDgaps);
    SUITE_ADD_TEST(suite, testBitAtEndOfBitSet);

    SUITE_ADD_TEST(suite, testNextSetBit);

    return suite; 
}
