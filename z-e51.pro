# sisGram for S60 3rd Edition FP1 (Nokia E51 and friends).
#
# Kept as a separate project so the Qt 4.7 / Qt Quick build (kutegramquick.pro)
# stays untouched.  The protocol library is shared; only the UI is widgets.
# Build with a Qt 4.6.3 Symbian SDK (the Qt SDK 1.1.x toolchain).

TEMPLATE = app
TARGET = sisGramE51
APPNAME = sisGramE51
VERSION = 1.2.0
PKG_VERSION = 1,2,0

DEFINES += KUTEGRAM_VERSION=$$VERSION

QT += core gui network xml sql
CONFIG += release
CONFIG -= debug
DEFINES += QT_NO_DEBUG_OUTPUT KG_NO_DEBUG KG_NO_INFO

INCLUDEPATH += src

SOURCES += \
    widgetui/main.cpp \
    widgetui/sisgramwindow.cpp \
    src/dialogsmodel.cpp \
    src/messagesmodel.cpp \
    src/avatardownloader.cpp \
    src/foldersmodel.cpp \
    src/currentuserinfo.cpp \
    src/messageutil.cpp \
    src/platformutils.cpp

HEADERS += \
    widgetui/sisgramwindow.h \
    src/apisettings.h \
    src/dialogsmodel.h \
    src/messagesmodel.h \
    src/avatardownloader.h \
    src/foldersmodel.h \
    src/currentuserinfo.h \
    src/messageutil.h \
    src/platformutils.h

include(libkg/libkg.pri)

symbian {
    LIBS += -lavkon -lapgrfx -lcone -leikcore -lapmime

    # Different UID from the Belle build so both SIS files can coexist.
    TARGET.UID3 = 0xE0713D52
    DEFINES += SYMBIAN_UID=$$TARGET.UID3

    # Show the same icon as the Belle build in the S60 application grid.
    ICON = kutegramquick.svg

    TARGET.CAPABILITY += ReadUserData WriteUserData UserEnvironment NetworkServices LocalServices

    supported_platforms = \
            "[0x102032BE],0,0,0,{\"S60ProductID\"}" \ # Symbian 9.2 / S60 3rd FP1
            "[0x102752AE],0,0,0,{\"S60ProductID\"}"   # Symbian 9.3 / S60 3rd FP2

    default_deployment.pkg_prerules -= pkg_platform_dependencies
    supported_platforms_deployment.pkg_prerules += supported_platforms
    DEPLOYMENT += supported_platforms_deployment

    vendor_info = \
        " " \
        "; Localised Vendor name" \
        "%{\"resymbian\"}" \
        " " \
        "; Unique Vendor name" \
        ":\"resymbian\"" \
        " "
    package.pkg_prerules += vendor_info

    DEPLOYMENT += package
    DEPLOYMENT.installer_header = "$${LITERAL_HASH}{\"sisGram E51 Installer\"},(0xE5E0AFB3),$$PKG_VERSION"
}

qtcAddDeployment()
