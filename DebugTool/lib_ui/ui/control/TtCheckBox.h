#ifndef UI_CONTROL_TTCHECKBOX_H
#define UI_CONTROL_TTCHECKBOX_H

#include <QCheckBox>

#include "ui/ui_pch.h"

namespace Ui {

class Tt_EXPORT TtCheckBox : public QCheckBox {
  Q_OBJECT
  Q_PROPERTY_CREATE(int, BorderRadius)
 public:
  explicit TtCheckBox(QWidget* parent = nullptr);
  explicit TtCheckBox(const QString& text, QWidget* parent = nullptr);
  ~TtCheckBox();
};

}  // namespace Ui

#endif  // UI_CONTROL_TTCHECKBOX_H
