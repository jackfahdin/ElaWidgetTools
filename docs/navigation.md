# 页面延迟创建与离开确认

[返回首页](../README.md) · [导航示例](../ElaWidgetToolsExample/ExamplePage/T_Navigation.cpp) · [懒加载编辑器](../ElaWidgetToolsExample/ExamplePage/T_PageLifecycle.cpp)

现有 `addPageNode()` 继续接收已经创建的 QWidget。对于初始化较重、未必会访问的页面，可使用 `addLazyPageNode()` 注册工厂。

## 首次访问时创建

```cpp
QString editorKey;
window->addLazyPageNode("编辑器", editorKey,
    [](QWidget* parent) -> QWidget* {
        return new MyEditorPage(parent);
    }, ElaIconType::Pen);

window->navigation(editorKey); // 第一次调用工厂，后续访问复用页面
QWidget* editor = window->getNavigationPage(editorKey);
```

分组节点使用带 `targetExpanderKey` 的重载：

```cpp
window->addLazyPageNode("编辑器", editorKey, factory, groupKey, ElaIconType::Pen);
```

- 注册时只添加轻量占位页，不执行工厂。首个页面如果是懒页面，会在窗口显示时创建。
- 工厂接收页面父控件，在 **GUI 线程**返回一个新 QWidget；创建失败返回 `nullptr`。成功后由窗口接管所有权。
- `getNavigationPage(key)` 在尚未创建、已移除或 key 无效时返回 `nullptr`。
- 成功发出 `navigationPageCreated(key, page)`；失败发出 `navigationPageCreationFailed(key)`，保持原页，再次导航时可以重试。
- `removeNavigationNode()` 清理对应工厂；不会因为后来访问旧 key 重新创建已删除节点。
- `backtrackNavigationNode()` 和“在新窗口打开”会再次调用工厂。工厂必须每次返回新控件，不能把缓存的已注册页面移给另一个窗口。

需要保留重型数据时，可让应用的数据对象独立于 QWidget；工厂只负责组装页面。不要在工厂中阻塞 GUI 线程执行网络或长时间数据库查询。

## 离开前确认

```cpp
window->setNavigationGuard([window](const QString& from, const QString& to) {
    auto* editor = qobject_cast<MyEditorPage*>(window->getNavigationPage(from));
    return !editor || !editor->hasUnsavedChanges() || editor->confirmLeave();
});

window->setNavigationGuard({}); // 移除守卫
```

守卫返回 `false` 时，不创建目标懒页面、不切换导航选中项，也不新增历史记录。通过标题栏前进／后退调用守卫时，拒绝操作同样不会推进历史位置；之后可以重新尝试。

同一导航过程中的重入请求会被拒绝。守卫可以弹出确认对话框，但不应从回调里再次导航。执行撤销／重做期间，清空或改写正在执行的命令域会被忽略，避免删除正在使用的命令；如需清空，应在导航完成后执行。

守卫只约束**导航页面之间的切换**。分类标题、没有页面的页脚动作、窗口关闭、显式删除／重建页面以及额外中央堆栈不属于该拦截范围。应用退出确认仍应由应用处理。

当前保留 `ElaWidgetToolsAction` 全局命令域，多个窗口仍共用既有命令历史。`ElaActionCommand` 新增默认成功的 `tryUndo()` / `tryRedo()` 扩展；原有 `undo()` / `redo()` 子类无需改写，需报告失败的命令可以覆盖这两个方法。

## Example

侧边导航的“懒加载编辑器”首次打开时才创建。输入文本后切换其他页面或后退，会弹出确认；选择“继续编辑”保留当前页，选择“仍然离开”允许跳转。返回页面后文本仍然保留。

“标记为已保存”只演示状态变更，不写入磁盘。
