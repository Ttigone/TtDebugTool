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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置可执行文件的输出目录为构建目录的根目录
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

get_filename_component(src_loc src REALPATH)
get_filename_component(res_loc res REALPATH)


macro(absolute_path_list VAR)
    set(ABSOLUTE_LIST "")
    foreach(FILE_PATH ${${VAR}})
        if(NOT IS_ABSOLUTE "${FILE_PATH}")
            get_filename_component(ABS_PATH "${FILE_PATH}" ABSOLUTE)
            list(APPEND ABSOLUTE_LIST "${ABS_PATH}")
        else()
            list(APPEND ABSOLUTE_LIST "${FILE_PATH}")
        endif()
    endforeach()
    set(${VAR} ${ABSOLUTE_LIST} CACHE INTERNAL "")
endmacro()

# 自动查找所有源文件和头文件
file(GLOB_RECURSE PROJECT_SOURCES
    "${src_loc}/*.cpp"
    "${src_loc}/*.h"
)


FILE(GLOB PROJECT_HEADER_FILES
    ${third_party}/qcustomplot/*.h
)

FILE(GLOB PROJECT_SOURCE_FILES
    ${third_party}/qcustomplot/*.cpp
)

set(RES_QRC
    res/shared.qrc
    res/sys-icon/sys-icon.qrc
    res/sys-icon/third/third.qrc
    res/sys-gif/sys-gif.qrc
    res/font/font.qrc
    res/styles/theme.qrc
)

# add_compile_options(-fpermissive)  # 全局启用宽松模式

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")


if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
            MANUAL_FINALIZATION
            WIN32
            ${PROJECT_SOURCES}
            ${RES_QRC}
            ${PROJECT_HEADER_FILES}
            ${PROJECT_SOURCE_FILES}
    )
else()
    add_executable(${PROJECT_NAME} WIN32)
    target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES} ${RES_QRC})
endif ()

# # 设置编译选项
# if(MSVC)
#     add_compile_options(/Zc:__cplusplus)  # MSVC 格式
# elseif(MINGW)
#     # MinGW 不需要此选项
# endif()


add_definitions(-DQCUSTOMPLOT_USE_OPENGL)

if(WIN32)

if(MINGW)
    # 添加调试信息
    set(CMAKE_BUILD_TYPE Debug) # 确保构建类型为 Debug
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g") # 添加调试标志
    target_link_libraries(${PROJECT_NAME} PRIVATE
        -lDbgHelp
        -lWininet
    )
elseif(MSVC)
    add_compile_options(/Zi)
    add_link_options(/DEBUG)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        dbghelp
        wininet
    )
endif()


endif()


# 添加编译选项
add_compile_definitions(QT_DEPRECATED_WARNINGS)
if(MINGW)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wa,-mbig-obj
    )
endif()


add_subdirectory(lib_base)
add_subdirectory(lib_ui)

include_directories(src)
include_directories(${third_party})


if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_include_directories(${PROJECT_NAME} PRIVATE "${third_party}/qtmqtt/6_6_3_msvc2019_64/include")
    target_link_directories(${PROJECT_NAME} PRIVATE "${third_party}/qtmqtt/6_6_3_msvc2019_64/lib")
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6Mqtt)
    message("TEST MSVC")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_include_directories(${PROJECT_NAME} PRIVATE "${third_party}/qtmqtt/6_6_3_mingw_64/include")
    target_link_directories(${PROJECT_NAME} PRIVATE "${third_party}/qtmqtt/6_6_3_mingw_64/lib")
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6Mqtt)
    message("TEST MINGW")
endif()


find_package(Qt6 REQUIRED COMPONENTS OpenGL)
find_package(OpenGL REQUIRED)


target_link_libraries(${PROJECT_NAME}
PRIVATE
  desktop-app::lib_base
  lib_ui
  QWindowKit::Widgets
  external_qtadvancedcss
  external_qt-material
  qscintilla2_qt
  QsLog
  qt-easy-logger
  # qcustomplot
#  ${third_party}/qtmqtt_mingw64/lib/libQt6Mqtt.a
  # 使用的是系统的 opengl函数
  # -lopengl32 -lglu32
  Qt6::OpenGL          # Qt OpenGL 模块
  OpenGL::GL           # 系统OpenGL
  OpenGL::GLU          # 系统GLU（可选
  lua5.4.7
  # modbus
)
# 查看链接的库
get_target_property(LINK_LIBS ${PROJECT_NAME} LINK_LIBRARIES)
message(STATUS "Libraries linked to MyExecutable: ${LINK_LIBS}")

set_target_properties(${PROJECT_NAME} PROPERTIES
        QT_QMAKE_EXECUTABLE_CONFIG resources_big
)

target_precompile_headers(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:${src_loc}/pch.h>)

# install(TARGETS SerialportTool DESTINATION bin)

find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

SET(TS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/res/language/${PROJECT_NAME}_en.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/res/language/${PROJECT_NAME}_zh.ts"
)

get_target_property(LUPDATE_EXECUTABLE Qt6::lupdate IMPORTED_LOCATION)
message(STATUS "Found lupdate at: ${LUPDATE_EXECUTABLE}")


add_custom_target(update_translations
    COMMAND "${LUPDATE_EXECUTABLE}" -recursive "${CMAKE_CURRENT_SOURCE_DIR}/src" -ts ${TS_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Updating translation files with lupdate"
    VERBATIM
)

add_dependencies(${PROJECT_NAME} update_translations)

# # 1. 使用lupdate从源代码提取字符串到.ts文件
# qt_add_lupdate(${PROJECT_NAME}
#     TS_FILES ${TS_FILES}
#     SOURCES ${ALL_SOURCE_FILES}
# )

# # 2. 使用lrelease编译.ts文件为.qm文件
# qt_add_lrelease(${PROJECT_NAME}
#     TS_FILES ${TS_FILES}
#     QM_FILES_OUTPUT_VARIABLE QM_FILES
# )

# # 3. 将.qm文件添加为Qt资源
# qt_add_resources(${PROJECT_NAME} "translations"
#     PREFIX "/"
#     BASE "${CMAKE_CURRENT_BINARY_DIR}"
#     FILES ${QM_FILES}
# )


# ----------------------------------------------------------------
# Set Icon of Window
# ----------------------------------------------------------------
function(add_win_rc _target)
    set(RC_VERSION "0.0.0.0")
    set(RC_VERSION_STRING "0.0.0.0")

    set(RC_DESCRIPTION ${_target})
    set(RC_COPYRIGHT ${_target})
    set(RC_APPLICATION_NAME ${_target})

    get_filename_component(RC_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/res/app/${PROJECT_NAME}.ico" ABSOLUTE)


    set(_out_path "${CMAKE_CURRENT_BINARY_DIR}/${_target}_res.rc")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/res/cmake/WinResource.rc.in" ${_out_path} @ONLY)
    target_sources(${_target} PRIVATE ${_out_path})

endfunction()
# execute function
add_win_rc(${PROJECT_NAME})

# set app version'file
function(add_win_manifest _target)
    set(_name ${_target})
    set(_version "0.0.0.0")
    set(_desc ${_name})

    set(MANIFEST_IDENTIFIER ${_name})
    set(MANIFEST_VERSION ${_version})
    set(MANIFEST_DESCRIPTION ${_desc})
    set(MANIFEST_UTF8)
    set(_out_path "${CMAKE_CURRENT_BINARY_DIR}/${_target}_manifest.manifest")
    # message(_out_path)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/res/cmake/WinManifest.manifest.in" ${_out_path} @ONLY)
    target_sources(${_target} PRIVATE ${_out_path})
endfunction()
add_win_manifest(${PROJECT_NAME})

