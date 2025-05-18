#ifndef CORE_FRAMESETTINGHELPER_H
#define CORE_FRAMESETTINGHELPER_H

#include <QObject>

namespace Core {

class FrameSettingHelper : public QObject {
  Q_OBJECT
public:
  explicit FrameSettingHelper(QObject *parent = nullptr);
  ~FrameSettingHelper();

signals:
  // 信号, 对应设置改变时发出
  void settingChanged();
};

} // namespace Core

#endif // CORE_FRAMESETTINGHELPER_H
