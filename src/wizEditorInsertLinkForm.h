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

    QString getContent();
    QString getUrl();
    void clear();

private:
    Ui::CWizEditorInsertLinkForm *ui;
};

#endif // WIZEDITORINSERTLINKFORM_H
