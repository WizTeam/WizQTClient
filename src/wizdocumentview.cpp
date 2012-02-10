#include "wizdocumentview.h"
#include "wizdocumentwebview.h"

#include <QWebView>
#include <QWebElement>
#include <QWebFrame>

#include <QBoxLayout>
#include <QLineEdit>


class CWizTitleContainer : public QWidget
{
public:
    CWizTitleContainer(QWidget* parent)
        : QWidget(parent)
    {
        QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
        setLayout(layout);
        layout->setMargin(0);

        setContentsMargins(4, 4, 4, 4);
        m_edit = new QLineEdit(this);
        layout->addWidget(m_edit);
        //
        m_edit->setStyleSheet("QLineEdit{border-color:#ffffff;border-width:1;border-style:solid;}QLineEdit:hover{border-color:#bbbbbb;border-width:1;border-style:solid;}");
        setStyleSheet("border-bottom-width:1;border-bottom-style:solid;border-bottom-color:#bbbbbb");
    }
private:
    QLineEdit* m_edit;
public:
    QLineEdit* edit() const { return m_edit; }
    void setText(const QString& str) { m_edit->setText(str); }
};


CWizDocumentView::CWizDocumentView(CWizExplorerApp& app, QWidget* parent)
    : QWidget(parent)
    , m_db(app.database())
    , m_title(new CWizTitleContainer(this))
    , m_web(new CWizDocumentWebView(app, this))
    , m_client(NULL)
{
    m_client = createClient();
    //
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    setLayout(layout);
    layout->addWidget(m_client);
    layout->setMargin(0);
    //
    connect(m_title->edit(), SIGNAL(textEdited(QString)), this, SLOT(on_title_textEdited(QString)));
}


QWidget* CWizDocumentView::createClient()
{
    QWidget* client = new QWidget(this);
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, client);
    client->setLayout(layout);
    //
    client->setAutoFillBackground(true);
    //
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0xff, 0xff, 0xff));
    client->setPalette(pal);
    //
    //
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_title);
    layout->addWidget(m_web);
    //
    layout->setStretchFactor(m_title, 0);
    layout->setStretchFactor(m_web, 1);
    //
    return client;
}
void CWizDocumentView::showClient(bool visible)
{
    if (visible)
    {
        m_client->show();
    }
    else
    {
        m_client->hide();
    }
}

bool CWizDocumentView::viewDocument(const WIZDOCUMENTDATA& data)
{
    bool ret = m_web->viewDocument(data);
    if (!ret)
    {
        showClient(false);
        return false;
    }
    //
    showClient(true);
    //
    m_title->setText(data.strTitle);
    return true;
}
bool CWizDocumentView::newDocument()
{
    return m_web->newDocument();
}

const WIZDOCUMENTDATA& CWizDocumentView::document()
{
    static WIZDOCUMENTDATA empty;
    if (!isVisible())
        return empty;
    //
    return m_web->document();
}

void CWizDocumentView::on_title_textEdited ( const QString & text )
{
    QString title = text;
    if (title.length() > 255)
    {
        title = title.left(255);
    }
    //
    WIZDOCUMENTDATA data;
    if (m_db.DocumentFromGUID(m_web->document().strGUID, data))
    {
        data.strTitle = title;
        m_db.ModifyDocumentInfo(data);
    }
}
