#ifndef UI_CONTROL_TTLINEEDIT_P_H
#define UI_CONTROL_TTLINEEDIT_P_H

#include "ui/TtTheme.h"

namespace Ui {

class TtLineEdit;

class TtLineEditPrivate : public QObject {
  Q_OBJECT
  Q_D_CREATE(TtLineEdit)
  Q_PROPERTY_CREATE_D(int, BorderRadius)
  Q_PROPERTY_CREATE_D(bool, IsClearButtonEnable)
  Q_PROPERTY_CREATE_D(bool, IsReadOnlyNoClearButtonEnable)
  Q_PROPERTY_CREATE(qreal, ExpandMarkWidth)
 public:
  explicit TtLineEditPrivate(QObject* parent = nullptr);
  ~TtLineEditPrivate();
  Q_INVOKABLE void onWMWindowClickedEvent(QVariantMap data);
  int calculateTargetWidth() const;

 public slots:
  void onThemeChanged(TtThemeType::ThemeMode themeMode);

 private:
  void init();
  TtThemeType::ThemeMode theme_mode_;
  // ElaEvent* _focusEvent{nullptr};
  qreal text_spacing_{0.5};
  QPropertyAnimation* pMarkAnimation = nullptr;  // 保存动画指针
};

}  // namespace Ui

#endif  // UI_CONTROL_TTLINEEDIT_P_H
