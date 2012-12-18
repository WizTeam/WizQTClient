/*------------------------------------------------------------------------------
* Copyright (C) 2003-2010 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_analysis_de_GermanAnalyzer
#define _lucene_analysis_de_GermanAnalyzer

CL_NS_DEF2(analysis,de)

/**
 * Analyzer for German language. Supports an external list of stopwords (words that
 * will not be indexed at all) and an external list of exclusions (word that will
 * not be stemmed, but indexed).
 * A default set of stopwords is used unless an alternative list is specified, the
 * exclusion list is empty by default.
 *
 * 
 * @version $Id: GermanAnalyzer.java 564236 2007-08-09 15:21:19Z gsingers $
 */
class CLUCENE_CONTRIBS_EXPORT GermanAnalyzer : public CL_NS(analysis)::Analyzer {
public:

  /**
   * List of typical german stopwords.
   */
  static CL_NS(util)::ConstValueArray<const TCHAR*> GERMAN_STOP_WORDS;

private:

  class SavedStreams;

  /**
   * Contains the stopwords used with the StopFilter.
   */
  CL_NS(analysis)::CLTCSetList* stopSet;

  /**
   * Contains words that should be indexed but not stemmed.
   */
  CL_NS(analysis)::CLTCSetList* exclusionSet;

public:

  /**
   * Builds an analyzer with the default stop words
   * (<code>GERMAN_STOP_WORDS</code>).
   */
  GermanAnalyzer();

  /**
   * Builds an analyzer with the given stop words.
   */
  GermanAnalyzer(const TCHAR** stopWords);

  /**
   * Builds an analyzer with the given stop words.
   */
  GermanAnalyzer(CL_NS(analysis)::CLTCSetList* stopwords);

  /**
   * Builds an analyzer with the given stop words.
   */
  GermanAnalyzer(const char* stopwordsFile, const char* enc = NULL);

  /**
   * Builds an analyzer with the given stop words.
   */
  GermanAnalyzer(CL_NS(util)::Reader* stopwordsReader, const bool deleteReader = false);

  /**
   */
  virtual ~GermanAnalyzer();

  /**
   * Builds an exclusionlist from an array of Strings.
   */
  void setStemExclusionTable(const TCHAR** exclusionlist);

  /**
   * Builds an exclusionlist from a Hashtable.
   */
  void setStemExclusionTable(CL_NS(analysis)::CLTCSetList* exclusionlist);

  /**
   * Builds an exclusionlist from the words contained in the given file.
   */
  void setStemExclusionTable(const char* exclusionlistFile, const char* enc = NULL);

  /**
   * Builds an exclusionlist from the words contained in the given file.
   */
  void setStemExclusionTable(CL_NS(util)::Reader* exclusionlistReader, const bool deleteReader = false);

  /**
   * Creates a TokenStream which tokenizes all the text in the provided Reader.
   *
   * @return A TokenStream build from a StandardTokenizer filtered with
   *         StandardFilter, LowerCaseFilter, StopFilter, GermanStemFilter
   */
  virtual TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);

  virtual TokenStream* reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
};

CL_NS_END2
#endif
