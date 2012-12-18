/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "ConstantScoreQuery.h"

#include "SearchHeader.h"
#include "Scorer.h"
#include "RangeFilter.h"
#include "Similarity.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/util/BitSet.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/_StringIntern.h"
#include "CLucene/util/Misc.h"

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_DEF(search)

class ConstantScorer : public Scorer {
    BitSet* bits;
    const float_t theScore;
    int32_t _doc;

public:
    ConstantScorer(Similarity* similarity, IndexReader* reader, Weight* w, Filter* filter) : Scorer(similarity),
        bits(filter->bits(reader)), theScore(w->getValue()), _doc(-1)
    {
    }
    virtual ~ConstantScorer() {
        _CLLDELETE(bits);
    }

    bool next() {
        _doc = bits->nextSetBit(_doc+1);
        return _doc >= 0;
    }

    int32_t doc() const {
        return _doc;
    }

    float_t score() {
        return theScore;
    }

    bool skipTo(int32_t target) {
        _doc = bits->nextSetBit(target);
        return _doc >= 0;
    }

    Explanation* explain(int32_t /*doc*/) {
        _CLTHROWA(CL_ERR_UnsupportedOperation, "Unsupported operation at ConstantScoreQuery::explain");
    }

    TCHAR* toString(){
        return STRDUP_TtoT(_T("ConstantScorer"));
    }

    friend class ConstantWeight;
};

class ConstantWeight : public Weight {
private:
    Similarity* similarity;
    float_t queryNorm;
    float_t queryWeight;
    const ConstantScoreQuery* parentQuery;

public:
    ConstantWeight(ConstantScoreQuery* enclosingInstance, Searcher* searcher) :
          similarity(enclosingInstance->getSimilarity(searcher)),
          queryNorm(0), queryWeight(0),
          parentQuery(enclosingInstance)
    {
    }
    virtual ~ConstantWeight(){}

    Query* getQuery() {
        return (Query*)parentQuery;
    }

    float_t getValue() {
        return queryWeight;
    }

    float_t sumOfSquaredWeights() {
        queryWeight = parentQuery->getBoost();
        return queryWeight * queryWeight;
    }

    void normalize(float_t norm) {
        this->queryNorm = norm;
        queryWeight *= this->queryNorm;
    }

    Scorer* scorer(IndexReader* reader) {
        return _CLNEW ConstantScorer(similarity, reader, this, parentQuery->filter);
    }

    Explanation* explain(IndexReader* reader, int32_t doc) {
        ConstantScorer* cs = (ConstantScorer*)scorer(reader);
        bool exists = cs->bits->get(doc);
        _CLDELETE(cs);

        ComplexExplanation* result = _CLNEW ComplexExplanation();

        if (exists) {
            StringBuffer buf(100);
            buf.append(_T("ConstantScoreQuery("));

            TCHAR* tmp = parentQuery->filter->toString();
            buf.append(tmp);
            _CLDELETE_LCARRAY(tmp);

            buf.append(_T("), product of:"));

            result->setDescription(buf.getBuffer());
            result->setValue(queryWeight);
            result->setMatch(true);
            result->addDetail(_CLNEW Explanation(parentQuery->getBoost(), _T("boost")));
            result->addDetail(_CLNEW Explanation(queryNorm, _T("queryNorm")));
        } else {
            StringBuffer buf(100);
            buf.append(_T("ConstantScoreQuery("));

            TCHAR* tmp = parentQuery->filter->toString();
            buf.append(tmp);
            _CLDELETE_LCARRAY(tmp);

            buf.append(_T(") doesn't match id "));
            buf.appendInt(doc);

            result->setDescription(buf.getBuffer());
            result->setValue(0);
            result->setMatch(true);
        }

        _CLLDELETE(cs);
        return result;
    }
};

ConstantScoreQuery::ConstantScoreQuery(Filter* _filter) : filter(_filter) {
}

ConstantScoreQuery::~ConstantScoreQuery() {
    _CLLDELETE(filter);
}

Filter* ConstantScoreQuery::getFilter() const {
    return filter;
}

Query* ConstantScoreQuery::rewrite(IndexReader* reader) {
    return this;
}

void ConstantScoreQuery::extractTerms( TermSet * termset ) const
{
    // OK to not add any terms when used for MultiSearcher,
    // but may not be OK for highlighting
}

Weight* ConstantScoreQuery::_createWeight(Searcher* searcher) {
    return _CLNEW /*ConstantScoreQuery::*/ConstantWeight(this, searcher);
}

TCHAR* ConstantScoreQuery::toString(const TCHAR* /*field*/) const
{
    StringBuffer buf;
    buf.append(_T("ConstantScore("));
    TCHAR* tmp = filter->toString();
    buf.append(tmp);
    _CLLDELETE(tmp);
    buf.appendBoost(getBoost());
    buf.appendChar(_T(')'));
    return buf.giveBuffer();
}

// TODO: Filter is missing an equals() function, hence this equals() is incomplete
bool ConstantScoreQuery::equals(Query* o) const {
    if (this == o) return true;
    if (!(o->instanceOf("ConstantScoreQuery"))) return false;
    ConstantScoreQuery* other = (ConstantScoreQuery*)o;
    return this->getBoost()==other->getBoost()
        /*&& filter->equals(other->filter)*/;
}

// TODO: Filter is missing hashCode()
size_t ConstantScoreQuery::hashCode() const {
    // Simple add is OK since no existing filter hashcode has a float component.
    //return filter->hashCode() + FloatToIntBits(getBoost());
    return 0;
}

ConstantScoreQuery::ConstantScoreQuery( const ConstantScoreQuery& copy ) : filter(copy.getFilter()->clone())
{
}

const char* ConstantScoreQuery::getObjectName() const { return "ConstantScoreQuery"; }
Query* ConstantScoreQuery::clone() const{
    return _CLNEW ConstantScoreQuery(*this);
}

ConstantScoreRangeQuery::ConstantScoreRangeQuery( const ConstantScoreRangeQuery& copy ):
    fieldName(const_cast<TCHAR*>(CLStringIntern::intern(copy.fieldName))),
    lowerVal(STRDUP_TtoT(copy.lowerVal)), upperVal(STRDUP_TtoT(copy.upperVal)),
    includeLower(copy.includeLower),includeUpper(copy.includeUpper)
{
}

ConstantScoreRangeQuery::ConstantScoreRangeQuery(const TCHAR* _fieldName, const TCHAR* _lowerVal, const TCHAR* _upperVal,
                        bool _includeLower, bool _includeUpper) : fieldName(NULL), lowerVal(NULL), upperVal(NULL)
{
    // do a little bit of normalization...
    // open ended range queries should always be inclusive.
    if (_lowerVal==NULL) {
        _includeLower=true;
    } else if (_includeLower && _tcscmp(_lowerVal, _T(""))==0) {
        _lowerVal=NULL;
    }
    if (_upperVal==NULL) {
        _includeUpper=true;
    }

    this->fieldName = const_cast<TCHAR*>(CLStringIntern::intern(_fieldName));  // intern it, just like terms...
    if (_lowerVal != NULL)
        this->lowerVal = STRDUP_TtoT(_lowerVal);
    if (_upperVal != NULL)
        this->upperVal = STRDUP_TtoT(_upperVal);
    this->includeLower = _includeLower;
    this->includeUpper = _includeUpper;
}
ConstantScoreRangeQuery::~ConstantScoreRangeQuery(){
    _CLDELETE_LCARRAY(lowerVal);
    _CLDELETE_LCARRAY(upperVal);
    CLStringIntern::unintern(this->fieldName);
}

Query* ConstantScoreRangeQuery::rewrite(CL_NS(index)::IndexReader* reader) {
    // Map to RangeFilter semantics which are slightly different...
    const TCHAR* lowerSafe = lowerVal!=NULL?lowerVal:_T("");
    RangeFilter* rangeFilt = _CLNEW RangeFilter(fieldName,
        lowerSafe,
        upperVal,
        (_tcscmp(lowerSafe, _T(""))==0)?false:includeLower,
        upperVal==NULL?false:includeUpper);
    Query* q = _CLNEW ConstantScoreQuery(rangeFilt);
    q->setBoost(getBoost());
    return q;
}

TCHAR* ConstantScoreRangeQuery::toString(const TCHAR* field) const
{
    StringBuffer buffer(30);
    if (_tcscmp(getField(), field) != 0)
    {
        buffer.append(getField());
        buffer.appendChar(_T(':'));
    }
    buffer.appendChar(includeLower ? _T('[') : _T('{'));
    buffer.append(lowerVal != NULL ? lowerVal : _T("*"));
    buffer.append(_T(" TO "));
    buffer.append(upperVal != NULL ? upperVal : _T("*"));
    buffer.appendChar(includeUpper ? _T(']') : _T('}'));
    buffer.appendBoost(getBoost());
    return buffer.giveBuffer();
}

bool ConstantScoreRangeQuery::equals(Query* o) const {
    if (this == o) return true;
    if (!(o->instanceOf("ConstantScoreRangeQuery"))) return false;
    ConstantScoreRangeQuery* other = (ConstantScoreRangeQuery*) o;

    if (this->fieldName != other->fieldName  // interned comparison
        || this->includeLower != other->includeLower
        || this->includeUpper != other->includeUpper
        ) { return false; }
    if (this->lowerVal != NULL ? _tcscmp(this->lowerVal, other->lowerVal) != 0 : other->lowerVal != NULL) return false;
    if (this->upperVal != NULL ? _tcscmp(this->upperVal, other->upperVal) != 0 : other->upperVal != NULL) return false;
    return this->getBoost() == other->getBoost();
}

// TODO: Complete this
size_t ConstantScoreRangeQuery::hashCode() const
{
    int32_t h = Similarity::floatToByte( getBoost() ) ^ Misc::thashCode( fieldName );
    // hashCode of "" is 0, so don't use that for null...

    h ^= ( lowerVal != NULL ) ? Misc::thashCode( lowerVal ) : 0x965a965a;
    // don't just XOR upperVal with out mixing either it or h, as it will cancel
    // out lowerVal if they are equal.

    h ^= (h << 17) | (h >> 16);  // a reversible (one to one) 32 bit mapping mix
    h ^= (upperVal != NULL) ? Misc::thashCode( upperVal ) : 0x5a695a69;
    h ^= (includeLower ? 0x665599aa : 0) ^ (includeUpper ? 0x99aa5566 : 0);

    return h;
}

const char* ConstantScoreRangeQuery::getObjectName() const { return "ConstantScoreRangeQuery"; }
Query* ConstantScoreRangeQuery::clone() const{
    return _CLNEW ConstantScoreRangeQuery(*this);
}

CL_NS_END

