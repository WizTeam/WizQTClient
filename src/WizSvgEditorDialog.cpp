#include "WizSvgEditorDialog.h"
#include "share/WizThreads.h"

WizSvgEditorDialog::WizSvgEditorDialog(QString url, QString data, std::function<void(QString)> callback, QWidget* parent)
    : WizWebSettingsDialog({{"WizQtObject", this}}, url, QSize(), parent)
    , m_data(data)
    , m_callback(callback)
{
    //setWindowFlags(Qt::Window);
}

void WizSvgEditorDialog::onLoaded(bool ok)
{
    if (!ok) {
        return;
    }
    //
    QString data = m_data;
    //
    data = data.replace("\\", "\\\\");
    //
    QString script = QString("wizSvgPainter.importPages(`%1`)").arg(data);
    web()->page()->runJavaScript(script);
}

void WizSvgEditorDialog::saveSvg(QString data)
{
    WizExecuteOnThread(WIZ_THREAD_MAIN, [=] {
        m_callback(data);
    });
}

