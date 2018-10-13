#include "mainwindow.h"
#include "process/installworker.h"
#include "process/removeworker.h"
#include "maintainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QFile>
#include <QTemporaryFile>
#include <QMessageBox>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator myappTranslator;
    myappTranslator.load(QLocale::system().name(), ":/translations/");
    //myappTranslator.load("vi_VN", ":/translations/");
    a.installTranslator(&myappTranslator);

    qsrand(QDateTime::currentMSecsSinceEpoch());

    qDebug() << a.arguments();
    if (a.arguments().contains("--install")) {
        //Installer mode
        InstallWorker worker;
        if (!worker.startWork()) return 1;

        a.setQuitOnLastWindowClosed(false);
        return a.exec();
    } else if (a.arguments().contains("--remove")) {
        //Remove mode
        RemoveWorker worker;
        if (!worker.startWork()) return 1;

        a.setQuitOnLastWindowClosed(false);
        return a.exec();
    } else if (a.arguments().contains("--uninstallmetadata")) {
        //Modify UI mode
        MaintainWindow w;
        w.show();

        return a.exec();
    } else if (a.arguments().contains("--update")) {
        //Prepare update mode
        QString tempInstallerPath = QDir::tempPath() + "/theinstaller.exe";
        if (QFile::exists(tempInstallerPath)) {
            QFile::remove(tempInstallerPath);
        }
        if (QFile::copy(QApplication::applicationFilePath(), tempInstallerPath)) {
            QProcess::startDetached(tempInstallerPath);
            return 0;
        } else {
            QMessageBox box;
            box.setWindowTitle("Error");
            box.setText("Couldn't prepare for update. We won't be able to update at this time.");
            box.setDetailedText("Here are some things you can try:\n"
                                "- Your antivirus software may be blocking the updater. Try disabling any antivirus software while you update\n"
                                "- Your temporary folder is not able to be written to");
            box.setIcon(QMessageBox::Critical);
            box.exec();
            return 1;
        }
    } else if (QFile(a.applicationDirPath() + "/uninstall.json").exists()) {
        //Prepare uninstall mode
        QString tempInstallerPath = QDir::tempPath() + "/theinstaller.exe";
        if (QFile::exists(tempInstallerPath)) {
            QFile::remove(tempInstallerPath);
        }
        if (QFile::copy(QApplication::applicationFilePath(), tempInstallerPath)) {
            QProcess::startDetached(tempInstallerPath, QStringList() << "--uninstallmetadata" << a.applicationDirPath() + "/uninstall.json");
            return 0;
        } else {
            QMessageBox box;
            box.setWindowTitle("Error");
            box.setText("Couldn't prepare for uninstallation. We won't be able to uninstall at this time.");
            box.setDetailedText("Here are some things you can try:\n"
                                "- Your antivirus software may be blocking the uninstaller. Try disabling any antivirus software while you uninstall\n"
                                "- Your temporary folder is not able to be written to");
            box.setIcon(QMessageBox::Critical);
            box.exec();
            return 1;
        }
    }

    //Install UI mode
    MainWindow w;
    w.show();

    return a.exec();
}

QString calculateSize(quint64 size) {
    QString ret;
    if (size > 1073741824) {
        ret = QString::number(((float) size / 1024 / 1024 / 1024), 'f', 2).append(" GiB");
    } else if (size > 1048576) {
        ret = QString::number(((float) size / 1024 / 1024), 'f', 2).append(" MiB");
    } else if (size > 1024) {
        ret = QString::number(((float) size / 1024), 'f', 2).append(" KiB");
    } else {
        ret = QString::number((float) size, 'f', 2).append(" B");
    }

    return ret;
}

float getDPIScaling() {
    float currentDPI = QApplication::desktop()->logicalDpiX();
    return currentDPI / (float) 96;
}
