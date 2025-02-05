#ifndef UI_WIDGETS_CUSTOMIZATIONTABWIDGET_H
#define UI_WIDGETS_CUSTOMIZATIONTABWIDGET_H

#include <QTabWidget>
#include <QMouseEvent>
#include <QApplication>

namespace Ui {

class CustomizationTabWidget : public QTabWidget {
  Q_OBJECT
 public:
  CustomizationTabWidget(QWidget *parent = nullptr);
  ~CustomizationTabWidget();


  void addTabWidthTitle(const QString &title);

 // protected:
 //  void dragEnterEvent(QDragEnterEvent *event);
 //  void dragLeaveEvent(QDragLeaveEvent *event);
 //  void dragMoveEvent(QDragMoveEvent *event);
 //  bool eventFilter(QObject *watched, QEvent *event);


 // private slots:
 //  void tabBarCustomContextMenuRequested(const QPoint &pos);
 //  void onCloseLeftTabAct();
 //  void onCloseRightTabAct();
 //  void onCloseOtherTabAct();
 //  void onFixedTabAct();
 //  void onUnFixedTabAct();
 //  void onCurrentChanged(int index);

 // private:
 //  bool m_tabClosable;
 //  bool m_isDrag;
 //  int m_dragIndex;
 //  int m_xOffset;
 //  int m_offset;
 //  bool m_tabBarPress;
 //  QSet<QWidget*> m_unDragWidgets;
 //  QPoint m_mousePressPoint;
 //  QMenu* m_menu;
 //  QAction* m_closeLeftTabAct;
 //  QAction* m_closeRightTabAct;
 //  QAction* m_closeOtherTabAct;
 //  QAction* m_fixedTabAct;
 //  QAction* m_unFixedTabAct;
 //  void startDrag(int index);
 //  void createContextMenu();
 //  void setTabCloseBtn(int index,QWidget* widget);
};

class CustomTabWidget : public QTabWidget {
 public:
  CustomTabWidget(QWidget *parent = nullptr) : QTabWidget(parent) {
    setTabsClosable(true);
    setMovable(true);
  }

 protected:
  void mousePressEvent(QMouseEvent *event) override {
    if (event->buttons() & Qt::LeftButton) {
      // Start dragging
      dragStartPosition = event->pos();
    }
    QTabWidget::mousePressEvent(event);
  }

  void mouseMoveEvent(QMouseEvent *event) override {
    if (!(event->buttons() & Qt::LeftButton))
      return;

    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
      return;

    // Dragging tab
    int tabIndex = tabBar()->tabAt(event->pos());
    if (tabIndex >= 0) {
      tabBar()->moveTab(tabIndex, event->pos().x());
      event->accept();
    }
  }

 private:
  QPoint dragStartPosition;
};


} // mamespace Ui


#endif  // CUSTOMIZATIONTABWIDGET_H
