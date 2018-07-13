#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(this->size());
    backgroundImage = QIcon(":/background.svg").pixmap(this->size());

    taskbarButton = new QWinTaskbarButton(this);

    //ui->appIcon->setPixmap(QIcon(":/icon.svg").pixmap(128, 128));
    ui->installButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_VistaShield));
    ui->installButton_2->setIcon(QApplication::style()->standardIcon(QStyle::SP_VistaShield));

    getInstallerMetadata();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::getInstallerMetadata() {
    ui->stack->setCurrentIndex(0);

    QTimer::singleShot(1000, [=] {
        QNetworkRequest req(QUrl(INSTALLER_METADATA_URL));
        req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
        QNetworkReply* reply = mgr.get(req);
        connect(reply, &QNetworkReply::finished, [=] {
            if (reply->error() != QNetworkReply::NoError) return;

            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (!doc.isObject()) {
                ui->metadataErrorLabel->setText(tr("Invalid metadata was received"));
                ui->stack->setCurrentIndex(1);
                return;
            }

            QJsonObject obj = doc.object();
            if (!obj.contains("name") || !obj.contains("vendor")) {
                ui->metadataErrorLabel->setText(tr("Application name not in metadata"));
                ui->stack->setCurrentIndex(1);
                return;
            }

            if (!obj.contains("blueprint")) {
                ui->streamLabel->setVisible(false);
                ui->streamContainer->setVisible(false);
            }

            ui->openCheckbox->setText(tr("Open %1").arg(obj.value("name").toString()));
            this->setWindowTitle(tr("Install %1").arg(obj.value("name").toString()));

            this->metadata = obj;
            setInstallPath();

            ui->stack->setCurrentIndex(2);
        });
        connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), [=](QNetworkReply::NetworkError code) {
            ui->metadataErrorLabel->setText(tr("Couldn't retrieve metadata"));
            ui->stack->setCurrentIndex(1);
        });
        connect(reply, &QNetworkReply::sslErrors, [=](QList<QSslError> errors) {
            ui->metadataErrorLabel->setText(tr("Couldn't connect securely to the server"));
            ui->stack->setCurrentIndex(1);
        });
    });
}

void MainWindow::setInstallPath() {
    QString tail = metadata.value("vendor").toString() + "\\" + metadata.value("name").toString() + "\\";
    if (ui->installEveryone->isChecked()) {
        ui->installPathLineEdit->setText(qgetenv("PROGRAMFILES") + "\\" + tail);
        ui->installButton_2->setIcon(QApplication::style()->standardIcon(QStyle::SP_VistaShield));
    } else {
        ui->installPathLineEdit->setText(qgetenv("LOCALAPPDATA") + "\\Programs\\" + tail);
        ui->installButton_2->setIcon(QIcon());
    }
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.drawPixmap(0, 0, backgroundImage);
}

void MainWindow::on_installOptions_clicked()
{
    ui->stack->setCurrentIndex(3);
}

void MainWindow::on_cancelMetadataButton_clicked()
{
    this->close();
}

void MainWindow::on_retryMetadataButton_clicked()
{
    getInstallerMetadata();
}

void MainWindow::on_installEveryone_toggled(bool checked)
{
    setInstallPath();
}

void MainWindow::on_installButton_clicked()
{
    this->setWindowFlag(Qt::WindowCloseButtonHint, false);
    this->show();

    taskbarButton->progress()->reset();
    taskbarButton->progress()->setRange(0, 0);
    ui->stack->setCurrentIndex(4);
    ui->statusLabel->setText(tr("Getting ready to install %1...").arg(metadata.value("name").toString()));

    QLocalServer* socketServer = new QLocalServer();

    QString serverNumber = QString::number(qrand());
    if (QApplication::arguments().contains("--server-only")) {
        serverNumber = QApplication::arguments().at(QApplication::arguments().indexOf("--server-only") + 1);
    }

    socketServer->listen("theinstaller." + metadata.value("vendor").toString() + "." + metadata.value("name").toString() + "." + serverNumber);
    connect(socketServer, &QLocalServer::newConnection, [=] {
        QLocalSocket* sock = socketServer->nextPendingConnection();

        connect(sock, &QLocalSocket::readyRead, [=] {
            QStringList lines = QString(sock->readAll()).split("\n");
            for (QString line : lines) {
                QStringList parts = line.split(" ");
                if (parts.at(0) == "PING") {
                    QMessageBox::warning(this, "Ping!", "Ping received", QMessageBox::Ok, QMessageBox::Ok);
                } else if (parts.at(0) == "STATUS") {
                    parts.takeFirst();
                    ui->statusLabel->setText(parts.join(" "));
                } else if (parts.at(0) == "PROGRESS") {
                    ui->installProgress->setMaximum(parts.at(2).toLongLong());
                    ui->installProgress->setValue(parts.at(1).toLongLong());

                    taskbarButton->progress()->setMaximum(parts.at(2).toLongLong());
                    taskbarButton->progress()->setValue(parts.at(1).toLongLong());
                } else if (parts.at(0) == "DEBUG") {
                    parts.takeFirst();
                    qDebug() << parts.join(" ");
                } else if (parts.at(0) == "COMPLETE") {
                    installDone = true;
                    this->setWindowFlag(Qt::WindowCloseButtonHint, true);
                    this->show();
                    ui->stack->setCurrentIndex(5);

                    taskbarButton->progress()->setVisible(false);
                } else if (parts.at(0) == "ALERT") {
                    parts.takeFirst();
                    QMessageBox::warning(this, tr("Warning"), parts.join(" "), QMessageBox::Ok, QMessageBox::Ok);
                }
            }
        });
        connect(sock, &QLocalSocket::disconnected, [=] {
            if (!installDone) {
                this->setWindowFlag(Qt::WindowCloseButtonHint, true);
                this->show();
                ui->stack->setCurrentIndex(6);
                taskbarButton->progress()->stop();
            }
        });

        socketServer->close();
    });

    if (QApplication::arguments().contains("--server-only")) {
        QMessageBox::warning(this, "Socket Server", socketServer->serverName(), QMessageBox::Ok, QMessageBox::Ok);
    } else {
        QStringList args;
        args.append("\"--install\"");
        args.append("\"--socket " + socketServer->serverName() + "\"");
        args.append("\"--vendor " + metadata.value("vendor").toString() + "\"");
        args.append("\"--name " + metadata.value("name").toString() + "\"");

        QString destdir = ui->installPathLineEdit->text();
        if (destdir.endsWith("\\")) {
            destdir.append("\\");
        }
        args.append("\"--destdir \"\"" + destdir + "\"\"\"");

        if (ui->stableStream->isChecked()) {
            args.append("\"--stable\"");
            args.append("\"--url " + metadata.value("stable").toObject().value("packageUrl").toString() + "\"");
            args.append("\"--executable " + metadata.value("stable").toObject().value("executableName").toString() + "\"");
        } else {
            args.append("\"--blueprint\"");
            args.append("\"--url " + metadata.value("blueprint").toObject().value("packageUrl").toString() + "\"");
            args.append("\"--executable " + metadata.value("blueprint").toObject().value("executableName").toString() + "\"");
        }

        if (ui->installEveryone->isChecked()) {
            args.append("\"--global\"");
        } else {
            args.append("\"--local\"");
        }

        QStringList psArgs;
        psArgs.append("-FilePath \"" + QApplication::applicationFilePath() + "\"");
        psArgs.append("-ArgumentList (" + args.join(",") + ")");
        if (ui->installEveryone->isChecked()) {
            psArgs.append("-Verb runAs");
        }
        psArgs.append("-PassThru");
        psArgs.append("-Wait");

        QString encodedCommand = "Start-Process " + psArgs.join(" ") + "";
        QProcess* proc = new QProcess();
        proc->setProcessChannelMode(QProcess::ForwardedChannels);

        QString command = "powershell -EncodedCommand " + QByteArray::fromRawData((const char*) encodedCommand.utf16(), encodedCommand.size() * 2).toBase64();
        qDebug() << "PowerShell Command: " + encodedCommand;
        qDebug() << "Executing " + command;
        proc->start(command);
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus) {
            this->setWindowFlag(Qt::WindowCloseButtonHint, true);
            this->show();
            if (socketServer->isListening()) {
                socketServer->close();
                socketServer->deleteLater();

                ui->stack->setCurrentIndex(6);
                taskbarButton->progress()->stop();
            }
            proc->deleteLater();
        });
    }
}

void MainWindow::on_installButton_2_clicked()
{
    on_installButton_clicked();
}

void MainWindow::on_exitButton_clicked()
{
    this->close();
}

void MainWindow::on_retryInstallButton_clicked()
{
    getInstallerMetadata();
}

void MainWindow::on_finishButton_clicked()
{
    if (ui->openCheckbox->isChecked()) {
        QString destdir = ui->installPathLineEdit->text();
        if (!destdir.endsWith("\\")) {
            destdir.append("\\");
        }
        QString executable;
        if (ui->stableStream->isChecked()) {
            executable = metadata.value("stable").toObject().value("executableName").toString();
        } else {
            executable = metadata.value("blueprint").toObject().value("executableName").toString();
        }
        QProcess::startDetached(destdir + executable);
    }
    this->close();
}

void MainWindow::showEvent(QShowEvent *event) {
    taskbarButton->setWindow(this->windowHandle());
    taskbarButton->progress()->setVisible(true);
}
