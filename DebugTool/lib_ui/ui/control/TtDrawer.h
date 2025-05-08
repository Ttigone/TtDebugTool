#ifndef UI_CONTROL_TTDRAWER_H
#define UI_CONTROL_TTDRAWER_H

#include "ui/ui_pch.h"
#include <QWidget>

namespace Ui {
class TtElidedLabel;
class TtSvgButton;
class TtVerticalLayout;
} // namespace Ui

namespace Ui {

class Tt_EXPORT TtDrawer : public QWidget {
  Q_OBJECT
public:
  explicit TtDrawer(QWidget *parent = nullptr);
  explicit TtDrawer(const QString &title, const QString &closeImagePath,
                    const QString &openImagePath, QWidget *bodyWidget,
                    bool enable, QWidget *parent = nullptr);
  ~TtDrawer();

  void setDrawerStatus(bool open);

signals:
  void drawerStateChanged(bool state);

private:
  Ui::TtVerticalLayout *main_layout_;
  // 右侧
  Ui::TtSvgButton *fold_btn_;
  Ui::TtElidedLabel *label_;

  QWidget *body_;
  // Ui::TtSvgButton *icon_;
  bool drawer_status_;
};

} // namespace Ui
#endif // TTDRAWER_H
