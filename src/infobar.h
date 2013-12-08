#ifndef CORE_INFOBAR_H
#define CORE_INFOBAR_H

#include <QWidget>

struct WIZDOCUMENTDATA;
class QLabel;

namespace Core {
namespace Internal {

class InfoBar : public QWidget
{
    Q_OBJECT

public:
    explicit InfoBar(QWidget *parent);
    void setDocument(const WIZDOCUMENTDATA& data);

private:
    QLabel* m_labelCreatedTime;
    QLabel* m_labelModifiedTime;
    QLabel* m_labelAuthor;
    QLabel* m_labelSize;
};

} // namespace Internal
} // namespace Core

#endif // CORE_INFOBAR_H
