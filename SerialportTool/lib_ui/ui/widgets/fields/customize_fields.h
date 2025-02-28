/*****************************************************************/ /**
 * \file   customize_fields.h
 * \brief  自定义编辑框
 * 
 * \author C3H3_Ttigone
 * \date   August 2024
 *********************************************************************/

#ifndef UI_FIELDS_CUSTOMIZE_FIELDS_H
#define UI_FIELDS_CUSTOMIZE_FIELDS_H

#include <QCompleter>
#include <QLineEdit>

#include "ui/Def.h"

namespace Ui {

class Tt_EXPORT TtLineEdit : public QLineEdit {
  Q_OBJECT
 public:
  explicit TtLineEdit(QWidget* parent = nullptr);
  explicit TtLineEdit(const QString& text, QWidget* parent = nullptr);
  ~TtLineEdit();

  void setMaxWidth(int width);
  void setDefaultText(const QString& text);

 signals:
  void sig_foucus_out();

 protected:
  void focusOutEvent(QFocusEvent* event) override;

 private:
  /// @brief 初始化操作
  void init();
  /// @brief 文本输入变化回调
  void textChangedCallBack(const QString& text);
  /// @brief 限制最大输入长度
  /// @param text
  void limitTextLength(QString text);

  int max_len_;

  QAction* front_action_;
  QAction* end_action_;
};

class Tt_EXPORT TtLabelLineEdit : public QWidget {
  Q_OBJECT
 public:
  TtLabelLineEdit(Qt::AlignmentFlag flag, const QString& text = "",
                  QWidget* parent = nullptr);
  TtLabelLineEdit(const QString& text = "", QWidget* parent = nullptr);
  ~TtLabelLineEdit();

  TtLineEdit* body();
  void addItem(const QString& atext, const QVariant& auserData = QVariant());

  void setCurrentItem(qint8 index);
  QString itemText(int index);
  QString currentText();
  int count();

  void shortCurrentItemText();

 signals:
  void currentIndexChanged(int index);

 private:
  TtLineEdit* line_edit_;
  QLabel* label_;
};

}  // namespace Ui

#endif  // UI_FIELDS_CUSTOMIZE_FIELDS_H
