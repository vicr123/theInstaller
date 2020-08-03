#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
#include <QAction>

struct UpdateCheckerPrivate;
class UpdateChecker : public QObject
{
    Q_OBJECT
    public:
        ~UpdateChecker();

        static void initialise(QUrl metadataUrl, QUrl updateDownloadUrl, int major, int minor, int build, int bp = 0);
        static UpdateChecker* instance();

        static bool updatesSupported();
        void checkForUpdates();

        static bool isUpdateAvailable();

        static QAction* checkForUpdatesAction();

    signals:
        void closeAllWindows();
        void updateAvailable();

    private:
        explicit UpdateChecker(QObject *parent = nullptr);
        UpdateCheckerPrivate* d;

        QString versionString(const int params[]);

        void updateActions();
        void updateAction(QAction* a);
};

#endif // UPDATECHECKER_H
