cmake_minimum_required(VERSION 3.16)  # 建议提升到 3.16+，对 Qt6 更好
project(WireForgeMain LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ==================== Qt 包查找 ====================
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
        Core
        Widgets
        Gui
        Svg
        Xml
        SerialPort     # DebugBlackBox 后续需要
)

set(BUILD_QWINDOWKIT ON)

# ==================== 自动扫描所有源码 ====================
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        "src/*.cpp"
        "src/*.h"
        "src/include/*.h"
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

# ==================== 资源文件 ====================
file(GLOB_RECURSE AUTO_RESOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        res/*.png res/*.jpg res/*.jpeg res/*.gif res/*.svg
        res/*.ico res/*.ttf res/*.qss
)

# ==================== 可执行文件 ====================
if(${QT_VERSION_MAJOR} EQUAL 6)
    add_executable(WireForgeMain
            MACOSX_BUNDLE
            ${CPP_SOURCES}
            ${HEADER_SOURCES}
            version.rc
    )
    qt_add_resources(WireForgeMain "WireForgeMain" PREFIX "/" FILES ${AUTO_RESOURCE_FILES})
else()
    add_executable(WireForgeMain
            MACOSX_BUNDLE
            ${CPP_SOURCES}
            ${HEADER_SOURCES}
            version.rc
            res/res.qrc
    )
endif()

# ==================== 包含目录（重点新增） ====================
target_include_directories(WireForgeMain PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics
        ${CMAKE_CURRENT_SOURCE_DIR}/src/UI
        ${CMAKE_CURRENT_SOURCE_DIR}/src/View
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Model
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Controller

)

# ==================== 链接库 ====================
target_link_libraries(WireForgeMain PRIVATE
        QFluent
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Svg
        Qt${QT_VERSION_MAJOR}::Xml
        Qt${QT_VERSION_MAJOR}::SerialPort
)

# ==================== QWindowKit 支持 ====================
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

        add_custom_command(TARGET WireForgeMain POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${_qt_platforms_dir}"
                "$<TARGET_FILE_DIR:WireForgeMain>/platforms"
                COMMENT "Copying Qt platform plugins..."
        )
    endif()
else()
    target_compile_definitions(WireForgeMain PRIVATE USE_QWINDOWKIT=0)
    if(WIN32)
        target_link_libraries(WireForgeMain PRIVATE dwmapi)
    endif()
endif()

# ==================== 复制 Components 目录 ====================
add_custom_command(TARGET WireForgeMain POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Components"
        "$<TARGET_FILE_DIR:WireForgeMain>/Components"
        COMMENT "Copying Components directory..."
)

# ==================== 编译警告 ====================
if(MSVC)
    target_compile_options(WireForgeMain PRIVATE /W4)
else()
    target_compile_options(WireForgeMain PRIVATE -Wall -Wextra -Wno-sign-compare)
endif()