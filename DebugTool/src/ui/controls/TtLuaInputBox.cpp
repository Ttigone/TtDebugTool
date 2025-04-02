#include "ui/controls/TtLuaInputBox.h"
#include <ui/control/TtListView.h>
#include <ui/widgets/buttons.h>

#include <QsciLexerLua.h>

#include <ui/layout/vertical_layout.h>

namespace Ui {

TtLuaInputBox::TtLuaInputBox(QWidget* parent) : QDialog(parent) {
  init();
}

TtLuaInputBox::~TtLuaInputBox() {
  qDebug() << "delete inpubot";
}

void TtLuaInputBox::init() {
  QHBoxLayout* layout = new QHBoxLayout(this);

  edit_lua_code_ = new QsciScintilla;
  edit_lua_code_->setMarginType(0, QsciScintilla::NumberMargin);

  QsciLexerLua* luaLexer = new QsciLexerLua(edit_lua_code_);
  edit_lua_code_->setLexer(luaLexer);
  edit_lua_code_->setFrameStyle(QFrame::NoFrame);

  layout->addWidget(edit_lua_code_);

  Ui::TtTextButton* canclebutton = new Ui::TtTextButton(tr("取消"), this);
  canclebutton->setCheckedColor(Qt::cyan);
  Ui::TtTextButton* savebutton = new Ui::TtTextButton(tr("保存"), this);
  savebutton->setCheckedColor(Qt::cyan);

  Ui::TtListView* script_listview = new Ui::TtListView;
  QGridLayout* vl = new QGridLayout;
  vl->setContentsMargins(QMargins());
  vl->setSpacing(0);
  vl->addWidget(script_listview, 0, 0, 1, 2);
  vl->addWidget(canclebutton, 1, 0, 1, 1, Qt::AlignRight);
  vl->addWidget(savebutton, 1, 1, 1, 1);

  layout->addLayout(vl, 0);
  layout->setStretch(0, 2);
  layout->setStretch(1, 1);
}

}  // namespace Ui
