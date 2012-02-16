#ifndef WIZCOMMONUI_H
#define WIZCOMMONUI_H

#include "wizqthelper.h"

class CWizCommonUI : public QObject
{
    Q_OBJECT
public:
    CWizCommonUI(QObject* parent);

public slots:
    //interface WizKMControls.WizCommonUI;
    QString LoadTextFromFile(const QString& strFileName);
    QString ClipboardToImage(int hwnd, const QString& strOptions);

};

#endif // WIZCOMMONUI_H
