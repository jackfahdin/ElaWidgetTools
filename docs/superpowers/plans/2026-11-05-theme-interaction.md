# 主题系统与关键交互实现计划

> **面向 AI 代理的工作者：** 使用 executing-plans 逐项执行，先验证失败测试，再实现及回归。

**目标：** 补齐系统主题跟随、强调色与即时配色刷新，并让现有开关支持键盘操作。

**架构：** 保留 `ThemeMode::Light/Dark` 及默认浅色行为，新增独立的跟随开关。通过 Qt 公共 `QStyleHints` 监听系统主题；手动设置主题退出跟随，关闭跟随保留当前实际主题，系统未报告主题时采用浅色。配色使用独立通知，不冒充模式切换；库不保存应用偏好。

**技术栈：** Qt 6.12.0 及以上、C++、QtTest、CMake Presets、Ninja。

## 任务一：主题状态与通知

文件：`ElaWidgetTools/ElaTheme.{h,cpp}`、`ElaWidgetTools/private/ElaThemePrivate.{h,cpp}`、`tests/tst_theme.cpp`、`tests/CMakeLists.txt`。

- [x] 新建 theme 测试组，覆盖重复模式无通知、系统 Light/Dark/Unknown、手动模式退出跟随、禁用跟随后系统变化无效。
- [x] 新增 `setFollowSystemTheme(bool)`、`getFollowSystemTheme()`、`followSystemThemeChanged(bool)`；`getThemeMode()` 始终返回实际浅/深模式。
- [x] 新增 `setAccentColor(const QColor&)` 与 `setAccentColor(ThemeMode, const QColor&)`，更新 PrimaryNormal/Hover/Press；`getAccentColor(ThemeMode)` 返回实际主色。
- [x] 新增 `themeColorsChanged(ThemeMode)` 和 `resetThemeColors()`。每个修改的模式一次通知，恢复配色不改变主题策略。忽略无效颜色和非法模式／角色，非法读取返回无效 QColor。

测试核心断言：
```cpp
QSignalSpy changed(eTheme, &ElaTheme::themeModeChanged);
eTheme->setThemeMode(eTheme->getThemeMode());
QCOMPARE(changed.count(), 0);
```

## 任务二：配色传播

文件：`ElaTheme.cpp`、`ElaPushButton.cpp`、`ElaIconButton.cpp`、`ElaColorDialog.cpp`、`ElaContentDialog.cpp`，以及使用 QPalette 缓存文字色的控件。

- [x] 添加已显示窗口／菜单／控件重绘和文字调色板回归，观察旧实现失败。
- [x] 主题切换、配色修改后请求现有 QWidget 重绘；缓存调色板的控件订阅配色通知，仅刷新当前模式。
- [x] 按钮的默认颜色动态读取主题；显式指定的局部颜色保持优先，无效 QColor 可恢复主题默认值。对话框主按钮继续跟随强调色。

测试核心断言：
```cpp
ElaPushButton button;
eTheme->setThemeColor(ElaThemeType::Light, ElaThemeType::BasicBase, QColor("#123456"));
QCOMPARE(button.getLightDefaultColor(), QColor("#123456"));
```

## 任务三：开关交互

文件：`ElaToggleSwitch.{h,cpp}`、`private/ElaToggleSwitchPrivate.{h,cpp}`、`tests/tst_controls.cpp`。

- [x] 先复现右键误切换和空格无操作，补充键盘焦点、重复键、禁用、焦点丢失取消、拖动状态不变不发信号的测试。
- [x] 支持 Tab 焦点、空格按下再释放切换、可见焦点；忽略非左键与自动重复，取消失焦／禁用时的未完成操作。
- [x] 保留现有鼠标拖动操作，`toggled` 仅在状态实际改变时发出。

## 任务四：示例、文档与交付

文件：`ElaWidgetToolsExample/ExamplePage/T_Setting.cpp`、`docs/theme.md`、`docs/testing.md`、`docs/roadmap.md`、`README.md`。

- [x] 设置页增加跟随系统、强调色选择、恢复默认配色及键盘操作提示，使用现有组件。
- [x] 中文文档解释实际模式、跟随优先级、配色信号、局部覆盖和平台检测限制，不强制保存配置。
- [x] 使用固定版本 clang-format / cmake-format，执行 `python tools/check-format.py`。
- [x] 执行 Linux Debug / Release 构建及 QtTest；Windows/macOS 交给 CI，保持 macOS 实验性说明。
- [x] 代码审查后按主题、交互、示例文档分类中文提交，快进 main 并推送，不移动 v2.0.0。

验证命令（隔离工作区）：
```bash
QT_ROOT=/home/zz/Qt/6.12.0/gcc_64 cmake --preset linux-debug -DELAWIDGETTOOLS_BUILD_TESTS=ON
cmake --build --preset linux-debug --parallel 6
xvfb-run -a ctest --preset linux-debug
QT_ROOT=/home/zz/Qt/6.12.0/gcc_64 cmake --preset linux-release -DELAWIDGETTOOLS_BUILD_TESTS=ON
cmake --build --preset linux-release --parallel 6
xvfb-run -a ctest --preset linux-release
```

## 验证记录

- 原有四组测试基线通过；新增测试先复现重复主题通知、默认按钮旧配色、右键误切换、空格无响应和取消交互问题。
- 审查补充了图标按钮动画透明度、PopularCard 强调色及 Dock 按钮颜色刷新，并验证修复。
- Linux / Qt 6.12.0：Debug、Release 的库与 Example 构建成功；各五组 QtTest 通过，两个 Example 启动检查正常退出。
- 固定工具版本格式检查和 `git diff --check` 通过。
- Windows/macOS 未在本机编译；原生构建和运行由推送后的 CI 执行。系统主题测试模拟 Qt 通知，不等同于真实桌面集成验证。
