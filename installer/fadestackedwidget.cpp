#include "fadestackedwidget.h"

FadeStackedWidget::FadeStackedWidget(QWidget *parent) : QStackedWidget(parent)
{

}

void FadeStackedWidget::setCurrentIndex(int index) {
    //Do some checks before setting the current index.
    if (currentIndex() != index) {
        doSetCurrentIndex(index);
    }
}

void FadeStackedWidget::setCurrentWidget(QWidget *widget)
{
    setCurrentIndex(this->indexOf(widget));
}

void FadeStackedWidget::doSetCurrentIndex(int index) {
    QWidget* currentWidget = widget(currentIndex());
    QWidget* nextWidget = widget(index);
    if (nextWidget == nullptr) {
        QStackedWidget::setCurrentIndex(index);
    } else {
        if (doingNewAnimation) {
            anim->stop();
            anim->deleteLater();
        }

        doingNewAnimation = true;
        QGraphicsOpacityEffect* currentOpacity = new QGraphicsOpacityEffect();
        QGraphicsOpacityEffect* nextOpacity = new QGraphicsOpacityEffect();
        currentWidget->setGraphicsEffect(currentOpacity);
        nextWidget->setGraphicsEffect(nextOpacity);

        anim = new QVariantAnimation();
        anim->setStartValue((float) 1);
        anim->setEndValue((float) 0);
        anim->setDuration(500);
        anim->setEasingCurve(QEasingCurve::InCubic);
        connect(anim, &QVariantAnimation::finished, [=] {
            if (anim->direction() == QVariantAnimation::Forward) {
                anim->setDirection(QVariantAnimation::Backward);
                QStackedWidget::setCurrentIndex(index);
                anim->start();
            } else {
                doingNewAnimation = false;
                anim->deleteLater();
                anim = nullptr;
            }
        });
        connect(anim, &QVariantAnimation::valueChanged, [=](QVariant value) {
            if (anim->direction() == QVariantAnimation::Forward) {
                currentOpacity->setOpacity(value.toFloat());
            } else {
                nextOpacity->setOpacity(value.toFloat());
            }
        });
        anim->start();
    }
}
