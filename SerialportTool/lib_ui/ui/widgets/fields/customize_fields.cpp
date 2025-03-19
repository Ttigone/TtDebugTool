#include "ui/widgets/fields/customize_fields.h"

namespace Ui {

// TtLineEdit::TtLineEdit(QWidget* parent) : QLineEdit(parent), max_len_(15) {
//   init();
// }

// TtLineEdit::TtLineEdit(const QString& text, QWidget* parent)
//     : TtLineEdit(parent) {
//   setDefaultText(text);
// }

// TtLineEdit::~TtLineEdit() {}

// void TtLineEdit::setMaxWidth(int width) {
//   max_len_ = width;
// }

// void TtLineEdit::setDefaultText(const QString& text) {
//   setPlaceholderText(text);
// }

// void TtLineEdit::focusOutEvent(QFocusEvent* event) {
//   QLineEdit::focusOutEvent(event);
//   emit sig_foucus_out();
// }

// void TtLineEdit::init() {
//   this->setStyleSheet(
//       // "background-color: #2e333d;"
//       "background-color: transparent;"
//       "border: 0px;"
//       "border-radius: 2;"  // 设置圆角半径为10px
//                            // "padding: 10px;"         // 内边距
//   );

//   //this->setContentsMargins(QMargins(10, 5, 10, 5));

//   //  front_action_ = new QAction(this);
//   //  front_action_->setIcon(QIcon(":/icon/ui/icons/mediaview/search-normal.png"));
//   //  this->addAction(front_action_, QLineEdit::LeadingPosition);

//   //  end_action_ = new QAction(this);
//   // end_action_->setIcon(QIcon(":/icon/ui/icons/mediaview/circle_close-normal.png"));
//   //  this->addAction(end_action_, QLineEdit::TrailingPosition);
//   //  end_action_->setVisible(false);
//   // end_action_->setEnabled(false);

//   // this->setPlaceholderText("Search");

//   // connect(this, &QLineEdit::textChanged, this,
//   //         &TtLineEdit::textChangedCallBack);
//   // connect(end_action_, &QAction::triggered, this, [this]() {
//   //   this->clear();
//   //   this->clearFocus();
//   // });
// }

// void TtLineEdit::textChangedCallBack(const QString& text) {
//   limitTextLength(text);
//   if (!text.isEmpty()) {
//     //end_action_->setIcon(QIcon(":/icon/ui/icons/mediaview/circle_close-normal.png"));
//     // end_action_->setEnabled(true);
//     // end_action_->setVisible(true);
//   } else {
//     //end_action_->setIcon(QIcon());
//     // end_action_->setVisible(false);
//     // end_action_->setEnabled(false);
//   }
// }

// void TtLineEdit::limitTextLength(QString text) {
//   if (max_len_ < 0) {
//     return;
//   }
//   QByteArray byteArray = text.toUtf8();
//   // 超过长度
//   if (byteArray.size() > max_len_) {
//     // 去掉超过的部分
//     byteArray = byteArray.left(max_len_);
//     this->setText(QString::fromUtf8(byteArray));
//   }
// }

// TtLabelLineEdit::TtLabelLineEdit(Qt::AlignmentFlag flag, const QString& text,
//                                  QWidget* parent)
//     : QWidget(parent) {
//   line_edit_ = new TtLineEdit(this);
//   line_edit_->setMinimumWidth(80);

//   label_ = new QLabel(text, this);
//   // 设置 tip
//   label_->setToolTip(text);
//   label_->setFixedWidth(60);

//   QHBoxLayout* layout = new QHBoxLayout(this);
//   layout->setContentsMargins(QMargins());
//   layout->setSpacing(5);

//   switch (flag) {
//     case Qt::AlignLeft:
//       layout->addWidget(label_);
//       layout->addWidget(line_edit_, 1);  // 添加拉伸因子
//       break;
//     case Qt::AlignRight:
//       layout->addStretch();
//       layout->addWidget(label_);
//       layout->addWidget(line_edit_, 1);
//       break;
//     case Qt::AlignHCenter:
//       layout->addStretch();
//       layout->addWidget(label_);
//       layout->addWidget(line_edit_, 1);
//       layout->addStretch();
//       break;
//     default:
//       layout->addWidget(label_);
//       layout->addWidget(line_edit_, 1);
//       break;
//   }
// }

// TtLabelLineEdit::TtLabelLineEdit(const QString& text, QWidget* parent)
//     : TtLabelLineEdit(Qt::AlignLeft, text, parent) {}

// TtLabelLineEdit::~TtLabelLineEdit() {}

// TtLineEdit* TtLabelLineEdit::body() {
//   return line_edit_;
// }

// QString TtLabelLineEdit::currentText() {
//   return line_edit_->text();
// }

}  // namespace Ui
