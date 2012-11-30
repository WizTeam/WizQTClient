#ifndef WIZTAGLISTWIDGET_H
#define WIZTAGLISTWIDGET_H

#include <QCheckBox>

#include "share/wizpopupwidget.h"
#include "share/wizdatabase.h"
class QScrollArea;

class QLineEdit;
class CWizTagCheckBox;

class CWizTagListWidget : public CWizPopupWidget
{
    Q_OBJECT
public:
    CWizTagListWidget(CWizDatabase& db, QWidget* parent);

    void setDocument(const WIZDOCUMENTDATA& data);

private:
    QLineEdit* m_tagsEdit;
    QScrollArea* m_scroll;
    CWizDatabase& m_db;
    WIZDOCUMENTDATA m_document;

    void updateTagsText();

public slots:
    void on_tagCheckBox_checked(CWizTagCheckBox* sender, int state);
    void on_tagsEdit_editingFinished();
};


class CWizTagCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    CWizTagCheckBox(const WIZTAGDATA& tag, QWidget* parent);
private:
    WIZTAGDATA m_tag;
public:
    const WIZTAGDATA& tag() const { return m_tag; }
public slots:
    void on_checkbox_checked(int state);
Q_SIGNALS:
    void tagChecked(CWizTagCheckBox* sender, int state);
};

#endif // WIZTAGLISTWIDGET_H
