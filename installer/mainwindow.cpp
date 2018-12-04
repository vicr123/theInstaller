#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QDirIterator>
#include <QFileDialog>
#include <QJsonArray>

extern float getDPIScaling();

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(this->size() * getDPIScaling());
    backgroundImage = QIcon(":/background.svg").pixmap(this->size());
    licenseWidget = new LicenseWidget(this);
    licenseWidget->hide();

    ui->topSpacer1->changeSize(ui->topSpacer1->sizeHint().width(), ui->topSpacer1->sizeHint().height() * getDPIScaling(), QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->topSpacer2->changeSize(ui->topSpacer2->sizeHint().width(), ui->topSpacer2->sizeHint().height() * getDPIScaling(), QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->topSpacer3->changeSize(ui->topSpacer3->sizeHint().width(), ui->topSpacer3->sizeHint().height() * getDPIScaling(), QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->topSpacer4->changeSize(ui->topSpacer4->sizeHint().width(), ui->topSpacer4->sizeHint().height() * getDPIScaling(), QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->topSpacer5->changeSize(ui->topSpacer5->sizeHint().width(), ui->topSpacer5->sizeHint().height() * getDPIScaling(), QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->topSpacer6->changeSize(ui->topSpacer6->sizeHint().width(), ui->topSpacer6->sizeHint().height() * getDPIScaling(), QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->topSpacer7->changeSize(ui->topSpacer7->sizeHint().width(), ui->topSpacer7->sizeHint().height() * getDPIScaling(), QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->leftSpacer->changeSize(ui->leftSpacer->sizeHint().width() * getDPIScaling(), ui->leftSpacer->sizeHint().height(), QSizePolicy::Fixed, QSizePolicy::Preferred);
    ui->rightSpacer->changeSize(ui->rightSpacer->sizeHint().width() * getDPIScaling(), ui->rightSpacer->sizeHint().height(), QSizePolicy::Fixed, QSizePolicy::Preferred);

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

            //Check that the application is not already installed
            QStringList hives;
            hives << "HKEY_LOCAL_MACHINE" << "HKEY_CURRENT_USER";
            for (QString hive : hives) {
                QSettings checker("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + obj.value("name").toString(), QSettings::NativeFormat);
                if (checker.contains("InstallLocation")) {
                    //We should update the installation instead
                    QFile metadataFile(checker.value("InstallLocation").toString() + "/uninstall.json");
                    metadataFile.open(QFile::ReadOnly);
                    QJsonObject metadata = QJsonDocument::fromJson(metadataFile.readAll()).object();
                    metadataFile.close();

                    ui->installPathLineEdit->setText(metadata.value("installPath").toString());
                    if (metadata.value("global").toBool()) {
                        ui->installEveryone->setChecked(true);
                        ui->installButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_VistaShield));
                    } else {
                        ui->installEveryone->setChecked(false);
                        ui->installButton->setIcon(QIcon());
                    }

                    if (metadata.value("stream").toBool(true)) {
                        ui->stableStream->setChecked(true);
                    } else {
                        ui->stableStream->setChecked(false);
                    }

                    ui->installButton->setText(tr("Update Now"));
                    ui->installOptions->setVisible(false);
                }
            }

            //Check the license
            if (obj.contains("license")) {
                QJsonArray licenses = obj.value("license").toArray();

                QStringList licenseLinks;
                for (QJsonValue l : licenses) {
                    QJsonObject license = l.toObject();
                    QString type = license.value("type").toString();

                    if (type == "GPL3") {
                        QFile text(":/licenses/gpl3.html");
                        text.open(QFile::ReadOnly);
                        this->licenses.insert(tr("GNU General Public License, version 3"), text.readAll());
                        licenseLinks.append(QString("<a href=\"%1\">%1</a>").arg(tr("GNU General Public License, version 3")));
                    } else if (type == "GPL3+") {
                        QFile text(":/licenses/gpl3.html");
                        text.open(QFile::ReadOnly);
                        this->licenses.insert(tr("GNU General Public License, version 3, or later"), text.readAll());
                        licenseLinks.append(QString("<a href=\"%1\">%1</a>").arg(tr("GNU General Public License, version 3, or later")));
                    } else if (type == "GPL2") {
                        QFile text(":/licenses/gpl2.html");
                        text.open(QFile::ReadOnly);
                        this->licenses.insert(tr("GNU General Public License, version 2"), text.readAll());
                        licenseLinks.append(QString("<a href=\"%1\">%1</a>").arg(tr("GNU General Public License, version 2")));
                    } else if (type == "GPL2+") {
                        QFile text(":/licenses/gpl2.html");
                        text.open(QFile::ReadOnly);
                        this->licenses.insert(tr("GNU General Public License, version 2, or later"), text.readAll());
                        licenseLinks.append(QString("<a href=\"%1\">%1</a>").arg(tr("GNU General Public License, version 2, or later")));
                    } else {
                        this->licenses.insert(type, license.value("text").toString());
                        licenseLinks.append(QString("<a href=\"%1\">%1</a>").arg(type));
                    }
                }

                ui->licenseLabel->setText(tr("By installing %1, you're indicating agreement to the terms of the %2.").arg(obj.value("name").toString(), licenseLinks.join(", ")));
                ui->licenseLabel->setVisible(true);
            } else {
                //No license available
                ui->licenseLabel->setVisible(false);
            }

            if (autoProgress) {
                ui->installButton->click();
            } else {
                ui->stack->setCurrentIndex(2);
            }
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
    //Check for executables inside destination directory
    QDir destdir(ui->installPathLineEdit->text());
    QStringList executableNames;
    QDirIterator iterator(destdir, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        iterator.next();
        QFileInfo executable = iterator.fileInfo();
        if (executable.suffix() == "exe" && executable.baseName() != "uninstall") {
            executableNames.append(executable.filePath());
        }
    }

    //Check app isn't running
    bool isOpen = false;

    DWORD processes[1024], needed;
    EnumProcesses(processes, sizeof(processes), &needed);
    DWORD count = needed / sizeof(DWORD);
    for (uint i = 0; i < count; i++) {
        if (processes[i] != 0) {
            DWORD pid = processes[i];
            HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
            if (proc != NULL) {
                HMODULE mod;
                DWORD needed;
                if (EnumProcessModules(proc, &mod, sizeof(mod), &needed)) {
                    TCHAR processName[MAX_PATH] = TEXT("");
                    GetModuleFileNameEx(proc, mod, processName, sizeof(processName) / sizeof(TCHAR));
                    QString name = QString::fromWCharArray(processName).replace("\\", "/");
                    for (QString executable : executableNames) {
                        if (name == executable) isOpen = true;
                    }
                }
            }
        }
    }

    if (isOpen) {
        QMessageBox::warning(this, tr("%1 currently running").arg(metadata.value("name").toString()), tr("Before we continue, you'll need to close %1.").arg(metadata.value("name").toString()), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    this->setWindowFlag(Qt::WindowCloseButtonHint, false);
    this->show();

    taskbarButton->progress()->reset();
    taskbarButton->progress()->setRange(0, 0);
    ui->cancelInstallButton->setVisible(true);
    ui->cancelInstallButton->setEnabled(true);
    ui->stack->setCurrentIndex(4);
    ui->statusLabel->setText(tr("Getting ready to install %1...").arg(metadata.value("name").toString()));

    QLocalServer* socketServer = new QLocalServer();

    QString serverNumber = QString::number(qrand());
    if (QApplication::arguments().contains("--server-only")) {
        serverNumber = QApplication::arguments().at(QApplication::arguments().indexOf("--server-only") + 1);
    }

    socketServer->listen("theinstaller." + metadata.value("vendor").toString() + "." + metadata.value("name").toString() + "." + serverNumber);
    connect(socketServer, &QLocalServer::newConnection, [=] {
        sock = socketServer->nextPendingConnection();

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

                    if (autoProgress) {
                        ui->openCheckbox->setChecked(true);
                        ui->finishButton->click();
                    } else {
                        ui->stack->setCurrentIndex(5);
                    }

                    taskbarButton->progress()->setVisible(false);
                } else if (parts.at(0) == "ALERT") {
                    parts.takeFirst();
                    QMessageBox::warning(this, tr("Warning"), parts.join(" "), QMessageBox::Ok, QMessageBox::Ok);
                } else if (parts.at(0) == "STOPCANCEL") {
                    ui->cancelInstallButton->setVisible(false);
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

        QString start = "\"" + destdir + executable + "\"";
        start.replace("\\", "/");
        qDebug() << "Starting " + start;
        QProcess::startDetached(start);
    }

    this->close();
}

void MainWindow::showEvent(QShowEvent *event) {
    taskbarButton->setWindow(this->windowHandle());
    taskbarButton->progress()->setVisible(true);
}

void MainWindow::on_cancelInstallButton_clicked()
{
    if (sock != nullptr) {
        ui->cancelInstallButton->setEnabled(false);
        sock->write(QString("CANCEL\n").toUtf8());
    }
}

void MainWindow::on_browseInstallPathButton_clicked()
{
    QFileDialog d;
    d.setAcceptMode(QFileDialog::AcceptOpen);
    d.setFileMode(QFileDialog::Directory);
    d.setDirectory(ui->installPathLineEdit->text());
    if (d.exec() == QFileDialog::Accepted) {
        ui->installPathLineEdit->setText(d.selectedFiles().first());
    }
}

void MainWindow::on_licenseLabel_linkActivated(const QString &link)
{
    licenseWidget->show(link, this->licenses.value(link));
}

void MainWindow::setAutoProgress(bool autoProgress) {
    this->autoProgress = autoProgress;
}
