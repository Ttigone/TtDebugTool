/*****************************************************************//**
 * \file   mb_widget.h
 * \brief  自定义QWidget
 * 
 * \author C3H3_Ttigone
 * \date   August 2024
 *********************************************************************/

#ifndef MB_WIDGET_H
#define MB_WIDGET_H

#include <QWidget>
#include <QtEvents>

namespace Ui
{

class MbWidget;

template <typename Base>
class MBWidgetHelper : public Base {
  //Q_OBJECT
 public:
  using Base::Base;

  virtual QMargins getMargins() const { return QMargins(); }

  void moveToLeft(int x, int y, int outerw = 0);
  void moveToRight(int x, int y, int outerw = 0);
  void setGeometryToLeft(const QRect &rect, int outerw = 0);
  void setGeometryToLeft(int x, int y, int w, int h, int outerw = 0);
  void setGeometryToRight(const QRect &rect, int outerw = 0);
  void setGeometryToRight(int x, int y, int w, int h, int outerw = 0);

 protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  // void enterEvent(QEnterEvent *event) final override {
  //   if (auto parent = mbParent()) {
  //     parent->leaveToChildEvent(event, this);
  //   }
  //   return enterEvent(event);
  // }

#else
  void enterEvent(QEvent *event) override {
    if (auto parent = mbParent()) {
    }
    return enterEvent(staitc_cast<QEnterEvent *>(event));
  }
#endif

  // virtual void enterEventHook(QEnterEvent *event)
  //{
  //  return Base::enterEvent(event);
  //}

  void leaveEvent(QEvent *event) override {
    //if (auto parent = mbParent()) {
    //  parent->enterFromChildEvent(event, this);
    //}
    return leaveEventHook(event);
  }
  virtual void leaveEventHook(QEvent *event) { return Base::leaveEvent(event); }

  virtual void leaveToChildEvent(QEvent *event, QWidget *child) {}

  virtual void enterFromChildEvent(QEvent *event, QWidget *child) {}

 private:
  //MbWidget *mbParent() {
  //  return qobject_cast<MbWidget *>(Base::parentWidget());
  //}

  //const MbWidget *mbParent() const {
  //  return qobject_cast<const MbWidget *>(Base::parentWidget());
  //}

  template <typename OtherBase>
  friend class MBWidgetHelper;
};


class MbWidget : public MBWidgetHelper<QWidget> {
  //class MbWidget : public QWidget {
  // 添加这个宏时， 可能会使 样式表无效
  //Q_OBJECT
 public:
  MbWidget(QWidget* parent = nullptr);

  virtual int naturalWidth() const { return -1; }

  void resizeToWidth(int new_width) {
    auto margins = getMargins();
    //auto margins = getMarigns();
    auto fullWidth = margins.left() + new_width + margins.right();
    auto fullHeight = margins.top() + resizeGetHieght() + margins.bottom();
    auto newSize = QSize(fullWidth, fullHeight);
    if (newSize != size()) {
      resize(newSize);
      update();
    }
  }

  void resizeToNaturalWidth(int new_width) {
    const auto natural = naturalWidth();
    resizeToWidth((natural > 0) ? qMin(new_width, natural) : new_width);
  }

  /// @brief 减去原生的 margin
  /// @return 返回没有 margin 的 size
  QRect rectNoMargins() const { return rect().marginsRemoved(getMargins()); }

  /// @brief 获取没有 margin 的宽度(r 宽度)
  /// @return r宽度
  int widthNoMargins() const { return rectNoMargins().width(); }

  /// @brief 获取没有 margin 的高度(h 高度)
  /// @return h高度
  int heightNoMargins() const { return rectNoMargins().height(); }

  /// @brief 
  /// @return 
  int bottomNoMargins() const {
    auto rectWithoutMargins = rectNoMargins();
    return y() + rectWithoutMargins.y() + rectNoMargins().height();
  }

  QSize sizeNoMargins() const { return rectNoMargins().size(); }

  void setVisibleTopBottom(int visible_top, int visible_bottom) {
    const auto max = std::max(height(), 0);
    // visi
  }

 protected:
  void setChildVisibleTopBottom(MbWidget* child, int visble_top,
                                int visible_bottom) {
    if (child) {
      auto top = child->y();
      child->setVisibleTopBottom(visble_top - top, visible_bottom - top);
    }
  }

  virtual int resizeGetHieght() const { return heightNoMargins(); }

  virtual void visibleTopBottomUpdated(int visible_top, int visible_bottom) {}
};


// void ResizeFitChild(not_null<MbWidget *> parent, not_null<QWidget *> child,
//                     int height_min = 0);

template <typename Widget>
using MbWidgetParent = std::conditional_t<std::is_same_v<Widget, QWidget>,
                                          MbWidget, MBWidgetHelper<Widget>>;

} // namespace Ui





#endif  // MB_WIDGET_H
