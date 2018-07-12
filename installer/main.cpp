#include "mainwindow.h"
#include "process/installworker.h"
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>

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
    }

    MainWindow w;
    w.show();

    return a.exec();
}
