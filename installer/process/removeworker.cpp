#include "removeworker.h"

RemoveWorker::RemoveWorker(QObject *parent) : QObject(parent)
{

}

bool RemoveWorker::startWork() {
    QLocalSocket* sock = new QLocalSocket();
    QString metadataFilePath;

    QString previousToken;
    for (QString arg : QApplication::arguments()) {
        if (previousToken != "") {
            if (previousToken == "--socket") {
                sock->setServerName(arg);
            } else if (previousToken == "--uninstallmetadata") {
                metadataFilePath = arg;
            }
            previousToken = "";
        } else {
            if (arg == "--socket" || arg == "--uninstallmetadata") {
                previousToken = arg;
            }
        }
    }

    if (sock->serverName() == "") {
        qDebug() << "Required argument --socket missing";
        return false;
    }

    if (metadataFilePath == "") {
        qDebug() << "Required argument --uninstallmetadata missing";
        return false;
    }

    qDebug() << "Connecting to socket server...";
    sock->connectToServer();
    if (!sock->waitForConnected()) {
        qDebug() << "Failed to connect to socket server";
        return false;
    }
    connect(sock, &QLocalSocket::disconnected, [=] {
        qDebug() << "Socket closed";
        QApplication::exit(1);
    });

    QFile metadataFile(metadataFilePath);
    metadataFile.open(QFile::ReadOnly);
    QJsonObject metadata = QJsonDocument::fromJson(metadataFile.readAll()).object();
    metadataFile.close();

    QString name = metadata.value("name").toString();
    QString vendor = metadata.value("vendor").toString();

    sock->write(QString("STATUS ").append(tr("Removing %1...").arg(name)).append("\n").toUtf8());

    //Remove Start menu entry
    QDir startMenu;
    if (metadata.value("global").toBool()) {
        startMenu = QDir("C:/ProgramData/Microsoft/Windows/Start Menu/Programs");
    } else {
        startMenu = QDir(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation));
    }
    startMenu.cd(vendor + "/" + name);
    startMenu.removeRecursively();
    startMenu.cdUp();
    if (startMenu.entryList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs).count() == 0) {
        startMenu.removeRecursively();
    }

    //Remove items in installation directory
    QDir dest(metadata.value("installPath").toString());
    dest.removeRecursively();

    //Remove registry entry
    QSettings* settings;
    if (metadata.value("global").toBool()) {
        settings = new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", QSettings::NativeFormat);
    } else {
        settings = new QSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", QSettings::NativeFormat);
    }
    settings->remove(metadata.value("registryUuid").toString());
    settings->sync();

    sock->write("COMPLETE\n");
    sock->flush();
    sock->waitForBytesWritten();
    QTimer::singleShot(0, [=] {
        QApplication::exit(0);
    });
    return 0;
}
