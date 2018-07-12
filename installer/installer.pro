#-------------------------------------------------
#
# Project created by QtCreator 2018-07-10T19:50:12
#
#-------------------------------------------------

QT       += core gui network svg
CONFIG   += static

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = installer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#DEFINES += INSTALLER_METADATA_URL=\\\"http://localhost:8000/installer.json\\\"
DEFINES += INSTALLER_METADATA_URL=\\\"http://vicr123.com/theslate/theinstaller/installer.json\\\"

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    fadestackedwidget.cpp \
    process/installworker.cpp

HEADERS += \
        mainwindow.h \
    fadestackedwidget.h \
    process/installworker.h

FORMS += \
        mainwindow.ui

TRANSLATIONS += \
    translations/vi_VN.ts

win32 {
    #CONFIG += embed_manifest_exe
    #QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\'
}

RESOURCES += \
    resources.qrc

