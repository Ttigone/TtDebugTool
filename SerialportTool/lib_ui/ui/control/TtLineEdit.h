#ifndef UI_CONTROL_TTLINEEDIT_H
#define UI_CONTROL_TTLINEEDIT_H

#include <QLineEdit>

#include "ui/ui_pch.h"

namespace Ui {

class TtLineEditPrivate;

class Tt_EXPORT TtLineEdit : public QLineEdit {
  Q_OBJECT
  Q_Q_CREATE(TtLineEdit)
  Q_PROPERTY_CREATE_Q_H(int, BorderRadius)          // 圆角
  Q_PROPERTY_CREATE_Q_H(bool, IsClearButtonEnable)  // 清楚按钮
 public:
  explicit TtLineEdit(QWidget* parent = nullptr);
  explicit TtLineEdit(const QString& text, QWidget* parent = nullptr);
  ~TtLineEdit();

  // void setMaxWidth(int width);
  // void setDefaultText(const QString& text);

 signals:
  Q_SIGNAL void focusIn(QString text);
  Q_SIGNAL void focusOut(QString text);
  Q_SIGNAL void wmFocusOut(QString text);

 protected:
  void focusInEvent(QFocusEvent* event) override;
  void focusOutEvent(QFocusEvent* event) override;
  void paintEvent(QPaintEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;

 private:
  // /// @brief 文本输入变化回调
  // void textChangedCallBack(const QString& text);
  // /// @brief 限制最大输入长度
  // /// @param text
  // void limitTextLength(QString text);
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

  void setText(const QString& text);
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

#endif  // UI_CONTROL_TTLINEEDIT_H
