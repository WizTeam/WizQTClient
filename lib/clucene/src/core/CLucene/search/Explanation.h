/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_Explanation
#define _lucene_search_Explanation

#include "CLucene/util/VoidList.h"

CL_NS_DEF(search)

#define LUCENE_SEARCH_EXPLANATION_DESC_LEN 200

/** Expert: Describes the score computation for document and query. */
class CLUCENE_EXPORT Explanation {
private:
	float_t value;															// the value of this node
	TCHAR description[LUCENE_SEARCH_EXPLANATION_DESC_LEN];					// what it represents
	CL_NS(util)::CLArrayList<Explanation*,
		CL_NS(util)::Deletor::Object<Explanation> >* details;					// sub-explanations

public:
	Explanation();
	Explanation(float_t _value, const TCHAR* _description);
	virtual ~Explanation();

	/**
	* Indicates whether or not this Explanation models a good match.
	*
	* <p>
	* By default, an Explanation represents a "match" if the value is positive.
	* </p>
	* @see #getValue
	*/
	virtual bool isMatch() const;

	/** The value assigned to this explanation node. */
	float_t getValue() const;
	/** Sets the value assigned to this explanation node. */
	void setValue(const float_t value);

	/** A description of this explanation node. */
	const TCHAR* getDescription() const; ///<returns reference
	/** Sets the description of this explanation node. */
	void setDescription(const TCHAR* description);
protected:
	/**
	* A short one line summary which should contain all high level
	* information about this Explanation, without the "Details"
	*/
	virtual TCHAR* getSummary();

public:
	Explanation(const Explanation& copy);
	void set(const Explanation& other);
	virtual Explanation* clone() const;

	/** The sub-nodes of this explanation node. 
	* @param ret this array of Explanations should be getDetailsLength()+1 in size. 
	*            The array will be null terminated.
	*/
	void getDetails(Explanation** ret);
	size_t getDetailsLength() const;

	/** Watch out: no NULL reference check is made! Make sure i exists by not calling this function
	without calling getDetailsLength() first!
	*/
	Explanation* getDetail(const size_t i);

	/** Adds a sub-node to this explanation node. */
	void addDetail(Explanation* detail);

	/** Render an explanation as text. */
	TCHAR* toString();
	TCHAR* toString(const int32_t depth);

	/** Render an explanation as HTML. */
	TCHAR* toHtml();
};

class CLUCENE_EXPORT ComplexExplanation : public Explanation {
private:
	bool match;
public:
	ComplexExplanation();
	ComplexExplanation(const ComplexExplanation& copy);
	ComplexExplanation(const bool _match, const float_t _value, const TCHAR* _description);
	virtual ~ComplexExplanation();

	/**
	* The match status of this explanation node.
	* @return May be null if match status is unknown
	*/
	bool getMatch() const;
	/**
	* Sets the match status assigned to this explanation node.
	* @param match May be null if match status is unknown
	*/
	void setMatch(const bool _match);
	/**
	* Indicates wether or not this Explanation models a good match.
	*
	* <p>
	* If the match statis is explicitly set (ie: not null) this method
	* uses it; otherwise it defers to the superclass.
	* </p>
	* @see #getMatch
	*/
	bool isMatch() const;

	Explanation* clone() const;

protected:
	TCHAR* getSummary();
};

CL_NS_END
#endif
