#include "ui/widgets/buttons.h"

#include "ui/abstract_button.h"
#include "ui/layout/horizontal_layout.h"
#include "ui/layout/vertical_layout.h"

#include <QIcon>
#include <QMouseEvent>
#include <QParallelAnimationGroup>
#include <QPixmap>
#include <QPropertyAnimation>

namespace Ui {
CommonButton::CommonButton(QWidget* parent) : AbstractButton(parent) {}

CommonButton::CommonButton(const QImage& image, QWidget* parent)
    : AbstractButton(image, parent) {}

CommonButton::CommonButton(const QString& image_path, QWidget* parent)
    : AbstractButton(image_path, parent) {}

CommonButton::CommonButton(const QImage& normal_image,
                           const QImage& entry_image, QWidget* parent)
    : AbstractButton(normal_image, entry_image, parent) {}

CommonButton::CommonButton(const int& w, const int& h,
                           const QImage& normal_image,
                           const QImage& entry_image, QWidget* parent)
    : AbstractButton(w, h, normal_image, entry_image, parent) {}

CommonButton::CommonButton(const QString& normal_image_path,
                           const QString& entry_image_path, QWidget* parent)
    : AbstractButton(normal_image_path, entry_image_path, parent) {}

CommonButton::CommonButton(const int& w, const int& h,
                           const QString& normal_image_path,
                           const QString& entry_image_path, QWidget* parent)
    : AbstractButton(w, h, normal_image_path, entry_image_path, parent) {}

CommonButton::~CommonButton() {}

void CommonButton::paintEvent(QPaintEvent* event) {
  QRectF rect(0, 0, this->width(), this->height());
  QPainter painter(this);

  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

  const auto img = isHover() ? getImage().at(1) : getImage().at(0);
  const auto& test_ = QPixmap::fromImage(img);
  painter.drawPixmap(rect.toRect(),
                     test_.scaled(rect.size().toSize(), Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation));

  AbstractButton::paintEvent(event);
}

ConnerButton::ConnerButton(QWidget* parent) : AbstractButton(parent) {
  setFixedSize(40, 40);
  conner_rect_ = QRectF(this->width() - icon_rect_.x() - 22, icon_rect_.y(), 18,
                        12);  // 留出一点边距，防止图像紧贴边缘
  conner_rect_w = 18, conner_rect_h = 12;

  conner_font_.setFamily("Arial");
  conner_font_.setBold(true);

  setMouseTracking(true);
  conner_rect_animation_ = new QPropertyAnimation(this, "rect");
  // font_animation_ = new QPropertyAnimation(this, "font");
  // 持续 200 ms
  conner_rect_animation_->setDuration(100);
  // font_animation_->setDuration(50);

  animation_group_ = new QParallelAnimationGroup(this);
  animation_group_->addAnimation(conner_rect_animation_);
  // animation_group_->addAnimation(font_animation_);
}

ConnerButton::ConnerButton(const QString& normal_image_path,
                           const QString& entry_image_path, QWidget* parent)
    : ConnerButton(parent) {
  setImage(normal_image_path, entry_image_path);
}

ConnerButton::ConnerButton(const QImage& normal_image,
                           const QImage& entry_image, QWidget* parent)
    : ConnerButton(parent) {
  setImage(normal_image, entry_image);
}

QString ConnerButton::connerText() const {
  return text_;
}

void ConnerButton::setConnerText(const QString& new_text) {
  if (text_ != new_text) {
    text_ = new_text;
    update();
  }
}

QRectF ConnerButton::rect() const {
  return conner_rect_;
}

void ConnerButton::setRect(const QRectF& rect) {
  conner_rect_ = rect;
  update();
}

qint16 ConnerButton::fontSize() const {
  return font_size_;
}

void ConnerButton::setFontSize(const qint16& font_size) {
  font_size_ = font_size;
  update();
}

void ConnerButton::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter painter(this);

  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

  // 加载和绘制图像

  // QSvgRenderer svgRenderer(QString("C:/Users/cssss/Downloads/chat-3-fill
  // (1).svg")); svgRenderer.render(&painter, iconRect);

  const auto img = is_hovering_ ? getImage().at(1) : getImage().at(0);
  const auto test_ = QPixmap::fromImage(img);
  painter.drawPixmap(
      icon_rect_.toRect(),
      test_.scaled(icon_rect_.size().toSize(), Qt::KeepAspectRatio,
                   Qt::SmoothTransformation));

  // conner_rect_ = QRectF(rect.width() - icon_Rect_.x() - 18, icon_Rect_.y(),
  // 18, 14); // 留出一点边距，防止图像紧贴边缘
  const qreal radius =
      (conner_rect_.width() > this->width() / 2) ? 8 : 6;  // 圆角半径

  // 底色
  QBrush brush(QColor("#6f6f6f"));
  painter.setBrush(brush);
  painter.setPen(Qt::NoPen);  // 无边框

  // 绘制矩形
  // qDebug() << conner_rect_;
  painter.drawRoundedRect(conner_rect_, radius, radius);

  QPen textPen(QColor("#c4c4c4"));
  painter.setPen(textPen);
  // 绘制文本
  // QString text = "+99";
  // qDebug() << ++i;
  // 7 - 9
  // QFont font("Arial", font_begin_size_, QFont::Bold); // 设置字体
  // qDebug() << font_size_;
  // QFont font("Arial", font_size_, QFont::Bold); // 设置字体
  conner_font_.setPixelSize(9);
  // QFont font("Arial", 7, QFont::Bold); // 设置字体
  // QFont font("Arial", 7, QFont::Bold); // 设置字体
  painter.setFont(conner_font_);
  // 计算文本的绘制位置
  painter.drawText(conner_rect_, Qt::AlignCenter, text_);
  AbstractButton::paintEvent(event);
}

void ConnerButton::enterEvent(QEnterEvent* event) {
  Q_UNUSED(event);
  is_hovering_ = true;
  // qDebug() << "e";
  update();
}

void ConnerButton::leaveEvent(QEvent* event) {
  is_hovering_ = false;
  is_hovering_image_ = false;
  update();
}

void ConnerButton::mouseMoveEvent(QMouseEvent* event) {
  // qDebug() << this->width();
  // QRectF iconRect(2, 6, 30, 30); // 图像区域
  // 处于图像区域
  const bool was_hovering_image = is_hovering_image_;
  is_hovering_image_ = icon_rect_.contains(event->pos());
  if (was_hovering_image != is_hovering_image_) {
    update();
  }
  const auto offset = zoom_radio_ * width();
  const bool is_in_conner_rect = conner_rect_.contains(event->pos());
  // 放大
  const bool should_expand =
      is_in_conner_rect && conner_rect_.width() < (conner_rect_w + offset * 3);
  // 缩小
  const bool should_shrink =
      !is_in_conner_rect && conner_rect_.width() > conner_rect_w;

  // 有操作发生
  if ((should_expand || should_shrink) &&
      animation_group_->state() != QPropertyAnimation::Running) {
    conner_rect_animation_->setStartValue(conner_rect_);
    conner_rect_animation_->setEndValue(QRectF(
        conner_rect_.x() + (should_expand ? -offset : offset),
        conner_rect_.y() + (should_expand ? -offset : offset),
        conner_rect_.width() + (should_expand ? offset * 3 : -offset * 3),
        conner_rect_.height() + (should_expand ? offset * 3 : -offset * 3)));
    animation_group_->start();
  }
  QWidget::mouseMoveEvent(event);
}

TtWordsButton::TtWordsButton(const QString& text, QWidget* parent)
    : AbstractButton(parent), bottom_words_(text) {
  setFixedSize(40, 50);
  setImageSize(30, 30);
  words_size_ = QSize(40, 10);
}

TtWordsButton::TtWordsButton(const QImage& image, const QString& text,
                             QWidget* parent)
    : TtWordsButton(text, parent) {
  setImage(image);
  if (!is_conner_enable_ && !getImage().empty()) {
    cooperate_btn_ = new CommonButton(getImage().at(0), getImage().at(1), this);
  }
}

TtWordsButton::TtWordsButton(const QString& normal_image_path,
                             const QString& entry_image_path,
                             const QString& text, QWidget* parent)
    : TtWordsButton(text, parent) {
  setImage(normal_image_path, entry_image_path);
  if (!is_conner_enable_ && !getImage().empty()) {
    cooperate_btn_ = new CommonButton(getImage().at(0), getImage().at(1), this);
  }
}

TtWordsButton::~TtWordsButton() {}

QString TtWordsButton::bottomText() const {
  return bottom_words_;
}

void TtWordsButton::setBottomText(const QString& text) {
  bottom_words_ = text;
}

QSize TtWordsButton::imageSize() const {
  return image_size_;
}

void TtWordsButton::setImageSize(qreal w, qreal h) {
  image_size_ = QSize(w, h);
}

bool TtWordsButton::isConnerEnable() const {
  return is_conner_enable_;
}

void TtWordsButton::setConnerEnable(bool enable) {
  is_conner_enable_ = enable;
  // 启用角标
  cooperate_btn_->deleteLater();
  cooperate_btn_ = enable ? static_cast<AbstractButton*>(new ConnerButton(
                                getImage().at(0), getImage().at(1), this))
                          : static_cast<AbstractButton*>(
                                new CommonButton(getImage().at(0), this));
}

void TtWordsButton::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

  QRectF rect(0, 0, this->width() - 1, this->height() - 1);
  setImageRect(0, 0, image_size_.width(), image_size_.height());

  painter.setFont(QFont("Arial", 7));
  painter.setPen(QPen(state() == StateFlag::Hover ? "#ffffff" : "a9beae"));
  painter.drawText(QRect(this->x(), this->height() - 10, words_size_.width(),
                         words_size_.height()),
                   Qt::AlignCenter, bottom_words_);
  AbstractButton::paintEvent(event);
}

void TtWordsButton::enterEvent(QEnterEvent* event) {
  AbstractButton::enterEvent(event);
}

void TtWordsButton::leaveEvent(QEvent* event) {
  AbstractButton::leaveEvent(event);
}

RichTextButton::RichTextButton(const QImage& image, const QString& title,
                               const QString& description, QWidget* parent)
    : QWidget(parent),
      image_(image),
      title_(new QLabel(title, this)),
      description_(new TtElidedLabel(description, this)),
      is_pressed_(false) {
  init();
  // 启用双缓冲
  setAttribute(Qt::WA_OpaquePaintEvent);
  setAttribute(Qt::WA_NoSystemBackground);
}

RichTextButton::~RichTextButton() {}

void RichTextButton::setNormalColor(const QColor& color) {
  if (normal_color_ != color) {
    normal_color_ = color;
    update();
    emit normalColorChanged(color);
  }
}

void RichTextButton::setHoverColor(const QColor& color) {
  if (hover_color_ != color) {
    hover_color_ = color;
    update();
    emit hoverColorChanged(color);
  }
}

void RichTextButton::setPressedColor(const QColor& color) {
  if (pressed_color_ != color) {
    pressed_color_ = color;
    update();
    emit pressedColorChanged(color);
  }
}

void RichTextButton::setIconSize(const QSize& size) {
  if (icon_size_ != size) {
    icon_size_ = size;
    update();
    emit iconSizeChanged(size);
  }
}

void RichTextButton::setTitle(const QString& title) {
  if (title_->text() != title) {
    title_->setText(title);
    emit titleChanged(title);
  }
}

void RichTextButton::setDescription(const QString& description) {
  if (description_->text() != description) {
    description_->setText(description);
    emit descriptionChanged(description);
  }
}

void RichTextButton::setColor(const QColor& color) {
  if (current_color_ != color) {
    current_color_ = color;
    update();  // 触发重绘
    emit colorChanged(color);
  }
}

void RichTextButton::setIcon(const QImage& image) {
  image_ = image;
  if (!image_.isNull()) {
    QLabel* imageLabel = new QLabel(this);
    imageLabel->setPixmap(QPixmap::fromImage(image_).scaled(
        icon_size_, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imageLabel->setStyleSheet("border: 0;");
    qobject_cast<QGridLayout*>(layout())->addWidget(imageLabel, 0, 0, 2, 1);
  }
}

void RichTextButton::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter painter(this);
  // painter.setRenderHint(QPainter::Antialiasing, true);

  QColor color = normal_color_;
  if (is_pressed_) {
    color = pressed_color_;
  } else if (underMouse()) {
    color = hover_color_;
  }

  //painter.fillRect(rect(), color);
  // 绘制背景
  painter.setBrush(color);
  painter.setPen(Qt::NoPen);
  painter.drawRoundedRect(rect(), 3, 3);  // 圆角矩形

  //QWidget::paintEvent(event);
}

void RichTextButton::mouseReleaseEvent(QMouseEvent* event) {
  is_pressed_ = false;
  update();

  if (rect().contains(event->pos())) {
    emit clicked();
  }
  QWidget::mouseReleaseEvent(event);
}

void RichTextButton::mousePressEvent(QMouseEvent* event) {
  is_pressed_ = true;
  update();
  QWidget::mousePressEvent(event);
}

void RichTextButton::enterEvent(QEnterEvent* event) {
  startHoverAnimation(normal_color_, hover_color_);
  QWidget::enterEvent(event);
}

void RichTextButton::leaveEvent(QEvent* event) {
  is_pressed_ = false;
  startHoverAnimation(hover_color_, normal_color_);
  QWidget::leaveEvent(event);
}

void RichTextButton::init() {
  // 默认大小
  // this->setMinimumSize(200, 60);
  // this->setMaximumSize(360, 60);
  setMinimumSize(0, 0);
  // setMinimumSize(200, 60);
  resize(200, 60);
  //this->setMaximumSize(360, 60);
  // 默认颜色
  setNormalColor(QColor(240, 240, 240));   // 浅灰色
  setHoverColor(QColor(200, 200, 255));    // 浅蓝色
  setPressedColor(QColor(150, 150, 255));  // 深蓝色
  setColor(normal_color_);                 // 初始化当前颜色

  QGridLayout* mainLayout = new QGridLayout(this);
  mainLayout->setSpacing(10);
  mainLayout->setContentsMargins(QMargins(10, 5, 10, 5));

  auto imageLabel = new QLabel();
  imageLabel->setMaximumSize(40, 60);
  imageLabel->setPixmap(QPixmap::fromImage(image_).scaled(
      30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  imageLabel->setStyleSheet("border: 0");

  //title_->setMaximumHeight(30);
  title_->setStyleSheet("border: 0");
  //description_->setMaximumHeight(30);
  description_->setStyleSheet("border: 0");
  title_->setStyleSheet("font-size: 14px; font-weight: bold; color: #333;");
  description_->setStyleSheet("font-size: 12px; color: #666;");
  mainLayout->addWidget(imageLabel, 0, 0, 2, 1);
  mainLayout->addWidget(title_, 0, 1, 1, 1);
  mainLayout->addWidget(description_, 1, 1, 1, 1);

  // 初始化动画
  color_animation_ = new QPropertyAnimation(this, "color");
  color_animation_->setDuration(220);  // 200ms 动画
}

void RichTextButton::startHoverAnimation(const QColor& startColor,
                                         const QColor& endColor) {
  color_animation_->stop();
  color_animation_->setStartValue(startColor);
  color_animation_->setEndValue(endColor);
  color_animation_->start();
}

TtSvgButton::TtSvgButton(QWidget* parent)
    : QWidget(parent),
      is_pressed_(false),
      svg_renderer_(new QSvgRenderer(this)),
      svg_size_(22, 22) {
  setObjectName("TtSvgButton");
  // setMinimumSize(22, 22);
  // resize(22, 22);
  // setFixedSize(22, 22);
}

TtSvgButton::TtSvgButton(const QString& svgPath, QWidget* parent)
    : QWidget(parent),
      svg_path_(svgPath),
      svg_renderer_(new QSvgRenderer(this)),
      hover_bg_color_(QColor(200, 200, 200)),
      svg_size_(22, 22),
      is_pressed_(false),
      is_hovered_(false),
      is_checked_(false) {
  // setMinimumSize(22, 22);
  // resize(22, 22);
  // setFixedSize(22, 22);
  setAttribute(Qt::WA_Hover);
  updateSvgContent();
}

void TtSvgButton::setColors(const QColor& firstColor,
                            const QColor& secondColor) {
  first_color_ = firstColor;
  second_color_ = secondColor;
  updateSvgContent();
}

void TtSvgButton::setHoverBackgroundColor(const QColor& color) {
  hover_bg_color_ = color;
  update();
}

void TtSvgButton::setText(const QString& text) {
  if (text_ != text) {
    text_ = text;
    updateGeometry();
    update();
  }
}

QSize TtSvgButton::svgSize() const {
  return svg_size_;
}

void TtSvgButton::setSvgSize(const int& w, const int& h) {
  setSvgSize(QSize(w, h));
}

void TtSvgButton::setSvgSize(const QSize& size) {
  if (svg_size_ != size) {
    svg_size_ = size;
    update();  // 触发重绘
  }
}

bool TtSvgButton::isChecked() const {
  return is_checked_;
}

void TtSvgButton::setChecked(bool checked) {
  if (is_checked_ != checked) {
    is_checked_ = checked;
    updateSvgContent();
    emit toggled(is_checked_);
  }
}

void TtSvgButton::setEnableHoldToCheck(bool enable) {
  enable_hold_to_check_ = enable;
}

void TtSvgButton::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // 绘制悬停背景
  if (is_hovered_) {
    painter.fillRect(rect(), hover_bg_color_);
    // painter.fillRect(QRect(svg_size_.), hover_bg_color_);
  }
  // 怀疑是 size 为 0
  // painter.fillRect(rect(), QColor(12, 12, 12));

  // int padding = 4;  // 边距
  // QRect svgRect(QPoint(padding, (height() - svg_size_.height()) / 2),
  //               svg_size_);
  // svg_renderer_->render(&painter, svgRect);

  // if (!text_.isEmpty()) {
  //   painter.setPen(current_color_);
  //   int textX = svgRect.right() + padding;
  //   QRect textRect(textX, 0, width() - textX - padding, height());
  //   painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text_);
  // }

  // 绘制SVG
  QRect svgRect;
  svgRect.setSize(svg_size_);
  svgRect.moveCenter(rect().center());
  svg_renderer_->render(&painter, svgRect);
}

void TtSvgButton::enterEvent(QEnterEvent* event) {
  // qDebug() << "enter";
  is_hovered_ = true;
  update();
  QWidget::enterEvent(event);
}

void TtSvgButton::leaveEvent(QEvent* event) {
  is_hovered_ = false;
  // qDebug() << "leave";
  update();
  QWidget::leaveEvent(event);
}

void TtSvgButton::mousePressEvent(QMouseEvent* event) {
  // qDebug() << "test";
  if (event->button() == Qt::LeftButton) {
    is_pressed_ = true;
    if (enable_hold_to_check_) {
      setChecked(true);
    }
    update();
  }
  QWidget::mousePressEvent(event);
}

void TtSvgButton::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton && is_pressed_) {
    is_pressed_ = false;
    if (enable_hold_to_check_) {
      setChecked(false);
    } else {
      setChecked(!is_checked_);
    }
    emit clicked();
    update();
  }
  QWidget::mouseReleaseEvent(event);
}

QSize TtSvgButton::sizeHint() const {
  QFontMetrics fm(font());
  int textWidth = fm.horizontalAdvance(text_);
  int padding = 4;
  int totalWidth = svg_size_.width() + textWidth + 3 * padding;
  int height = qMax(svg_size_.height(), fm.height()) + 2 * padding;

  // qDebug() << "size:" << QSize(totalWidth, height);
  return QSize(totalWidth, height);
}

void TtSvgButton::updateSvgContent() {
  QFile file(svg_path_);
  if (!file.open(QIODevice::ReadOnly)) {
    return;
  }

  QString svgContent = file.readAll();
  file.close();

  current_color_ = is_checked_ ? second_color_ : first_color_;
  // 替换颜色
  svgContent.replace(QRegularExpression("fill=\"[^\"]*\""),
                     QString("fill=\"%1\"").arg(current_color_.name()));

  svg_renderer_->load(svgContent.toUtf8());
  update();
}

TtImageButton::TtImageButton(const QString& svgPath, QWidget* parent)
    : QWidget(parent), svg_path_(svgPath), is_pressed_(false) {
  // setFixedSize(22, 22);  // 设置按钮初始大小
  setMinimumSize(22, 22);  // 设置按钮初始大小
}

void TtImageButton::setSvg(const QString& path) {
  svg_path_ = path;
  update();  // 重新绘制按钮
}


TtSpecialDeleteButton::TtSpecialDeleteButton(QWidget* parent)
    : QWidget(parent) {}

TtSpecialDeleteButton::TtSpecialDeleteButton(const QString& name,
                                             const QString& icon_path,
                                             const QString& delete_path,
                                             QWidget* parent)
    : QWidget(parent),
      is_hovered_(false),
      is_pressed_(false),
      icon_(icon_path),
      old_state_(false) {
  setObjectName("TtSpecialDeleteButton");
  layout_ = new Ui::TtHorizontalLayout(this);
  layout_->setContentsMargins(5, 5, 5, 5);  // 设置边距
  layout_->setSpacing(5);

  // 创建图标标签
  QLabel* iconLabel = new QLabel(this);
  iconLabel->setPixmap(
      icon_.scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  iconLabel->setFixedSize(QSize(20, 20));
  layout_->addWidget(iconLabel, 0, Qt::AlignLeft);

  // 设置文字标签
  name_ = new Ui::TtElidedLabel(name, this);
  name_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  layout_->addWidget(name_);

  delete_button_ = new TtSvgButton(delete_path, this);
  delete_button_->setSvgSize(QSize(12, 12));
  delete_button_->setColors(Qt::blue, Qt::black);
  delete_button_->setStyleSheet("background-color: transparent");
  layout_->addWidget(delete_button_, 0, Qt::AlignRight);

  connect(delete_button_, &Ui::TtSvgButton::clicked, this,
          &TtSpecialDeleteButton::deleteRequest);
}


void TtSpecialDeleteButton::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  QColor backgroundColor;
  // // 优先级：悬停 > 点击状态
  if (old_state_) {
    backgroundColor = QColor(186, 231, 255);  // 点击
  } else {
    backgroundColor = is_hovered_ ? QColor(229, 229, 229) : Qt::white;
  }
  painter.fillRect(rect(), backgroundColor);
  QWidget::paintEvent(event);
}

void TtSpecialDeleteButton::enterEvent(QEnterEvent* event) {
  is_hovered_ = true;
  update();
  QWidget::enterEvent(event);
}

void TtSpecialDeleteButton::leaveEvent(QEvent* event) {
  is_hovered_ = false;
  update();
  QWidget::leaveEvent(event);
}

void TtSpecialDeleteButton::resizeEvent(QResizeEvent* event) {
  QWidget::resizeEvent(event);
}

void TtSpecialDeleteButton::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    is_pressed_ = true;
    update();
  }
}

void TtSpecialDeleteButton::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton && is_pressed_) {
    is_pressed_ = false;
    update();
    emit clicked();
  }
}

TtTextButton::TtTextButton(QWidget* parent) : QPushButton(parent) {
  setStyleSheet("background-color: transparent");
}

TtTextButton::TtTextButton(const QString& text, QWidget* parent)
    : Ui::TtTextButton(parent) {
  setText(text);
}

TtTextButton::TtTextButton(const QColor& color, const QString& text,
                           QWidget* parent)
    : Ui::TtTextButton(parent) {
  QPalette palette = this->palette();
  palette.setColor(QPalette::ButtonText, color);
  setPalette(palette);
  setText(text);
}

TtTextButton::~TtTextButton() {}

}  // namespace Ui
