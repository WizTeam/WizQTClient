#ifndef WIZSEARCHREPLACEWIDGET_H
#define WIZSEARCHREPLACEWIDGET_H

#include <QWidget>

namespace Ui {
class CWizSearchReplaceWidget;
}

class CWizDocumentWebView;
class CWizSearchReplaceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CWizSearchReplaceWidget(QWidget *parent = 0);
    ~CWizSearchReplaceWidget();

signals:
    void findPre(QString strTxt, bool bCasesensitive);
    void findNext(QString strTxt, bool bCasesensitive);
    void replaceCurrent(QString strSource, QString strTarget);
    void replaceAndFindNext(QString strSource, QString strTarget, bool bCasesensitive);
    void replaceAll(QString strSource, QString strTarget, bool bCasesensitive);

private:
    Ui::CWizSearchReplaceWidget *ui;
};

#endif // WIZSEARCHREPLACEWIDGET_H
