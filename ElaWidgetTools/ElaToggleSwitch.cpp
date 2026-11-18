#include "ElaToggleSwitch.h"

#include "ElaColorAnimation.h"
#include "ElaControlStyle.h"
#include "ElaTheme.h"
#include "private/ElaToggleAccessible.h"
#include "private/ElaToggleSwitchPrivate.h"

#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QStyleOption>

ElaToggleSwitch::ElaToggleSwitch(QWidget* parent) : QWidget{parent}, d_ptr(new ElaToggleSwitchPrivate())
{
    Q_D(ElaToggleSwitch);
    d->q_ptr = this;
    elaEnsureToggleAccessibility();
    setObjectName("ElaToggleSwitch");
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(44, 22);
    d->_circleCenterX = -1;
    d->_isToggled = false;
    d->_themeMode = eTheme->getThemeMode();
    setProperty("circleCenterX", 0.01);
    setProperty("circleRadius", 0.01);
    connect(eTheme, &ElaTheme::themeModeChanged, this, [=](ElaThemeType::ThemeMode themeMode) {
        d->_themeMode = themeMode;
        update();
    });
    connect(eTheme, &ElaTheme::themeColorsChanged, this, [this] {
        update();
    });
}

ElaToggleSwitch::~ElaToggleSwitch()
{
}

void ElaToggleSwitch::setIsToggled(bool isToggled)
{
    Q_D(ElaToggleSwitch);
    if (d->_isToggled == isToggled)
    {
        return;
    }
    if (d->_isToggled)
    {
        d->_startPosAnimation(width() - height() / 2 - d->_margin * 2, height() / 2, isToggled);
    }
    else
    {
        d->_startPosAnimation(height() / 2, width() - height() / 2 - d->_margin * 2, isToggled);
    }
}

bool ElaToggleSwitch::getIsToggled() const
{
    return d_ptr->_isToggled;
}

bool ElaToggleSwitch::event(QEvent* event)
{
    Q_D(ElaToggleSwitch);
    switch (event->type())
    {
    case QEvent::FocusIn: {
        update();
        break;
    }
    case QEvent::FocusOut:
    case QEvent::Hide:
    case QEvent::WindowDeactivate:
    case QEvent::EnabledChange: {
        d->_cancelPress();
        break;
    }
    case QEvent::Enter: {
        if (isEnabled())
        {
            d->_startRadiusAnimation(height() * 0.3, height() * 0.35);
        }
        break;
    }
    case QEvent::Leave: {
        if (isEnabled())
        {
            d->_startRadiusAnimation(height() * 0.35, height() * 0.3);
        }
        break;
    }
    case QEvent::MouseMove: {
        update();
        break;
    }
    default: {
        break;
    }
    }
    return QWidget::event(event);
}

void ElaToggleSwitch::mousePressEvent(QMouseEvent* event)
{
    Q_D(ElaToggleSwitch);
    if (event->button() != Qt::LeftButton || !isEnabled() || d->_isSpacePressed ||
        !rect().contains(event->position().toPoint()))
    {
        event->ignore();
        return;
    }
    if (d->_positionAnimation)
    {
        d->_positionAnimation->stop();
    }
    d->_adjustCircleCenterX();
    setProperty("circleCenterX", d->_circleCenterX);
    d->_isLeftButtonPress = true;
    d->_isDragging = false;
    d->_mousePressPos = event->position().toPoint();
    d->_lastMouseX = event->position().toPoint().x();
    d->_startRadiusAnimation(d->_circleRadius, height() * 0.25);
    event->accept();
}

void ElaToggleSwitch::mouseReleaseEvent(QMouseEvent* event)
{
    Q_D(ElaToggleSwitch);
    if (event->button() != Qt::LeftButton || !isEnabled() || !d->_isLeftButtonPress)
    {
        event->ignore();
        return;
    }
    d->_isLeftButtonPress = false;
    const bool releasedInside = rect().contains(event->position().toPoint());
    bool isToggled = d->_isToggled;
    if (releasedInside)
    {
        isToggled = d->_isDragging ? d->_circleCenterX > width() / 2 : !isToggled;
    }
    d->_isDragging = false;
    d->_startRadiusAnimation(d->_circleRadius, height() * (releasedInside ? 0.35 : 0.3));
    event->accept();
    // 状态通知可能删除控件，所有按压清理必须先完成。
    d->_startPosAnimation(d->_circleCenterX, isToggled ? width() - height() / 2 - d->_margin * 2 : height() / 2,
                          isToggled);
}

void ElaToggleSwitch::mouseMoveEvent(QMouseEvent* event)
{
    Q_D(ElaToggleSwitch);
    if (!d->_isLeftButtonPress)
    {
        QWidget::mouseMoveEvent(event);
        return;
    }
    // 滑块立即跟手，距离阈值仅用于松开时区分点击与拖动
    if ((event->pos() - d->_mousePressPos).manhattanLength() >= qMax(1, QApplication::startDragDistance()))
    {
        d->_isDragging = true;
    }
    d->_circleCenterX += event->pos().x() - d->_lastMouseX;
    d->_lastMouseX = event->pos().x();
    d->_adjustCircleCenterX();
    setProperty("circleCenterX", d->_circleCenterX);
    update();
    event->accept();
}

void ElaToggleSwitch::keyPressEvent(QKeyEvent* event)
{
    Q_D(ElaToggleSwitch);
    if (event->key() == Qt::Key_Escape && (d->_isSpacePressed || d->_isLeftButtonPress))
    {
        d->_cancelPress();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Space && isEnabled())
    {
        if (!event->isAutoRepeat() && !d->_isLeftButtonPress)
        {
            d->_isSpacePressed = true;
        }
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void ElaToggleSwitch::keyReleaseEvent(QKeyEvent* event)
{
    Q_D(ElaToggleSwitch);
    if (event->key() == Qt::Key_Space && isEnabled())
    {
        event->accept();
        if (!event->isAutoRepeat() && d->_isSpacePressed)
        {
            d->_isSpacePressed = false;
            setIsToggled(!d->_isToggled);
        }
        return;
    }
    QWidget::keyReleaseEvent(event);
}

void ElaToggleSwitch::paintEvent(QPaintEvent* event)
{
    Q_D(ElaToggleSwitch);
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    // 背景绘制
    painter.setPen(d->_isToggled ? QPen(ElaThemeColor(d->_themeMode, BasicBorder), 1.5)
                                 : QPen(ElaThemeColor(d->_themeMode, BasicBorderDeep), 1.5));
    QStyleOption option;
    option.initFrom(this);
    option.state.setFlag(QStyle::State_Sunken, d->_isLeftButtonPress);
    painter.setBrush(ElaColorAnimation::getColor(
        this, ElaControlStyle::getIndicatorBackgroundColor(d->_themeMode, option.state, d->_isToggled)));
    QPainterPath path;
    path.moveTo(width() - height() - d->_margin, height() - d->_margin);
    path.arcTo(QRectF(QPointF(width() - height() - d->_margin, d->_margin),
                      QSize(height() - d->_margin * 2, height() - d->_margin * 2)),
               -90, 180);
    path.lineTo(height() / 2 + d->_margin, d->_margin);
    path.arcTo(QRectF(QPointF(d->_margin, d->_margin), QSize(height() - d->_margin * 2, height() - d->_margin * 2)), 90,
               180);
    path.lineTo(width() - height() - d->_margin, height() - d->_margin);
    path.closeSubpath();
    painter.drawPath(path);

    // 圆心绘制
    painter.setPen(Qt::NoPen);
    painter.setBrush(ElaColorAnimation::getColor(this,
                                                 isEnabled() ? d->_isToggled
                                                                   ? ElaThemeColor(d->_themeMode, BasicTextInvert)
                                                                   : ElaThemeColor(d->_themeMode, BasicIndicator)
                                                             : ElaThemeColor(d->_themeMode, BasicTextDisable),
                                                 "Thumb"));
    if (d->_circleRadius == 0)
    {
        d->_circleRadius = this->isEnabled() ? (underMouse() ? height() * 0.35 : height() * 0.3) : height() * 0.3;
    }
    if (d->_isLeftButtonPress)
    {
        painter.drawEllipse(QPointF(d->_circleCenterX, height() / 2), d->_circleRadius, d->_circleRadius);
    }
    else
    {
        if (d->_circleCenterX == -1)
        {
            d->_circleCenterX = d->_isToggled ? width() - height() / 2 - d->_margin * 2 : height() / 2;
        }
        painter.drawEllipse(QPointF(d->_circleCenterX, height() / 2), d->_circleRadius, d->_circleRadius);
    }
    if (hasFocus() && isEnabled())
    {
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(ElaThemeColor(d->_themeMode, BasicText), 1, Qt::DotLine));
        painter.drawRoundedRect(QRectF(rect()).adjusted(2, 2, -2, -2), height() / 2.0 - 2, height() / 2.0 - 2);
    }
    painter.restore();
}
