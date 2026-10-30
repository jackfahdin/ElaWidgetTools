# 标题栏布局

[返回组件目录](components.md)

`ElaWindow` 使用 `ElaAppBar` 绘制标题栏。Linux 版布局从左到右为：

```text
[后退][前进][导航][图标][自定义区] ← 空白 → [窗口标题] ← 空白 → [主题][最小化][最大化][关闭]
```

标题在**自定义区右边缘与右侧按钮组左边缘之间**水平居中。左侧内容、窗口大小或右侧按钮显隐变化后，布局自动重新计算；不是相对整个窗口居中。

Windows 版在主题按钮之前保留置顶按钮。

## Linux 置顶策略

Linux 版统一移除应用内置顶功能，适用于 Wayland 和 X11 / XWayland：不创建置顶按钮，也不向窗口系统发送置顶请求。需要置顶时，可使用 KDE 等桌面提供的窗口菜单或窗口规则。

为保持跨平台代码可编译，`ElaAppBar`、`ElaWindow`、`ElaWidget`、`ElaDialog` 的 `setIsStayTop()` / `getIsStayTop()` 接口仍保留：Linux 下设置被忽略，查询始终为 `false`，不会因设置产生置顶状态变更通知。`StayTopButtonHint` 自动从有效按钮标志中移除，不能通过显隐接口重新启用。桌面手动设置的置顶状态不由这些接口读取或清除。

Windows 继续使用原有置顶实现。

## 设置标题和自定义内容

```cpp
setWindowTitle("我的应用");
setCustomWidget(ElaAppBarType::LeftArea, customWidget);
```

标题直接跟随 `QWidget::windowTitle` 更新，空标题隐藏文字。标题按纯文本显示；空间不足时在自己的区域内裁剪，鼠标悬停可查看完整标题，不挤占自定义内容和系统按钮。

Example 的菜单栏已经放入 `LeftArea`，搜索框仍属于内容区，工具栏仍是标题栏下面的独立一行。

菜单区按内容宽度自适应，不再固定宽度：空间足够时完整显示，窗口缩窄、可用空间不足时自动收缩，多出的菜单项收进右侧的 `>` 按钮；窗口变宽后自动恢复显示。剩余空间用于居中显示标题。

`ElaMenuBar` 本身也允许水平收缩，避免 Qt 默认的菜单尺寸策略阻止折叠。

自定义控件的宽度由 Qt 布局结合 `sizeHint()`、`minimumSizeHint()` 和 `sizePolicy()` 分配。需要类似的自适应行为时，可使用水平 `QSizePolicy::Preferred`，避免设置固定宽度；普通控件如何收缩或折叠仍由控件自身决定。

## 原有自定义接口

为保留现有调用，`LeftArea`、`MiddleArea`、`RightArea` 的枚举值及方法签名不变，但三个入口现在依次排列在同一个自定义区中，均位于图标之后、窗口标题之前。新代码通常只使用 `LeftArea`，多个控件可先放入自己的布局容器。

- `getCustomWidget(area)` 返回该入口的控件；无效枚举返回 `nullptr`。
- 设置新控件时，标题栏接管其父子关系；旧控件被隐藏并保留为标题栏子对象，不立即删除。
- 不要把同一个控件同时设置到两个入口。空指针或无效枚举设置会被忽略。
- 临时隐藏自定义内容可对取到的控件调用 `hide()`，恢复调用 `show()`；标题会自动重新居中。
- 自定义内容默认作为可交互区域。原有 `hitTestObject` / `hitTestFunctionName` 参数仍可指定可拖动的位置。

标题栏默认高度为 45。`setAppBarHeight()` 同步调整自定义控件的最大高度；原有按钮高度及垂直对齐方式沿用现有实现。按钮显隐继续使用 `setWindowButtonFlag()` / `setWindowButtonFlags()`。
