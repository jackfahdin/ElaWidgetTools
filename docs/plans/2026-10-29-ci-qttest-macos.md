# CI、QtTest 与 macOS 实施计划

目标：保留 Qt 6.12.0 及以上的维护范围，用同一套 CMake Presets 完成跨平台构建和回归检查，并建立 macOS 的实验性支持入口。

## 第一阶段：构建与 CI

- [x] 将 `CMakePresets.json` 的公共 Ninja 配置抽出，保留 Linux 预设，新增 Windows / macOS Debug、Release 及 Linux 静态库预设。
- [x] 新增 `.github/workflows/ci.yml`，在 PR、main 推送和手动触发时构建 Linux Debug / Release、Linux 静态库、Windows Release、macOS Release。
- [x] CI 固定 Qt 6.12.0，缓存 SDK；仅下载需要的 Qt 模块，保留失败日志和测试结果。项目的 `find_package` 仍表示最低版本。
- [x] 新增独立格式检查，复用 `tools/requirements-code-tools.txt` 中的版本；clang-tidy 单独手动触发，输出报告。

## 第二阶段：QtTest

- [x] 顶层新增默认关闭的 `ELAWIDGETTOOLS_BUILD_TESTS`，开启后查找 `Qt6::Test` 并注册 CTest；不向消费项目传递测试依赖。
- [x] 新增 `tests/CMakeLists.txt` 和按行为划分的 QtTest 文件：标题栏、自定义区和菜单布局，开关状态及信号，导航与命令历史，字体及图像资源。
- [x] Linux 置顶接口不生效和按钮标志过滤纳入平台回归；窗口事件覆盖关闭拦截和外部最大化状态同步；原生最小化、全屏与 Spaces 列入设备验证清单。
- [x] 使用 `QSignalSpy`、QtTest 鼠标事件和 `QTRY_*`；不依赖像素截图或固定休眠。Linux 用 Xvfb，Windows / macOS 使用原生 Qt 平台插件。
- [x] 新增安装后消费项目 `tests/consumer/`，分别验证动态库和静态库的导出目标、资源初始化和正常退出。
- [x] 接入 test presets；配置不存在或没有测试时必须报错，避免空跑成功。

## 第三阶段：macOS

- [x] 审查 Windows / Linux / macOS 条件分支，移除含 `Q_OS_WIM` 拼写错误的手动图标更新分支，统一从窗口状态通知同步；已有窗口行为先用测试约束。
- [x] 菜单栏作为标题栏控件使用时，在 macOS 显式关闭原生菜单迁移；不强制影响消费者自己的系统菜单。
- [x] 完善 `.app` 的库加载路径、安装和 Qt 部署。CI 对部署后应用执行有限时的启动检查并保留截图或日志所能提供的证据。
- [x] 更新 `README.md`、`docs/build.md`、`docs/roadmap.md`，新增 `docs/testing.md`、`docs/ci.md`、`docs/macos.md`。区分本机已验证、CI 配置已准备和云端实际结果。

## 验证与交付

- [x] 本机使用 Qt 6.12.0 完成 Debug / Release 编译及全部 QtTest。
- [x] 完成 Linux 静态库构建与安装消费验证，以及测试默认关闭的普通构建检查。
- [x] 对新增源码和 CMake 文件运行固定版本格式检查；使用 actionlint 验证 workflow。
- [x] 检查跨平台依赖、DLL 搜索路径、macOS RPATH 和部署脚本；审查测试的独立性与覆盖边界。
- [x] 明确区分本机证据与云端待验证结果，记录 macOS 原生桌面人工验证清单。
- [ ] 推送后实际执行 Windows / macOS CI，根据输出修复云端问题并更新支持状态。

实施在当前干净工作区进行；改动按 CI、测试、平台适配组织，准备完成后交由用户决定提交与推送。


## 本机验证记录

2026-10-29，Linux / Qt 6.12.0：

- Debug、Release 动态库及 Example、Linux 静态库及 Example 均编译通过。
- 每种配置的 4 组 QtTest 均通过；Debug、Release 和静态安装包的独立消费项目均通过。
- 三种配置的 Example 使用 `--smoke-test` 在 Xvfb 中进入事件循环并正常退出。
- 默认关闭测试时不查找 Qt Test；CTest 空目录检查按预期失败。
- 全仓固定版本格式检查、差异空白检查及 actionlint 1.7.12 校验通过。
- clang-tidy 21.1.6 配置校验及 `ElaAppBar.cpp`、`ElaMenuBar.cpp`、`ElaApplication.cpp` 分析完成，未报告项目内告警；依赖头文件的 372 条告警被过滤，不代表全仓静态分析通过。
- 首轮测试捕获菜单无法收缩与外部最大化图标未同步的问题；修复后转为通过。非 Windows 默认字体也完成先失败、后通过的回归验证。
- macOS 部署检查经只读审查后改为复制整个应用至临时目录再启动，并拒绝非系统绝对 RPATH；尚未在 macOS 执行。

Windows / macOS 的云端构建、原生测试和部署启动结果仍等待提交推送后的 Actions；以上本机结果不替代这些验证。
