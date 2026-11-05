# 版本下载与自动发布

[项目首页](https://github.com/jackfahdin/ElaWidgetTools) · [编译指南](https://github.com/jackfahdin/ElaWidgetTools/blob/main/docs/build.md) · [全部版本](https://github.com/jackfahdin/ElaWidgetTools/releases)

## 两个发布渠道

| 渠道 | 触发方式 | 下载入口 | 更新规则 |
| --- | --- | --- | --- |
| 稳定版 | 推送与组件库版本一致的 `vX.Y.Z` 标签 | GitHub Releases 中对应版本 | 已公开版本不可覆盖，修复使用新版本 |
| Continuous Build | 每日北京时间 00:00，或手动运行 | [固定预览下载页](https://github.com/jackfahdin/ElaWidgetTools/releases/tag/continuous-build) | 定时任务仅在 main 有新提交时更新；手动任务强制重建 |

定时运行由 GitHub 调度，可能延迟；长期无活动的公共仓库也可能被 GitHub 暂停定时任务。Continuous Build 标记为 Pre-release，不会取代稳定版的 Latest。更新滚动包时会暂时转为草稿，下载页可能短暂不可用；如果上传失败，修复原因后重新运行恢复草稿。

自动发布指 GitHub Release 构建与上传，组件库和 Example 不包含应用内更新器。

## 下载哪个文件

文件名中的 `<标签>` 是稳定版号（如 `2.0.0`）或 `Continuous-Build`。所有平台首版发布 **Release 动态库**，不发布 Debug、静态库、安装器或候选版。

| 文件名 | 系统 / 架构 | 包内内容 |
| --- | --- | --- |
| `ElaWidgetTools-<标签>-linux-x86_64.tar.gz` | Linux x86_64；构建基线 Ubuntu 24.04 | `.so`、公共头文件、CMake 包配置、Example |
| `ElaWidgetTools-<标签>-windows-x86_64.zip` | Windows x64；MSVC 2022 | DLL、导入库、公共头文件、CMake 包配置、部署 Qt 依赖的 Example |
| `ElaWidgetTools-<标签>-macos-arm64.zip` | macOS 14.4+ / Apple Silicon | `.dylib`、公共头文件、CMake 包配置、部署完成的 `.app` |
| `SHA256SUMS-<标签>.txt` | 全平台 | 三个平台包及元数据的 SHA-256 |
| `release-info-<标签>.json` | 全平台 | 版本、Qt 基线、源提交、时间、工作流链接及资产摘要 |

GitHub 额外生成 Source code（zip / tar.gz）。需要静态库、Debug 或其他工具链时下载源码，按编译指南自行构建。Intel Mac、Windows ARM64 和 Linux ARM64 不在首版二进制发布范围。

### Qt 与 ABI

源码维护范围仍是 **Qt 6.12.0 及以上**。当前发布包固定使用 **Qt 6.12.0**，由于组件库依赖 `WidgetsPrivate`，预编译库需要与构建时匹配的 Qt SDK/工具链；不要直接把包内 Qt 替换为其他版本。使用更新 Qt 时重新编译组件库。

开发包不包含完整 Qt SDK，消费项目仍需安装 Qt 开发环境。例如：

```bash
cmake -S MyApp -B MyApp/build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="/path/to/unpacked/ElaWidgetTools-package;$QT_ROOT"
```

### Linux

Linux 首版是开发安装包，**不是携带全部依赖的便携程序**。需要匹配 Qt 6.12.0、兼容的 GCC/libstdc++ 和系统图形依赖；Ubuntu 24.04 CI 的通过结果不代表所有发行版都能直接运行。

```bash
tar -xzf ElaWidgetTools-Continuous-Build-linux-x86_64.tar.gz
export QT_ROOT="$HOME/Qt/6.12.0/gcc_64"
LD_LIBRARY_PATH="$QT_ROOT/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
  ./ElaWidgetTools-Continuous-Build-linux-x86_64/bin/ElaWidgetToolsExample
```

发布脚本移除 CI 主机的绝对 RPATH，Example 通过相对路径寻找同一包内的组件库，Qt 路径由使用者提供。

### Windows

解压整个 ZIP 后启动 `bin/ElaWidgetToolsExample.exe`，保持相邻 DLL 和插件目录完整。包针对 MSVC x64；消费项目需安装对应 Qt SDK，目标机器需满足 Qt 和 Microsoft Visual C++ 运行库要求。不提供签名安装程序。

### macOS

解压 ZIP 后打开 `ElaWidgetToolsExample.app`。应用内携带 Qt framework 和插件，CI 会验证归档解压后的应用在隔离目录中启动。当前没有 Developer ID 签名和 Apple 公证，下载后的 Gatekeeper 提示及真实桌面交互仍需设备验证；开发包不能代表已经完成正式 macOS 分发认证。

## 验证下载

Linux 下载三个归档、元数据和校验文件到同一目录后执行：

```bash
sha256sum -c SHA256SUMS-Continuous-Build.txt
```

macOS 使用 `shasum -a 256 -c SHA256SUMS-Continuous-Build.txt`。只下载某个平台包时，仅对照清单中该文件的行，其他文件缺失是正常的。

Windows 可计算单个包的摘要，再与清单中的同名文件对照：

```powershell
Get-FileHash .\ElaWidgetTools-Continuous-Build-windows-x86_64.zip -Algorithm SHA256
```

SHA-256 用于核对内容完整性，不等于代码签名。源提交和构建记录可在 `release-info` 中查看。

## 维护者操作

### 手动更新 Continuous Build

在 GitHub Actions 选择“自动发布”，选择 `main` 并点击 Run workflow；也可以执行：

```bash
gh workflow run release.yml --repo jackfahdin/ElaWidgetTools --ref main
```

此操作使用当前 main 的完整提交 SHA，全部平台都使用同一个提交。任何平台失败都不会开始上传 Release。重新运行整个工作流可恢复失败草稿；旧运行若会回退已发布的预览提交，则拒绝发布，请从 main 重新运行。已经公开的稳定版不能通过重跑覆盖。

### 发布稳定版

1. 修改 `ElaWidgetTools/CMakeLists.txt` 中 `project(ElaWidgetTools VERSION X.Y.Z)` 的版本，提交并让常规 CI 通过。
2. 在该提交上创建带注释的标签并推送。例如发布 `2.0.0`：

```bash
git tag -a v2.0.0 -m "发布 ElaWidgetTools 2.0.0"
git push origin v2.0.0
```

3. 发布工作流再次检查格式、发布脚本测试、三个平台 Release 构建、QtTest、归档后的安装消费和 Example 启动。全部通过后公开 Release。

顶层工程和 Example 自身的 `0.1` 是原有工程元数据，**发布版本以组件库的 `2.0.0` 为准**。自动化不会猜测下一个版本，也不会创建稳定标签。现阶段只接受 `vX.Y.Z`，不接受 `vX.Y` 或 `-rc` 标签。

### 文件职责

| 文件 | 负责内容 |
| --- | --- |
| `.github/workflows/release.yml` | 触发条件、固定版本与 SHA、编排和发布权限 |
| `.github/workflows/release-linux.yml` | Linux SDK、编译、测试、归档验证 |
| `.github/workflows/release-windows.yml` | Windows SDK、MSVC、PowerShell、部署验证 |
| `.github/workflows/release-macos.yml` | macOS SDK、ARM64、ditto、应用验证 |
| `tools/release/package_*.py` | 各平台安装树检查和打包 |
| `tools/release/common.py` | 版本、文件名、摘要和元数据约定 |
| `tools/release/prepare.py` | 检查标签、判断是否跳过无变更预览 |
| `tools/release/assemble.py` | 汇总本次运行的平台资产和校验和 |
| `tools/release/publish.py` | 草稿恢复、上传复核、公开 Release |

平台文件各自声明依赖，便于独立维护。调整 Qt 发布基线时同步三个发布工作流、普通 CI、`common.py` 的元数据常量及本说明。发布脚本不使用第三方 Python 库，测试命令为：

```bash
python3 -m unittest discover -s tools/release/tests -v
```
