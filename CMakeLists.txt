cmake_minimum_required(VERSION 3.25)

project(ElaFramework VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(QT_SDK_DIR "" CACHE PATH "Qt 6.12 及以上版本的安装目录")
if(QT_SDK_DIR)
    list(PREPEND CMAKE_PREFIX_PATH "${QT_SDK_DIR}")
endif()

# 6.12.0 是最低版本，不使用 EXACT，也不设置 Qt 6 的版本上限。
find_package(Qt6 6.12.0 REQUIRED COMPONENTS Widgets WidgetsPrivate)
message(STATUS "使用 Qt ${Qt6_VERSION}：${Qt6_DIR}")

include(GNUInstallDirs)
option(ELAWIDGETTOOLS_BUILD_STATIC_LIB "构建静态组件库" OFF)
option(ELAWIDGETTOOLS_BUILD_EXAMPLE "构建示例程序" ON)

# 安装后的示例优先从同一安装前缀加载组件库；保留本机 Qt 的运行库路径。
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

add_subdirectory(ElaWidgetTools)
if(ELAWIDGETTOOLS_BUILD_EXAMPLE)
    add_subdirectory(ElaWidgetToolsExample)
endif()
