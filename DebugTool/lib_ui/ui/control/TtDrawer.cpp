#include "TtDrawer.h"

#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>

namespace Ui {

TtDrawer::TtDrawer(QWidget* parent) : QWidget(parent) {}

TtDrawer::TtDrawer(const QString& title, const QString& closeImagePath,
                   const QString& openImagePath, QWidget* bodyWidget,
                   bool enable, QWidget* parent)
    : QWidget(parent), body_(bodyWidget), drawer_status_(enable) {
  main_layout_ = new Ui::TtVerticalLayout(this);
  label_ = new Ui::TtElidedLabel(title, this);
  fold_btn_ = new Ui::TtSvgButton(closeImagePath, this);

  Ui::TtHorizontalLayout* headerLayout = new Ui::TtHorizontalLayout();
  headerLayout->addWidget(label_);
  headerLayout->addStretch();
  headerLayout->addWidget(fold_btn_);

  main_layout_->addLayout(headerLayout);
  main_layout_->addWidget(bodyWidget);

  bodyWidget->setVisible(enable);

  connect(fold_btn_, &Ui::TtSvgButton::clicked, this,
          [this, openImagePath, closeImagePath]() {
            drawer_status_ ^= 1;
            body_->setVisible(drawer_status_);
            if (drawer_status_) {
              fold_btn_->setSvgPath(openImagePath);
            } else {
              fold_btn_->setSvgPath(closeImagePath);
            }
          });
}

TtDrawer::~TtDrawer() {}

void TtDrawer::setDrawerStatus(bool open) {
  drawer_status_ = open;
}

}  // namespace Ui
