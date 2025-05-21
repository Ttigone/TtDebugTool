#project(lua5.4.7 VERSION 0.1)

set(LIBRARY_NAME "lua5.4.7")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB LUA_SOURCES CONFIGURE_DEPENDS 
    "${third_party}/lua-5.4.7/src/*.c"
)

file(GLOB LUA_HEADERS CONFIGURE_DEPENDS 
    "${third_party}/lua-5.4.7/src/*.h"
)

# TODO 动态编译, 没有 lib
# Setting up the target
# add_library(${LIBRARY_NAME} SHARED
add_library(${LIBRARY_NAME} STATIC
    ${LUA_SOURCES}
    ${LUA_HEADERS}
)

# set_target_properties(${PROJECT_NAME} PROPERTIES
#     LINKER_LANGUAGE C  # 指定使用C语言链接器
# )

# 本 CMakeListst.txt 与 源文件在同一地方
# target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


# Installation settings for Unix (excluding macOS)
if(UNIX AND NOT APPLE)
    # Install headers
    install(FILES
        ${LUA_HEADERS}
        DESTINATION include/lua-5.4.7
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

# 设置动态库的输出路径
set_target_properties(${LIBRARY_NAME} PROPERTIES
     LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
     ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
#    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
#    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
#    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# 确保所有配置的输出路径
foreach(CONFIG_TYPE IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
    set_target_properties(${LIBRARY_NAME} PROPERTIES
         LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
         RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
         ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
#        LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}
#        RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}
#        ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}
    )
endforeach()
