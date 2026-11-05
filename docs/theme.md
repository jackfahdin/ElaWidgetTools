# 主题与配色

[返回首页](../README.md) · [设置页示例](../ElaWidgetToolsExample/ExamplePage/T_Setting.cpp) · [测试指南](testing.md)

`ElaTheme` 提供浅色、深色、系统主题跟随及全局配色。默认仍为手动浅色模式，不改变现有应用的启动行为。所有主题接口在创建 `QApplication` 后、GUI 线程中调用；配置是否保存由应用自行决定。

## 选择主题

```cpp
#include "ElaTheme.h"

// 手动模式：同时退出系统跟随。
eTheme->setThemeMode(ElaThemeType::Light);
eTheme->setThemeMode(ElaThemeType::Dark);

// 跟随 Qt 从桌面读取的主题偏好。
eTheme->setFollowSystemTheme(true);

// 停止跟随，保留当前实际浅色／深色模式。
eTheme->setFollowSystemTheme(false);
```

`getThemeMode()` 始终返回实际使用的 `Light` 或 `Dark`，现有控件不需要处理第三种绘制模式。使用 `getFollowSystemTheme()` 判断是否跟随系统。

| 通知 | 触发条件 |
| --- | --- |
| `themeModeChanged(ThemeMode)` | 实际浅色／深色模式改变 |
| `followSystemThemeChanged(bool)` | 系统跟随开关改变 |
| `themeColorsChanged(ThemeMode)` | 对应模式的配色发生变化 |

重复设置相同状态不会重复发送通知。即使实际模式没有改变，手动调用 `setThemeMode()` 也会关闭系统跟随；设置页同时监听前两个信号，确保选择项正确同步。

系统检测使用 Qt 公共 `QStyleHints::colorScheme` 和 `colorSchemeChanged`，不修改操作系统主题。桌面未报告偏好（`Unknown`）时采用浅色。在 Linux 上检测能力取决于 Qt 平台插件、桌面和门户配置；无法检测时仍可手动选择。macOS 的真实设备交互仍待验证。

## 强调色与颜色角色

```cpp
// 一次设置浅、深两套配色的强调色。
eTheme->setAccentColor(QColor("#9250b5"));

// 也可以为深色主题选用更亮的强调色。
eTheme->setAccentColor(ElaThemeType::Dark, QColor("#c899e3"));
QColor accent = eTheme->getAccentColor(ElaThemeType::Dark);

// 精细控制某一个颜色角色。
eTheme->setThemeColor(ElaThemeType::Light, ElaThemeType::WindowBase, QColor("#f4f6fa"));

// 恢复两套默认配色，不改变当前模式或跟随策略。
eTheme->resetThemeColors();
```

强调色同时更新 `PrimaryNormal`、`PrimaryHover`、`PrimaryPress`。浅色模式交互态逐渐变亮，深色模式逐渐变暗；每套发生变化的配色只发出一次 `themeColorsChanged`。这套接口不会自动计算文字对比度，应用应为浅、深背景选择易于辨认的颜色；也可通过 `setThemeColor()` 分别覆盖交互态。

配色修改会刷新已创建的控件及已打开的菜单，对话框主按钮也会同步强调色。它不会伪造 `themeModeChanged`，因此单独修改颜色不会触发模式切换逻辑。自定义控件若缓存了画刷或图片，应监听 `themeColorsChanged` 更新缓存。

无效 `QColor`、非法模式和越界颜色角色不会改变全局配色；非法颜色读取返回无效 `QColor`。不要把这个行为用于判断未来版本是否增加了颜色角色，应使用当前头文件中的枚举。

## 控件单独配色

`ElaPushButton` 和 `ElaIconButton` 默认实时读取主题颜色。通过控件 setter 指定的颜色优先于全局配色，恢复全局默认配色也不会清除这些局部覆盖。

```cpp
button->setLightDefaultColor(QColor("#207050")); // 固定这个按钮的浅色背景
button->setLightDefaultColor(QColor());          // 取消覆盖，重新跟随主题
```

这里的无效 `QColor` 表示取消**控件局部覆盖**；`ElaTheme::setThemeColor()` 则忽略无效颜色，两者用途不同。图片、用户自定义样式表和显式设置的固定颜色不会被全局主题改写。

## 示例与键盘操作

Example 的设置页提供主题选择、颜色对话框、恢复默认配色，以及复选框、滑块和开关预览。选择的强调色同时应用于两套主题，关闭示例后不自动保存。

`ElaToggleSwitch` 支持 Tab 聚焦、空格按下后释放切换，并绘制焦点提示。右键不会改变状态；失去焦点、禁用或鼠标在控件外释放时取消未完成的操作。拖动后状态不变时不发送 `toggled`，键盘自动重复也不会反复切换。
