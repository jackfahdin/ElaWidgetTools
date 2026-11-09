# 切换控件键盘与无障碍实现计划

> 使用 executing-plans 按任务实现，合并前使用 requesting-code-review 独立审查。

**目标：** 补齐现有 ElaToggleButton 的输入行为，并为它与 ElaToggleSwitch 提供 Qt 无障碍语义。

**架构：** 保留 QWidget 继承及已有 API，在原控件内修复输入状态。内部无障碍工厂通过 QAccessibleWidget 适配这两个类，不替换已有 Qt 原生控件的接口。

**技术栈：** C++17、Qt 6.12.0 及以上、QtTest、CMake Presets + Ninja。

## 方案边界

改继承为 QAbstractButton 会扩大 API／对象模型变化；为每个控件公开新无障碍类没有必要。本轮采用内部工厂和现有 setAccessibleName/setAccessibleDescription。切换按钮默认用可见文本作名称，开关需要应用提供用途名称。角色分别为 Button（checkable）和 Switch。

保证 Tab/Shift+Tab 导航、空格释放切换一次、忽略自动重复、禁用／失焦／隐藏／Escape 取消待完成输入；鼠标仅接受配对的左键内部释放。程序设置同值不发 toggled。复用一个动画，避免旧动画覆盖程序设值。公开状态通知允许应用重入或销毁控件。

无障碍接口提供 checked、checkable、focused、focusable、disabled 状态、toggle／press／setFocus 动作与空格键说明。更新 checked 发送 Qt 状态变化通知。通过 Qt 接口测试不代表已完成各平台真实读屏器体验验证。

## 任务 0：修正搜索 CI 用例

文件：`tests/tst_suggest.cpp`。

- [x] 根据 CI 42557711029 的 macOS LastTest.log 修正用例组织。失败发生在混合用例的第二段请求等待，实际 requests=0。
- [x] 本地键盘选择先等待窗口暴露及输入焦点；异步 Escape 取消使用独立控件，避免混合原生窗口焦点事件。
- [x] 本机运行 suggest，独立中文提交；macOS 修正结果交 CI 验证。

## 任务 1：输入与状态回归

文件：`ElaWidgetTools/ElaToggleButton.h/.cpp`、`private/ElaToggleButtonPrivate.h`、`ElaToggleSwitch.cpp`、`private/ElaToggleSwitchPrivate.cpp`、`tests/tst_controls.cpp`。

- [x] 先编写并运行失败测试：右键和孤立释放不切换、左键外部释放取消、Tab/Shift+Tab、空格／自动重复／中断、同值设定、toggled 槽销毁控件。
  ```cpp
  ElaToggleButton button;
  QSignalSpy spy(&button, &ElaToggleButton::toggled);
  QTest::mouseClick(&button, Qt::RightButton);
  QCOMPARE(spy.count(), 0);
  QTest::keyClick(&button, Qt::Key_Space);
  QCOMPARE(spy.count(), 1);
  ```
- [x] 实现配对输入、取消、焦点绘制和单一切换动画；修正开关通知后继续访问自身的生命周期问题。
- [x] `xvfb-run -a build/linux-debug/tests/tst_controls` 全部通过。

## 任务 2：无障碍接口

文件：新增 `ElaWidgetTools/private/ElaToggleAccessible.h/.cpp` 和 `tests/tst_accessibility.cpp`；修改两种控件构造及通知、`tests/CMakeLists.txt`。

- [x] 先验证接口缺失：
  ```cpp
  ElaToggleSwitch toggle;
  auto* iface = QAccessible::queryAccessibleInterface(&toggle);
  QCOMPARE(iface->role(), QAccessible::Switch);
  QVERIFY(iface->state().checkable);
  ```
- [x] 工厂在控件构造时注册一次，支持独立使用。名称优先 accessibleName，切换按钮回退可见文本；禁用或隐藏时不执行切换动作。
- [x] 测试角色、名称／描述、状态变化、toggle／press／setFocus、禁用、删除；确认继承 QCheckBox/QSlider 的控件仍可使用 Qt 原生接口。

## 任务 3：示例、文档与交付

文件：`ElaWidgetToolsExample/ExamplePage/T_BaseComponents.cpp`、新增 `docs/accessibility.md`、`README.md`、`docs/testing.md`、`docs/roadmap.md`。

- [x] 基础组件页加入键盘提示、用途名称和状态反馈。
- [x] 文档说明操作方式、应用设置名称、Qt 接口覆盖及真实读屏器验证边界。
- [x] Linux Debug/Release 构建、全部 QtTest、Example 启动、固定版本格式检查和 git diff --check。
- [x] 独立审查后分类中文提交、快进 main 并推送；Windows/macOS 原生构建与测试交 CI。

## 验证记录

- 上一轮 CI 的 macOS 日志显示搜索混合用例 requests=0。拆分用例后，CI 42559980880 进一步定位到 `edit->hasFocus()` 未成立；后续使用普通父窗口和 Qt 活动窗口状态构造确定的键盘测试环境。平台修正结果由后续 CI 确认。
- 原八组 Linux Debug 基线通过。新增测试先复现重复通知、非左键误切换、缺少键盘行为及无障碍角色。
- 独立审查补充并修复空闲 Escape 阻断父窗口、取消按压半径未恢复、无障碍回调导致公开信号顺序倒置，新增回归先失败后通过，原始探针复验通过。
- Qt 6.12.0 / Linux Debug 与 Release Ninja 构建完成，九组 QtTest 全部通过，两种配置的 Example 启动检查正常退出。
- 固定版本 clang-format、cmake-format 和 git diff --check 通过；本地没有执行 Windows/macOS 跨平台编译。
- 无障碍测试不调用 eApp->init，验证控件独立注册；覆盖 Qt 更新通知和原生控件接口兼容。真实读屏器体验未验证。
