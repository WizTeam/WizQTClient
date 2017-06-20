#ifndef WIZXMLRPC_H
#define WIZXMLRPC_H

#include "WizXml.h"


//enum WizXmlRpcError
//{
//    errorNetwork = -987,
//    errorContentType,
//    errorXmlFormat,
//    errorXmlRpcFormat,
//    errorXmlRpcFault,
//    errorTokenInvalid = -982,
//    errorPermissionException,
//    errorWizAllError
//};




/* ------------------------- CWizXmlRpcValue ------------------------- */
// Abstract base class for all XML-RPC return
class WizXmlRpcValue
{
public:
    virtual ~WizXmlRpcValue() {}

    virtual bool write(WizXMLNode& nodeParent) = 0;
    virtual bool read(WizXMLNode& nodeParent) = 0;
    virtual QString toString() const = 0;

    template <class TData>
    bool toData(TData& data);

    template <class TData>
    bool toArray(std::deque<TData>& arrayData);
};


/* ------------------------- CWizXmlRpcIntValue ------------------------- */
class WizXmlRpcIntValue  : public WizXmlRpcValue
{
public:
    WizXmlRpcIntValue(int n = 0);

    virtual bool write(WizXMLNode& nodeValue);
    virtual bool read(WizXMLNode& nodeValue);
    virtual QString toString() const;

    operator int();

private:
    int m_n;
};


/* ------------------------- CWizXmlRpcBoolValue ------------------------- */
class WizXmlRpcBoolValue : public WizXmlRpcValue
{
public:
    WizXmlRpcBoolValue(bool b = false);

    virtual bool write(WizXMLNode& nodeValue);
    virtual bool read(WizXMLNode& nodeValue);
    virtual QString toString() const;

    operator bool();

private:
    bool m_b;
};


/* ------------------------- CWizXmlRpcStringValue ------------------------- */
class WizXmlRpcStringValue : public WizXmlRpcValue
{
public:
    WizXmlRpcStringValue(const QString& strDef = "");

    virtual bool write(WizXMLNode& nodeValue);
    virtual bool read(WizXMLNode& nodeValue);
    virtual QString toString() const;

    operator QString ();

private:
    QString m_str;
};


/* ------------------------- CWizXmlRpcTimeValue ------------------------- */
class WizXmlRpcTimeValue : public WizXmlRpcValue
{
public:
    WizXmlRpcTimeValue();
    WizXmlRpcTimeValue(const WizOleDateTime& t);

    virtual bool write(WizXMLNode& nodeValue);
    virtual bool read(WizXMLNode& nodeValue);
    virtual QString toString() const;

    operator WizOleDateTime();

private:
    WizOleDateTime m_t;
};


/* ------------------------- CWizXmlRpcBase64Value ------------------------- */
class WizXmlRpcBase64Value : public WizXmlRpcValue
{
public:
    WizXmlRpcBase64Value(const QByteArray& arrayData = QByteArray());

    virtual bool write(WizXMLNode& nodeValue);
    virtual bool read(WizXMLNode& nodeValue);
    virtual QString toString() const;

    bool getStream(QByteArray& arrayData);

private:
    QByteArray m_arrayData;
};


/* ------------------------- CWizXmlRpcArrayValue ------------------------- */
class WizXmlRpcArrayValue  : public WizXmlRpcValue
{
public:
	WizXmlRpcArrayValue();
	WizXmlRpcArrayValue(const CWizStdStringArray& arrayData);
    virtual ~WizXmlRpcArrayValue ();

    virtual bool write(WizXMLNode& nodeValue);
    virtual bool read(WizXMLNode& nodeValue);
    virtual QString toString() const;

	template <class TData>
    bool toArray(std::deque<TData>& arrayRet);

    template <class TData>
    bool toArray(std::vector<TData>& arrayRet);

    void add(WizXmlRpcValue* pValue);
    bool toStringArray(CWizStdStringArray& arrayRet);
    //
    std::deque<WizXmlRpcValue*> value() const { return m_array; }

private:
    std::deque<WizXmlRpcValue*> m_array;

    void clear();
    bool setStringArray(const CWizStdStringArray& arrayData);
};


/* ------------------------- CWizXmlRpcStructValue ------------------------- */
class WizXmlRpcStructValue  : public WizXmlRpcValue
{
public:
	virtual ~WizXmlRpcStructValue();

    virtual bool write(WizXMLNode& nodeValue);
    virtual bool read(WizXMLNode& nodeValue);
    virtual QString toString() const;

    // compose method
    void addInt(const QString& strName, int n);
    void addString(const QString& strName, const QString& str);
    void addStruct(const QString& strName, WizXmlRpcStructValue* pStruct);
    void addArray(const QString& strName, WizXmlRpcValue* pArray);
    void addBool(const QString& strName, bool b);
    void addTime(const QString& strName, const WizOleDateTime& t);
    void addBase64(const QString& strName, const QByteArray& arrayData);
    void addInt64(const QString& strName, __int64 n);
    void addColor(const QString& strName, COLORREF cr);
    void addStringArray(const QString& strName, const CWizStdStringArray& arrayData);

    template <class TData>
    bool addArray(const QString& strName, const std::deque<TData>& arrayData);

    // parse method
    bool getBool(const QString& strName, bool& b) const;
    bool getInt(const QString& strName, int& n) const;
    bool getInt(const QString& strName, long& n) const;
    //bool GetInt(const QString& strName, quint32& n) const;
    bool getInt64(const QString& strName, qint64& n) const;
    //bool GetInt64(const QString& strName, quint64& n) const;
    bool getString(const QString& strName, QString& str) const;
    bool getStr(const QString& strName, QString& str) const { return getString(strName, str); }
    bool getTime(const QString& strName, WizOleDateTime& t) const;
    bool getStream(const QString& strName, QByteArray& arrayData) const;
    WizXmlRpcStructValue* getStruct(const QString& strName) const;
    WizXmlRpcArrayValue* getArray(const QString& strName) const;
    bool getColor(const QString& strName, COLORREF& cr) const;
    bool getStringArray(const QString& strName, CWizStdStringArray& arrayData) const;
    //
    bool toStringMap(std::map<QString, QString>& ret) const;


	template <class TData>
    bool getArray(const QString& strName, std::deque<TData>& arrayData);

private:
    std::map<QString, WizXmlRpcValue*> m_map;

    // map management methods
    void clear();
    void removeValue(const QString& strName);
    void deleteValue(WizXmlRpcValue* pValue);
    void addValue(const QString& strzName, WizXmlRpcValue* pValue);
    WizXmlRpcValue* getMappedValue(const QString& strName) const;

    template<class T>
    T* getValue(const QString& strName) const { return dynamic_cast<T*>(getMappedValue(strName)); }
};


/* ------------------------- CWizXmlRpcFaultValue ------------------------- */
class WizXmlRpcFaultValue : public WizXmlRpcValue
{
public:
    ~WizXmlRpcFaultValue();

    virtual bool write(WizXMLNode& nodeValue) { Q_UNUSED(nodeValue); ATLASSERT(FALSE); return FALSE; }
    virtual bool read(WizXMLNode& nodeValue);
    virtual QString toString() const;

    int getFaultCode() const;
    QString getFaultString() const;

private:
    WizXmlRpcStructValue m_val;
};


/* ------------------------- CWizXmlRpcRequest ------------------------- */
// XML-RPC request class
class WizXmlRpcRequest
{
public:
    WizXmlRpcRequest(const QString& strMethodName);
    void addParam(WizXmlRpcValue* pParam);
    QByteArray toData();

protected:
    WizXMLDocument m_doc;
};


// XML-RPC server use this function to parse the dom tree
bool WizXmlRpcResultFromXml(WizXMLDocument& doc, WizXmlRpcValue** ppRet);


// template methods

template <class TData>
inline bool WizXmlRpcValue::toData(TData& data)
{
    if (WizXmlRpcStructValue* pStruct = dynamic_cast<WizXmlRpcStructValue*>(this))
    {
        return data.loadFromXmlRpc(*pStruct);
    }

    return false;
}

template <class TData>
inline bool WizXmlRpcValue::toArray(std::deque<TData>& arrayData)
{
    if (WizXmlRpcArrayValue* pArray = dynamic_cast<WizXmlRpcArrayValue*>(this))
    {
        return pArray->toArray<TData>(arrayData);
    }

    return false;
}

template <class TData>
inline bool WizXmlRpcArrayValue::toArray(std::deque<TData>& arrayRet)
{
    std::deque<WizXmlRpcValue*>::const_iterator it;
    for (it = m_array.begin(); it != m_array.end(); it++) {
        WizXmlRpcValue* pValue = *it;
        if (!pValue)
        {
            //TOLOG("Fault error: element of array is null");
            return false;
        }

        TData data;
        if (!pValue->toData<TData>(data))
        {
            //TOLOG("Failed load data form value");
            return false;
        }

        arrayRet.push_back(data);
    }

    return true;
}


template <class TData>
inline bool WizXmlRpcArrayValue::toArray(std::vector<TData>& arrayRet)
{
    std::deque<WizXmlRpcValue*>::const_iterator it;
    for (it = m_array.begin(); it != m_array.end(); it++) {
        WizXmlRpcValue* pValue = *it;
        if (!pValue)
        {
            //TOLOG("Fault error: element of array is null");
            return false;
        }

        TData data;
        if (!pValue->toData<TData>(data))
        {
            //TOLOG("Failed load data form value");
            return false;
        }

        arrayRet.push_back(data);
    }

    return true;
}


template <class TData>
inline bool WizXmlRpcStructValue::getArray(const QString& strName,
                                            std::deque<TData>& arrayData)
{
    WizXmlRpcArrayValue* pArray = getArray(strName);
    if (!pArray)
    {
        //TOLOG("Failed to get array data in struct");
        return false;
    }

    return pArray->toArray<TData>(arrayData);
}

template <class TData>
inline bool WizXmlRpcStructValue::addArray(const QString& strName, const std::deque<TData>& arrayData)
{
    WizXmlRpcArrayValue* pArray = new WizXmlRpcArrayValue();
    addArray(strName, pArray);

    for (typename std::deque<TData>::const_iterator it = arrayData.begin();
        it != arrayData.end();
        it++)
    {
        WizXmlRpcStructValue* pStruct = new WizXmlRpcStructValue();
        it->saveToXmlRpc(*pStruct);

        pArray->add(pStruct);
    }

    return true;
}


class WizXmlRpcResult
{
    WizXmlRpcValue* m_pResult;
    int m_nFaultCode;
    QString m_strFaultString;
    BOOL m_bXmlRpcSucceeded;
    BOOL m_bFault;
public:
    WizXmlRpcResult();
    ~WizXmlRpcResult();
public:
    void setResult(const QString& strMethodName, WizXmlRpcValue* pRet);
    //
    BOOL isXmlRpcSucceeded() const;
    BOOL isFault() const;
    BOOL isNoError() const;
    //
    template <class T>
    T* getResultValue() const
    {
        return dynamic_cast<T*>(m_pResult);
    }
    //
    BOOL getString(QString& str) const;
    BOOL getBool(BOOL& b) const;
};

#endif //WIZXMLRPC_H
