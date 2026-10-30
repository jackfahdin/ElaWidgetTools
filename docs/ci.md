# 持续集成

[返回首页](../README.md) · [测试指南](testing.md) · [macOS](macos.md)

GitHub Actions 工作流位于 [ci.yml](../.github/workflows/ci.yml)。推送到 `main`、创建或更新 PR，以及 Actions 页面手动触发时运行。PR 使用普通 `pull_request`，检查任务只申请仓库读取权限。

## 构建矩阵

| 环境 | 预设 | 内容 |
| --- | --- | --- |
| Ubuntu 24.04 x86_64 | `linux-debug` | 动态库、Example、QtTest、安装消费 |
| Ubuntu 24.04 x86_64 | `linux-release` | 动态库、Example、QtTest、安装消费 |
| Ubuntu 24.04 x86_64 | `linux-static` | 静态组件库、Example、QtTest、安装消费 |
| Windows Server 2022 / MSVC 2022 x64 | `windows-release` | 动态库、Example、QtTest、安装部署、安装消费 |
| macOS 15 / Apple Silicon | `macos-release` | 动态库、Example、QtTest、安装消费、部署后 `.app` 启动检查 |

CI 固定 Qt **6.12.0** 作为当前验证基线，构建工具统一为 Ninja。SDK 从官方源通过 `jurplel/install-qt-action` 安装并缓存，安装 `qtbase` 和 `qttools`，Linux 额外安装官方 SDK 的 `icu` 包；Qt Test 和私有头文件随匹配版本的 SDK 提供。组件库静态构建不表示 Qt 本身静态链接。

Linux 的 Qt 工具依赖 ICU 73，Ubuntu 24.04 自带的 ICU 版本不能替代，因此构建和静态分析任务都保留 `icu` 下载包。Windows 的 Qt 6.11 及以上版本使用带 MSVC 架构后缀的仓库子目录，`aqtinstall 3.3.0` 尚未支持；Windows 任务暂时固定使用包含[上游修复](https://github.com/miurahr/aqtinstall/commit/7e5a5c3d95cf962cfc2f36c86ffa0d2c07f1a0d4)的安装器提交，待正式版本包含修复并验证后再切回发布版。

固定 CI 的版本是为了复现结果，项目仍接受 Qt 6.12.0 及以上。引入后续版本时，新增具体版本的矩阵任务并确认私有接口兼容，再更新验证范围。

## 格式和静态分析

格式检查独立于编译任务，使用 `tools/requirements-code-tools.txt` 中固定版本的 clang-format、cmake-format。`tools/check-format.py` 检查组件库、Example、测试源码和 CMake 文件，不修改文件。

```bash
python tools/check-format.py
```

`clang-tidy` 不接入普通构建，也不把全部历史告警设为阻塞条件。[analysis.yml](../.github/workflows/analysis.yml) 可在 Actions 页面手动运行，先构建生成 Qt 头文件，再分析标题栏、菜单栏和应用初始化实现，上传报告。它不等同于全仓静态分析。

## 结果与产物

- 构建失败时先查看失败步骤日志；下载的 `logs-<preset>` 包含 CMake 配置诊断、CTest 日志和可用的 JUnit 结果。
- macOS 任务另外保留启动日志，成功时上传 `.app` 压缩包。应用未经开发者签名和公证，不作为正式发行包。
- 同一分支重复触发时取消旧任务；矩阵中一个平台失败不会取消其他平台，便于同时定位差异。

跨平台改动在本地只做静态检查，各平台的实际编译、测试和部署启动验证交给对应的 CI runner。

本地 actionlint 检查只能验证工作流结构与部分命令问题。Windows、macOS 的实际构建与运行状态以提交推送后的 Actions 结果为准；工作流写好不表示云端任务已经通过。
