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

#include <iostream>

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
};

#endif // INSTALLWORKER_H
