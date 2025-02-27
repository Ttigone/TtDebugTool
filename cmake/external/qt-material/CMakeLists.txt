# 项目名称和版本
project(QtMaterialWidgets VERSION 1.0 LANGUAGES CXX)

# 设置目标名称
set(TARGET_NAME external_qt-material)

# 启用自动处理 Qt 的 UIC、MOC 和 RCC
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC 专用选项
    add_compile_options(/Zc:__cplusplus)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # MinGW (g++) 专用选项
    add_compile_options(-fpermissive)
endif()

# 如果要生成共享库还是静态库
option(QtMaterial_BUILD_STATIC "Build as static library" ON)
if(QtMaterial_BUILD_STATIC)
    add_library(${TARGET_NAME} STATIC)
else()
    add_library(${TARGET_NAME} SHARED)
endif()

# 设置输出路径
set(QtMaterial_OUT_ROOT "${CMAKE_BINARY_DIR}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${QtMaterial_OUT_ROOT}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QtMaterial_OUT_ROOT}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QtMaterial_OUT_ROOT}/bin)

# 设置 qt-material 源代码位置
set(qt_material_loc ${third_party}/qt-material-widgets/components)

# 源文件
file(GLOB_RECURSE PROJECT_SRC
    ${qt_material_loc}/*.cpp
)

# 头文件
file(GLOB_RECURSE PROJECT_INC
    ${qt_material_loc}/*.h
)

# 包含目录
target_include_directories(${TARGET_NAME}
    PUBLIC
    ${qt_material_loc}
)

# 添加源文件和头文件到目标
target_sources(${TARGET_NAME}
    PRIVATE
    ${PROJECT_SRC}
    ${PROJECT_INC}
)

# 链接库
target_link_libraries(${TARGET_NAME}
    PRIVATE
    external_qt
)

# 设置目标属性
set_target_properties(${TARGET_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED TRUE
)

# 安装规则（可选）
install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION ${QtMaterial_OUT_ROOT}/lib
    LIBRARY DESTINATION ${QtMaterial_OUT_ROOT}/lib
    RUNTIME DESTINATION ${QtMaterial_OUT_ROOT}/bin
)
