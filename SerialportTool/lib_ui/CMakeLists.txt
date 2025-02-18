add_library(lib_ui STATIC)
add_library(desktop-app::lib_ui ALIAS lib_ui)

set(CMAKE_AUTOMOC ON)

get_filename_component(src_loc . REALPATH)

# target_precompile_headers(lib_ui
# PRIVATE 
#   $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:${src_loc}/base/basic_pch.h>
# )

set_target_properties(lib_ui PROPERTIES AUTOMOC ON)
set_target_properties(lib_ui PROPERTIES AUTOUIC OFF)

target_precompile_headers(lib_ui PRIVATE $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:${src_loc}/ui/ui_pch.h>)

target_sources(lib_ui
PRIVATE
  ui/ui_pch.h
  ui/Def.h

  ui/window/title/windowbar.h ui/window/title/windowbar.cpp  ui/window/title/windowbar_p.h
  ui/window/title/window_button.h ui/window/title/window_button.cpp  ui/window/title/window_button_p.h

  ui/window/combobox.h ui/window/combobox.cpp
  ui/window/window_switcher.h ui/window/window_switcher.cpp


  ui/style/style_core_types.h
  ui/style/style_font.h ui/style/style_font.cpp
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
  ui/widgets/snack_bar.h ui/widgets/snack_bar.cpp
  ui/widgets/widget_group.h ui/widgets/widget_group.cpp

  ui/layout/vertical_layout.h ui/layout/vertial_layout.cpp
  ui/layout/horizontal_layout.h ui/layout/horizontal_layout.cpp

  ui/control/buttonbox/TtButtonBox.h ui/control/buttonbox/TtButtonBox.cpp
  ui/control/TtTableView.h ui/control/TtTableView.cpp
  ui/control/ChatWidget/TtChatView.h  ui/control/ChatWidget/TtChatView.cpp
  ui/control/ChatWidget/TtChatMessage.h  ui/control/ChatWidget/TtChatMessage.cpp
  ui/control/ChatWidget/TtChatMessageDelegate.h  ui/control/ChatWidget/TtChatMessageDelegate.cpp
  ui/control/ChatWidget/TtChatMessageModel.h  ui/control/ChatWidget/TtChatMessageModel.cpp
	ui/control/TtPopUpDrawer.h ui/control/TtPopUpDrawer.cpp
	ui/control/TtPopUpDrawer_p.h
)

# 省略相对路径的麻烦
target_include_directories(lib_ui
PUBLIC
  ${src_loc}
)

target_link_libraries(lib_ui
PUBLIC
    desktop-app::lib_base
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()



