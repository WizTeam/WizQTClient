#ifndef CWIZTAGBAR_H
#define CWIZTAGBAR_H

#include <QWidget>
#include <QToolButton>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <memory>
#include <map>
#include "share/wizobject.h"

class CWizExplorerApp;

namespace Core {
namespace Internal {

class CTagItem : public QWidget
{
    Q_OBJECT
public:
    explicit CTagItem(const QString guid, const QString text, QWidget *parent = 0);
    ~CTagItem();

    QString guid();
    QString name();

    bool	isReadOnly() const;
    void	setReadOnly(bool b);

    void setSelected(bool b);
    QSize	sizeHint() const override;
    static int textWidth(const QString text);

signals:
    void deleteTagRequest(const QString& guid);
    void selectedItemChanged(CTagItem* item);


protected:
    void paintEvent(QPaintEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QString m_tagGuid;
    QString m_tagName;
    bool m_readOnly;
    bool m_selected;
    bool m_closeButtonPressed;
    std::shared_ptr<QPixmap> m_pixDeleteNormal;
    std::shared_ptr<QPixmap> m_pixDeletePressed;
};

class CWizTagBar : public QWidget
{
    Q_OBJECT
public:
    explicit CWizTagBar(CWizExplorerApp& app, QWidget *parent = 0);
    ~CWizTagBar();

    void setDocument(const WIZDOCUMENTDATA& doc);

signals:
    void deleteTagRequest(const QString& guid);

public slots:
    void on_deleteTagRequest(const QString& guid);
    void on_lineEditReturnPressed();
    void on_selectedItemChanged(CTagItem* item);

protected:
    void resizeEvent(QResizeEvent* event);

private:
    void reset();
    void applyStyleSheet();
    void addTag(const QString& guid, const QString text);
    void calculateTagWidgetWidth();

private:
    WIZDOCUMENTDATABASE m_doc;
    QLabel* m_label;
    QWidget* m_tagWidget;
    QToolButton* m_btnAdd;
    QHBoxLayout* m_tagLayout;
    QLineEdit* m_lineEdit;
    std::map<QString, CTagItem*> m_mapTagWidgets;
    std::map<QString, QString> m_mapMoreTags;
    CWizExplorerApp& m_app;
};

}}

#endif // CWIZTAGBAR_H
