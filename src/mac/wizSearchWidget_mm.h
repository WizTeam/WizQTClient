#ifndef WIZSEARCHWIDGET_MM_H
#define WIZSEARCHWIDGET_MM_H


#ifdef USECOCOATOOLBAR

#include <QMacCocoaViewContainer>
#include <QTreeWidget>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QTimer;
class QTreeWidget;
QT_END_NAMESPACE

class WizSuggestCompletionon;

class CWizSearchWidget : public QMacCocoaViewContainer
{
    Q_OBJECT

public:
    CWizSearchWidget(QWidget* parent = 0);
    void clear();
    void focus();
    void setText(const QString& text);

    bool isEditing();

    bool isCompleterVisible();
    void hideCompleter();
    void moveCompleter(bool up);
    QString getCurrentCompleterText();

    virtual QSize sizeHint() const;

    void processEvent(QEvent* ev);

public Q_SLOTS:
    void on_search_editFinished(const QString& strText);
    void on_search_textChanged(const QString& strText);
    void setFocus();
    void clearSearchFocus();

Q_SIGNALS:
    void doSearch(const QString& keywords);
    void textEdited(const QString& text);

private:
    WizSuggestCompletionon* m_completer;
};

class CWizSuggestiongContainer : public QWidget
{
    Q_OBJECT
public:
    CWizSuggestiongContainer(QWidget* parent = 0);

protected:
//    void paintEvent(QPaintEvent* ev);
//    void showEvent(QShowEvent* ev);
};

class CWizSuggestiongList : public QTreeWidget
{
    Q_OBJECT

public:
    CWizSuggestiongList(QWidget* parent = 0);

protected:
    void mouseMoveEvent(QMouseEvent* event);

};


class WizSuggestCompletionon : public QObject
{
    Q_OBJECT

public:
    WizSuggestCompletionon(CWizSearchWidget *parent = 0);
    ~WizSuggestCompletionon();
    bool eventFilter(QObject *obj, QEvent *ev) Q_DECL_OVERRIDE;
    void showCompletion(const QStringList &choices);

    bool isVisible();
    void hide();
    void selectSuggestItem(bool up);
    QString getCurrentText();

public slots:

    void doneCompletion();
    void preventSuggest();
    void autoSuggest();

private:
    void resetContainerSize(int width, int height);

private:
    CWizSearchWidget *m_editor;
    QWidget *m_popupWgt;
    QWidget *m_infoWgt;
    QTreeWidget *m_treeWgt;
    QTimer *m_timer;
};

#endif

#endif // WIZSEARCHWIDGET_MM_H
