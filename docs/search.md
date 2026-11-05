# 本地与异步搜索

[返回首页](../README.md) · [导航示例](../ElaWidgetToolsExample/ExamplePage/T_Navigation.cpp) · [接口](../ElaWidgetTools/ElaSuggestBox.h)

`ElaSuggestBox` 默认继续在 `addSuggestion()` 添加的建议中做本地子串匹配。异步模式是可选功能，适用于数据库、网络或其他外部查询来源。

## 本地模式

```cpp
search->addSuggestion("主题设置", {{"page", "settings"}});
search->setSearchText("主题");
```

`setSearchText()` 与实际输入一样触发搜索，`getSearchText()` 返回当前文本。建议的添加、移除和清空会同步更新本地结果；负索引移除被忽略。

结果列表支持上下键选择、Enter 确认、Escape 取消。选择结果时发送 `suggestionClicked(SuggestData)`，并将建议文本填入输入框。

## 异步模式

```cpp
search->setAsyncSearchEnabled(true);
search->setSearchDebounceInterval(200); // 毫秒；默认 250，0 表示不延迟

connect(search, &ElaSuggestBox::searchRequested,
        service, &SearchService::startSearch);

connect(service, &SearchService::resultsReady, search,
        [search](quint64 requestId, const QList<ElaSuggestBox::SuggestData>& results) {
            search->setSearchResults(requestId, results);
        });

connect(search, &ElaSuggestBox::searchCancelled,
        service, &SearchService::cancelSearch);
```

示例中的 `SearchService` 代表应用自己的查询服务，并非库内置类型。服务自行管理网络、数据库或工作线程。将返回结果的连接上下文设为搜索控件，以便回调在 GUI 线程执行，并在控件销毁后自动断开。

| 接口／通知 | 含义 |
| --- | --- |
| `searchRequested(id, text)` | 防抖结束，可以执行该次查询 |
| `setSearchResults(id, results)` | 返回 `true` 表示采纳；旧编号、已取消请求及重复提交返回 `false` |
| `searchCancelled(id)` | 已派发的查询失效，服务可据此停止外部任务 |
| `searchPendingChanged(bool)` | 是否仍在防抖或等待结果 |
| `getIsSearchPending()` | 查询当前等待状态 |
| `cancelSearch()` | 取消等待并收起结果，保留输入文本 |

请求编号在同一个控件实例内递增，`0` 无效。新文本、清空输入、Escape、隐藏控件或关闭异步模式都会让旧请求失效；迟到的结果不会重新打开列表或覆盖新结果。

取消表示**结果不再被接受**，不会强制终止应用的线程或网络请求。尚未派发的防抖请求不会发送 `searchCancelled`。完成查询提交空列表即可表示没有结果；错误提示由应用决定，可配合自己的状态文本显示。

异步模式不再过滤本地建议，也不会对服务结果进行二次子串匹配；结果的顺序、匹配规则和业务数据由服务决定。`SuggestData` 可携带自定义 key 和 `QVariantMap`，未指定 key 时自动生成。关闭异步模式会恢复本地匹配。

所有控件方法在 GUI 线程调用。不要在后台线程直接操作 QWidget；需要自行保存控件引用时，使用 `QPointer` 检查存活，或采用以上带接收上下文的连接。

## Example

导航组件页提供异步搜索演示，使用 `QTimer` 模拟不同查询耗时。连续输入可以观察防抖、查询取消及旧结果丢弃，不依赖网络。主窗口顶部的页面搜索仍使用原有本地建议模式。
