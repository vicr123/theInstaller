#ifndef REMOVEWORKER_H
#define REMOVEWORKER_H

#include <QObject>
#include <QLocalSocket>
#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QTimer>
#include <QStandardPaths>
#include <QSettings>

class RemoveWorker : public QObject
{
    Q_OBJECT
public:
    explicit RemoveWorker(QObject *parent = nullptr);

signals:

public slots:
    bool startWork();
};

#endif // REMOVEWORKER_H
