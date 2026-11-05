# 稳定版与 Continuous Build 自动发布设计

## 目标

为 ElaWidgetTools 建立可审计的 GitHub Release 自动发布流程。稳定版由版本标签触发，开发版由每日北京时间 00:00（UTC 16:00，GitHub 可能延迟）的定时任务和手动任务更新固定的 `continuous-build` 预览 Release。Linux、Windows、macOS 的构建和打包逻辑分别放在独立文件中，平台之间只通过明确的工作流输入和 GitHub Artifact 传递结果。

第一阶段发布开发包和示例程序包，不在组件库中加入应用内更新器。发布流程只负责构建、测试、打包、校验和 GitHub Release，不改变库的运行时行为。

## 版本规则

项目当前版本来自 `ElaWidgetTools/CMakeLists.txt` 的 `project(ElaWidgetTools VERSION ...)`。

- 稳定版标签使用 `vX.Y.Z`，例如 `v2.0.0`、`v2.1.0`、`v2.1.1`。
- 稳定版工作流在构建前校验标签版本与 CMake 项目版本完全一致；不一致时在构建平台之前失败。
- 稳定版 Release 使用不可变标签和公开 Release。已公开的稳定版不能被后续任务覆盖。
- Continuous Build 使用固定的 `continuous-build` 标签和 GitHub Pre-release。每次发布记录完整源提交 SHA、工作流地址和构建时间。
- 定时任务只在 `main` 的 HEAD 与当前 Continuous Build 记录的源提交不同时构建。手动任务允许强制重新构建当前 HEAD。
- 第一阶段不自动创建候选版标签。未来需要候选版时，沿用同一稳定标签校验，增加 `vX.Y.Z-rc.N` 的预发布规则。

版本升级由维护者在代码提交中修改 CMake 项目版本，再推送匹配的稳定标签。自动发布流程不会猜测或自动递增版本号，避免标签、安装包版本和 CMake 包版本不一致。

## 发布资产

每个平台的安装目录都通过现有 CMake 安装规则生成，再由对应平台脚本打包。资产名称包含版本或 `Continuous-Build` 标签，便于在下载页面直接识别平台和架构。

| 平台 | 资产 | 内容 |
| --- | --- | --- |
| Linux x86_64 | `ElaWidgetTools-<标签>-linux-x86_64.tar.gz` | 动态库、公共头文件、CMake 包配置和 Example |
| Windows x64 | `ElaWidgetTools-<标签>-windows-x86_64.zip` | DLL、导入库、公共头文件、CMake 包配置和 Example |
| macOS Apple Silicon | `ElaWidgetTools-<标签>-macos-arm64.zip` | 部署后的 `ElaWidgetToolsExample.app` 和开发文件 |

发布任务额外生成：

- `SHA256SUMS-<标签>.txt`：所有平台资产的 SHA-256 校验和。
- `release-info-<标签>.json`：版本、发布模式、源提交、工作流地址、平台资产名称、大小和摘要。

GitHub 自动提供的源码压缩包继续保留。第一阶段不生成 `.deb`、`.rpm`、Windows 安装程序或 macOS DMG；这些格式需要额外的安装元数据、签名和平台验收，后续按平台独立增加。

## 工作流边界

工作流文件职责固定如下：

```text
.github/workflows/
├── ci.yml                    # Pull Request、main 推送和手动门禁
├── release.yml               # stable/nightly 编排、版本判断、Release 发布
├── release-linux.yml         # Linux 构建、测试、安装树和 tar.gz
├── release-windows.yml       # Windows 构建、测试、安装树和 zip
└── release-macos.yml         # macOS 构建、测试、部署和 zip
```

### 常规 CI

`ci.yml` 继续负责 Pull Request、`main` 推送和手动验证。它只验证代码，不创建 Release，也不需要写权限。现有格式检查、Linux Debug/Release/Static、Windows MSVC、macOS Apple Silicon、QtTest、安装消费和 macOS 部署检查保持原有范围。

### 平台发布工作流

三个平台发布工作流都使用 `workflow_call`，输入保持一致：

- `source_sha`：要构建的完整提交 SHA。
- `release_label`：稳定版本号或 `Continuous-Build`。
- `release_mode`：`stable` 或 `nightly`。

每个工作流只使用自己的运行器和 shell：

- `release-linux.yml` 使用 Ubuntu、Qt Linux SDK、Ninja、Bash 和 `tar`。
- `release-windows.yml` 使用 Windows Server、MSVC、Qt Windows SDK、Ninja 和 PowerShell。
- `release-macos.yml` 使用 macOS 15 Apple Silicon、Qt macOS SDK、Ninja、Bash 和 `ditto`。

平台工作流执行顺序固定为：检出 `source_sha`、安装平台依赖、配置对应 Preset、编译、运行 QtTest、安装到临时目录、检查安装树、生成平台资产、解压归档并运行安装消费和 Example 启动检查、上传 GitHub Artifact。任何测试或资产检查失败都会阻止发布任务。

### 发布编排工作流

`release.yml` 只处理跨平台编排：

1. `push` 到 `v*` 标签时进入 stable 模式。
2. `schedule` 和 `workflow_dispatch` 在 `main` 上进入 nightly 模式。
3. 准备任务解析项目版本、校验标签、确定源提交和发布标签，并读取现有 Continuous Build 的源提交。
4. 三个平台工作流以相同的 `source_sha`、`release_label` 和 `release_mode` 并行运行。
5. 汇总任务下载三个平台 Artifact，调用通用资产校验逻辑，生成校验和与发布元数据。
6. 发布任务使用 `contents: write` 创建或更新 Release；稳定版发布完成后设置为公开稳定 Release，Continuous Build 发布完成后设置为预发布。

稳定版使用草稿 Release 组装资产，所有文件校验成功后才公开。Continuous Build 替换固定标签的资产，更新前先保持 Release 草稿状态，避免下载者看到混合版本。

## 脚本边界

脚本目录按通用逻辑和平台逻辑拆分：

```text
tools/release/
├── common.py                 # 版本、标签、文件摘要和元数据模型
├── package_linux.py          # Linux 安装树和 tar.gz
├── package_windows.py       # Windows 安装树和 zip
├── package_macos.py         # macOS .app 和开发文件 zip
├── publish.py                # GitHub Release、资产上传和远端摘要校验
└── tests/
    ├── test_common.py
    ├── test_package_linux.py
    ├── test_package_windows.py
    ├── test_package_macos.py
    └── test_release.py         # 发布恢复、标签移动、摘要失败与回退保护
```

`common.py` 只定义版本解析、稳定标签校验、文件摘要、资产元数据和安全文件名校验，不读取平台环境变量，也不执行平台命令。三个 `package_*.py` 只接收输入目录、输出目录、标签和源提交，分别实现自己的归档格式和目录检查。`publish.py` 只处理 GitHub API 和 Release 状态，不负责编译或理解平台目录结构。

所有新增 Python 脚本和工作流中的关键步骤都使用中文注释说明输入、输出、失败条件和安全边界。脚本通过参数和返回码通信，不通过隐式全局文件共享状态。

## 错误处理和安全边界

- 稳定标签与 CMake 版本不一致时，在平台构建前失败。
- 旧运行重试发布前比较滚动标签的提交关系，只接受同一提交或后续提交，阻止 Continuous Build 回退；上传期间标签被其他操作修改则保持草稿。
- 稳定标签已指向其他提交、已存在公开 Release 或资产摘要不一致时，拒绝覆盖。
- 任何平台缺少安装目录、Example、库文件或压缩包时，汇总任务失败。
- 发布前重新计算所有资产的大小和 SHA-256，发布后通过 GitHub API 读取远端资产摘要进行复核。
- Release 编排任务默认只读权限；只有最终发布任务临时拥有 `contents: write`。
- 任务固定检出完整提交 SHA，避免构建过程中 `main` 移动造成源代码和发布元数据不一致。
- 第一阶段不包含代码签名、Windows 安装器签名、macOS 公证或更新签名密钥。发布文档明确标注这些资产不代表已完成商业分发验证。

## 测试策略

发布脚本测试不依赖 GitHub 网络或真实平台工具链：

- 版本测试覆盖正常 SemVer、稳定标签匹配、标签不匹配和 Continuous Build 标签。
- 资产测试使用临时目录生成最小安装树，验证三个平台的文件命名、必需文件、空文件拒绝和归档内容。
- 元数据测试验证源提交、资产大小、SHA-256 和 JSON 字段的一致性。
- 发布状态测试使用假的 GitHub API 响应，验证稳定版拒绝覆盖和 Continuous Build 可更新。

平台工作流继续运行现有 QtTest 和安装消费验证；新增的发布测试在准备任务中先执行，失败时不触发昂贵的跨平台构建。

## 文档和使用流程

新增 `docs/releases.md`，说明：

1. 如何修改 CMake 项目版本并准备稳定标签。
2. 如何手动触发 Continuous Build。
3. 如何按平台下载资产和验证 SHA-256。
4. 如何区分开发包、Example 和 GitHub 源码包。
5. 当前 Apple Silicon、Linux x86_64 和 Windows x64 的验证范围，以及尚未提供的签名、公证和安装器格式。

README、`docs/ci.md` 和 `docs/roadmap.md` 只链接发布说明，不重复平台脚本细节。

## 非目标

- 不在本阶段加入应用内自动更新器。
- 不把 Linux、Windows、macOS 的构建命令合并到一个跨平台脚本。
- 不自动递增项目版本或自动移动稳定版本标签。
- 不在没有对应平台签名凭据和设备验证的情况下声明正式安装器、签名或公证支持。
