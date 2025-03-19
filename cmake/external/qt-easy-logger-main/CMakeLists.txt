# add_subdirectory(${third_party}/qt-easy-logger-main)
cmake_minimum_required(VERSION 3.5)

project(qt-easy-logger VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出路径
set(qt-easy-logger-main_OUT_ROOT "${CMAKE_BINARY_DIR}")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Release 模式下也输出文件、函数
add_compile_definitions(QT_MESSAGELOGCONTEXT)


set(LIBRARY_SOURCES
    ${third_party}/qt-easy-logger-main/logger.h
    ${third_party}/qt-easy-logger-main/logger.cpp
    ${third_party}/qt-easy-logger-main/logview.h
    ${third_party}/qt-easy-logger-main/logview.cpp
)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_library(qt-easy-logger STATIC
    ${third_party}/qt-easy-logger-main/logger.h
    ${third_party}/qt-easy-logger-main/logger.cpp
    ${third_party}/qt-easy-logger-main/logview.h
    ${third_party}/qt-easy-logger-main/logview.cpp
)


target_link_libraries(qt-easy-logger PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_include_directories(qt-easy-logger PUBLIC ${third_party}/qt-easy-logger-main)


# 安装规则（可选）
install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION ${qt-easy-logger-main_OUT_ROOT}/lib
    LIBRARY DESTINATION ${qt-easy-logger-main_OUT_ROOT}}/lib
    RUNTIME DESTINATION ${qt-easy-logger-main_OUT_ROOT}}/bin
)
