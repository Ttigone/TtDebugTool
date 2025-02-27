#ifndef WINDOW_SWITCHER_H
#define WINDOW_SWITCHER_H

#include <qmimedata.h>
#include <qtimer.h>
#include <QMap>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QProxyStyle>
#include <QStackedLayout>
#include <QState>
#include <QStateMachine>
#include <QTabWidget>

#include <QToolButton>
#include <functional>

namespace Ui {

class TabCloseButton : public QToolButton {
  Q_OBJECT
 public:
  TabCloseButton(QWidget* parent = nullptr) : QToolButton(parent) {
    setFixedSize(16, 16);
    setCursor(Qt::PointingHandCursor);
  }
  ~TabCloseButton() = default;

 protected:
  virtual void paintEvent(QPaintEvent* event) override {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    if (underMouse()) {
      painter.setBrush(QColor(200, 200, 200, 150));
      painter.setPen(Qt::NoPen);
      painter.drawEllipse(rect());
    }

    // 绘制关闭图标
    painter.setPen(QPen(isEnabled() ? Qt::black : Qt::gray, 1.5));
    int margin = 4;
    painter.drawLine(margin, margin, width() - margin, height() - margin);
    painter.drawLine(width() - margin, margin, margin, height() - margin);
  }
  void enterEvent(QEnterEvent* event) override {
    QToolButton::enterEvent(event);
  }
  void leaveEvent(QEvent* event) override { QToolButton::leaveEvent(event); }
};

class ExtTabBar : public QTabBar {
  Q_OBJECT

 public:
  explicit ExtTabBar(QWidget* parent = nullptr) : QTabBar(parent) {
    setMovable(true);
    setUsesScrollButtons(true);
    setDrawBase(false);
    setExpanding(false);
    setElideMode(Qt::ElideRight);
  }

 signals:
  void addTabRequested();

 protected:
  // void dragEnterEvent(QDragEnterEvent* event) {
  //   if (event->mimeData()->hasUrls())
  //     event->acceptProposedAction();
  //   else
  //     event->ignore();
  // }
  // void dropEvent(QDropEvent* event) {
  //   const QMimeData* mimeData = event->mimeData();
  //   if (mimeData->hasUrls()) {
  //     for (const QUrl& url : mimeData->urls()) {
  //       // emit openFileRequest(url.toLocalFile(), tabAt(event->pos()));
  //     }
  //     event->acceptProposedAction();
  //   } else {
  //     event->ignore();
  //   }
  // }

 private:
};

class TabManager : public QTabWidget {
  Q_OBJECT
  // 所有打开过界面的, 都会有记录保存, 不管是什么类型的
  enum class TabType : char {
    Serial,
    Udp_client,
    Modbus_client,
    MQTT_client,
    Udp_server,
    Modbus_server,
    MQTT_broker,
  };

 public:
  // 定义序列化接口
  class ISerializable {
   public:
    virtual ~ISerializable() = default;
    virtual QByteArray saveState() const = 0;
    virtual bool restoreState(const QByteArray& state) = 0;
  };

  using WidgetFactory = std::function<QWidget*()>;

  explicit TabManager(QWidget* defalue_widget, QWidget* parent = nullptr);
  ~TabManager() override;

  void addNewTab(const QString& title = "");
  void addNewTab(QWidget* defaultWidget);
  void addNewTab(QWidget* defaultWidget, const QString& title);

  // 注册 Widget 工厂函数
  void registerWidget(int widgetId, const WidgetFactory& factory,
                      const QString& title);

  // 在指定 Tab 中切换 Widget
  void switchToWidget(int tabIndex, int widgetId);

  void setupTabBar();

  // 序列化支持
  bool saveState(const QString& filePath) const;
  bool restoreState(const QString& filePath);

  QString getCurrentWidgetUUid();

  void setTabIcon(int index, const QString& iconPath);
  void setTabIcon(int index, const QIcon& icon);

  // 点击了 save 才保存
  void setTabTitle(const QString& title);

 signals:
  void newTabRequested();  // 新建标签信号

  // private slots:
 public slots:
  // 处理 widget1 中按钮点击事件
  void handleButtonClicked(int tabIndex, int widgetId);
  void handleAddNewTab();                   // 处理关闭标签
  void handleTabCloseRequested(int index);  // 处理关闭标签
  void removeUuidWidget(const QString& index);

  void switchToCurrentIndex(const QString& index);

 private:
  QToolButton* addButton;
  void setupCustomTabButton(int index);
  void updateTabStyle(int index);

  void setupCornerButton() {
    auto* addButton = new QToolButton(this);
    addButton->setIcon(QIcon(":/sys/plus-circle.svg"));
    addButton->setToolTip(tr("New Tab"));

    // connect(addButton, &QToolButton::clicked, this,
    //         [this]() { addTab(new QWidget, "New Tab"); });

    // 设置按钮样式
    addButton->setStyleSheet(R"(
        QToolButton {
            border: none;
            padding: 2px;
            background: transparent;
        }
        QToolButton:hover {
            background: rgba(0, 0, 0, 0.1);
        }
        QToolButton:pressed {
            background: rgba(0, 0, 0, 0.15);
        }
    )");

    setCornerWidget(addButton, Qt::TopRightCorner);

    // connect(addButton, &QToolButton::clicked,
    //         this, &CustomTabWidget::handleAddTabRequest);
  }

  int getTabIndexFromButton(QWidget* button) const {
    // 通过遍历找到按钮对应的标签页索引
    QTabBar* bar = tabBar();
    for (int i = 0; i < bar->count(); ++i) {
      if (bar->tabButton(i, QTabBar::RightSide) == button) {
        return i;
      }
    }
    return -1;
  }
  // 初始化默认的 widget1
  QWidget* createDefaultWidget(int tabIndex);
  QString findWidget(QWidget* widget);

  void handleTabClose(int index);

  // 恢复最近关闭的标签页
  void restoreLastClosedTab();
  // 获取可恢复的标签页列表
  QStringList getClosedTabsList() const;
  // 序列化
  QJsonObject serializeTab(int index) const;
  // 反序列化
  void deserializeTab(const QJsonObject& obj);

  // struct TabInfo {
  //   QString title;
  //   QWidget* widget;
  //   QIcon icon;
  // };
  struct TabData {
    QString title;
    QWidget* widget;
    QIcon icon;
    QByteArray state;  // 用于保存widget状态
  };

  // uuid, 存储已关闭的标签页信息
  QList<QPair<QString, TabData>> closedTabs_;
  // 最大保存历史数量
  const int maxClosedTabs_ = 10;
  QPoint dragStartPosition_;

  QHash<int, WidgetFactory> widgetFactories;  // Widget 工厂函数映射
  QHash<int, QString> widgetTitles;           // Widget 标题映射
  QMap<QString, QWidget*> widgetInstances;    // Widget 实例
};

/*
// 标签页实现示例
class CustomTabPage : public QWidget, public TabManager::ISerializable {
  Q_OBJECT
 public:
  explicit CustomTabPage(QWidget* parent = nullptr);

  // 实现序列化接口
  QByteArray saveState() const override {
    QByteArray state;
    QDataStream stream(&state, QIODevice::WriteOnly);
    // 保存需要的数据
    stream << someData_;
    return state;
  }

  bool restoreState(const QByteArray& state) override {
    QDataStream stream(state);
    // 恢复数据
    stream >> someData_;
    return true;
  }

 private:
  QString someData_;
};
*/

}  // namespace Ui

#endif  // WINDOW_SWITCHER_H
