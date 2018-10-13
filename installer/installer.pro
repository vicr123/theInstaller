#-------------------------------------------------
#
# Project created by QtCreator 2018-07-10T19:50:12
#
#-------------------------------------------------

QT       += core gui network svg winextras
CONFIG   += static

INCLUDEPATH += "C:/Program Files (x86)/zlib/include"
LIBS     += -L"C:/Program Files (x86)/zlib/lib" -lzlibstat
DEFINES += QUAZIP_STATIC ZLIB_WINAPI

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

#This will create an installer for theSlate. Change this URL to a different one to change what is installed.
#Also change the branding files.
DEFINES += INSTALLER_METADATA_URL=\\\"http://vicr123.com/theslate/theinstaller/installer.json\\\"

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    fadestackedwidget.cpp \
    process/installworker.cpp \
    quazip/quazipfile.cpp \
    process/installworker.cpp \
    quazip/quazip.cpp \
    quazip/quazipfile.cpp \
    fadestackedwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    process/installworker.cpp \
    quazip/quazip.cpp \
    quazip/quazipfile.cpp \
    fadestackedwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    quazip/unzip.c \
    quazip/zip.c \
    process/installworker.cpp \
    quazip/quazip.cpp \
    quazip/quazipfile.cpp \
    fadestackedwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    quazip/unzip.c \
    quazip/zip.c \
    process/installworker.cpp \
    quazip/quazip.cpp \
    quazip/quazipfile.cpp \
    quazip/quazipfileinfo.cpp \
    fadestackedwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    quazip/unzip.c \
    quazip/zip.c \
    process/installworker.cpp \
    quazip/quazip.cpp \
    quazip/quazipfile.cpp \
    quazip/quazipfileinfo.cpp \
    quazip/quazipnewinfo.cpp \
    fadestackedwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    quazip/unzip.c \
    quazip/zip.c \
    process/installworker.cpp \
    quazip/qioapi.cpp \
    quazip/quazip.cpp \
    quazip/quazipfile.cpp \
    quazip/quazipfileinfo.cpp \
    quazip/quazipnewinfo.cpp \
    fadestackedwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    quazip/unzip.c \
    quazip/zip.c \
    process/installworker.cpp \
    quazip/JlCompress.cpp \
    quazip/qioapi.cpp \
    quazip/quazip.cpp \
    quazip/quazipfile.cpp \
    quazip/quazipfileinfo.cpp \
    quazip/quazipnewinfo.cpp \
    fadestackedwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    quazip/unzip.c \
    quazip/zip.c \
    maintainwindow.cpp \
    process/removeworker.cpp \
    licensewidget.cpp

HEADERS += \
        mainwindow.h \
    fadestackedwidget.h \
    process/installworker.h \
    quazip/quazipfile.h \
    quazip/quazip_global.h \
    process/installworker.h \
    quazip/quazip.h \
    quazip/quazip_global.h \
    quazip/quazipfile.h \
    fadestackedwidget.h \
    mainwindow.h \
    process/installworker.h \
    quazip/quazip.h \
    quazip/quazip_global.h \
    quazip/quazipfile.h \
    quazip/unzip.h \
    quazip/zip.h \
    fadestackedwidget.h \
    mainwindow.h \
    process/installworker.h \
    quazip/ioapi.h \
    quazip/quazip.h \
    quazip/quazip_global.h \
    quazip/quazipfile.h \
    quazip/unzip.h \
    quazip/zip.h \
    fadestackedwidget.h \
    mainwindow.h \
    process/installworker.h \
    quazip/ioapi.h \
    quazip/minizip_crypt.h \
    quazip/quazip.h \
    quazip/quazip_global.h \
    quazip/quazipfile.h \
    quazip/quazipfileinfo.h \
    quazip/unzip.h \
    quazip/zip.h \
    fadestackedwidget.h \
    mainwindow.h \
    process/installworker.h \
    quazip/ioapi.h \
    quazip/minizip_crypt.h \
    quazip/quazip.h \
    quazip/quazip_global.h \
    quazip/quazipfile.h \
    quazip/quazipfileinfo.h \
    quazip/quazipnewinfo.h \
    quazip/unzip.h \
    quazip/zip.h \
    fadestackedwidget.h \
    mainwindow.h \
    process/installworker.h \
    quazip/ioapi.h \
    quazip/minizip_crypt.h \
    quazip/quazip.h \
    quazip/quazip_global.h \
    quazip/quazipfile.h \
    quazip/quazipfileinfo.h \
    quazip/quazipnewinfo.h \
    quazip/unzip.h \
    quazip/zip.h \
    fadestackedwidget.h \
    mainwindow.h \
    process/installworker.h \
    quazip/ioapi.h \
    quazip/JlCompress.h \
    quazip/minizip_crypt.h \
    quazip/quazip.h \
    quazip/quazip_global.h \
    quazip/quazipfile.h \
    quazip/quazipfileinfo.h \
    quazip/quazipnewinfo.h \
    quazip/unzip.h \
    quazip/zip.h \
    fadestackedwidget.h \
    mainwindow.h \
    maintainwindow.h \
    process/removeworker.h \
    licensewidget.h

FORMS += \
        mainwindow.ui \
    maintainwindow.ui \
    licensewidget.ui

TRANSLATIONS += \
    translations/vi_VN.ts \
    translations/pt_BR.ts \
    translations/en_US.ts \
    translations/en_GB.ts \
    translations/en_AU.ts \
    translations/en_NZ.ts

qtPrepareTool(LUPDATE, lupdate)
genlang.commands = "$$LUPDATE -no-obsolete -source-language en_US $$_PRO_FILE_"

qtPrepareTool(LRELEASE, lrelease)
rellang.commands = "$$LRELEASE -removeidentical $$_PRO_FILE_"
QMAKE_EXTRA_TARGETS = genlang rellang
PRE_TARGETDEPS = genlang rellang

win32 {
    #CONFIG += embed_manifest_exe
    #QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\'
}

RESOURCES += \
    resources.qrc

