# 页面状态容器实现计划

> 使用 executing-plans 按任务实现，在合并前执行独立代码审查。

**目标：** 提供可复用的页面状态展示，并演示与现有表格的筛选、排序和请求状态组合。

**架构：** ElaStateWidget 只管理内容所有权与状态展示；应用持有数据模型与请求。复用现有主题、进度环和按钮。

**技术栈：** C++17、Qt 6.12.0 及以上、QtTest、CMake Presets、Ninja。

## 任务 1：容器与回归

文件：新增 `ElaWidgetTools/ElaStateWidget.h/.cpp`、`ElaWidgetTools/private/ElaStateWidgetPrivate.h`、`tests/tst_statewidget.cpp`；修改 `tests/CMakeLists.txt`。

- [x] 先写行为回归。核心用例：
  ```cpp
  ElaStateWidget state;
  auto* content = new QLineEdit;
  QVERIFY(state.setContentWidget(content));
  state.setState(ElaStateWidget::State::Content);
  state.show();
  QVERIFY(content->isVisible());
  state.setState(ElaStateWidget::State::Loading);
  QVERIFY(!content->isVisible());
  state.setState(ElaStateWidget::State::Content);
  QCOMPARE(state.getContentWidget(), content);
  ```
  另覆盖规范中的所有权、信号、键盘及动画行为。
- [x] `cmake --build --preset linux-debug --parallel 6`，确认新增测试因缺少接口而失败。
- [x] 实现四状态堆栈、内容接管／取回、文本、重试和可见性动画控制；使用 QPointer 跟踪内容和回调生命周期。
- [x] `xvfb-run -a build/linux-debug/tests/tst_statewidget`，预期全部通过。

## 任务 2：表格组合示例

文件：新增 `ElaWidgetToolsExample/ExamplePage/T_DataView.h/.cpp`、`tests/tst_dataview.cpp`；修改 `ElaWidgetToolsExample/mainwindow.cpp` 和 `tests/CMakeLists.txt`。实际示例参与独立回归，验证错误状态和请求过期规则。

- [x] QtTest 增加真实 ElaTableView + QStandardItemModel + QSortFilterProxyModel 组合回归，验证过滤为空与恢复、排序及状态切换不重建视图。
- [x] 新页面提供名称搜索、加载数据、空数据、模拟失败与重试操作，列为名称／分类／状态。由代理 rowCount 更新结果文本和 Content/Empty 状态；请求等待和错误优先于行数。
- [x] 使用递增编号和带页面接收上下文的 singleShot：
  ```cpp
  const auto request = ++_requestId;
  QTimer::singleShot(600, this, [this, request] {
      if (request != _requestId) return;
      // 当前请求完成后填充模型，再根据代理行数切换状态。
  });
  ```
- [x] 注册页面到 ElaView 分组；构建并运行 Example 启动检查。

## 任务 3：文档、验证与提交

文件：新增 `docs/page-state.md`；更新 `README.md`、`docs/components.md`、`docs/testing.md`、`docs/roadmap.md`。

- [x] 记录状态、内容所有权、GUI 线程、信号和请求责任边界，提供代码用例及 Example 入口。
- [x] 固定版本格式检查、`git diff --check`、Linux Debug/Release 构建、全部 QtTest 和 Example 启动检查。
- [x] 独立代码审查，修复阻塞问题并执行相关回归。
- [x] 按容器、示例、文档分类中文提交，快进 main、推送 origin，确认 CI 启动。

## 验证记录（2026-11-09）

- 原六组 Linux Debug 基线测试通过；新容器测试先确认缺少接口的构建失败。
- Linux / Qt 6.12.0，Debug 和 Release 的 Ninja 预设构建通过；八组 QtTest 全部通过，两种配置 Example 启动检查正常退出。
- 状态容器覆盖 12 个行为用例，含独立审查发现的隐藏回调销毁候选控件、状态切换时焦点回调删除内容；两项回归先失败后通过，原始探针也复查通过。
- 组合页面验证加载、无结果、空数据、错误、重试、筛选保留、过期请求与等待时销毁。独立 ElaWindow 中检查浅色内容与深色错误展示。
- 固定版本 clang-format / cmake-format 检查和 `git diff --check` 通过。
- 本轮 Windows / macOS 编译及运行交给推送后的 CI；本地未跨平台编译。
