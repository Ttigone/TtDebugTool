set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_filename_component(src_loc src REALPATH)
get_filename_component(res_loc res REALPATH)


set(PROJECT_SOURCES
  src/main.cpp
  src/stdafx.h

  src/core/serial_port.h src/core/serial_port.cpp
  src/core/udp_server.h src/core/udp_server.cpp
  src/core/tcp_server.h src/core/tcp_server.cpp
  src/core/tcp_client.h src/core/tcp_client.cpp

  src/window/main_window.h src/window/main_window.cpp
  src/window/left_bar.h src/window/left_bar.cpp
  src/window/popup_window.h src/window/popup_window.cpp
  src/window/function_selection_window.h src/window/function_selection_window.cpp
  src/window/serial_window.h src/window/serial_window.cpp
  src/window/udp_window.h src/window/udp_window.cpp
  src/window/tcp_window.h src/window/tcp_window.cpp

  src/widget/serial_setting.h src/widget/serial_setting.cpp
  src/widget/serial_operation.h src/widget/serial_operation.cpp
  src/widget/shortcut_instruction.h src/widget/shortcut_instruction.cpp

  src/ui/widgets/text_widget.h src/ui/widgets/text_widget.cpp
  src/ui/widgets/pop_widget.h src/ui/widgets/pop_widget.cpp
)

set(RES_QRC
        # res/TtigoneChat.qrc
        res/windowbar/windowbar_icon.qrc
        res/sys-icon/sys-icon.qrc
        res/sys-icon/third/third.qrc
)

if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
            ${RES_QRC}
    )
else()
  add_executable(SerialportTool)
    target_sources(SerialportTool PRIVATE ${PROJECT_SOURCES} ${RES_QRC}
    )
endif ()



# 添加编译选项
add_compile_definitions(QT_DEPRECATED_WARNINGS)



add_subdirectory(lib_base)
add_subdirectory(lib_ui)

include_directories(src)
include_directories(${third_party})

target_link_libraries(SerialportTool 
PRIVATE
  desktop-app::lib_base
  desktop-app::lib_ui 
  QWindowKit::Widgets
  external_qtadvancedcss
  external_qt-material
  qscintilla2_qt
  QsLog
)

# 查看链接的库
get_target_property(LINK_LIBS SerialportTool LINK_LIBRARIES)
message(STATUS "Libraries linked to MyExecutable: ${LINK_LIBS}")

target_precompile_headers(SerialportTool PRIVATE $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:${src_loc}/stdafx.h>)

install(TARGETS SerialportTool DESTINATION bin)

