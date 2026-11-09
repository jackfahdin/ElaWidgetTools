#include "ElaToggleSwitchPrivate.h"

#include <QPropertyAnimation>
#include <QPointer>
#include "ElaToggleAccessible.h"

#include "ElaToggleSwitch.h"
ElaToggleSwitchPrivate::ElaToggleSwitchPrivate(QObject* parent) : QObject{parent}
{
}

ElaToggleSwitchPrivate::~ElaToggleSwitchPrivate()
{
}

void ElaToggleSwitchPrivate::_startPosAnimation(qreal startX, qreal endX, bool isToggle)
{
    Q_Q(ElaToggleSwitch);
    QPropertyAnimation* circleAnimation = new QPropertyAnimation(q, "circleCenterX", q);
    connect(circleAnimation, &QPropertyAnimation::valueChanged, q, [=](const QVariant& value) {
        this->_circleCenterX = value.toReal();
        q->update();
    });
    circleAnimation->setStartValue(startX);
    circleAnimation->setEndValue(endX);
    circleAnimation->setEasingCurve(QEasingCurve::InOutSine);
    circleAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    if (_isToggled != isToggle)
    {
        _isToggled = isToggle;
        const QPointer<ElaToggleSwitch> alive(q);
        Q_EMIT q->toggled(isToggle);
        if (alive)
            elaNotifyToggleStateChanged(q);
    }
}

void ElaToggleSwitchPrivate::_startRadiusAnimation(qreal startRadius, qreal endRadius)
{
    Q_Q(ElaToggleSwitch);
    QPropertyAnimation* circleRadiusAnimation = new QPropertyAnimation(q, "circleRadius", q);
    connect(circleRadiusAnimation, &QPropertyAnimation::valueChanged, q, [=](const QVariant& value) {
        this->_circleRadius = value.toReal();
        q->update();
    });
    circleRadiusAnimation->setEasingCurve(QEasingCurve::InOutSine);
    circleRadiusAnimation->setStartValue(startRadius);
    circleRadiusAnimation->setEndValue(endRadius);
    circleRadiusAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void ElaToggleSwitchPrivate::_adjustCircleCenterX()
{
    Q_Q(ElaToggleSwitch);
    if (_circleCenterX > q->width() - q->height() / 2 - _margin * 2)
    {
        _circleCenterX = q->width() - q->height() / 2 - _margin * 2;
    }
    if (_circleCenterX < q->height() / 2)
    {
        _circleCenterX = q->height() / 2;
    }
}

void ElaToggleSwitchPrivate::_cancelPress()
{
    Q_Q(ElaToggleSwitch);
    _isSpacePressed = false;
    if (_isLeftButtonPress)
    {
        _isLeftButtonPress = false;
        _isMousePressMove = false;
        _startPosAnimation(_circleCenterX, _isToggled ? q->width() - q->height() / 2 - _margin * 2 : q->height() / 2,
                           _isToggled);
    }
    // 取消按压也要恢复圆点大小，禁用／隐藏时恢复普通半径。
    const bool hovered = q->isEnabled() && q->isVisible() && q->underMouse();
    _startRadiusAnimation(_circleRadius, q->height() * (hovered ? 0.35 : 0.3));
    q->update();
}
