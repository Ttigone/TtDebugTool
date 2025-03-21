#include "TtCheckBox.h"

#include "ui/style/TtCheckBoxStyle.h"

namespace Ui {

TtCheckBox::TtCheckBox(QWidget* parent) : QCheckBox(parent) {
  pBorderRadius_ = 3;
  setMouseTracking(true);
  setObjectName("TtCheckBox");
  setStyle(new style::TtCheckBoxStyle(style()));
  QFont font = this->font();
  font.setPixelSize(15);
  setFont(font);
}

TtCheckBox::TtCheckBox(const QString& text, QWidget* parent)
    : Ui::TtCheckBox(parent) {
  setText(text);
}

TtCheckBox::~TtCheckBox() {}

}  // namespace Ui
