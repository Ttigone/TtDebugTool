#ifndef WIDGET_SUBSCRIPITION_WIDGET_H
#define WIDGET_SUBSCRIPITION_WIDGET_H

#include <QPropertyAnimation>

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
QT_END_NAMESPACE

namespace Ui {
class TtComboBox;
class TtLineEdit;
class TtLabelComboBox;
class TtLabelLineEdit;
class TtVerticalLayout;
class TtRadioButton;
class TtSvgButton;
class TtTextButton;
}  // namespace Ui

namespace Widget {

class SubscripitionWidget : public QWidget {
  Q_OBJECT
 public:
  explicit SubscripitionWidget(QWidget* parent = nullptr);
  ~SubscripitionWidget();

 signals:
  void closed();
  void saveConfigToManager(const QByteArray& config);

 private slots:
  void confirmSub();
  void onCloseButtonClicked();
  void validateTopicInput();

 private:
  void init();
  // void getSubscripition

  Ui::TtVerticalLayout* main_layout_;
  Ui::TtSvgButton* close_button_;

  QPlainTextEdit* topic_edit_;
  QLabel* error_label_;  // 新增错误提示标签
  QPropertyAnimation* error_animation_;

  Ui::TtComboBox* qos_;
  Ui::TtLineEdit* color_;

  QPlainTextEdit* alias_edit_;

  Ui::TtLabelLineEdit* subscripition_identifier_;

  QVector<Ui::TtRadioButton*> no_local_flag_;
  QVector<Ui::TtRadioButton*> retain_as_pub_flag_;
  QButtonGroup* no_local_flag_bgr_;
  QButtonGroup* retain_as_pub_flag_bgr_;

  Ui::TtLabelComboBox* retain_handling_;

  Ui::TtTextButton* cancle_button_;
  Ui::TtTextButton* confirm_button_;
};

}  // namespace Widget

#endif  // WIDGET_SUBSCRIPITION_WIDGET_H
