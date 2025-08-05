project(realtimeplot LANGUAGES CXX VERSION 1.0 )
set(TARGET_NAME external_realtimeplot)

set(realtimeplot_loc ${third_party}/realtimeplot)


file(GLOB_RECURSE PROJECT_SRC 
	${realtimeplot_loc}/*.cpp
)

file(GLOB_RECURSE PROJECT_INC
	${realtimeplot_loc}/*.h
)

file(GLOB_RECURSE PROJECT_UI
	${realtimeplot_loc}/*.ui
)

add_library(${TARGET_NAME} ${PROJECT_SRC} ${PROJECT_INC} ${PROJECT_UI})

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS
    Widgets
)



target_link_libraries(${TARGET_NAME} external_qcustomplot cpp_ringbuffer Qt${QT_VERSION_MAJOR}::Widgets)
