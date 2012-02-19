#include "wizdocumentview.h"
#include "wizdocumentwebview.h"

#include <QWebView>
#include <QWebElement>
#include <QWebFrame>

#include <QBoxLayout>


CWizTitleContainer::CWizTitleContainer(QWidget* parent)
        : QWidget(parent)
        , m_bLocked(true)
{
    //QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    QHBoxLayout* layout = new QHBoxLayout(this);
    setLayout(layout);
    layout->setMargin(0);

    setContentsMargins(4, 4, 4, 4);

    m_edit = new QLineEdit(this);
    QIcon icon(WizGetSkinResourceFileName("lock"));
    m_lockBtn = new QPushButton(icon, "", this);

    layout->addWidget(m_edit);
    layout->addWidget(m_lockBtn);
    //
    m_edit->setStyleSheet("QLineEdit{padding:2 2 2 2;border-color:#ffffff;border-width:1;border-style:solid;}QLineEdit:hover{border-color:#bbbbbb;border-width:1;border-style:solid;}");
    setStyleSheet("border-bottom-width:1;border-bottom-style:solid;border-bottom-color:#bbbbbb");
}

void CWizTitleContainer::on_unlockBtnClicked()
{
    if (!m_bLocked) {
        QIcon icon(WizGetSkinResourceFileName("lock"));
        m_lockBtn->setIcon(icon);
        m_bLocked = true;
    } else {
        QIcon icon(WizGetSkinResourceFileName("unlock"));
        m_lockBtn->setIcon(icon);
        m_bLocked = false;
    }
}

void CWizTitleContainer::setLock()
{
    QIcon icon(WizGetSkinResourceFileName("lock"));
    m_lockBtn->setIcon(icon);
    m_bLocked = true;
}


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

    // when lock/unlock button clicked, reset document, reset icon
    connect(m_title->unlock(), SIGNAL(clicked()), m_web, SLOT(on_unlockBtnCliked()));
    connect(m_title->unlock(), SIGNAL(clicked()), m_title, SLOT(on_unlockBtnClicked()));

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
    m_title->setLock();
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
