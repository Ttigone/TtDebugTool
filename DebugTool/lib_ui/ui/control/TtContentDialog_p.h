#ifndef TTCONTENTDIALOG_P_H
#define TTCONTENTDIALOG_P_H

#include "ui/Def.h"

class QHBoxLayout;
class QVBoxLayout;

namespace Ui {

class TtTextButton;

class TtContentDialog;
class TtContentDialogPrivate : public QObject {
  Q_OBJECT
  Q_D_CREATE(TtContentDialog)
 public:
  explicit TtContentDialogPrivate(QObject* parent = nullptr);
  ~TtContentDialogPrivate();

 private:
  // TtApplicationType::ThemeMode _themeMode;
  TtThemeType::ThemeMode _themeMode;
  QWidget* _shadowWidget{nullptr};
  QWidget* _centralWidget{nullptr};
  QVBoxLayout* _mainLayout{nullptr};
  QHBoxLayout* _buttonLayout{nullptr};

  QString _leftButtonText{"cancel"};
  QString _middleButtonText{"minimum"};
  QString _rightButtonText{"exit"};

  TtTextButton* _leftButton{nullptr};
  TtTextButton* _middleButton{nullptr};
  TtTextButton* _rightButton{nullptr};

  QString center_text_;
  QLabel* content_{nullptr};

  QWidget* main_window_{nullptr};  // 监控的父窗口
  QWidget* shadow_widget_{nullptr};
  bool enable_point_on_mouse_{false};
};

}  // namespace Ui

#endif  // TTCONTENTDIALOG_P_H
