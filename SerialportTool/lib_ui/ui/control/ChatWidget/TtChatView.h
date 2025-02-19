#ifndef UI_CONTROL_CHATWIDGET_TTCHATVIEW_H
#define UI_CONTROL_CHATWIDGET_TTCHATVIEW_H

#include <QListView>
#include <QPointer>
#include <QTimer>

#include "ui/control/ChatWidget/TtChatMessageDelegate.h"
#include "ui/control/ChatWidget/TtChatMessageModel.h"

//class TtChatMessageModel;
//class TtChatMessageDelegate;

namespace Ui {

#if 0
class TtChatView : public QListView
{
    Q_OBJECT

    Q_PROPERTY(bool loadingOlderMessages READ isLoadingOlderMessages NOTIFY loadingStateChanged)
    Q_PROPERTY(int scrollMargin READ scrollMargin WRITE setScrollMargin NOTIFY scrollMarginChanged)

public:
    explicit TtChatView(QWidget *parent = nullptr);
    ~TtChatView();

    void setModel(TtChatMessageModel *model);
    TtChatMessageModel *chatModel() const;

    bool isLoadingOlderMessages() const;
    int scrollMargin() const;

    Q_INVOKABLE void scrollToMessage(const QString &messageId);
    Q_INVOKABLE void copySelectedText();

    // 新增获取可见索引的方法
    QModelIndexList visibleIndexes() const;

public slots:
    void setScrollMargin(int margin);
    void triggerResize();
    void onDataChanged(const QModelIndex &topLeft,
                       const QModelIndex &bottomRight,
                       const QVector<int> &roles = QVector<int>());

signals:
    void needLoadOlderMessages();
    void needLoadNewerMessages();
    void messageSelectionChanged();
    void scrollMarginChanged();
    void loadingStateChanged();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void scrollContentsBy(int dx, int dy) override;

private slots:
    void handleModelReset();
    void updateVisibleItems();
    void checkFetchMore();

private:
    // 选中状态
    struct SelectionState
    {
        QModelIndex anchorIndex;   // 锚点消息索引
        int anchorCursorPos;       // 锚点光标位置
        QModelIndex currentIndex;  // 当前消息索引
        int currentCursorPos = -1; // 当前光标位置
        bool active = false;
        // bool isSelecting = false;
        // QPointF anchorPos;      //
    };

    void initializeScrollbars();
    void updateSelections();
    QRect messageContentRect(const QModelIndex &index) const;
    void adjustScrollForSelection();
    void maintainScrollPosition();
    void fetchMoreIfNeeded();
    void handleDelayedResize();
    // QPointer<ChatMessageModel> m_model;
    TtChatMessageModel *m_model;
    TtChatMessageDelegate *m_delegate;
    SelectionState m_selection;
    QTimer *m_scrollTimer;
    QTimer *m_fetchTimer;
    QTimer m_resizeTimer;    // 防抖定时器（成员变量）
    int m_scrollMargin = 50; // 滚动边距触发加载
    bool m_loadingOlder = false;
    bool m_keepScrollPosition = false;
    double m_scrollRatio = 0.0;

    QMenu *m_contextMenu;
    QAction *m_copyAction;
};

#else

class TtChatView : public QListView {
  Q_OBJECT

  Q_PROPERTY(bool loadingOlderMessages READ isLoadingOlderMessages NOTIFY
                 loadingStateChanged)
  Q_PROPERTY(int scrollMargin READ scrollMargin WRITE setScrollMargin NOTIFY
                 scrollMarginChanged)

 public:
  explicit TtChatView(QWidget* parent = nullptr);
  ~TtChatView();

  void setModel(TtChatMessageModel* model);
  TtChatMessageModel* chatModel() const;

  bool isLoadingOlderMessages() const;
  int scrollMargin() const;

  Q_INVOKABLE void scrollToMessage(const QString& messageId);
  Q_INVOKABLE void copySelectedText();

  // 新增获取可见索引的方法
  QModelIndexList visibleIndexes();

 public slots:
  void setScrollMargin(int margin);
  void triggerResize();
  void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                     const QVector<int>& roles = QVector<int>());

 signals:
  void needLoadOlderMessages();
  void needLoadNewerMessages();
  void messageSelectionChanged();
  void scrollMarginChanged();
  void loadingStateChanged();

 protected:
  void mousePressEvent(QMouseEvent* e) override;
  void mouseMoveEvent(QMouseEvent* e) override;
  void mouseReleaseEvent(QMouseEvent* e) override;
  void wheelEvent(QWheelEvent* e) override;
  void paintEvent(QPaintEvent* e) override;
  void resizeEvent(QResizeEvent* e) override;
  void scrollContentsBy(int dx, int dy) override;

 private slots:
  void handleModelReset();
  void updateVisibleItems();
  void checkFetchMore();

 private:
  // 选中状态
  struct SelectionState {
    QModelIndex anchorIndex;    // 锚点消息索引
    int anchorCursorPos;        // 锚点光标位置
    QModelIndex currentIndex;   // 当前消息索引
    int currentCursorPos = -1;  // 当前光标位置
    bool active = false;
    // bool isSelecting = false;
    // QPointF anchorPos;      //
  };

  void initializeScrollbars();
  void updateSelections();
  QRect messageContentRect(const QModelIndex& index) const;
  void adjustScrollForSelection();
  void maintainScrollPosition();
  void fetchMoreIfNeeded();
  void handleViewportResize();
  // QPointer<ChatMessageModel> m_model;
  TtChatMessageModel* m_model;
  // QAbstractItemModel *m_model;
  TtChatMessageDelegate* m_delegate;
  SelectionState m_selection;
  QTimer* m_scrollTimer;
  QTimer* m_fetchTimer;
  QTimer m_resizeTimer;     // 防抖定时器（成员变量）
  int m_scrollMargin = 50;  // 滚动边距触发加载
  bool m_loadingOlder = false;
  bool m_keepScrollPosition = false;
  double m_scrollRatio = 0.0;

  QMenu* m_contextMenu;
  QAction* m_copyAction;

  QAtomicInt m_resizeFlag{0};  // 原子操作标志位
  //QFuture<void> m_layoutFuture;  // 用于异步布局计算
  QMap<QModelIndex, int> tmp_index_list_;
  QMap<QModelIndex, int> tmp_index_wheel_list_;

  bool m_isMousePressed = false;  // 记录鼠标是否按下
  QModelIndex m_anchorIndex;      // 记录鼠标按下时的初始选中项
  int m_scrollDirection = 0;  // 记录滚动方向（1: 向下，-1: 向上）
  QModelIndex old_index_;
};

#endif

}  // namespace Ui
#endif  // UI_CONTROL_CHATWIDGET_TTCHATVIEW_H
