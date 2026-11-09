# 页面状态与数据视图组合

[返回首页](../README.md) · [接口](../ElaWidgetTools/ElaStateWidget.h) · [组合示例](../ElaWidgetToolsExample/ExamplePage/T_DataView.cpp)

`ElaStateWidget` 在内容、加载中、空内容、失败四种状态之间切换。可以放入 `ElaListView`、`ElaTableView`、`ElaTreeView`，也可以放入任意自定义 QWidget。状态切换隐藏原内容，不重建视图，模型和选择会保留。

## 创建与切换

```cpp
auto* state = new ElaStateWidget(parent);
auto* table = new ElaTableView; // 交给容器接管
state->setContentWidget(table);
state->setStateText(ElaStateWidget::State::Loading, "正在加载记录", "请稍候…");
state->setStateText(ElaStateWidget::State::Empty, "没有匹配结果", "请调整筛选条件。");
state->setStateText(ElaStateWidget::State::Error, "加载失败", "请检查连接后重试。");
state->setState(ElaStateWidget::State::Loading);

// 数据加载完成并更新模型后，由应用选择显示状态。
state->setState(model->rowCount() ? ElaStateWidget::State::Content
                                 : ElaStateWidget::State::Empty);
```

| 状态 | 展示 |
| --- | --- |
| `Content` | 原内容控件；未设置内容时为空白区域 |
| `Loading` | 进度环、标题、描述；容器不可见时停止动画 |
| `Empty` | 空内容标题、描述 |
| `Error` | 失败标题、描述，以及可选的重试按钮 |

默认状态为 `Empty`。设置内容不会自动切换状态；重复设置同一状态不重复发送 `stateChanged(State)`，非法枚举值被忽略。标题与描述是纯文本，支持换行；空文本隐藏对应标签。`Content` 不显示提示文本。

容器不推断模型状态、不发起请求，也不自动清空数据。加载中或失败时仍保留内容对象，应用可以保留旧数据，在请求成功后再更新。

## 重试与异步请求

```cpp
connect(state, &ElaStateWidget::retryRequested, controller, &Controller::reload);
// 没有重试业务时可隐藏按钮。
state->setRetryEnabled(false);
```

点击重试只发出 `retryRequested()`，不会自动切换到 Loading。应用决定是否开始请求、更新模型，以及何时结束加载。按钮支持 Tab 焦点和空格操作。

所有控件接口在 GUI 线程调用。工作线程结果应通过带接收上下文的连接回到 GUI 线程；计时器回调使用 `QTimer::singleShot(delay, receiver, callback)`，接收对象销毁后回调自动取消。连续加载时由应用使用请求编号丢弃过期结果，完整做法见 Example。

`stateChanged` 和 `retryRequested` 的槽可以切换状态或销毁容器。内部布局、显示与隐藏事件处理中拒绝重入修改；不要在内容控件事件中递归更换内容。

## 内容所有权

- `setContentWidget(widget)` 成功返回 `true` 并接管所有权。接受无父控件，或直接以当前容器为父对象的新控件。
- 重复设置当前内容成功且不销毁它。拒绝空指针、容器本身、祖先、内部展示控件和其他容器拥有的控件。
- 替换时隐藏旧内容并调用 `deleteLater()`；不再使用旧指针。
- `takeContentWidget()` 移出并隐藏当前内容，清除父对象，将所有权交回调用者；没有内容时返回 `nullptr`。
- 取走或外部销毁内容时，Content 状态退回 Empty；其他状态保持不变。
- 容器销毁时销毁仍持有的内容。`getContentWidget()` 返回借用指针，不转移所有权。

需要复用内容时，先取出再交给新的容器：

```cpp
QWidget* content = oldState->takeContentWidget();
if (content && !newState->setContentWidget(content))
    content->deleteLater(); // 未接管时由调用者处置
```

## Example 入口

侧边导航 **ElaView → 数据加载与筛选**：

1. 首次进入展示加载状态，随后显示示例记录。
2. 输入名称、分类或状态筛选；无匹配时显示空状态，上方输入框仍可操作。
3. 点击列标题排序；点击“加载空数据”查看真正的空数据状态。
4. 点击“模拟失败”，再用“重试”恢复；筛选条件保持。
5. 快速连续点击加载操作，只有最后一次模拟请求能够更新页面。

页面使用 Qt 标准模型与排序过滤代理。加载和错误状态优先于筛选结果：请求尚未完成或失败时，修改筛选条件不会提前显示成功或空列表。
