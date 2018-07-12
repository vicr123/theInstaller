#include "installworker.h"

InstallWorker::InstallWorker(QObject *parent) : QObject(parent)
{
}

bool InstallWorker::startWork() {
    QLocalSocket* sock = new QLocalSocket();
    QString vendor, name, url, destPath;
    bool isStableStream = true, isGlobalInstall = true;

    QString previousToken;
    for (QString arg : QApplication::arguments()) {
        if (previousToken != "") {
            if (previousToken == "--socket") {
                sock->setServerName(arg);
            } else if (previousToken == "--vendor") {
                vendor = arg;
            } else if (previousToken == "--name") {
                name = arg;
            } else if (previousToken == "--url") {
                url = arg;
            } else if (previousToken == "--destdir") {
                destPath = arg;
            }
            previousToken = "";
        } else {
            if (arg == "--socket" || arg == "--vendor" || arg == "--name" || arg == "--url" || arg == "--destdir") {
                previousToken = arg;
            } else if (arg == "--blueprint") {
                isStableStream = false;
            } else if (arg == "--stable") {
                isStableStream = true;
            } else if (arg == "--local") {
                isGlobalInstall = false;
            } else if (arg == "--global") {
                isGlobalInstall = true;
            }
        }
    }

    if (sock->serverName() == "") {
        qDebug() << "Required argument --socket missing";
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

    if (!packageFile.open() || !packageTemporaryDir.isValid()) {
        return false;
    }
    sock->write(QString("STATUS ").append(tr("Downloading %1...").arg(name)).append("\n").toUtf8());
    sock->write(QString("DEBUG %1").arg(packageFile.fileName()).toUtf8());

    QNetworkRequest req(QUrl((QString) url));
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    req.setHeader(QNetworkRequest::UserAgentHeader, "theInstaller/1.0");
    QNetworkReply* reply = mgr.get(req);
    connect(reply, &QNetworkReply::finished, [=] {
        sock->write(QString("STATUS ").append(tr("Unpacking %1...").arg(name)).append("\n").toUtf8());
        sock->write("PROGRESS 0 0\n");
        sock->write(QString("DEBUG %1").arg(packageTemporaryDir.path()).toUtf8());
    });
    connect(reply, &QNetworkReply::downloadProgress, [=](qint64 bytesReceived, qint64 bytesTotal) {
        sock->write(QString("PROGRESS %1 %2\n").arg(QString::number(bytesReceived), QString::number(bytesTotal)).toUtf8());
    });

    return true;
}
