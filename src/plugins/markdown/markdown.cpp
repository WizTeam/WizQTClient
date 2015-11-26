#include "markdown.h"

#include <QtPlugin>
#include <QDebug>
#include <QDir>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QWebFrame>
//#include <QWebEnginePage>
#include <QCoreApplication>
#include <QApplication>
#include <QTimer>
#include <QSettings>

#include "coreplugin/icore.h"
#include "coreplugin/inoteview.h"
#include "extensionsystem/pluginmanager.h"

#include "../../share/wizobject.h"

using namespace Core;

namespace Markdown {
namespace Internal {

MarkdownPlugin::MarkdownPlugin()
{
}

MarkdownPlugin::~MarkdownPlugin()
{
}

bool MarkdownPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorMessage);

    getCustomCssFile();
    return copyRes2Cache();
}

void MarkdownPlugin::extensionsInitialized()
{
    connect(Core::ICore::instance(), SIGNAL(viewNoteLoaded(Core::INoteView*,WIZDOCUMENTDATA,bool)),
            SLOT(onViewNoteLoaded(Core::INoteView*,WIZDOCUMENTDATA,bool)));
    connect(Core::ICore::instance(), SIGNAL(markdownSettingChanged()),
            SLOT(onMarkdownSettingChanged()), Qt::QueuedConnection);
}

void MarkdownPlugin::onViewNoteLoaded(INoteView* view, const WIZDOCUMENTDATA& doc, bool bOk)
{
    if (!bOk)
        return;

    if (canRender(view, doc))
    {
#ifdef USEWEBENGINE
        render(view->notePage());
#else
//        render(view->noteFrame());
        QWebFrame* frame = view->noteFrame();
        frame->evaluateJavaScript("renderMarkdown();");
#endif
    }
}

void MarkdownPlugin::onMarkdownSettingChanged()
{
    getCustomCssFile();
}

bool MarkdownPlugin::canRender(INoteView* view, const WIZDOCUMENTDATA& doc)
{
    if (view->isEditing())
        return false;

    if (doc.strTitle.indexOf(".md") == -1 && doc.strTitle.indexOf(".mj") == -1)
        return false;

    int nPointPos = doc.strTitle.length() - 3;
    if (doc.strTitle.lastIndexOf(".md") == nPointPos || doc.strTitle.lastIndexOf(".mj") == nPointPos)
        return true;

    if (doc.strTitle.indexOf(".md ") != -1 || doc.strTitle.indexOf(".md@") != -1 ||
            doc.strTitle.indexOf(".mj ") != -1|| doc.strTitle.indexOf(".mj@") != -1)
        return true;

    return false;
}

#ifdef USEWEBENGINE
void MarkdownPlugin::render(QWebEnginePage* page)
{
    Q_ASSERT(page);

    QString strExec = getExecString();
    page->runJavaScript(strExec);
}
#else
void MarkdownPlugin::render(QWebFrame* frame)
{
//    Q_ASSERT(frame);
    if (!frame)
    {
        qCritical() << "can not find web frame.";
        return;
    }

    QString strExec = getExecString();
    frame->evaluateJavaScript(strExec);
}
#endif

void MarkdownPlugin::changeCssToInline(QWebFrame* frame)
{
    if (frame)
    {
        QString strHtml = frame->toHtml();
        QRegExp regHeadContant("<head[^>]*>[\\s\\S]*</head>");
        QString strPath = cachePath() + "plugins/markdown/";
        QString strNewHead = QString("<head><link rel=\"stylesheet\" href=\"file://" + strPath + "markdown/github2.css\">"
                                     "<script src=\"file://" + strPath +"markdown/jquery.min.js\"></script>"
                                     "<script src=\"file://" + strPath + "inlinecss/jquery.inlineStyler.min.js\"></script>"
                                     "<script src=\"file://" + strPath + "inlinecss/csstoinline.js\"></script></head>");
        strHtml.replace(regHeadContant, strNewHead);
        frame->setHtml(strHtml);
    }
}

// FIXME: about to remove
QString MarkdownPlugin::cachePath()
{
    QString strCachePath = qgetenv("XDG_CACHE_HOME");
    if (strCachePath.isEmpty()) {
#ifdef Q_OS_MAC
    #ifdef BUILD4APPSTORE
        strCachePath = QDir::homePath() + "/Library/Caches/";
    #else
        strCachePath = qgetenv("HOME") + "/.wiznote/cache/";
    #endif
#else
        strCachePath = qgetenv("HOME") + "/.cache/wiznote/";
#endif
    } else {
        strCachePath += "/wiznote/";
    }

    QDir dir;
    dir.mkpath(strCachePath);
    return strCachePath;
}

void addBackslash(QString& strPath)
{
    strPath.replace('\\', '/');

    if (strPath.endsWith('/'))
        return;

    strPath += '/';
}

QString MarkdownPlugin::resourcesPath()
{
    QString strAppPath = QApplication::applicationDirPath();
    addBackslash(strAppPath);
#ifdef Q_OS_MAC
    QDir dir(strAppPath);
    dir.cdUp();
    dir.cd("Resources");
    QString strPath = dir.path();
    addBackslash(strPath);
    return strPath;
#elif defined(Q_OS_LINUX)
    QDir dir(strAppPath);
    dir.cdUp();
    dir.cd("share/wiznote");
    QString strPath = dir.path();
    addBackslash(strPath);
    return strPath;
#else
    return strAppPath;
#endif
}

void copyFolder(QString sourceFolder, QString destFolder)
{
    QDir sourceDir(sourceFolder);
    if(!sourceDir.exists())
        return;
    QDir destDir(destFolder);
    if(!destDir.exists())
    {
        destDir.mkdir(destFolder);
    }
    QStringList files = sourceDir.entryList(QDir::Files);
    for(int i = 0; i< files.count(); i++)
    {
        QString srcName = sourceFolder + "/" + files[i];
        QString destName = destFolder + "/" + files[i];
        if (QFile::exists(destName))
        {
            QFile::remove(destName);
        }
        QFile::copy(srcName, destName);
    }
    files.clear();
    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for(int i = 0; i< files.count(); i++)
    {
        QString srcName = sourceFolder + "/" + files[i];
        QString destName = destFolder + "/" + files[i];
        copyFolder(srcName, destName);
    }
}

bool MarkdownPlugin::copyRes2Cache()
{
    QString destPath = cachePath() + "plugins/markdown/";
    QString sourcePath = resourcesPath() + "files/markdown/";

    QDir cacheDir(destPath);
    cacheDir.mkpath(destPath);
    copyFolder(sourcePath, destPath);

    return true;
}

QString MarkdownPlugin::getExecString()
{
    QString strFile = cachePath() + "plugins/markdown/WizNote-Markdown.js";
    QFile f(strFile);
    if (!f.exists())
    {
        copyRes2Cache();
    }

    if (!f.open(QIODevice::ReadOnly))
    {
        qDebug() << "[Markdown]Failed to get render execute code";
        return "";
    }

    QTextStream ts(&f);
    QString strExec = ts.readAll();
    f.close();

    Q_ASSERT(strExec.indexOf("${CACHE_PATH}") != -1);
    QString strPath = cachePath() + "plugins/markdown/";
    QDir dir;
    dir.mkpath(strPath);
    strExec.replace("${CACHE_PATH}", strPath);
    strExec.replace("${CSS_FILE_PATH}", m_strCssFile);
    return strExec;
}

void MarkdownPlugin::getCustomCssFile()
{
    const QString strCategory = "MarkdownTemplate/";
    QSettings* settings = ExtensionSystem::PluginManager::settings();
    QByteArray ba = QByteArray::fromBase64(settings->value(strCategory + "SelectedItem").toByteArray());
    QString strFile = QString::fromUtf8(ba);
    if (strFile.isEmpty())
    {
        strFile = cachePath() + "plugins/markdown/markdown/github2.css";
    }
    else if (QFile::exists(strFile))
    {
        m_strCssFile = strFile;
    }
    else
    {
        qDebug() << QString("[Markdown] You have choose %1 as you Markdown style template, but"
                            "we can not find this file. Please check wether file exists.").arg(strFile);
        strFile  = cachePath() + "plugins/markdown/markdown/github2.css";
    }
    m_strCssFile = strFile;
}


} // namespce Internal
} // namespce Markdown

Q_EXPORT_PLUGIN(Markdown::Internal::MarkdownPlugin)
