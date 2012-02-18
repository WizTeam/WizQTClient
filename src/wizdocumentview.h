#ifndef WIZDOCUMENTVIEW_H
#define WIZDOCUMENTVIEW_H

#ifndef QWIDGET_H
#include <QWidget>
#endif

#include <QLineEdit>
#include <QPushButton>

#ifndef WIZDEF_H
#include "wizdef.h"
#endif

class QLineEdit;
class CWizDocumentWebView;
class CWizDatabase;
//

class CWizTitleContainer
    : public QWidget
{
    Q_OBJECT;

public:
    CWizTitleContainer(QWidget* parent);

private:
    QLineEdit* m_edit;

    bool m_bLocked;
    QPushButton* m_lockBtn;

public:
    QLineEdit* edit() const { return m_edit; }
    QPushButton* unlock() const { return m_lockBtn; }
    void setLock();
    void setText(const QString& str) { m_edit->setText(str); }

public slots:
    void on_unlockBtnClicked();
};

class CWizDocumentView
    : public QWidget
{
    Q_OBJECT;
public:
    CWizDocumentView(CWizExplorerApp& app, QWidget* parent = 0);
protected:
    CWizDatabase& m_db;
    CWizTitleContainer* m_title;
    CWizDocumentWebView* m_web;
    QWidget* m_client;
    //
    QWidget* createClient();
public:
    bool viewDocument(const WIZDOCUMENTDATA& data);
    bool newDocument();
    //
    void showClient(bool visible);
    //
    const WIZDOCUMENTDATA& document();
    //
public slots:
    void on_title_textEdited ( const QString & text );
};

#endif // WIZDOCUMENTVIEW_H
