# 维护与路线图

[返回首页](../README.md) · [组件目录](components.md) · [参与贡献](contributing.md)

## 分支定位

本仓库以 [Liniyous/ElaWidgetTools](https://github.com/Liniyous/ElaWidgetTools) 的代码为基础，开展独立开发与维护。现有代码基线对应上游提交 `aae58f5`（ElaWidgetTools V3.0 重构初步版本）。

[RainbowCandyX/ElaWidgetTools](https://github.com/RainbowCandyX/ElaWidgetTools) 是文档组织方式的参考。本仓库与其代码基线和后续演进不同，不沿用其组件数量、平台验证结论或附加功能声明。

上游的介绍、发布版本与 API 文档可供参考；本分支的接口、兼容性与维护范围以当前仓库为准。

## Qt 版本策略

**后续仅维护 Qt 6.12.0 及以上版本，以 Qt 6.12.0 为最低维护目标。**

- Qt 5 和 Qt 6.12.0 以下版本不在后续维护范围内。
- 现有源码中的旧版本兼容分支将在迁移阶段评估，不代表继续提供旧版支持。
- 新版 Qt 是否通过验证，需要以届时的构建与运行结果为准；维护范围不等于所有未来版本已经验证。

顶层 [CMakeLists.txt](../CMakeLists.txt) 已统一要求 Qt 6.12.0 及以上版本，移除 Qt 5 查找和旧版上限。仓库提供 Linux、Windows、macOS 的 [CMake Presets](../CMakePresets.json)，配置、构建和集成方式见[编译指南](build.md)。

## 平台计划

| 平台 | 现有基础 | 后续重点 |
| --- | --- | --- |
| Windows | 已有窗口处理、DXGI 及 MSVC CI 配置 | 云端编译、测试和部署验证 |
| Linux | 已在 Ubuntu 26.04.1 / Qt 6.12.0 完成构建、安装与基础运行验证 | 扩展桌面交互、显示后端和发行版验证 |
| macOS | 实验性适配，已准备构建、测试及部署 CI | 云端实际验证、原生窗口交互和多显示器体验 |

当前验证覆盖 Linux 的 Debug / Release 动态库与示例、静态库及安装后的消费项目；运行验证包含资源加载、图标、主题和选项卡。Windows、macOS 及其他 Qt 版本尚未验证。

部分能力具有平台限制：例如 `ElaDxgiManager` 依赖 Windows 的 DXGI，不能直接作为 Linux 或 macOS 的屏幕采集实现。

## 推进顺序

### 第一阶段：建立独立维护入口（已完成）

- 明确本仓库与上游、参考分支的关系。
- 整理中文首页、组件索引和贡献说明。
- 公布 Qt 版本策略与平台计划。

### 第二阶段：迁移构建并验证（Linux 基线已完成）

- 已将 Qt 最低版本要求调整为 6.12.0，修复图标枚举到 `QChar` 的转换兼容问题。
- 已整理 Qt 路径、目标依赖、标准安装目录和示例构建，加入 CMake Presets。
- 已明确 `WidgetsPrivate` 依赖边界；升级 Qt 后仍需重新构建和验证。
- 已建立 Linux 构建与基础运行验证；下一步补充 Windows 验证与更完整的 GUI 回归。
- 已补充[编译与集成指南](build.md)，独立分发包的部署流程另行完善。

### 第三阶段：建立 CI 与自动化回归（已实现，云端结果待推送）

- GitHub Actions 覆盖 Linux Debug / Release、Linux 静态库、Windows MSVC 和 macOS Apple Silicon。
- QtTest 默认可选，首批覆盖标题栏、菜单、控件信号、导航、命令历史及资源；另有安装消费验证。
- 格式检查使用固定工具版本；clang-tidy 作为独立手动工作流。
- 入口、运行方式和结果范围见 [CI](ci.md) 与[测试指南](testing.md)。

### 第四阶段：推进 macOS 适配

- 已补充 macOS 构建预设、加载路径及 `.app` 部署脚本；CI 检查部署后应用的独立启动。
- 已补充窗口状态同步、嵌入菜单策略和非 Windows 默认字体处理。
- 继续验证无边框窗口、拖动缩放、系统全屏、焦点、弹出层与高分屏表现，边界见 [macOS 说明](macos.md)。
- 完成验证后更新支持矩阵，再标记为正式支持。

上述阶段表示工作顺序，不承诺发布日期。后续平台适配和功能改动将随验证结果更新。
