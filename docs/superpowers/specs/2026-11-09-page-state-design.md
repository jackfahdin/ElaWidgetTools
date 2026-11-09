# 页面状态容器与数据视图组合

本轮承接路线图中已确认的页面状态与数据视图方向。

## 方案

新增 `ElaStateWidget`，继承 QWidget，使用堆栈切换内容与提示区域。提示区域复用 ElaProgressRing、ElaText、ElaPushButton，跟随现有主题。内容可以是列表、表格、树或任意 QWidget。应用显式决定状态，组件不监听模型、不执行网络请求。

其他选择：给三个视图分别增加接口会重复状态及所有权逻辑；绑定模型自动推断状态无法区分加载中、错误与过滤无结果。本轮采用独立容器。

## 接口与行为

- `State { Content, Loading, Empty, Error }`；默认 Empty。`setState/getState` 切换状态，只有实际变化才发 `stateChanged(State)`，非法枚举被忽略。
- `setContentWidget(QWidget*) -> bool` 接管内容，接受无父控件或直接以容器为父对象的新控件。同一控件重复设置成功且无副作用；拒绝空指针、祖先、内部控件及其他容器所有的控件。替换时旧内容隐藏并延迟销毁。
- `getContentWidget()` 返回当前内容；`takeContentWidget()` 移出并隐藏内容，将所有权交回调用者。取走或外部销毁内容时，Content 状态退回 Empty；其他状态保持。
- 设置内容不会自动改变状态。切换 Loading/Empty/Error 只隐藏内容，保留内容对象、模型和选择。
- `setStateText(State, title, description)` 配置三个提示状态的纯文本，允许为空；Content 无提示文本。标题和描述换行，布局适应可用宽度。
- 错误状态默认显示重试按钮；`setRetryEnabled/getRetryEnabled` 控制其显示。点击仅发 `retryRequested()`，应用负责是否转到 Loading 和发起请求。支持 Tab 和空格键。
- Loading 仅在容器可见时运行动画，隐藏或离开 Loading 停止动画。
- GUI 线程使用；公开通知回调可以再次切换状态或销毁容器。控件 show/hide 等内部布局事件处理中拒绝重入修改。

## Example

独立“数据加载与筛选”页面组合现有 ElaTableView、ElaLineEdit、ElaPushButton 与 Qt 标准模型／排序过滤代理。演示加载、筛选无结果、成功、模拟失败、重试、排序和结果数量。使用有接收上下文的 QTimer 模拟服务，并用请求编号丢弃旧结果。状态容器之外保留筛选工具栏，使空状态下可以清空条件。

## 验证与边界

QtTest 覆盖状态信号、可见性、内容保留、所有权转移、非法复用、内容销毁、键盘重试、信号重入与销毁、隐藏时停止动画，以及表格代理筛选组合。Linux Debug/Release 本机验证，Windows/macOS 交 CI。同步中文使用指南、组件目录和路线图；不改动菜单间距和 v2.0.0 标签。
