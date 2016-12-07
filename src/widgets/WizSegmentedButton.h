#ifndef WIZSEGMENTEDBUTTON_H
#define WIZSEGMENTEDBUTTON_H

#include <QPushButton>
#include <QIcon>

class WizSegmentedButton : public QPushButton
{
    Q_OBJECT

public:
    explicit WizSegmentedButton(QWidget *parent = 0);
    void setCellSpacing(int nSpacing);
    void addCell(const QIcon& icon);

protected:
    virtual QSize sizeHint() const;
    virtual void paintEvent(QPaintEvent *);
    
private:
    int m_nMargin; // builtin margin of top and bottom
    int m_nArcWidth;
    int m_nSpace;
    QList<QIcon> m_cells;
};

#endif // WIZSEGMENTEDBUTTON_H
