# QtTest 回归测试

[返回首页](../README.md) · [编译指南](build.md) · [CI](ci.md)

测试作为独立可执行文件构建，默认关闭；`dataview` 复用 Example 的组合页面源码，无需启动完整主窗口。只使用组件库时不需要安装 Qt Test；开启测试才会查找同一 SDK 的 `Qt6::Test`，测试依赖不会写入组件库的安装导出。

## 配置与运行

Linux 使用仓库预设：

```bash
export QT_ROOT="$HOME/Qt/6.12.0/gcc_64"
cmake --preset linux-debug -DELAWIDGETTOOLS_BUILD_TESTS=ON
cmake --build --preset linux-debug --parallel 6
xvfb-run -a ctest --preset linux-debug
```

已有桌面会话时可以直接运行 `ctest --preset linux-debug`。无显示服务器时需安装 Xvfb，不将 `offscreen` 的结果当作桌面窗口行为验证。

Release 或静态库分别换用 `linux-release`、`linux-static`。Windows 在 MSVC 开发者终端执行 `windows-release` 预设；macOS 使用 `macos-release`，两者直接运行对应的 `ctest --preset ...`。

本机用户预设也可开启测试：

```bash
cmake --preset local-linux-debug -DELAWIDGETTOOLS_BUILD_TESTS=ON
cmake --build --preset local-linux-debug
xvfb-run -a ctest --test-dir build/local-linux-debug --output-on-failure --no-tests=error
```

Test presets 设置了失败输出、超时和“没有测试时报错”。单独运行 `ctest` 时也应使用 `--no-tests=error`，避免配置错误却显示成功。测试开关保存在 CMake 缓存中；恢复普通构建可显式传入 `-DELAWIDGETTOOLS_BUILD_TESTS=OFF`。

## 当前覆盖范围

| 测试组 | 观察的行为 |
| --- | --- |
| `appbar` | 三个区域的索引与越界处理、控件替换和非法复用、标题同步与居中、菜单折叠后恢复、外部最大化状态同步、拦截关闭、Linux 置顶功能移除 |
| `controls` | 开关与切换按钮的信号次数、鼠标与键盘、焦点遍历、自动重复、中断取消、拖动、回调销毁和主题切换 |
| `theme` | 系统主题通知、手动优先级、重复设置、配色与恢复通知、非法输入、局部覆盖、已打开控件与菜单刷新 |
| `navigation` | 延迟创建、失败重试、离开确认与历史拒绝、页面删除、工厂所有权、重入与窗口销毁、命令撤回与重做 |
| `suggest` | 防抖、乱序结果、取消／清空／隐藏、模式切换、键盘高亮和选择、回调重入、带焦点销毁 |
| `accessibility` | 控件角色、名称与描述、选中状态、状态通知、切换／按下／焦点动作、禁用与隐藏限制、Qt 原生接口兼容 |
| `statewidget` | 四状态切换、内容保留与所有权、非法复用、重试键盘操作、通知重入与销毁、可见性动画 |
| `dataview` | 实际示例的加载、筛选无结果、失败重试、过期请求丢弃和等待时销毁 |
| `resources` | 内置字体和图像链接、图标实际绘制、非 Windows 平台保留应用默认字体 |

测试使用 `QSignalSpy`、QtTest 输入事件和 `QTRY_*` 等待条件成立，不依赖参考截图逐像素比对或固定休眠。配色测试检查菜单实际背景颜色、图标按钮动画透明度，并观察控件重绘事件；系统主题测试模拟 Qt 通知，验证库的响应逻辑，不代表真实桌面已发送该通知。标题布局允许整数像素舍入差异。窗口状态测试由 Qt API 触发，无法代替真实窗口管理器操作、多显示器或原生全屏验证。

## 安装后的独立消费验证

`tests/consumer/` 是单独的 CMake 工程，只通过安装的 `Ela::WidgetTools` 目标引用组件库，不引用源码目录，也不依赖 QtTest。它验证资源、图标、主题、选项卡绘制和正常退出。

```bash
cmake --install build/linux-release
cmake -S tests/consumer -B build/consumer -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$PWD/install/linux-release;$QT_ROOT"
cmake --build build/consumer
xvfb-run -a ctest --test-dir build/consumer --output-on-failure --no-tests=error
```

静态库使用独立的安装前缀和消费构建目录，避免缓存混用。Windows / macOS 的原生运行不需要 Xvfb。

## 后续增加测试

优先测试可观察行为和已修复的问题：公共接口、状态、信号、所有权及布局边界。新回归测试应先确认能捕获原问题。不要只重复 getter/setter 的实现，也不要为测试增加产品公开接口。

当前九组测试是回归基线，不代表所有组件和平台已经完整覆盖。
