cmake_minimum_required(VERSION 3.10)
project(libmodbus-3.1.11 C)

set(CMAKE_C_STANDARD 99)

# set(
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/modbus-data.c
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/modbus-rtu.c
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/modbus-tcp.c
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/modbus.c
# )

file(GLOB LIBMODBUS_SOURCES CONFIGURE_DEPENDS
    "${third_party}/${PROJECT_NAME}/src/*.c"
)

# set(HEADERS
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/modbus-private.h
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/modbus-rtu-private.h
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/modbus-rtu.h
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/modbus-tcp-private.h
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/modbus-tcp.h
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/modbus-version.h
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/modbus.h
# )
file(GLOB LIBMODBUS_HEADERS CONFIGURE_DEPENDS
    "${third_party}/${PROJECT_NAME}/src/*.h"
    "${third_party}/${PROJECT_NAME}/src/win32/*.h"
)

# 创建库
add_library(modbus STATIC ${LIBMODBUS_SOURCES} ${LIBMODBUS_HEADERS})

# 包含目录
# target_include_directories(modbus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(modbus PUBLIC ${third_party}/${PROJECT_NAME}/src)
target_include_directories(modbus PUBLIC ${third_party}/${PROJECT_NAME}/src/win32)

# Windows平台特定配置
if(WIN32)
    target_link_libraries(modbus ws2_32)
endif()

# 可选：安装规则
install(TARGETS modbus
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

# install(FILES 
#         ${CMAKE_CURRENT_SOURCE_DIR}/modbus.h
#         ${CMAKE_CURRENT_SOURCE_DIR}/modbus-rtu.h
#         ${CMAKE_CURRENT_SOURCE_DIR}/modbus-tcp.h
#         ${CMAKE_CURRENT_SOURCE_DIR}/modbus-version.h
#         DESTINATION include)
# install(FILES 
#         ${CMAKE_CURRENT_SOURCE_DIR}/modbus.h
#         ${CMAKE_CURRENT_SOURCE_DIR}/modbus-rtu.h
#         ${CMAKE_CURRENT_SOURCE_DIR}/modbus-tcp.h
#         ${CMAKE_CURRENT_SOURCE_DIR}/modbus-version.h
#         DESTINATION include)
