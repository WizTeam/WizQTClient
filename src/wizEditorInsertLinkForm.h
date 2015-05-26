#ifndef WIZEDITORINSERTLINKFORM_H
#define WIZEDITORINSERTLINKFORM_H

#include <QDialog>

namespace Ui {
class CWizEditorInsertLinkForm;
}

class CWizEditorInsertLinkForm : public QDialog
{
    Q_OBJECT
    
public:
    explicit CWizEditorInsertLinkForm(QWidget *parent = 0);
    ~CWizEditorInsertLinkForm();

//    QString getContent();
//    void setContent(const QString& strText);

    QString getUrl();
    void setUrl(const QString& strText);

    void clear();

private slots:
    void on_pushButton_cancel_clicked();

    void on_pushButton_ok_clicked();

private:
    Ui::CWizEditorInsertLinkForm *ui;
};

#endif // WIZEDITORINSERTLINKFORM_H
