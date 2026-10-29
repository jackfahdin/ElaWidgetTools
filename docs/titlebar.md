# 标题栏布局

[返回组件目录](components.md)

`ElaWindow` 使用 `ElaAppBar` 绘制标题栏。布局从左到右为：

```text
[后退][前进][导航][图标][自定义区] ← 空白 → [窗口标题] ← 空白 → [置顶][主题][最小化][最大化][关闭]
```

标题在**自定义区右边缘与右侧按钮组左边缘之间**水平居中。左侧内容、窗口大小或右侧按钮显隐变化后，布局自动重新计算；不是相对整个窗口居中。

## 设置标题和自定义内容

```cpp
setWindowTitle("我的应用");
setCustomWidget(ElaAppBarType::LeftArea, customWidget);
```

标题直接跟随 `QWidget::windowTitle` 更新，空标题隐藏文字。标题按纯文本显示；空间不足时在自己的区域内裁剪，鼠标悬停可查看完整标题，不挤占自定义内容和系统按钮。

Example 的菜单栏已经放入 `LeftArea`，搜索框仍属于内容区，工具栏仍是标题栏下面的独立一行。

## 原有自定义接口

为保留现有调用，`LeftArea`、`MiddleArea`、`RightArea` 的枚举值及方法签名不变，但三个入口现在依次排列在同一个自定义区中，均位于图标之后、窗口标题之前。新代码通常只使用 `LeftArea`，多个控件可先放入自己的布局容器。

- `getCustomWidget(area)` 返回该入口的控件；无效枚举返回 `nullptr`。
- 设置新控件时，标题栏接管其父子关系；旧控件被隐藏并保留为标题栏子对象，不立即删除。
- 不要把同一个控件同时设置到两个入口。空指针或无效枚举设置会被忽略。
- 临时隐藏自定义内容可对取到的控件调用 `hide()`，恢复调用 `show()`；标题会自动重新居中。
- 自定义内容默认作为可交互区域。原有 `hitTestObject` / `hitTestFunctionName` 参数仍可指定可拖动的位置。

标题栏默认高度为 45。`setAppBarHeight()` 同步调整自定义控件的最大高度；原有按钮高度及垂直对齐方式沿用现有实现。按钮显隐继续使用 `setWindowButtonFlag()` / `setWindowButtonFlags()`。
