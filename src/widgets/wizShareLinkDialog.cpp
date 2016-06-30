#include "wizShareLinkDialog.h"
#include "sync/token.h"
#include "utils/pathresolve.h"
#include "utils/misc.h"
#include "share/wizsettings.h"
#include <QVBoxLayout>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QTimer>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDebug>
#include "share/wizwebengineview.h"

#define ShareLinkFirstTips "ShareLinkFirstTips"

CWizShareLinkDialog::CWizShareLinkDialog(CWizUserSettings& settings, QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f)
    , m_settings(settings)
    , m_view(new WizWebEngineView(this))
{
    setWindowFlags(Qt::CustomizeWindowHint);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);

    m_view->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    m_view->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    //
    m_view->addToJavaScriptWindowObject("external", this);
    //m_view->addToJavaScriptWindowObject("customObject", this);

    m_animation = new QPropertyAnimation(this, "size", this);
}

CWizShareLinkDialog::~CWizShareLinkDialog()
{
}

QSize CWizShareLinkDialog::sizeHint() const
{
    if (m_settings.locale() == ::WizGetDefaultTranslatedLocal())
    {
        return QSize(630, 337);
    }

    return QSize(541, 335);
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
    QString strToken = Token::token();
    m_view->page()->runJavaScript(QString("setToken('%1')").arg(strToken), [=](const QVariant& vRet){

        emit tokenObtained();

    });
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
    QRect rec = geometry();
    rec.setHeight(nHeight);
    setGeometry(rec);
//    m_animation->stop();
//    m_animation->setDuration(100);
//    m_animation->setStartValue(geometry().size());
//    m_animation->setEndValue(QSize(nWidth, nHeight));
////    m_animation->setEasingCurve(QEasingCurve::InOutQuad);

//    m_animation->start();
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

void CWizShareLinkDialog::copyLink(const QString& link, const QString& callBack)
{
    if (link.isEmpty())
    {
        qDebug() << "[Share link] link is empty, nothing to copy";
        return;
    }

    Utils::Misc::copyTextToClipboard(link);

    if (callBack.isEmpty())
        return;

    m_view->page()->runJavaScript(callBack);
}

QString CWizShareLinkDialog::getShareLinkFirstTips()
{
    return m_settings.get(ShareLinkFirstTips);
}

void CWizShareLinkDialog::setShareLinkFirstTips(const QString& value)
{
    m_settings.set(ShareLinkFirstTips, value);
}

QString CWizShareLinkDialog::getLocalLanguage()
{
    return m_settings.locale();
}

void CWizShareLinkDialog::setFormateISO8601StringParam(const QString& param)
{
    m_formateISO8601StringParam = param;
    emit formateISO8601StringChanged();
}

QString CWizShareLinkDialog::formateISO8601String()
{
    QDateTime date = QDateTime::fromString(m_formateISO8601StringParam, Qt::ISODate);
    if (!date.isValid() || date.isNull())
        return m_formateISO8601StringParam;

    return date.toString(Qt::ISODate);
}

void CWizShareLinkDialog::loadHtml()
{
    QString strFile = Utils::PathResolve::resourcesPath() + "files/share_link/index.html";
    QUrl url = QUrl::fromLocalFile(strFile);
    m_view->load(url);
}


