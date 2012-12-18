/* This is the Porter stemming algorithm, originally written by Martin Porter.
   It may be regarded as cononical, in that it follows the
   algorithm presented in

   Porter, 1980, An algorithm for suffix stripping, Program, Vol. 14,
   no. 3, pp 130-137,

   See also http://www.tartarus.org/~martin/PorterStemmer

   Modified by "Hemant Muthiyan"
   email: hemant_muthiyan@yahoo.co.in
    
   The Porter stemmer should be regarded as ‘frozen’, that is, strictly defined, 
   and not amenable to further modification. As a stemmer, it is slightly inferior 
   to the Snowball English or Porter2 stemmer, which derives from it, and which is 
   subjected to occasional improvements. For practical work, therefore, the new 
   Snowball stemmer is recommended. The Porter stemmer is appropriate to IR 
   research work involving stemming where the experiments need to be exactly 
   repeatable. 

*/
#ifndef _lucene_analysis_PorterStemmer_
#define _lucene_analysis_PorterStemmer_

CL_NS_DEF(analysis)

class CLUCENE_CONTRIBS_EXPORT PorterStemmer
{
private:
	TCHAR *b;
    size_t i,    /* offset into b */
    j, k, k0;
	bool dirty;
    //private static final int32_t EXTRA = 1;

  /* cons(i) is true <=> b[i] is a consonant. */

private:
	bool cons(size_t i);

  /* m() measures the number of consonant sequences between k0 and j. if c is
     a consonant sequence and v a vowel sequence, and <..> indicates arbitrary
     presence,

          <c><v>       gives 0
          <c>vc<v>     gives 1
          <c>vcvc<v>   gives 2
          <c>vcvcvc<v> gives 3
          ....
  */

   int32_t m();

  /* vowelinstem() is true <=> k0,...j contains a vowel */

   bool vowelinstem();

  /* doublec(j) is true <=> j,(j-1) contain a double consonant. */
   bool doublec(size_t j);

  /* cvc(i) is true <=> i-2,i-1,i has the form consonant - vowel - consonant
     and also if the second c is not w,x or y. this is used when trying to
     restore an e at the end of a short word. e.g.

          cav(e), lov(e), hop(e), crim(e), but
          snow, box, tray.

  */
   bool cvc(size_t i);

  bool ends(TCHAR *s);

  /* setto(s) sets (j+1),...k to the characters in the string s, readjusting
     k. */

  void setto(const TCHAR *s);

  /* r(s) is used further down. */

  void r(const TCHAR *s);

  /* step1() gets rid of plurals and -ed or -ing. e.g.

           caresses  ->  caress
           ponies    ->  poni
           ties      ->  ti
           caress    ->  caress
           cats      ->  cat

           feed      ->  feed
           agreed    ->  agree
           disabled  ->  disable

           matting   ->  mat
           mating    ->  mate
           meeting   ->  meet
           milling   ->  mill
           messing   ->  mess

           meetings  ->  meet

  */

  void step1();

  /* step2() turns terminal y to i when there is another vowel in the stem. */

  void step2();

  /* step3() maps double suffices to single ones. so -ization ( = -ize plus
     -ation) maps to -ize etc. note that the string before the suffix must give
     m() > 0. */

  void step3();

  /* step4() deals with -ic-, -full, -ness etc. similar strategy to step3. */

  void step4();

  /* step5() takes off -ant, -ence etc., in context <c>vcvc<v>. */

  void step5();

  /* step6() removes a final -e if m() > 1. */

  void step6();

 public:

 	PorterStemmer(TCHAR *Text);
   ~PorterStemmer();


   /**
    * Returns the length of the word resulting from the stemming process.
    */
   int32_t getResultLength();

	 bool stem();

  /**
   * Returns a reference to a character buffer containing the results of
   * the stemming process.  You also need to consult getResultLength()
   * to determine the length of the result.
   */
  const TCHAR* getResultBuffer();

};
CL_NS_END

#endif
