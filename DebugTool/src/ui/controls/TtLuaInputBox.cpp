#include "ui/controls/TtLuaInputBox.h"

namespace Ui {

LuaInputBox::LuaInputBox(QWidget* parent) : QWidget{parent} {
  init();
}

LuaInputBox::~LuaInputBox() {}

void LuaInputBox::init() {
  QHBoxLayout* layout = new QHBoxLayout;

  layout->addWidget(edit_lua_code_);
  edit_lua_code_ = new QsciScintilla;
}

}  // namespace Ui
