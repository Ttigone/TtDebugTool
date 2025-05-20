# 配置调试和发布版本同时编译
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# 设置目标库名称
set(TARGET_NAME external_qtadvancedcss)


set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


# 设置文件路径
set(src_loc ${third_party}/Qt-Advanced-Stylesheets/src)

# message(${PROJECT_NAME})

# 设置输出路径
set(ACSS_OUT_ROOT "${CMAKE_BINARY_DIR}/${PROJECT_NAME}")


# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${ACSS_OUT_ROOT}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${ACSS_OUT_ROOT}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ACSS_OUT_ROOT}/bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${ACSS_OUT_ROOT})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${ACSS_OUT_ROOT})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ACSS_OUT_ROOT})

# 添加编译选项
add_compile_definitions(-DQT_DEPRECATED_WARNINGS)

set(HEADERS
    ${src_loc}/acss_globals.h
    ${src_loc}/QmlStyleUrlInterceptor.h
    ${src_loc}/QtAdvancedStylesheet.h
)

set(SOURCES
    ${src_loc}/QmlStyleUrlInterceptor.cpp
    ${src_loc}/QtAdvancedStylesheet.cpp
)


# 创建库
if(NOT acssBuildStatic)
    add_library(${TARGET_NAME} SHARED ${SOURCES} ${HEADERS})
    target_compile_definitions(${TARGET_NAME} PRIVATE ACSS_SHARED_EXPORT)
else()
    add_library(${TARGET_NAME} STATIC ${SOURCES} ${HEADERS})
    target_compile_definitions(${TARGET_NAME} PRIVATE ACSS_STATIC)
endif()

target_link_libraries(${TARGET_NAME}
PRIVATE
  external_qt
)

include_directories(${TARGET_NAME}
PUBLIC
    ${src_loc}
)

# # 设置编译选项
# if(MSVC)
#     # target_compile_options(${TARGET_NAME} PRIVATE /utf-8)

#     target_compile_options(${TARGET_NAME} PRIVATE  /Zc:__cplusplus /permissive- /utf-8)
# elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#     target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -pedantic)
# endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # add_compile_options(/Zc:__cplusplus)
    target_compile_options(${TARGET_NAME} PRIVATE  /Zc:__cplusplus /permissive- /utf-8)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${TARGET_NAME} PRIVATE -fpermissive   -Wall -Wextra -pedantic)
    # target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()



# 安装规则
if(NOT DEFINED PREFIX)
    set(PREFIX ${CMAKE_SOURCE_DIR}/../installed)
    message(${PREFIX})
endif()

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION ${PREFIX}/lib
    LIBRARY DESTINATION ${PREFIX}/lib
    RUNTIME DESTINATION ${PREFIX}/bin
)

install(FILES ${HEADERS} DESTINATION ${PREFIX}/include)
