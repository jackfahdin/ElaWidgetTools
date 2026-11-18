# 上游同步记录

[返回首页](../README.md) · [维护与路线图](roadmap.md) · [组件目录](components.md)

## 2026-11-18：同步 V3.0 重构与下拉框优化

来源为 [Liniyous/ElaWidgetTools](https://github.com/Liniyous/ElaWidgetTools)，本次从 `aae58f5` 同步至 [`18a9bdad86af2c2538dac7d76b0f185e610cd991`](https://github.com/Liniyous/ElaWidgetTools/commit/18a9bdad86af2c2538dac7d76b0f185e610cd991)，包含：

- `cfb761287ff8848f4f39496f8648b8adf97d54ff`：V3.0 视觉、动画与性能重构。
- `18a9bdad86af2c2538dac7d76b0f185e610cd991`：下拉框效果优化。

上游提交以内容整合的方式纳入，沿用本仓库精简后的提交历史；后续同步应以本记录中的完整上游提交为比较起点，不要对两个仓库执行无共同祖先的直接合并。

## 本次合入

统一控件颜色过渡、输入焦点动画、阴影缓存，更新下拉框、多选标签、日历、抽屉、菜单、导航与滚动效果。日志增加异步批量写入与 `flush()`。整合时修复标签页面外部销毁或移出后容器未同步的问题，防止后续关闭标签访问失效对象。新增 `ElaSplitter` 和 `ElaTeachingTip`；导航页展示可调节布局，弹出交互页展示教学提示。示例页采用上游新的分组布局。

Fluent System Icons 的两个箭头及来源、许可证随资源和安装包分发，安装位置为 `share/licenses/ElaWidgetTools/FluentSystemIcons`（可由 `CMAKE_INSTALL_DATADIR` 调整）。

## 本仓库保留的差异

- Qt 最低版本仍为 **6.12.0**，继续使用 Ninja、CMake Presets、现有安装包和 CI 配置。没有修改版本号或发布标签。
- 保留标题栏居中布局、自适应菜单、Linux 无置顶按钮、macOS 实验性适配。
- 保留页面懒加载、离开确认、异步搜索取消、页面状态容器、数据筛选示例。
- 保留系统主题跟随、强调色设置、运行时配色刷新、按钮局部配色优先，以及切换控件的键盘与无障碍接口。
- 原有 43 个 `ThemeColor` 枚举值保持不变，新颜色角色追加到末尾。上游重构后的控件使用统一的新角色；已不被新绘制路径使用的旧角色仍可读取和设置。
- `ElaReminderCard` 继续作为兼容组件保留；新教学提示使用 `ElaTeachingTip`。

## 标签容器迁移

本次沿用上游的标签容器重构，**不是完全兼容的接口更新**。应用应重新编译，并检查以下用法：

| 旧用法 | 本次接口 |
| --- | --- |
| 将 `ElaTabWidget` 转换为 `QTabWidget*` | 它现在继承 `QWidget`，通过自身的页面接口操作 |
| `tabBar()` | `getTabBar()`；固定标签行使用 `getTabBar(true)` |
| `setTabPosition()`、其它仅由 `QTabWidget` 提供的方法 | 新容器没有提供，布局为顶部固定标签行与普通标签行 |
| `ElaTabBar::tabDragCreate/Enter/Leave/Drop` | 使用 `tabDragRequested`、`tabDropRequested`；容器级使用 `tabTransferRequested` |
| 普通标签管理 | `addTab/insertTab/removeTab/currentWidget/setCurrentIndex` 等常用操作仍提供 |

新接口支持标签固定、跨容器转移和拖出浮窗。移除标签不会销毁页面；示例在关闭请求中显式调用 `deleteLater()`。新增使用方式见[导航示例](../ElaWidgetToolsExample/ExamplePage/T_Navigation.cpp)。

## 验证范围

本地执行 Linux 原生构建与 QtTest，新增 `upstream` 测试组覆盖分隔布局、标签固定与转移、下拉框模型变化、教学提示内容所有权和目标销毁。Windows/macOS 仅检查平台分支，原生编译和测试交由 CI；本次不在本机交叉编译，也不持续监控 CI。

本次整合完成后的验证结果：Linux Debug、Release、静态库各 10 组 QtTest 全部通过；动态库与静态库安装消费通过；示例启动退出、格式检查和 22 项发布辅助脚本测试通过。标签页生命周期回归在修复前有 8 个失败场景，修复后通过，并完成独立复审。Windows/macOS 的本次 CI 结果尚未确认。
