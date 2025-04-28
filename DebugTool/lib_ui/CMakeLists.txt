cmake_minimum_required(VERSION 3.16)

#project(lib_ui VERSION 1.0.0 LANGUAGES CXX)
set(TARGET_NAME lib_ui)

set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTOUIC OFF)
# set_target_properties(lib_ui PROPERTIES AUTOUIC ON)

# 使能静态库, 出现找不到对应的函数定义
# 只能动态库
option(TTLIBUI_LIBRARY_BUILD_STATIC_LIB "Build static library." OFF)

# 设置输出路径
set(LIB_UI_OUT_ROOT "${CMAKE_BINARY_DIR}/${PROJECT_NAME}")


add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_definitions(-DTTLIBUI_LIBRARY) # 在库项目中定义 TTLIBUI_LIBRARY


# 设置库导出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_UI_OUT_ROOT})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_UI_OUT_ROOT})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LIB_UI_OUT_ROOT})

if (TTLIBUI_LIBRARY_BUILD_STATIC_LIB)
    set(LIB_TYPE "STATIC")
else ()
    set(LIB_TYPE "SHARED")
endif ()

if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${TARGET_NAME}
        ${LIB_TYPE}
        ui/res/fonts.qrc
        # ${PROJECT_SOURCES}
    )
else ()
    add_library(${TARGET_NAME} ${LIB_TYPE}
        # ${PROJECT_SOURCES}
    )
endif ()

set_target_properties(${TARGET_NAME} PROPERTIES AUTOUIC OFF)

# 没有使用
# target_precompile_headers(lib_ui PRIVATE $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:${src_loc}/ui/ui_pch.h>)
# target_precompile_headers(lib_ui PRIVATE ${src_loc}/ui/ui_pch.h)
target_precompile_headers(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/ui_pch.h
    # ${CMAKE_CURRENT_SOURCE_DIR}/ui/Def.h
)


target_sources(${TARGET_NAME}
PRIVATE
    ui/res/fonts.qrc

    ui/ui_pch.h
    ui/Def.h
    ui/singleton.h
    ui/TtTheme.h ui/TtTheme.cpp
    ui/TtTheme_p.h

    ui/text/font/TtFontManager.h ui/text/font/TtFontManager.cpp

    ui/window/title/windowbar.h ui/window/title/windowbar.cpp  ui/window/title/windowbar_p.h
    ui/window/title/window_button.h ui/window/title/window_button.cpp  ui/window/title/window_button_p.h

    ui/style/style_core_types.h
    ui/style/style_font.h ui/style/style_font.cpp
    ui/style/TtCheckBoxStyle.h ui/style/TtCheckBoxStyle.cpp
    ui/style/TtLineEditStyle.h ui/style/TtLineEditStyle.cpp
    ui/style/TtComboBoxStyle.h ui/style/TtComboBoxStyle.cpp
    ui/style/TtScrollBarStyle.h ui/style/TtScrollBarStyle.cpp
    ui/style/TtRadioButtonStyle.h ui/style/TtRadioButtonStyle.cpp
    ui/style/TtListViewStyle.h ui/style/TtListViewStyle.cpp
    ui/style/TtBaseTableStyle.h ui/style/TtBaseTableStyle.cpp

    ui/text/text.h ui/text/text.cpp
    ui/text/text_entity.h ui/text/text_entity.cpp

    ui/effects/animation_value.h ui/effects/animation_value.cpp
    ui/effects/animated_drawer.h ui/effects/animated_drawer.cpp

    ui/click_handler.h ui/click_handler.cpp
    ui/mb_widget.h ui/mb_widget.cpp
    ui/abstract_button.h ui/abstract_button.cpp

    ui/widgets/buttons.h ui/widgets/buttons.cpp
    ui/widgets/labels.h ui/widgets/labels.cpp
    ui/widgets/message_textedit.h ui/widgets/message_textedit.cpp
    ui/widgets/fields/customize_edit.h ui/widgets/fields/customize_edit.cpp
    ui/widgets/fields/customize_fields.h ui/widgets/fields/customize_fields.cpp
    ui/widgets/customizationtabwidget.h ui/widgets/customizationtabwidget.cpp
    ui/widgets/collapsible_panel.h ui/widgets/collapsible_panel.cpp
    ui/widgets/overlay_widget.h ui/widgets/overlay_widget.cpp
    ui/widgets/widget_group.h ui/widgets/widget_group.cpp
    ui/widgets/message_bar.h ui/widgets/message_bar.cpp
    ui/widgets/message_bar_p.h ui/widgets/message_bar_p.cpp

    ui/layout/vertical_layout.h ui/layout/vertial_layout.cpp
    ui/layout/horizontal_layout.h ui/layout/horizontal_layout.cpp

    ui/control/TtPopUpDrawer.h ui/control/TtPopUpDrawer.cpp
    ui/control/TtPopUpDrawer_p.h
    ui/control/TtIconButton.h ui/control/TtIconButton.cpp
    ui/control/TtIconButton_p.h
    ui/control/TtMaskWidget.h ui/control/TtMaskWidget.cpp
    ui/control/TtMaskWidget_p.h
    ui/control/TtCheckBox.h ui/control/TtCheckBox.cpp
    ui/control/TtLineEdit.h ui/control/TtLineEdit.cpp
    ui/control/TtLineEdit_p.h
    ui/control/TtComboBox.h ui/control/TtComboBox.cpp
    ui/control/TtComboBox_p.h
    ui/control/TtScrollBar.h ui/control/TtScrollBar.cpp
    ui/control/TtScrollBar_p.h
    ui/control/TtRadioButton.h ui/control/TtRadioButton.cpp
    ui/control/TtRadioButton_p.h
    ui/control/TtListView.h ui/control/TtListView.cpp
    ui/control/TtListView_p.h
    ui/control/TtDialog.h ui/control/TtDialog.cpp
    ui/control/TtDialog_p.h
    ui/control/TtSwitchButton.h ui/control/TtSwitchButton.cpp
    ui/control/TtDrawer.h ui/control/TtDrawer.cpp
    ui/control/TtContentDialog.h ui/control/TtContentDialog.cpp
    ui/control/TtContentDialog_p.h
    ui/control/TtTextButton.h ui/control/TtTextButton.cpp
    ui/control/TtTextButton_p.h


    ui/control/buttonbox/TtButtonBox.h ui/control/buttonbox/TtButtonBox.cpp
    ui/control/ChatWidget/TtChatView.h  ui/control/ChatWidget/TtChatView.cpp
    ui/control/ChatWidget/TtChatMessage.h  ui/control/ChatWidget/TtChatMessage.cpp
    ui/control/ChatWidget/TtChatMessageDelegate.h  ui/control/ChatWidget/TtChatMessageDelegate.cpp
    ui/control/ChatWidget/TtChatMessageModel.h  ui/control/ChatWidget/TtChatMessageModel.cpp
    ui/control/TableWidget/TtBaseTableWidget.h ui/control/TableWidget/TtBaseTableWidget.cpp
    ui/control/TableWidget/TtHeaderView.h ui/control/TableWidget/TtHeaderView.cpp

)


# add_library(desktop-app::lib_ui ALIAS ${TARGET_NAME}) # 别名


if (MINGW)
    set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "")
endif ()
if (MSVC)
    # 后缀添加了一个 lib_ui.dll -> lib_uid.dll
    set_target_properties(${TARGET_NAME} PROPERTIES DEBUG_POSTFIX "d")
endif ()

# 省略相对路径的麻烦
target_include_directories(${TARGET_NAME}
PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${TARGET_NAME}
PUBLIC
    desktop-app::lib_base
    external_qt
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()


# install(TARGETS ${TARGET_NAME}
#     ARCHIVE DESTINATION ${LIB_UI_OUT_ROOT}/lib
#     LIBRARY DESTINATION ${LIB_UI_OUT_ROOT}}/lib
#     RUNTIME DESTINATION ${LIB_UI_OUT_ROOT}}/bin
# )

# 设置动态库的输出路径
#set_target_properties(${TARGET_NAME} PROPERTIES
    #LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    #RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    #ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
#)


