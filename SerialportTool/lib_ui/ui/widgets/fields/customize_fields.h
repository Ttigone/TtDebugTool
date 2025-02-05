/*****************************************************************//**
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

namespace Ui {
namespace Widgets {

class TtCustomizeFields : public QLineEdit {
  Q_OBJECT
 public:
  explicit TtCustomizeFields(QWidget* parent = nullptr);
  explicit TtCustomizeFields(const QString& text, QWidget* parent = nullptr);
  ~TtCustomizeFields();

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

  QAction *front_action_;
  QAction *end_action_;
};

}  // namespace Widgets

}  // namespace Ui



#endif  // UI_FIELDS_CUSTOMIZE_FIELDS_H


