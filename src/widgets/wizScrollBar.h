#ifndef WIZSCROLLBAR_H
#define WIZSCROLLBAR_H

#include <QScrollBar>
#include <QPointer>
#include <QTimer>
#include <QListWidget>


class CWizScrollBar : public QScrollBar
{
    Q_OBJECT

public:
    explicit CWizScrollBar(QWidget* parent = 0);
    void syncWith(QScrollBar* source);
    void applyStyle(const QString& bgColorName, const QString& handleColorName, bool leftBorder);

    virtual QSize sizeHint() const;
    virtual void mouseMoveEvent(QMouseEvent* event);

public Q_SLOTS:
    void showHandle();
    void hideHandle();

    void on_sourceValueChanged(int value);
    void on_sourceRangeChanged(int min, int max);
    void on_valueChanged(int value);
    void on_scrollTimeout();

private:
    void setHandleVisible(bool visible);

private:
    QPointer<QScrollBar> m_scrollSyncSource;
    QTimer m_timerScrollTimeout;
    QString m_bgColor;
    QString m_handleColor;
    bool m_bLeftBorder;
};

class CWizListWidgetWithCustomScorllBar : public  QListWidget
{
    Q_OBJECT
public:
    CWizListWidgetWithCustomScorllBar(QWidget* parent = 0);

protected:
    void resizeEvent(QResizeEvent* event);

protected:
    CWizScrollBar* m_vScroll;
};

#endif // WIZSCROLLBAR_H
