#ifndef __WIZHTMLREADER_H__
#define __WIZHTMLREADER_H__

#ifndef WIZQTHELPER_H
#include "../share/wizqthelper.h"
#endif //WIZQTHELPER_H

#define WIZ_SAFE_DELETE_POINTER(_P)  (void)(_P != NULL ? delete _P, _P = NULL : 0)

class CWizHtmlReader;
class CWizHtmlAttributes;
class CWizHtmlElemAttr;
class CWizHtmlTag;


class IWizHtmlReaderEvents
{
    friend class CWizHtmlReader;
// Events
protected:
    virtual void BeginParse(DWORD dwAppData, bool &bAbort) { UNUSED_ALWAYS(dwAppData); bAbort = false; }
    virtual void StartTag(CWizHtmlTag *pTag, DWORD dwAppData, bool &bAbort) { UNUSED_ALWAYS(pTag); UNUSED_ALWAYS(dwAppData); bAbort = false; }
    virtual void EndTag(CWizHtmlTag *pTag, DWORD dwAppData, bool &bAbort) { UNUSED_ALWAYS(pTag); UNUSED_ALWAYS(dwAppData); bAbort = false; }
    virtual void Characters(const CString &rText, DWORD dwAppData, bool &bAbort) { UNUSED_ALWAYS(rText); UNUSED_ALWAYS(dwAppData); bAbort = false; }
    virtual void Comment(const CString &rComment, DWORD dwAppData, bool &bAbort) { UNUSED_ALWAYS(rComment); UNUSED_ALWAYS(dwAppData); bAbort = false; }
    virtual void EndParse(DWORD dwAppData, bool bIsAborted) { UNUSED_ALWAYS(dwAppData); UNUSED_ALWAYS(bIsAborted); }
public:
    virtual ~IWizHtmlReaderEvents() {}
};



class CWizHtmlElemAttr
{
public:
    CWizHtmlElemAttr(const CString& strAttribName = _T(""), const CString& strAttribValue = _T(""));
    CWizHtmlElemAttr(const CWizHtmlElemAttr &rSource);

// Initialization Helpers
private:
    static void Init(void);

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
    bool isPercentValue(void) const { return (m_strAttrValue.Right(1) == _T("%") ? true : false); }

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
    friend class CWizHtmlAttributes;
};


class CWizHtmlAttributes
{
// Construction/Destruction
public:
    CWizHtmlAttributes() : m_parrAttrib(NULL) { }
    CWizHtmlAttributes(CWizHtmlAttributes &rSource, bool bCopy = false);
    virtual ~CWizHtmlAttributes() { removeAll(); }

// Initialization
public:
    // parses attribute/value pairs from the given string
    UINT parseFromStr(const unsigned short* lpszString);

// Attributes
public:
    int getCount(void) const;
    int getIndexFromName(const CString& strAttributeName) const;
    CWizHtmlElemAttr operator[](int nIndex) const;
    CWizHtmlElemAttr operator[](const CString& strIndex) const { return ((*this)[getIndexFromName(strIndex)]); }

    CWizHtmlElemAttr getAttribute(int nIndex) const { return ((*this)[nIndex]); }
    CWizHtmlElemAttr getAttribute(const CString& strIndex) const { return ((*this)[getIndexFromName(strIndex)]); }
    CString getName(int nIndex) const { return ((*this)[nIndex].m_strAttrName); }

    CString getValue(int nIndex) const { return ((*this)[nIndex].m_strAttrValue); }
    CString getValueFromName(const CString& strAttributeName) const { return ((*this)[strAttributeName].m_strAttrValue); }
    void setValueToName(const CString& strAttributeName, const CString& strValue);

// Operations
public:
    CWizHtmlElemAttr* addAttribute(const CString& strName, const CString& strValue);
    bool removeAttribute(int nIndex);
    bool removeAttribute(const CString& strAttributeName);
    bool removeAll(void);
    //
// Data Members
private:
    typedef std::deque<CWizHtmlElemAttr*>	CElemAttrArray;
    CElemAttrArray	*m_parrAttrib;	// array of attributes/value pairs
};

class CWizHtmlTag
{
// Construction/Destruction
public:
    CWizHtmlTag() : m_pcollAttr(NULL), m_bIsOpeningTag(false), m_bIsClosingTag(false), m_bModified(false) { }
    CWizHtmlTag(CWizHtmlTag &rSource, bool bCopy = false);
    virtual ~CWizHtmlTag() { WIZ_SAFE_DELETE_POINTER(m_pcollAttr); }
// Attributes
public:
    CString getTagName(void) const { return (m_strTagName); }
    const CWizHtmlAttributes* getAttributes(void) const { return (m_pcollAttr);  }
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
    CWizHtmlAttributes	*m_pcollAttr;
    CString				m_strTagName;
    CString             m_strTag;
    bool m_bIsOpeningTag;
    bool m_bIsClosingTag;
    bool m_bModified;
};


class CWizHtmlReader
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
    CWizHtmlReader();

public:
    EventMaskEnum getEventMask(void) const { return (m_eventMask); }

    EventMaskEnum setEventMask(DWORD dwNewEventMask) ;
    EventMaskEnum setEventMask(DWORD addFlags, DWORD removeFlags);
    DWORD getAppData(void) const { return (m_dwAppData); }

    DWORD setAppData(DWORD dwNewAppData);
    IWizHtmlReaderEvents* getEventHandler(void) const { return (m_pEventHandler); }
    IWizHtmlReaderEvents* setEventHandler(IWizHtmlReaderEvents* pNewHandler);

    bool getBoolOption(ReaderOptionsEnum option, bool& bCurVal) const;
	bool setBoolOption(ReaderOptionsEnum option, bool bNewVal);

// Operations
public:
    UINT Read(const QString &strHtml);
// Helpers
protected:
	virtual UINT parseDocument(void);
    virtual bool parseComment(QString &rComment);
    virtual bool parseTag(CWizHtmlTag &rTag, bool &bIsOpeningTag, bool &bIsClosingTag);
    virtual void NormalizeCharacters(CString &rCharacters);
    void ResetSeekPointer(void) { m_dwBufPos = 0L; }

    const unsigned short ReadChar(void);
    const unsigned short UngetChar(void);
    bool getEventNotify(DWORD dwEvent) const ;
    bool isWhiteSpace(int ch) const { return (::wiz_isspace(ch) ? true : false); }

protected:
	bool	m_bResolveEntities;
	DWORD	m_dwAppData;
	DWORD	m_dwBufPos;
	DWORD	m_dwBufLen;
	EventMaskEnum	m_eventMask;
    IWizHtmlReaderEvents*	m_pEventHandler;
    const unsigned short*	m_lpszBuffer;
};


#endif	// !__WIZHTMLREADER_H__
