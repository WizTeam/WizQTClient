#include "wizShareLinkDialog.h"
#include "sync/token.h"
#include "utils/pathresolve.h"
#include <QVBoxLayout>
#include <QWebFrame>
#include <QTimer>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QMessageBox>
#include <QDebug>

CWizShareLinkDialog::CWizShareLinkDialog(QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f)
    , m_view(new QWebView(this))
{
    setWindowFlags(Qt::CustomizeWindowHint);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);

    m_view->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
    m_view->settings()->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, true);
    m_view->settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);
    connect(m_view->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            SLOT(onJavaScriptWindowObject()));

    m_animation = new QPropertyAnimation(this, "size", this);
}

CWizShareLinkDialog::~CWizShareLinkDialog()
{
}

QSize CWizShareLinkDialog::sizeHint() const
{
    return QSize(800, 331);
}

void CWizShareLinkDialog::shareDocument(const WIZDOCUMENTDATA& doc)
{
    m_doc = doc;
    loadHtml();
}

void CWizShareLinkDialog::logAction(const QString& strAction)
{
    qDebug() << "[Share Link] " << strAction;
}

void CWizShareLinkDialog::writeToLog(const QString& strLog)
{
    qDebug() << "[Share Link] " << strLog;
}

void CWizShareLinkDialog::getToken()
{
    QString strToken = WizService::Token::token();
    m_view->page()->mainFrame()->evaluateJavaScript(QString("setToken('%1')").arg(strToken));
    emit tokenObtained();
}

QString CWizShareLinkDialog::getKbGuid()
{
    return m_doc.strKbGUID;
}

QString CWizShareLinkDialog::getGuid()
{
    return m_doc.strGUID;
}

QString CWizShareLinkDialog::getTitle()
{
    return m_doc.strTitle;
}

void CWizShareLinkDialog::resizeEx(int nWidth, int nHeight)
{
//    resize(nWidth, nHeight);

    m_animation->stop();
    m_animation->setDuration(100);
    m_animation->setStartValue(geometry().size());
    m_animation->setEndValue(QSize(nWidth, nHeight));
//    m_animation->setEasingCurve(QEasingCurve::InOutQuad);

    m_animation->start();
}

void CWizShareLinkDialog::openindefaultbrowser(const QString& url)
{
    QDesktopServices::openUrl(QUrl(url));
}

void CWizShareLinkDialog::dragcaption(int x, int y)
{
    QPoint pos = QCursor::pos();
    move(pos.x() - x, pos.y() - y);
}

void CWizShareLinkDialog::copyLink(const QString& link)
{
    if (link.isEmpty())
    {
        qDebug() << "[Share link] link is empty, nothing to copy";
        return;
    }

    QClipboard* clip = QApplication::clipboard();
    QMimeData* data = new QMimeData();
    data->setHtml(link);
    data->setText(link);
    clip->setMimeData(data);
    QMessageBox::information(this, tr("Info"), tr("Link copied successfully!"));
}

void CWizShareLinkDialog::loadHtml()
{
    QString strFile = Utils::PathResolve::resourcesPath() + "files/share_link/index.html";
    QUrl url = QUrl::fromLocalFile(strFile);
    m_view->load(url);
}

void CWizShareLinkDialog::onJavaScriptWindowObject()
{
    m_view->page()->mainFrame()->addToJavaScriptWindowObject("external", this);
    m_view->page()->mainFrame()->addToJavaScriptWindowObject("customObject", this);
}
