project(
    ElaWidgetTools
    VERSION 2.0.0
    LANGUAGES CXX)

file(GLOB Origin CONFIGURE_DEPENDS *.h *.cpp)
file(GLOB private CONFIGURE_DEPENDS private/*.h private/*.cpp)
file(GLOB Development CONFIGURE_DEPENDS Development/*.h Development/*.cpp)
file(GLOB Development_Command CONFIGURE_DEPENDS Development/Command/*.h Development/Command/*.cpp)

source_group(private FILES ${private})
source_group(Development FILES ${Development})
source_group(Development/Command FILES ${Development_Command})

set(PROJECT_SOURCES ${Origin} ${private} ${Development} ${Development_Command}
                    ${CMAKE_CURRENT_SOURCE_DIR}/ElaWidgetTools.qrc)

if(ELAWIDGETTOOLS_BUILD_STATIC_LIB)
    set(LIB_TYPE "STATIC")
else()
    set(LIB_TYPE "SHARED")
endif()

add_library(${PROJECT_NAME} ${LIB_TYPE} ${PROJECT_SOURCES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_definitions(${PROJECT_NAME} PRIVATE ELAWIDGETTOOLS_LIBRARY)
if(ELAWIDGETTOOLS_BUILD_STATIC_LIB)
    target_compile_definitions(${PROJECT_NAME} PUBLIC ELAWIDGETTOOLS_STATIC)
endif()
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES EXPORT_NAME WidgetTools)
add_library(Ela::WidgetTools ALIAS ${PROJECT_NAME})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ElaWidgetTools>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private ${CMAKE_CURRENT_SOURCE_DIR}/Development
            ${CMAKE_CURRENT_SOURCE_DIR}/Development/Command)

if(MINGW)
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
endif()
if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
endif()

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC Qt6::Widgets
    PRIVATE Qt6::WidgetsPrivate)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE D3D11 DXGI)
endif()

install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ElaWidgetTools
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "Font" EXCLUDE
    PATTERN "Image" EXCLUDE
    PATTERN "private" EXCLUDE
    PATTERN "Development" EXCLUDE)

install(
    EXPORT ${PROJECT_NAME}Targets
    NAMESPACE Ela::
    FILE ${PROJECT_NAME}Targets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ElaWidgetTools)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ElaWidgetTools)

install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ElaWidgetTools)
