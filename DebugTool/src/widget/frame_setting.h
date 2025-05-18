#ifndef WIDGET_FRAME_SETTING_H
#define WIDGET_FRAME_SETTING_H

#include "Def.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
class QComboBox;
class QLineEdit;
QT_END_NAMESPACE

namespace Widget {

class FrameSetting : public QWidget {
  Q_OBJECT
public:
  explicit FrameSetting(QWidget *parent = nullptr);
  virtual ~FrameSetting();

  ///
  /// @brief link
  /// 链接信号槽
  void link();

signals:
  void settingChanged();
  void showScriptSetting();
  void sendPackageMaxSizeChanged(uint16_t size);
  void sendPackageIntervalChanged(uint16_t interval);
  void heartbeatInterval(uint32_t interval);
  void heartbeatContentChanged(QString content);
  void heartbeatType(TtTextFormat::Type type);

protected:
  void addComboBox(QComboBox *);
  void addLineEdit(QLineEdit *);

private:
  QList<QComboBox *> comboBoxes_;
  QList<QLineEdit *> lineEdits_;
};

} // namespace Widget

#endif // WIDGET_FRAME_SETTING_H
