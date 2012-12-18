#include "CLucene/_ApiHeader.h"
#include "IndexWriter4Test.h"
#include "CLucene/index/_SegmentInfos.h"

CL_NS_DEF(index)

IndexWriter4Test::IndexWriter4Test(CL_NS(store)::Directory* d, CL_NS(analysis)::Analyzer* a, const bool create) :
    IndexWriter(d, a, create)
{
}

IndexWriter4Test::IndexWriter4Test(CL_NS(store)::Directory* d, bool autoCommit, CL_NS(analysis)::Analyzer* a, const bool create) :
    IndexWriter(d, autoCommit, a, create)
{
}

// for test purpose 
int32_t IndexWriter4Test::getDocCount(int32_t i) {
    return IndexWriter::getDocCount(i);
}

// for test purpose
int32_t IndexWriter4Test::getNumBufferedDocuments(){
    return IndexWriter::getNumBufferedDocuments();
}

// for test purpose
int32_t IndexWriter4Test::getSegmentCount(){
    return IndexWriter::getSegmentCount();
}

int32_t IndexWriter4Test::getBufferedDeleteTermsSize() {
    return IndexWriter::getBufferedDeleteTermsSize();
}

int32_t IndexWriter4Test::getNumBufferedDeleteTerms() {
    return IndexWriter::getNumBufferedDeleteTerms();
}

SegmentInfo* IndexWriter4Test::newestSegment() {
    return IndexWriter::newestSegment();
}

CL_NS_END
