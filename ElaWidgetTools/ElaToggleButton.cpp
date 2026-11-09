#include "ElaToggleButton.h"

#include "ElaApplication.h"
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPointer>
#include "private/ElaToggleAccessible.h"
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>

#include "ElaTheme.h"
#include "private/ElaToggleButtonPrivate.h"
ELA_PROPERTY_Q_CPP(ElaToggleButton, int, BorderRadius)

ElaToggleButton::ElaToggleButton(QWidget* parent) : QWidget(parent), d_ptr(new ElaToggleButtonPrivate())
{
    Q_D(ElaToggleButton);
    d->q_ptr = this;
    elaEnsureToggleAccessibility();
    setFocusPolicy(Qt::StrongFocus);
    d->_pBorderRadius = 3;
    d->_themeMode = eTheme->getThemeMode();
    d->_pToggleAlpha = 0;
    d->_toggleAnimation = new QPropertyAnimation(d, "pToggleAlpha", d);
    d->_toggleAnimation->setDuration(250);
    connect(d->_toggleAnimation, &QPropertyAnimation::valueChanged, this, [this] {
        update();
    });
    connect(d->_toggleAnimation, &QPropertyAnimation::finished, this, [this] {
        Q_D(ElaToggleButton);
        d->_isAlphaAnimationFinished = true;
        update();
    });
    setMouseTracking(true);
    setFixedSize(80, 32);
    QFont font = this->font();
    font.setPixelSize(eApp->getFontPixelSize() + 2);
    setFont(font);
    setObjectName("ElaToggleButton");
    setStyleSheet("#ElaToggleButton{background-color:transparent;}");
    connect(eTheme, &ElaTheme::themeModeChanged, this, [=](ElaThemeType::ThemeMode themeMode) {
        d->_themeMode = themeMode;
        update();
    });
    connect(eTheme, &ElaTheme::themeColorsChanged, this, [this] {
        update();
    });
}

ElaToggleButton::ElaToggleButton(const QString& text, QWidget* parent) : ElaToggleButton(parent)
{
    Q_D(ElaToggleButton);
    d->_pText = text;
}

ElaToggleButton::~ElaToggleButton()
{
}

void ElaToggleButton::setText(const QString& text)
{
    Q_D(ElaToggleButton);
    if (d->_pText == text)
        return;
    d->_pText = text;
    update();
    const QPointer<ElaToggleButton> alive(this);
    Q_EMIT pTextChanged();
    if (alive)
        elaNotifyToggleNameChanged(this);
}

const QString& ElaToggleButton::getText() const
{
    Q_D(const ElaToggleButton);
    return d->_pText;
}

void ElaToggleButton::setIsToggled(bool isToggled)
{
    Q_D(ElaToggleButton);
    d->_setToggled(isToggled, false);
}

bool ElaToggleButton::getIsToggled() const
{
    Q_D(const ElaToggleButton);
    return d->_isToggled;
}

bool ElaToggleButton::event(QEvent* event)
{
    Q_D(ElaToggleButton);
    switch (event->type())
    {
    case QEvent::FocusOut:
    case QEvent::Hide:
    case QEvent::WindowDeactivate:
    case QEvent::EnabledChange:
        d->_cancelPress();
        break;
    case QEvent::FocusIn:
    case QEvent::Enter:
    case QEvent::Leave:
        update();
        break;
    default:
        break;
    }
    return QWidget::event(event);
}

void ElaToggleButton::mousePressEvent(QMouseEvent* event)
{
    Q_D(ElaToggleButton);
    if (event->button() != Qt::LeftButton || !isEnabled() || d->_isSpacePressed ||
        !rect().contains(event->position().toPoint()))
    {
        event->ignore();
        return;
    }
    d->_isMousePressed = true;
    d->_isPressed = true;
    update();
    event->accept();
}

void ElaToggleButton::mouseReleaseEvent(QMouseEvent* event)
{
    Q_D(ElaToggleButton);
    if (event->button() != Qt::LeftButton || !isEnabled() || !d->_isMousePressed)
    {
        event->ignore();
        return;
    }
    const bool activate = rect().contains(event->position().toPoint());
    d->_cancelPress();
    event->accept();
    if (activate)
    {
        d->_setToggled(!d->_isToggled, true);
    }
}

void ElaToggleButton::mouseMoveEvent(QMouseEvent* event)
{
    Q_D(ElaToggleButton);
    if (d->_isMousePressed)
    {
        d->_isPressed = rect().contains(event->position().toPoint());
        update();
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void ElaToggleButton::keyPressEvent(QKeyEvent* event)
{
    Q_D(ElaToggleButton);
    if (event->key() == Qt::Key_Escape && (d->_isSpacePressed || d->_isMousePressed))
    {
        d->_cancelPress();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Space && isEnabled())
    {
        if (!event->isAutoRepeat() && !d->_isMousePressed)
        {
            d->_isSpacePressed = true;
            d->_isPressed = true;
            update();
        }
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void ElaToggleButton::keyReleaseEvent(QKeyEvent* event)
{
    Q_D(ElaToggleButton);
    if (event->key() == Qt::Key_Space && isEnabled())
    {
        event->accept();
        if (!event->isAutoRepeat() && d->_isSpacePressed)
        {
            d->_cancelPress();
            d->_setToggled(!d->_isToggled, true);
        }
        return;
    }
    QWidget::keyReleaseEvent(event);
}

void ElaToggleButton::paintEvent(QPaintEvent* event)
{
    Q_D(ElaToggleButton);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    painter.save();
    QRect foregroundRect(1, 1, width() - 2, height() - 2);
    if (d->_isAlphaAnimationFinished)
    {
        if (d->_isToggled)
        {
            painter.setPen(ElaThemeColor(d->_themeMode, BasicBorder));
            painter.setBrush(isEnabled() ? d->_isPressed ? ElaThemeColor(d->_themeMode, PrimaryPress)
                                                         : (underMouse() ? ElaThemeColor(d->_themeMode, PrimaryHover)
                                                                         : ElaThemeColor(d->_themeMode, PrimaryNormal))
                                         : ElaThemeColor(d->_themeMode, BasicDisable));
        }
        else
        {
            painter.setPen(ElaThemeColor(d->_themeMode, BasicBorder));
            painter.setBrush(isEnabled() ? d->_isPressed ? ElaThemeColor(d->_themeMode, BasicPress)
                                                         : (underMouse() ? ElaThemeColor(d->_themeMode, BasicHover)
                                                                         : ElaThemeColor(d->_themeMode, BasicBase))
                                         : ElaThemeColor(d->_themeMode, BasicDisable));
        }
    }
    else
    {
        painter.setPen(Qt::NoPen);
        QColor toggleColor = ElaThemeColor(d->_themeMode, PrimaryNormal);
        toggleColor.setAlpha(d->_pToggleAlpha);
        painter.setBrush(toggleColor);
    }
    painter.drawRoundedRect(foregroundRect, d->_pBorderRadius, d->_pBorderRadius);
    // 底边线绘制
    if (!d->_isPressed && !d->_isToggled)
    {
        painter.setPen(ElaThemeColor(d->_themeMode, BasicBaseLine));
        painter.drawLine(foregroundRect.x() + d->_pBorderRadius, height() - 1,
                         foregroundRect.x() + foregroundRect.width() - d->_pBorderRadius, height() - 1);
    }

    // 文字绘制
    painter.setPen(isEnabled() ? d->_isToggled ? ElaThemeColor(d->_themeMode, BasicTextInvert)
                                               : ElaThemeColor(d->_themeMode, BasicText)
                               : ElaThemeColor(d->_themeMode, BasicTextDisable));
    painter.drawText(foregroundRect, Qt::AlignCenter, d->_pText);
    if (hasFocus() && isEnabled())
    {
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(ElaThemeColor(d->_themeMode, BasicText), 1, Qt::DotLine));
        painter.drawRoundedRect(QRectF(rect()).adjusted(3, 3, -3, -3), d->_pBorderRadius, d->_pBorderRadius);
    }
    painter.restore();
}

void ElaToggleButtonPrivate::_cancelPress()
{
    Q_Q(ElaToggleButton);
    _isPressed = false;
    _isMousePressed = false;
    _isSpacePressed = false;
    q->update();
}

void ElaToggleButtonPrivate::_setToggled(bool toggled, bool animated)
{
    Q_Q(ElaToggleButton);
    if (_isToggled == toggled)
        return;
    _isToggled = toggled;
    _toggleAnimation->stop();
    _isAlphaAnimationFinished = !animated;
    if (animated)
    {
        _toggleAnimation->setStartValue(_pToggleAlpha);
        _toggleAnimation->setEndValue(toggled ? 255 : 0);
        _toggleAnimation->start();
    }
    else
    {
        _pToggleAlpha = toggled ? 255 : 0;
    }
    q->update();
    const QPointer<ElaToggleButton> alive(q);
    Q_EMIT q->toggled(toggled);
    if (alive)
        elaNotifyToggleStateChanged(q);
}
