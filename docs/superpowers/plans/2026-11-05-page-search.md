# 页面管理与异步搜索实现计划

**目标：** 在已有 ElaWindow / ElaSuggestBox 上补充延迟创建、离开确认和异步搜索，保持已有同步接口可用。

**方案选择：** 页面使用轻量占位控件保留原导航结构，首次访问时调用工厂替换；不引入新的路由框架。搜索采用信号与请求编号连接外部服务，不在组件内固定线程池或网络实现。

## 页面管理

- [x] 新增 `ElaWindow::PageFactory`（`QWidget*(QWidget*)`）、根节点和分组节点的 `addLazyPageNode`，及 `getNavigationPage`。
- [x] 工厂在 GUI 线程首次访问执行，成功后复用；首个懒页面在窗口显示时创建。返回空指针时发 `navigationPageCreationFailed`，不切页，可重试；成功发 `navigationPageCreated`。
- [x] 新增窗口级 `setNavigationGuard`，签名 `bool(const QString& from, const QString& to)`。回调先于工厂、选中状态和历史变化执行；可用对话框确认，空回调取消拦截。重入导航拒绝。
- [x] 在导航栏统一入口调用守卫，覆盖直接调用、点击、菜单以及历史跳转。分类节点和没有绑定页面的页脚动作不触发离页确认。
- [x] 命令增加默认成功的 `tryUndo/tryRedo` 扩展，导航命令返回实际跳转结果，失败不推进历史；批量历史遇到取消停止。
- [x] 懒页面删除时清理工厂；重建与新窗口操作沿用工厂，避免打开空白占位页。页面指针用 QPointer 跟踪。
- [x] 测试：一次创建、失败重试、拒绝后无创建无切页、历史取消及重试、删除节点、空窗口查询、首个懒页面、页脚与分组导航。

## 异步搜索

- [x] 新增可选 `setAsyncSearchEnabled`、`setSearchDebounceInterval`、`setSearchText/getSearchText`、`setSearchResults(requestId, results)`、`cancelSearch`；默认保持本地匹配。
- [x] 新增 `searchRequested(requestId, text)`、`searchCancelled(requestId)`、`searchPendingChanged(bool)`。请求编号单调递增，0 无效；新文本、清空、关闭异步模式、隐藏和 Escape 使旧请求失效。
- [x] 外部服务在 GUI 线程回送结果；旧编号、重复提交和失效请求返回 false。取消是失效通知，不保证强制停止外部服务。
- [x] 维护结果所有权，模型重置前清理旧引用，修复负索引删除与关闭动画覆盖新结果的问题；新增上下键、Enter 选择和 Escape 取消。
- [x] 测试：防抖、乱序返回、清空／取消／隐藏、重复提交、点击和键盘选择、本地模式兼容及删除建议时模型更新。

## 示例与验证

- [x] 导航页演示异步搜索，以 QTimer 模拟可乱序返回的服务并显示请求状态；主窗口增加懒加载编辑页，使用现有对话框展示未保存内容确认。
- [x] 中文文档说明生命周期、所有权、线程约束、导航守卫范围和取消语义。
- [x] 先运行失败回归再实现；执行固定格式检查、Linux Debug/Release CMake Presets + Ninja 构建、全部 QtTest 和示例启动。
- [x] 代码审查后分类中文提交并推送。Windows/macOS 原生构建与运行交给 CI。

另先修复上一轮 macOS Tab 焦点测试参照控件差异，独立提交；不修改平台的系统键盘导航偏好。

## 验证记录

- Linux / Qt 6.12.0：使用 Ninja 和 `linux-debug`、`linux-release` 预设完成构建。
- 两种配置的六组 QtTest 全部通过，Example `--smoke-test` 均正常退出。
- 固定版本 clang-format、cmake-format 检查及 `git diff --check` 通过。
- 页面管理与异步搜索分别完成代码审查，补充工厂所有权、回调销毁、取消重入及历史状态信号重入回归。
- 历史信号重入回归先复现 QList 越界，再验证提前检查执行状态可以阻止嵌套重做。
- Windows / macOS 的本轮编译和运行由推送后的 CI 验证，本地未进行跨平台编译。
