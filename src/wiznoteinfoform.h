#ifndef WIZNOTEINFOFORM_H
#define WIZNOTEINFOFORM_H

#include "share/wizpopupwidget.h"
#include "share/wizDatabaseManager.h"

namespace Ui {
class CWizNoteInfoForm;
}

class CWizNoteInfoForm : public CWizPopupWidget
{
    Q_OBJECT
    
public:
    explicit CWizNoteInfoForm(QWidget *parent = 0);
    ~CWizNoteInfoForm();

    void setDocument(const WIZDOCUMENTDATA& data);

    void setGroupLabelVisible(bool isGroupNote);

signals:
    void widgetStatusChanged();

protected:
    virtual QSize sizeHint() const;
    void hideEvent(QHideEvent* ev);

private slots:
    void on_labelOpenDocument_linkActivated(const QString &link);

    void on_editURL_editingFinished();

    void on_editAuthor_editingFinished();

    void on_checkEncrypted_clicked(bool checked);

    void on_labelHistory_linkActivated(const QString &link);

    void on_labelOpenURL_linkActivated(const QString &link);

private:
    Ui::CWizNoteInfoForm *ui;
    QString m_docKbGuid;
    QString m_docGuid;
    QSize m_size;
};


#endif // WIZNOTEINFOFORM_H
