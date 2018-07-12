#include "maintainwindow.h"
#include "ui_maintainwindow.h"

MaintainWindow::MaintainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MaintainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(this->size());
    backgroundImage = QIcon(":/background.svg").pixmap(this->size());

    taskbarButton = new QWinTaskbarButton(this);

    ui->performUninstallButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_VistaShield));

    QFile metadataFile(QApplication::applicationDirPath() + "/uninstall.json");
    metadataFile.open(QFile::ReadOnly);
    metadata = QJsonDocument::fromJson(metadataFile.readAll()).object();
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
