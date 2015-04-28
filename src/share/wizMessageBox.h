#ifndef CWIZMESSAGEBOX_H
#define CWIZMESSAGEBOX_H

#include <QMessageBox>

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
};

#endif // CWIZMESSAGEBOX_H
