cmake_minimum_required(VERSION 3.5)
project(qcustomplot VERSION 1.0 LANGUAGES CXX)
set(TARGET_NAME external_qcustomplot)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(qcustomplot_loc ${third_party}/qcustomplot)

# 设置输出路径
set(qcustomplot_OUT_ROOT "${CMAKE_BINARY_DIR}")

set(PROJECT_INC
    ${qcustomplot_loc}/qcustomplot.h
)

set(PROJECT_SRC
    ${qcustomplot_loc}/qcustomplot.cpp
)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_library(${PROJECT_NAME} STATIC
        ${PROJECT_INC}
        ${PROJECT_SRC}
)

# add_definitions(-DQCUSTOMPLOT_USE_OPENGL)
# 添加编译定义（等价于 .pro 文件的 DEFINES += QCUSTOMPLOT_USE_OPENGL）
# target_compile_definitions(${PROJECT_NAME} PRIVATE QCUSTOMPLOT_USE_OPENGL)


if(MINGW)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wa,-mbig-obj
    )
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${third_party}/${PROJECT_NAME})

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS
    Widgets
    PrintSupport
    OpenGL
)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Widgets
    PrintSupport
    OpenGL
)

find_package(OpenGL REQUIRED)

if(MINGW)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::PrintSupport
        Qt${QT_VERSION_MAJOR}::OpenGL
        -lopengl32 -lglu32
    )
endif()
