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
            QMessageBox::warning(nullptr, "Error", "Failed to prepare uninstallation", QMessageBox::Ok, QMessageBox::Ok);
            return 1;
        }
    }

    //Install UI mode
    MainWindow w;
    w.show();

    return a.exec();
}
