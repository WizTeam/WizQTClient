#ifndef WIZSVGEDITORDIALOG_H
#define WIZSVGEDITORDIALOG_H

#include "WizWebSettingsDialog.h"

class WizSvgEditorDialog: public WizWebSettingsDialog
{
    Q_OBJECT
public:
    WizSvgEditorDialog(QString url, QString data, std::function<void(QString)> callback, QWidget* parent = nullptr);
    //
private:
    QString m_data;
    std::function<void(QString)> m_callback;
protected:
    virtual void onLoaded(bool ok);
public:
    Q_INVOKABLE void saveSvg(QString data);
};


#endif // WIZSVGEDITORDIALOG_H
