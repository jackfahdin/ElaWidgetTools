#include "ElaPushButton.h"

#include "ElaApplication.h"
#include <QPainter>
#include <QPainterPath>

#include "ElaTheme.h"
#include "private/ElaPushButtonPrivate.h"
ELA_PROPERTY_Q_CPP(ElaPushButton, int, BorderRadius)
void ElaPushButton::setLightDefaultColor(const QColor& color)
{
    Q_D(ElaPushButton);
    d->_pLightDefaultColor = color;
    update();
    Q_EMIT pLightDefaultColorChanged();
}

const QColor& ElaPushButton::getLightDefaultColor() const
{
    Q_D(const ElaPushButton);
    return d->_pLightDefaultColor.isValid() ? d->_pLightDefaultColor : ElaThemeColor(ElaThemeType::Light, BasicBase);
}

void ElaPushButton::setDarkDefaultColor(const QColor& color)
{
    Q_D(ElaPushButton);
    d->_pDarkDefaultColor = color;
    update();
    Q_EMIT pDarkDefaultColorChanged();
}

const QColor& ElaPushButton::getDarkDefaultColor() const
{
    Q_D(const ElaPushButton);
    return d->_pDarkDefaultColor.isValid() ? d->_pDarkDefaultColor : ElaThemeColor(ElaThemeType::Dark, BasicBase);
}

void ElaPushButton::setLightHoverColor(const QColor& color)
{
    Q_D(ElaPushButton);
    d->_pLightHoverColor = color;
    update();
    Q_EMIT pLightHoverColorChanged();
}

const QColor& ElaPushButton::getLightHoverColor() const
{
    Q_D(const ElaPushButton);
    return d->_pLightHoverColor.isValid() ? d->_pLightHoverColor : ElaThemeColor(ElaThemeType::Light, BasicHover);
}

void ElaPushButton::setDarkHoverColor(const QColor& color)
{
    Q_D(ElaPushButton);
    d->_pDarkHoverColor = color;
    update();
    Q_EMIT pDarkHoverColorChanged();
}

const QColor& ElaPushButton::getDarkHoverColor() const
{
    Q_D(const ElaPushButton);
    return d->_pDarkHoverColor.isValid() ? d->_pDarkHoverColor : ElaThemeColor(ElaThemeType::Dark, BasicHover);
}

void ElaPushButton::setLightPressColor(const QColor& color)
{
    Q_D(ElaPushButton);
    d->_pLightPressColor = color;
    update();
    Q_EMIT pLightPressColorChanged();
}

const QColor& ElaPushButton::getLightPressColor() const
{
    Q_D(const ElaPushButton);
    return d->_pLightPressColor.isValid() ? d->_pLightPressColor : ElaThemeColor(ElaThemeType::Light, BasicPress);
}

void ElaPushButton::setDarkPressColor(const QColor& color)
{
    Q_D(ElaPushButton);
    d->_pDarkPressColor = color;
    update();
    Q_EMIT pDarkPressColorChanged();
}

const QColor& ElaPushButton::getDarkPressColor() const
{
    Q_D(const ElaPushButton);
    return d->_pDarkPressColor.isValid() ? d->_pDarkPressColor : ElaThemeColor(ElaThemeType::Dark, BasicPress);
}

ElaPushButton::ElaPushButton(QWidget* parent) : QPushButton(parent), d_ptr(new ElaPushButtonPrivate())
{
    Q_D(ElaPushButton);
    d->q_ptr = this;
    d->_pBorderRadius = 3;
    d->_themeMode = eTheme->getThemeMode();
    setMouseTracking(true);
    setFixedHeight(30);
    QFont font = this->font();
    font.setPixelSize(eApp->getFontPixelSize() + 2);
    setFont(font);
    setObjectName("ElaPushButton");
    setStyleSheet("#ElaPushButton{background-color:transparent;}");
    connect(eTheme, &ElaTheme::themeModeChanged, this, [=](ElaThemeType::ThemeMode themeMode) {
        d->_themeMode = themeMode;
    });
}

ElaPushButton::ElaPushButton(const QString& text, QWidget* parent) : ElaPushButton(parent)
{
    setText(text);
}

ElaPushButton::~ElaPushButton()
{
}

void ElaPushButton::setLightTextColor(const QColor& color)
{
    Q_D(ElaPushButton);
    d->_lightTextColor = color;
    update();
}

const QColor& ElaPushButton::getLightTextColor() const
{
    Q_D(const ElaPushButton);
    return d->_lightTextColor.isValid() ? d->_lightTextColor : ElaThemeColor(ElaThemeType::Light, BasicText);
}

void ElaPushButton::setDarkTextColor(const QColor& color)
{
    Q_D(ElaPushButton);
    d->_darkTextColor = color;
    update();
}

const QColor& ElaPushButton::getDarkTextColor() const
{
    Q_D(const ElaPushButton);
    return d->_darkTextColor.isValid() ? d->_darkTextColor : ElaThemeColor(ElaThemeType::Dark, BasicText);
}

void ElaPushButton::mousePressEvent(QMouseEvent* event)
{
    Q_D(ElaPushButton);
    d->_isPressed = true;
    QPushButton::mousePressEvent(event);
}

void ElaPushButton::mouseReleaseEvent(QMouseEvent* event)
{
    Q_D(ElaPushButton);
    d->_isPressed = false;
    QPushButton::mouseReleaseEvent(event);
}

void ElaPushButton::paintEvent(QPaintEvent* event)
{
    Q_D(ElaPushButton);
    QPainter painter(this);
    painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing | QPainter::TextAntialiasing);

    // 背景绘制
    painter.save();
    QRect foregroundRect = rect().adjusted(1, 1, -1, -1);
    if (d->_themeMode == ElaThemeType::Light)
    {
        painter.setPen(ElaThemeColor(ElaThemeType::Light, BasicBorder));
        painter.setBrush(isEnabled() ? d->_isPressed ? getLightPressColor()
                                                     : (underMouse() ? getLightHoverColor() : getLightDefaultColor())
                                     : ElaThemeColor(d->_themeMode, BasicDisable));
    }
    else
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(isEnabled() ? d->_isPressed ? getDarkPressColor()
                                                     : (underMouse() ? getDarkHoverColor() : getDarkDefaultColor())
                                     : ElaThemeColor(d->_themeMode, BasicDisable));
    }
    painter.drawRoundedRect(foregroundRect, d->_pBorderRadius, d->_pBorderRadius);
    // 底边线绘制
    if (!d->_isPressed)
    {
        painter.setPen(ElaThemeColor(d->_themeMode, BasicBaseLine));
        painter.drawLine(foregroundRect.x() + d->_pBorderRadius, height() - 1,
                         foregroundRect.width() - d->_pBorderRadius, height() - 1);
    }
    // 文字绘制
    painter.setPen(isEnabled() ? d->_themeMode == ElaThemeType::Light ? getLightTextColor() : getDarkTextColor()
                               : ElaThemeColor(d->_themeMode, BasicTextDisable));
    painter.drawText(foregroundRect, Qt::AlignCenter, text());
    painter.restore();
}
