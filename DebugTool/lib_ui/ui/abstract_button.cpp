#include  "ui/abstract_button.h"
// #include  "ui/style/style_core_types.h"

#include <QPointer>
#include <QMouseEvent>


namespace Ui {
AbstractButton::AbstractButton(QWidget* parent) : QWidget(parent) {
  // initialSize(30, 30);
  initialSize(30, 30);
  setMouseTracking(true);
}

AbstractButton::AbstractButton(const int &w, const int &h, QWidget* parent) : QWidget(parent) {
  initialSize(w, h);
  setMouseTracking(true);
}

AbstractButton::AbstractButton(const QImage& image, QWidget* parent)
    : AbstractButton(parent) {
  setImage(image);
}

AbstractButton::AbstractButton(const QString& image_path, QWidget* parent)
    : AbstractButton(parent) {
  setImage(QImage(image_path));
}

AbstractButton::AbstractButton(const QImage& normal_image, const QImage& entry_image, QWidget* parent)
	: AbstractButton(parent)
{setImage(normal_image, entry_image);}

AbstractButton::AbstractButton(const int &w, const int &h, const QImage& normal_image, const QImage& entry_image, QWidget* parent)
	//: AbstractButton(parent)
	: AbstractButton(w, h, parent)
{setImage(normal_image, entry_image);}


AbstractButton::AbstractButton(const QString& normal_image_path,
                               const QString& entry_image_path, QWidget* parent)
    : AbstractButton(parent) {
  setImage(QImage(normal_image_path), QImage(entry_image_path));
}

AbstractButton::AbstractButton(const int &w, const int &h, const QString& normal_image_path,
                               const QString& entry_image_path, QWidget* parent)
    //: AbstractButton(parent) {
    : AbstractButton(w, h, parent) {
  setImage(QImage(normal_image_path), QImage(entry_image_path));
}

AbstractButton::~AbstractButton() = default;

void AbstractButton::setDisabled(bool disabled) {
  auto was = state_;
  // 设置禁用状态, 并且之前不处于禁用状态
  if (disabled && !(state_ & StateFlag::Disabled)) {
    state_ |= StateFlag::Disabled;
    onStateChanged(was, StateChangedSource::ByUser);
  } else if (!disabled && (state_ & StateFlag::Disabled)) {
    state_ &= ~State(StateFlag::Disabled);
    onStateChanged(was, StateChangedSource::ByUser);
  }
}

void AbstractButton::clearState() {
  auto was = state_;
  state_ = StateFlag::None;
  onStateChanged(was, StateChangedSource::ByUser);
}

bool AbstractButton::isHover() const { return state_ & StateFlag::Hover; }

bool AbstractButton::isDown() const { return state_ & StateFlag::Down; }

bool AbstractButton::isDisabled() const { return state_ & StateFlag::Disabled; }

std::vector<QImage> AbstractButton::getImage() const {
  if (!normal_image_.isNull() && !entry_image_.isNull()) {
    return std::vector<QImage>{normal_image_, entry_image_};
  }
  return {};
}

void AbstractButton::setImage(const QImage& image)
{
  normal_image_ = entry_image_ = image;
}

void AbstractButton::setImage(const QString& image)
{ setImage(QImage(image)); }

void AbstractButton::setImage(const QImage& normal_image,
                              const QImage& entry_image) {
  normal_image_ = normal_image;
  entry_image_ = entry_image;
}

void AbstractButton::setImage(const QString& normal_image_path,
                              const QString& entry_image_path) {
  setImage(QImage(normal_image_path), QImage(entry_image_path));
}

QRectF AbstractButton::imageRect() const { return image_rect_; }

void AbstractButton::setImageRect(const QRectF image_rect) {
  image_rect_ = image_rect;
}

void AbstractButton::setImageRect(qreal x, qreal y, qreal w, qreal h) {
  image_rect_ = QRectF(x, y, w, h);
}

void AbstractButton::clicked(Qt::KeyboardModifiers modifiers,
                             Qt::MouseButton button) {
  modifiers_ = modifiers;
  const auto weak = QPointer<QWidget>(this);
  if (button == Qt::LeftButton) {
    if (const auto callBack = callBack_) {
      // 执行回调函数
      callBack();
    }
  }
  if (weak) {
    // cli
  }
}

void AbstractButton::enterEvent(QEnterEvent* event) {
  // 右边进入无相应
  // LOG_DEBUG() << "DSADASDASCCCCCCC";
  checkContainPoint(mapFromGlobal(QCursor::pos()));
  QWidget::enterEvent(event);
}

void AbstractButton::leaveEvent(QEvent* event) {
  if (!(state_ & StateFlag::Down)) {
		setHover(false, StateChangedSource::ByHover);
  }
  QWidget::leaveEvent(event);
}

void AbstractButton::mousePressEvent(QMouseEvent* event) {
  checkContainPoint(event->pos());
  // 处于覆盖状态
  if (state_ & StateFlag::Hover) {
    if (setDown(true, StateChangedSource::ByPress, event->modifiers(),
                event->button())) {
      event->accept();
    }
  }
  QWidget::mousePressEvent(event);
}

void AbstractButton::mouseMoveEvent(QMouseEvent* event) {
  setHover(rect().marginsRemoved(contentsMargins()).contains(event->pos()),
           StateChangedSource::ByHover);
  QWidget::mouseMoveEvent(event);
}

void AbstractButton::mouseReleaseEvent(QMouseEvent* event) {
  if (setDown(false, StateChangedSource::ByPress, event->modifiers(),
              event->button())) {
    event->accept();
  }
  QWidget::mouseReleaseEvent(event);
}

void AbstractButton::setHover(bool hover, StateChangedSource source) {
  // 正处于覆盖
  if (hover == isHover()) {
    return;
  }
  auto was = state_;
  if (hover) {
    state_ |= StateFlag::Hover;
  } else {
    state_ &= ~State(StateFlag::Hover);
  }
  onStateChanged(was, source);
  updateCursor();
  update();
}

bool AbstractButton::setDown(bool down, StateChangedSource source,
                             Qt::KeyboardModifiers modifiers,
                             Qt::MouseButton button) {
  // 之前处于 非按下状态, 并且当前是鼠标左键按下
  if (down && !(state_ & StateFlag::Down) &&
      (accept_both_ || button == Qt::LeftButton)) {
    auto was = state_;
    state_ |= StateFlag::Down;
    onStateChanged(was, source);
    return true;
  } else if (!down && (state_ & StateFlag::Down)) {
    // 之前是按下状态, 并且当前是鼠标左键释放
    const auto was = state_;
    state_ &= ~State(StateFlag::Down);
    // QPointer 管理指针, 当指针被释放时, QPointer 会自动将指针置为 nullptr
    const auto weak = QPointer<QWidget>(this);
    onStateChanged(was, source);
    if (weak) {
      // 释放时, 需要判断是否还在当前窗口内
      if (was & StateFlag::Hover) {
        setHover(true, source);
      } else {
        // 不在窗口内, 需要判断是否在覆盖状态
        setHover(false, source);
      }
    }
    return true;
  }
  return false;
}

void AbstractButton::initialSize(int w, int h) { setFixedSize(w, h); }

void AbstractButton::updateCursor() {
  // 启用鼠标指针, 并且覆盖
  const auto pointerCursor = enable_pointer_cursor_ && isHover();
  // 之前不是这个状态, 需要做出改变
  // if (pointerCursor != pointer_cursor_) {
  //   pointer_cursor_ = pointerCursor;
  //   setCursor(pointer_cursor_ ? style::cur_pointer : style::cur_default);
  // }
}

void AbstractButton::checkContainPoint(QPoint point) {
  setHover(rect().marginsRemoved(contentsMargins()).contains(point), StateChangedSource::ByHover);
}
}  // namespace Ui
