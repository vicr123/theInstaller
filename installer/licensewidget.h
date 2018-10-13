#ifndef LICENSEWIDGET_H
#define LICENSEWIDGET_H

#include <QWidget>
#include <QGraphicsOpacityEffect>

namespace Ui {
    class LicenseWidget;
}

class LicenseWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit LicenseWidget(QWidget *parent = nullptr);
        ~LicenseWidget();

    public slots:
        void show(QString title, QString text);

    private slots:
        void on_backButton_clicked();

        void on_license_anchorClicked(const QUrl &arg1);

    private:
        Ui::LicenseWidget *ui;

        QWidget* coverWidget = nullptr;
        QGraphicsOpacityEffect* opacityEffect;
};

#endif // LICENSEWIDGET_H
