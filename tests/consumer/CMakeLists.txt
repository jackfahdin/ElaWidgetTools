cmake_minimum_required(VERSION 3.25)
project(ElaInstalledConsumer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(ElaWidgetTools REQUIRED)
add_executable(ela_consumer main.cpp)
target_link_libraries(ela_consumer PRIVATE Ela::WidgetTools)
if(MSVC)
    target_compile_options(ela_consumer PRIVATE /utf-8)
    add_custom_command(
        TARGET ela_consumer
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:ela_consumer>
                $<TARGET_FILE_DIR:ela_consumer>
        COMMAND_EXPAND_LISTS)
endif()
enable_testing()
add_test(NAME installed_consumer COMMAND ela_consumer)
set_tests_properties(installed_consumer PROPERTIES TIMEOUT 30)
