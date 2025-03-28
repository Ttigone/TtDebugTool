#include "ui/controls/TtLuaInputBox.h"

#include <QsciLexerLua.h>

namespace Ui {

TtLuaInputBox::TtLuaInputBox(QWidget* parent) : QWidget{parent} {
  init();
}

TtLuaInputBox::~TtLuaInputBox() {}

void TtLuaInputBox::init() {
  QHBoxLayout* layout = new QHBoxLayout(this);

  edit_lua_code_ = new QsciScintilla;
  edit_lua_code_->setMarginType(0, QsciScintilla::NumberMargin);
  // editor->setMarginType(1, QsciScintilla::);

  QsciLexerLua* luaLexer = new QsciLexerLua(edit_lua_code_);
  edit_lua_code_->setLexer(luaLexer);
  edit_lua_code_->setFrameStyle(QFrame::NoFrame);

  layout->addWidget(edit_lua_code_);
}

}  // namespace Ui
