#
# 日志库
#

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 避免 moc 失败
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core)
include_directories(${Qt6Core_INCLUDE_DIRS})

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC 专用选项
    add_compile_options(/Zc:__cplusplus)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # MinGW (g++) 专用选项
    add_compile_options(-fpermissive)
endif()

# Setting up the target
add_library(QsLog SHARED
    ${third_party}/QsLog/QsLogDest.cpp
    ${third_party}/QsLog/QsLog.cpp
    ${third_party}/QsLog/QsLogDestConsole.cpp
    ${third_party}/QsLog/QsLogDestFile.cpp
    ${third_party}/QsLog/QsLogDestFunctor.cpp
)

target_link_libraries(QsLog Qt6::Core)

# Specify include directories
target_include_directories(QsLog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})



# Define preprocessor macros for Windows
if(WIN32)
    target_compile_definitions(QsLog PRIVATE QSLOG_IS_SHARED_LIBRARY)
endif()

# Installation settings for Unix (excluding macOS)
if(UNIX AND NOT APPLE)
    # Install headers
    install(FILES
        ${third_party}/QsLog/QsLog.h
        ${third_party}/QsLog/QsLogDest.h
        ${third_party}/QsLog/QsLogLevel.h
        ${third_party}/QsLog/QsLogDestConsole.h
        ${third_party}/QsLog/QsLogDestFile.h
        ${third_party}/QsLog/QsLogDisableForThisFile.h
        ${third_party}/QsLog/QsLogDestFunctor.h
        DESTINATION include/QsLog
    )

    # Install other files
    install(FILES
        ${third_party}/QsLog/QsLogChanges.txt
        ${third_party}/QsLog/QsLogReadme.txt
        ${third_party}/QsLog/LICENSE.txt
        DESTINATION share/QsLog
    )

    # Determine the installation path for the shared library
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64-bit system
        set(LIB_INSTALL_DIR lib64)
    else()
        # 32-bit system
        set(LIB_INSTALL_DIR lib)
    endif()

    install(TARGETS QsLog
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    )
endif()

# 设置 QsLog 动态库的输出路径
set_target_properties(QsLog PROPERTIES
    # LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
    # RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
    # ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# 确保所有配置的输出路径
foreach(CONFIG_TYPE IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
    set_target_properties(QsLog PROPERTIES
        # LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
        # RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
        # ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
        LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}
        ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}
    )
endforeach()
