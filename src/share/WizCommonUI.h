#ifndef WIZCOMMONUI_H
#define WIZCOMMONUI_H

#include "WizQtHelper.h"

class WizCommonUI : public QObject
{
    Q_OBJECT
public:
    WizCommonUI(QObject* parent);

public slots:
    //interface WizKMControls.WizCommonUI;
    QString loadTextFromFile(const QString& strFileName);
    QString clipboardToImage(int hwnd, const QString& strOptions);

};

#endif // WIZCOMMONUI_H
