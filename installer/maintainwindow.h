#ifndef MAINTAINWINDOW_H
#define MAINTAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>
#include <QWinTaskbarButton>
#include <QShowEvent>
#include <QWinTaskbarProgress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QDirIterator>

#include <Windows.h>
#include <Psapi.h>

namespace Ui {
class MaintainWindow;
}

class MaintainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MaintainWindow(QWidget *parent = 0);
    ~MaintainWindow();

private slots:
    void on_uninstallButton_clicked();

    void on_cancelUninstallButton_clicked();

    void on_performUninstallButton_clicked();

    void on_finishButton_clicked();

    void on_exitButton_clicked();

    void on_retryInstallButton_clicked();

private:
    Ui::MaintainWindow *ui;

    void paintEvent(QPaintEvent* event);
    void showEvent(QShowEvent *event);

    QPixmap backgroundImage;
    QWinTaskbarButton* taskbarButton;
    QJsonObject metadata;
    QString metadataFile;
    bool modifyDone = false;
};

#endif // MAINTAINWINDOW_H
