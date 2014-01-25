#ifndef CORE_ABOUTDIALOG_H
#define CORE_ABOUTDIALOG_H

#include <QDialog>

namespace Core {
namespace Internal{

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent);
};

} // namespace Internal
} // namespace Core

#endif // CORE_ABOUTDIALOG_H
