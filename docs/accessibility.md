# 键盘操作与无障碍

[返回首页](../README.md) · [基础组件示例](../ElaWidgetToolsExample/ExamplePage/T_BaseComponents.cpp) · [测试指南](testing.md)

本轮补齐 `ElaToggleButton` 的键盘操作，并为它和 `ElaToggleSwitch` 增加 Qt 无障碍接口。两者保留原有 API 和 QWidget 继承关系。

## 切换按钮和开关

| 操作 | 行为 |
| --- | --- |
| Tab / Shift+Tab | 按焦点顺序前进／后退，聚焦时显示提示框线 |
| 空格 | 松开时切换一次，自动重复不连续切换 |
| 左键点击 | 配对按下、在控件内松开时切换；开关仍支持拖动 |
| 右键、孤立的释放事件 | 不切换 |
| Esc | 有待完成的按键或鼠标操作时取消；空闲时继续交给父窗口处理 |
| 失焦、隐藏、禁用、窗口失去激活 | 取消未完成操作，不补发切换 |

`setIsToggled()` 只在实际状态变化时发送一次 `toggled(bool)`；同值设置不再重复通知。切换按钮的程序设值会停止旧的过渡动画。应用可以在 `toggled` 槽中设置新状态或销毁控件。

键盘焦点的实际遍历仍受平台设置、父窗口和应用的 `setFocusPolicy()` / `setTabOrder()` 影响。

## 为控件设置用途名称

```cpp
auto* notifications = new ElaToggleSwitch(parent);
notifications->setAccessibleName("启用通知");
notifications->setAccessibleDescription("控制是否显示桌面通知");

auto* pinned = new ElaToggleButton("固定工具栏", parent);
// 默认使用按钮可见文字作名称；需要更明确的描述时再覆盖。
pinned->setAccessibleName("固定顶部工具栏");
```

开关没有内置文字，应用应设置 `accessibleName`，说明它控制什么。名称不必包含“打开／关闭”，状态由无障碍接口单独提供。不要只把用途放在 tooltip 中。

切换按钮优先使用明确设置的 `accessibleName`；未设置时回退到 `getText()`，文字修改后发送名称变化通知。明确名称不会被 `setText()` 覆盖。描述使用 QWidget 原有的 `accessibleDescription`。

## Qt 无障碍接口

- `ElaToggleSwitch` 的角色为 `QAccessible::Switch`；`ElaToggleButton` 为可选中的 `QAccessible::Button`。
- 提供可选中、已选中、可聚焦、已聚焦、禁用等状态；选中状态变化会发送 Qt 无障碍通知。
- 提供 `toggleAction()`、`pressAction()` 和 Qt 的 `setFocusAction()`。切换／按下动作都切换一次；控件禁用或隐藏时不执行操作。
- 工厂随控件构造注册，无需应用安装适配器；正常的应用字体和资源初始化仍按原有方式调用 `eApp->init()`。
- 继承 Qt 原生控件的 `ElaCheckBox`、`ElaSlider` 等继续使用 Qt 已有的无障碍接口。

应用不需要包含库的私有无障碍头文件，也不应把内部接口指针作为长期持有的控件引用。

## Example 与验证边界

基础组件页提供键盘提示、切换后的文字反馈和禁用操作。开关与禁用开关设置了用途名称；设置页的预览、日志和用户卡片开关也有名称。

QtTest 验证输入取消、重复通知、焦点导航、回调销毁，以及通过 `QAccessible::queryAccessibleInterface()` 查询角色、名称、状态和执行动作。通知测试观察 Qt 的更新接口，验证状态和名称通知。

这些测试验证 Qt 层面的接口行为，**不等于已完成 NVDA、VoiceOver 或 Orca 的真实读屏体验验证**。读屏工具、桌面无障碍服务、焦点策略和平台桥接仍需在对应设备上验证。Windows / macOS 编译和 QtTest 通过 CI 进行，本地只验证 Linux。

后续逐步覆盖其他自绘控件；当前不宣称整个组件库已经完整支持无障碍。
