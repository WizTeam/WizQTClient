#ifndef WIZSEARCHREPLACEWIDGET_H
#define WIZSEARCHREPLACEWIDGET_H

#include <QDialog>

namespace Ui {
class WizSearchReplaceWidget;
}

class WizDocumentWebView;
class WizSearchReplaceWidget : public QDialog
{
    Q_OBJECT

public:
    explicit WizSearchReplaceWidget(QWidget *parent = 0);
    ~WizSearchReplaceWidget();

    void showInEditor(const QRect& rcEditor);
signals:
    void findPre(QString strTxt, bool bCasesensitive);
    void findNext(QString strTxt, bool bCasesensitive);
    void replaceCurrent(QString strSource, QString strTarget);
    void replaceAndFindNext(QString strSource, QString strTarget, bool bCasesensitive);
    void replaceAll(QString strSource, QString strTarget, bool bCasesensitive);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_btn_pre_clicked();

    void on_btn_next_clicked();

    void on_btn_replace_clicked();

    void on_btn_replaceAll_clicked();

    void on_lineEdit_source_returnPressed();
private:
    Ui::WizSearchReplaceWidget *ui;

    void clearAllText();
};

#endif // WIZSEARCHREPLACEWIDGET_H
