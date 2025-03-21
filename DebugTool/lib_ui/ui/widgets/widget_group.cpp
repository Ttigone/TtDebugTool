#include "ui/widgets/widget_group.h"

namespace Ui {
TtWidgetGroup::TtWidgetGroup(QObject* parent)
    : QObject(parent), exclusive_(false), checked_widget_(nullptr) {
}
TtWidgetGroup::~TtWidgetGroup() {}
}  // namespace Ui
