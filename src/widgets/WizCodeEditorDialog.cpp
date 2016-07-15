#include "WizCodeEditorDialog.h"
#include "wizdef.h"
#include "utils/pathresolve.h"
#include "share/wizsettings.h"
#include "wizDocumentWebView.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QFile>
#include <QDir>
#include <QPlainTextEdit>
#include <QEvent>
#include <QDebug>
#include <QWebEnginePage>
#include <QApplication>

#include "share/wizGlobal.h"
#include "share/wizwebengineview.h"
#include "share/wizthreads.h"

#define LASTUSEDCODETYPE "LASTUSEDCODETYPE"

WizCodeEditorDialog::WizCodeEditorDialog(CWizExplorerApp& app, CWizDocumentWebView* external, QWidget *parent) :
    QDialog(parent)
  , m_app(app)
  , m_external(external)
  , m_codeBrowser(new WizWebEngineView(this))
{
    m_codeBrowser->addToJavaScriptWindowObject("codeEditor", this);
    m_codeBrowser->addToJavaScriptWindowObject("external", m_external);

    //
    //setAttribute(Qt::WA_DeleteOnClose);
    //setWindowFlags(Qt::WindowStaysOnTopHint);          //could cause fullscreen problem on mac when mainwindow was fullscreen
    setWindowState(windowState() & ~Qt::WindowFullScreen);
    resize(650, 550);
    //
    //
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(5, 5, 5, 5);

    verticalLayout->addWidget(m_codeBrowser);

    QString strFileName = Utils::PathResolve::resourcesPath() + "files/code/insert_code.htm";
    QString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);
    strHtml.replace("Wiz_Language_Replace", tr("Language"));
    strHtml.replace("Wiz_OK_Replace", tr("OK"));
    strHtml.replace("Wiz_Cancel_Replace", tr("Cancel"));
    QUrl url = QUrl::fromLocalFile(strFileName);

    m_codeBrowser->page()->setHtml(strHtml, url);
}

void WizCodeEditorDialog::setCode(const QString& strCode)
{
//    if (!strCode.isEmpty())
//    {
//        //m_codeEditor->page()->mainFrame()->setHtml(strCode);
//        m_codeEditor->setPlainText(strCode);
//        renderCodeToHtml();
//    }
}


void WizCodeEditorDialog::insertHtml(const QString& strResultDiv)
{
    QString strHtml = strResultDiv;
    strHtml.replace("\\", "\\\\");
    strHtml.replace("'", "\\'");
    //
    emit insertHtmlRequest(strHtml);
}

QString WizCodeEditorDialog::getLastCodeType()
{
    QString strLastType = m_app.userSettings().get((LASTUSEDCODETYPE));
    if (!strLastType.isEmpty())
    {
        return strLastType.toLower();
    }

    return "c";
}

void WizCodeEditorDialog::saveLastCodeType(const QString& codeType)
{
    m_app.userSettings().set(LASTUSEDCODETYPE, codeType);
}

static WizCodeEditorDialog* g_instance = NULL;

void WizCodeEditorDialog::showEvent(QShowEvent *)
{
    g_instance = this;
}

void WizCodeEditorDialog::hideEvent(QHideEvent *)
{
    g_instance = NULL;
}

void WizCodeEditorDialog::closeEvent(QCloseEvent *)
{
    g_instance = NULL;
}


bool WizCodeEditorDialog::selectAll()
{
    if (!g_instance)
        return false;
    if (QApplication::activeWindow() != g_instance)
        return false;

    g_instance->m_codeBrowser->page()->triggerAction(QWebEnginePage::SelectAll);
    //
    return true;
}

bool WizCodeEditorDialog::undo()
{
    if (!g_instance)
        return false;
    if (QApplication::activeWindow() != g_instance)
        return false;

    g_instance->m_codeBrowser->page()->triggerAction(QWebEnginePage::Undo);
    //
    return true;
}

bool WizCodeEditorDialog::copy()
{
    if (!g_instance)
        return false;
    if (QApplication::activeWindow() != g_instance)
        return false;

    g_instance->m_codeBrowser->page()->triggerAction(QWebEnginePage::Copy);
    //
    return true;
}

bool WizCodeEditorDialog::cut()
{
    if (!g_instance)
        return false;
    if (QApplication::activeWindow() != g_instance)
        return false;

    g_instance->m_codeBrowser->page()->triggerAction(QWebEnginePage::Cut);
    //
    return true;
}

bool WizCodeEditorDialog::paste()
{
    if (!g_instance)
        return false;
    if (QApplication::activeWindow() != g_instance)
        return false;

    g_instance->m_codeBrowser->page()->triggerAction(QWebEnginePage::Paste);
    //
    return true;
}

