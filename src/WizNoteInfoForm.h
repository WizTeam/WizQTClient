#ifndef WIZNOTEINFOFORM_H
#define WIZNOTEINFOFORM_H

#include "share/WizPopupWidget.h"
#include "share/WizDatabaseManager.h"

namespace Ui {
class WizNoteInfoForm;
}

class WizNoteInfoForm : public WizPopupWidget
{
    Q_OBJECT
    
public:
    explicit WizNoteInfoForm(QWidget *parent = 0);
    ~WizNoteInfoForm();

    void setDocument(const WIZDOCUMENTDATA& data);
    void setWordCount(int nWords, int nChars, int nCharsWithSpace, int nNonAsianWords, int nAsianChars);

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
    Ui::WizNoteInfoForm *ui;
    QString m_docKbGuid;
    QString m_docGuid;
    QSize m_size;
    QString m_sizeText;
};


#endif // WIZNOTEINFOFORM_H
