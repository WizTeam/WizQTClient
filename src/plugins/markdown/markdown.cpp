#include "markdown.h"

#include <QtPlugin>
#include <QDebug>
#include <QDir>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QWebFrame>

#include <coreplugin/icore.h>

#include "../../wizDocumentView.h"
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

    return copyRes2Cache();
}

void MarkdownPlugin::extensionsInitialized()
{
    connect(Core::ICore::instance(), SIGNAL(viewNoteLoaded(Core::INoteView*,WIZDOCUMENTDATA,bool)),
            SLOT(onViewNoteLoaded(Core::INoteView*,WIZDOCUMENTDATA,bool)));
    connect(Core::ICore::instance(), SIGNAL(frameRenderRequested(QWebFrame*)),
            SLOT(onFrameRenderRequested(QWebFrame*)));
}

void MarkdownPlugin::onViewNoteLoaded(INoteView* view, const WIZDOCUMENTDATA& doc, bool bOk)
{
    if (!bOk)
        return;

    if (canRender(view, doc))
        render(view->noteFrame());
}

void MarkdownPlugin::onFrameRenderRequested(QWebFrame* frame)
{
    if (frame)
    {
        render(frame);
    }
}

bool MarkdownPlugin::canRender(INoteView* view, const WIZDOCUMENTDATA& doc)
{
    if (view->isEditing())
        return false;

    if (doc.strTitle.indexOf(".md") == -1)
        return false;

    if (doc.strTitle.indexOf(".md ") != -1)
        return true;

    if (doc.strTitle.lastIndexOf(".md") == doc.strTitle.length() - 3)
        return true;

    return false;
}

void MarkdownPlugin::render(QWebFrame* frame)
{
    Q_ASSERT(frame);

    QFile f(":/res/WizNote-Markdown.js");
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "[Markdown]Failed to get render execute code";
        return;
    }

    QTextStream ts(&f);
    QString strExec = ts.readAll();
    f.close();

    Q_ASSERT(strExec.indexOf("${CACHE_PATH}") != -1);
    QString strPath = cachePath() + "plugins/markdown/";
    QDir dir;
    dir.mkpath(strPath);
    strExec.replace("${CACHE_PATH}", strPath);

    frame->evaluateJavaScript(strExec);
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

bool MarkdownPlugin::copyRes2Cache()
{
    QString strPath = cachePath() + "plugins/markdown/";
    QDir cacheDir(strPath);
    cacheDir.mkpath(strPath);

    QStringList lsRes;
    lsRes << ":/res/WizNote-Markdown.js" << ":/res/wiznote-markdown-inject.js";

    QString strMarkdownPath = strPath + "markdown/";
    cacheDir.mkpath(strMarkdownPath);
    QString strGoogleCodePath = strPath + "google-code-prettify/";
    cacheDir.mkpath(strGoogleCodePath);

    lsRes <<":/res/google-code-prettify/lang-yaml.js"
            <<":/res/google-code-prettify/lang-xq.js"
            <<":/res/google-code-prettify/lang-wiki.js"
            <<":/res/google-code-prettify/lang-vhdl.js"
            <<":/res/google-code-prettify/lang-vb.js"
            <<":/res/google-code-prettify/lang-tex.js"
            <<":/res/google-code-prettify/lang-tcl.js"
            <<":/res/google-code-prettify/lang-sql.js"
            <<":/res/google-code-prettify/lang-scala.js"
            <<":/res/google-code-prettify/lang-rd.js"
            <<":/res/google-code-prettify/lang-r.js"
            <<":/res/google-code-prettify/lang-proto.js"
            <<":/res/google-code-prettify/lang-pascal.js"
            <<":/res/google-code-prettify/lang-n.js"
            <<":/res/google-code-prettify/lang-mumps.js"
            <<":/res/google-code-prettify/lang-ml.js"
            <<":/res/google-code-prettify/lang-matlab.js"
            <<":/res/google-code-prettify/lang-lua.js"
            <<":/res/google-code-prettify/lang-llvm.js"
            <<":/res/google-code-prettify/lang-lisp.js"
            <<":/res/google-code-prettify/lang-hs.js"
            <<":/res/google-code-prettify/lang-go.js"
            <<":/res/google-code-prettify/lang-erlang.js"
            <<":/res/google-code-prettify/lang-dart.js"
            <<":/res/google-code-prettify/lang-css.js"
            <<":/res/google-code-prettify/lang-clj.js"
            <<":/res/google-code-prettify/lang-basic.js"
            <<":/res/google-code-prettify/lang-apollo.js"
            <<":/res/google-code-prettify/run_prettify.js"
            <<":/res/google-code-prettify/prettify.js"
            <<":/res/google-code-prettify/prettify.css"
            <<":/res/markdown/github2.css"
            <<":/res/markdown/marked.min.js"
            <<":/res/markdown/jquery.min.js";

    for (int i = 0; i < lsRes.size(); i++) {
        QString strInter = lsRes.at(i);
        QFile f(strInter);
        if (!f.open(QIODevice::ReadOnly)) {
            Q_ASSERT(0);
            return false;
        }

        QString strName = strInter.remove(0, 6);
        QFile f2(cacheDir.filePath(strName));
        if (!f2.open(QIODevice::Truncate|QIODevice::WriteOnly)) {
            qDebug() << "[Markdown]failed to write cache: " << f2.fileName();
            return false;
        }

        f2.write(f.readAll());
        f.close();
        f2.close();
    }


    return true;
}


} // namespce Internal
} // namespce Markdown

Q_EXPORT_PLUGIN(Markdown::Internal::MarkdownPlugin)
