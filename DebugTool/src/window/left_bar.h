#ifndef WINDOW_LEFT_BAR_H
#define WINDOW_LEFT_BAR_H

namespace Window {

class LeftBar : QWidget {
 public:
  LeftBar(QWidget *parent);
  ~LeftBar();

 private:
  ///
  /// @brief init
  /// 初始化函数
  void init();

};

} // namespace Window


#endif  // LEFT_BAR_H
