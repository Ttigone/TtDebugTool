#ifndef UI_CONTROLS_LUAINPUTBOX_H
#define UI_CONTROLS_LUAINPUTBOX_H

#include <Qsci/qsciscintilla.h>
#include <QWidget>

namespace Ui {

class LuaInputBox : public QWidget {
  Q_OBJECT
 public:
  explicit LuaInputBox(QWidget* parent = nullptr);
  ~LuaInputBox();

 signals:

 private:
  void init();
  QsciScintilla* edit_lua_code_;
};

}  // namespace Ui

#endif  // UI_CONTROLS_LUAINPUTBOX_H
