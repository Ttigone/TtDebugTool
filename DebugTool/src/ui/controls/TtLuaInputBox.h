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

  void updateLineNumberWidth() {
    // 获取当前行数
    int lines = edit_lua_code_->lines();

    // 计算所需的数字位数
    int digits = 1;
    int max = 10;
    while (lines >= max) {
      max *= 10;
      ++digits;
    }

    // 计算像素宽度
    QFontMetrics metrics(edit_lua_code_->font());
    int digitWidth = metrics.horizontalAdvance("9");  // 通常9是最宽的数字
    int pixelWidth = digitWidth * (digits + 1) + 10;  // 额外加一些空间

    edit_lua_code_->setMarginWidth(0, pixelWidth);
  }

  QsciScintilla* edit_lua_code_;

  QMap<QString, QString> lua_code_;
};

}  // namespace Ui

#endif  // UI_CONTROLS_LUAINPUTBOX_H
