#ifndef UI_CONTROLS_LUAINPUTBOX_H
#define UI_CONTROLS_LUAINPUTBOX_H

#include <Qsci/qsciscintilla.h>
#include <QsciAPIs.h>
#include <QDialog>

namespace Ui {

class TtLuaInputBox : public QDialog {
  Q_OBJECT
 public:
  explicit TtLuaInputBox(QWidget* parent = nullptr);
  ~TtLuaInputBox();

  QString getLuaCode();

 signals:
  void closed();

 private:
  void init();
  void connectSignals();
  void addLuaApis(QsciAPIs* apis);
  void enhanceCompletion(QsciScintilla* editor);
  void updateLineNumberWidth();
  void appendCodeToEnd(QsciScintilla* editor, const QString& code);

  QsciScintilla* edit_lua_code_;

  QMap<QString, QString> lua_code_;
};

}  // namespace Ui

#endif  // UI_CONTROLS_LUAINPUTBOX_H
