# find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Network Svg REQUIRED)
# find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS Quick QuickWidgets QUIET)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS
    Core
    # Gui
    Widgets
    SerialPort
    SerialBus
    Qml
    Svg
    SvgWidgets
    StateMachine
    PrintSupport
    Concurrent
    OpenGL
    Core5Compat
    # Mqtt
)


find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Core
    # Gui
    Widgets
    SerialPort
    SerialBus
    Qml
    Svg
    SvgWidgets
    StateMachine
    PrintSupport
    Concurrent
    OpenGL
    Core5Compat
    # Mqtt
)

# # 验证找到的 Qt
# if(NOT Qt6_FOUND)
#     message(FATAL_ERROR "Qt6 not found. Please set CMAKE_PREFIX_PATH to your Qt installation.")
# endif()

# # 打印 Qt 版本信息
# message(STATUS "Found Qt ${Qt6_VERSION} at ${Qt6_DIR}")

if(NOT Qt6OpenGL_FOUND)
    message(FATAL_ERROR "Qt6 OpenGL module not found!")
endif()

# if (LINUX)
#     find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS DBus WaylandClient WaylandCompositor QUIET)
# endif()

set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "(gen)")
set_property(GLOBAL PROPERTY AUTOGEN_TARGETS_FOLDER "(gen)")
