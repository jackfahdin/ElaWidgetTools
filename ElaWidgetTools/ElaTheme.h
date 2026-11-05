#ifndef ELAWORKSPACE_ELAWIDGETTOOLS_ELATHEME_H_
#define ELAWORKSPACE_ELAWIDGETTOOLS_ELATHEME_H_

#include "ElaPropertyMacro.h"
#include "ElaSingletonMacro.h"
#include "ElaWidgetToolsDef.h"
#include "ElaWidgetToolsExport.h"
#include <QColor>
#include <QObject>

#define eTheme ElaTheme::getInstance()
#define ElaThemeColor(themeMode, themeColor) eTheme->getThemeColor(themeMode, ElaThemeType::themeColor)
class QPainter;
class ElaThemePrivate;
class ELA_EXPORT ElaTheme : public QObject
{
    Q_OBJECT
    ELA_PIMPL_Q(ElaTheme)
    ELA_SINGLETON_H(ElaTheme)
private:
    explicit ElaTheme(QObject* parent = nullptr);
    ~ElaTheme();

public:
    void setThemeMode(ElaThemeType::ThemeMode themeMode);
    ElaThemeType::ThemeMode getThemeMode() const;

    // 跟随策略与实际模式分离；手动设置模式会退出跟随。
    void setFollowSystemTheme(bool follow);
    bool getFollowSystemTheme() const;

    void setAccentColor(const QColor& color);
    void setAccentColor(ElaThemeType::ThemeMode themeMode, const QColor& color);
    const QColor& getAccentColor(ElaThemeType::ThemeMode themeMode) const;
    void resetThemeColors();

    void drawEffectShadow(QPainter* painter, QRect widgetRect, int shadowBorderWidth, int borderRadius,
                          qreal alphaRatio = 1.0, const QColor& shadowColor = {});

    void setThemeColor(ElaThemeType::ThemeMode themeMode, ElaThemeType::ThemeColor themeColor, const QColor& newColor);
    const QColor& getThemeColor(ElaThemeType::ThemeMode themeMode, ElaThemeType::ThemeColor themeColor);
Q_SIGNALS:
    Q_SIGNAL void followSystemThemeChanged(bool follow);
    Q_SIGNAL void themeColorsChanged(ElaThemeType::ThemeMode themeMode);
    Q_SIGNAL void themeModeChanged(ElaThemeType::ThemeMode themeMode);
};

#endif // ELAWORKSPACE_ELAWIDGETTOOLS_ELATHEME_H_
