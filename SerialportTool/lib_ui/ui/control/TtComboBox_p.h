#ifndef UI_CONTROL_TTCOMBOBOX_P_H
#define UI_CONTROL_TTCOMBOBOX_P_H

#include "ui/TtTheme.h"

namespace style {
class TtComboBoxStyle;
}

namespace Ui {

class TtComboBox;

class TtComboBoxPrivate : public QObject {
  Q_OBJECT
  Q_D_CREATE(TtComboBox);
  Q_PROPERTY_CREATE_D(int, BorderRadius)

 public:
  explicit TtComboBoxPrivate(QObject* parent = nullptr);
  ~TtComboBoxPrivate();

 private:
  bool is_allow_hide_popup_ = false;
  style::TtComboBoxStyle* comboBox_style_{nullptr};
  TtThemeType::ThemeMode theme_mode_;
};

}  // namespace Ui

#endif  // UI_CONTROL_TTCOMBOBOX_P_H
