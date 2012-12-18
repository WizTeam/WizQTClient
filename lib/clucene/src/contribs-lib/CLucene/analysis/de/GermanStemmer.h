/*------------------------------------------------------------------------------
* Copyright (C) 2003-2010 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_analysis_de_GermanStemmer
#define _lucene_analysis_de_GermanStemmer

CL_CLASS_DEF(util,StringBuffer)

CL_NS_DEF2(analysis,de)

/**
 * A stemmer for German words. The algorithm is based on the report
 * "A Fast and Simple Stemming Algorithm for German Words" by J&ouml;rg
 * Caumanns (joerg.caumanns at isst.fhg.de).
 */
class CLUCENE_CONTRIBS_EXPORT GermanStemmer
{
private:

    /**
     * Buffer for the terms while stemming them.
     */
    CL_NS(util)::StringBuffer sb;

    /**
     * Amount of characters that are removed with <tt>substitute()</tt> while stemming.
     */
    int substCount;

public:

    /**
     */
    GermanStemmer();

    /**
     * Stemms the given term to an unique <tt>discriminator</tt>.
     *
     * @param term  The term that should be stemmed.
     * @return      Discriminator for <tt>term</tt>
     */
    TCHAR* stem(const TCHAR* term, size_t length = -1);

private:

    /**
     * Checks if a term could be stemmed.
     *
     * @return  true if, and only if, the given term consists in letters.
     */
    bool isStemmable(const TCHAR* term, size_t length = -1) const;

    /**
     * suffix stripping (stemming) on the current term. The stripping is reduced
     * to the seven "base" suffixes "e", "s", "n", "t", "em", "er" and * "nd",
     * from which all regular suffixes are build of. The simplification causes
     * some overstemming, and way more irregular stems, but still provides unique.
     * discriminators in the most of those cases.
     * The algorithm is context free, except of the length restrictions.
     */
     void strip(CL_NS(util)::StringBuffer& buffer);

    /**
     * Does some optimizations on the term. This optimisations are
     * contextual.
     */
    void optimize(CL_NS(util)::StringBuffer& buffer);

    /**
     * Removes a particle denotion ("ge") from a term.
     */
    void removeParticleDenotion(CL_NS(util)::StringBuffer& buffer);

    /**
     * Do some substitutions for the term to reduce overstemming:
     *
     * - Substitute Umlauts with their corresponding vowel: äöü -> aou,
     *   "ß" is substituted by "ss"
     * - Substitute a second char of a pair of equal characters with
     *   an asterisk: ?? -> ?*
     * - Substitute some common character combinations with a token:
     *   sch/ch/ei/ie/ig/st -> $/§/%/&/#/!
     */
    void substitute(CL_NS(util)::StringBuffer& buffer);

    /**
     * Undoes the changes made by substitute(). That are character pairs and
     * character combinations. Umlauts will remain as their corresponding vowel,
     * as "ß" remains as "ss".
     */
    void resubstitute(CL_NS(util)::StringBuffer& buffer);
};

CL_NS_END2
#endif
