#ifndef WIZDOCUMENTVIEW_H
#define WIZDOCUMENTVIEW_H

#ifndef QWIDGET_H
#include <QWidget>
#endif

#ifndef WIZDEF_H
#include "wizdef.h"
#endif

class QLineEdit;
class CWizDocumentWebView;
class CWizDatabase;
//

class CWizTitleContainer;

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
