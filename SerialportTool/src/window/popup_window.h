#ifndef POPUP_WINDOW_H
#define POPUP_WINDOW_H

namespace Ui {
class VerticalLayout;
class HorizontalLayout;
class TtNormalLabel;
} // namespace Ui

namespace Window {

class PopUpWindow : public QWidget {
 public:
  PopUpWindow(QWidget *parent = nullptr);

  ///
  /// @brief setTitle
  /// @param title
  /// 设置标题
  void setTitle(const QString &title);

  ///
  /// @brief setWidget
  /// @param widget
  /// 设置窗口
  void setWidget(QWidget *widget);

  ///
  /// @brief body
  /// @return
  /// 获取窗口
  const QWidget *body();

 private:
  ///
  /// @brief init
  /// 初始化
  void init();

  // QVBoxLayout *main_layout_;
  Ui::VerticalLayout *main_layout_;

  QWidget *up_widget_;
  // QHBoxLayout *up_layout_;
  Ui::HorizontalLayout *up_layout_;
  Ui::TtNormalLabel* title_label_;
};


} // namespace Window

#endif  // POPUP_WINDOW_H
