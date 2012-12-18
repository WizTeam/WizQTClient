#include "test.h"
#include "CLucene/_SharedHeader.h"

CL_NS_USE(document);

  void testFieldConfig(CuTest* tc) {
    Field termVector(_T("name"), _T("value"), Field::INDEX_TOKENIZED | Field::TERMVECTOR_YES);
    Field termVectorPositions(_T("name"), _T("value"), Field::INDEX_TOKENIZED | Field::TERMVECTOR_WITH_POSITIONS);
    Field termVectorOffsets(_T("name"), _T("value"), Field::INDEX_TOKENIZED | Field::TERMVECTOR_WITH_OFFSETS);
    Field termVectorPositionsOffsets(_T("name"), _T("value"), Field::INDEX_TOKENIZED | Field::TERMVECTOR_WITH_POSITIONS_OFFSETS);

    CuAssertTrue(tc, termVector.isTermVectorStored(), _T("Term vector is not stored!"));
    CuAssertTrue(tc, !termVector.isStoreOffsetWithTermVector(), _T("Term vector with offset is stored!"));
    CuAssertTrue(tc, !termVector.isStorePositionWithTermVector(), _T("Term vector with position is stored!"));

    CuAssertTrue(tc, termVectorPositions.isTermVectorStored(), _T("Term vector is not stored!"));
    CuAssertTrue(tc, !termVectorPositions.isStoreOffsetWithTermVector(), _T("Term vector with offset is stored!"));
    CuAssertTrue(tc, termVectorPositions.isStorePositionWithTermVector(), _T("Term vector with position is not stored!"));

    CuAssertTrue(tc, termVectorOffsets.isTermVectorStored(), _T("Term vector is not stored!"));
    CuAssertTrue(tc, termVectorOffsets.isStoreOffsetWithTermVector(), _T("Term vector with offset is not stored!"));
    CuAssertTrue(tc, !termVectorOffsets.isStorePositionWithTermVector(), _T("Term vector with position is stored!"));

    CuAssertTrue(tc, termVectorPositionsOffsets.isTermVectorStored(), _T("Term vector is not stored!"));
    CuAssertTrue(tc, termVectorPositionsOffsets.isStoreOffsetWithTermVector(), _T("Term vector with offset is not stored!"));
    CuAssertTrue(tc, termVectorPositionsOffsets.isStorePositionWithTermVector(), _T("Term vector with position is not stored!"));
  }

CuSuite *testField(void) {
  CuSuite *suite = CuSuiteNew(_T("CLucene Field Test"));

  SUITE_ADD_TEST(suite, testFieldConfig);

  return suite;
}
