# find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Network Svg REQUIRED)
# find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS Quick QuickWidgets QUIET)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS  Core Widgets SerialPort Qml Svg SvgWidgets StateMachine PrintSupport Concurrent Core5Compat)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS  Core  Widgets SerialPort Qml Svg SvgWidgets StateMachine PrintSupport Concurrent Core5Compat)

if (QT_VERSION_MAJOR GREATER_EQUAL 6)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS OpenGL OpenGLWidgets REQUIRED)
endif()

# if (LINUX)
#     find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS DBus WaylandClient WaylandCompositor QUIET)
# endif()

set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "(gen)")
set_property(GLOBAL PROPERTY AUTOGEN_TARGETS_FOLDER "(gen)")
