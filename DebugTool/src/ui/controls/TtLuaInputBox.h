#ifndef UI_CONTROLS_LUAINPUTBOX_H
#define UI_CONTROLS_LUAINPUTBOX_H

#include <Qsci/qsciscintilla.h>
#include <QWidget>

namespace Ui {

class TtLuaInputBox : public QWidget {
  Q_OBJECT
 public:
  explicit TtLuaInputBox(QWidget* parent = nullptr);
  ~TtLuaInputBox();

 signals:

 private:
  void init();
  QsciScintilla* edit_lua_code_;
};

}  // namespace Ui

#endif  // UI_CONTROLS_LUAINPUTBOX_H
