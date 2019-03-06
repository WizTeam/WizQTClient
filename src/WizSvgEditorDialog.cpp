#include "WizSvgEditorDialog.h"
#include "share/WizThreads.h"
#include "share/WizMisc.h"
#include "utils/WizPathResolve.h"
#include "utils/WizMisc.h"
#include "share/jsoncpp/json/json.h"
#include <QFile>


//WizSvgEditorDialog::WizSvgEditorDialog(QString url, QString data, std::function<void()> manualSaveCallback, std::function<void(QString)> postMessageToNoteEditorCallback, QString htmlFilePath, QWidget* parent)
WizSvgEditorDialog::WizSvgEditorDialog(QString url, QString data, std::function<void(QString html)> saveCallback, std::function<void()> reloadCallback, QString htmlFilePath, QString indexFilesPath, QWidget* parent)
    : WizWebSettingsDialog({{"WizQtObject", this}}, url, QSize(), parent)
    , m_data(data)
    , m_saveCallback(saveCallback)
    , m_reloadCallback(reloadCallback)
    //, m_manualSaveCallback(manualSaveCallback)
    //, m_postMessageToNoteEditorCallback(postMessageToNoteEditorCallback)
    , m_htmlFilePath(htmlFilePath)
    , m_indexFilesPath(indexFilesPath)
{
#ifdef QT_DEBUG
    //m_url = "http://192.168.1.73:8888/static/index.html?clientType=macosx";
#endif
    setWindowTitle(QObject::tr("Handwriting Note"));
    //
    QString type = "";
    QString a = + "type=" + type;
    m_url = m_url + "&a=" + QUrl::toPercentEncoding(a);
}

void WizSvgEditorDialog::reject()
{
    web()->page()->runJavaScript("wizSvgPainter.save({closeDialog: true})");
}

void WizSvgEditorDialog::onLoaded(bool ok)
{
    if (!ok) {
        return;
    }
    //
    if (web()->page()->url() == QUrl("about:blank")) {
        return;
    }
    //
    if (m_data.isEmpty()) {
        return;
    }
    //
    QString data = m_data;
    //
    data = data.replace("\\", "\\\\");
    //
    QString script = QString("wizSvgPainter.importData(`%1`)").arg(data);
    web()->page()->runJavaScript(script);
}

void WizSvgEditorDialog::loadData(QString fileName, QString callback)
{
    QString text;
    fileName = m_indexFilesPath + fileName;
    WizLoadUnicodeTextFromFile(fileName, text);
    //
    text = text.replace("\\", "\\\\");
    QString js = QString("%1(`%2`)").arg(callback).arg(text);
    web()->page()->runJavaScript(js);
}

void WizSvgEditorDialog::saveData(QString options, QString svgData, QString htmlData)
{
    Json::Value o;
    Json::Reader reader;
    if (!reader.parse(options.toUtf8().constData(), o))
        return;
    //
    if (!svgData.isEmpty()) {
        //
        QString fileName = QString::fromUtf8(o["fileName"].asString().c_str());
        QString oldFileName = QString::fromUtf8(o["oldFileName"].asString().c_str());
        if (!fileName.isEmpty()) {
            fileName = m_indexFilesPath + fileName;
            ::WizSaveUnicodeTextToUtf8File(fileName, svgData, false);
            //
            if (!oldFileName.isEmpty()) {
                oldFileName = m_indexFilesPath + oldFileName;
                QFile::remove(oldFileName);
            }
        }
    }

    //
    if (!htmlData.isEmpty()) {
        //
        ::WizSaveUnicodeTextToUtf8File(m_htmlFilePath, htmlData);
        m_saveCallback(htmlData);
        //
    }
    //
    bool closeDialog = o["closeDialog"].asBool();
    if (closeDialog) {
        m_reloadCallback();
        WizWebSettingsDialog::reject();
    }
    //
}

