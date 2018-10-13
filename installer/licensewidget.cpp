#include "licensewidget.h"
#include "ui_licensewidget.h"

#include <QPropertyAnimation>
#include <QDesktopServices>

LicenseWidget::LicenseWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LicenseWidget)
{
    ui->setupUi(this);
}

LicenseWidget::~LicenseWidget()
{
    delete ui;
}

void LicenseWidget::show(QString title, QString text) {
    ui->titleWidget->setText(title);
    ui->license->setHtml(text);

    this->setGeometry(parentWidget()->width(), 0, parentWidget()->width(), parentWidget()->height());

    QWidget::show();

    QPropertyAnimation* anim = new QPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(0, 0, this->width(), this->height()));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QPropertyAnimation::DeleteWhenStopped);

    coverWidget = new QWidget(this->parentWidget());
    coverWidget->setGeometry(0, 0, this->width(), this->height());
    coverWidget->setPalette(this->palette());
    coverWidget->setAutoFillBackground(true);

    opacityEffect = new QGraphicsOpacityEffect(coverWidget);
    coverWidget->setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(0);
    coverWidget->show();

    coverWidget->raise();
    this->raise();

    QPropertyAnimation* opac = new QPropertyAnimation(opacityEffect, "opacity");
    opac->setStartValue((qreal) 0);
    opac->setEndValue((qreal) 1);
    opac->setDuration(500);
    opac->setEasingCurve(QEasingCurve::OutCubic);
    opac->start(QPropertyAnimation::DeleteWhenStopped);
}

void LicenseWidget::on_backButton_clicked()
{
    QPropertyAnimation* anim = new QPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(this->width(), 0, this->width(), this->height()));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QPropertyAnimation::DeleteWhenStopped);

    QPropertyAnimation* opac = new QPropertyAnimation(opacityEffect, "opacity");
    opac->setStartValue((qreal) 1);
    opac->setEndValue((qreal) 0);
    opac->setDuration(500);
    opac->setEasingCurve(QEasingCurve::OutCubic);
    opac->start(QPropertyAnimation::DeleteWhenStopped);
    connect(opac, &QPropertyAnimation::finished, [=] {
        coverWidget->deleteLater();
    });
}

void LicenseWidget::on_license_anchorClicked(const QUrl &arg1)
{
    QDesktopServices::openUrl(arg1);
}
