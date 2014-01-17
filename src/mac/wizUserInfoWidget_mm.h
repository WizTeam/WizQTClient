#ifndef WIZUSERINFOWIDGET_H
#define WIZUSERINFOWIDGET_H


#include <QMacCocoaViewContainer>

class CWizExplorerApp;
class CWizDatabase;

class CWizUserInfoWidget : public QMacCocoaViewContainer
{
    Q_OBJECT

public:
    explicit CWizUserInfoWidget(CWizExplorerApp& app, QWidget *parent = 0);

protected:
    CWizExplorerApp& m_app;
    CWizDatabase& m_db;
    QString m_text;

    virtual QSize sizeHint() const;

    QString text() const;
    void setText(QString val);

    void resetUserInfo();

};



#endif // WIZUSERINFOWIDGET_H
