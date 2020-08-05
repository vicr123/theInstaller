#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QPaintEvent>
#include <QPainter>
#include <QStyle>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QStandardPaths>
#include <QDebug>
#include <QProcess>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMessageBox>
#include <QWinTaskbarProgress>
#include <QWinTaskbarButton>
#include <QShowEvent>
#include "licensewidget.h"

#include <windows.h>
#include <shellapi.h>
#include <Psapi.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    public slots:
        void setAutoProgress(bool autoProgress);

    private slots:
        void on_installOptions_clicked();

        void getInstallerMetadata();

        void setInstallPath();

        void on_cancelMetadataButton_clicked();

        void on_retryMetadataButton_clicked();

        void on_installEveryone_toggled(bool checked);

        void on_installButton_clicked();

        void on_installButton_2_clicked();

        void on_exitButton_clicked();

        void on_retryInstallButton_clicked();

        void on_finishButton_clicked();

        void on_cancelInstallButton_clicked();

        void on_browseInstallPathButton_clicked();

        void on_licenseLabel_linkActivated(const QString &link);

        void on_stableStream_toggled(bool checked);

        void on_blueprintStream_toggled(bool checked);

private:
        Ui::MainWindow *ui;

        void paintEvent(QPaintEvent* event);
        void showEvent(QShowEvent* event);

        QNetworkAccessManager mgr;
        QPixmap backgroundImage;
        QJsonObject metadata;
        QWinTaskbarButton* taskbarButton;
        QLocalSocket* sock = nullptr;
        bool installDone = false;
        bool autoProgress = false;
        LicenseWidget* licenseWidget;
        bool useStableStream = true;

        QMap<QString, QString> licenses;
};

#endif // MAINWINDOW_H
