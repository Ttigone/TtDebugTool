#ifndef UI_CONTROL_TTLISTVIEW_P_H
#define UI_CONTROL_TTLISTVIEW_P_H

namespace style {
class TtListViewStyle;
}  // namespace style

namespace Ui {

class TtListView;
class TtListViewStyle;

class TtListViewPrivate : public QObject {
  Q_OBJECT
  Q_D_CREATE(TtListView)
 public:
  explicit TtListViewPrivate(QObject* parent = nullptr);
  ~TtListViewPrivate();

 private:
  style::TtListViewStyle* listView_style_{nullptr};
};

}  // namespace Ui

#endif  // TTLISTVIEW_P_H
