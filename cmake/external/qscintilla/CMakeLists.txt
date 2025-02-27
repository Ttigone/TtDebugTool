cmake_minimum_required(VERSION 3.10)
# 项目名称和版本
project(QScintilla VERSION 2.14.1 LANGUAGES CXX)

# 设置目标名称
set(TARGET_NAME qscintilla2_qt${QT_MAJOR_VERSION})

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用自动处理 Qt 的 MOC、UIC 和 RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 设置 qt-material 源代码位置
set(qscintilla_loc ${third_party}/QScintilla_src-2.14.1/src)
set(scintilla_loc ${third_party}/QScintilla_src-2.14.1/scintilla)

# 添加定义
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DSCINTILLA_QT -DSCI_LEXER -DINCLUDE_DEPRECATED_FEATURES)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC 专用选项
    add_compile_options(/Zc:__cplusplus)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # MinGW (g++) 专用选项
    add_compile_options(-fpermissive)
endif()


# 源文件和头文件
file(GLOB_RECURSE PROJECT_SRC
    ${qscintilla_loc}/*.cpp
    ${scintilla_loc}/src/*.cpp
    ${scintilla_loc}/lexlib/*.cpp
    ${scintilla_loc}/lexers/*.cpp
)

file(GLOB_RECURSE PROJECT_INC
    ${qscintilla_loc}/*.h
    ${scintilla_loc}/include/*.h
    ${scintilla_loc}/lexlib/*.h
    ${scintilla_loc}/src/*.h
)

# 创建库
# add_library(${TARGET_NAME} SHARED ${PROJECT_SRC} ${PROJECT_INC})
# MSVC 仅支持 STATIC
add_library(${TARGET_NAME} STATIC ${PROJECT_SRC} ${PROJECT_INC})

# 包含目录 文件各自包含
target_include_directories(${TARGET_NAME}
    PUBLIC
    ${qscintilla_loc}
    ${qscintilla_loc}/Qsci
    ${scintilla_loc}/include
    ${scintilla_loc}/lexlib
    ${scintilla_loc}/src
)


target_link_libraries(${TARGET_NAME} Qt6::Core Qt6::Widgets Qt6::PrintSupport)

# # 链接 Qt 库
# target_link_libraries(${TARGET_NAME}
#     PRIVATE
#     external_qt
# )

# 设置编译选项
if(MSVC)
    target_compile_options(${TARGET_NAME} PRIVATE /W0)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${TARGET_NAME} PRIVATE -w)
endif()

# 设置目标属性
set_target_properties(${TARGET_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED TRUE
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# 安装规则
install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/Qsci
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(FILES ${CMAKE_SOURCE_DIR}/qscintilla_*.qm
    DESTINATION ${CMAKE_INSTALL_DATADIR}/translations
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/../qsci
    DESTINATION ${CMAKE_INSTALL_DATADIR}
)

install(FILES ${CMAKE_SOURCE_DIR}/features_staticlib/qscintilla2.prf
    DESTINATION ${CMAKE_INSTALL_DATADIR}/mkspecs/features
    CONFIGURATIONS Debug
)

install(FILES ${CMAKE_SOURCE_DIR}/features/qscintilla2.prf
    DESTINATION ${CMAKE_INSTALL_DATADIR}/mkspecs/features
    CONFIGURATIONS Release
)
