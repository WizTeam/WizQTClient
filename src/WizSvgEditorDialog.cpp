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
    , m_closeAfterSaving(false)
{
#ifdef QT_DEBUG
    //m_url = "http://192.168.1.73:8888/static/index.html?saveToEditor=1&clientType=macosx";
#endif
    setWindowTitle(QObject::tr("Handwriting Note"));
    //
    QString type = "";
    QString a = + "type=" + type;
    m_url = m_url + "&a=" + QUrl::toPercentEncoding(a);
}

void WizSvgEditorDialog::reject()
{
    m_closeAfterSaving = true;
    qDebug() << "---close dialog";
    web()->page()->runJavaScript("wizSvgPainter.save()", [=] (const QVariant& ret) {
        qDebug() << "---request save return";
        if (!ret.toBool()) {
            //没有修改，不会调用saveData, 因此直接关闭
            m_reloadCallback();
            WizWebSettingsDialog::reject();
        }
        //
    });
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
    bool isManual = o["isManual"].asBool();
    //bool isBodyHtml = o["isBodyHtml"].asBool();
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
    //
    if (!htmlData.isEmpty()) {
        //
        ::WizSaveUnicodeTextToUtf8File(m_htmlFilePath, htmlData);
        m_saveCallback(htmlData);
        //
    }
//    //
//    if (isBodyHtml) {
//        //
//        //
//    } else {
//        if (!htmlData.isEmpty()) {
//            postMessageToNoteEditor(htmlData);
//        }
//    }
    //
    if (isManual) {
        //m_manualSaveCallback();
    }
    //
    qDebug() << "---save data done";
    qDebug() << "-----------------";
    //
    if (m_closeAfterSaving) {
        m_reloadCallback();
        WizWebSettingsDialog::reject();
    }
    //
}


//void WizSvgEditorDialog::postMessageToSvgEditor(QString message)
//{
//    message = message.replace("\\", "\\\\");
//    QString js = QString("wizSvgPainter.onMessageFromNoteEditor(`%1`)").arg(message);
//    web()->page()->runJavaScript(js);
//}

//void WizSvgEditorDialog::postMessageToNoteEditor(QString message)
//{
//    m_postMessageToNoteEditorCallback(message);
//}
