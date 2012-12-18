/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_FuzzyQuery_
#define _lucene_search_FuzzyQuery_

#include "MultiTermQuery.h"
#include "FilteredTermEnum.h"

CL_CLASS_DEF(index,Term)

CL_NS_DEF(search)

/** Implements the fuzzy search query. The similiarity measurement
* is based on the Levenshtein (edit distance) algorithm.
*/
class CLUCENE_EXPORT FuzzyQuery : public MultiTermQuery {
private:
	float_t minimumSimilarity;
	size_t prefixLength;
protected:
	FuzzyQuery(const FuzzyQuery& clone);
public:
	static float_t defaultMinSimilarity;
	static int32_t defaultPrefixLength;

	/**
	* Create a new FuzzyQuery that will match terms with a similarity 
	* of at least <code>minimumSimilarity</code> to <code>term</code>.
	* If a <code>prefixLength</code> &gt; 0 is specified, a common prefix
	* of that length is also required.
	* 
	* @param term the term to search for
	* @param minimumSimilarity a value between 0 and 1 to set the required similarity
	*  between the query term and the matching terms. For example, for a
	*  <code>minimumSimilarity</code> of <code>0.5</code> a term of the same length
	*  as the query term is considered similar to the query term if the edit distance
	*  between both terms is less than <code>length(term)*0.5</code>
	* @param prefixLength length of common (non-fuzzy) prefix
	* @throws IllegalArgumentException if minimumSimilarity is &gt; 1 or &lt; 0
	* or if prefixLength &lt; 0 or &gt; <code>term.text().length()</code>.
	*/
	FuzzyQuery(CL_NS(index)::Term* term, float_t minimumSimilarity=-1, size_t prefixLength=0);
	virtual ~FuzzyQuery();

	/**
	* Returns the minimum similarity that is required for this query to match.
	* @return float value between 0.0 and 1.0
	*/
	float_t getMinSimilarity() const;

	/**
	* Returns the prefix length, i.e. the number of characters at the start
	* of a term that must be identical (not fuzzy) to the query term if the query
	* is to match that term. 
	*/
	size_t getPrefixLength() const;

	Query* rewrite(CL_NS(index)::IndexReader* reader);

	TCHAR* toString(const TCHAR* field) const;

	//Returns the name "FuzzyQuery"
	static const char* getClassName();
	const char* getObjectName() const;

	Query* clone() const;
	bool equals(Query * other) const;
	size_t hashCode() const;

protected:
	FilteredTermEnum* getEnum(CL_NS(index)::IndexReader* reader);
};

/** Subclass of FilteredTermEnum for enumerating all terms that are similiar
* to the specified filter term.
*
* <p>Term enumerations are always ordered by Term.compareTo().  Each term in
* the enumeration is greater than all that precede it.
*/
class CLUCENE_EXPORT FuzzyTermEnum: public FilteredTermEnum {
private:
	/* Allows us save time required to create a new array
	* everytime similarity is called.
	*/
	int32_t* d;
	size_t dLen;

	//float_t distance;
	float_t _similarity;
	bool _endEnum;

	CL_NS(index)::Term* searchTerm; 
	//String field;
	TCHAR* text;
	size_t textLen;
	TCHAR* prefix;
	size_t prefixLength;

	float_t minimumSimilarity;
	double scale_factor;
	int32_t maxDistances[LUCENE_TYPICAL_LONGEST_WORD_IN_INDEX];

	/******************************
	* Compute Levenshtein distance
	******************************/

	/**
	* <p>Similarity returns a number that is 1.0f or less (including negative numbers)
	* based on how similar the Term is compared to a target term.  It returns
	* exactly 0.0f when
	* <pre>
	*    editDistance &lt; maximumEditDistance</pre>
	* Otherwise it returns:
	* <pre>
	*    1 - (editDistance / length)</pre>
	* where length is the length of the shortest term (text or target) including a
	* prefix that are identical and editDistance is the Levenshtein distance for
	* the two words.</p>
	*
	* <p>Embedded within this algorithm is a fail-fast Levenshtein distance
	* algorithm.  The fail-fast algorithm differs from the standard Levenshtein
	* distance algorithm in that it is aborted if it is discovered that the
	* mimimum distance between the words is greater than some threshold.
	*
	* <p>To calculate the maximum distance threshold we use the following formula:
	* <pre>
	*     (1 - minimumSimilarity) * length</pre>
	* where length is the shortest term including any prefix that is not part of the
	* similarity comparision.  This formula was derived by solving for what maximum value
	* of distance returns false for the following statements:
	* <pre>
	*   similarity = 1 - ((float)distance / (float) (prefixLength + Math.min(textlen, targetlen)));
	*   return (similarity > minimumSimilarity);</pre>
	* where distance is the Levenshtein distance for the two words.
	* </p>
	* <p>Levenshtein distance (also known as edit distance) is a measure of similiarity
	* between two strings where the distance is measured as the number of character
	* deletions, insertions or substitutions required to transform one string to
	* the other string.
	* @param target the target word or phrase
	* @return the similarity,  0.0 or less indicates that it matches less than the required
	* threshold and 1.0 indicates that the text and target are identical
	*/
	float_t similarity(const TCHAR* target, const size_t targetLen);

	/**
	* The max Distance is the maximum Levenshtein distance for the text
	* compared to some other value that results in score that is
	* better than the minimum similarity.
	* @param m the length of the "other value"
	* @return the maximum levenshtein distance that we care about
	*/
	int32_t getMaxDistance(const size_t m);

	void initializeMaxDistances();

	int32_t calculateMaxDistance(const size_t m) const;

protected:
	/**
	* The termCompare method in FuzzyTermEnum uses Levenshtein distance to 
	* calculate the distance between the given term and the comparing term. 
	*/
	bool termCompare(CL_NS(index)::Term* term) ;

	/** Returns the fact if the current term in the enumeration has reached the end */
	bool endEnum();
public:

	/**
	* Constructor for enumeration of all terms from specified <code>reader</code> which share a prefix of
	* length <code>prefixLength</code> with <code>term</code> and which have a fuzzy similarity &gt;
	* <code>minSimilarity</code>.
	* <p>
	* After calling the constructor the enumeration is already pointing to the first 
	* valid term if such a term exists. 
	* 
	* @param reader Delivers terms.
	* @param term Pattern term.
	* @param minSimilarity Minimum required similarity for terms from the reader. Default value is 0.5f.
	* @param prefixLength Length of required common prefix. Default value is 0.
	* @throws IOException
	*/
	FuzzyTermEnum(CL_NS(index)::IndexReader* reader, CL_NS(index)::Term* term, float_t minSimilarity=FuzzyQuery::defaultMinSimilarity, size_t prefixLength=0);
	virtual ~FuzzyTermEnum();

	/** Close the enumeration */
	void close();

	/** Returns the difference between the distance and the fuzzy threshold
	*  multiplied by the scale factor
	*/
	float_t difference();

	const char* getObjectName() const;
	static const char* getClassName();
};

CL_NS_END
#endif
