#ifndef UI_CONTROL_TTBUTTONBOX_H
#define UI_CONTROL_TTBUTTONBOX_H

#include "ui/ui_pch.h"

namespace Ui {

class TtSpecialDeleteButton;

class Tt_EXPORT WidgetGroup : public QObject {
  Q_OBJECT
public:
  explicit WidgetGroup(QObject *parent = nullptr);
  void addButton(const QString &uuid, int specialType,
                 TtSpecialDeleteButton *button);
  void setCurrentIndex(const QString &index);
  int currentIndex() const;

  ///
  /// @brief setSpecificOptionStatus
  /// @param uuid
  /// @param state
  /// 根据窗口的运行状态改变
  void setSpecificOptionStatus(const QString &uuid, bool state);

signals:
  void currentIndexChanged(QString index, int type);

public slots:
  void updateUuid(const QString &index);

private slots:
  void handleButtonClicked();

private:
  TtSpecialDeleteButton *findButton(const QString &uuid);

  QMap<QPair<QString, int>, TtSpecialDeleteButton *> buttons_;
  int m_currentIndex;
  QString current_uuid_;
};

} // namespace Ui

#endif // UI_CONTROL_BUTTONBOX_H
