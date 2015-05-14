#ifndef CWIZTAGBAR_H
#define CWIZTAGBAR_H

#include <QWidget>

namespace Core {
namespace Internal {

class CTagItem : public QWidget
{
    Q_OBJECT
public:
    explicit CTagItem(const QString text, QWidget *parent = 0);
    ~CTagItem();

    QSize	sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event);
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);
    void mousePressEvent(QMouseEvent* event);

private:
    QString m_text;
    bool m_focused;
    QPixmap m_pixTag;
    QPixmap m_pixDelete;
};

class CWizTagBar : public QWidget
{
    Q_OBJECT
public:
    explicit CWizTagBar(QWidget *parent = 0);
    ~CWizTagBar();

signals:

public slots:
};
}}

#endif // CWIZTAGBAR_H
