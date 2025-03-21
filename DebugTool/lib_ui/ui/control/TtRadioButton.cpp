#include "ui/control/TtRadioButton.h"
#include "ui/control/TtRadioButton_p.h"

#include "ui/TtTheme.h"
#include "ui/style/TtRadioButtonStyle.h"

namespace Ui {

TtRadioButton::TtRadioButton(QWidget* parent)
    : QRadioButton(parent), d_ptr(new TtRadioButtonPrivate) {
  Q_D(TtRadioButton);
  d->q_ptr = this;
  setFixedHeight(20);
  QFont font = this->font();
  font.setPixelSize(15);
  setFont(font);
  setStyle(new style::TtRadioButtonStyle(style()));
  d->onThemeChanged(tTheme->getThemeMode());
  connect(tTheme, &TtTheme::themeModeChanged, d,
          &TtRadioButtonPrivate::onThemeChanged);
}

TtRadioButton::TtRadioButton(const QString& text, QWidget* parent)
    : Ui::TtRadioButton(parent) {
  setText(text);
}

TtRadioButton::~TtRadioButton() {}

Ui::TtRadioButtonPrivate::TtRadioButtonPrivate(QObject* parent) {}

Ui::TtRadioButtonPrivate::~TtRadioButtonPrivate() {}

void Ui::TtRadioButtonPrivate::onThemeChanged(
    TtThemeType::ThemeMode themeMode) {
  Q_Q(TtRadioButton);
  QPalette palette = q->palette();
  palette.setColor(QPalette::WindowText,
                   Ui::TtThemeColor(themeMode, BasicText));
  q->setPalette(palette);
}

}  // namespace Ui
