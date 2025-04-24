/*****************************************************************/ /**
 * \file   buttons.h
 * \brief  底层按钮(基于 QWidget)
 * 
 * \author C3H3_Ttigone
 * \date   August 2024
 *********************************************************************/

#ifndef UI_WIDGETS_BUTTONS_H
#define UI_WIDGETS_BUTTONS_H

#include "ui/abstract_button.h"
#include "ui/widgets/labels.h"

#include <QWidget>
#include <QtSvgWidgets/QtSvgWidgets>

QT_BEGIN_NAMESPACE
class QPropertyAnimation;
class QParallelAnimationGroup;
QT_END_NAMESPACE

namespace Ui {

class TtHorizontalLayout;

class Tt_EXPORT CommonButton : public AbstractButton {
 public:
  CommonButton(QWidget* parent = nullptr);
  CommonButton(const QImage& image, QWidget* parent = nullptr);
  CommonButton(const QString& image_path, QWidget* parent = nullptr);
  CommonButton(const QImage& normal_image, const QImage& entry_image,
               QWidget* parent = nullptr);
  CommonButton(const int& w, const int& h, const QImage& normal_image,
               const QImage& entry_image, QWidget* parent = nullptr);
  CommonButton(const QString& normal_image_path,
               const QString& entry_image_path, QWidget* parent = nullptr);
  CommonButton(const int& w, const int& h, const QString& normal_image_path,
               const QString& entry_image_path, QWidget* parent = nullptr);
  ~CommonButton();

 protected:
  void paintEvent(QPaintEvent* event) override;
};

class Tt_EXPORT ConnerButton : public AbstractButton {
  Q_OBJECT
  Q_PROPERTY(QRectF rect READ rect WRITE setRect)
  Q_PROPERTY(qint16 font READ fontSize WRITE setFontSize)

 public:
  explicit ConnerButton(QWidget* parent = nullptr);
  explicit ConnerButton(const QString& normal_image_path,
                        const QString& entry_image_path,
                        QWidget* parent = nullptr);
  explicit ConnerButton(const QImage& normal_image_path,
                        const QImage& entry_image_path,
                        QWidget* parent = nullptr);

  QString connerText() const;
  void setConnerText(const QString& new_text);

  QRectF rect() const;
  void setRect(const QRectF& rect);

  qint16 fontSize() const;
  void setFontSize(const qint16& font_size);

 protected:
  void paintEvent(QPaintEvent* event) override;
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

 private:
  // QRectF icon_Rect_ = QRectF(2, 6, 30, 30);
  // 图标框
  QRectF icon_rect_ = QRectF(2, 6, 30, 30);
  // 角标框
  QRectF conner_rect_;
  qint16 conner_rect_w;
  qint16 conner_rect_h;
  // 角标缩放比例
  double zoom_radio_ = 0.05;
  // float zoom_radio_ = 0.03;
  // 角标字体
  QFont conner_font_;
  qint16 font_size_;
  qint16 font_begin_size_;
  qint16 font_end_size_;

  QString text_ = "+99";
  bool is_hovering_ = false;
  bool is_hovering_image_ = false;
  QPropertyAnimation* conner_rect_animation_;
  QPropertyAnimation* font_animation_;
  QParallelAnimationGroup* animation_group_;
};

class Tt_EXPORT TtWordsButton : public AbstractButton {
  // class WordsButton : public CommonButton {
 public:
  TtWordsButton(const QString& text, QWidget* parent = nullptr);
  TtWordsButton(const QImage& image, const QString& text,
                QWidget* parent = nullptr);
  TtWordsButton(const QString& normal_image_path,
                const QString& entry_image_path, const QString& text,
                QWidget* parent = nullptr);
  ~TtWordsButton();

  QString bottomText() const;
  void setBottomText(const QString& text);

  QSize imageSize() const;
  void setImageSize(qreal w, qreal h);

  /// @brief 是否禁用角标
  /// @return true - yes
  [[nodiscard]] bool isConnerEnable() const;
  void setConnerEnable(bool enable = false);

 protected:
  void paintEvent(QPaintEvent* event) override;
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;

 private:
  QSize image_size_;
  QSize words_size_;

  QString bottom_words_;

  bool is_conner_enable_ = false;
  AbstractButton* cooperate_btn_;
};

class Tt_EXPORT TtImageButton : public QWidget {
  Q_OBJECT
 public:
  TtImageButton(const QString& svgPath, QWidget* parent = nullptr);

  void setSvg(const QString& path);

 signals:
  void clicked();

 protected:
  void paintEvent(QPaintEvent* event) override {
    QPainter painter(this);

    // 设置背景颜色
    // QColor backgroundColor = is_pressed_ ? QColor(39, 44, 49) : QColor(49, 54, 59);

    // 设置背景颜色，当按钮按下时为深色，否则为透明
    QColor backgroundColor =
        //is_pressed_ ? QColor("#e9e9ea") : QColor(0, 0, 0, 0);  // 透明背景
        is_pressed_ ? QColor("#424242") : QColor(0, 0, 0, 0);  // 透明背景

    painter.fillRect(rect(), backgroundColor);

    // 绘制 SVG
    QSvgRenderer renderer(svg_path_);
    renderer.render(&painter, rect().marginsRemoved(QMargins(2, 2, 2, 2)));

    QWidget::paintEvent(event);
  }

  void resizeEvent(QResizeEvent* event) override {
    QWidget::resizeEvent(event);
    update();  // 重新绘制按钮
  }

  void mousePressEvent(QMouseEvent* event) override {
    if (event->button() == Qt::LeftButton) {
      is_pressed_ = true;
    }
    update();
    QWidget::mousePressEvent(event);
  }

  void mouseReleaseEvent(QMouseEvent* event) override {
    if (is_pressed_) {
      emit clicked();
      is_pressed_ = false;
    }
    update();
    QWidget::mouseReleaseEvent(event);
  }

 private:
  QString svg_path_;
  bool is_pressed_;
};

class Tt_EXPORT RichTextButton : public QWidget {
  Q_OBJECT
  Q_PROPERTY(QColor normalColor READ normalColor WRITE setNormalColor NOTIFY
                 normalColorChanged)
  Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor NOTIFY
                 hoverColorChanged)
  Q_PROPERTY(QColor pressedColor READ pressedColor WRITE setPressedColor NOTIFY
                 pressedColorChanged)
  Q_PROPERTY(
      QSize iconSize READ iconSize WRITE setIconSize NOTIFY iconSizeChanged)
  Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
  Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY
                 descriptionChanged)
  Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY
                 colorChanged)  // 新增 color 属性

 public:
  RichTextButton(const QImage& image, const QString& title,
                 const QString& description, QWidget* parent = nullptr);
  ~RichTextButton();

  // 属性 getter
  QColor normalColor() const { return normal_color_; }
  QColor hoverColor() const { return hover_color_; }
  QColor pressedColor() const { return pressed_color_; }
  QSize iconSize() const { return icon_size_; }
  QString title() const { return title_->text(); }
  QString description() const { return description_->text(); }
  QColor color() const { return current_color_; }  // 新增 color 的 getter

  // 属性 setter
  void setNormalColor(const QColor& color);
  void setHoverColor(const QColor& color);
  void setPressedColor(const QColor& color);
  void setIconSize(const QSize& size);
  void setTitle(const QString& title);
  void setDescription(const QString& description);
  void setColor(const QColor& color);  // 新增 color 的 setter

  // 设置图标
  void setIcon(const QImage& image);

 signals:
  void clicked();
  void normalColorChanged(const QColor& color);
  void hoverColorChanged(const QColor& color);
  void pressedColorChanged(const QColor& color);
  void iconSizeChanged(const QSize& size);
  void titleChanged(const QString& title);
  void descriptionChanged(const QString& description);
  void colorChanged(const QColor& color);  // 新增 color 的 signal

 protected:
  void paintEvent(QPaintEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;

 private:
  void init();
  void updateButtonColor();
  void startHoverAnimation(const QColor& startColor, const QColor& endColor);

  QImage image_;
  QLabel* title_;
  TtElidedLabel* description_;
  QSize icon_size_;
  QColor normal_color_;
  QColor hover_color_;
  QColor pressed_color_;
  QColor current_color_;  // 当前颜色（用于动画）

  bool is_pressed_;
  QPropertyAnimation* color_animation_;
};

class Tt_EXPORT TtSvgButton : public QWidget {
  Q_OBJECT
  Q_PROPERTY(QSize svgSize READ svgSize WRITE setSvgSize)
  Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
 public:
  explicit TtSvgButton(QWidget* parent = nullptr);
  TtSvgButton(const QString& svgPath, QWidget* parent = nullptr);

  void setColors(const QColor& firstColor, const QColor& secondColor);
  void setHoverBackgroundColor(const QColor& color);
  void setText(const QString& text);

  QSize svgSize() const;
  void setSvgSize(const int& w, const int& h);
  void setSvgSize(const QSize& size);

  QString svgPath() const;
  void setSvgPath(const QString& path);

  bool isChecked() const;
  void setChecked(bool checked);

  void setEnableHoldToCheck(bool enable);

  void setEnable(bool enabled);

 signals:
  void clicked();
  void toggled(bool checked);

 protected:
  void paintEvent(QPaintEvent* event) override;
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  QSize sizeHint() const override;

 private:
  void updateSvgContent();

  bool is_pressed_;
  QSvgRenderer* svg_renderer_;
  QSize svg_size_;
  bool is_checked_;
  bool enable_hold_to_check_;

  QString svg_path_;
  QString text_;
  QColor first_color_;
  QColor second_color_;
  QColor current_color_;
  QColor normal_color_;
  QColor hover_color_;
  QColor hover_bg_color_;
  QByteArray disabled_svg_content_;  // 缓存禁用状态的 SVG 内容

  bool is_hovered_;
};

class Tt_EXPORT TtSpecialDeleteButton : public QWidget {
  Q_OBJECT
 public:
  explicit TtSpecialDeleteButton(QWidget* parent = nullptr);
  explicit TtSpecialDeleteButton(const QString& name, const QString& icon_path,
                                 const QString& delete_path,
                                 QWidget* parent = nullptr);
  ~TtSpecialDeleteButton() = default;

  // 在实现文件中添加
  void setChecked(bool checked) {
    old_state_ = checked;
    update();  // 触发重绘
  }

  void setTitle(const QString& title) { name_->setText(title); }

 signals:
  void clicked();
  void deleteRequest();

 protected:
  void paintEvent(QPaintEvent* event) override;
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

 private:
  Ui::TtHorizontalLayout* layout_;
  bool is_hovered_;
  bool is_pressed_;
  QPixmap icon_;
  TtElidedLabel* name_;
  TtSvgButton* delete_button_;
  bool old_state_;
};

class Tt_EXPORT TtTextButton : public QPushButton {
  Q_OBJECT
  Q_PROPERTY(QColor checkedColor READ checkedColor WRITE setCheckedColor)
  Q_PROPERTY(bool checked READ isChecked WRITE setChecked)  // 新增属性

 public:
  explicit TtTextButton(QWidget* parent = nullptr);
  explicit TtTextButton(const QString& text, QWidget* parent = nullptr);
  explicit TtTextButton(const QColor& color, const QString& text,
                        QWidget* parent = nullptr);
  ~TtTextButton();

  // 设置/获取选中状态
  void setChecked(bool checked);
  bool isChecked() const { return is_checked_; }

  // 设置/获取选中时的文字颜色
  void setCheckedColor(const QColor& color);
  QColor checkedColor() const { return checked_color_; }

 signals:
  void toggled(bool checked);

 protected:
  void updateStyle();  // 更新样式

 private:
  bool is_checked_ = false;
  QColor checked_color_ = Qt::blue;  // 默认选中颜色
  QColor default_color_;             // 初始文字颜色
};

class Tt_EXPORT TtCodeButton : public QWidget {
  Q_OBJECT
 public:
  explicit TtCodeButton(const QString& text, const QString& path,
                        QWidget* parent);
  ~TtCodeButton();

 signals:
  void clicked();

 protected:
  void paintEvent(QPaintEvent* event) override;
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

 private:
  Ui::TtHorizontalLayout* layout_;
  QPixmap icon_;
  TtElidedLabel* name_;
  bool is_hovered_;
  bool is_pressed_;
  bool old_state_;
};

class Tt_EXPORT TtFancyButton : public QWidget {
  Q_OBJECT
 public:
  explicit TtFancyButton(const QColor& color, const QString& text,
                         QWidget* parent = nullptr)
      : QWidget(parent), background_color_(color), text_(text) {
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setMinimumSize(80, 30);
    setCursor(Qt::PointingHandCursor);
  }
  ~TtFancyButton() {}

  void setColor(const QColor& color) {
    background_color_ = color;
    update();
  }

  QColor color() const { return background_color_; }

  void setText(const QString& text) {
    text_ = text;
    updateGeometry();  // 更新布局
    update();          // 触发重绘
  }

  QString text() const { return text_; }

  // 实现类似 QPushButton 的尺寸计算逻辑
  QSize sizeHint() const override {
    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(text_) + 20;  // 左右边距各 10
    int textHeight = fm.height() + 10;                 // 上下边距各 5
    return QSize(qMax(textWidth, 80), qMax(textHeight, 30));
  }

 signals:
  void clicked();

 protected:
  void paintEvent(QPaintEvent* event) override {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bgColor = background_color_;
    if (!isEnabled()) {
      bgColor = bgColor.darker(120);
    } else if (mouse_pressed_) {
      bgColor = bgColor.lighter(150);
    } else if (underMouse()) {
      bgColor = bgColor.lighter(120);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    painter.drawRoundedRect(rect(), 4, 4);

    // 2. 绘制文本（自动计算颜色对比度）
    QColor textColor = getContrastColor(background_color_);
    painter.setPen(textColor);
    painter.setFont(font());

    QRect textRect = rect().adjusted(10, 5, -10, -5);
    painter.drawText(rect(), Qt::AlignCenter, text_);
  }

  // 鼠标事件处理
  void mousePressEvent(QMouseEvent* event) override {
    if (event->button() == Qt::LeftButton && isEnabled()) {
      mouse_pressed_ = true;
      update();  // 触发重绘显示按下状态
    }
    QWidget::mousePressEvent(event);
  }
  void mouseReleaseEvent(QMouseEvent* event) override {
    if (mouse_pressed_ && isEnabled()) {
      mouse_pressed_ = false;
      if (rect().contains(event->pos())) {
        emit clicked();  // 触发点击信号
      }
      update();
    }
    QWidget::mouseReleaseEvent(event);
  }
  // 悬停检测
  void enterEvent(QEnterEvent* event) override {
    update();
    QWidget::enterEvent(event);
  }

  void leaveEvent(QEvent* event) override {
    update();
    QWidget::leaveEvent(event);
  }

 private:
  // 自动计算对比色（深色背景用白色，浅色用黑色）
  QColor getContrastColor(const QColor& bg) const {
    return (bg.lightness() > 128) ? Qt::black : Qt::white;
  }

  QColor background_color_;
  QString text_;
  bool mouse_pressed_ = false;
};

}  // namespace Ui

#endif  // UI_WIDGETS_BUTTONS_H
