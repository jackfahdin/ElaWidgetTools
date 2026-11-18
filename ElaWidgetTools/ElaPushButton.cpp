#include "ElaPushButton.h"

#include "ElaApplication.h"
#include "ElaColorAnimation.h"
#include "ElaControlStyle.h"
#include "ElaTheme.h"
#include "private/ElaPushButtonPrivate.h"

#include <QPainter>
#include <QStyleOption>

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
    return d->_pLightDefaultColor.isValid() ? d->_pLightDefaultColor
                                            : (d->_isAccent ? ElaThemeColor(ElaThemeType::Light, PrimaryNormal)
                                                            : ElaThemeColor(ElaThemeType::Light, BasicBase));
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
    return d->_pDarkDefaultColor.isValid() ? d->_pDarkDefaultColor
                                           : (d->_isAccent ? ElaThemeColor(ElaThemeType::Dark, PrimaryNormal)
                                                           : ElaThemeColor(ElaThemeType::Dark, BasicBase));
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
    return d->_pLightHoverColor.isValid() ? d->_pLightHoverColor
                                          : (d->_isAccent ? ElaThemeColor(ElaThemeType::Light, PrimaryHover)
                                                          : ElaThemeColor(ElaThemeType::Light, BasicHover));
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
    return d->_pDarkHoverColor.isValid() ? d->_pDarkHoverColor
                                         : (d->_isAccent ? ElaThemeColor(ElaThemeType::Dark, PrimaryHover)
                                                         : ElaThemeColor(ElaThemeType::Dark, BasicHover));
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
    return d->_pLightPressColor.isValid() ? d->_pLightPressColor
                                          : (d->_isAccent ? ElaThemeColor(ElaThemeType::Light, PrimaryPress)
                                                          : ElaThemeColor(ElaThemeType::Light, BasicPress));
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
    return d->_pDarkPressColor.isValid() ? d->_pDarkPressColor
                                         : (d->_isAccent ? ElaThemeColor(ElaThemeType::Dark, PrimaryPress)
                                                         : ElaThemeColor(ElaThemeType::Dark, BasicPress));
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
        update();
    });
}

ElaPushButton::ElaPushButton(const QString& text, QWidget* parent) : ElaPushButton(parent)
{
    setText(text);
}

ElaPushButton::~ElaPushButton()
{
}

void ElaPushButton::setIsAccent(bool isAccent)
{
    Q_D(ElaPushButton);
    d->_isAccent = isAccent;
    update();
}

bool ElaPushButton::getIsAccent() const
{
    Q_D(const ElaPushButton);
    return d->_isAccent;
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
    return d->_lightTextColor.isValid() ? d->_lightTextColor
                                        : (d->_isAccent ? ElaThemeColor(ElaThemeType::Light, BasicTextInvert)
                                                        : ElaThemeColor(ElaThemeType::Light, BasicText));
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
    return d->_darkTextColor.isValid() ? d->_darkTextColor
                                       : (d->_isAccent ? ElaThemeColor(ElaThemeType::Dark, BasicTextInvert)
                                                       : ElaThemeColor(ElaThemeType::Dark, BasicText));
}

QSize ElaPushButton::sizeHint() const
{
    const auto hint = QPushButton::sizeHint();
    const auto metrics = fontMetrics();
    const int margin = qMax(12, metrics.height() * 3 / 4);
    int contentWidth = metrics.horizontalAdvance(text());
    if (!icon().isNull())
    {
        contentWidth += iconSize().width() + (text().isEmpty() ? 0 : qMax(6, metrics.horizontalAdvance(' ')));
    }
    return {qMax(hint.width(), contentWidth + margin * 2), hint.height()};
}

QSize ElaPushButton::minimumSizeHint() const
{
    return sizeHint();
}

void ElaPushButton::paintEvent(QPaintEvent* event)
{
    Q_D(ElaPushButton);
    QPainter painter(this);
    painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing | QPainter::TextAntialiasing);

    // 背景和边框共用圆角轮廓
    const auto foregroundRect = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    painter.setPen(QPen(ElaControlStyle::getControlBorder(foregroundRect, d->_themeMode, isEnabled() && !isDown()), 1));
    if (d->_themeMode == ElaThemeType::Light)
    {
        painter.setBrush(ElaColorAnimation::getColor(
            this,
            isEnabled()
                ? isDown() ? getLightPressColor() : (underMouse() ? getLightHoverColor() : getLightDefaultColor())
                : (d->_isAccent ? ElaThemeColor(d->_themeMode, PrimaryDisabled)
                                : ElaThemeColor(d->_themeMode, BasicDisable)),
            "Background"));
    }
    else
    {
        painter.setBrush(ElaColorAnimation::getColor(
            this,
            isEnabled() ? isDown() ? getDarkPressColor() : (underMouse() ? getDarkHoverColor() : getDarkDefaultColor())
                        : (d->_isAccent ? ElaThemeColor(d->_themeMode, PrimaryDisabled)
                                        : ElaThemeColor(d->_themeMode, BasicDisable)),
            "Background"));
    }
    painter.drawRoundedRect(foregroundRect, d->_pBorderRadius, d->_pBorderRadius);
    // 图标和文字整体居中
    painter.setPen(!isEnabled()                           ? ElaThemeColor(d->_themeMode, BasicTextDisable)
                   : d->_themeMode == ElaThemeType::Light ? getLightTextColor()
                                                          : getDarkTextColor());
    QRect textRect = rect();
    if (!icon().isNull())
    {
        const int spacing = text().isEmpty() ? 0 : qMax(6, fontMetrics().horizontalAdvance(' '));
        const int textWidth = fontMetrics().horizontalAdvance(text());
        const QSize size = iconSize();
        const QRect iconRect((width() - size.width() - spacing - textWidth) / 2, (height() - size.height()) / 2,
                             size.width(), size.height());
        icon().paint(&painter, iconRect, Qt::AlignCenter, isEnabled() ? QIcon::Normal : QIcon::Disabled);
        textRect.setLeft(iconRect.right() + spacing + 1);
        textRect.setWidth(textWidth);
    }
    painter.drawText(textRect, Qt::AlignCenter, text());
}
