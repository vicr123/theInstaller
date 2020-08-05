#ifndef FADESTACKEDWIDGET_H
#define FADESTACKEDWIDGET_H

#include <QObject>
#include <QWidget>
#include <QStackedWidget>
#include <QVariantAnimation>
#include <QGraphicsOpacityEffect>

class FadeStackedWidget : public QStackedWidget
{
    Q_OBJECT
    public:
        explicit FadeStackedWidget(QWidget *parent = nullptr);

    signals:

    public slots:
        void setCurrentIndex(int index);
        void setCurrentWidget(QWidget* widget);

    private slots:
        void doSetCurrentIndex(int index);

    private:
        bool doingNewAnimation = false;
        QVariantAnimation* anim;
};

#endif // FADESTACKEDWIDGET_H
