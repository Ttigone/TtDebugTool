/*****************************************************************//**
 * \file   abstract_button.h
 * \brief  自定义抽象按钮
 * 
 * \author C3H3_Ttigone
 * \date   August 2024
 *********************************************************************/

#ifndef ABSTRACT_BUTTON_H
#define ABSTRACT_BUTTON_H

#include "base/flags.h"


namespace Ui {

class AbstractButton : public QWidget {
 public:
  AbstractButton(QWidget* parent = nullptr);
  AbstractButton(const int &w, const int &h, QWidget* parent = nullptr);
  AbstractButton(const QImage &image, QWidget* parent = nullptr);
  AbstractButton(const QString &image_path, QWidget* parent = nullptr);
  AbstractButton(const QImage &normal_image, const QImage &entry_image, QWidget* parent = nullptr);
  AbstractButton(const int &w, const int &h, const QImage &normal_image, const QImage &entry_image, QWidget* parent = nullptr);
  AbstractButton(const QString &normal_image_path, const QString &entry_image_path, QWidget* parent = nullptr);
  AbstractButton(const int &w, const int &h, const QString &normal_image_path, const QString &entry_image_path, QWidget* parent = nullptr);
  virtual ~AbstractButton();

  /// @brief 设置按钮的状态
  /// @param disabled 
  void setDisabled(bool disabled = true);

  /// @brief 清除按钮的状态
  virtual void clearState();

  /// @brief 是否覆盖
  /// @return true - yes
  [[nodiscard]] bool isHover() const;
  
  /// @brief 是否按下
  /// @return true - yes
  [[nodiscard]] bool isDown() const;
  /// @brief 是否无效
  /// @return true - yes
  [[nodiscard]] bool isDisabled() const;

  /// @brief 获取按钮图片
  /// @return 图片
  std::vector<QImage> getImage() const;
  /// @brief 只有单张图片
  /// @param image 
  void setImage(const QImage &image);
  /// @brief 只有单张图片
  /// @param image 图片路径
  void setImage(const QString &image);
  /// @brief 鼠标进入和离开的图片
  /// @param normal_image 正常显示的图片
  /// @param entry_image 进入时显示的图片
  void setImage(const QImage &normal_image, const QImage &entry_image);
  /// @brief 鼠标进入和离开的图片
  /// @param normal_image_path 正常显示的图片路径
  /// @param entry_image_path 进入时显示的图片路径
  void setImage(const QString &normal_image_path, const QString &entry_image_path);



  /// @brief 获取图片矩形
  /// @return 图像矩形
  QRectF imageRect() const;
  /// @brief 设置图片矩形
  /// @param image_rect 参数
  void setImageRect(const QRectF image_rect);
  void setImageRect(qreal x, qreal y, qreal w, qreal h);


  void clicked(Qt::KeyboardModifiers modifiers, Qt::MouseButton button); 

 protected:
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

 protected:
  enum class StateFlag {
    None = 0,
    Hover = (1 << 0),
    Down = (1 << 1),
    Disabled = (1 << 2),
  };
  using State = base::flags<StateFlag>;
  State state() const { return state_; }

  enum class StateChangedSource {
    ByUser = 0x00,
    ByPress = 0x01,
    ByHover = 0x02,
  };
  void setHover(bool hover,
                StateChangedSource source = StateChangedSource::ByUser);
  bool setDown(bool down, StateChangedSource source,
               Qt::KeyboardModifiers modifiers, Qt::MouseButton button);
  virtual void onStateChanged(
      State was, StateChangedSource source = StateChangedSource::ByUser) {}

 private:
  void initialSize(int w, int h);
  void updateCursor();
  void checkContainPoint(QPoint point);

  QImage normal_image_;
  QImage entry_image_;
  QRectF image_rect_;
  bool mouse_is_inside_ = false;
  QString image_path_;

  State state_ = StateFlag::None;
  bool enable_pointer_cursor_ = false;
  bool pointer_cursor_ = false;
  bool accept_both_ = false;

  Qt::KeyboardModifiers modifiers_;
  // 按钮回调
  std::function<void()> callBack_;

};

} // namespace Ui


#endif  // ABSTRACT_BUTTON_H
