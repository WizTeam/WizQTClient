#ifndef WIZMD5_H
#define WIZMD5_H

#include "WizQtHelper.h"

CString WizMd5String(const unsigned char* pBuffer, DWORD dwLen);
CString WizMd5StringNoSpace(const unsigned char* pBuffer, DWORD dwLen);
CString WizMd5StringNoSpaceJava(const unsigned char* pBuffer, DWORD dwLen);
CString WizMd5StringNoSpaceJava(const QByteArray& arr);

CString WizMd5FileStringNoSpaceJava(const CString& strFileName);
CString WizMd5FileString(const CString& strFileName);
CString WizMd5StringNoSpace(const CString& str);

#endif // WIZMD5_H
