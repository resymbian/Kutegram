#include <QApplication>
#include "sisgramwindow.h"

#include "tgclient.h"
#include "tgstream.h"
#include "systemname.h"

#define KG_STRINGIFY_(x) #x
#define KG_STRINGIFY(x) KG_STRINGIFY_(x)

// QML registration is a no-op when QtDeclarative is not linked (Qt 4.6.3),
// but the metatype registrations inside it are still needed for signal
// connections, so register them here explicitly for the widget build.
static void registerTelegramMetaTypes()
{
    qRegisterMetaType<TelegramObject>("TelegramObject");
    qRegisterMetaType<TgVariant>("TgVariant");
    qRegisterMetaType<TgObject>("TgObject");
    qRegisterMetaType<TgMap>("TgMap");
    qRegisterMetaType<TgVector>("TgVector");
    qRegisterMetaType<TgList>("TgList");
    qRegisterMetaType<TgArray>("TgArray");
    qRegisterMetaType<TgInt128>("TgInt128");
    qRegisterMetaType<TgInt256>("TgInt256");
    qRegisterMetaType<TgInt>("TgInt");
    qRegisterMetaType<TgInteger>("TgInteger");
    qRegisterMetaType<TgLong>("TgLong");
    qRegisterMetaType<TgLongLong>("TgLongLong");
    qRegisterMetaType<TgLongVariant>("TgLongVariant");
    qRegisterMetaType<TgDouble>("TgDouble");
    qRegisterMetaType<TgString>("TgString");
    qRegisterMetaType<TgBool>("TgBool");
    qRegisterMetaType<TgByteArray>("TgByteArray");
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

#ifdef Q_OS_SYMBIAN
#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
    QApplication::setAttribute(Qt::AA_S60DisablePartialScreenInputMode, false);
#endif
#endif

    QApplication::setApplicationVersion(QString(KG_STRINGIFY(KUTEGRAM_VERSION)));
    QApplication::setApplicationName("sisGram");
    QApplication::setOrganizationName("sisGram");
    QApplication::setOrganizationDomain("kg.crx.moe");

    registerTelegramMetaTypes();

    SisGramWindow window;
    window.show();

    return app.exec();
}
