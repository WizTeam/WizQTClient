#ifndef PLUGIN_MARKDOWN_H
#define PLUGIN_MARKDOWN_H

#include <extensionsystem/iplugin.h>

struct WIZDOCUMENTDATA;
namespace Core {
class INoteView;
}

class QWebFrame;
class QWebEnginePage;

namespace Markdown {
namespace Internal {

class MarkdownPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "wiz.plugin.system.internal.markdown")

public:
    MarkdownPlugin();
    ~MarkdownPlugin();

    bool initialize(const QStringList& arguments, QString* errorMessage = 0);
    void extensionsInitialized();

private:
    QString cachePath();
    QString resourcesPath();
    bool copyRes2Cache();
    bool canRender(Core::INoteView* view, const WIZDOCUMENTDATA& data);
#ifdef USEWEBENGINE
    void render(QWebEnginePage* page);
#else
    void render(QWebFrame* frame);
#endif
    void changeCssToInline(QWebFrame* frame);
    QString getExecString();
    void getCustomCssFile();

private Q_SLOTS:
    void onViewNoteLoaded(Core::INoteView* view, const WIZDOCUMENTDATA& doc, bool bOk);
    void onMarkdownSettingChanged();

private:
    QString m_strCssFile;
};

} // namespace Internal
} // namespace Markdown

#endif // PLUGIN_MARKDOWN_H
