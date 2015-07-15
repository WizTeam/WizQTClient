#ifndef CORE_INFOBAR_H
#define CORE_INFOBAR_H

#include <QWidget>

struct WIZDOCUMENTDATA;
class QLabel;
class CWizExplorerApp;

namespace Core {
namespace Internal {

class InfoBar : public QWidget
{
    Q_OBJECT

public:
    explicit InfoBar(CWizExplorerApp& app, QWidget *parent);
    void setDocument(const WIZDOCUMENTDATA& data);

private:
    QLabel* m_labelCreatedTime;
    QLabel* m_labelModifiedTime;
    QLabel* m_labelOwner;
    QLabel* m_labelSize;
    CWizExplorerApp& m_app;
};

} // namespace Internal
} // namespace Core

#endif // CORE_INFOBAR_H
