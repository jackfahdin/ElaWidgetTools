# macOS 实验性支持

[返回首页](../README.md) · [编译指南](build.md) · [CI](ci.md)

本分支已准备 macOS 的 Ninja 预设、窗口状态同步、标题栏菜单处理和 `.app` 部署。开发主机为 Linux，没有本地 Mac 设备；当前状态是**实验性适配，等待 macOS CI 实际验证**，不声明完整桌面兼容性。

## 构建

使用 Qt 6.12.0 及以上的 macOS SDK、匹配的 Xcode / Command Line Tools、CMake 3.25 及以上和 Ninja。当前预设以 macOS 14.4 为部署下限，后续 Qt SDK 如果要求更高系统版本，应相应调整。SDK 与目标 CPU 架构必须匹配。

```bash
export QT_ROOT="$HOME/Qt/6.12.0/macos"
cmake --preset macos-release -DELAWIDGETTOOLS_BUILD_TESTS=ON
cmake --build --preset macos-release --parallel 3
ctest --preset macos-release
cmake --install build/macos-release
```

`QT_ROOT` 指向包含 `bin`、`lib` 的实际目录；不同安装器的目录名称可能不同。调试构建换用 `macos-debug`。现有 CI 使用 `macos-15` Apple Silicon runner，尚未安排 Intel 或 Universal 应用验证。

## 窗口与菜单

- 保留项目的自定义标题栏布局；不引入原生交通灯按钮或 Objective-C 窗口扩展。
- 通过 Qt 的窗口状态通知同步最大化 / 还原图标，包含窗口系统触发的状态变化。
- 示例中的 `ElaMenuBar` 设置 `setNativeMenuBar(false)`，防止自定义区菜单被转移至屏幕顶部。库仍允许其他使用者自行选择原生菜单。
- 非 Windows 平台保留应用已有字体族，不强制要求 Windows 字体；字体大小和图标字体沿用组件库设置。
- DXGI 和 Windows 原生窗口材质不属于 macOS 功能，不注册 DXGI 示例页。

Qt Cocoa 提供系统窗口移动接口，但没有实现 `startSystemResize()`；边缘缩放最终还涉及 AppKit 的窗口处理，须在真实桌面继续验证。CI 中的 Qt API 状态测试不能证明手动拖动和缩放体验正常。

## 应用部署

安装结果包括组件库的 CMake 开发包和根目录下的 `ElaWidgetToolsExample.app`。安装脚本使用 Qt 部署工具收集应用所需的动态库与插件，`.app` 使用相对路径查找内部依赖。

CI 运行 `tools/check-macos-bundle.py`，检查 Mach-O 文件不再链接 SDK 目录中的绝对库路径或搜索路径，再清理 Qt / QML / DYLD 环境变量，将整个 `.app` 复制至独立临时目录后以 `--smoke-test` 启动，防止借用原安装前缀中的库。示例须进入事件循环并正常退出；超时和崩溃会导致检查失败。

此检查验证部署后的基础启动，不验证 Finder、Gatekeeper 或签名公证流程。CI 上传的应用仅供验证，不属于正式发行包。

## 仍需真实设备确认

| 场景 | 检查内容 |
| --- | --- |
| 标题栏与窗口 | 拖动、边缘缩放、双击、最小化与恢复、系统全屏 / Spaces |
| 输入与焦点 | 菜单、对话框、弹出层、输入法、Command 快捷键 |
| 显示 | Retina、多显示器切换、缩放、主题和字体回退 |
| 部署 | Finder 启动、移动应用目录、目标系统版本、签名与公证 |

获得 CI 或设备验证证据后再更新支持状态；静态检查和 Linux 测试不计为 macOS 运行验证。
