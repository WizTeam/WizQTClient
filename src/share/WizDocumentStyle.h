#ifndef WIZDOCUMENTSTYLE_H
#define WIZDOCUMENTSTYLE_H

#include "WizObject.h"

class WizDocumentStylePrivate;

class WizDocumentStyle
{
public:
    WizDocumentStyle();
public:
    WIZSTYLEDATA getStyle(QString styleGuid);
private:
    WizDocumentStylePrivate* m_data;
public:
    static WizDocumentStyle& instance();
};

#endif // WIZDOCUMENTSTYLE_H
