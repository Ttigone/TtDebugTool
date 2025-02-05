#ifndef WINDOW_SWITCHER_H
#define WINDOW_SWITCHER_H

#include <qproxystyle.h>
#include <QPainter>

#include <QMap>
#include <QMouseEvent>
#include <QObject>
#include <QStackedLayout>
#include <QState>
#include <QStateMachine>
#include <QTabWidget>
#include <functional>

namespace Ui {


class ExtTabBarStyle : public QProxyStyle {
 public:
  explicit ExtTabBarStyle(QStyle* style = nullptr)
      : QProxyStyle(style), showPlus(true) {}

  void drawControl(ControlElement element, const QStyleOption* option,
                   QPainter* painter, const QWidget* widget) const override {
    if (element == CE_TabBarTabLabel) {
      // 自定义 Tab 文字颜色
      auto tabOption = qstyleoption_cast<const QStyleOptionTab*>(option);
      if (tabOption->state & State_Selected) {
        painter->setPen(Qt::red);  // 选中 Tab 文字颜色
      } else {
        painter->setPen(Qt::black);  // 未选中 Tab 文字颜色
      }
      QProxyStyle::drawControl(element, option, painter, widget);
    } else {
      QProxyStyle::drawControl(element, option, painter, widget);
    }
  }

  void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget) const override {
    //if (element == PE_IndicatorTabClose) {
    //  // 自定义关闭按钮样式
    //  painter->drawText(option->rect, Qt::AlignCenter, "×");
    //} else if (element == PE_IndicatorTabClose) {
    ////} else if (element == QStyle::PE_IndicatorTab) {
    //  // 自定义 "+" 按钮样式
    //  painter->drawText(option->rect, Qt::AlignCenter, "+");
    //} else {
    //  QProxyStyle::drawPrimitive(element, option, painter, widget);
    //}
    if (element == PE_IndicatorTabClose) {
      // 自定义关闭按钮样式
      painter->drawText(option->rect, Qt::AlignCenter, "×");
    } else {
      QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
  }

  bool showPlus;  // 是否显示 "+" 按钮
};

class ExtTabBar : public QTabBar {
  Q_OBJECT

 public:
  explicit ExtTabBar(QWidget* parent = nullptr) : QTabBar(parent) {
    setStyle(new ExtTabBarStyle(style()));  // 应用自定义样式
    setMovable(true);
    setTabsClosable(true);  // 启用关闭按钮
  }

 signals:
  void plusClicked();  // "+" 按钮点击信号

 protected:
  void paintEvent(QPaintEvent* event) override {
    QTabBar::paintEvent(event);
    if (static_cast<ExtTabBarStyle*>(style())->showPlus) {
      // 手动绘制 "+" 按钮
      QPainter painter(this);
      QRect plusRect = plusButtonRect();
      painter.drawText(plusRect, Qt::AlignCenter, "+");
    }
  }

  void mousePressEvent(QMouseEvent* event) override {
    if (event->button() == Qt::LeftButton) {
      // 处理关闭按钮点击
      for (int i = 0; i < count(); ++i) {
        if (closeButtonRect(i).contains(event->pos())) {
          emit tabCloseRequested(i);
          return;
        }
      }
      // 处理 "+" 按钮点击
      if (plusButtonRect().contains(event->pos())) {
        emit plusClicked();
        return;
      }
    }
    QTabBar::mousePressEvent(event);
  }

  QSize tabSizeHint(int index) const override {
    // 调整 Tab 大小，为关闭按钮留出空间
    QSize size = QTabBar::tabSizeHint(index);
    size.setWidth(size.width() + 30);  // 宽度增加 30px
    return size;
  }

 private:
  QRect closeButtonRect(int index) const {
    // 计算关闭按钮的位置
    QRect rect = tabRect(index);
    return QRect(rect.right() - 20, rect.top() + 5, 16, 16);
  }

  QRect plusButtonRect() const {
    // 计算 "+" 按钮的位置（位于最右侧）
    int width = style()->pixelMetric(QStyle::PM_TabBarScrollButtonWidth);
    return QRect(this->width() - width, 0, width, height());
  }
};


class TabManager : public QTabWidget {
  Q_OBJECT

 public:
  using WidgetFactory = std::function<QWidget*()>;

  explicit TabManager(QWidget* parent = nullptr);
  explicit TabManager(QWidget* defalue_widget, QWidget* parent = nullptr);
  ~TabManager() override;

  // 添加新的 Tab，默认包含 widget1
  void addNewTab(const QString& title = "");

  // 添加新的 Tab，默认包含 widget1
  void addNewTab(QWidget* defaultWidget);

  // 添加新的 Tab，默认包含 widget1
  void addNewTab(QWidget* defaultWidget, const QString& title);

  // 注册 Widget 工厂函数
  void registerWidget(int widgetId, const WidgetFactory& factory,
                      const QString& title);

  // 在指定 Tab 中切换 Widget
  void switchToWidget(int tabIndex, int widgetId);

  // private slots:
 public slots:
  // 处理 widget1 中按钮点击事件
  void handleButtonClicked(int tabIndex);

 private:
  // 初始化默认的 widget1
  QWidget* createDefaultWidget(int tabIndex);

  QHash<int, WidgetFactory> widgetFactories;             // Widget 工厂函数映射
  QHash<int, QString> widgetTitles;                      // Widget 标题映射
  QHash<int, QWidget*> widgetInstances;                  // Widget 实例
};

} // namespace Ui


#endif  // WINDOW_SWITCHER_H
