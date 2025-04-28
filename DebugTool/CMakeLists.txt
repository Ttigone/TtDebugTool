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

set(PROJECT_SOURCES
    ${src_loc}/main.cpp
    ${src_loc}/pch.h
    ${src_loc}/Def.h

    ${src_loc}/core/serial_port.h ${src_loc}/core/serial_port.cpp
    ${src_loc}/core/serial_helper.h ${src_loc}/core/serial_helper.cpp
    ${src_loc}/core/udp_server.h ${src_loc}/core/udp_server.cpp
    ${src_loc}/core/tcp_server.h ${src_loc}/core/tcp_server.cpp
    ${src_loc}/core/tcp_client.h ${src_loc}/core/tcp_client.cpp
    ${src_loc}/core/udp_client.h ${src_loc}/core/udp_client.cpp
    ${src_loc}/core/mqtt_client.h ${src_loc}/core/mqtt_client.cpp
    ${src_loc}/core/modbus_client.h ${src_loc}/core/modbus_client.cpp
    ${src_loc}/core/modbus_server.h ${src_loc}/core/modbus_server.cpp
    ${src_loc}/core/download.h ${src_loc}/core/download.cpp
    ${src_loc}/core/lua_kernel.h ${src_loc}/core/lua_kernel.cpp

    # src/lang/ ${src_loc}/core/lua_kernel.cpp
    src/lang/translation_manager.h src/lang/translation_manager.cpp

    ${src_loc}/window/frame_window.h ${src_loc}/window/frame_window.cpp
    ${src_loc}/window/main_window.h ${src_loc}/window/main_window.cpp
    ${src_loc}/window/left_bar.h ${src_loc}/window/left_bar.cpp
    ${src_loc}/window/popup_window.h ${src_loc}/window/popup_window.cpp
    ${src_loc}/window/function_selection_window.h ${src_loc}/window/function_selection_window.cpp
    ${src_loc}/window/serial_window.h ${src_loc}/window/serial_window.cpp
    ${src_loc}/window/udp_window.h ${src_loc}/window/udp_window.cpp
    ${src_loc}/window/tcp_window.h ${src_loc}/window/tcp_window.cpp
    ${src_loc}/window/mqtt_window.h ${src_loc}/window/mqtt_window.cpp
    ${src_loc}/window/instruction_window.h ${src_loc}/window/instruction_window.cpp
    ${src_loc}/window/modbus_window.h ${src_loc}/window/modbus_window.cpp

    ${src_loc}/widget/serial_setting.h ${src_loc}/widget/serial_setting.cpp
    ${src_loc}/widget/tcp_setting.h ${src_loc}/widget/tcp_setting.cpp
    ${src_loc}/widget/udp_setting.h ${src_loc}/widget/udp_setting.cpp
    ${src_loc}/widget/mqtt_setting.h ${src_loc}/widget/mqtt_setting.cpp
    ${src_loc}/widget/subscripition_widget.h ${src_loc}/widget/subscripition_widget.cpp
    ${src_loc}/widget/modbus_setting.h ${src_loc}/widget/modbus_setting.cpp
    ${src_loc}/widget/mqtt_meta_setting_widget.h ${src_loc}/widget/mqtt_meta_setting_widget.cpp

    ${src_loc}/storage/setting_manager.h ${src_loc}/storage/setting_manager.cpp
    ${src_loc}/storage/csv_handler.h ${src_loc}/storage/csv_handler.cpp
    ${src_loc}/storage/configs_manager.h ${src_loc}/storage/configs_manager.cpp

    ${src_loc}/ui/widgets/text_widget.h ${src_loc}/ui/widgets/text_widget.cpp
    ${src_loc}/ui/widgets/pop_widget.h ${src_loc}/ui/widgets/pop_widget.cpp
    ${src_loc}/ui/widgets/window_switcher.h ${src_loc}/ui/widgets/window_switcher.cpp
    ${src_loc}/ui/widgets/session_manager.h ${src_loc}/ui/widgets/session_manager.cpp
    # ${src_loc}/ui/widgets/data_presentation_widget.h ${src_loc}/ui/widgets/data_presentation_widget.cpp
    ${src_loc}/ui/widgets/subscripition_manager.h ${src_loc}/ui/widgets/subscripition_manager.cpp
    ${src_loc}/ui/widgets/setting_widget.h ${src_loc}/ui/widgets/setting_widget.cpp


    ${src_loc}/ui/controls/TtQCustomPlot.h ${src_loc}/ui/controls/TtQCustomPlot.cpp
    ${src_loc}/ui/controls/TtPlotItem.h ${src_loc}/ui/controls/TtPlotItem.cpp
    ${src_loc}/ui/controls/TtModbusDelegate.h ${src_loc}/ui/controls/TtModbusDelegate.cpp
    ${src_loc}/ui/controls/TtTableView.h ${src_loc}/ui/controls/TtTableView.cpp
    ${src_loc}/ui/controls/TtModbusPlot.h ${src_loc}/ui/controls/TtModbusPlot.cpp
    ${src_loc}/ui/controls/TtDownloadDialog.h ${src_loc}/ui/controls/TtDownloadDialog.cpp
    ${src_loc}/ui/controls/TtSerialLexer.h ${src_loc}/ui/controls/TtSerialLexer.cpp
    ${src_loc}/ui/controls/TtQCPItemRichText.h ${src_loc}/ui/controls/TtQCPItemRichText.cpp
    ${src_loc}/ui/controls/TtQCPItemRichText.h ${src_loc}/ui/controls/TtQCPItemRichText.cpp
    ${src_loc}/ui/controls/TtLuaInputBox.h ${src_loc}/ui/controls/TtLuaInputBox.cpp
    ${src_loc}/ui/controls/TtChannelButton.h ${src_loc}/ui/controls/TtChannelButton.cpp
    ${src_loc}/ui/controls/TtChannelButtonEditorDialog.h ${src_loc}/ui/controls/TtChannelButtonEditorDialog.cpp
    ${src_loc}/ui/controls/TtSerialPortPlot.h ${src_loc}/ui/controls/TtSerialPortPlot.cpp
)

# Apply the macro
absolute_path_list(PROJECT_SOURCES)


FILE(GLOB PROJECT_HEADER_FILES
    ${third_party}/qcustomplot/*.h
)

FILE(GLOB PROJECT_SOURCE_FILES
    ${third_party}/qcustomplot/*.cpp
)

absolute_path_list(PROJECT_HEADER_FILES)
absolute_path_list(PROJECT_SOURCE_FILES)

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
            ${PROJECT_SOURCES}
            ${RES_QRC}
            ${PROJECT_HEADER_FILES}
            ${PROJECT_SOURCE_FILES}
    )
else()
  add_executable(${PROJECT_NAME})
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

if(MINGW)
    list(APPEND CMAKE_PREFIX_PATH "{third_party}/qtmqtt_mingw64/lib/cmake")
    find_package(Qt6Mqtt REQUIRED)
    target_link_directories(${PROJECT_NAME} PRIVATE "${third_party}/qtmqtt_mingw64/lib")
endif()


find_package(Qt6 REQUIRED COMPONENTS OpenGL)
find_package(OpenGL REQUIRED)


target_link_libraries(${PROJECT_NAME}
PRIVATE
  desktop-app::lib_base
  lib_ui
  # desktop-app::lib_ui
  QWindowKit::Widgets
  external_qtadvancedcss
  external_qt-material
  qscintilla2_qt
  QsLog
  qt-easy-logger
  # qcustomplot
  ${third_party}/qtmqtt_mingw64/lib/libQt6Mqtt.a
  # 使用的是系统的 opengl函数
  # -lopengl32 -lglu32
  Qt6::OpenGL          # Qt OpenGL 模块
  OpenGL::GL           # 系统OpenGL
  OpenGL::GLU          # 系统GLU（可选
  lua5.4.7
)
# 查看链接的库
get_target_property(LINK_LIBS ${PROJECT_NAME} LINK_LIBRARIES)
message(STATUS "Libraries linked to MyExecutable: ${LINK_LIBS}")

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

