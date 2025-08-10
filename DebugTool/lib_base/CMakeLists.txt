add_library(lib_base STATIC
)
add_library(desktop-app::lib_base ALIAS lib_base)

set(CMAKE_AUTOMOC ON)

get_filename_component(src_loc . REALPATH)

set(LIB_BASE_OUT_ROOT "${CMAKE_BINARY_DIR}/${PROJECT_NAME}")
#set(LIB_BASE_OUT_ROOT ${CMAKE_BINARY_DIR})
# 设置库导出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_BASE_OUT_ROOT})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_BASE_OUT_ROOT})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LIB_BASE_OUT_ROOT})

target_precompile_headers(lib_base
PRIVATE 
  $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:${src_loc}/base/basic_pch.h>
)

target_sources(lib_base 
PRIVATE
  base/basic_pch.h
  base/basic_types.h
  base/flags.h
  base/time.h
  base/timer.h base/timer.cpp
  base/invoke_queued.h
  base/glog_helper.h
  base/wchar_logging.h
  base/encode_helper.h
)

# 省略相对路径的麻烦
target_include_directories(lib_base
PUBLIC
  ${src_loc}
  ${third_party}/glog
)


target_include_directories(external_qt SYSTEM
INTERFACE
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)


target_link_libraries(lib_base
PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    glog
    glog_internal
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()
