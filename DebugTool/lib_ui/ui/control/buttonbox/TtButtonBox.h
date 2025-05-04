#ifndef UI_CONTROL_TTBUTTONBOX_H
#define UI_CONTROL_TTBUTTONBOX_H

#include "ui/ui_pch.h"

namespace Ui {

class TtSpecialDeleteButton;

class Tt_EXPORT WidgetGroup : public QObject {
  Q_OBJECT
 public:
  explicit WidgetGroup(QObject* parent = nullptr);
  // void addButton(TtSpecialDeleteButton* button);
  void addButton(const QString& uuid, int specialType,
                 TtSpecialDeleteButton* button);
  // void setCurrentIndex(int index);
  void setCurrentIndex(QString index);
  int currentIndex() const;

 signals:
  // // void currentIndexChanged(int index);
  // void currentIndexChanged(QString index);
  void currentIndexChanged(QString index, int type);

 public slots:
  void updateUuid(const QString& index);

 private slots:
  void handleButtonClicked();

 private:
  TtSpecialDeleteButton* findButton(const QString& uuid);

  // QList<TtSpecialDeleteButton*> m_buttons;
  // QMap<QString, TtSpecialDeleteButton*> buttons_;
  QMap<QPair<QString, int>, TtSpecialDeleteButton*> buttons_;
  int m_currentIndex;
  QString current_uuid_;
};

} // namespace Ui

#endif  // UI_CONTROL_BUTTONBOX_H
