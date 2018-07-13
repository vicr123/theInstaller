#ifndef INSTALLWORKER_H
#define INSTALLWORKER_H

#include <QObject>
#include <QTextStream>
#include <QLocalSocket>
#include <QDebug>
#include <QApplication>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QSettings>
#include <QTimer>

#include <quazip/JlCompress.h>

#include <winreg.h>

class InstallWorker : public QObject
{
    Q_OBJECT
public:
    explicit InstallWorker(QObject *parent = nullptr);

signals:

public slots:
    bool startWork();

private:
    QNetworkAccessManager mgr;
    QTemporaryFile packageFile;
    QTemporaryDir packageTemporaryDir;

    int lastBytesReceived;
    QDateTime lastTimeUpdate;
    bool emitStatus = true;
};

#endif // INSTALLWORKER_H
