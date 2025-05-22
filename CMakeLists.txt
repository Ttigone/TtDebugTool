set(Project_Langs C CXX)

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(TtDebugTool
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


if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # 使用 MSVC 版本的 Qt
    set(CMAKE_PREFIX_PATH "D:/qt/qt6/6.6.3/msvc2019_64")
    set(Qt6_DIR "D:/qt/qt6/6.6.3/msvc2019_64/lib/cmake/Qt6")
    # MSVC 专用选项
    add_compile_options(/Zc:__cplusplus /permissive- /utf-8 /EHsc)
    include_directories("D:/qt/qt6/6.6.3/msvc2019_64/include")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # 使用 MinGW 版本的 Qt
    set(CMAKE_PREFIX_PATH "D:/qt/qt6/6.6.3/mingw_64")
    set(QT6_DIR "D:/qt/qt6/6.6.3/mingw_64/lib/cmake/Qt6")
    include_directories("D:/qt/qt6/6.6.3/mingw_64/include")
    # 只设置 C++ 编译器标志
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
endif()


# if (MINGW)
#     set(CMAKE_PREFIX_PATH "D:/qt/qt6/6.6.3/mingw_64")
#     set(QT_DIR "D:/qt/qt6/6.6.3/mingw_64/lib/cmake/Qt6")
# elseif(MSVC)

# endif()


include(cmake/external/qt/package.cmake)

# set(desktop_app_skip_libs
#     glibmm
#     variant
# )

add_subdirectory(cmake)
add_subdirectory(DebugTool)
