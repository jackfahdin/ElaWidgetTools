# 代码风格与静态分析

[返回首页](../README.md) · [参与贡献](contributing.md) · [编译指南](build.md)

## 使用原则

配置沿用现有代码习惯：C++ 使用 4 空格缩进，类型、函数和控制语句的大括号换行，指针写作 `QWidget*`，引用写作 `const QString&`，目标行宽为 120。保留现有头文件顺序；Qt 与 Ela 的声明宏单独配置，避免连续宏调用被合并排版。

组件库和 Example 的现有 C++ 源码及 CMake 文件已统一格式，作为后续开发的格式基线。新建或主动修改的文件继续遵循规则，格式整理与功能修改分别提交。格式检查报差异不等于编译失败。

| 文件 | 作用 |
| --- | --- |
| [`.editorconfig`](../.editorconfig) | UTF-8、LF 换行、空格缩进及文件末尾换行；Markdown 保留行尾空格，避免破坏硬换行 |
| [`.clang-format`](../.clang-format) | C++ 排版，识别 Qt / Ela 属性、单例及枚举宏 |
| [`.clang-tidy`](../.clang-tidy) | 手动静态分析，先关注空指针、对象生命周期、移动后使用等问题，并提供 `nullptr` 改进建议 |
| [`.cmake-format.yaml`](../.cmake-format.yaml) | CMake 排版，命令小写、关键字大写，保留注释文字和列表顺序 |

这些文件需要对应工具或编辑器插件读取。普通 CMake / Ninja 构建不依赖它们，不接入提交钩子；CI 使用固定版本执行格式检查，clang-tidy 保留为独立手动工作流。静态分析不替代编译、运行检查或 QtTest。

## 工具版本与安装

统一使用 **clang-format / clang-tidy 21.1.6、cmakelang 0.6.13、PyYAML 6.0.3**。`cmake-format` 由 `cmakelang` 提供；读取 YAML 配置需要 PyYAML。顶层工具版本固定在[开发工具依赖](../tools/requirements-code-tools.txt)，升级时应重新检查代表文件的格式变化和分析告警。

在仓库根目录执行以下 Linux 命令，安装到已被 Git 忽略的独立虚拟环境：

```bash
python3 -m venv .venv-code-tools
.venv-code-tools/bin/python -m pip install -r tools/requirements-code-tools.txt
source .venv-code-tools/bin/activate
clang-format --version
clang-tidy --version
cmake-format --version
```

如果系统缺少 `venv`，先安装对应 Python 虚拟环境支持包。Windows 可使用 `py -m venv .venv-code-tools` 并在 PowerShell 中执行 `.venv-code-tools\Scripts\Activate.ps1`，也可使用本机 LLVM 工具；本轮实际验证平台为 Linux。编辑器中应选用同一工具版本及仓库配置。

## C++ 格式

先预览一个文件的差异，不修改源码：

```bash
clang-format --style=file ElaWidgetTools/ElaPushButton.cpp > /tmp/ElaPushButton.formatted.cpp
diff -u ElaWidgetTools/ElaPushButton.cpp /tmp/ElaPushButton.formatted.cpp
```

确认排版后，按需格式化具体文件：

```bash
clang-format --style=file -i ElaWidgetTools/ElaPushButton.cpp
clang-format --style=file --dry-run --Werror ElaWidgetTools/ElaPushButton.cpp
```

`--dry-run --Werror` 在发现格式差异时返回非零退出码。提交前对本次修改的源码运行检查，CI 对仓库源码执行同一检查。自动生成的 `moc_*`、`ui_*`、`qrc_*` 及 `build/`、`install/` 中的文件不应作为格式化输入。

## CMake 格式

在仓库根目录运行，显式指定配置以保证编辑器与命令行一致：

```bash
cmake-format -c .cmake-format.yaml --check CMakeLists.txt
cmake-format -c .cmake-format.yaml -i CMakeLists.txt
```

第一条只检查，第二条会修改文件。按需对 `ElaWidgetTools/CMakeLists.txt`、`ElaWidgetToolsExample/CMakeLists.txt` 和 `.cmake` / `.cmake.in` 文件使用相同命令。`CMakePresets.json` 是 JSON，不由 cmake-format 处理。

## C++ 静态分析

现有 Linux Presets 已启用 `CMAKE_EXPORT_COMPILE_COMMANDS`。先按[编译指南](build.md)配置本机 Qt，并完成一次构建，使 Qt 自动生成头文件准备就绪：

```bash
cmake --preset local-linux-debug
cmake --build --preset local-linux-debug
clang-tidy --verify-config
clang-tidy -p build/local-linux-debug ElaWidgetTools/ElaPushButton.cpp
```

若使用公共 `linux-debug` 预设，改用对应的 `build/linux-debug` 目录。`-p` 指向含 `compile_commands.json` 的构建目录；不要手写 Qt 头文件路径或复制其他主机的编译数据库。

目前仅启用少量 Clang 分析器、`bugprone` 和 `modernize-use-nullptr` 规则，不启用所有命名、性能或现代化检查。头文件诊断限定为本项目源码，过滤构建、安装和系统头文件；这只限制告警展示，分析仍需解析实际依赖。

普通告警暂不升级为错误，但配置错误或编译解析错误仍会导致命令失败。发现告警后先核对实际行为，尤其注意 Qt 父子对象所有权、隐式共享和宏展开；不直接对全仓使用 `--fix`。现有源码存在告警时应单独评估和修复，不与工具接入混在一起。

## 编辑器设置

- 启用 EditorConfig 支持，读取仓库根目录规则。
- C++ 格式工具选择 clang-format，并使用 `file` 风格及约定版本。
- CMake 格式工具选择 cmake-format，加载根目录 `.cmake-format.yaml`。
- 保存时自动格式化由开发者自行开启；首次编辑历史文件时先检查差异，避免混入大范围排版修改。

工具初次接入时通过解析检查、代表文件的格式预览及单个翻译单元静态分析验证。现在另有可选的 [QtTest 回归测试](testing.md) 和 [CI](ci.md)，验证范围随功能和工具版本继续更新。

2026-10-29 已在本机 Linux / Qt 6.12.0 验证上述固定版本：C++ 头文件、实现文件和枚举宏格式预览正常，CMake 格式输出可再次通过格式检查，clang-tidy 配置校验通过。`ElaPushButton.cpp` 分析完成，未报告项目内告警（依赖代码的 125 条告警被过滤）；这不代表全仓静态分析通过。`local-linux-debug` 配置与组件库、Example 构建通过。

同日完成全仓格式基线整理：455 个 C++ 文件和 4 个 CMake 文件（含安装配置模板）全部通过格式检查，Debug / Release 组件库及 Example 重新编译通过。已核对宏、条件编译、长字符串及 CMake 差异，未执行 clang-tidy 自动修复或功能修改。
