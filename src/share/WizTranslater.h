#ifndef WIZTRANSLATER_H
#define WIZTRANSLATER_H

#include <QString>

struct WizTranslateItem
{
    QString key;
    QString value;
};

class WizTranslater
{
public:
    WizTranslater();
    ~WizTranslater();

    WizTranslateItem* itemsData();
};


QString WizTranlateString(const QString& strString);

#endif // WIZTRANSLATER_H
