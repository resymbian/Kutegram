#ifndef SISGRAMWINDOW_H
#define SISGRAMWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QWidget>
#include <QString>
#include <QList>

#include "tgclient.h"
#include "tgstream.h"
#include "avatardownloader.h"
#include "foldersmodel.h"
#include "dialogsmodel.h"
#include "messagesmodel.h"

// Qt 4.6 / S60 3rd Edition FP1 front end.  The protocol code (libkg) is
// unchanged; only the QML UI is replaced by plain widgets because Qt Quick
// never shipped for S60 3rd (max official Qt there is 4.6.3).
class SisGramWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SisGramWindow(QWidget *parent = 0);

private slots:
    void onStart();
    void onSendCode();
    void onSignIn();
    void onSendMessage();
    void onDialogClicked(QListWidgetItem *item);
    void onBack();

    void onInitialized(bool hasUserId);
    void onAuthorized(TgLongVariant userId);
    void onAuthSentCodeResponse(TgObject data, TgLongVariant messageId);
    void onAuthAuthorizationResponse(TgObject data, TgLongVariant messageId);
    void onRpcError(qint32 errorCode, QString errorMessage, TgLongVariant messageId);
    void onSocketError(qint32 errorCode, QString errorMessage);
    void onDisconnected(bool hasUserId);
    void onTfaRequired();

    void rebuildDialogs();
    void rebuildMessages();

private:
    void setBusy(bool busy);
    void showStatus(const QString &text);
    void showPage(QWidget *page);

    TgClient *client;
    AvatarDownloader *avatars;
    FoldersModel *folders;
    DialogsModel *dialogs;
    MessagesModel *messages;

    QStackedWidget *stack;
    QWidget *introPage;
    QWidget *phonePage;
    QWidget *codePage;
    QWidget *mainPage;

    QLineEdit *apiIdEdit;
    QLineEdit *apiHashEdit;
    QPushButton *startButton;

    QLineEdit *phoneEdit;
    QPushButton *codeButton;

    QLineEdit *codeEdit;
    QPushButton *signInButton;

    QListWidget *dialogList;
    QTextEdit *messageView;
    QLineEdit *messageEdit;
    QPushButton *sendButton;
    QPushButton *backButton;
    QLabel *titleLabel;
    QLabel *statusLabel;
    QList<QLabel*> statusLabels;

    QString phoneNumber;
    QString phoneCodeHash;
};

#endif // SISGRAMWINDOW_H
