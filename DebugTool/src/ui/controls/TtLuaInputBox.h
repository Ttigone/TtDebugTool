#ifndef UI_CONTROLS_LUAINPUTBOX_H
#define UI_CONTROLS_LUAINPUTBOX_H

// #include <Qsci/qsciscintilla.h>
#include "qsciscintilla.h"
#include <QDialog>
#include <QsciAPIs.h>

namespace Ui {

const QString functionTemplate = "function getValue(value)\n"
                                 " -- value 是解析后的整数值\n"
                                 "    return value + 1\n"
                                 "end\n";

class TtLuaInputBox : public QDialog {
  Q_OBJECT
public:
  explicit TtLuaInputBox(bool enableSaveSetting, QWidget *parent = nullptr);
  ~TtLuaInputBox();

  void setLuaCode(const QString &code);
  QString getLuaCode() const;

  // void setEnableSaveSetting(bool enable);

signals:
  void closed();

private slots:
  void applyChanges();

private:
  void init();
  void connectSignals();
  void addLuaApis(QsciAPIs *apis);
  void enhanceCompletion(QsciScintilla *editor);
  void updateLineNumberWidth();
  void appendCodeToEnd(QsciScintilla *editor, const QString &code);

  QsciScintilla *edit_lua_code_;

  QMap<QString, QString> lua_code_;

  bool save_setting_ = true;
};

} // namespace Ui

#endif // UI_CONTROLS_LUAINPUTBOX_H
