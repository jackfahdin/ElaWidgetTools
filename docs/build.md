# 编译与集成

[返回首页](../README.md) · [维护与路线图](roadmap.md) · [组件目录](components.md)

## 环境要求

| 工具 | 要求 |
| --- | --- |
| Qt | 6.12.0 及以上，包含 Widgets 和同版本私有开发头文件 |
| CMake | 3.25 及以上 |
| 构建工具 | Ninja |
| 编译器 | 支持 C++17，且与所用 Qt 工具链兼容 |

当前仓库提供 **Linux** 预设。已验证环境为 Ubuntu 26.04.1 x86_64、Qt 6.12.0、GCC 15.2.0、CMake 4.3.3、Ninja 1.12.1。Windows 和 macOS 尚未完成这次迁移后的验证。

CMake 中的 `find_package(Qt6 6.12.0 ...)` 表示最低版本要求，未使用 `EXACT`，也未设置 Qt 6 版本上限。后续 Qt 6.13、6.14 等版本同样属于维护范围；下方的 6.12.0 路径只是本机安装示例，可以换成更新版本的工具链目录。

库中的选项卡实现使用 `Qt6::WidgetsPrivate`。升级 Qt 后需要重新构建组件库，运行时使用与编译时匹配的 Qt；不要把不同 Qt 构建的组件库混用。静态构建仅指 ElaWidgetTools 本身，Qt 是否静态取决于安装的 Qt 工具链。

## 使用仓库预设

在仓库根目录执行，将 `QT_ROOT` 设为 Qt 工具链目录（即包含 `bin`、`lib`、`include` 的目录）：

```bash
export QT_ROOT="$HOME/Qt/6.12.0/gcc_64"
cmake --list-presets

cmake --preset linux-debug
cmake --build --preset linux-debug --parallel 6
./build/linux-debug/ElaWidgetToolsExample/ElaWidgetToolsExample
```

Release 构建：

```bash
cmake --preset linux-release
cmake --build --preset linux-release --parallel 6
./build/linux-release/ElaWidgetToolsExample/ElaWidgetToolsExample
```

两个预设默认构建动态组件库和完整示例。可根据机器内存调整并行数。构建和安装目录按预设隔离，编译数据库位于对应目录的 `compile_commands.json`。

## 本机用户预设

可以在根目录的 `CMakeUserPresets.json` 中保存个人 Qt 路径，避免每次设置环境变量。该文件已被 Git 忽略，构建产物也不会进入版本控制。

以下为当前主机可用的配置形式；已有该文件时，将条目合并进去，并按自己的安装位置调整路径：

```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "local-linux-debug",
      "inherits": "linux-debug",
      "cacheVariables": {
        "QT_SDK_DIR": "/home/zz/Qt/6.12.0/gcc_64"
      }
    },
    {
      "name": "local-linux-release",
      "inherits": "linux-release",
      "cacheVariables": {
        "QT_SDK_DIR": "/home/zz/Qt/6.12.0/gcc_64"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "local-linux-debug",
      "configurePreset": "local-linux-debug",
      "jobs": 6
    },
    {
      "name": "local-linux-release",
      "configurePreset": "local-linux-release",
      "jobs": 6
    }
  ]
}
```

当前主机已配置这两个本地预设，可以直接执行：

```bash
cmake --preset local-linux-debug
cmake --build --preset local-linux-debug
./build/local-linux-debug/ElaWidgetToolsExample/ElaWidgetToolsExample
```

更换 Qt 安装或编译器时，建议使用新的预设名和构建目录，避免旧缓存中的 `Qt6_DIR` 等路径继续指向之前的工具链。命令行的 `-DQT_SDK_DIR=...` 也可覆盖预设中的路径。

## 仅构建组件库与静态库

| CMake 选项 | 默认值 | 作用 |
| --- | --- | --- |
| `QT_SDK_DIR` | 从预设的 `QT_ROOT` 环境变量读取 | 指定 Qt 工具链目录；留空时使用 CMake 标准查找路径 |
| `ELAWIDGETTOOLS_BUILD_EXAMPLE` | `ON` | 是否构建示例 |
| `ELAWIDGETTOOLS_BUILD_STATIC_LIB` | `OFF` | 是否构建静态组件库 |
| `ELAWIDGETTOOLS_BUILD_TESTS` | `OFF` | 是否查找 Qt Test 并构建独立回归测试 |

例如，以仓库 Release 预设为基础，在单独目录构建静态库：

```bash
cmake --preset linux-release -B build/linux-static \
  -DELAWIDGETTOOLS_BUILD_STATIC_LIB=ON \
  -DELAWIDGETTOOLS_BUILD_EXAMPLE=OFF \
  -DCMAKE_INSTALL_PREFIX="$PWD/install/linux-static"
cmake --build build/linux-static --parallel 6
cmake --install build/linux-static
```

重复使用的静态配置也可以继承 `linux-release` 写入用户预设。静态库目标会自动向消费者传递 `ELAWIDGETTOOLS_STATIC`，不需要手动设置导入导出宏。

## 安装与引用

以仓库 Release 预设为例：

```bash
cmake --install build/linux-release
```

默认安装前缀为 `install/linux-release`。本地预设则使用各自的目录，例如 `install/local-linux-release`。

| 安装内容 | 默认位置 |
| --- | --- |
| 公共头文件 | `include/ElaWidgetTools/` |
| 组件库 | `lib/`（以 `CMAKE_INSTALL_LIBDIR` 为准） |
| CMake 包配置 | `lib/cmake/ElaWidgetTools/` |
| 示例可执行文件 | `bin/ElaWidgetToolsExample` |

外部项目使用导出的 `Ela::WidgetTools` 目标：

```cmake
cmake_minimum_required(VERSION 3.25)
project(MyApp LANGUAGES CXX)
set(CMAKE_AUTOMOC ON)

find_package(ElaWidgetTools REQUIRED)
add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE Ela::WidgetTools)
```

配置外部项目时，将组件库的安装前缀和 Qt 工具链目录加入 `CMAKE_PREFIX_PATH`，例如：

```bash
cmake -S /path/to/MyApp -B /path/to/MyApp/build -G Ninja \
  -DCMAKE_PREFIX_PATH="$PWD/install/linux-release;$QT_ROOT"
cmake --build /path/to/MyApp/build --parallel 6
```

应用创建 `QApplication` 后应调用 `ElaApplication::getInstance()->init()`，以初始化字体与内置资源。

Linux 安装使用相对路径定位同一安装前缀中的组件库，并保留构建所用 Qt 的运行库路径。**安装结果用于本机开发验证，尚不是可直接分发到任意机器的独立部署包**；Qt 插件、系统依赖和跨机器打包需要另行处理。

## 已验证范围

- Linux Debug / Release 动态库和示例均完成编译、安装。
- 示例在 Xvfb 虚拟显示器中完成初始化并持续运行，随后由超时命令结束。
- 独立项目通过 `find_package(ElaWidgetTools)` 使用安装后的动态库和静态库，验证资源、图标、主题切换、选项卡绘制与正常退出。
- Qt 6.11.1 会在配置阶段被拒绝，符合最低版本策略。

已新增可选的 QtTest 和安装消费检查，运行方式见[测试指南](testing.md)。这些验证不等于完整 GUI 交互回归。Windows 的 DXGI 示例仅在 Windows 注册，macOS 适配仍在计划中。

## 常见提示

- **找不到 Qt 6.12**：检查 `QT_ROOT` / `QT_SDK_DIR` 是否指向工具链目录，以及旧缓存是否仍指向其他 Qt。
- **找不到 WidgetsPrivate 或私有头文件**：安装与 Qt 版本匹配的私有开发文件，不要混用系统 Qt 的私有头文件。
- **Qt 私有接口警告**：这是当前选项卡实现的实际依赖，保留提示，升级 Qt 后重新构建。
- **找不到 Vulkan 头文件**：在本次 Widgets 构建中是可选依赖提示，已验证不会阻止构建。
- **弃用接口警告**：上游仍有 `QCheckBox::stateChanged` 和旧鼠标事件接口调用；当前可编译，后续可继续清理。
- **无显示服务器**：桌面运行需要 X11 / Wayland 会话。自动化启动检查可使用 `xvfb-run -a`，需要系统安装 Xvfb。
