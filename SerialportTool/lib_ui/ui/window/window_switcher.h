#ifndef WINDOW_SWITCHER_H
#define WINDOW_SWITCHER_H

#include <QMap>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QProxyStyle>
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
      // 绘制默认标签
      QProxyStyle::drawControl(element, option, painter, widget);

      // 添加拖动状态文字绘制
      if (const QStyleOptionTab* tab =
              qstyleoption_cast<const QStyleOptionTab*>(option)) {
        // 获取实际显示区域（考虑拖动偏移）
        QRect textRect = tab->rect.adjusted(
            4, 0, -24, 0);  // 左侧4px，右侧24px为关闭按钮留空
        if (tab->state & State_Sibling) {
        //if (tab->state & State_Sunken) {  // 拖动
          qDebug() << "silbling";
          painter->setOpacity(0.7);  // 半透明效果
        }

        // 绘制文字（自动省略号）
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter,
                          //fontMetrics().elidedText(tab->text, Qt::ElideRight,
                          widget->fontMetrics().elidedText(tab->text, Qt::ElideRight,
                                                   textRect.width()));
        painter->setOpacity(1.0);  // 恢复不透明
      }

      // 自定义 Tab 文字颜色
      auto tabOption = qstyleoption_cast<const QStyleOptionTab*>(option);
      if (tabOption) {
        QRect closeRect = closeButtonRect(tabOption->rect);
        drawCloseButton(painter, closeRect, tabOption->state & State_MouseOver);
      }
      if (tabOption->state & State_Selected) {
        painter->setPen(Qt::red);  // 选中 Tab 文字颜色
      } else {
        painter->setPen(Qt::black);  // 未选中 Tab 文字颜色
      }
    } else {
      QProxyStyle::drawControl(element, option, painter, widget);
    }
  }

  QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                         const QSize& size,
                         const QWidget* widget) const override {
    QSize newSize = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type == CT_TabBarTab) {
      newSize.rwidth() += 20;  // 为关闭按钮预留空间
    }
    return newSize;
  }

  void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget) const override {
    if (element == PE_IndicatorTabClose) {
      // 自定义关闭按钮样式
      painter->drawText(option->rect, Qt::AlignCenter, "×");
    } else {
      QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
  }

  QRect closeButtonRect(const QRect& tabRect) const {
    const int size = 16;
    return QRect(tabRect.right() - size - 4, tabRect.center().y() - size / 2,
                 size, size);
  }

 private:
  void drawCloseButton(QPainter* painter, const QRect& rect, bool hover) const {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    // 根据悬停状态设置颜色
    QColor color = hover ? QColor(255, 100, 100) : QColor(160, 160, 160);
    painter->setPen(QPen(color, 1.5));

    // 绘制 "×" 符号
    painter->drawLine(rect.topLeft() + QPoint(4, 4),
                      rect.bottomRight() - QPoint(4, 4));
    painter->drawLine(rect.topRight() - QPoint(4, -4),
                      rect.bottomLeft() + QPoint(4, -4));

    painter->restore();
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
    setMouseTracking(true);

    // 新建按钮（仅一个实例）
    newTabButton = new QPushButton("+", this);
    newTabButton->setFixedSize(24, 24);
    newTabButton->setStyleSheet("border: none; color: red;");
    connect(newTabButton, &QPushButton::clicked, this,
            &ExtTabBar::newTabRequested);
    // 确保按钮初始位置正确
    updateNewButtonPosition();
  }

 signals:
  //void plusClicked();  // "+" 按钮点击信号
  void newTabRequested();

 protected:
  void paintEvent(QPaintEvent* event) override {
    QTabBar::paintEvent(event);
    //updateButtonPositions();
  }

  //QSize tabSizeHint(int index) const override {
  //  //// 调整 Tab 大小，为关闭按钮留出空间
  //  // 增加宽度以容纳关闭按钮
  //  QSize size = QTabBar::tabSizeHint(index);
  //  return QSize(size.width() + 24, size.height());  // 24为关闭按钮宽度
  //}

  void resizeEvent(QResizeEvent* event) override {
    QTabBar::resizeEvent(event);
    //updateButtonPositions();
  }

  void mousePressEvent(QMouseEvent* event) override {
    // 检测是否点击关闭按钮区域
    for (int i = 0; i < count(); ++i) {
      QRect tabRect = this->tabRect(i);
      QRect closeRect =
          static_cast<ExtTabBarStyle*>(style())->closeButtonRect(tabRect);
      if (closeRect.contains(event->pos())) {
        emit tabCloseRequested(i);
        return;
      }
    }
    QTabBar::mousePressEvent(event);
  }

 private slots:
  void handleCloseButtonClicked() {
    // 通过信号通知关闭标签
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    int index = closeButtons.key(btn, -1);
    if (index != -1) {
      emit tabCloseRequested(index);
    }
  }
  void handleNewTabButtonClicked() {
    emit newTabRequested();  // 通知外部创建新标签
  }

 private:
  void updateNewButtonPosition() {
    //// 计算 "+" 按钮的位置
    //const int scrollWidth = (usesScrollButtons() ? 30 : 0);  // 滚动按钮宽度补偿
    //newTabButton->move(width() - newTabButton->width() - scrollWidth - 2,
    //                   (height() - newTabButton->height()) / 2);
    // 获取实际可用宽度（排除滚动按钮区域）
    int availableWidth = width();
    if (usesScrollButtons()) {
      availableWidth -= 30;  // 滚动按钮区域宽度
    }

    // 设置按钮位置（右侧留2像素边距）
    newTabButton->move(availableWidth - newTabButton->width() - 2,
                       (height() - newTabButton->height()) / 2);
    newTabButton->raise();  // 确保按钮在最上层
  }
  void updateButtonPositions() {
    qDebug() << "ex";
    // 更新关闭按钮位置
    for (int i = 0; i < count(); ++i) {
      QWidget* btn = closeButtons.value(i);
      if (!btn) {
        btn = new QPushButton("×", this);  // 关闭符号
        btn->setFixedSize(20, 20);
        btn->setStyleSheet("QPushButton { border: none; color: gray; }");
        //connect(btn, &QPushButton::clicked,
        //        [this, i]() {
        //emit tabCloseRequested(i);
        //});
        closeButtons[i] = btn;
      }
      QRect tabRect = this->tabRect(i);
      btn->move(tabRect.right() - 22, tabRect.center().y() - 10);  // 右侧偏移
      btn->show();
    }

    // 更新 "新建" 按钮位置到最右侧
    newTabButton->move(width() - newTabButton->width() - 2,
                       (height() - newTabButton->height()) / 2);
  }

  QPushButton* newTabButton;          // "新建"按钮
  QHash<int, QWidget*> closeButtons;  // 每个标签的关闭按钮
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

  void setupTabBar();

 signals:
  void newTabRequested();  // 新建标签信号

  // private slots:
 public slots:
  // 处理 widget1 中按钮点击事件
  void handleButtonClicked(int tabIndex, int widgetId);
  void handleAddNewTab();                   // 处理关闭标签
  void handleTabCloseRequested(int index);  // 处理关闭标签

 private:
  // 初始化默认的 widget1
  QWidget* createDefaultWidget(int tabIndex);

  QHash<int, WidgetFactory> widgetFactories;  // Widget 工厂函数映射
  QHash<int, QString> widgetTitles;           // Widget 标题映射
  QHash<int, QWidget*> widgetInstances;       // Widget 实例
};

}  // namespace Ui

#endif  // WINDOW_SWITCHER_H
