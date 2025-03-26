#include "TtDownloadDialog.h"

#include <ui/control/TtLineEdit.h>
#include <ui/widgets/buttons.h>

DownloadDialog::DownloadDialog(QWidget* parent) : QDialog(parent) {
  init();
  connectSignals();
}

DownloadDialog::~DownloadDialog() {}

void DownloadDialog::acceptedBtnClicked() {
  emit accepted(url_->currentText(), save_as_->currentText());
  this->accept();
}

void DownloadDialog::saveAsBtnClicked() {
  QString str = QFileDialog::getSaveFileName(this, "Save As");
  if (!str.isEmpty()) {
    save_as_->setText(str);
  }
}

void DownloadDialog::init() {
  QGridLayout* layout = new QGridLayout(this);

  url_ = new Ui::TtLabelLineEdit("URL", this);
  save_as_ = new Ui::TtLabelLineEdit("Save As", this);
  save_as_btn_ = new Ui::TtSvgButton(this);
  save_as_btn_->setText("选择路径");
  ok_btn_ = new Ui::TtTextButton("OK", this);
  cancle_btn_ = new Ui::TtTextButton("Cancle", this);

  layout->addWidget(url_, 0, 0, 1, 3);
  layout->addWidget(save_as_, 1, 0, 0, 2);
  layout->addWidget(save_as_btn_, 1, 2, 1, 1);

  layout->addWidget(ok_btn_, 2, 1, 1, 1, Qt::AlignRight);
  layout->addWidget(cancle_btn_, 2, 2, 1, 1, Qt::AlignLeft);
}

void DownloadDialog::connectSignals() {
  connect(ok_btn_, &Ui::TtTextButton::clicked, this,
          &DownloadDialog::acceptedBtnClicked);
  connect(save_as_btn_, &Ui::TtSvgButton::clicked, this,
          &DownloadDialog::saveAsBtnClicked);
}
