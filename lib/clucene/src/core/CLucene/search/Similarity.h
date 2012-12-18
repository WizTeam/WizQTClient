/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_Similarity_
#define _lucene_search_Similarity_

#include "CLucene/util/VoidList.h"
CL_CLASS_DEF(index,Term)

CL_NS_DEF(search)

class Searcher;
class DefaultSimilarity;

/** Expert: Scoring API.
* <p>Subclasses implement search scoring.
*
* <p>The score of query <code>q</code> for document <code>d</code> is defined
* in terms of these methods as follows:
*
* <table cellpadding="0" cellspacing="0" border="0">
*  <tr>
*    <td valign="middle" align="right" rowspan="2">score(q,d) =<br></td>
*    <td valign="middle" align="center">
*    <big><big><big><big><big>&Sigma;</big></big></big></big></big></td>
*    <td valign="middle"><small>
*    {@link #tf(int32_t) tf}(t in d) *
*    {@link #idf(Term,Searcher) idf}(t) *
*    {@link Field#getBoost getBoost}(t.field in d) *
*    {@link #lengthNorm(TCHAR*,int32_t) lengthNorm}(t.field in d)
*    </small></td>
*    <td valign="middle" rowspan="2">&nbsp;*
*    {@link #coord(int32_t,int32_t) coord}(q,d) *
*    {@link #queryNorm(float_t) queryNorm}(q)
*    </td>
*  </tr>
*  <tr>
*   <td valign="top" align="right">
*    <small>t in q</small>
*    </td>
*  </tr>
* </table>
*
* @see #setDefault(Similarity)
* @see IndexWriter#setSimilarity(Similarity)
* @see Searcher#setSimilarity(Similarity)
*/
class CLUCENE_EXPORT Similarity:LUCENE_BASE {
public:
	virtual ~Similarity();
	
  /** Set the default Similarity implementation used by indexing and search
   * code.
   *
   * @see Searcher#setSimilarity(Similarity)
   * @see IndexWriter#setSimilarity(Similarity)
   */
   static void setDefault(Similarity* similarity);
   
   /** Return the default Similarity implementation used by indexing and search
   * code.
   *
   * <p>This is initially an instance of {@link DefaultSimilarity}.
   *
   * @see Searcher#setSimilarity(Similarity)
   * @see IndexWriter#setSimilarity(Similarity)
   */
   static Similarity* getDefault();
   
	/** Cleanup static data */
	static CLUCENE_LOCAL void _shutdown();
   
   /** Encodes a normalization factor for storage in an index.
   *
   * <p>The encoding uses a five-bit exponent and three-bit mantissa, thus
   * representing values from around 7x10^9 to 2x10^-9 with about one
   * significant decimal digit of accuracy.  Zero is also represented.
   * Negative numbers are rounded up to zero.  Values too large to represent
   * are rounded down to the largest representable value.  Positive values too
   * small to represent are rounded up to the smallest positive representable
   * value.
   *
   * @see Field#setBoost(float_t)
   */
   static uint8_t encodeNorm(float_t f);
   
   /** Decodes a normalization factor stored in an index.
   * @see #encodeNorm(float_t)
   */
   static float_t decodeNorm(uint8_t b);
   
   static uint8_t floatToByte(float_t f);
   static float_t byteToFloat(uint8_t b);

   /** Computes a score factor for a phrase.
   *
   * <p>The default implementation sums the {@link #idf(Term,Searcher)} factor
   * for each term in the phrase.
   *
   * @param terms the terms in the phrase
   * @param searcher the document collection being searched
   * @return a score factor for the phrase
   */
   float_t idf(CL_NS(util)::CLVector<CL_NS(index)::Term*>* terms, Searcher* searcher);

   template<class TermIterator>
   float_t idf( TermIterator first, TermIterator last, Searcher* searcher )
   {
      float_t _idf = 0.0f;
      for( ; first != last; first++ ) {
         _idf += idf(*first, searcher);
      }
      return _idf;
   }

   //float_t idf(Term** terms, Searcher* searcher);

   
   /** Computes a score factor for a simple term.
   *
   * <p>The default implementation is:<pre>
   *   return idf(searcher.docFreq(term), searcher.maxDoc());
   * </pre>
   *
   * Note that {@link Searcher#maxDoc()} is used instead of
   * {@link IndexReader#numDocs()} because it is proportional to
   * {@link Searcher#docFreq(Term)} , i.e., when one is inaccurate,
   * so is the other, and in the same direction.
   *
   * @param term the term in question
   * @param searcher the document collection being searched
   * @return a score factor for the term
   */
   float_t idf(CL_NS(index)::Term* term, Searcher* searcher);

   
   /** Computes a score factor based on a term or phrase's frequency in a
   * document.  This value is multiplied by the {@link #idf(Term, Searcher)}
   * factor for each term in the query and these products are then summed to
   * form the initial score for a document.
   *
   * <p>Terms and phrases repeated in a document indicate the topic of the
   * document, so implementations of this method usually return larger values
   * when <code>freq</code> is large, and smaller values when <code>freq</code>
   * is small.
   *
   * <p>The default implementation calls {@link #tf(float_t)}.
   *
   * @param freq the frequency of a term within a document
   * @return a score factor based on a term's within-document frequency
   */
   inline float_t tf(int32_t freq){ return tf((float_t)freq); }

   /** Computes the normalization value for a field given the total number of
   * terms contained in a field.  These values, together with field boosts, are
   * stored in an index and multipled into scores for hits on each field by the
   * search code.
   *
   * <p>Matches in longer fields are less precise, so implemenations of this
   * method usually return smaller values when <code>numTokens</code> is large,
   * and larger values when <code>numTokens</code> is small.
   *
   * <p>That these values are computed under {@link
   * IndexWriter#addDocument(Document)} and stored then using
   * {#encodeNorm(float_t)}.  Thus they have limited precision, and documents
   * must be re-indexed if this method is altered.
   *
   * @param fieldName the name of the field
   * @param numTokens the total number of tokens contained in fields named
   * <i>fieldName</i> of <i>doc</i>.
   * @return a normalization factor for hits on this field of this document
   *
   * @see Field#setBoost(float_t)
   */
   virtual float_t lengthNorm(const TCHAR* fieldName, int32_t numTokens) = 0;

   /** Computes the normalization value for a query given the sum of the squared
   * weights of each of the query terms.  This value is then multipled into the
   * weight of each query term.
   *
   * <p>This does not affect ranking, but rather just attempts to make scores
   * from different queries comparable.
   *
   * @param sumOfSquaredWeights the sum of the squares of query term weights
   * @return a normalization factor for query weights
   */
   virtual float_t queryNorm(float_t sumOfSquaredWeights) = 0;

   /** Computes the amount of a sloppy phrase match, based on an edit distance.
   * This value is summed for each sloppy phrase match in a document to form
   * the frequency that is passed to {@link #tf(float_t)}.
   *
   * <p>A phrase match with a small edit distance to a document passage more
   * closely matches the document, so implementations of this method usually
   * return larger values when the edit distance is small and smaller values
   * when it is large.
   *
   * @see PhraseQuery#setSlop(int32_t)
   * @param distance the edit distance of this sloppy phrase match
   * @return the frequency increment for this match
   */
   virtual float_t sloppyFreq(int32_t distance) = 0;

   /** Computes a score factor based on a term or phrase's frequency in a
   * document.  This value is multiplied by the {@link #idf(Term, Searcher)}
   * factor for each term in the query and these products are then summed to
   * form the initial score for a document.
   *
   * <p>Terms and phrases repeated in a document indicate the topic of the
   * document, so implemenations of this method usually return larger values
   * when <code>freq</code> is large, and smaller values when <code>freq</code>
   * is small.
   *
   * @param freq the frequency of a term within a document
   * @return a score factor based on a term's within-document frequency
   */
   virtual float_t tf(float_t freq) = 0;

   /** Computes a score factor based on a term's document frequency (the number
   * of documents which contain the term).  This value is multiplied by the
   * {@link #tf(int32_t)} factor for each term in the query and these products are
   * then summed to form the initial score for a document.
   *
   * <p>Terms that occur in fewer documents are better indicators of topic, so
   * implemenations of this method usually return larger values for rare terms,
   * and smaller values for common terms.
   *
   * @param docFreq the number of documents which contain the term
   * @param numDocs the total number of documents in the collection
   * @return a score factor based on the term's document frequency
   */
   virtual float_t idf(int32_t docFreq, int32_t numDocs) = 0;

   /** Computes a score factor based on the fraction of all query terms that a
   * document contains.  This value is multiplied into scores.
   *
   * <p>The presence of a large portion of the query terms indicates a better
   * match with the query, so implemenations of this method usually return
   * larger values when the ratio between these parameters is large and smaller
   * values when the ratio between them is small.
   *
   * @param overlap the number of query terms matched in the document
   * @param maxOverlap the total number of terms in the query
   * @return a score factor based on term overlap with the query
   */
   virtual float_t coord(int32_t overlap, int32_t maxOverlap) = 0;
};


/** Expert: Default scoring implementation. */
class CLUCENE_EXPORT DefaultSimilarity: public Similarity {
public:
	DefaultSimilarity();
	~DefaultSimilarity();

  /** Implemented as <code>1/sqrt(numTerms)</code>. */
  float_t lengthNorm(const TCHAR* fieldName, int32_t numTerms);
  
  /** Implemented as <code>1/sqrt(sumOfSquaredWeights)</code>. */
  float_t queryNorm(float_t sumOfSquaredWeights);

  /** Implemented as <code>sqrt(freq)</code>. */
  inline float_t tf(float_t freq);
    
  /** Implemented as <code>1 / (distance + 1)</code>. */
  float_t sloppyFreq(int32_t distance);
    
  /** Implemented as <code>log(numDocs/(docFreq+1)) + 1</code>. */
  float_t idf(int32_t docFreq, int32_t numDocs);
    
  /** Implemented as <code>overlap / maxOverlap</code>. */
  float_t coord(int32_t overlap, int32_t maxOverlap);
};

CL_NS_END
#endif
