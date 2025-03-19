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


set(PROJECT_SOURCES
    src/main.cpp
    src/pch.h
    src/Def.h

    src/core/serial_port.h src/core/serial_port.cpp
    src/core/udp_server.h src/core/udp_server.cpp
    src/core/tcp_server.h src/core/tcp_server.cpp
    src/core/tcp_client.h src/core/tcp_client.cpp
    src/core/udp_client.h src/core/udp_client.cpp
    src/core/mqtt_client.h src/core/mqtt_client.cpp
    src/core/modbus_client.h src/core/modbus_client.cpp
    src/core/modbus_server.h src/core/modbus_server.cpp

    src/window/main_window.h src/window/main_window.cpp
    src/window/left_bar.h src/window/left_bar.cpp
    src/window/popup_window.h src/window/popup_window.cpp
    src/window/function_selection_window.h src/window/function_selection_window.cpp
    src/window/serial_window.h src/window/serial_window.cpp
    src/window/udp_window.h src/window/udp_window.cpp
    src/window/tcp_window.h src/window/tcp_window.cpp
    src/window/mqtt_window.h src/window/mqtt_window.cpp
    src/window/instruction_window.h src/window/instruction_window.cpp
    src/window/modbus_window.h src/window/modbus_window.cpp

    src/widget/serial_setting.h src/widget/serial_setting.cpp
    src/widget/tcp_setting.h src/widget/tcp_setting.cpp
    src/widget/udp_setting.h src/widget/udp_setting.cpp
    src/widget/shortcut_instruction.h src/widget/shortcut_instruction.cpp
    src/widget/mqtt_setting.h src/widget/mqtt_setting.cpp
    src/widget/subscripition_widget.h src/widget/subscripition_widget.cpp
    src/widget/modbus_setting.h src/widget/modbus_setting.cpp
    src/widget/mqtt_meta_setting_widget.h src/widget/mqtt_meta_setting_widget.cpp

    src/storage/setting_manager.h src/storage/setting_manager.cpp

    src/ui/widgets/text_widget.h src/ui/widgets/text_widget.cpp
    src/ui/widgets/pop_widget.h src/ui/widgets/pop_widget.cpp
    src/ui/widgets/window_switcher.h src/ui/widgets/window_switcher.cpp
    src/ui/widgets/session_manager.h src/ui/widgets/session_manager.cpp
    # src/ui/widgets/data_presentation_widget.h src/ui/widgets/data_presentation_widget.cpp
    src/ui/widgets/subscripition_manager.h src/ui/widgets/subscripition_manager.cpp


    src/ui/controls/TtQCustomPlot.h src/ui/controls/TtQCustomPlot.cpp
    src/ui/controls/TtTracer.h src/ui/controls/TtTracer.cpp
    src/ui/controls/TtModbusDelegate.h src/ui/controls/TtModbusDelegate.cpp
    src/ui/controls/TtTableView.h src/ui/controls/TtTableView.cpp
)


FILE(GLOB PROJECT_HEADER_FILES
    ${third_party}/qcustomplot/*.h
)

FILE(GLOB PROJECT_SOURCE_FILES
    ${third_party}/qcustomplot/*.cpp
)


set(RES_QRC
    res/windowbar/windowbar_icon.qrc
    res/sys-icon/sys-icon.qrc
    res/sys-icon/third/third.qrc
    res/sys-gif/sys-gif.qrc
    res/font/font.qrc
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

find_package(Qt6 REQUIRED COMPONENTS LinguistTools)


qt_add_lupdate(${PROJECT_NAME} TS_FILES  TestHello.ts)
qt_add_lrelease(${PROJECT_NAME}
    TS_FILES TestHello.ts
    QM_FILES_OUTPUT_VARIABLE qm_files)
qt_add_resources(${PROJECT_NAME} "translations"
    PREFIX "/"
    BASE "${CMAKE_CURRENT_BINARY_DIR}"
    FILES "${qm_files}")

add_definitions(-DQCUSTOMPLOT_USE_OPENGL)


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

# list(APPEND CMAKE_PREFIX_PATH "{third_party}/qtmqtt/lib/cmake")

# find_package(Qt6Mqtt REQUIRED)
link_directories(${third_party}/qtmqtt/lib)

find_package(Qt6 REQUIRED COMPONENTS OpenGL)
find_package(OpenGL REQUIRED)


target_link_libraries(SerialportTool 
PRIVATE
  desktop-app::lib_base
  desktop-app::lib_ui
  QWindowKit::Widgets
  external_qtadvancedcss
  external_qt-material
  qscintilla2_qt
  QsLog
  qt-easy-logger
  # qcustomplot
  ${third_party}/qtmqtt/lib/libQt6Mqtt.a
  # 使用的是系统的 opengl函数
  # -lopengl32 -lglu32
  Qt6::OpenGL          # Qt OpenGL 模块
  OpenGL::GL           # 系统OpenGL
  OpenGL::GLU          # 系统GLU（可选
)
# 查看链接的库
get_target_property(LINK_LIBS SerialportTool LINK_LIBRARIES)
message(STATUS "Libraries linked to MyExecutable: ${LINK_LIBS}")

target_precompile_headers(SerialportTool PRIVATE $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:${src_loc}/pch.h>)

# install(TARGETS SerialportTool DESTINATION bin)

