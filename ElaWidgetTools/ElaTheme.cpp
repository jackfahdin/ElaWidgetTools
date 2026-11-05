#include "ElaTheme.h"

#include <QApplication>
#include <QStyleHints>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>

#include "ElaThemePrivate.h"
namespace
{
bool isValidMode(ElaThemeType::ThemeMode mode)
{
    return mode == ElaThemeType::Light || mode == ElaThemeType::Dark;
}
bool isValidRole(ElaThemeType::ThemeColor role)
{
    return role >= ElaThemeType::ScrollBarHandle && role <= ElaThemeType::Win10BorderInactive;
}
ElaThemeType::ThemeMode systemMode(Qt::ColorScheme scheme)
{
    return scheme == Qt::ColorScheme::Dark ? ElaThemeType::Dark : ElaThemeType::Light;
}
void refreshWidgets()
{
    // update 合并到事件循环，覆盖已打开的弹出层和无模式切换通知的自绘控件。
    for (QWidget* widget : QApplication::allWidgets())
    {
        widget->update();
    }
}
} // namespace
ELA_SINGLETON_CPP(ElaTheme)
ElaTheme::ElaTheme(QObject* parent) : QObject{parent}, d_ptr(new ElaThemePrivate())
{
    Q_D(ElaTheme);
    d->q_ptr = this;
    d->_initThemeColor();
    connect(this, &ElaTheme::themeColorsChanged, this, &refreshWidgets);
    connect(this, &ElaTheme::themeModeChanged, this, &refreshWidgets);
}

ElaTheme::~ElaTheme()
{
}

void ElaTheme::setThemeMode(ElaThemeType::ThemeMode themeMode)
{
    Q_D(ElaTheme);
    if (!isValidMode(themeMode))
    {
        return;
    }
    setFollowSystemTheme(false);
    d->_applyThemeMode(themeMode);
}

ElaThemeType::ThemeMode ElaTheme::getThemeMode() const
{
    Q_D(const ElaTheme);
    return d->_themeMode;
}

void ElaTheme::setFollowSystemTheme(bool follow)
{
    Q_D(ElaTheme);
    if (d->_followSystemTheme == follow)
    {
        return;
    }
    // QApplication 创建后才能订阅平台主题；不更改 Qt 或桌面的全局配色偏好。
    if (follow && !QGuiApplication::instance())
    {
        return;
    }
    if (follow && !d->_systemThemeConnection)
    {
        d->_systemThemeConnection = connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this,
                                            [this](Qt::ColorScheme scheme) {
                                                Q_D(ElaTheme);
                                                if (d->_followSystemTheme)
                                                {
                                                    d->_applyThemeMode(systemMode(scheme));
                                                }
                                            });
    }
    d->_followSystemTheme = follow;
    if (follow)
    {
        d->_applyThemeMode(systemMode(QGuiApplication::styleHints()->colorScheme()));
    }
    Q_EMIT followSystemThemeChanged(d->_followSystemTheme);
}

bool ElaTheme::getFollowSystemTheme() const
{
    Q_D(const ElaTheme);
    return d->_followSystemTheme;
}

void ElaTheme::setAccentColor(const QColor& color)
{
    Q_D(ElaTheme);
    if (!color.isValid())
    {
        return;
    }
    const bool lightChanged = d->_setAccentColor(ElaThemeType::Light, color);
    const bool darkChanged = d->_setAccentColor(ElaThemeType::Dark, color);
    if (lightChanged)
    {
        Q_EMIT themeColorsChanged(ElaThemeType::Light);
    }
    if (darkChanged)
    {
        Q_EMIT themeColorsChanged(ElaThemeType::Dark);
    }
}

void ElaTheme::setAccentColor(ElaThemeType::ThemeMode themeMode, const QColor& color)
{
    Q_D(ElaTheme);
    if (isValidMode(themeMode) && color.isValid() && d->_setAccentColor(themeMode, color))
    {
        Q_EMIT themeColorsChanged(themeMode);
    }
}

const QColor& ElaTheme::getAccentColor(ElaThemeType::ThemeMode themeMode) const
{
    Q_D(const ElaTheme);
    static const QColor invalid;
    if (!isValidMode(themeMode))
    {
        return invalid;
    }
    return (themeMode == ElaThemeType::Light ? d->_lightThemeColorList
                                             : d->_darkThemeColorList)[ElaThemeType::PrimaryNormal];
}

void ElaTheme::resetThemeColors()
{
    Q_D(ElaTheme);
    const auto light = d->_lightThemeColorList;
    const auto dark = d->_darkThemeColorList;
    d->_initThemeColor();
    if (light != d->_lightThemeColorList)
    {
        Q_EMIT themeColorsChanged(ElaThemeType::Light);
    }
    if (dark != d->_darkThemeColorList)
    {
        Q_EMIT themeColorsChanged(ElaThemeType::Dark);
    }
}

void ElaTheme::drawEffectShadow(QPainter* painter, QRect widgetRect, int shadowBorderWidth, int borderRadius,
                                qreal alphaRatio, const QColor& shadowColor)
{
    Q_D(ElaTheme);
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setBrush(Qt::NoBrush);
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    QColor color = shadowColor.isValid()                  ? shadowColor
                   : d->_themeMode == ElaThemeType::Light ? QColor(0x70, 0x70, 0x70)
                                                          : QColor(0x9C, 0x9B, 0x9E);
    for (int i = 0; i < shadowBorderWidth; i++)
    {
        path.addRoundedRect(widgetRect.x() + shadowBorderWidth - i, widgetRect.y() + shadowBorderWidth - i,
                            widgetRect.width() - (shadowBorderWidth - i) * 2,
                            widgetRect.height() - (shadowBorderWidth - i) * 2, borderRadius + i, borderRadius + i);
        qreal alpha = alphaRatio * (shadowBorderWidth - i + 1);
        color.setAlpha(alpha > 255 ? 255 : static_cast<int>(alpha));
        painter->setPen(color);
        painter->drawPath(path);
    }
    painter->restore();
}

void ElaTheme::setThemeColor(ElaThemeType::ThemeMode themeMode, ElaThemeType::ThemeColor themeColor,
                             const QColor& newColor)
{
    Q_D(ElaTheme);
    if (!isValidMode(themeMode) || !isValidRole(themeColor) || !newColor.isValid())
    {
        return;
    }
    auto& color = (themeMode == ElaThemeType::Light ? d->_lightThemeColorList : d->_darkThemeColorList)[themeColor];
    if (color == newColor)
    {
        return;
    }
    color = newColor;
    Q_EMIT themeColorsChanged(themeMode);
}

const QColor& ElaTheme::getThemeColor(ElaThemeType::ThemeMode themeMode, ElaThemeType::ThemeColor themeColor)
{
    Q_D(ElaTheme);
    static const QColor invalid;
    if (!isValidMode(themeMode) || !isValidRole(themeColor))
    {
        return invalid;
    }
    return (themeMode == ElaThemeType::Light ? d->_lightThemeColorList : d->_darkThemeColorList)[themeColor];
}
