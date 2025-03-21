cmake_minimum_required(VERSION 3.5)

project(qt-easy-logger VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Release 模式下也输出文件、函数
add_compile_definitions(QT_MESSAGELOGCONTEXT)

set(LIBRARY_SOURCES
    logger.h logger.cpp
    logview.h logview.cpp
)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_library(qt-easy-logger SHARED)

target_sources(${PROJECT_NAME} PRIVATE ${LIBRARY_SOURCE})

target_link_libraries(qt-easy-logger PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# set(SAMPLE_SOURCES
#     main.cpp
#     samplewindow.cpp
#     samplewindow.h
# )

# if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
#     qt_add_executable(qt-easy-logger-sample
#         MANUAL_FINALIZATION
#         ${SAMPLE_SOURCES}
#     )
# else()
#     add_executable(qt-easy-logger-sample
#         ${SAMPLE_SOURCES}
#     )
# endif()

# target_link_libraries(qt-easy-logger-sample PRIVATE qt-easy-logger)
# target_link_libraries(qt-easy-logger-sample PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# if(QT_VERSION_MAJOR EQUAL 6)
#     qt_finalize_executable(qt-easy-logger-sample)
# endif()
