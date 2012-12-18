/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CLucene/index/Term.h"
#include "CLucene/index/IndexReader.h"
#include "Similarity.h"
#include "FuzzyQuery.h"
#include "BooleanQuery.h"
#include "BooleanClause.h"
#include "TermQuery.h"

#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/PriorityQueue.h"

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_DEF(search)


/** Finds and returns the smallest of three integers
 *	precondition: Must define int32_t __t for temporary storage and result
 */
#define min3(a, b, c) __t = (a < b) ? a : b; __t = (__t < c) ? __t : c;


	FuzzyTermEnum::FuzzyTermEnum(IndexReader* reader, Term* term, float_t minSimilarity, size_t _prefixLength):
		FilteredTermEnum(),d(NULL),dLen(0),_similarity(0),_endEnum(false),searchTerm(_CL_POINTER(term)),
		text(NULL),textLen(0),prefix(NULL)/* ISH: was STRDUP_TtoT(LUCENE_BLANK_STRING)*/,prefixLength(0),
		minimumSimilarity(minSimilarity)
	{
		CND_PRECONDITION(term != NULL,"term is NULL");

		if (minSimilarity >= 1.0f)
			_CLTHROWA(CL_ERR_IllegalArgument,"minimumSimilarity cannot be greater than or equal to 1");
		else if (minSimilarity < 0.0f)
			_CLTHROWA(CL_ERR_IllegalArgument,"minimumSimilarity cannot be less than 0");

		scale_factor = 1.0f / (1.0f - minimumSimilarity); // only now we are safe from a division by zero
		//TODO: this.field = searchTerm.field();

		//The prefix could be longer than the word.
		//It's kind of silly though.  It means we must match the entire word.
		const size_t fullSearchTermLength = searchTerm->textLength();
		const size_t realPrefixLength = _prefixLength > fullSearchTermLength ? fullSearchTermLength : _prefixLength;

		text = STRDUP_TtoT(searchTerm->text() + realPrefixLength);
		textLen = fullSearchTermLength - realPrefixLength;

		prefix = _CL_NEWARRAY(TCHAR,realPrefixLength+1);
		_tcsncpy(prefix, searchTerm->text(), realPrefixLength);
		prefix[realPrefixLength]='\0';
        prefixLength = realPrefixLength;

		initializeMaxDistances();

		Term* trm = _CLNEW Term(searchTerm->field(), prefix); // _CLNEW Term(term, prefix); -- not intern'd?
		setEnum(reader->terms(trm));
		_CLLDECDELETE(trm);


		/* LEGACY:
		//Initialize e to NULL
		e          = NULL;
		eWidth     = 0;
		eHeight    = 0;

		if(prefixLength > 0 && prefixLength < textLen){
		this->prefixLength = prefixLength;

		prefix = _CL_NEWARRAY(TCHAR,prefixLength+1);
		_tcsncpy(prefix,text,prefixLength);
		prefix[prefixLength]='\0';

		textLen = prefixLength;
		text[textLen]='\0';
		}
		*/
	}

	FuzzyTermEnum::~FuzzyTermEnum(){
		close();
	}

	const char* FuzzyTermEnum::getObjectName() const{ return getClassName(); }
	const char* FuzzyTermEnum::getClassName(){ return "FuzzyTermEnum"; }

	bool FuzzyTermEnum::endEnum() {
		return _endEnum;
	}

	void FuzzyTermEnum::close(){

		FilteredTermEnum::close();

		//Finalize the searchTerm
		_CLDECDELETE(searchTerm);

		free(d);
		d=NULL;

		_CLDELETE_CARRAY(text);

		_CLDELETE_CARRAY(prefix);
	}

	bool FuzzyTermEnum::termCompare(Term* term) {
		//Func - Compares term with the searchTerm using the Levenshtein distance.
		//Pre  - term is NULL or term points to a Term
		//Post - if pre(term) is NULL then false is returned otherwise
		//       if the distance of the current term in the enumeration is bigger than the FUZZY_THRESHOLD
		//       then true is returned

		if (term == NULL){
			return false;  //Note that endEnum is not set to true!
		}

		const TCHAR* termText = term->text();
		const size_t termTextLen = term->textLength();

		//Check if the field name of searchTerm of term match
		//(we can use == because fields are interned)
		if ( searchTerm->field() == term->field() &&
			(prefixLength==0 || _tcsncmp(termText,prefix,prefixLength)==0 )) {

				const TCHAR* target = termText+prefixLength;
				const size_t targetLen = termTextLen-prefixLength;
				_similarity = similarity(target, targetLen);
				return (_similarity > minimumSimilarity);
		}
		_endEnum = true;
		return false;
	}

	float_t FuzzyTermEnum::difference() {
		return (float_t)((_similarity - minimumSimilarity) * scale_factor );
	}

	// TODO: had synchronized in definition
	float_t FuzzyTermEnum::similarity(const TCHAR* target, const size_t m) {
		const size_t n = textLen; // TODO: remove after replacing n with textLen
		if (n == 0)  {
			//we don't have anything to compare.  That means if we just add
			//the letters for m we get the new word
			return prefixLength == 0 ? 0.0f : 1.0f - ((float_t) m / prefixLength);
		}
		if (m == 0) {
			return prefixLength == 0 ? 0.0f : 1.0f - ((float_t) n / prefixLength);
		}

		const uint32_t maxDistance = getMaxDistance(m);

		if ( maxDistance < (uint32_t)(abs((int32_t)(m-n))) ) {
			//just adding the characters of m to n or vice-versa results in
			//too many edits
			//for example "pre" length is 3 and "prefixes" length is 8.  We can see that
			//given this optimal circumstance, the edit distance cannot be less than 5.
			//which is 8-3 or more precisesly Math.abs(3-8).
			//if our maximum edit distance is 4, then we can discard this word
			//without looking at it.
			return 0.0f;
		}

		//let's make sure we have enough room in our array to do the distance calculations.
		//Check if the array must be reallocated because it is too small or does not exist
		size_t dWidth  = n+1;
		size_t dHeight = m+1;
    if (d == NULL){
      dLen = dWidth*dHeight;
			d = (int32_t*)(malloc(sizeof(int32_t)*dLen));
		} else if (dLen < dWidth*dHeight) {
      dLen = dWidth*dHeight;
			d = (int32_t*)(realloc(d, sizeof(int32_t)*dLen));
		}
    memset(d,0,dLen);

  	size_t i;     // iterates through the source string
		size_t j;     // iterates through the target string

		// init matrix d
		for (i = 0; i <= n; i++){
			d[i + (0*dWidth)] = i;
		}
		for (j = 0; j <= m; j++){
			d[0 + (j*dWidth)] = j;
		}

		int32_t __t; //temporary variable for min3

		// start computing edit distance
		TCHAR s_i; // ith character of s
		for (i = 1; i <= n; i++) {
			size_t bestPossibleEditDistance = m;
			s_i = text[i - 1];
			for (j = 1; j <= m; j++) {
				if (s_i != target[j-1]) {
					min3(d[i-1 + (j*dWidth)], d[i + ((j-1)*dWidth)], d[i-1 + ((j-1)*dWidth)]);
					d[i + (j*dWidth)] = __t+1;
				}
				else {
					min3(d[i-1 + (j*dWidth)]+1, d[i + ((j-1)*dWidth)]+1, d[i-1 + ((j-1)*dWidth)]);
					d[i + (j*dWidth)] = __t;
				}
				bestPossibleEditDistance = cl_min(bestPossibleEditDistance, d[i + (j*dWidth)]);
			}

			//After calculating row i, the best possible edit distance
			//can be found by finding the smallest value in a given column.
			//If the bestPossibleEditDistance is greater than the max distance, abort.

			if (i > maxDistance && bestPossibleEditDistance > maxDistance) {  //equal is okay, but not greater
				//the closest the target can be to the text is just too far away.
				//this target is leaving the party early.
				return 0.0f;
			}
		}

		// this will return less than 0.0 when the edit distance is
		// greater than the number of characters in the shorter word.
		// but this was the formula that was previously used in FuzzyTermEnum,
		// so it has not been changed (even though minimumSimilarity must be
		// greater than 0.0)
		return 1.0f - ((float_t)d[n + m*dWidth] / (float_t) (prefixLength + cl_min(n, m)));
	}

	int32_t FuzzyTermEnum::getMaxDistance(const size_t m) {
		return (m < LUCENE_TYPICAL_LONGEST_WORD_IN_INDEX) ? maxDistances[m] : calculateMaxDistance(m);
	}

	void FuzzyTermEnum::initializeMaxDistances() {
		for (int32_t i = 0; i < LUCENE_TYPICAL_LONGEST_WORD_IN_INDEX; i++) {
			maxDistances[i] = calculateMaxDistance(i);
		}
	}

	int32_t FuzzyTermEnum::calculateMaxDistance(const size_t m) const {
		return (int32_t) ((1-minimumSimilarity) * (cl_min(textLen, m) + prefixLength));
	}

  // TODO: Make ScoreTerm and ScoreTermQueue reside under FuzzyQuery
  class ScoreTerm {
  public:
	  Term* term;
	  float_t score;

	  ScoreTerm(Term* _term, float_t _score):term(_term),score(_score){
	  }
	  virtual ~ScoreTerm(){
          _CLLDECDELETE(term);
	  }
  };

  class ScoreTermQueue : public PriorityQueue<ScoreTerm*, CL_NS(util)::Deletor::Object<ScoreTerm> > {
  public:
	  ScoreTermQueue(int32_t size){
		  initialize(size, true);
	  }
	  virtual ~ScoreTermQueue(){
	  }

  protected:
	  bool lessThan(ScoreTerm* termA, ScoreTerm* termB) {
		  if (termA->score == termB->score)
			  return termA->term->compareTo(termB->term) > 0;
		  else
			  return termA->score < termB->score;
	  }
  };


  FuzzyQuery::FuzzyQuery(Term* term, float_t _minimumSimilarity, size_t _prefixLength):
    MultiTermQuery(term),
    minimumSimilarity(_minimumSimilarity),
    prefixLength(_prefixLength)
  {
	  if ( minimumSimilarity < 0 )
		  minimumSimilarity = defaultMinSimilarity;

	  CND_PRECONDITION(term != NULL,"term is NULL");

	  if (minimumSimilarity >= 1.0f)
		  _CLTHROWA(CL_ERR_IllegalArgument,"minimumSimilarity >= 1");
	  else if (minimumSimilarity < 0.0f)
		  _CLTHROWA(CL_ERR_IllegalArgument,"minimumSimilarity < 0");
  }

  float_t FuzzyQuery::defaultMinSimilarity = 0.5f;
  int32_t FuzzyQuery::defaultPrefixLength = 0;

  FuzzyQuery::~FuzzyQuery(){
  }

  float_t FuzzyQuery::getMinSimilarity() const {
    return minimumSimilarity;
  }

  size_t FuzzyQuery::getPrefixLength() const {
    return prefixLength;
  }

  TCHAR* FuzzyQuery::toString(const TCHAR* field) const{
	  StringBuffer buffer(100); // TODO: Have a better estimation for the initial buffer length
	  Term* term = getTerm(false); // no need to increase ref count
	  if ( field==NULL || _tcscmp(term->field(),field)!=0 ) {
		  buffer.append(term->field());
		  buffer.appendChar( _T(':'));
	  }
	  buffer.append(term->text());
	  buffer.appendChar( _T('~') );
	  buffer.appendFloat(minimumSimilarity,1);
	  buffer.appendBoost(getBoost());
	  return buffer.giveBuffer();
  }

  const char* FuzzyQuery::getObjectName() const{
	  //Func - Returns the name of the query
	  //Pre  - true
	  //post - The string FuzzyQuery has been returned

	  return getClassName();
  }
  const char* FuzzyQuery::getClassName(){
	  //Func - Returns the name of the query
	  //Pre  - true
	  //post - The string FuzzyQuery has been returned

	  return "FuzzyQuery";
  }

  FuzzyQuery::FuzzyQuery(const FuzzyQuery& clone):
  MultiTermQuery(clone)
  {
	  this->minimumSimilarity = clone.getMinSimilarity();
	  this->prefixLength = clone.getPrefixLength();

	  //if(prefixLength < 0)
	  //	_CLTHROWA(CL_ERR_IllegalArgument,"prefixLength < 0");
	  //else
	  if(prefixLength >= clone.getTerm()->textLength())
		  _CLTHROWA(CL_ERR_IllegalArgument,"prefixLength >= term.textLength()");

  }

  Query* FuzzyQuery::clone() const{
	  return _CLNEW FuzzyQuery(*this);
  }
  size_t FuzzyQuery::hashCode() const{
	  //todo: we should give the query a seeding value... but
	  //need to do it for all hascode functions
	  // TODO: does not conform with JL
	  size_t val = Similarity::floatToByte(getBoost()) ^ getTerm()->hashCode();
	  val ^= Similarity::floatToByte(this->getMinSimilarity());
	  val ^= this->getPrefixLength();
	  return val;
  }
  bool FuzzyQuery::equals(Query* other) const{
	  if (this == other) return true;
	  if (!(other->instanceOf(FuzzyQuery::getClassName())))
		  return false;

	  FuzzyQuery* fq = static_cast<FuzzyQuery*>(other);
	  return (this->getBoost() == fq->getBoost())
		  && this->minimumSimilarity == fq->getMinSimilarity()
		  && this->prefixLength == fq->getPrefixLength()
		  && getTerm()->equals(fq->getTerm());
  }

  FilteredTermEnum* FuzzyQuery::getEnum(IndexReader* reader){
	  Term* term = getTerm(false);
	  FuzzyTermEnum* ret = _CLNEW FuzzyTermEnum(reader, term, minimumSimilarity, prefixLength);
	  return ret;
  }

  Query* FuzzyQuery::rewrite(IndexReader* reader) {
	  FilteredTermEnum* enumerator = getEnum(reader);
	  const size_t maxClauseCount = BooleanQuery::getMaxClauseCount();
	  ScoreTermQueue* stQueue = _CLNEW ScoreTermQueue(maxClauseCount);
	  ScoreTerm* reusableST = NULL;

	  try {
		  do {
			  float_t score = 0.0f;
			  Term* t = enumerator->term();
			  if (t != NULL) {
				  score = enumerator->difference();
				  if (reusableST == NULL) {
					  reusableST = _CLNEW ScoreTerm(t, score);
				  } else if (score >= reusableST->score) {
					  // reusableST holds the last "rejected" entry, so, if
					  // this new score is not better than that, there's no
					  // need to try inserting it
					  reusableST->score = score;
					  reusableST->term = t;
				  } else {
					  continue;
				  }

				  reusableST = stQueue->insertWithOverflow(reusableST);
			  }
		  } while (enumerator->next());
	  } _CLFINALLY({
		  enumerator->close();
		  _CLLDELETE(enumerator);
          //_CLLDELETE(reusableST);
	  });

	  BooleanQuery* query = _CLNEW BooleanQuery(true);
	  const size_t size = stQueue->size();
	  for(size_t i = 0; i < size; i++){
		  ScoreTerm* st = stQueue->pop();
		  TermQuery* tq = _CLNEW TermQuery(st->term);      // found a match
		  tq->setBoost(getBoost() * st->score); // set the boost
		  query->add(tq, true, BooleanClause::SHOULD);          // add to query
          _CLLDELETE(st);
	  }
	  _CLLDELETE(stQueue);

	  return query;
  }


CL_NS_END
