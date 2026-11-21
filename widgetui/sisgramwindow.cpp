#include "sisgramwindow.h"

#include "apicredentials.h"
#include "systemname.h"

#include <QCoreApplication>
#include <QTimer>
#include <QTextCursor>
#include <QListWidgetItem>
#include <QRegExp>
#include <QDir>

SisGramWindow::SisGramWindow(QWidget *parent)
    : QMainWindow(parent)
    , client(0)
    , avatars(0)
    , folders(0)
    , dialogs(0)
    , messages(0)
    , stack(0)
    , introPage(0)
    , phonePage(0)
    , codePage(0)
    , mainPage(0)
    , apiIdEdit(0)
    , apiHashEdit(0)
    , startButton(0)
    , phoneEdit(0)
    , codeButton(0)
    , codeEdit(0)
    , signInButton(0)
    , dialogList(0)
    , messageView(0)
    , messageEdit(0)
    , sendButton(0)
    , backButton(0)
    , titleLabel(0)
    , statusLabel(0)
{
    setWindowTitle("sisGram");

    client = new TgClient(this);
    avatars = new AvatarDownloader(this);
    avatars->setClient(client);

    folders = new FoldersModel(this);
    folders->setClient(client);

    dialogs = new DialogsModel(this);
    dialogs->setClient(client);
    dialogs->setAvatarDownloader(avatars);
    dialogs->setFolders(folders);

    messages = new MessagesModel(this);
    messages->setClient(client);
    messages->setAvatarDownloader(avatars);

    connect(client, SIGNAL(initialized(bool)), this, SLOT(onInitialized(bool)));
    connect(client, SIGNAL(authorized(TgLongVariant)), this, SLOT(onAuthorized(TgLongVariant)));
    connect(client, SIGNAL(authSentCodeResponse(TgObject,TgLongVariant)), this, SLOT(onAuthSentCodeResponse(TgObject,TgLongVariant)));
    connect(client, SIGNAL(authAuthorizationResponse(TgObject,TgLongVariant)), this, SLOT(onAuthAuthorizationResponse(TgObject,TgLongVariant)));
    connect(client, SIGNAL(rpcError(qint32,QString,TgLongVariant)), this, SLOT(onRpcError(qint32,QString,TgLongVariant)));
    connect(client, SIGNAL(socketError(qint32,QString)), this, SLOT(onSocketError(qint32,QString)));
    connect(client, SIGNAL(disconnected(bool)), this, SLOT(onDisconnected(bool)));
    connect(client, SIGNAL(tfaRequired()), this, SLOT(onTfaRequired()));

    connect(dialogs, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rebuildDialogs()));
    connect(dialogs, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(rebuildDialogs()));
    connect(dialogs, SIGNAL(modelReset()), this, SLOT(rebuildDialogs()));
    connect(messages, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rebuildMessages()));
    connect(messages, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(rebuildMessages()));
    connect(messages, SIGNAL(modelReset()), this, SLOT(rebuildMessages()));

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    // ---- intro / API credentials -------------------------------------------------
    introPage = new QWidget(stack);
    QVBoxLayout *introLayout = new QVBoxLayout(introPage);
    QLabel *introTitle = new QLabel("<b>sisGram</b>", introPage);
    introTitle->setAlignment(Qt::AlignCenter);
    introLayout->addWidget(introTitle);
    introLayout->addWidget(new QLabel("Unofficial Telegram client for Symbian 3rd FP1.\nA fork of Kutegram.", introPage));
    introLayout->addWidget(new QLabel("Telegram API ID / hash\nCreate an app at my.telegram.org/apps and enter its ID and hash once.\nThey are stored on this device only.", introPage));

    QFormLayout *apiForm = new QFormLayout();
    apiIdEdit = new QLineEdit(introPage);
    apiIdEdit->setText(QString::number(kgApiId()));
    apiHashEdit = new QLineEdit(introPage);
    apiHashEdit->setText(kgApiHash());
    apiForm->addRow("API ID", apiIdEdit);
    apiForm->addRow("API hash", apiHashEdit);
    introLayout->addLayout(apiForm);

    startButton = new QPushButton("Connect", introPage);
    introLayout->addWidget(startButton);
    QLabel *introStatus = new QLabel(introPage);
    introLayout->addWidget(introStatus);
    introLayout->addStretch(1);
    statusLabels << introStatus;
    connect(startButton, SIGNAL(clicked()), this, SLOT(onStart()));

    // ---- phone number ------------------------------------------------------------
    phonePage = new QWidget(stack);
    QVBoxLayout *phoneLayout = new QVBoxLayout(phonePage);
    phoneLayout->addWidget(new QLabel("<b>Your Phone Number</b>", phonePage));
    phoneLayout->addWidget(new QLabel("Please, enter your phone number.", phonePage));
    phoneEdit = new QLineEdit(phonePage);
    phoneLayout->addWidget(phoneEdit);
    codeButton = new QPushButton("Send code", phonePage);
    phoneLayout->addWidget(codeButton);
    QLabel *phoneStatus = new QLabel(phonePage);
    phoneLayout->addWidget(phoneStatus);
    phoneLayout->addStretch(1);
    statusLabels << phoneStatus;
    connect(codeButton, SIGNAL(clicked()), this, SLOT(onSendCode()));

    // ---- confirmation code -------------------------------------------------------
    codePage = new QWidget(stack);
    QVBoxLayout *codeLayout = new QVBoxLayout(codePage);
    codeLayout->addWidget(new QLabel("<b>Confirmation Code</b>", codePage));
    codeLayout->addWidget(new QLabel("Please, enter your confirmation code.", codePage));
    codeEdit = new QLineEdit(codePage);
    codeLayout->addWidget(codeEdit);
    signInButton = new QPushButton("Sign in", codePage);
    codeLayout->addWidget(signInButton);
    QLabel *codeStatus = new QLabel(codePage);
    codeLayout->addWidget(codeStatus);
    codeLayout->addStretch(1);
    statusLabels << codeStatus;
    connect(signInButton, SIGNAL(clicked()), this, SLOT(onSignIn()));

    // ---- chat list / conversation ------------------------------------------------
    mainPage = new QWidget(stack);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainPage);

    QHBoxLayout *topLayout = new QHBoxLayout();
    backButton = new QPushButton("<", mainPage);
    backButton->setFixedWidth(30);
    titleLabel = new QLabel("sisGram", mainPage);
    topLayout->addWidget(backButton);
    topLayout->addWidget(titleLabel, 1);
    mainLayout->addLayout(topLayout);

    QHBoxLayout *contentLayout = new QHBoxLayout();
    dialogList = new QListWidget(mainPage);
    messageView = new QTextEdit(mainPage);
    messageView->setReadOnly(true);
    contentLayout->addWidget(dialogList, 1);
    contentLayout->addWidget(messageView, 2);
    mainLayout->addLayout(contentLayout, 1);

    QHBoxLayout *editLayout = new QHBoxLayout();
    messageEdit = new QLineEdit(mainPage);
    sendButton = new QPushButton("Send", mainPage);
    editLayout->addWidget(messageEdit, 1);
    editLayout->addWidget(sendButton);
    mainLayout->addLayout(editLayout);

    statusLabel = new QLabel(mainPage);
    mainLayout->addWidget(statusLabel);
    statusLabels << statusLabel;

    connect(dialogList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onDialogClicked(QListWidgetItem*)));
    connect(sendButton, SIGNAL(clicked()), this, SLOT(onSendMessage()));
    connect(messageEdit, SIGNAL(returnPressed()), this, SLOT(onSendMessage()));
    connect(backButton, SIGNAL(clicked()), this, SLOT(onBack()));

    stack->addWidget(introPage);
    stack->addWidget(phonePage);
    stack->addWidget(codePage);
    stack->addWidget(mainPage);
    showPage(introPage);

    if (client->hasSession()) {
        setBusy(true);
        client->start();
    }
}

void SisGramWindow::showPage(QWidget *page)
{
    stack->setCurrentWidget(page);
}

void SisGramWindow::showStatus(const QString &text)
{
    for (int i = 0; i < statusLabels.size(); ++i)
        statusLabels.at(i)->setText(text);
    setWindowTitle(text.isEmpty() ? QString("sisGram") : QString("sisGram - %1").arg(text));
}

void SisGramWindow::setBusy(bool busy)
{
    startButton->setEnabled(!busy);
    codeButton->setEnabled(!busy);
    signInButton->setEnabled(!busy);
    showStatus(busy ? "Connecting..." : QString());
}

void SisGramWindow::onStart()
{
    bool ok = false;
    int id = apiIdEdit->text().toInt(&ok);
    QString hash = apiHashEdit->text();
    hash.remove(QRegExp("\\s"));
    if (!ok || id <= 0 || hash.length() < 20) {
        showStatus("Enter the API ID and hash from my.telegram.org/apps.");
        return;
    }

    kgSetApiCredentials(id, hash);
    setBusy(true);
    client->resetSession();
    client->start();
}

void SisGramWindow::onSendCode()
{
    QString number = phoneEdit->text();
    number.remove(' ');
    number.remove('-');
    if (number.isEmpty()) {
        showStatus("Invalid phone number. Please try again.");
        return;
    }

    phoneNumber = number;
    phoneCodeHash.clear();
    setBusy(true);
    client->authSendCode(phoneNumber);
}

void SisGramWindow::onSignIn()
{
    QString code = codeEdit->text().trimmed();
    if (code.isEmpty()) {
        showStatus("You have entered an invalid code.");
        return;
    }

    setBusy(true);
    client->authSignIn(phoneNumber, phoneCodeHash, code);
}

void SisGramWindow::onSendMessage()
{
    QString text = messageEdit->text();
    if (text.trimmed().isEmpty())
        return;

    messages->sendMessage(text);
    messageEdit->clear();
}

void SisGramWindow::onDialogClicked(QListWidgetItem *item)
{
    if (!item)
        return;

    int row = item->data(Qt::UserRole).toInt();
    QModelIndex index = dialogs->index(row, 0);
    if (!index.isValid())
        return;

    messages->setPeer(dialogs->data(index, dialogs->roleNames().key("peerBytes")).toByteArray());
    titleLabel->setText(dialogs->data(index, dialogs->roleNames().key("title")).toString());
    messageView->clear();
    rebuildMessages();
    dialogList->hide();
    messageView->show();
    messageEdit->setFocus();
}

void SisGramWindow::onBack()
{
    dialogList->show();
    titleLabel->setText("sisGram");
}

void SisGramWindow::onInitialized(bool hasUserId)
{
    if (hasUserId)
        return;

    setBusy(false);
    showPage(phonePage);
    client->helpGetCountriesList();
}

void SisGramWindow::onAuthorized(TgLongVariant)
{
    setBusy(false);
    showPage(mainPage);
    rebuildDialogs();
}

void SisGramWindow::onAuthSentCodeResponse(TgObject data, TgLongVariant)
{
    setBusy(false);
    phoneCodeHash = data["phone_code_hash"].toString();
    showPage(codePage);
}

void SisGramWindow::onAuthAuthorizationResponse(TgObject data, TgLongVariant)
{
    setBusy(false);
    if (data["_"].toInt() == 0x44747e9a)
        showStatus("Sign up isn't supported. Please use the official app for signing up.");
}

void SisGramWindow::onRpcError(qint32 errorCode, QString errorMessage, TgLongVariant)
{
    setBusy(false);
    if (errorMessage == "PHONE_NUMBER_INVALID")
        showStatus("Invalid phone number. Please try again.");
    else if (errorMessage == "PHONE_NUMBER_FLOOD")
        showStatus("Phone is used too many times recently.");
    else if (errorMessage == "PHONE_CODE_INVALID")
        showStatus("You have entered an invalid code.");
    else
        showStatus(QString("RPC error: %1 (%2)").arg(errorMessage).arg(errorCode));
}

void SisGramWindow::onSocketError(qint32 errorCode, QString errorMessage)
{
    showStatus(QString("Socket error: %1 (%2)").arg(errorMessage).arg(errorCode));
}

void SisGramWindow::onDisconnected(bool hasUserId)
{
    if (!hasUserId) {
        setBusy(false);
        showPage(introPage);
    } else {
        showStatus("Reconnecting...");
        QTimer::singleShot(1000, client, SLOT(start()));
    }
}

void SisGramWindow::onTfaRequired()
{
    setBusy(false);
    showStatus("2FA isn't supported now. Disable 2FA, log in, then enable it again.");
}

void SisGramWindow::rebuildDialogs()
{
    if (!dialogList)
        return;

    dialogList->clear();
    int titleRole = dialogs->roleNames().key("title");
    int textRole = dialogs->roleNames().key("messageText");
    int timeRole = dialogs->roleNames().key("messageTime");

    for (int i = 0; i < dialogs->rowCount(); ++i) {
        QModelIndex index = dialogs->index(i, 0);
        QString title = dialogs->data(index, titleRole).toString();
        QString preview = dialogs->data(index, textRole).toString();
        QString time = dialogs->data(index, timeRole).toString();
        QString line = title;
        if (!time.isEmpty())
            line += "  " + time;
        if (!preview.isEmpty())
            line += "\n" + preview;

        QListWidgetItem *item = new QListWidgetItem(line, dialogList);
        item->setData(Qt::UserRole, i);
    }
}

void SisGramWindow::rebuildMessages()
{
    if (!messageView)
        return;

    int textRole = messages->roleNames().key("messageText");
    int senderRole = messages->roleNames().key("senderName");
    int timeRole = messages->roleNames().key("messageTime");

    QString plain;
    for (int i = 0; i < messages->rowCount(); ++i) {
        QModelIndex index = messages->index(i, 0);
        QString sender = messages->data(index, senderRole).toString();
        QString time = messages->data(index, timeRole).toString();
        QString text = messages->data(index, textRole).toString();
        if (!plain.isEmpty())
            plain += "\n";
        plain += sender;
        if (!time.isEmpty())
            plain += " [" + time + "]";
        plain += ": " + text;
    }

    messageView->setPlainText(plain);
    QTextCursor cursor = messageView->textCursor();
    cursor.movePosition(QTextCursor::End);
    messageView->setTextCursor(cursor);
}
