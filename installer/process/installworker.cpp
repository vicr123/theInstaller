#include "installworker.h"

extern QString calculateSize(quint64 size);

InstallWorker::InstallWorker(QObject *parent) : QObject(parent)
{
}

bool InstallWorker::startWork() {
    QLocalSocket* sock = new QLocalSocket();
    QString vendor, name, url, destPath, executable;
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
            } else if (previousToken == "--executable") {
                executable = arg;
            }
            previousToken = "";
        } else {
            if (arg == "--socket" || arg == "--vendor" || arg == "--name" || arg == "--url" || arg == "--destdir" || arg == "--executable") {
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
    connect(sock, &QLocalSocket::readyRead, [=] {
        QStringList lines = QString(sock->readAll()).split("\n");
        for (QString line : lines) {
            QStringList parts = line.split(" ");
            if (parts.at(0) == "CANCEL") {
                if (currentlyCancelable) {
                    QApplication::exit(0);
                }
            }
        }
    });

    if (!packageFile.open() || !packageTemporaryDir.isValid()) {
        return false;
    }
    sock->write(QString("STATUS ").append(tr("Downloading %1...").arg(name)).append("\n").toUtf8());
    sock->write(QString("DEBUG %1").arg(packageFile.fileName()).toUtf8());

    QTimer* flipper = new QTimer();
    flipper->setInterval(5000);
    connect(flipper, &QTimer::timeout, [=] {
        emitStatus = !emitStatus;
    });
    flipper->start();

    QNetworkRequest req(QUrl((QString) url));
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    req.setHeader(QNetworkRequest::UserAgentHeader, "theInstaller/1.0");
    QNetworkReply* reply = mgr.get(req);

    lastBytesReceived = 0;
    lastTimeUpdate = QDateTime::fromMSecsSinceEpoch(0);
    connect(reply, &QNetworkReply::finished, [=] {
        packageFile.flush();
        packageFile.seek(0);

        flipper->stop();

        currentlyCancelable = false;
        sock->write(QString("STOPCANCEL\n").toUtf8());
        sock->write(QString("STATUS ").append(tr("Unpacking %1...").arg(name)).append("\n").toUtf8());
        sock->write("PROGRESS 0 0\n");
        sock->write(QString("DEBUG %1").arg(packageTemporaryDir.path()).toUtf8());

        if (QDir(destPath).exists()) {
            QDir(destPath).removeRecursively();
        }
        QDir::root().mkpath(destPath);
        QDir dest(destPath);

        QStringList extracted = JlCompress::extractDir(packageFile.fileName(), destPath);
        if (extracted.length() == 0) {
            //Error occurred
            QApplication::exit(1);
            return;
        }

        sock->write(QString("STATUS ").append(tr("Configuring %1...").arg(name)).append("\n").toUtf8());

        QDir startMenu;
        if (isGlobalInstall) {
            startMenu = QDir("C:/ProgramData/Microsoft/Windows/Start Menu/Programs");
        } else {
            startMenu = QDir(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation));
        }
        startMenu.mkpath(vendor + "/" + name);
        startMenu.cd(vendor + "/" + name);

        QFileInfo executableFile(destPath + "/" + executable);
        QString linkFile = startMenu.absoluteFilePath(executableFile.completeBaseName() + ".lnk");
        if (QFile::exists(linkFile)) {
            QFile::remove(linkFile);
        }
        QFile::link(executableFile.absoluteFilePath(), linkFile);
        QFile::copy(QApplication::applicationFilePath(), dest.absoluteFilePath("uninstall.exe"));

        QJsonObject dataRoot;
        dataRoot.insert("vendor", vendor);
        dataRoot.insert("name", name);
        dataRoot.insert("installPath", destPath);
        dataRoot.insert("global", isGlobalInstall);
        dataRoot.insert("appurl", url);
        dataRoot.insert("stream", isStableStream);
        dataRoot.insert("registryUuid", name);

        QSettings* settings;
        if (isGlobalInstall) {
            settings = new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + name, QSettings::NativeFormat);
        } else {
            settings = new QSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + name, QSettings::NativeFormat);
        }

        settings->clear();
        settings->setValue("DisplayName", name);
        settings->setValue("Publisher", vendor);
        settings->setValue("Contact", vendor);
        settings->setValue("ModifyPath", "\"" + dest.absoluteFilePath("uninstall.exe").replace("/", "\\") + "\"");
        settings->setValue("UninstallString", "\"" + dest.absoluteFilePath("uninstall.exe").replace("/", "\\") + "\"");
        settings->setValue("InstallDate", QDateTime::currentDateTime().toString("yyyyMMdd"));
        settings->setValue("InstallLocation", dest.path());
        settings->setValue("DisplayIcon", executableFile.absoluteFilePath() + ",0");
        settings->sync();
        settings->deleteLater();

        QFile uninstallDataFile(dest.absoluteFilePath("uninstall.json"));
        uninstallDataFile.open(QFile::WriteOnly);
        uninstallDataFile.write(QJsonDocument(dataRoot).toJson());

        sock->write("COMPLETE\n");
        sock->flush();
        sock->waitForBytesWritten();
        QApplication::exit(0);
    });
    connect(reply, &QNetworkReply::readyRead, [=] {
        packageFile.write(reply->readAll());
    });
    connect(reply, &QNetworkReply::downloadProgress, [=](qint64 bytesReceived, qint64 bytesTotal) {
        sock->write(QString("PROGRESS %1 %2\n").arg(QString::number(bytesReceived), QString::number(bytesTotal)).toUtf8());

        if (lastTimeUpdate.toMSecsSinceEpoch() == 0) lastTimeUpdate = QDateTime::currentDateTimeUtc();

        if (emitStatus) {
            sock->write(QString("STATUS ").append(tr("Downloading %1...").arg(name)).append("\n").toUtf8());
        } else {
            QDateTime current = QDateTime::currentDateTimeUtc();

            float speed = (float) bytesReceived / (float) (current.toSecsSinceEpoch() - lastTimeUpdate.toSecsSinceEpoch()); //bytes per second

            qint64 bytesToGo = bytesTotal - bytesReceived;
            int secondsToGo = bytesToGo / speed;

            QString downloaded = tr("%1 of %2").arg(calculateSize(bytesReceived), calculateSize(bytesTotal));
            QString currentSpeed = calculateSize(speed) + "/s";
            QString remainingTime;

            int minutes = secondsToGo / 60;
            int seconds = secondsToGo % 60;
            int hours = minutes / 60;
            minutes = minutes % 60;
            int days = hours / 24;
            hours = hours % 24;

            if (days > 0) {
                remainingTime = tr("%n days remaining", nullptr, days);
            } else if (hours > 0) {
                remainingTime = tr("%n hours remaining", nullptr, hours);
            } else if (minutes > 0) {
                remainingTime = tr("%n minutes remaining", nullptr, minutes);
            } else {
                remainingTime = tr("%n seconds remaining", nullptr, seconds);
            }

            sock->write(QString("STATUS ").append(downloaded + " - " + currentSpeed + " - " + remainingTime).append("\n").toUtf8());
        }
    });

    return true;
}
