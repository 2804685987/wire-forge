cmake_minimum_required(VERSION 3.5)
project(WireForgeMain LANGUAGES CXX)

# 支持Qt5和Qt6
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Svg Xml)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(BUILD_QWINDOWKIT ON)

file(GLOB_RECURSE SOURCES
    CONFIGURE_DEPENDS
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    src/*.cpp
    src/*.h
        src/include/*.h
)
set(CPP_SOURCES "")
set(HEADER_SOURCES "")
foreach(file ${SOURCES})
    if(file MATCHES "\\.cpp$")
        list(APPEND CPP_SOURCES ${file})
    elseif(file MATCHES "\\.h$")
        list(APPEND HEADER_SOURCES ${file})
    endif()
endforeach()


file(GLOB_RECURSE AUTO_RESOURCE_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    res/*.png res/*.jpg res/*.jpeg res/*.gif res/*.svg res/*.ico res/*.ttf res/*.qss
)

# 资源处理：兼容Qt5和Qt6
if(${QT_VERSION_MAJOR} EQUAL 6)
    add_executable(WireForgeMain
            MACOSX_BUNDLE
            ${CPP_SOURCES}
        ${HEADER_SOURCES}
        version.rc
    )
    qt_add_resources(WireForgeMain "WireForgeMain"
        PREFIX "/"
        FILES ${AUTO_RESOURCE_FILES}
    )
else()
    add_executable(WireForgeMain
         MACOSX_BUNDLE
        ${CPP_SOURCES}
        ${HEADER_SOURCES}
        version.rc
        res/res.qrc
    )
endif()

target_link_libraries(WireForgeMain PRIVATE QFluent)
target_link_libraries(WireForgeMain
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Svg
        Qt${QT_VERSION_MAJOR}::Xml
)

target_include_directories(WireForgeMain PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 运行前将 Components 目录复制到可执行文件目录，便于运行时按相对路径自动发现组件 JSON
add_custom_command(TARGET WireForgeMain POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Components"
        "$<TARGET_FILE_DIR:WireForgeMain>/Components"
        COMMENT "Copying Components directory..."
)


if(WIN32 AND NOT BUILD_QWINDOWKIT)
    target_link_libraries(WireForgeMain PRIVATE dwmapi)
endif()

if (BUILD_QWINDOWKIT)
    add_subdirectory(libs/qwindowkit)
    target_link_libraries(WireForgeMain PRIVATE QWindowKit::Widgets)
    target_compile_definitions(WireForgeMain PRIVATE USE_QWINDOWKIT=1)

    if(WIN32)
        get_target_property(_qt_core_path Qt${QT_VERSION_MAJOR}::Core LOCATION)
        get_filename_component(_qt_bin_dir ${_qt_core_path} DIRECTORY)
        get_filename_component(_qt_install_dir ${_qt_bin_dir} DIRECTORY)
        set(_qt_platforms_dir "${_qt_install_dir}/plugins/platforms")

        add_custom_command(TARGET WireForgeMain POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:QWindowKit::Core>
                $<TARGET_FILE:QWindowKit::Widgets>
                $<TARGET_FILE_DIR:WireForgeMain>
                COMMENT "Copying QWindowKit DLLs..."
        )

        # 复制 Qt 平台插件（解决 "no Qt platform plugin could be initialized" 错误）
        add_custom_command(TARGET WireForgeMain POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${_qt_platforms_dir}"
                "$<TARGET_FILE_DIR:WireForgeMain>/platforms"
                COMMENT "Copying Qt platform plugins..."
        )
    endif()
else()
    target_compile_definitions(WireForgeMain PRIVATE USE_QWINDOWKIT=0)
endif()