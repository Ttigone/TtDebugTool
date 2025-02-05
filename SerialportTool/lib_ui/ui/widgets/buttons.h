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

#include <QtSvgWidgets/QtSvgWidgets>

QT_BEGIN_NAMESPACE
class QPropertyAnimation;
class QParallelAnimationGroup;
QT_END_NAMESPACE

namespace Ui {

class CommonButton : public AbstractButton {
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

class ConnerButton : public AbstractButton {
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

class TtWordsButton : public AbstractButton {
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

class TtImageButton : public QWidget {
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
        is_pressed_ ? QColor("#e9e9ea") : QColor(0, 0, 0, 0);  // 透明背景

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

class RichTextButton : public QWidget {
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

  QImage image_;         // 图标
  QLabel* title_;        // 标题
  QLabel* description_;  // 描述
  QSize icon_size_;      // 图标大小

  QColor normal_color_;
  QColor hover_color_;
  QColor pressed_color_;
  QColor current_color_;  // 当前颜色（用于动画）

  bool is_pressed_;
  QPropertyAnimation* color_animation_;  // 颜色动画
};

class TtSvgButton : public QWidget {
  Q_OBJECT
  Q_PROPERTY(QString svgPath1 READ svgFirstPath WRITE setFirstSvgPath)
  Q_PROPERTY(QString svgPath2 READ svgSecondPath WRITE setSecondSvgPath)
  Q_PROPERTY(bool currentSvg READ currentSvg WRITE setCurrentSvg NOTIFY
                 currentSvgChanged)
  Q_PROPERTY(QSize svgSize READ svgSize WRITE setSvgSize)  // 新增属性
 public:
  explicit TtSvgButton(QWidget* parent = nullptr);
  TtSvgButton(const QString& svgPath1, const QString& svgPath2,
              QWidget* parent = nullptr);

  QString svgFirstPath() const;
  void setFirstSvgPath(const QString& svgPath1);

  QString svgSecondPath() const;
  void setSecondSvgPath(const QString& svgPath2);

  bool currentSvg() const;
  void setCurrentSvg(bool currentSvg);

  void setState(bool state);

  QSize svgSize() const;
  void setSvgSize(const QSize& size);

 signals:
  void clicked();
  void currentSvgChanged();

 protected:
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

 private slots:
  void toggleSvg();

 private:
  void loadSvg();

  QString svg_path1_;
  QString svg_path2_;
  bool current_svg_;
  bool is_pressed_;
  QSvgRenderer* svg_renderer_;
  QSize svg_size_;
};

class TtToggleButton : public QWidget {
  Q_OBJECT
  Q_PROPERTY(qreal knobPosition READ knobPosition WRITE setKnobPosition)

 public:
  explicit TtToggleButton(QWidget* parent = nullptr);
  bool isChecked() const;

 signals:
  void toggled(bool checked);

 public slots:
  void setChecked(bool checked);

 protected:
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;

  qreal knobPosition() const;
  void setKnobPosition(qreal position);

 private:
  bool checked;
  qreal m_knobPosition;
  QPropertyAnimation* animation;
};

}  // namespace Ui

#endif  // UI_WIDGETS_BUTTONS_H
