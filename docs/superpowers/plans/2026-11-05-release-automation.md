# 自动发布实现计划

> 执行方式：使用 executing-plans 在当前会话逐项实现；以测试、静态检查和实际发布结果作为检查点。

**目标：** 稳定标签自动发布，每日北京时间 00:00 更新有变更的 Continuous Build。

**架构：** 三个平台各有独立工作流和打包脚本，只共享版本、摘要和发布元数据约定。编排工作流固定源 SHA，所有平台通过测试后才公开 Release。

**技术栈：** GitHub Actions、Python 3.14 标准库 unittest、CMake Presets、Ninja、gh CLI。

## 任务 1：版本与平台打包

文件：`tools/release/common.py`、`package_linux.py`、`package_windows.py`、`package_macos.py` 及 `tests/test_common.py`、`test_package_linux.py`、`test_package_windows.py`、`test_package_macos.py`。

- [x] 先写测试，覆盖 `project_version(source)`、`validate_identity(version, label, sha)`，使用 `2.0.0`、错误的 `v2.0`、`../bad` 和非 SHA 输入。
- [x] 测试三个 `package(stage, output, label, sha)` 接口：解压实际测试归档，校验必需文件、文件摘要、Linux 执行权限和 macOS framework 符号链接；缺少或为空的库、Example、头文件、CMake 配置必须失败。
- [x] 执行 `python3 -m unittest discover -s tools/release/tests -v`，确认缺少实现导致失败。
- [x] 实现平台打包及公共元数据；平台输出一个压缩包和 `package-info-<平台>.json`，记录版本、Qt 版本、平台、SHA、文件大小和摘要。Windows 拒绝符号链接，Linux 和 macOS 保留安装树内相对链接。
- [x] 重跑测试，确认归档可读取且不会包含工作区绝对路径、越界链接或递归输出目录。

## 任务 2：准备、汇总与发布

文件：`tools/release/prepare.py`、`assemble.py`、`publish.py`、`tests/test_release.py`。

- [x] 先用假的 GitHub API 和真实临时资产目录测试：版本标签匹配、无变更跳过、手动强制重建、错误源提交、缺平台、内容被修改、稳定版拒绝覆盖、移动标签拒绝发布、失败保留草稿和可重试。
- [x] 执行 `python3 -m unittest discover -s tools/release/tests -v` 并确认新增用例失败。
- [x] `prepare.py` 解析源版本及 HEAD；稳定版要求 `vX.Y.Z`；nightly 只允许 main。输出 mode、sha、version、label、tag、changed 到 GITHUB_OUTPUT。
- [x] `assemble.py` 验证恰好三个包及其元数据，生成 `release-info-<标签>.json` 和 `SHA256SUMS-<标签>.txt`；发布说明为中文，包含依赖和平台限制。
- [x] `publish.py` 通过 gh API 先检查标签与 Release，再创建或恢复草稿；上传并验证远端摘要后更新滚动标签并公开。稳定标签永不移动，公开稳定版本永不覆盖；滚动任务串行执行且不取消正在发布的任务。
- [x] 重跑全部脚本测试，包含网络中断后保留草稿的故障注入。

## 任务 3：独立平台工作流

文件：`.github/workflows/release.yml`、`release-linux.yml`、`release-windows.yml`、`release-macos.yml`；在 `ci.yml` 的格式任务加入发布脚本测试。

- [x] 每个平台使用 workflow_call 输入 source_sha、release_label、release_mode，检出精确提交，以对应 Release preset 配置、编译、QtTest、安装、消费验证和打包。
- [x] Linux 用 Ubuntu 24.04、Qt 6.12.0、Bash 和 tar.gz，发布为需要匹配 Qt SDK 的开发安装包。
- [x] Windows 用 Windows 2022、MSVC x64、PowerShell，逐条检查外部命令退出码，保留已验证的 Qt 安装器修复。
- [x] macOS 用 macos-15 ARM64、Bash 和 ditto，解压最终包后执行现有 check-macos-bundle.py；平台声明以 runner 架构断言为准。
- [x] release.yml 在准备阶段执行格式检查和脚本测试，cron 使用 `0 16 * * *`，稳定版由 `v*` 标签触发，手动预览只允许 main。下载并验证本次运行的三个平台产物后发布。
- [x] actionlint 检查所有 workflow；执行 unittest、py_compile、全仓格式检查和 git diff --check。本机不交叉编译其他平台。

## 任务 4：文档、提交与真实验证

文件：`docs/releases.md`、`README.md`、`docs/ci.md`、`docs/roadmap.md`、本计划及规格中的实现细节。

- [x] 文档明确预编译包使用 Qt 6.12.0、库依赖 Qt 私有 ABI；支持 Qt 6.12+ 表示可重建源码。写清 Linux 环境依赖、Windows 部署包和 macOS 未签名/未公证边界。
- [x] 检查本地 Markdown 链接、归档必要内容及许可文本；本机仅做 Linux 原生打包验证和跨平台静态检查。
- [x] 分类中文提交，快进 main 并推送；手动触发 `gh workflow run release.yml --ref main`。
- [x] 查询新运行，修复实际失败；成功后核对 Release 的源提交、三个资产及 SHA256SUMS。记录运行链接，不创建稳定版标签。

## 本地验证记录

- Python 发布脚本 22 项测试通过；旧任务回退用例已先失败、后通过。
- actionlint 1.7.12、固定版本格式检查、Python 编译检查及文档链接检查通过。
- Linux Qt 6.12.0 Release 构建和 4 组 QtTest 通过。
- 本机原生 Linux 安装树经过 RPATH 清理、打包、解压后，消费项目构建/测试与 Example `--smoke-test` 均通过。
- Windows 和 macOS 没有在本机交叉编译；平台实际打包和启动由发布 CI 验证。

## 云端验证记录

- 首次发布运行发现 Windows 插件检查路径与 Qt CMake 部署布局不一致；按部署日志将检查改为 `plugins/platforms/qwindows.dll`，同时要求 `bin/qt.conf` 存在，提交 `91c7811`。
- [自动发布运行 41984031758](https://github.com/jackfahdin/ElaWidgetTools/actions/runs/41984031758) 全部通过：准备、Linux x86_64、Windows x64、macOS ARM64 和最终发布。
- [常规 CI 运行 41984031156](https://github.com/jackfahdin/ElaWidgetTools/actions/runs/41984031156) 全部通过。
- [Continuous Build](https://github.com/jackfahdin/ElaWidgetTools/releases/tag/continuous-build) 已公开，首包源提交为 `91c781170073cb990037e5ef0462ebbc4d9d29bb`，包含三平台归档、构建元数据和 SHA256SUMS。
- 发布后独立读取 GitHub 资产摘要并下载元数据/校验清单，验证三个包的文件大小、摘要和源提交全部一致。
- 在相同源提交运行准备脚本，真实返回 `changed=false`，确认每日无变更跳过；没有创建稳定标签或稳定 Release。
