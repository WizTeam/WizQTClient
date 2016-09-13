#ifndef WIZTABLESELECTOR_H
#define WIZTABLESELECTOR_H

#include <QWidget>
class QMenu;

class WizTableItemWidget : public QWidget
{
    Q_OBJECT
public:
    WizTableItemWidget(int row, int col, QWidget* parent);
    void setSelected(bool selected);
    bool selected() const { return m_selected; }
    //
protected:
    virtual void mouseMoveEvent(QMouseEvent * e);
    virtual void mousePressEvent(QMouseEvent * e);
    virtual void paintEvent(QPaintEvent *);
Q_SIGNALS:
    void mouseTracked(int row, int col);
    void mousePressed(int row, int col);
private:
    bool m_selected;
    int m_row;
    int m_col;
};

class WizTableSelectorWidget : public QWidget
{
    Q_OBJECT
public:
    WizTableSelectorWidget(QWidget* parent);
public slots:
    void onItemMouseTracked(int currentRow, int currentCol);
Q_SIGNALS:
    void itemSelected(int row, int col);
private:
    static const int COL_COUNT = 10;
    static const int ROW_COUNT = 10;
};

#endif // WIZTABLESELECTOR_H
