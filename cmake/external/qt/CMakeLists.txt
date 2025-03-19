add_library(external_qt INTERFACE IMPORTED GLOBAL)


target_link_libraries(external_qt
    INTERFACE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::WidgetsPrivate
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::SerialBus
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::SvgWidgets
    Qt${QT_VERSION_MAJOR}::StateMachine
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Core5Compat
    # Qt${QT_VERSION_MAJOR}::Mqtt
    Qt${QT_VERSION_MAJOR}::OpenGL
)

target_include_directories(external_qt SYSTEM
INTERFACE
)
