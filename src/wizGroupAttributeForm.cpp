#include "wizGroupAttributeForm.h"

#include <QPushButton>
#include "wizmainwindow.h"

CWizGroupAttributeForm::CWizGroupAttributeForm(CWizExplorerApp& app, QWidget* parent)
    : QFrame(parent)
    , m_app(app)
    , m_web(new QWebView(parent))
{
    setFrameShadow(QFrame::Raised);
    setFrameShape(QFrame::StyledPanel);
    setAutoFillBackground(true);
    setAttribute(Qt::WA_MacShowFocusRect, true);


    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addWidget(m_web);

    QPushButton* btnClose = new QPushButton(this);
    btnClose->setDefault(true);
    btnClose->setIcon(::WizLoadSkinIcon(app.userSettings().skin(), palette().window().color(), "sheetHide"));
    btnClose->setFlat(true);
    btnClose->setMaximumHeight(18);
    btnClose->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(btnClose, SIGNAL(clicked()), SLOT(on_btnClose_clicked()));

    layout->addWidget(btnClose);
    layout->setAlignment(btnClose, Qt::AlignHCenter);

    connect(&m_api, SIGNAL(clientLoginDone()), SLOT(on_clientLoginDone()));
}

void CWizGroupAttributeForm::on_btnClose_clicked()
{
    sheetHide();
}

void CWizGroupAttributeForm::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);

    raise();
}

void CWizGroupAttributeForm::sheetShow(const QString& strKbGUID)
{
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());

    int width = mainWindow->client()->size().width();
    const QPoint& startP = mainWindow->client()->mapTo(mainWindow, QPoint(0, -sizeHint().height()));
    setGeometry(startP.x(), startP.y(), width, 380);

    const QPoint& endP = mainWindow->client()->mapTo(mainWindow, QPoint(0, 0));

    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
    animation->setDuration(250);
    animation->setEndValue(endP);
    animation->start();

    m_web->setHtml("");

    m_strKbGUID = strKbGUID;
    QString strUserId = m_app.databaseManager().db().getUserId();
    QString strPasswd = m_app.databaseManager().db().getPassword();
    m_api.callClientLogin(strUserId, strPasswd);

    show();
}

void CWizGroupAttributeForm::sheetHide()
{
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    const QPoint& endP = mainWindow->client()->mapTo(mainWindow, QPoint(0, -sizeHint().height()));

    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
    connect(animation, SIGNAL(stateChanged(QAbstractAnimation::State, QAbstractAnimation::State)),
            SLOT(onAnimationSheetHideStateChanged(QAbstractAnimation::State, QAbstractAnimation::State)));

    animation->setDuration(300);
    animation->setEndValue(endP);
    animation->start();
}

void CWizGroupAttributeForm::onAnimationSheetHideStateChanged(QAbstractAnimation::State newState,
                                                              QAbstractAnimation::State oldState)
{
    if (newState == QAbstractAnimation::Stopped
            && oldState == QAbstractAnimation::Running) {
        hide();
    }
}

void CWizGroupAttributeForm::on_clientLoginDone()
{
    QString strExt = QString("token=%1&kb_guid=%2").arg(WizGlobal()->token()).arg(m_strKbGUID);
    QByteArray bytes = QUrl::toPercentEncoding(strExt);
    QString strUrl = WizApiGetUrl("wiz", WIZ_CLIENT_VERSION, "view_group", bytes);

    // use raw url
    QUrl url;
    url.setEncodedUrl(strUrl.toAscii());

    m_web->load(url);
}
