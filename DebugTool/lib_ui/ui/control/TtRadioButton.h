#ifndef UI_CONTROL_TTRADIOBUTTON_H
#define UI_CONTROL_TTRADIOBUTTON_H

#include <QRadioButton>

#include <ui/ui_pch.h>

namespace Ui {

class TtRadioButtonPrivate;

class Tt_EXPORT TtRadioButton : public QRadioButton {
  Q_OBJECT
  Q_Q_CREATE(TtRadioButton)
 public:
  explicit TtRadioButton(QWidget* parent = nullptr);
  explicit TtRadioButton(const QString& text, QWidget* parent = nullptr);
  ~TtRadioButton();
};

}  // namespace Ui

#endif  // UI_CONTROL_TTRADIOBUTTON_H
