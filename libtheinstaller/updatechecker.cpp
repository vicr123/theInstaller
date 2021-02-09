#include "updatechecker.h"

#include <QApplication>
#include <QFile>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>

struct UpdateCheckerPrivate {
    QUrl metadataUrl;
    QUrl updateDownloadUrl;
    QList<QAction*> checkForUpdateActions;

    enum State {
        Checking,
        Idle,
        NewUpdateAvailable,
        Errored
    };

    State state = Idle;

    int version[4];
    int newUpdateVersion[4];

    bool isStableStream = true;

    QNetworkAccessManager mgr;
};

UpdateChecker::UpdateChecker(QObject *parent) : QObject(parent)
{
    d = new UpdateCheckerPrivate();

    QFile metadataFile(QApplication::applicationDirPath() + "/uninstall.json");
    metadataFile.open(QFile::ReadOnly);
    QJsonObject metadata = QJsonDocument::fromJson(metadataFile.readAll()).object();
    metadataFile.close();

    if (metadata.contains("stream")) {
        d->isStableStream = metadata.value("stream").toBool();
    }
}

QString UpdateChecker::versionString(const int params[])
{
    return QString::number(params[0]) + "." + QString::number(params[1]) + (params[2] == 0 ? "" : ("." + QString::number(params[2])));
}

UpdateChecker::~UpdateChecker()
{
    delete d;
}

void UpdateChecker::initialise(QUrl metadataUrl, QUrl updateDownloadUrl, int major, int minor, int build, int bp)
{
    UpdateChecker::instance()->d->metadataUrl = metadataUrl;
    UpdateChecker::instance()->d->updateDownloadUrl = updateDownloadUrl;
    UpdateChecker::instance()->d->version[0] = major;
    UpdateChecker::instance()->d->version[1] = minor;
    UpdateChecker::instance()->d->version[2] = build;
    UpdateChecker::instance()->d->version[3] = bp;
    UpdateChecker::instance()->checkForUpdates();
}

UpdateChecker *UpdateChecker::instance()
{
    static UpdateChecker* instance = new UpdateChecker();
    return instance;
}

bool UpdateChecker::updatesSupported()
{
#ifndef Q_OS_WIN
    return false;
#endif

    return QFile::exists(QApplication::applicationDirPath() + "/uninstall.json");
}

void UpdateChecker::checkForUpdates()
{
    if (!updatesSupported()) return;

    if (d->state == UpdateCheckerPrivate::NewUpdateAvailable) {
        if (QFile(QApplication::applicationDirPath() + "/uninstall.exe").exists()) {
            //Use theInstaller to update the app
            emit closeAllWindows();

            QProcess::startDetached(QApplication::applicationDirPath() + "/uninstall.exe", {"--update-from-app"});
            return;
        }

        //Open the website to download the latest version
        QDesktopServices::openUrl(d->updateDownloadUrl);
    } else if (d->state == UpdateCheckerPrivate::Idle || d->state == UpdateCheckerPrivate::Errored) {
        d->state = UpdateCheckerPrivate::Checking;

        QNetworkRequest req(d->metadataUrl);
        req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
        QNetworkReply* reply = d->mgr.get(req);
        connect(reply, &QNetworkReply::errorOccurred, [=] {
            d->state = UpdateCheckerPrivate::Errored;
            updateActions();
        });
        connect(reply, &QNetworkReply::finished, [=] {
            d->state = UpdateCheckerPrivate::Idle;
            updateActions();

            if (reply->error() != QNetworkReply::NoError) return;

            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (!doc.isObject()) return;

            QJsonObject obj = doc.object();
            if (!obj.contains("version")) return;

            QJsonObject versions = d->isStableStream ? obj.value("version").toObject() : obj.value("bpVersion").toObject();

            int newVersion[] = {
                versions.value("major").toInt(),
                versions.value("minor").toInt(),
                versions.value("bugfix").toInt(),
                versions.value("bp").toInt()
            };

            bool isNewVersion = false;
            for (int i = 0; i < 4; i++) {
                if (d->version[i] < newVersion[i]) {
                    isNewVersion = true;
                }
            }

            if (isNewVersion) {
                for (int i = 0; i < 4; i++) {
                    d->newUpdateVersion[i] = newVersion[i];
                }
                d->state = UpdateCheckerPrivate::NewUpdateAvailable;
                emit updateAvailable();
            }

            updateActions();
        });

        updateActions();
    }
}

bool UpdateChecker::isUpdateAvailable()
{
    return UpdateChecker::instance()->d->state == UpdateCheckerPrivate::NewUpdateAvailable;
}

QAction *UpdateChecker::checkForUpdatesAction()
{
    QAction* a = new QAction();
    UpdateChecker::instance()->d->checkForUpdateActions.append(a);
    a->setMenuRole(QAction::ApplicationSpecificRole);
    connect(a, &QAction::triggered, UpdateChecker::instance(), [=] {
        if (QApplication::keyboardModifiers() & Qt::AltModifier) UpdateChecker::instance()->d->state = UpdateCheckerPrivate::NewUpdateAvailable;
        UpdateChecker::instance()->checkForUpdates();
    });
    connect(a, &QAction::destroyed, UpdateChecker::instance(), [=] {
        UpdateChecker::instance()->d->checkForUpdateActions.removeOne(a);
    });

    UpdateChecker::instance()->updateAction(a);

    return a;
}

QPixmap UpdateChecker::updateAvailableIcon(QPixmap originalIcon)
{
    QPainter p(&originalIcon);
    p.setRenderHint(QPainter::Antialiasing);

    QSize circleSize = originalIcon.size() / 2;
    QRect circleRect(QPoint(circleSize.width(), circleSize.height()), circleSize);

    p.setPen(Qt::transparent);
    p.setBrush(QColor(200, 0, 0));
    p.drawEllipse(circleRect);

    return originalIcon;
}

void UpdateChecker::updateActions()
{
    for (QAction* a : d->checkForUpdateActions) {
        updateAction(a);
    }
}

void UpdateChecker::updateAction(QAction *a)
{
    switch (d->state) {
        case UpdateCheckerPrivate::Checking:
            a->setText(tr("Checking for updates..."));
            a->setEnabled(false);
            break;
        case UpdateCheckerPrivate::Idle:
            a->setText(tr("Check for updates"));
            a->setEnabled(true);
            break;
        case UpdateCheckerPrivate::NewUpdateAvailable:
            a->setText(tr("Update to %1 available").arg(versionString(d->newUpdateVersion)));
            a->setEnabled(true);
            break;
        case UpdateCheckerPrivate::Errored:
            a->setText(tr("Error checking for updates"));
            a->setEnabled(true);
            break;
    }
}