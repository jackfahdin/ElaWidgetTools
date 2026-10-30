project(
    ElaWidgetToolsExample
    VERSION 0.1
    LANGUAGES CXX)

file(GLOB ORIGIN CONFIGURE_DEPENDS *.cpp *.h)
file(GLOB MODELVIEW CONFIGURE_DEPENDS ModelView/*.h ModelView/*.cpp)
file(GLOB EXAMPLEPAGE CONFIGURE_DEPENDS ExamplePage/*.h ExamplePage/*.cpp)

source_group(ModelView FILES ${MODELVIEW})
source_group(ExamplePage FILES ${EXAMPLEPAGE})

qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION ${ORIGIN} ${MODELVIEW} ${EXAMPLEPAGE})

file(
    GLOB_RECURSE resource_files CONFIGURE_DEPENDS
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    Resource/*.png
    Resource/*.jpg
    Resource/*.svg
    Resource/*.ico
    Resource/*.ttf
    Resource/*.webp
    Resource/*.js)
qt_add_resources(${PROJECT_NAME} "ElaWidgetToolsExample" PREFIX "/" FILES ${resource_files})

target_include_directories(${PROJECT_NAME} PRIVATE ExamplePage ModelView)
target_link_libraries(${PROJECT_NAME} PRIVATE Ela::WidgetTools)
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
               MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
               MACOSX_BUNDLE_GUI_IDENTIFIER io.github.jackfahdin.ElaWidgetToolsExample
               MACOSX_BUNDLE TRUE
               WIN32_EXECUTABLE TRUE)
if(APPLE)
    set_target_properties(
        ${PROJECT_NAME} PROPERTIES INSTALL_RPATH
                                   "@executable_path/../Frameworks;@executable_path/../../../${CMAKE_INSTALL_LIBDIR}")
endif()
qt_finalize_executable(${PROJECT_NAME})

install(
    TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(WIN32 OR APPLE)
    qt_generate_deploy_app_script(TARGET ${PROJECT_NAME} OUTPUT_SCRIPT deploy_script NO_TRANSLATIONS)
    install(SCRIPT ${deploy_script})
endif()
