#ifndef POPUP_WINDOW_H
#define POPUP_WINDOW_H

#include <QStackedLayout>

namespace Ui {
class TtVerticalLayout;
class TtHorizontalLayout;
class TtNormalLabel;
}  // namespace Ui

namespace Window {

class PopUpWindow : public QWidget {
 public:
  PopUpWindow(QWidget* parent = nullptr);

  void addPairedWidget(int index, QWidget* widget);

  //void setWidget(QWidget *widget);
  void registerWidget(int index, QWidget* widget);

 public slots:
  bool switchToWidget(int index);

 private:
  void init();

  QMap<int, QWidget *> registered_widgets_;
  QStackedLayout* layout_;

};

}  // namespace Window

#endif  // POPUP_WINDOW_H
