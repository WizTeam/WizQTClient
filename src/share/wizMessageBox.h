#ifndef CWIZMESSAGEBOX_H
#define CWIZMESSAGEBOX_H

#include <QMessageBox>
#include <QList>

class CWizMessageBox
{
public:
    CWizMessageBox();
    ~CWizMessageBox();

    static QMessageBox::StandardButton critical(QWidget * parent, const QString & title, const QString & text,
                                         QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                         QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    static QMessageBox::StandardButton information(QWidget * parent, const QString & title, const QString & text,
                                            QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    static QMessageBox::StandardButton question(QWidget * parent, const QString & title, const QString & text,
                                         QMessageBox::StandardButtons buttons = QMessageBox::StandardButtons( QMessageBox::Yes | QMessageBox::No ),
                                         QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    static QMessageBox::StandardButton warning(QWidget * parent, const QString & title, const QString & text,
                                        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

private:
    static QString buttonTextFromStandardButton(QMessageBox::StandardButton button);
    static void buttonsFromStandardButtons(QMessageBox::StandardButtons buttons, QList<QMessageBox::StandardButton>& buttonList);
    static QMessageBox::StandardButton messageBox(QWidget * parent, const QString & title, const QString & text,
                                         QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton,
                                                QMessageBox::Icon icon);


};

class CMessageBox : public QMessageBox
{
    Q_OBJECT
public:
    explicit CMessageBox(QWidget *parent = 0) : QMessageBox(parent) { }

signals:
    void resized();

protected:
    void resizeEvent(QResizeEvent* event);
};

#endif // CWIZMESSAGEBOX_H
