#include "ui/controls/TtChannelButtonEditorDialog.h"

TtChannelButtonEditorDialog::TtChannelButtonEditorDialog(QWidget* parent)
    : QDialog(parent) {
  this->resize(500, 320);
  setWindowTitle(tr("编辑通道属性"));
  QVBoxLayout* mainLayout = new QVBoxLayout(this);

  title_edit_ = new Ui::TtLabelLineEdit(tr("名称"), this);
  title_edit_->setText("Untitled");
  frame_header_ = new Ui::TtLabelLineEdit(tr("帧头(HEX)"), this);
  frame_header_length_ = new Ui::TtLabelLineEdit(tr("帧头长度"), this);
  frame_type_offset_ = new Ui::TtLabelLineEdit(tr("类型字段偏移"), this);
  frame_length_offset_ = new Ui::TtLabelLineEdit(tr("长度字段偏移"), this);
  frame_length_ = new Ui::TtLabelLineEdit(tr("长度字段(字节)"), this);
  frame_length_->setText("1");
  frame_end_ = new Ui::TtLabelLineEdit(tr("帧尾"), this);

  mainLayout->addWidget(title_edit_);
  mainLayout->addWidget(frame_header_);
  mainLayout->addWidget(frame_header_length_);
  mainLayout->addWidget(frame_type_offset_);
  mainLayout->addWidget(frame_length_offset_);
  mainLayout->addWidget(frame_length_);
  mainLayout->addWidget(frame_end_);

  Ui::TtFancyButton* scriptBtton =
      new Ui::TtFancyButton(Qt::white, tr("脚本编辑"), this);
  mainLayout->addWidget(scriptBtton);

  connect(scriptBtton, &Ui::TtFancyButton::clicked, this, [this]() {

  });

  // QHBoxLayout* colorLayout = new QHBoxLayout;
  // colorLayout->addWidget(new QLabel(tr("背景色:"), this));
  // m_colorButton =
  //     new Ui::TtFancyButton(m_button->getColor(), tr("选择颜色"), this);
  // connect(m_colorButton, &Ui::TtFancyButton::clicked, this, [this]() {
  //   QColor chosen = QColorDialog::getColor(m_button->getColor(), this,
  //                                          tr("选择背景颜色"));
  //   if (chosen.isValid()) {
  //     m_chosenColor = chosen;
  //     m_colorButton->setColor(m_chosenColor);
  //   }
  // });
  // colorLayout->addWidget(m_colorButton);
  // mainLayout->addLayout(colorLayout);

  QHBoxLayout* checkBlockLayout = new QHBoxLayout;
  checkBlockLayout->addWidget(new QLabel(tr("勾选块色:"), this));

  // m_checkBlockButton = new Ui::TtFancyButton(m_button->getCheckBlockColor(),
  //                                            tr("选择颜色"), this);

  int red = QRandomGenerator::global()->bounded(0, 256);
  int green = QRandomGenerator::global()->bounded(0, 256);
  int blue = QRandomGenerator::global()->bounded(0, 256);
  QColor randomCheckColor = QColor(red, green, blue, 100);
  m_checkBlockButton =
      new Ui::TtFancyButton(randomCheckColor, tr("选择颜色"), this);
  m_chosenCheckBlockColor = randomCheckColor;

  connect(m_checkBlockButton, &Ui::TtFancyButton::clicked, this, [this]() {
    QColor chosen = QColorDialog::getColor(m_chosenCheckBlockColor, this,
                                           tr("选择背景颜色"));
    if (chosen.isValid()) {
      m_chosenCheckBlockColor = chosen;
      m_checkBlockButton->setColor(m_chosenCheckBlockColor);
    }
  });
  checkBlockLayout->addWidget(m_checkBlockButton);

  mainLayout->addLayout(checkBlockLayout);

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  QPushButton* okButton = new QPushButton(tr("确定"), this);
  QPushButton* cancelButton = new QPushButton(tr("取消"), this);
  buttonLayout->addWidget(okButton);
  buttonLayout->addWidget(cancelButton);
  mainLayout->addLayout(buttonLayout);

  connect(okButton, &QPushButton::clicked, this,
          &TtChannelButtonEditorDialog::applyChanges);
  connect(cancelButton, &QPushButton::clicked, this,
          &TtChannelButtonEditorDialog::reject);
  qDebug() << "test";
}

TtChannelButtonEditorDialog::TtChannelButtonEditorDialog(
    TtChannelButton* button, QWidget* parent)
    : QDialog(parent), m_button(button) {
  this->resize(500, 320);
  setWindowTitle(tr("编辑通道属性"));
  QVBoxLayout* mainLayout = new QVBoxLayout(this);

  title_edit_ = new Ui::TtLabelLineEdit(tr("名称"), this);
  title_edit_->setText(m_button->getText());
  frame_header_ = new Ui::TtLabelLineEdit(tr("帧头"), this);
  frame_header_length_ = new Ui::TtLabelLineEdit(tr("帧头长度"), this);
  frame_type_offset_ = new Ui::TtLabelLineEdit(tr("类型字段偏移"), this);
  frame_length_offset_ = new Ui::TtLabelLineEdit(tr("长度字段频偏移"), this);
  frame_length_ = new Ui::TtLabelLineEdit(tr("长度字段(字节)"), this);
  frame_length_->setText("1");
  frame_end_ = new Ui::TtLabelLineEdit(tr("帧尾"), this);

  mainLayout->addWidget(title_edit_);
  mainLayout->addWidget(frame_header_);
  mainLayout->addWidget(frame_header_length_);
  mainLayout->addWidget(frame_type_offset_);
  mainLayout->addWidget(frame_length_offset_);
  mainLayout->addWidget(frame_length_);
  mainLayout->addWidget(frame_end_);

  Ui::TtFancyButton* scriptBtton =
      new Ui::TtFancyButton(Qt::white, tr("脚本编辑"), this);
  mainLayout->addWidget(scriptBtton);

  connect(scriptBtton, &Ui::TtFancyButton::clicked, this, [this]() {

  });

  // QHBoxLayout* colorLayout = new QHBoxLayout;
  // colorLayout->addWidget(new QLabel(tr("背景色:"), this));
  // m_colorButton =
  //     new Ui::TtFancyButton(m_button->getColor(), tr("选择颜色"), this);
  // connect(m_colorButton, &Ui::TtFancyButton::clicked, this, [this]() {
  //   QColor chosen = QColorDialog::getColor(m_button->getColor(), this,
  //                                          tr("选择背景颜色"));
  //   if (chosen.isValid()) {
  //     m_chosenColor = chosen;
  //     m_colorButton->setColor(m_chosenColor);
  //   }
  // });
  // colorLayout->addWidget(m_colorButton);
  // mainLayout->addLayout(colorLayout);

  QHBoxLayout* checkBlockLayout = new QHBoxLayout;
  checkBlockLayout->addWidget(new QLabel(tr("勾选块色:"), this));

  m_checkBlockButton = new Ui::TtFancyButton(m_button->getCheckBlockColor(),
                                             tr("选择颜色"), this);
  connect(m_checkBlockButton, &Ui::TtFancyButton::clicked, this, [this]() {
    QColor chosen = QColorDialog::getColor(m_button->getCheckBlockColor(), this,
                                           tr("选择背景颜色"));
    if (chosen.isValid()) {
      m_chosenCheckBlockColor = chosen;
      m_checkBlockButton->setColor(m_chosenCheckBlockColor);
    }
  });
  checkBlockLayout->addWidget(m_checkBlockButton);

  mainLayout->addLayout(checkBlockLayout);

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  QPushButton* okButton = new QPushButton(tr("确定"), this);
  QPushButton* cancelButton = new QPushButton(tr("取消"), this);
  buttonLayout->addWidget(okButton);
  buttonLayout->addWidget(cancelButton);
  mainLayout->addLayout(buttonLayout);

  connect(okButton, &QPushButton::clicked, this,
          &TtChannelButtonEditorDialog::applyChanges);
  connect(cancelButton, &QPushButton::clicked, this,
          &TtChannelButtonEditorDialog::reject);
}

TtChannelButtonEditorDialog::~TtChannelButtonEditorDialog() {
  qDebug() << "dilog delete";
}

void TtChannelButtonEditorDialog::setMetaInfo(const QByteArray& meta) {
  QDataStream in(meta);
  in.setVersion(QDataStream::Qt_6_4);

  QString title, header, headerLength, typeOffset, lengthOffset, length, tail;
  in >> title >> header >> headerLength >> typeOffset >> lengthOffset >>
      length >> tail;

  title_edit_->setText(title);
  frame_header_->setText(header);
  frame_header_length_->setText(headerLength);
  frame_type_offset_->setText(typeOffset);
  frame_length_offset_->setText(lengthOffset);
  frame_length_->setText(length);  // 长度字段字节数
  frame_end_->setText(tail);
}

QByteArray TtChannelButtonEditorDialog::metaInfo() {
  QByteArray buffer;
  QDataStream out(&buffer, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_6_4);

  out << title_edit_->currentText() << frame_header_->currentText().trimmed()
      << frame_header_length_->currentText()
      << frame_type_offset_->currentText()
      << frame_length_offset_->currentText() << frame_length_->currentText()
      << frame_end_->currentText();

  return buffer;
}

QString TtChannelButtonEditorDialog::title() const {
  return title_edit_->currentText();
}

QColor TtChannelButtonEditorDialog::checkBlockColor() const {
  if (m_chosenCheckBlockColor.isValid()) {
    return m_chosenCheckBlockColor;
  }
  return QColor();
}

void TtChannelButtonEditorDialog::applyChanges() {
  if (m_button) {
    // m_button->setText(title_edit_->currentText());
    // if (m_chosenColor.isValid()) {
    //   m_button->setColors(m_chosenColor);
    // }
    if (m_chosenCheckBlockColor.isValid()) {
      m_button->setCheckBlockColor(m_chosenCheckBlockColor);
    }
  }
  accept();
}
