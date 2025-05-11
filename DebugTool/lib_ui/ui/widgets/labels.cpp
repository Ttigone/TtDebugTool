#include "ui/widgets/labels.h"

#include <QApplication>
#include <QClipboard>
#include <QGuiApplication>

#include "base/invoke_queued.h"

namespace Ui {
FlatLabel::FlatLabel(QWidget *parent) {}

FlatLabel::FlatLabel(QWidget *parent, const QString &text) {}

void FlatLabel::setOpacity(float64 o) {}

void FlatLabel::setTextColorOverride(std::optional<QColor> color) {}

void FlatLabel::setText(const QString &text) {}

void FlatLabel::setSelectable(bool selectable) {}

void FlatLabel::setDoubleClickSelectsParagraph(
    bool double_click_selects_paragraph) {}

void FlatLabel::setContextCopyText(const QString &copyText) {}

void FlatLabel::setBreakEverywhere(bool breakEverywhere) {}

void FlatLabel::setTryMakeSimilarLines(bool tryMakeSimilarLines) {}

void FlatLabel::setAnimationPausedCallBack(
    std::function<WhichAnimationsPaused()> call_back) {}

int FlatLabel::textMaxWidth() const { return text_.maxWidth(); }

int FlatLabel::naturalWidth() const { return textMaxWidth(); }

QMargins FlatLabel::getMargins() const { return MbWidget::getMargins(); }

void FlatLabel::setLink(uint16 index, const ClickHandlerPtr &lnk) {}

void FlatLabel::setLinksTrusted() {}

void FlatLabel::setClickHandlerFilter(ClickHandlerFilter &&filter) {}

void FlatLabel::overrideLinkClickHandler(std::function<void()> handler) {}

void FlatLabel::overrideLinkClickHandler(
    std::function<void(QString url)> handler) {}

void FlatLabel::setContextMenuHook(
    std::function<void(ContextMenuReques)> hook) {}

void FlatLabel::fillContextMenu(ContextMenuReques request) {}

void FlatLabel::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  painter.setOpacity(opacity_);
  // 设置笔的颜色
  if (text_color_override_) {
    painter.setPen(*text_color_override_);
  } else {
    painter.setPen(QColor());
  }
  // painter.setTe
}

void FlatLabel::mouseMoveEvent(QMouseEvent *event) {
  MbWidget::mouseMoveEvent(event);
}

void FlatLabel::mousePressEvent(QMouseEvent *event) {
  MbWidget::mousePressEvent(event);
}

void FlatLabel::mouseReleaseEvent(QMouseEvent *event) {
  MbWidget::mouseReleaseEvent(event);
}

void FlatLabel::mouseDoubleClickEvent(QMouseEvent *event) {
  MbWidget::mouseDoubleClickEvent(event);
}

void FlatLabel::enterEvent(QEnterEvent *event) { MbWidget::enterEvent(event); }

void FlatLabel::leaveEvent(QEvent *event) { MbWidget::leaveEvent(event); }

void FlatLabel::focusInEvent(QFocusEvent *event) {
  MbWidget::focusInEvent(event);
}

void FlatLabel::focusOutEvent(QFocusEvent *event) {
  MbWidget::focusOutEvent(event);
}

void FlatLabel::keyPressEvent(QKeyEvent *event) {
  MbWidget::keyPressEvent(event);
}

void FlatLabel::contextMenuEvent(QContextMenuEvent *event) {
  MbWidget::contextMenuEvent(event);
}

int FlatLabel::resizeGetHieght() const { return MbWidget::resizeGetHieght(); }

void FlatLabel::copySelectedText() {}

void FlatLabel::copyContextText() {}

void FlatLabel::touchSelect() {}

void FlatLabel::executeDrag() {}

void FlatLabel::init() { context_copy_text = "Copy Text"; }

Text::StateResult FlatLabel::dragActionUpdate() {
  // 全局映射到局部
  auto m = mapFromGlobal(last_mouse_pos_);
  // 获取文本状态
  auto state = getTextState(m);
  // 更新
  updateHover(state);
  if (drag_action_ == PrepareDrag &&
      (m - drag_start_position_).manhattanLength() >=
          QApplication::startDragDistance()) {
    drag_action_ = Dragging;
    InvokeQueued(this, [=] { executeDrag(); });
  }
  return state;
}

Text::StateResult FlatLabel::dragActionStart(const QPoint &p,
                                             Qt::MouseButton button) {
  last_mouse_pos_ = p;
  auto state = dragActionUpdate();
  if (button != Qt::LeftButton)
    return state;
  // 按压状态
  ClickHandler::pressed();
  drag_action_ = NoDrag;
  // drag_was_in_active_ =
  // if (ClickHandlerFilter) }
  return state;
}

Text::StateResult FlatLabel::dragActionFinish(const QPoint &p,
                                              Qt::MouseButton button) {
  last_mouse_pos_ = p;
  auto state = dragActionUpdate();

  auto activated = ClickHandler::unPressed();
  if (drag_action_ == Dragging) {
    activated = nullptr;
  } else if (drag_action_ == PrepareDrag) {
    selection_ = {0, 0};
    saved_selection_ = {0, 0};
    update();
  }
  drag_action_ = NoDrag;
  selection_type_ = TextSelectType::Letters;

  if (activated) {
    // _clickHandlerFilter may delete `this`. In that case we don't want
    // to try to show a context menu or smth like that.
    // crl::on_main(this, [=] {
    //  const auto guard = window();
    //  if (!_clickHandlerFilter || _clickHandlerFilter(activated, button)) {
    //    ActivateClickHandler(guard, activated, button);
    //  }
    //});
  }

  if (QGuiApplication::clipboard()->supportsSelection() &&
      !selection_.empty()) {
    TextUtilities::SetClipboardText(text_.toTextForMimeData(selection_),
                                    QClipboard::Selection);
  }
  return state;
}

void FlatLabel::updateHover(const Text::StateResult &state) {
  bool lnkChanged = ClickHandler::setActive(state.link, this);
  if (!select_able_) {
    refreshCursor(state.uponSymbol);
    return;
  }
  Qt::CursorShape cur = style::cur_default;
  if (drag_action_ == NoDrag) {
    if (state.link) {
      cur = style::cur_pointer;

    } else if (state.uponSymbol) {
      cur = style::cur_text;
    }
  } else {
    if (drag_action_ == Seleting) {
      uint16 second = state.symbol;
      if (state.afterSymbol && selection_type_ == TextSelectType::Letters) {
        ++second;
      }
      auto selection = text_.adjustSelection(
          {qMin(second, drag_symbol_), qMax(second, drag_symbol_)},
          selection_type_);
      if (selection_ != selection) {
        selection_ = selection;
        saved_selection_ = {0, 0};
        setFocus();
        update();
      }
    } else if (drag_action_ == Dragging) {
    }
    if (ClickHandler::getPressed()) {
      cur = style::cur_pointer;
    } else if (drag_action_ == Seleting) {
      cur = style::cur_text;
    }
  }
  if (drag_action_ == Seleting) {
  } else {
  }
  if (drag_action_ == NoDrag && (lnkChanged || cur != cursor_)) {
    setCursor(cursor_ = cur);
  }
}

Text::StateResult FlatLabel::getTextState(const QPoint &p) const {
  Text::StateRequestElided request;
  request.align = style::al_left;
  if (select_able_) {
    request.flags |= Text::StateRequest::Flag::LookupSymbol;
  }
  int textWidth = width() - getMargins().left() - getMargins().right();
  Text::StateResult state;
  return state;
}

void FlatLabel::refreshCursor(bool uponSumbol) {
  if (drag_action_ != NoDrag) {
    return;
  }
  bool needTextCursor = select_able_ && uponSumbol;
  style::cursor newCursor =
      needTextCursor ? style::cur_text : style::cur_default;
  if (ClickHandler::getActive()) {
    newCursor = style::cur_pointer;
  }
  if (newCursor != cursor_) {
    cursor_ = newCursor;
    setCursor(cursor_);
  }
}

int FlatLabel::countTextWidth() const {
  // const auto available = _allowedWidth
  //                            ? _allowedWidth
  //                            : (_st.minWidth ? _st.minWidth :
  //                            _text.maxWidth());
  // if (_allowedWidth > 0 && _allowedWidth < _text.maxWidth() &&
  //     _tryMakeSimilarLines) {
  //   auto large = _allowedWidth;
  //   auto small = _allowedWidth / 2;
  //   const auto largeHeight = _text.countHeight(large, _breakEverywhere);
  //   while (large - small > 1) {
  //     const auto middle = (large + small) / 2;
  //     if (largeHeight == _text.countHeight(middle, _breakEverywhere)) {
  //       large = middle;
  //     } else {
  //       small = middle;
  //     }
  //   }
  //   return large;
  // }
  // return available;
  return 0;
}

int FlatLabel::countTextHeight(int textWidth) { return 0; }

void FlatLabel::refreshSize() {}

TtNormalLabel::TtNormalLabel(QWidget *parent) : QLabel(parent) { init(); }

TtNormalLabel::TtNormalLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent) {
  init();
}

TtNormalLabel::~TtNormalLabel() { qDebug() << "delete" << __FUNCTION__; }

void TtNormalLabel::init() { this->setStyleSheet("border: 0px"); }

} // namespace Ui
