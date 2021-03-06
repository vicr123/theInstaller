#include "maintainwindow.h"
#include "ui_maintainwindow.h"

extern float getDPIScaling();

MaintainWindow::MaintainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MaintainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(this->size() * getDPIScaling());
    backgroundImage = QIcon(":/background.svg").pixmap(this->size());

    ui->topSpacer1->changeSize(ui->topSpacer1->sizeHint().width(), ui->topSpacer1->sizeHint().height() * getDPIScaling(), QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->topSpacer2->changeSize(ui->topSpacer2->sizeHint().width(), ui->topSpacer2->sizeHint().height() * getDPIScaling(), QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->topSpacer3->changeSize(ui->topSpacer3->sizeHint().width(), ui->topSpacer3->sizeHint().height() * getDPIScaling(), QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->topSpacer4->changeSize(ui->topSpacer4->sizeHint().width(), ui->topSpacer4->sizeHint().height() * getDPIScaling(), QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->topSpacer5->changeSize(ui->topSpacer5->sizeHint().width(), ui->topSpacer5->sizeHint().height() * getDPIScaling(), QSizePolicy::Preferred, QSizePolicy::Fixed);

    taskbarButton = new QWinTaskbarButton(this);

    ui->performUninstallButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_VistaShield));

    QString previousToken;
    for (QString arg : QApplication::arguments()) {
        if (previousToken != "") {
            if (previousToken == "--uninstallmetadata") {
                this->metadataFile = arg;
            }
            previousToken = "";
        } else {
            if (arg == "--uninstallmetadata") {
                previousToken = arg;
            }
        }
    }

    QFile metadataFile(this->metadataFile);
    metadataFile.open(QFile::ReadOnly);
    metadata = QJsonDocument::fromJson(metadataFile.readAll()).object();

    this->setWindowTitle(tr("Modify %1").arg(metadata.value("name").toString()));
    ui->areYouSureText->setText(tr("Are you sure you want to uninstall %1?").arg(metadata.value("name").toString()));
}

MaintainWindow::~MaintainWindow()
{
    delete ui;
}

void MaintainWindow::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.drawPixmap(0, 0, backgroundImage);
}

void MaintainWindow::on_uninstallButton_clicked()
{
    //Check for executables inside destination directory
    QDir destdir(metadata.value("installPath").toString());
    QStringList executableNames;
    QDirIterator iterator(destdir, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        iterator.next();
        QFileInfo executable = iterator.fileInfo();
        if (executable.suffix() == "exe") {
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
                    //GetModuleBaseName(proc, mod, processName, sizeof(processName) / sizeof(TCHAR));
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

    ui->stack->setCurrentIndex(1);
}

void MaintainWindow::on_cancelUninstallButton_clicked()
{
    ui->stack->setCurrentIndex(0);
}

void MaintainWindow::on_performUninstallButton_clicked()
{

    this->setWindowFlag(Qt::WindowCloseButtonHint, false);
    this->show();

    taskbarButton->progress()->reset();
    taskbarButton->progress()->setRange(0, 0);
    ui->stack->setCurrentIndex(2);
    ui->statusLabel->setText(tr("Getting ready to uninstall %1...").arg(metadata.value("name").toString()));

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
                    ui->modifyProgress->setMaximum(parts.at(2).toLongLong());
                    ui->modifyProgress->setValue(parts.at(1).toLongLong());

                    taskbarButton->progress()->setMaximum(parts.at(2).toLongLong());
                    taskbarButton->progress()->setValue(parts.at(1).toLongLong());
                } else if (parts.at(0) == "DEBUG") {
                    parts.takeFirst();
                    qDebug() << parts.join(" ");
                } else if (parts.at(0) == "COMPLETE") {
                    modifyDone = true;
                    this->setWindowFlag(Qt::WindowCloseButtonHint, true);
                    this->show();
                    ui->stack->setCurrentIndex(3);

                    taskbarButton->progress()->setVisible(false);
                }
            }
        });
        connect(sock, &QLocalSocket::disconnected, [=] {
            if (!modifyDone) {
                this->setWindowFlag(Qt::WindowCloseButtonHint, true);
                this->show();
                ui->stack->setCurrentIndex(4);
                taskbarButton->progress()->stop();
            }
        });

        socketServer->close();
    });

    if (QApplication::arguments().contains("--server-only")) {
        QMessageBox::warning(this, "Socket Server", socketServer->serverName(), QMessageBox::Ok, QMessageBox::Ok);
    } else {
        QStringList args;
        args.append("\"--remove\"");
        args.append("\"--socket " + socketServer->serverName() + "\"");
        args.append("\"--uninstallmetadata \"\"" + metadataFile + "\"\"\"");

        QStringList psArgs;
        psArgs.append("-FilePath \"" + QApplication::applicationFilePath() + "\"");
        psArgs.append("-ArgumentList (" + args.join(",") + ")");
        psArgs.append("-Verb runAs");
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

                ui->stack->setCurrentIndex(4);
                taskbarButton->progress()->stop();
            }
            proc->deleteLater();
        });
    }
}

void MaintainWindow::showEvent(QShowEvent *event) {
    taskbarButton->setWindow(this->windowHandle());
    taskbarButton->progress()->setVisible(true);
}

void MaintainWindow::on_finishButton_clicked()
{
    this->close();
}

void MaintainWindow::on_exitButton_clicked()
{
    this->close();
}

void MaintainWindow::on_retryInstallButton_clicked()
{
    ui->stack->setCurrentIndex(0);
}
