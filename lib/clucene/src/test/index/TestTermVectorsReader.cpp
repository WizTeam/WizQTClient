/*------------------------------------------------------------------------------
* Copyright (C) 2003-2010 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
#include "CLucene/_SharedHeader.h"
#include "../store/MockRAMDirectory.h"
#include "CLucene/index/_FieldInfos.h"
#include "CLucene/index/_SegmentInfos.h"
#include "CLucene/index/_IndexFileNames.h"
#include "CLucene/index/_TermVector.h"
#include "CLucene/util/_Arrays.h"

#include <algorithm>
#include <functional>

CL_NS_USE(analysis);
CL_NS_USE(index);
CL_NS_USE(util);

  //Must be lexicographically sorted, will do in setup, versus trying to maintain here
  const TCHAR* testFields_values[] = {_T("f1"), _T("f2"), _T("f3"), _T("f4")};
  const bool testFieldsStorePos_values[] = {true, false, true, false};
  const bool testFieldsStoreOff_values[] = {true, false, false, true};
  const TCHAR* testTerms_values[] = {_T("this"), _T("is"), _T("a"), _T("test")};

  CL_NS(store)::MockRAMDirectory dir;
  std::string seg;
  FieldInfos *fieldInfos = NULL;
  const int TERM_FREQ = 3;

  struct TestToken {
    const TCHAR* text;
    int pos;
    int startOffset;
    int endOffset;
  };

  std::vector<const TCHAR*> testFields(testFields_values, testFields_values + sizeof(testFields_values) / sizeof(TCHAR*));
  std::vector<const TCHAR*> testTerms(testTerms_values, testTerms_values + sizeof(testTerms_values) / sizeof(TCHAR*));
  std::vector<bool> testFieldsStorePos(testFieldsStorePos_values, testFieldsStorePos_values + sizeof(testFieldsStorePos_values) / sizeof(bool));
  std::vector<bool> testFieldsStoreOff(testFieldsStoreOff_values, testFieldsStoreOff_values + sizeof(testFieldsStoreOff_values) / sizeof(bool));
  std::vector<const TestToken*> tokens;
  std::vector< std::vector<int> > positions(4);
  std::vector< std::vector<TermVectorOffsetInfo*> > offsets(4);

  class MyTokenStream : public TokenStream {
  private:
    std::vector<const TestToken*>::size_type tokenUpto;
  public:
    MyTokenStream() :
      tokenUpto(0)
    {
    }
    virtual Token* next(Token *token) {
      if (tokenUpto >= tokens.size())
        return NULL;
      else {
        if (token == NULL) {
          token = _CLNEW Token();
        }
        const TestToken* testToken = tokens[tokenUpto++];
        token->setText(testToken->text);
        if (tokenUpto > 1)
          token->setPositionIncrement(testToken->pos - tokens[tokenUpto-2]->pos);
        else
          token->setPositionIncrement(testToken->pos+1);
        token->setStartOffset(testToken->startOffset);
        token->setEndOffset(testToken->endOffset);
        return token;
      }
    }
    virtual void close() {
    }
 };

  class MyAnalyzer : public CL_NS(analysis)::Analyzer {
  public:
      virtual TokenStream* tokenStream(const TCHAR* fieldName, Reader* reader) {
        return _CLNEW MyTokenStream();
      }
  };

  class MyIndexWriter : public IndexWriter {
  public:
    MyIndexWriter(CL_NS(store)::Directory* d, CL_NS(analysis)::Analyzer* a, const bool create) :
      IndexWriter(d, a, create) {
    }
    virtual SegmentInfo* newestSegment() {
      return IndexWriter::newestSegment();
    }
  };

  struct MyTCharCompare :
    public std::binary_function<const TCHAR*, const TCHAR*, bool>
  {
    bool operator () (const TCHAR* v1, const TCHAR* v2) const {
      return _tcscmp(v1, v2) < 0;
    }
  };

  struct TestTokenCompare : 
    public std::binary_function<const TestToken*, const TestToken*, bool>
  {
    bool operator () (const TestToken* t1, const TestToken* t2) const {
      return t1->pos < t2->pos;
    }
  };

  void setUp() {

    tokens.clear();
    std::sort(testTerms.begin(), testTerms.end(), MyTCharCompare());
    int tokenUpto = 0;
    for (std::vector<const TCHAR*>::size_type i = 0; i < testTerms.size(); i++) {
      positions[i] = std::vector<int>(TERM_FREQ);
      offsets[i] = std::vector<TermVectorOffsetInfo*>(TERM_FREQ);
      // first position must be 0
      for (int j = 0; j < TERM_FREQ; j++) {
        // positions are always sorted in increasing order
        positions[i][j] = (int) (j * 10 + (rand() % 10));
        // offsets are always sorted in increasing order
        offsets[i][j] = _CLNEW TermVectorOffsetInfo(j * 10, j * 10 + _tcslen(testTerms[i]));
        TestToken* token = _CLNEW TestToken();
        tokens.push_back(token);
        tokenUpto++;
        token->text = testTerms[i];
        token->pos = positions[i][j];
        token->startOffset = offsets[i][j]->getStartOffset();
        token->endOffset = offsets[i][j]->getEndOffset();
      }
    }

    std::sort(tokens.begin(), tokens.end(), TestTokenCompare());

    MyAnalyzer analyzer;
    MyIndexWriter writer(&dir, &analyzer, true);
    writer.setUseCompoundFile(false);
    Document* doc = _CLNEW Document();
    for(std::vector<const TCHAR*>::size_type i=0;i<testFields.size();i++) {
      Field::TermVector tv;
      if (testFieldsStorePos[i] && testFieldsStoreOff[i])
        tv = Field::TERMVECTOR_WITH_POSITIONS_OFFSETS;
      else if (testFieldsStorePos[i] && !testFieldsStoreOff[i])
        tv = Field::TERMVECTOR_WITH_POSITIONS;
      else if (!testFieldsStorePos[i] && testFieldsStoreOff[i])
        tv = Field::TERMVECTOR_WITH_OFFSETS;
      else
        tv = Field::TERMVECTOR_YES;
      doc->add(* _CLNEW Field(testFields[i], _T(""), Field::STORE_NO | Field::INDEX_TOKENIZED | tv));
    }

    //Create 5 documents for testing, they all have the same
    //terms
    for(int j=0;j<5;j++)
      writer.addDocument(doc);
    _CLLDELETE(doc);
    writer.flush();
    seg = writer.newestSegment()->name;
    writer.close();

    std::string tmp = seg;
    tmp.append(".");
    tmp.append(IndexFileNames::FIELD_INFOS_EXTENSION);
    fieldInfos = _CLNEW FieldInfos(&dir, tmp.c_str());
  }


  void test(CuTest* tc) {
    //Check to see the files were created properly in setup
    std::string tmp = seg;
    tmp.append(".");
    tmp.append(IndexFileNames::VECTORS_DOCUMENTS_EXTENSION);
    CuAssertTrue(tc, dir.fileExists(tmp.c_str()), _T("Missing file!"));

    tmp = seg;
    tmp.append(".");
    tmp.append(IndexFileNames::VECTORS_INDEX_EXTENSION);
    CuAssertTrue(tc, dir.fileExists(tmp.c_str()), _T("Missing file!"));
  }

  void testTermVectorsReader(CuTest* tc) {
    TermVectorsReader reader(&dir, seg.c_str(), fieldInfos);
    for (int j = 0; j < 5; j++) {
      TermFreqVector* vector = reader.get(j, testFields[0]);
      CuAssertTrue(tc, vector != NULL, _T("Expected term frequency vector!"));
      const ArrayBase<const TCHAR*>* terms = vector->getTerms();
      CuAssertTrue(tc, terms != NULL, _T("Array of terms expected!"));
      CuAssertTrue(tc, terms->length == testTerms.size());
      for (int i = 0; i < terms->length; i++) {
        const TCHAR* term = (*terms)[i];
        CuAssertStrEquals(tc, _T(""), testTerms[i], (TCHAR*)term, false);
      }
    }
  }

  void testPositionReader(CuTest* tc) {
    TermVectorsReader reader(&dir, seg.c_str(), fieldInfos);
    TermPositionVector* vector;
    const ArrayBase<const TCHAR*>* terms;
    vector = dynamic_cast<TermPositionVector*>(reader.get(0, testFields[0]));
    CuAssertTrue(tc, vector != NULL, _T("Term position vector expected!"));
    terms = vector->getTerms();
    CuAssertTrue(tc, terms != NULL, _T("Terms expected!"));
    CuAssertTrue(tc, terms->length == testTerms.size(), _T("Unexpected number of terms!"));
    for (int i = 0; i < terms->length; i++) {
      const TCHAR* term = (*terms)[i];
      CuAssertStrEquals(tc, _T(""), testTerms[i], (TCHAR*)term, false);
      const ArrayBase<int32_t>* termPositions = vector->getTermPositions(i);
      CuAssertTrue(tc, termPositions != NULL, _T("Term positions expected!"));
      CuAssertTrue(tc, termPositions->length == positions[i].size(), _T("Unexpected number of term positions!"));
      for (int j = 0; j < termPositions->length; j++) {
        int position = (*termPositions)[j];
        CuAssertTrue(tc, position == positions[i][j], _T("Postion not equal!"));
      }
      const ArrayBase<TermVectorOffsetInfo*>* termOffset = vector->getOffsets(i);
      CuAssertTrue(tc, termOffset != NULL, _T("Term vector offset info expected!"));
      CuAssertTrue(tc, termOffset->length == offsets[i].size(), _T("Unexpected length of term positions!"));
      for (int j = 0; j < termOffset->length; j++) {
        TermVectorOffsetInfo* termVectorOffsetInfo = (*termOffset)[j];
        CuAssertTrue(tc, termVectorOffsetInfo->equals(offsets[i][j]), _T("Term vector offset info not equal!"));
      }
    }

    TermFreqVector* freqVector = reader.get(0, testFields[1]); //no pos, no offset
    CuAssertTrue(tc, freqVector != NULL, _T("Term frequency vector expected!"));
    CuAssertTrue(tc, dynamic_cast<TermPositionVector*>(freqVector) == NULL, _T("Unepexcted term position vector!"));
    terms = freqVector->getTerms();
    CuAssertTrue(tc, terms != NULL, _T("Terms expected!"));
    CuAssertTrue(tc, terms->length == testTerms.size(), _T("Unexpected length of term positions!"));
    for (int i = 0; i < terms->length; i++) {
      const TCHAR* term = (*terms)[i];
      CuAssertStrEquals(tc, _T(""), testTerms[i], (TCHAR*)term, false);
    }
  }

  class DocNumAwareMapper : public TermVectorMapper {
  private:

    int documentNumber;

  public:

    DocNumAwareMapper() : documentNumber(-1) {
    }

    virtual ~DocNumAwareMapper() {
    }

    virtual void setExpectations(const TCHAR* _field, const int32_t numTerms, const bool storeOffsets, const bool storePositions) {
      if (documentNumber == -1) {
        _CLTHROWA(CL_ERR_Runtime, "Documentnumber should be set at this point!");
      }
    }

    virtual void map(const TCHAR* term, const int32_t termLen, const int32_t frequency, CL_NS(util)::ArrayBase<TermVectorOffsetInfo*>* offsets, CL_NS(util)::ArrayBase<int32_t>* positions) {
      if (documentNumber == -1) {
        _CLTHROWA(CL_ERR_Runtime, "Documentnumber should be set at this point!");
      }
    }

    int getDocumentNumber() const {
      return documentNumber;
    }

    virtual void setDocumentNumber(const int32_t documentNumber) {
      this->documentNumber = documentNumber;
    }
  };

  void testOffsetReader(CuTest* tc) {
    TermVectorsReader reader(&dir, seg.c_str(), fieldInfos);
    TermPositionVector* vector = dynamic_cast<TermPositionVector*>(reader.get(0, testFields[0]));
    CuAssertTrue(tc, vector != NULL, _T("Term position vector expected!"));
    const CL_NS(util)::ArrayBase<const TCHAR*>* terms = vector->getTerms();
    CuAssertTrue(tc, terms != NULL, _T("Terms expected"));
    CuAssertTrue(tc, terms->length == testTerms.size(), _T("Unexpected number of terms!"));
    for (int i = 0; i < terms->length; i++) {
      const TCHAR* term = (*terms)[i];
      CuAssertStrEquals(tc, _T(""), testTerms[i], (TCHAR*)term, false);
      const ArrayBase<int32_t>* termPositions = vector->getTermPositions(i);
      CuAssertTrue(tc, termPositions != NULL, _T("Term positions expected!"));
      CuAssertTrue(tc, termPositions->length == positions[i].size());
      for (int j = 0; j < termPositions->length; j++) {
        int position = (*termPositions)[j];
        CuAssertTrue(tc, position == positions[i][j], _T("Unexpected position!"));
      }
      const ArrayBase<TermVectorOffsetInfo*>* termOffset = vector->getOffsets(i);
      CuAssertTrue(tc, termOffset != NULL, _T("Term vector offset info expected!"));
      CuAssertTrue(tc, termOffset->length == offsets[i].size(), _T("Unexpected number of term positions!"));
      for (int j = 0; j < termOffset->length; j++) {
        TermVectorOffsetInfo* termVectorOffsetInfo = (*termOffset)[j];
        CuAssertTrue(tc, termVectorOffsetInfo->equals(offsets[i][j]), _T("Term vector offset info not equal!"));
      }
    }
  }

  //void testMapper(CuTest* tc) {
  //  TermVectorsReader reader(&dir, seg, fieldInfos);
  //  SortedTermVectorMapper mapper = new SortedTermVectorMapper(new TermVectorEntryFreqSortedComparator());
  //  reader.get(0, mapper);
  //  SortedSet set = mapper.getTermVectorEntrySet();
  //  CuAssertTrue(tc, set != NULL, "set is null and it shouldn't be");
  //  //three fields, 4 terms, all terms are the same
  //  CuAssertTrue(set.size() == 4, _T("set Size: " + set.size() + " is not: 4");
  //  //Check offsets and positions
  //  for (Iterator iterator = set.iterator(); iterator.hasNext();) {
  //    TermVectorEntry* tve = (TermVectorEntry) iterator.next();
  //    CuAssertTrue(tc, tve != NULL, _T("tve is null and it shouldn't be"));
  //    CuAssertTrue(tc, tve->getOffsets() != NULL, _T("tve.getOffsets() is null and it shouldn't be"));
  //    CuAssertTrue(tc, tve->getPositions() != NULL, _T("tve.getPositions() is null and it shouldn't be"));
  //  }

  //  mapper = new SortedTermVectorMapper(new TermVectorEntryFreqSortedComparator());
  //  reader.get(1, mapper);
  //  set = mapper.getTermVectorEntrySet();
  //  assertTrue("set is null and it shouldn't be", set != null);
  //  //three fields, 4 terms, all terms are the same
  //  assertTrue("set Size: " + set.size() + " is not: " + 4, set.size() == 4);
  //  //Should have offsets and positions b/c we are munging all the fields together
  //  for (Iterator iterator = set.iterator(); iterator.hasNext();) {
  //    TermVectorEntry tve = (TermVectorEntry) iterator.next();
  //    assertTrue("tve is null and it shouldn't be", tve != null);
  //    assertTrue("tve.getOffsets() is null and it shouldn't be", tve.getOffsets() != null);
  //    assertTrue("tve.getPositions() is null and it shouldn't be", tve.getPositions() != null);

  //  }


  //  FieldSortedTermVectorMapper fsMapper = new FieldSortedTermVectorMapper(new TermVectorEntryFreqSortedComparator());
  //  reader.get(0, fsMapper);
  //  Map map = fsMapper.getFieldToTerms();
  //  assertTrue("map Size: " + map.size() + " is not: " + testFields.length, map.size() == testFields.length);
  //  for (Iterator iterator = map.entrySet().iterator(); iterator.hasNext();) {
  //    Map.Entry entry = (Map.Entry) iterator.next();
  //    SortedSet sortedSet = (SortedSet) entry.getValue();
  //    assertTrue("sortedSet Size: " + sortedSet.size() + " is not: " + 4, sortedSet.size() == 4);
  //    for (Iterator inner = sortedSet.iterator(); inner.hasNext();) {
  //      TermVectorEntry tve = (TermVectorEntry) inner.next();
  //      assertTrue("tve is null and it shouldn't be", tve != null);
  //      //Check offsets and positions.
  //      assertTrue("tve is null and it shouldn't be", tve != null);
  //      String field = tve.getField();
  //      if (field.equals(testFields[0])) {
  //        //should have offsets

  //        assertTrue("tve.getOffsets() is null and it shouldn't be", tve.getOffsets() != null);
  //        assertTrue("tve.getPositions() is null and it shouldn't be", tve.getPositions() != null);
  //      }
  //      else if (field.equals(testFields[1])) {
  //        //should not have offsets

  //        assertTrue("tve.getOffsets() is not null and it shouldn't be", tve.getOffsets() == null);
  //        assertTrue("tve.getPositions() is not null and it shouldn't be", tve.getPositions() == null);
  //      }
  //    }
  //  }
  //  //Try mapper that ignores offs and positions
  //  fsMapper = new FieldSortedTermVectorMapper(true, true, new TermVectorEntryFreqSortedComparator());
  //  reader.get(0, fsMapper);
  //  map = fsMapper.getFieldToTerms();
  //  assertTrue("map Size: " + map.size() + " is not: " + testFields.length, map.size() == testFields.length);
  //  for (Iterator iterator = map.entrySet().iterator(); iterator.hasNext();) {
  //    Map.Entry entry = (Map.Entry) iterator.next();
  //    SortedSet sortedSet = (SortedSet) entry.getValue();
  //    assertTrue("sortedSet Size: " + sortedSet.size() + " is not: " + 4, sortedSet.size() == 4);
  //    for (Iterator inner = sortedSet.iterator(); inner.hasNext();) {
  //      TermVectorEntry tve = (TermVectorEntry) inner.next();
  //      assertTrue("tve is null and it shouldn't be", tve != null);
  //      //Check offsets and positions.
  //      assertTrue("tve is null and it shouldn't be", tve != null);
  //      String field = tve.getField();
  //      if (field.equals(testFields[0])) {
  //        //should have offsets

  //        assertTrue("tve.getOffsets() is null and it shouldn't be", tve.getOffsets() == null);
  //        assertTrue("tve.getPositions() is null and it shouldn't be", tve.getPositions() == null);
  //      }
  //      else if (field.equals(testFields[1])) {
  //        //should not have offsets

  //        assertTrue("tve.getOffsets() is not null and it shouldn't be", tve.getOffsets() == null);
  //        assertTrue("tve.getPositions() is not null and it shouldn't be", tve.getPositions() == null);
  //      }
  //    }
  //  }

  //  // test setDocumentNumber()
  //  IndexReader ir = IndexReader.open(&dir);
  //  DocNumAwareMapper docNumAwareMapper = new DocNumAwareMapper();
  //  assertEquals(-1, docNumAwareMapper.getDocumentNumber());

  //  ir.getTermFreqVector(0, docNumAwareMapper);
  //  assertEquals(0, docNumAwareMapper.getDocumentNumber());
  //  docNumAwareMapper.setDocumentNumber(-1);

  //  ir.getTermFreqVector(1, docNumAwareMapper);
  //  assertEquals(1, docNumAwareMapper.getDocumentNumber());
  //  docNumAwareMapper.setDocumentNumber(-1);

  //  ir.getTermFreqVector(0, "f1", docNumAwareMapper);
  //  assertEquals(0, docNumAwareMapper.getDocumentNumber());
  //  docNumAwareMapper.setDocumentNumber(-1);

  //  ir.getTermFreqVector(1, "f2", docNumAwareMapper);
  //  assertEquals(1, docNumAwareMapper.getDocumentNumber());
  //  docNumAwareMapper.setDocumentNumber(-1);

  //  ir.getTermFreqVector(0, "f1", docNumAwareMapper);
  //  assertEquals(0, docNumAwareMapper.getDocumentNumber());

  //  ir.close();

  //}

  /**
   * Make sure exceptions and bad params are handled appropriately
   */
  void testBadParams(CuTest* tc) {
    try {
      TermVectorsReader reader(&dir, seg.c_str(), fieldInfos);
      //Bad document number, good field number
      reader.get(50, testFields[0]);
      CuFail(tc, _T("Expected an IO exception!"));
    } catch (CLuceneError& e) {
      if (e.number() != CL_ERR_IO) {
        CuFail(tc, e.twhat());
      }
    }
    try {
      TermVectorsReader reader(&dir, seg.c_str(), fieldInfos);
      //Bad document number, no field
      reader.get(50);
      CuFail(tc, _T(""));
    } catch (CLuceneError& e) {
      if (e.number() != CL_ERR_IO) {
        CuFail(tc, e.twhat());
      }
    }
    try {
      TermVectorsReader reader(&dir, seg.c_str(), fieldInfos);
      //good document number, bad field number
      TermFreqVector* vector = reader.get(0, _T("f50"));
      CuAssertTrue(tc, vector == NULL, _T(""));
    } catch (CLuceneError& e) {
      CuFail(tc, e.twhat());
    }
  }


CuSuite *testTermVectorsReader(void) {
  CuSuite *suite = CuSuiteNew(_T("CLucene TermVectorsReader Test"));

  setUp();

  SUITE_ADD_TEST(suite, test);
  SUITE_ADD_TEST(suite, testTermVectorsReader);
  SUITE_ADD_TEST(suite, testPositionReader);
  SUITE_ADD_TEST(suite, testOffsetReader);
  //SUITE_ADD_TEST(suite, testMapper);
  SUITE_ADD_TEST(suite, testBadParams);

  return suite;
}
