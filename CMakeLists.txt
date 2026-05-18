cmake_minimum_required(VERSION 3.5)

project(WireForge VERSION 0.1 LANGUAGES CXX)

set(CMAKE_PREFIX_PATH "F:\\Qt\\6.7.0\\msvc2019_64")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_subdirectory(QFluent)
add_subdirectory(QFluentExample)
add_subdirectory(WireForgeMain)

# find_package(QFluent QUIET)
# if(QFluent_FOUND)
#     add_subdirectory(MinimalExample)
# endif()


