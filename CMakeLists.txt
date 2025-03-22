set(Project_Langs C CXX)

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(DebugTool
    LANGUAGES ${Project_Langs}
    DESCRIPTION "Serial port debugging tool"
    HOMEPAGE_URL "none"
)

get_filename_component(third_party "DebugTool/3rdparty" REALPATH)

# include(cmake/version.cmake)

# 自定义的 cmake 函数文件  全局的 CMakeLists.txt 都可以使用
# include(cmake/init_target.cmake)
# include(cmake/nice_target_sources.cmake)

# message(STATUS "Using compiler: ${CMAKE_CXX_COMPILER_ID} version: ${CMAKE_CXX_COMPILER_VERSION}")

include(cmake/external/qt/package.cmake)

# set(desktop_app_skip_libs
#     glibmm
#     variant
# )


add_subdirectory(cmake)
add_subdirectory(DebugTool)
