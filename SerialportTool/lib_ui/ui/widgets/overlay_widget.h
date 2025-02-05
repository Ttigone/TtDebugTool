/*****************************************************************/ /**
 * \file   overlay_widget.h
 * \brief  悬浮界面
 * 
 * \author C3H3_Ttigone
 * \date   February 2025
 *********************************************************************/

#ifndef UI_WIDGETS_OVERLAY_WIDGET_H
#define UI_WIDGETS_OVERLAY_WIDGET_H

#include <QWidget>

namespace Ui {

class TtOverlayWidget : public QWidget {
  Q_OBJECT

 public:
  explicit TtOverlayWidget(QWidget* parent = nullptr);
  TtOverlayWidget();

 protected:
  bool event(QEvent* event) override;
  bool eventFilter(QObject* obj, QEvent* event) override;

  virtual QRect overlayGeometry() const;

 private:
  Q_DISABLE_COPY(TtOverlayWidget)
};

}  // namespace Ui

#endif  // UI_WIDGETS_OVERLAY_WIDGET_H