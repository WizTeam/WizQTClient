#ifndef WIZSVGEDITORDIALOG_H
#define WIZSVGEDITORDIALOG_H

#include "WizWebSettingsDialog.h"

class WizSvgEditorDialog: public WizWebSettingsDialog
{
    Q_OBJECT
public:
    //WizSvgEditorDialog(QString url, QString data, std::function<void()> manualSaveCallback, std::function<void(QString)> postMessageToNoteEditorCallback, QString htmlFilePath, QWidget* parent = nullptr);
    WizSvgEditorDialog(QString url, QString data, std::function<void(QString html)> saveCallback, std::function<void()> reloadCallback, QString htmlFilePath, QString indexFilesPath, QWidget* parent = nullptr);
    //
private:
    QString m_data;
    std::function<void(QString)> m_saveCallback;
    std::function<void()> m_reloadCallback;
    QString m_htmlFilePath;
    QString m_indexFilesPath;
private:
protected:
    virtual void onLoaded(bool ok);
    virtual void reject();
public:
    Q_INVOKABLE void loadData(QString fileName, QString callback);
    Q_INVOKABLE void saveData(QString options, QString svgData, QString htmlData);
    //Q_INVOKABLE void postMessageToNoteEditor(QString message);
};


#endif // WIZSVGEDITORDIALOG_H
