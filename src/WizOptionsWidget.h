#ifndef WIZOPTIONSWIDGET_H
#define WIZOPTIONSWIDGET_H

#include "share/WizPopupWidget.h"
#include "wiznotesettings.h"

#ifndef Q_OS_MAC
class QLabel;
#endif

class WizOptionsWidget : public WizPopupWidget
{
    Q_OBJECT
public:
    WizOptionsWidget(QWidget* parent);
private:
#ifndef Q_OS_MAC
    QLabel* m_labelRestartForSkin;
#endif
public slots:
    void on_radioAuto_clicked(bool checked);
    void on_radioAlwaysReading_clicked(bool checked);
    void on_radioAlwaysEditing_clicked(bool checked);
    void on_checkAutoSync_clicked(bool checked);
    void on_checkDownloadAllNotesData_clicked(bool checked);
    void on_labelProxy_linkActivated(const QString & link);
#ifndef Q_OS_MAC
    void on_comboSkin_currentIndexChanged(const QString& text);
    void on_labelRestartForSkin_linkActivated(const QString& text);
#endif
Q_SIGNALS:
    void settingsChanged(WizOptionsType type);
#ifndef Q_OS_MAC
    void restartForSettings();
#endif
};

#endif // WIZOPTIONSWIDGET_H
