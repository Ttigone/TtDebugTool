#ifndef UI_SUBSCRIPITION_MANAGER_H
#define UI_SUBSCRIPITION_MANAGER_H

#include <ui/control/TtListView.h>

namespace Ui {

class SubscripitionManager : public Ui::TtListView {
 public:
  SubscripitionManager(QWidget* parent = nullptr);
  ~SubscripitionManager();
};

}  // namespace Ui

#endif  // UI_SUBSCRIPITION_MANAGER_H
