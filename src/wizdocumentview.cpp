#include "wizdocumentview.h"
#include "wizdocumentwebview.h"

#include <QWebView>
#include <QWebElement>
#include <QWebFrame>

#include <QBoxLayout>
#include <QLineEdit>
#include <QPushButton>

class CWizTitleBar
    : public QWidget
{
public:
    CWizTitleBar(QWidget* parent)
            : QWidget(parent)
    {
        QHBoxLayout* layout = new QHBoxLayout(this);
        setLayout(layout);
        layout->setMargin(0);

        setContentsMargins(4, 4, 4, 4);

        m_titleEdit = new QLineEdit(this);
        //
        m_editIcon = WizLoadSkinIcon("unlock");
        m_commitIcon = WizLoadSkinIcon("lock");
        //
        m_editDocumentButton = new QPushButton(m_editIcon, "", this);
        updateEditDocumentButtonIcon(false);
        //
        layout->addWidget(m_titleEdit);
        layout->addWidget(m_editDocumentButton);
        //
        m_titleEdit->setStyleSheet("QLineEdit{padding:2 2 2 2;border-color:#ffffff;border-width:1;border-style:solid;}QLineEdit:hover{border-color:#bbbbbb;border-width:1;border-style:solid;}");
    }
private:
    QLineEdit* m_titleEdit;

    QPushButton* m_editDocumentButton;
    //
    QIcon m_editIcon;
    QIcon m_commitIcon;
private:
    void updateEditDocumentButtonIcon(bool editing)
    {
        m_editDocumentButton->setIcon(editing ? m_commitIcon : m_editIcon);
        m_editDocumentButton->setText(editing ? tr("Save && Read") : tr("Edit Note"));
    }
public:
    QLineEdit* titleEdit() const { return m_titleEdit; }
    QPushButton* editDocumentButton() const { return m_editDocumentButton; }
    //
    void setEditingDocument(bool editing) { updateEditDocumentButtonIcon(editing); }
    void setTitle(const QString& str) { m_titleEdit->setText(str); }
};



CWizDocumentView::CWizDocumentView(CWizExplorerApp& app, QWidget* parent)
    : QWidget(parent)
    , m_db(app.database())
    , m_title(new CWizTitleBar(this))
    , m_web(new CWizDocumentWebView(app, this))
    , m_client(NULL)
    , m_editingDocument(true)
    , m_viewMode(viewmodeKeep)
{
    m_client = createClient();
    //
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    setLayout(layout);
    layout->addWidget(m_client);
    layout->setMargin(0);
    //
    m_title->setEditingDocument(m_editingDocument);
    //
    connect(m_title->titleEdit(), SIGNAL(textEdited(QString)), this, SLOT(on_title_textEdited(QString)));
    connect(m_title->editDocumentButton(), SIGNAL(clicked()), this, SLOT(on_editDocumentButton_clicked()));
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
    QWidget* line = new QWidget(this);
    line->setMaximumHeight(1);
    line->setMinimumHeight(1);
    line->setStyleSheet("border-bottom-width:1;border-bottom-style:solid;border-bottom-color:#bbbbbb");
    //
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_title);
    layout->addWidget(line);
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

bool CWizDocumentView::viewDocument(const WIZDOCUMENTDATA& data, bool forceEdit)
{
    bool edit = false;
    if (forceEdit)
    {
        edit = true;
    }
    else
    {
        switch (m_viewMode)
        {
        case viewmodeAlwaysEditing:
            edit = true;
            break;
        case viewmodeAlwaysReading:
            edit = false;
            break;
        default:
            edit = m_editingDocument;
            break;
        }
    }
    //
    bool ret = m_web->viewDocument(data, edit);
    if (!ret)
    {
        showClient(false);
        return false;
    }
    //
    showClient(true);
    editDocument(edit);
    //
    m_title->setTitle(data.strTitle);
    return true;
}

const WIZDOCUMENTDATA& CWizDocumentView::document()
{
    static WIZDOCUMENTDATA empty;
    if (!isVisible())
        return empty;
    //
    return m_web->document();
}

void CWizDocumentView::editDocument(bool editing)
{
    m_editingDocument = editing;
    m_title->setEditingDocument(m_editingDocument);
    m_web->setEditingDocument(m_editingDocument);
}

void CWizDocumentView::on_title_textEdited(const QString & text )
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

void CWizDocumentView::on_editDocumentButton_clicked()
{
    editDocument(!m_editingDocument);
}
