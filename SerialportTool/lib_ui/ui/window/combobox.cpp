#include "ui/window/combobox.h"

#include "ui/widgets/buttons.h"

#include <QHBoxLayout>
#include <QRegularExpression>

namespace Ui {

TtComboBox::TtComboBox(QWidget* parent) : QComboBox(parent) {
  setEditable(true);
  //setSizeAdjustPolicy(QComboBox::AdjustToContents); // 自动调整宽度
}

TtComboBox::~TtComboBox() {}

TtLabelComboBox::TtLabelComboBox(Qt::AlignmentFlag flag, const QString& text,
                                 QWidget* parent)
    : QWidget(parent) {
  combo_box_ = new TtComboBox(this);
  //combo_box_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  combo_box_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  combo_box_->setMinimumWidth(80);

  label_ = new QLabel(text, this);
  // 设置 tip
  label_->setToolTip(text);
  label_->setFixedWidth(60);

  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(5);

  switch (flag) {
    case Qt::AlignLeft:
      layout->addWidget(label_);
      layout->addWidget(combo_box_, 1);  // 添加拉伸因子
      break;
    case Qt::AlignRight:
      layout->addStretch();
      layout->addWidget(label_);
      layout->addWidget(combo_box_, 1);
      break;
    case Qt::AlignHCenter:
      layout->addStretch();
      layout->addWidget(label_);
      layout->addWidget(combo_box_, 1);
      layout->addStretch();
      break;
    default:
      layout->addWidget(label_);
      layout->addWidget(combo_box_, 1);
      break;
  }
  //if (flag & Qt::AlignLeft) {
  //  // 左
  //  // layout->addStretch();
  //  // layout->addWidget(label_, Qt::AlignLeft);
  //  // layout->addWidget(combo_box_, Qt::AlignLeft);
  //  // layout->addStretch();
  //  layout->addWidget(label_);
  //  // layout->addSpacerItem(new QSpacerItem(10, 10));
  //  layout->addWidget(combo_box_);
  //} else if (flag & Qt::AlignRight) {

  //} else if (flag & (Qt::AlignTop | Qt::AlignHCenter)) {
  //  // 水平居中
  //} else if (flag & (Qt::AlignTop | Qt::AlignLeft)) {
  //}
  //setLayout(layout);

  connect(combo_box_, &QComboBox::currentIndexChanged, this,
          &TtLabelComboBox::currentIndexChanged);
}

TtLabelComboBox::TtLabelComboBox(const QString& text, QWidget* parent)
    : Ui::TtLabelComboBox(Qt::AlignLeft, text, parent) {}

TtLabelComboBox::~TtLabelComboBox() {}

QComboBox* TtLabelComboBox::body() {
  return combo_box_;
}

void TtLabelComboBox::addItem(const QString& atext, const QVariant& auserData) {
  combo_box_->addItem(atext, auserData);
}

void TtLabelComboBox::setCurrentItem(qint8 index) {
  combo_box_->setCurrentIndex(index);
}

QString TtLabelComboBox::itemText(int index) {
  return combo_box_->itemText(index);
}

QString TtLabelComboBox::currentText() {
  return combo_box_->currentText();
}

int TtLabelComboBox::count() {
  return combo_box_->count();
}

void TtLabelComboBox::shortCurrentItemText() {
  // 虚拟串口一样
  QRegularExpression regex("COM\\d+");
  QRegularExpressionMatch match = regex.match(combo_box_->currentText());
  if (match.hasMatch()) {
    combo_box_->setCurrentText(match.captured(0));
  } else {
  }
}

TtLabelBtnComboBox::TtLabelBtnComboBox(Qt::AlignmentFlag flag,
                                       const QString& text,
                                       const QString& image_path,
                                       QWidget* parent)
    : QWidget(parent) {
  QHBoxLayout* layout = new QHBoxLayout(this);
  part_ = new TtLabelComboBox(flag, text, this);
  //auto refresh_btn = new Ui::CommonButton(":/sys/refresh-normal.svg",
  //                                        ":/sys/refresh-hover.svg", this);
  auto refresh_btn = new Ui::TtSvgButton(":/sys/refresh-normal.svg",
                                         ":/sys/refresh-hover.svg", this);
  refresh_btn->setEnableToggle(true);

  layout->addWidget(part_);
  layout->addWidget(refresh_btn);
  connect(refresh_btn, &Ui::TtSvgButton::clicked, [this]() {
    qDebug() << "clicked";
    emit clicked();
  });
}

TtLabelBtnComboBox::TtLabelBtnComboBox(const QString& text, QWidget* parent)
    : QWidget(parent) {
  part_ = new TtLabelComboBox(text, this);
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);
  auto refresh_btn = new Ui::TtSvgButton(":/sys/refresh-normal.svg",
                                          ":/sys/refresh-hover.svg", this);
  refresh_btn->setEnableToggle(true);
  refresh_btn->setObjectName("test");

  layout->addWidget(part_);
  layout->addWidget(refresh_btn);

  connect(part_, &TtLabelComboBox::currentIndexChanged, this,
          &TtLabelBtnComboBox::displayCurrentCOMx);

  connect(refresh_btn, &Ui::TtSvgButton::clicked, [this]() {
    //qDebug() << "clicked";
    emit clicked();
  });
}

TtLabelBtnComboBox::~TtLabelBtnComboBox() {}

void TtLabelBtnComboBox::addItem(const QString& atext,
                                 const QVariant& auserData) {
  // 只有 current, 才会截取显示
  part_->addItem(atext, auserData);
}

QComboBox* TtLabelBtnComboBox::body() {
  return part_->body();
}

void TtLabelBtnComboBox::setCurrentItem(qint8 index) {
  part_->setCurrentItem(index);
}

QString TtLabelBtnComboBox::itemText(int index) {
  return part_->itemText(index);
}

QString TtLabelBtnComboBox::currentText() {
  return part_->currentText();
}

int TtLabelBtnComboBox::count() {
  return part_->count();
}

void TtLabelBtnComboBox::displayCurrentCOMx() {
  part_->shortCurrentItemText();
}

}  // namespace Ui
