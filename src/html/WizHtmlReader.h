#ifndef __WIZHTMLREADER_H__
#define __WIZHTMLREADER_H__

#ifndef WIZQTHELPER_H
#include "../share/WizQtHelper.h"
#endif //WIZQTHELPER_H

#define WIZ_SAFE_DELETE_POINTER(_P)  (void)(_P != NULL ? delete _P, _P = NULL : 0)

class WizHtmlReader;
class WizHtmlAttributes;
class WizHtmlElemAttr;
class WizHtmlTag;


class WizHtmlReaderEvents
{
    friend class WizHtmlReader;
// Events
protected:
    virtual void beginParse(DWORD dwAppData, bool &bAbort) { UNUSED_ALWAYS(dwAppData); bAbort = false; }
    virtual void startTag(WizHtmlTag *pTag, DWORD dwAppData, bool &bAbort) { UNUSED_ALWAYS(pTag); UNUSED_ALWAYS(dwAppData); bAbort = false; }
    virtual void endTag(WizHtmlTag *pTag, DWORD dwAppData, bool &bAbort) { UNUSED_ALWAYS(pTag); UNUSED_ALWAYS(dwAppData); bAbort = false; }
    virtual void characters(const CString &rText, DWORD dwAppData, bool &bAbort) { UNUSED_ALWAYS(rText); UNUSED_ALWAYS(dwAppData); bAbort = false; }
    virtual void comment(const CString &rComment, DWORD dwAppData, bool &bAbort) { UNUSED_ALWAYS(rComment); UNUSED_ALWAYS(dwAppData); bAbort = false; }
    virtual void endParse(DWORD dwAppData, bool bIsAborted) { UNUSED_ALWAYS(dwAppData); UNUSED_ALWAYS(bIsAborted); }
public:
    virtual ~WizHtmlReaderEvents() {}
};



class WizHtmlElemAttr
{
public:
    WizHtmlElemAttr(const CString& strAttribName = "", const CString& strAttribValue = "");
    WizHtmlElemAttr(const WizHtmlElemAttr &rSource);

// Initialization Helpers
private:
    static void init(void);

// Attributes
public:
    CString getName(void) const { return (m_strAttrName); }
    CString getValue(void) const { return (m_strAttrValue); }
    bool isNamedColorValue(void) const;
    bool isSysColorValue(void) const;
    bool isHexColorValue(void) const;
    bool isColorValue(void) const { return (isNamedColorValue() || isHexColorValue()); }
    COLORREF getColorValue(void) const;
    CString getColorHexValue(void) const;
    bool isPercentValue(void) const { return (m_strAttrValue.right(1) == "%" ? true : false); }

    unsigned short getPercentValue(unsigned short max = _percentMax) const;

    enum LengthUnitsEnum { em, ex, px, per, in, cm, mm, pt, pc };
    short getLengthValue(LengthUnitsEnum &rUnit) const;

// Operators
public:
    operator bool() const;
    operator short() const { return ((short)::wiz_atoi(m_strAttrValue)); }
    operator const unsigned short*() const { return (m_strAttrValue); }

// Private Operations
private:
    void putValue(const CString& strValue);

// Parsing Helpers
public:
    // parses an attribute/value pair from the given string
    UINT parseFromStr(const unsigned short* lpszString);
    CString toString()const;

// Data Members
public:
    static const COLORREF		_clrInvalid;	// an invalid color
    static const unsigned short	_percentMax;	// maximum allowable percentage value
private:
    typedef std::map<CString, COLORREF>	CNamedColors;
    static CNamedColors		_namedColors;	// collection of named colors
    CString					m_strAttrName;  // attribute name
    CString                 m_strAttrValue; // attribute value
    friend class WizHtmlAttributes;
};


class WizHtmlAttributes
{
// Construction/Destruction
public:
    WizHtmlAttributes() : m_parrAttrib(NULL) { }
    WizHtmlAttributes(WizHtmlAttributes &rSource, bool bCopy = false);
    virtual ~WizHtmlAttributes() { removeAll(); }

// Initialization
public:
    // parses attribute/value pairs from the given string
    UINT parseFromStr(const unsigned short* lpszString);

// Attributes
public:
    int getCount(void) const;
    int getIndexFromName(const CString& strAttributeName) const;
    WizHtmlElemAttr operator[](int nIndex) const;
    WizHtmlElemAttr operator[](const CString& strIndex) const { return ((*this)[getIndexFromName(strIndex)]); }

    WizHtmlElemAttr getAttribute(int nIndex) const { return ((*this)[nIndex]); }
    WizHtmlElemAttr getAttribute(const CString& strIndex) const { return ((*this)[getIndexFromName(strIndex)]); }
    CString getName(int nIndex) const { return ((*this)[nIndex].m_strAttrName); }

    CString getValue(int nIndex) const { return ((*this)[nIndex].m_strAttrValue); }
    CString getValueFromName(const CString& strAttributeName) const { return ((*this)[strAttributeName].m_strAttrValue); }
    void setValueToName(const CString& strAttributeName, const CString& strValue);

// Operations
public:
    WizHtmlElemAttr* addAttribute(const CString& strName, const CString& strValue);
    bool removeAttribute(int nIndex);
    bool removeAttribute(const CString& strAttributeName);
    bool removeAll(void);
    //
// Data Members
private:
    typedef std::deque<WizHtmlElemAttr*>	CElemAttrArray;
    CElemAttrArray	*m_parrAttrib;	// array of attributes/value pairs
};

class WizHtmlTag
{
// Construction/Destruction
public:
    WizHtmlTag() : m_pcollAttr(NULL), m_bIsOpeningTag(false), m_bIsClosingTag(false), m_bModified(false) { }
    WizHtmlTag(WizHtmlTag &rSource, bool bCopy = false);
    virtual ~WizHtmlTag() { WIZ_SAFE_DELETE_POINTER(m_pcollAttr); }
// Attributes
public:
    CString getTagName(void) const { return (m_strTagName); }
    const WizHtmlAttributes* getAttributes(void) const { return (m_pcollAttr);  }
    bool isOpening(void) const { return m_bIsOpeningTag; }
    bool isClosing(void) const { return m_bIsClosingTag; }
    CString getValueFromName(const CString& strAttributeName) const { if (!m_pcollAttr) return CString(); return m_pcollAttr->getValueFromName(strAttributeName); }
    CString getTag(void);
    void setValueToName(const CString& strAttributeName, const CString& strValue);
    void removeAttribute(const CString& strAttributeName);

// Parsing Helpers
public:
    UINT parseFromStr(const unsigned short* lpszString, bool &bIsOpeningTag, bool &bIsClosingTag, bool bParseAttrib = true);

// Data Members
private:
    WizHtmlAttributes	*m_pcollAttr;
    CString				m_strTagName;
    CString             m_strTag;
    bool m_bIsOpeningTag;
    bool m_bIsClosingTag;
    bool m_bModified;
};


class WizHtmlReader
{
public:
	enum EventMaskEnum {
		notifyStartStop		= 0x00000001L,	// raise BeginParse and EndParse?
		notifyTagStart		= 0x00000002L,	// raise StartTag?
		notifyTagEnd		= 0x00000004L,	// raise EndTag?
		notifyCharacters	= 0x00000008L,	// raise Characters?
        notifyComment		= 0x00000010L	// raise Comment?
	};

	enum ReaderOptionsEnum {
        resolveEntities	    // determines whether entity references should be resolved
	};

// Construction/Destruction
public:
    WizHtmlReader();

public:
    EventMaskEnum getEventMask(void) const { return (m_eventMask); }

    EventMaskEnum setEventMask(DWORD dwNewEventMask) ;
    EventMaskEnum setEventMask(DWORD addFlags, DWORD removeFlags);
    DWORD getAppData(void) const { return (m_dwAppData); }

    DWORD setAppData(DWORD dwNewAppData);
    WizHtmlReaderEvents* getEventHandler(void) const { return (m_pEventHandler); }
    WizHtmlReaderEvents* setEventHandler(WizHtmlReaderEvents* pNewHandler);

    bool getBoolOption(ReaderOptionsEnum option, bool& bCurVal) const;
	bool setBoolOption(ReaderOptionsEnum option, bool bNewVal);

// Operations
public:
    UINT read(const QString &strHtml);
// Helpers
protected:
	virtual UINT parseDocument(void);
    virtual bool parseComment(QString &rComment);
    virtual bool parseTag(WizHtmlTag &rTag, bool &bIsOpeningTag, bool &bIsClosingTag);
    virtual void normalizeCharacters(CString &rCharacters);
    void resetSeekPointer(void) { m_dwBufPos = 0L; }

    const unsigned short readChar(void);
    const unsigned short ungetChar(void);
    bool getEventNotify(DWORD dwEvent) const ;
    bool isWhiteSpace(int ch) const { return (::wiz_isspace(ch) ? true : false); }

protected:
	bool	m_bResolveEntities;
	DWORD	m_dwAppData;
	DWORD	m_dwBufPos;
	DWORD	m_dwBufLen;
	EventMaskEnum	m_eventMask;
    WizHtmlReaderEvents*	m_pEventHandler;
    const unsigned short*	m_lpszBuffer;
};


void WizHtmlRemoveStyle(QString& strHtml, const QString& styleId);
void WizHtmlInsertStyle(QString& strHtml, const QString& styleId, const QString& strCssText);
void WizHtmlInsertHtmlBeforeAllBodyChildren(QString& strHtml, const QString& strHtmlPart);


#endif	// !__WIZHTMLREADER_H__
