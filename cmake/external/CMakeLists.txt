macro(add_checked_subdirectory name)
    if (NOT DEFINED desktop_app_skip_libs
        OR NOT ${name} IN_LIST desktop_app_skip_libs)
        add_subdirectory(${name})
    endif()
endmacro()

add_checked_subdirectory(qt)
add_checked_subdirectory(qwindowkit)
add_checked_subdirectory(Qt-Advanced-Stylesheets)
add_checked_subdirectory(qt-material)
add_checked_subdirectory(qscintilla)
add_checked_subdirectory(QsLog)
add_checked_subdirectory(qt-easy-logger-main)
add_checked_subdirectory(lua)
<<<<<<< Updated upstream
add_checked_subdirectory(qcustomplot)
add_checked_subdirectory(realTimePlot)
add_checked_subdirectory(ringbuffer)
=======
add_checked_subdirectory(realtimeplot)
>>>>>>> Stashed changes
# add_checked_subdirectory(libmodbus-3.1.11)

