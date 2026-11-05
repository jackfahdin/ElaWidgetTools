#ifndef ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELATHEMEPRIVATE_H_
#define ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELATHEMEPRIVATE_H_

#include "ElaPropertyMacro.h"
#include <QColor>
#include <array>
#include <QMap>
#include <QObject>

#include "ElaWidgetToolsDef.h"
class ElaTheme;
class ElaThemePrivate : public QObject
{
    Q_OBJECT
    ELA_PIMPL_D(ElaTheme)
public:
    explicit ElaThemePrivate(QObject* parent = nullptr);
    ~ElaThemePrivate();

private:
    ElaThemeType::ThemeMode _themeMode{ElaThemeType::Light};
    bool _followSystemTheme{false};
    QMetaObject::Connection _systemThemeConnection;
    std::array<QColor, ElaThemeType::Win10BorderInactive + 1> _lightThemeColorList;
    std::array<QColor, ElaThemeType::Win10BorderInactive + 1> _darkThemeColorList;
    void _applyThemeMode(ElaThemeType::ThemeMode mode);
    bool _setAccentColor(ElaThemeType::ThemeMode mode, const QColor& color);
    void _initThemeColor();
};

#endif // ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELATHEMEPRIVATE_H_
