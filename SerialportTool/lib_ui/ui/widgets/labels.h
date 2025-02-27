/*****************************************************************//**
 * \file   labels.h
 * \brief  自定义 label
 * 
 * \author C3H3_Ttigone
 * \date   August 2024
 *********************************************************************/

#ifndef UI_WIDGETS_LABELS_H
#define UI_WIDGETS_LABELS_H

#include "base/timer.h"
#include "ui/Def.h"
#include "ui/click_handler.h"
#include "ui/mb_widget.h"
#include "ui/style/style_core_types.h"
#include "ui/text/text.h"
#include "ui/text/text_entity.h"
#include "ui/widgets/popup_menu.h"

#include <any>

namespace Ui {

class FlatLabel : public MbWidget, public ClickHandlerHost {
 public:
  FlatLabel(QWidget *parent);
  FlatLabel(QWidget *parent, const QString &text);

  void setOpacity(float64 o);
  void setTextColorOverride(std::optional<QColor> color);

  void setText(const QString &text);

  void setMarkedText(const TextWithEntities &text_with_entities,
                     const std::any &context = {});

  /// @brief 是否可选择
  /// @param selectable
  void setSelectable(bool selectable);

  /// @brief 双击选择后
  /// @param doubleClickSelectsParagraph
  void setDoubleClickSelectsParagraph(bool double_click_selects_paragraph);

  /// @brief 设置复制的文本
  /// @param copyText
  void setContextCopyText(const QString &copyText);

  /// @brief
  /// @param breakEverywhere
  void setBreakEverywhere(bool breakEverywhere);
  void setTryMakeSimilarLines(bool tryMakeSimilarLines);

  enum class WhichAnimationsPaused {
    None,
    CustomEmoji,
    Spoiler,
    All,
  };

  void setAnimationPausedCallBack(
      std::function<WhichAnimationsPaused()> call_back);

  [[nodiscard]] int textMaxWidth() const;
  int naturalWidth() const override;
  QMargins getMargins() const override;

  void setLink(uint16 index, const ClickHandlerPtr &lnk);
  void setLinksTrusted();

  using ClickHandlerFilter =
      std::function<bool(const ClickHandlerPtr &, Qt::MouseButton)>;
  void setClickHandlerFilter(ClickHandlerFilter &&filter);
  void overrideLinkClickHandler(std::function<void()> handler);
  void overrideLinkClickHandler(std::function<void(QString url)> handler);

  /// @brief 右键弹出的菜单请求
  struct ContextMenuReques {
    // not_null<PopupMenu *> menu;
    PopupMenu *menu;
    ClickHandlerPtr link;
    TextSelection selection;
    bool uponSelection = false;
    bool fullSelection = false;
  };

  /// @brief 设置弹出菜单
  /// @param hook
  void setContextMenuHook(std::function<void(ContextMenuReques)> hook);
  /// @brief 填充菜单
  /// @param request
  void fillContextMenu(ContextMenuReques request);

 protected:
  void paintEvent(QPaintEvent *event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;
	void focusInEvent(QFocusEvent* event) override;
  void focusOutEvent(QFocusEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;



  int resizeGetHieght() const override;
  void copySelectedText();
  void copyContextText();
  void touchSelect();
  void executeDrag();

 private:
  /// @brief 初始化函数
  void init();

  /// @brief 拖动操作更新
  /// @return 
  Text::StateResult dragActionUpdate();

  /// @brief 拖动状态开始
  /// @param p 坐标
  /// @param button 按钮 
  /// @return 
  Text::StateResult dragActionStart(const QPoint &p, Qt::MouseButton button);
  /// @brief 拖动状态完成
  /// @param p 坐标
  /// @param button 按钮 
  /// @return 
  Text::StateResult dragActionFinish(const QPoint &p, Qt::MouseButton button);

  /// @brief 更新悬浮状态
  /// @param state 
  void updateHover(const Text::StateResult &state);
  /// @brief 获取文本状态
  /// @param p 
  /// @return 
  Text::StateResult getTextState(const QPoint &p) const;
  /// @brief 鼠标弹起标志
  /// @param uponSumbol  
  void refreshCursor(bool uponSumbol);

  /// @brief 计算文本宽度
  /// @return 
  int countTextWidth() const;
  /// @brief 计算文本高度
  /// @param textWidth 
  /// @return 
  int countTextHeight(int textWidth);
  /// @brief 更新文本大小
  void refreshSize();


  Text::TString text_;
  std::optional<QColor> text_color_override_;
  float64 opacity_ = 1;

  style::cursor cursor_ = style::cur_default;
  bool select_able_ = false;

  // 文本选择, 保存的文本选择
  TextSelection selection_, saved_selection_;
  // 文本类型
  TextSelectType selection_type_ = TextSelectType::Letters;
  // 双击选择整个文段
  bool double_click_selects_paragraph = false;

  enum DragAction {
    NoDrag = 0x00,
    PrepareDrag = 0x01,
    Dragging = 0x02,
    Seleting = 0x03,
  };
  // 初始没有拖动操作
  DragAction drag_action_ = DragAction::NoDrag;
  // 拖动起始位置
  QPoint drag_start_position_;
  // 拖动的标志
  uint16 drag_symbol_ = 0;
  // 未启用拖动
  bool drag_was_in_active_ = false;
  // 最后鼠标的位置
  QPoint last_mouse_pos_;
  // 鼠标单击
  QPoint tripple_click_point_;
  // 鼠标时间
  base::Timer tripple_click_timer_;
  // 弹出菜单
  std::unique_ptr<PopupMenu> context_menu_;
  // 弹出菜单函数
  std::function<void(ContextMenuReques)> context_menu_hook;
  // 选择的复制文本
  QString context_copy_text;
  // 过滤器
  ClickHandlerFilter click_handler_filter_;
  // 动作回调函数
  std::function<WhichAnimationsPaused()> animation_paused_call_back;
  int _allowedWidth = 0;
  int _textWidth = 0;
  int _fullTextHeight = 0;
  bool _breakEverywhere = false;
  bool _tryMakeSimilarLines = false;
};

class Tt_EXPORT TtNormalLabel : public QLabel {
 public:
  TtNormalLabel(QWidget* parent = nullptr);
  TtNormalLabel(const QString& text, QWidget* parent = nullptr);
  ~TtNormalLabel();

 private:
  ///
  /// @brief init
  /// 初始化
  void init();
};

class Tt_EXPORT TtElidedLabel : public QLabel {
  Q_OBJECT
  Q_PROPERTY(Qt::TextElideMode elideMode READ elideMode WRITE setElideMode)

 public:
  explicit TtElidedLabel(const QString& text = QString(),
                         QWidget* parent = nullptr)
      : QLabel(text, parent) {
    setContentsMargins(0, 0, 0, 0);
    setMinimumWidth(0);
    setIndent(0);
  }

  Qt::TextElideMode elideMode() const { return mode_; }
  void setElideMode(Qt::TextElideMode mode) {
    mode_ = mode;
    update();
  }

 protected:
  void paintEvent(QPaintEvent* event) override {
    if (text().isEmpty()) {
      QLabel::paintEvent(event);
      return;
    }

    QPainter painter(this);
    QFontMetrics fm = painter.fontMetrics();

    // qDebug() << "content width: " << contentsRect().width();
    const int availableWidth = contentsRect().width();

    // 处理极端小宽度情况
    if (availableWidth <= fm.horizontalAdvance("...")) {
      painter.drawText(rect(), alignment(), "...");
      return;
    }

    // QString elidedText = fm.elidedText(text(), mode_, width());
    QString elidedText = fm.elidedText(text(), mode_, availableWidth);
    // qDebug() << "rect: " << rect();
    painter.drawText(rect(), alignment(), elidedText);
  }

 private:
  Qt::TextElideMode mode_ = Qt::ElideRight;
};

}  // namespace Ui



#endif  // UI_WIDGETS_LABELS_H
