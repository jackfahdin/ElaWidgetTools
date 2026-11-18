#include "ElaTheme.h"

#include <QApplication>
#include <QStyleHints>
#include <QWidget>
#include "ElaColorAnimation.h"
#include "ElaThemePrivate.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <qdrawutil.h>

namespace
{
bool isValidMode(ElaThemeType::ThemeMode mode)
{
    return mode == ElaThemeType::Light || mode == ElaThemeType::Dark;
}
bool isValidRole(ElaThemeType::ThemeColor role)
{
    return role >= 0 && role < ElaThemeType::ThemeColorCount;
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
    connect(this, &ElaTheme::themeColorsChanged, this, [this](ElaThemeType::ThemeMode mode) {
        if (mode == getThemeMode())
        {
            ElaColorAnimation::beginThemeTransition();
        }
        refreshWidgets();
    });
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

QMargins ElaTheme::getShadowMargins(int shadowBorderWidth) const
{
    const int width = qMax(0, shadowBorderWidth);
    return QMargins(qCeil(width * 0.75), qCeil(width * 0.5), qCeil(width * 0.75), width);
}

void ElaTheme::drawEffectShadow(QPainter* painter, QRect widgetRect, int shadowBorderWidth, int borderRadius,
                                qreal alphaRatio, const QColor& shadowColor)
{
    drawEffectShadow(painter, widgetRect,
                     QMargins(shadowBorderWidth, shadowBorderWidth, shadowBorderWidth, shadowBorderWidth), borderRadius,
                     alphaRatio, shadowColor);
}

void ElaTheme::drawEffectShadow(QPainter* painter, QRect widgetRect, const QMargins& shadowMargins, int borderRadius,
                                qreal alphaRatio, const QColor& shadowColor)
{
    Q_D(ElaTheme);
    const QRect body = widgetRect.marginsRemoved(shadowMargins);
    const int shadowBorderWidth =
        qMax(qMax(shadowMargins.left(), shadowMargins.right()), qMax(shadowMargins.top(), shadowMargins.bottom()));
    if (!painter || !painter->isActive() || shadowBorderWidth <= 0 || body.isEmpty() || alphaRatio <= 0)
    {
        return;
    }
    const qreal ratio = painter->device()->devicePixelRatioF();
    const int radius = qBound(0, borderRadius, qMin(body.width(), body.height()) / 2);
    // 圆角和模糊过渡保持原尺寸 仅拉伸直边 避免窗口缩放或卡片动画反复生成阴影
    const int edge = radius + shadowBorderWidth;
    const QSize bodySize(qMin(body.width(), 2 * edge + 1), qMin(body.height(), 2 * edge + 1));
    const QSize size =
        bodySize + QSize(shadowMargins.left() + shadowMargins.right(), shadowMargins.top() + shadowMargins.bottom());
    QPainterPath path;
    path.addRoundedRect(QRectF(QPointF(shadowMargins.left(), shadowMargins.top()), bodySize), radius, radius);
    const QPixmap shadow =
        d->_getShadowPixmap(path, size, ratio, shadowBorderWidth, qMax(qreal(1), alphaRatio), shadowColor);
    const int horizontalEdge = qMin(edge, bodySize.width() / 2);
    const int verticalEdge = qMin(edge, bodySize.height() / 2);
    const QMargins edges = shadowMargins + QMargins(horizontalEdge, verticalEdge, horizontalEdge, verticalEdge);
    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->setOpacity(painter->opacity() * qMin(qreal(1), alphaRatio));
    // 中心完全透明 裁去这部分避免九宫格拉伸时仍遍历面板内部像素
    const QRect center = widgetRect.marginsRemoved(edges);
    painter->setClipRegion(QRegion(widgetRect).subtracted(QRegion(center)), Qt::IntersectClip);
    qDrawBorderPixmap(painter, widgetRect, edges, shadow, QRect(QPoint(), size * ratio),
                      QMargins(qRound(edges.left() * ratio), qRound(edges.top() * ratio), qRound(edges.right() * ratio),
                               qRound(edges.bottom() * ratio)));
    painter->restore();
}

void ElaTheme::drawEffectShadow(QPainter* painter, const QPainterPath& contentPath, int shadowBorderWidth,
                                qreal alphaRatio, const QColor& shadowColor)
{
    Q_D(ElaTheme);
    if (!painter || !painter->isActive() || contentPath.isEmpty() || shadowBorderWidth <= 0 || alphaRatio <= 0)
    {
        return;
    }
    const QRect rect =
        contentPath.boundingRect().marginsAdded(QMarginsF(getShadowMargins(shadowBorderWidth))).toAlignedRect();
    const QPainterPath path = contentPath.translated(-rect.topLeft());
    const QPixmap shadow = d->_getShadowPixmap(path, rect.size(), painter->device()->devicePixelRatioF(),
                                               shadowBorderWidth, qMax(qreal(1), alphaRatio), shadowColor);
    painter->save();
    painter->setOpacity(painter->opacity() * qMin(qreal(1), alphaRatio));
    painter->drawPixmap(rect.topLeft(), shadow);
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
