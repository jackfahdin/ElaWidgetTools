#include "ElaThemePrivate.h"
#include <QIODevice>
#include "ElaTheme.h"
#include "ElaColorAnimation.h"

#include "ElaExponentialBlur.h"

#include <QDataStream>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

ElaThemePrivate::ElaThemePrivate(QObject* parent) : QObject(parent)
{
    _initThemeColor();
}

void ElaThemePrivate::_initThemeColor()
{
    // 按 ThemeColor 顺序集中维护浅色和深色配对 透明色叠加到实际背景
    const QRgb colors[][2] = {
        {0xFF8A8A8A, 0xFF9F9F9F}, // ScrollBarHandle
        {0xFF6A6A6A, 0xFFD0D0D0}, // ToggleSwitchNoToggledCenter
        {0xFFF3F3F3, 0xFF202020}, // WindowBase
        {0x5AF8F8F8, 0x3C3E3E3E}, // WindowCentralStackBase
        {0xFF0067C0, 0xFF60CDFF}, // PrimaryNormal
        {0xE60067C0, 0xE660CDFF}, // PrimaryHover
        {0xCC0067C0, 0xCC60CDFF}, // PrimaryPress
        {0xFFC8C8C8, 0xFF474747}, // PopupBorder
        {0xFFBCBCBC, 0xFF545454}, // PopupBorderHover
        {0xFFF9F9F9, 0xFF2C2C2C}, // PopupBase
        {0xFFEEEEEE, 0xFF383838}, // PopupHover
        {0xFFF7F7F7, 0xFF1F1F1F}, // DialogBase
        {0xFFEDEDED, 0xFF202020}, // DialogLayoutArea
        {0xE4000000, 0xFFFFFFFF}, // BasicText
        {0xFFFFFFFF, 0xFF000000}, // BasicTextInvert
        {0xFF878787, 0xFFADADB0}, // BasicDetailsText
        {0x9E000000, 0xC5FFFFFF}, // BasicTextNoFocus
        {0x5C000000, 0x5DFFFFFF}, // BasicTextDisable
        {0xFF5A5A5D, 0xFFBBBBBF}, // BasicTextPress
        {0xFF5C5C5F, 0xFFCCCCCE}, // BasicTextCategory
        {0x0F000000, 0x12FFFFFF}, // BasicBorder
        {0x72000000, 0x8BFFFFFF}, // BasicBorderDeep
        {0xFFC8C8C8, 0xFF575757}, // BasicBorderHover
        {0xB3FFFFFF, 0x0FFFFFFF}, // BasicBase
        {0xFFE6E6E6, 0xFF616161}, // BasicBaseDeep
        {0x4DF9F9F9, 0x0BFFFFFF}, // BasicDisable
        {0x80F9F9F9, 0x15FFFFFF}, // BasicHover
        {0x4DF9F9F9, 0x08FFFFFF}, // BasicPress
        {0xFFEBEBEB, 0xFF383838}, // BasicSelectedHover
        {0x29000000, 0x18FFFFFF}, // BasicBaseLine
        {0xFF868686, 0xFF9A9A9A}, // BasicHemline
        {0x72000000, 0x8BFFFFFF}, // BasicIndicator
        {0xFFB3B3B3, 0xFF636363}, // BasicChute
        {0xA0EFEFEF, 0x7D454545}, // BasicAlternating
        {0xB3FFFFFF, 0x0DFFFFFF}, // BasicBaseAlpha
        {0xA0C8C8C8, 0x5F727272}, // BasicBaseDeepAlpha
        {0x09000000, 0x0FFFFFFF}, // BasicHoverAlpha
        {0x06000000, 0x0AFFFFFF}, // BasicPressAlpha
        {0x6ECECECE, 0x4B4B4B4B}, // BasicSelectedAlpha
        {0x50D0D0D0, 0x374B4B4B}, // BasicSelectedHoverAlpha
        {0xFFC42B1C, 0xFFFF99A4}, // StatusDanger
        {0xFF6E6E6E, 0xFF333333}, // Win10BorderActive
        {0xFFA7A7A7, 0xFF3D3D3D}, // Win10BorderInactive
        {0xFFFFFFFF, 0xB31E1E1E}, // InputActive
        {0xFF0F7B0F, 0xFF6CCB5F}, // StatusSuccess
        {0xFFDFF6DD, 0xFF393D1B}, // StatusSuccessBackground
        {0xFF9D5D00, 0xFFFCE100}, // StatusWarning
        {0xFFFFF4CE, 0xFF433519}, // StatusWarningBackground
        {0xFFFDE7E9, 0xFF442726}, // StatusDangerBackground
        {0x37000000, 0x28FFFFFF}, // PrimaryDisabled
        {0x06000000, 0x19000000}, // BasicAltBase
        {0x0F000000, 0x0BFFFFFF}, // BasicAltHover
        {0x18000000, 0x12FFFFFF}, // BasicAltPress
        {0xFFFFFFFF, 0xFF454545}, // BasicSolid
    };
    static_assert(sizeof(colors) / sizeof(colors[0]) == ElaThemeType::ThemeColorCount, "主题色表必须覆盖全部颜色角色");
    for (int index = 0; index < ElaThemeType::ThemeColorCount; ++index)
    {
        _lightThemeColorList[index] = QColor::fromRgba(colors[index][0]);
        _darkThemeColorList[index] = QColor::fromRgba(colors[index][1]);
    }
}

ElaThemePrivate::~ElaThemePrivate()
{
}

QPixmap ElaThemePrivate::_getShadowPixmap(const QPainterPath& path, const QSize& size, qreal devicePixelRatio,
                                          int shadowBorderWidth, qreal alphaRatio, const QColor& shadowColor)
{
    const QColor color = shadowColor.isValid() ? shadowColor : QColor(Qt::black);
    QByteArray key;
    QDataStream stream(&key, QIODevice::WriteOnly);
    stream << path << size << devicePixelRatio << shadowBorderWidth << alphaRatio << color.rgba()
           << static_cast<int>(_themeMode);
    if (const auto* cached = _shadowCache.object(key))
    {
        return *cached;
    }
    const qreal scale = shadowBorderWidth / 32.0;
    const QSize pixelSize(qCeil(size.width() * devicePixelRatio), qCeil(size.height() * devicePixelRatio));
    QImage mask(pixelSize, QImage::Format_ARGB32_Premultiplied);
    mask.setDevicePixelRatio(devicePixelRatio);
    mask.fill(Qt::transparent);
    {
        QPainter painter(&mask);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);
        painter.drawPath(path);
    }
    QPixmap shadow(pixelSize);
    shadow.setDevicePixelRatio(devicePixelRatio);
    shadow.fill(Qt::transparent);
    QPainter painter(&shadow);
    for (int layer = 0; layer < 2; ++layer)
    {
        const quint16 radius = static_cast<quint16>(qMax(1, qRound((layer == 0 ? 3 : 9) * scale * devicePixelRatio)));
        QPixmap blurred = ElaExponentialBlur::doExponentialBlur(mask, radius);
        blurred = ElaExponentialBlur::doExponentialBlur(blurred.toImage(), radius);
        blurred.setDevicePixelRatio(devicePixelRatio);
        const qreal opacity =
            _themeMode == ElaThemeType::Light ? (layer == 0 ? 0.12 : 0.20) : (layer == 0 ? 0.20 : 0.38);
        painter.setOpacity(qMin(qreal(1), opacity * alphaRatio));
        painter.drawPixmap(QPointF(0, (layer == 0 ? 2 : 8) * scale), blurred);
    }
    painter.setOpacity(1);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(QRect(QPoint(), size), color);
    // 清除内容内部的阴影 半透明面板只叠加自身主题色
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawPath(path);
    painter.end();
    const int cost = qMax(1, qCeil(static_cast<qreal>(shadow.width()) * shadow.height() * 4 / 1024));
    _shadowCache.insert(key, new QPixmap(shadow), cost);
    return shadow;
}

void ElaThemePrivate::_applyThemeMode(ElaThemeType::ThemeMode mode)
{
    Q_Q(ElaTheme);
    if (_themeMode == mode)
    {
        return;
    }
    _themeMode = mode;
    ElaColorAnimation::beginThemeTransition();
    Q_EMIT q->themeModeChanged(mode);
}

bool ElaThemePrivate::_setAccentColor(ElaThemeType::ThemeMode mode, const QColor& color)
{
    auto& colors = mode == ElaThemeType::Light ? _lightThemeColorList : _darkThemeColorList;
    // 浅色主题交互态逐渐变亮，深色主题逐渐变暗。
    const QColor hover = mode == ElaThemeType::Light ? color.lighter(110) : color.darker(110);
    const QColor press = mode == ElaThemeType::Light ? color.lighter(120) : color.darker(120);
    const bool changed = colors[ElaThemeType::PrimaryNormal] != color || colors[ElaThemeType::PrimaryHover] != hover ||
                         colors[ElaThemeType::PrimaryPress] != press;
    colors[ElaThemeType::PrimaryNormal] = color;
    colors[ElaThemeType::PrimaryHover] = hover;
    colors[ElaThemeType::PrimaryPress] = press;
    return changed;
}
