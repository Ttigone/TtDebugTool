#ifndef UI_CONTROL_TTCHATMESSAGEDELEGATE_H
#define UI_CONTROL_TTCHATMESSAGEDELEGATE_H

#include <QCache>
#include <QListView>
#include <QMutex>
#include <QPointer>
#include <QStyledItemDelegate>
#include <QTextLayout>

#include "ui/Def.h"
namespace Ui {

class TtChatMessage;

#if 0

class TtChatMessageDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  enum RenderQuality {
    HighQuality,    // 抗锯齿+精确布局
    MediumQuality,  // 基本抗锯齿
    LowQuality      // 快速滚动模式
  };
  Q_ENUM(RenderQuality)

  explicit TtChatMessageDelegate(QObject* parent = nullptr);
  ~TtChatMessageDelegate();

  // 核心绘制方法
  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;

  // 交互支持
  bool hitTest(const QPoint& pos, const QModelIndex& index,
               QRect* textRect = nullptr) const;
  int posToCursor(const QPointF& pos, const QModelIndex& index) const;
  QRect cursorRect(int cursorPos, const QModelIndex& index) const;

  // 缓存管理
  void invalidateCache();
  void setRenderQuality(RenderQuality quality);

  // 新增视图设置方法
  void setView(QListView* view);

  // 新增调整状态标记
  void setResizing(bool resizing);

  // ChatMessageDelegate.cpp
  void updateLayoutForIndex(const QModelIndex& index);

  void beginUpdateSelections();
  void endUpdateSelections();

 public slots:
  void handleHoverIndexChanged(const QModelIndex& index);

 protected:
  void initStyleOption(QStyleOptionViewItem* option,
                       const QModelIndex& index) const override;

 private:
  struct LayoutCache {
    QTextLayout textLayout;    // 文本尺寸
    QRectF bubbleRect;         // 气泡尺寸
    QVector<QTextLine> lines;  // underline
    QRectF contentRect;        // 内容尺寸
    QString test;
  };

  void drawBubble(QPainter* painter, const QStyleOptionViewItem& option,
                  const QModelIndex& index) const;
  void drawContent(QPainter* painter, const QStyleOptionViewItem& option,
                   const QModelIndex& index) const;
  void drawSelection(QPainter* painter, const QStyleOptionViewItem& option,
                     const QModelIndex& index) const;
  void drawStatusIndicator(QPainter* painter,
                           const QStyleOptionViewItem& option,
                           const TtChatMessage* msg) const;

  void drawSingleCheckmark(QPainter* painter, const QRect& rect) const;
  void drawDoubleCheckmark(QPainter* painter, const QRect& rect) const;
  void drawErrorIndicator(QPainter* painter, const QRect& rect) const;
  void drawSendingIndicator(QPainter* painter, const QRect& rect) const;

  LayoutCache* getLayoutCache(const QModelIndex& index,
                              const QStyleOptionViewItem& option) const;
  QVector<QTextLayout::FormatRange> createSelectionFormats(
      const QModelIndex& index) const;
  QColor blendColors(QColor c1, QColor c2, float ratio) const;

  mutable QCache<QString, LayoutCache> m_layoutCache;
  QPointer<QAbstractItemView> m_view;
  RenderQuality m_renderQuality = HighQuality;
  QModelIndex m_hoverIndex;
  int m_maxCacheSize = 100;  // 缓存最近100条消息布局
  // QListView* m_view = nullptr; // 新增视图指针成员
  bool m_isResizing = false;
  QMutex m_cacheMutex;  // 缓存线程安全
  void drawSimpleText(QPainter* painter, const QStyleOptionViewItem& option,
                      const QModelIndex& index) const;
  // 新增边距常量
  static constexpr int horizontalMargin = 8;
  static constexpr int verticalMargin = 6;
};

#else

class Tt_EXPORT TtChatMessageDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  enum RenderQuality {
    LowQuality,     // 快速滚动模式
    MediumQuality,  // 基本抗锯齿
    HighQuality     // 抗锯齿+精确布局
  };
  Q_ENUM(RenderQuality)

  explicit TtChatMessageDelegate(QObject* parent = nullptr);
  ~TtChatMessageDelegate();

  // 核心绘制方法
  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;

  // 交互支持
  bool hitTest(const QPoint& pos, const QModelIndex& index,
               QRect* textRect = nullptr) const;
  int posToCursor(const QPointF& pos, const QModelIndex& index) const;
  QRect cursorRect(int cursorPos, const QModelIndex& index) const;

  // 缓存管理
  void invalidateCache();
  void setRenderQuality(RenderQuality quality);

  // 新增视图设置方法
  void setView(QListView* view);

  // 新增调整状态标记
  void setResizing(bool resizing);

  // ChatMessageDelegate.cpp
  void updateLayoutForIndex(const QModelIndex& index);

  void beginUpdateSelections();
  void endUpdateSelections();
  void clearCache();

 public slots:
  void handleHoverIndexChanged(const QModelIndex& index);

 protected:
  void initStyleOption(QStyleOptionViewItem* option,
                       const QModelIndex& index) const override;

 private:
  struct LayoutCache {
    QTextLayout textLayout;    // 文本尺寸
    QRectF bubbleRect;         // 气泡尺寸
    QVector<QTextLine> lines;  // 行
    QRectF contentRect;        // 内容尺寸
  };

  void drawBubble(QPainter* painter, const QStyleOptionViewItem& option,
                  const QModelIndex& index, LayoutCache* cache) const;
  void drawSelection(QPainter* painter, const QStyleOptionViewItem& option,
                     const QModelIndex& index, LayoutCache* cache) const;
  void drawStatusIndicator(QPainter* painter,
                           const QStyleOptionViewItem& option,
                           const TtChatMessage* msg) const;

  void drawSingleCheckmark(QPainter* painter, const QRect& rect) const;
  void drawDoubleCheckmark(QPainter* painter, const QRect& rect) const;
  void drawErrorIndicator(QPainter* painter, const QRect& rect) const;
  void drawSendingIndicator(QPainter* painter, const QRect& rect) const;

  LayoutCache* getLayoutCache(const QModelIndex& index,
                              const QStyleOptionViewItem& option) const;
  QVector<QTextLayout::FormatRange> createSelectionFormats(
      const QModelIndex& index) const;
  QColor blendColors(QColor c1, QColor c2, float ratio) const;

  mutable QCache<QString, LayoutCache> m_layoutCache;
  // mutable QHash<QString, LayoutCache *> m_layoutCache;
  QPointer<QAbstractItemView> m_view;
  RenderQuality m_renderQuality = HighQuality;
  QModelIndex m_hoverIndex;
  int m_maxCacheSize = 500;  // 缓存最近100条消息布局
  bool m_isResizing = false;
  QMutex m_cacheMutex;  // 缓存线程安全
  void drawSimpleText(QPainter* painter, const QStyleOptionViewItem& option,
                      const QModelIndex& index) const;
  // 新增边距常量
  static constexpr int horizontalMargin = 6;
  static constexpr int verticalMargin = 4;

  // static int cache_width_;
  // QHash<QString, QSize> size_cache_;
};

#endif


}  // namespace Ui

#endif // UI_CONTROL_TTCHATMESSAGEDELEGATE_H
