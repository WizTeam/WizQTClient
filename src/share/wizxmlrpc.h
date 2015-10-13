#ifndef WIZXMLRPC_H
#define WIZXMLRPC_H

#include "wizxml.h"


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

//重新定义token无效的错误码，因为301已经被QT定义，在登录界面无法判断301是服务器还是QT
//返回的网络错误。 此处使用客户端自定义的错误代码
#define WIZKM_XMLRPC_ERROR_INVALID_TOKEN		-982
#define WIZKM_XMLRPC_ERROR_PERMISSION_EXCEPTION		-981
#define WIZKM_XMLRPC_ERROR_WIZ_ALL_ERROR		-980
#define WIZKM_XMLRPC_ERROR_INVALID_USER			31001
#define WIZKM_XMLRPC_ERROR_INVALID_PASSWORD		31002
#define WIZKM_XMLRPC_ERROR_TOO_MANY_LOGINS		31004


/* ------------------------- CWizXmlRpcValue ------------------------- */
// Abstract base class for all XML-RPC return
class CWizXmlRpcValue
{
public:
    virtual ~CWizXmlRpcValue() {}

    virtual bool Write(CWizXMLNode& nodeParent) = 0;
    virtual bool Read(CWizXMLNode& nodeParent) = 0;
    virtual QString ToString() const = 0;

    template <class TData>
    bool ToData(TData& data);

    template <class TData>
    bool ToArray(std::deque<TData>& arrayData);
};


/* ------------------------- CWizXmlRpcIntValue ------------------------- */
class CWizXmlRpcIntValue  : public CWizXmlRpcValue
{
public:
    CWizXmlRpcIntValue(int n = 0);

    virtual bool Write(CWizXMLNode& nodeValue);
    virtual bool Read(CWizXMLNode& nodeValue);
    virtual QString ToString() const;

    operator int();

private:
    int m_n;
};


/* ------------------------- CWizXmlRpcBoolValue ------------------------- */
class CWizXmlRpcBoolValue : public CWizXmlRpcValue
{
public:
    CWizXmlRpcBoolValue(bool b = false);

    virtual bool Write(CWizXMLNode& nodeValue);
    virtual bool Read(CWizXMLNode& nodeValue);
    virtual QString ToString() const;

    operator bool();

private:
    bool m_b;
};


/* ------------------------- CWizXmlRpcStringValue ------------------------- */
class CWizXmlRpcStringValue : public CWizXmlRpcValue
{
public:
    CWizXmlRpcStringValue(const QString& strDef = "");

    virtual bool Write(CWizXMLNode& nodeValue);
    virtual bool Read(CWizXMLNode& nodeValue);
    virtual QString ToString() const;

    operator QString ();

private:
    QString m_str;
};


/* ------------------------- CWizXmlRpcTimeValue ------------------------- */
class CWizXmlRpcTimeValue : public CWizXmlRpcValue
{
public:
    CWizXmlRpcTimeValue();
    CWizXmlRpcTimeValue(const COleDateTime& t);

    virtual bool Write(CWizXMLNode& nodeValue);
    virtual bool Read(CWizXMLNode& nodeValue);
    virtual QString ToString() const;

    operator COleDateTime();

private:
    COleDateTime m_t;
};


/* ------------------------- CWizXmlRpcBase64Value ------------------------- */
class CWizXmlRpcBase64Value : public CWizXmlRpcValue
{
public:
    CWizXmlRpcBase64Value(const QByteArray& arrayData = QByteArray());

    virtual bool Write(CWizXMLNode& nodeValue);
    virtual bool Read(CWizXMLNode& nodeValue);
    virtual QString ToString() const;

    bool GetStream(QByteArray& arrayData);

private:
    QByteArray m_arrayData;
};


/* ------------------------- CWizXmlRpcArrayValue ------------------------- */
class CWizXmlRpcArrayValue  : public CWizXmlRpcValue
{
public:
	CWizXmlRpcArrayValue();
	CWizXmlRpcArrayValue(const CWizStdStringArray& arrayData);
    virtual ~CWizXmlRpcArrayValue ();

    virtual bool Write(CWizXMLNode& nodeValue);
    virtual bool Read(CWizXMLNode& nodeValue);
    virtual QString ToString() const;

	template <class TData>
    bool ToArray(std::deque<TData>& arrayRet);

    template <class TData>
    bool ToArray(std::vector<TData>& arrayRet);

    void Add(CWizXmlRpcValue* pValue);
    bool ToStringArray(CWizStdStringArray& arrayRet);

private:
    std::deque<CWizXmlRpcValue*> m_array;

    void Clear();
    bool SetStringArray(const CWizStdStringArray& arrayData);
};


/* ------------------------- CWizXmlRpcStructValue ------------------------- */
class CWizXmlRpcStructValue  : public CWizXmlRpcValue
{
public:
	virtual ~CWizXmlRpcStructValue();

    virtual bool Write(CWizXMLNode& nodeValue);
    virtual bool Read(CWizXMLNode& nodeValue);
    virtual QString ToString() const;

    // compose method
    void AddInt(const QString& strName, int n);
    void AddString(const QString& strName, const QString& str);
    void AddStruct(const QString& strName, CWizXmlRpcStructValue* pStruct);
    void AddArray(const QString& strName, CWizXmlRpcValue* pArray);
    void AddBool(const QString& strName, bool b);
    void AddTime(const QString& strName, const COleDateTime& t);
    void AddBase64(const QString& strName, const QByteArray& arrayData);
    void AddInt64(const QString& strName, __int64 n);
    void AddColor(const QString& strName, COLORREF cr);
    void AddStringArray(const QString& strName, const CWizStdStringArray& arrayData);

    template <class TData>
    bool AddArray(const QString& strName, const std::deque<TData>& arrayData);

    // parse method
    bool GetBool(const QString& strName, bool& b) const;
    bool GetInt(const QString& strName, int& n) const;
    bool GetInt(const QString& strName, long& n) const;
    //bool GetInt(const QString& strName, quint32& n) const;
    bool GetInt64(const QString& strName, qint64& n) const;
    //bool GetInt64(const QString& strName, quint64& n) const;
    bool GetString(const QString& strName, QString& str) const;
    bool GetStr(const QString& strName, QString& str) const { return GetString(strName, str); }
    bool GetTime(const QString& strName, COleDateTime& t) const;
    bool GetStream(const QString& strName, QByteArray& arrayData) const;
    CWizXmlRpcStructValue* GetStruct(const QString& strName) const;
    CWizXmlRpcArrayValue* GetArray(const QString& strName) const;
    bool GetColor(const QString& strName, COLORREF& cr) const;
    bool GetStringArray(const QString& strName, CWizStdStringArray& arrayData) const;
    //
    bool ToStringMap(std::map<QString, QString>& ret) const;


	template <class TData>
    bool GetArray(const QString& strName, std::deque<TData>& arrayData);

private:
    std::map<QString, CWizXmlRpcValue*> m_map;

    // map management methods
    void Clear();
    void RemoveValue(const QString& strName);
    void DeleteValue(CWizXmlRpcValue* pValue);
    void AddValue(const QString& strzName, CWizXmlRpcValue* pValue);
    CWizXmlRpcValue* GetMappedValue(const QString& strName) const;

    template<class T>
    T* GetValue(const QString& strName) const { return dynamic_cast<T*>(GetMappedValue(strName)); }
};


/* ------------------------- CWizXmlRpcFaultValue ------------------------- */
class CWizXmlRpcFaultValue : public CWizXmlRpcValue
{
public:
    ~CWizXmlRpcFaultValue();

    virtual bool Write(CWizXMLNode& nodeValue) { Q_UNUSED(nodeValue); ATLASSERT(FALSE); return FALSE; }
    virtual bool Read(CWizXMLNode& nodeValue);
    virtual QString ToString() const;

    int GetFaultCode() const;
    QString GetFaultString() const;

private:
    CWizXmlRpcStructValue m_val;
};


/* ------------------------- CWizXmlRpcRequest ------------------------- */
// XML-RPC request class
class CWizXmlRpcRequest
{
public:
    CWizXmlRpcRequest(const QString& strMethodName);
    void addParam(CWizXmlRpcValue* pParam);
    QByteArray toData();

protected:
    CWizXMLDocument m_doc;
};


// XML-RPC server use this function to parse the dom tree
bool WizXmlRpcResultFromXml(CWizXMLDocument& doc, CWizXmlRpcValue** ppRet);


// template methods

template <class TData>
inline bool CWizXmlRpcValue::ToData(TData& data)
{
    if (CWizXmlRpcStructValue* pStruct = dynamic_cast<CWizXmlRpcStructValue*>(this))
    {
        return data.LoadFromXmlRpc(*pStruct);
    }

    return false;
}

template <class TData>
inline bool CWizXmlRpcValue::ToArray(std::deque<TData>& arrayData)
{
    if (CWizXmlRpcArrayValue* pArray = dynamic_cast<CWizXmlRpcArrayValue*>(this))
    {
        return pArray->ToArray<TData>(arrayData);
    }

    return false;
}

template <class TData>
inline bool CWizXmlRpcArrayValue::ToArray(std::deque<TData>& arrayRet)
{
    std::deque<CWizXmlRpcValue*>::const_iterator it;
    for (it = m_array.begin(); it != m_array.end(); it++) {
        CWizXmlRpcValue* pValue = *it;
        if (!pValue)
        {
            //TOLOG(_T("Fault error: element of array is null"));
            return false;
        }

        TData data;
        if (!pValue->ToData<TData>(data))
        {
            //TOLOG(_T("Failed load data form value"));
            return false;
        }

        arrayRet.push_back(data);
    }

    return true;
}


template <class TData>
inline bool CWizXmlRpcArrayValue::ToArray(std::vector<TData>& arrayRet)
{
    std::deque<CWizXmlRpcValue*>::const_iterator it;
    for (it = m_array.begin(); it != m_array.end(); it++) {
        CWizXmlRpcValue* pValue = *it;
        if (!pValue)
        {
            //TOLOG(_T("Fault error: element of array is null"));
            return false;
        }

        TData data;
        if (!pValue->ToData<TData>(data))
        {
            //TOLOG(_T("Failed load data form value"));
            return false;
        }

        arrayRet.push_back(data);
    }

    return true;
}


template <class TData>
inline bool CWizXmlRpcStructValue::GetArray(const QString& strName,
                                            std::deque<TData>& arrayData)
{
    CWizXmlRpcArrayValue* pArray = GetArray(strName);
    if (!pArray)
    {
        //TOLOG(_T("Failed to get array data in struct"));
        return false;
    }

    return pArray->ToArray<TData>(arrayData);
}

template <class TData>
inline bool CWizXmlRpcStructValue::AddArray(const QString& strName, const std::deque<TData>& arrayData)
{
    CWizXmlRpcArrayValue* pArray = new CWizXmlRpcArrayValue();
    AddArray(strName, pArray);

    for (typename std::deque<TData>::const_iterator it = arrayData.begin();
        it != arrayData.end();
        it++)
    {
        CWizXmlRpcStructValue* pStruct = new CWizXmlRpcStructValue();
        it->SaveToXmlRpc(*pStruct);

        pArray->Add(pStruct);
    }

    return true;
}


class CWizXmlRpcResult
{
    CWizXmlRpcValue* m_pResult;
    int m_nFaultCode;
    QString m_strFaultString;
    BOOL m_bXmlRpcSucceeded;
    BOOL m_bFault;
public:
    CWizXmlRpcResult();
    ~CWizXmlRpcResult();
public:
    void SetResult(const QString& strMethodName, CWizXmlRpcValue* pRet);
    //
    BOOL IsXmlRpcSucceeded() const;
    BOOL IsFault() const;
    BOOL IsNoError() const;
    //
    template <class T>
    T* GetResultValue() const
    {
        return dynamic_cast<T*>(m_pResult);
    }
    //
    BOOL GetString(QString& str) const;
    BOOL GetBool(BOOL& b) const;
};

#endif //WIZXMLRPC_H
