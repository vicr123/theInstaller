#include "installworker.h"

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
        packageFile.flush();
        packageFile.seek(0);

        sock->write(QString("STATUS ").append(tr("Unpacking %1...").arg(name)).append("\n").toUtf8());
        sock->write("PROGRESS 0 0\n");
        sock->write(QString("DEBUG %1").arg(packageTemporaryDir.path()).toUtf8());

        QDir::root().mkpath(destPath);
        QDir dest(destPath);

        JlCompress::extractDir(packageFile.fileName(), destPath);

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
        QFile::link(executableFile.absoluteFilePath(), startMenu.absoluteFilePath(executableFile.completeBaseName() + ".lnk"));
        QFile::copy(QApplication::applicationFilePath(), dest.absoluteFilePath("uninstall.exe"));

        QJsonObject dataRoot;
        dataRoot.insert("vendor", vendor);
        dataRoot.insert("name", name);
        dataRoot.insert("installPath", destPath);
        dataRoot.insert("global", isGlobalInstall);
        dataRoot.insert("appurl", url);

        //Write uninstall information to registry
        QUuid uuid = QUuid::createUuid();
        dataRoot.insert("registryUuid", uuid.toString());
        /*HKEY SoftwareEntry;
        HKEY hive;
        LPCWSTR keyPath = (LPCWSTR) QString("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + uuid.toString()).utf16();

        if (isGlobalInstall) {
            hive = HKEY_LOCAL_MACHINE;
        } else {
            hive = HKEY_CURRENT_USER;
        }

        LSTATUS createReturn = RegCreateKeyEx(HKEY_LOCAL_MACHINE, keyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &SoftwareEntry, NULL);
        if (createReturn == ERROR_SUCCESS) {
            RegSetValueEx(SoftwareEntry, TEXT("DisplayName"), 0, REG_SZ, (const BYTE*) vendor.toStdString().data(), vendor.count() + 1);

            RegCloseKey(SoftwareEntry);

            sock->write(QString("DEBUG Uninstall GUID: " + uuid.toString()).toUtf8());
        } else {
            sock->write(QString("ALERT " + tr("Error writing uninstall information to the registry: Error 0x%1").arg(QString::number(createReturn, 16)) + "\n").toUtf8());
        }*/

        QSettings* settings;
        if (isGlobalInstall) {
            settings = new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + uuid.toString(), QSettings::NativeFormat);
        } else {
            settings = new QSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + uuid.toString(), QSettings::NativeFormat);
        }

        settings->setValue("DisplayName", name);
        settings->setValue("Publisher", vendor);
        settings->setValue("Contact", vendor);
        settings->setValue("ModifyPath", dest.absoluteFilePath("uninstall.exe"));
        settings->setValue("UninstallString", dest.absoluteFilePath("uninstall.exe"));
        settings->setValue("InstallDate", QDateTime::currentDateTime().toString("yyyymmdd"));
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
    });

    return true;
}
