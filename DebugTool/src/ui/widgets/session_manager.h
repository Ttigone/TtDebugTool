#ifndef UI_SESSION_MANAGER_H
#define UI_SESSION_MANAGER_H

#include <QEvent>
#include <QListWidget>

namespace Ui {

class SessionManager : public QListWidget {
  Q_OBJECT
 public:
  SessionManager(QWidget* parent = nullptr);
  ~SessionManager() = default;

  bool addAdaptiveWidget(const QString& title, const QString& uuid,
                         QWidget* widget);

 protected:
  void resizeEvent(QResizeEvent* event) override {
    QListWidget::resizeEvent(event);
    updateAllItemSizes();  // 在 resizeEvent 中更新所有项的大小
  }

 signals:
  void uuidsChanged(const QString& index);

 private:
  void updateItemSize(QListWidgetItem* item);

  void updateAllItemSizes();

  // QVector<QString> uuids_;
  QMap<QString, QListWidgetItem*> uuids_;
};

}  // namespace Ui

#endif  // UI_SESSION_MANAGER_H
