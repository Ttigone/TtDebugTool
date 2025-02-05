#include "ui/control/ChatWidget/TtChatMessageDelegate.h"
#include "ui/control/ChatWidget/TtChatMessage.h"
#include "ui/control/ChatWidget/TtChatView.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QBrush>
#include <QPainter>
#include <QPainterPath>

namespace Ui {

#if 0
TtChatMessageDelegate::TtChatMessageDelegate(QObject* parent)
    : QStyledItemDelegate(parent), m_layoutCache(100) {  // 缓存100个消息布局
}

TtChatMessageDelegate::~TtChatMessageDelegate() {
  m_layoutCache.clear();
}

// 核心绘制方法实现
void TtChatMessageDelegate::paint(QPainter* painter,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const {
  if (!index.isValid()) {
    return;
  }
  // 初始化样式选项
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  // static int i = 0;
  // qDebug() << QString("Delegate: %1").arg(i++);

  painter->setRenderHint(QPainter::Antialiasing,
                         m_renderQuality >= MediumQuality);
  painter->setRenderHint(QPainter::TextAntialiasing,
                         m_renderQuality >= MediumQuality);

  LayoutCache* cache = getLayoutCache(index, option);

  // 分层绘制
  drawBubble(painter, opt, index);     // 1. 绘制气泡背景
  drawContent(painter, opt, index);    // 2. 绘制文本内容
  drawSelection(painter, opt, index);  // 3. 绘制选择高亮
  // qDebug() << "Delegate: 2";

  TtChatMessage* msg = qobject_cast<TtChatMessage*>(
      index.data(TtChatMessageModel::MessageObjectRole).value<QObject*>());
  // 绘制是否被阅读后的标志
  // drawStatusIndicator(painter, opt, msg);
}

QSize TtChatMessageDelegate::sizeHint(const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);  // 必须初始化以获取正确尺寸

  LayoutCache* cache =
      getLayoutCache(index, option);  // 需要添加 option 参数    if (!cache)
  if (!cache) {
    return QSize(200, 40);  // 默认尺寸
  }

  auto size =
      QSize(opt.rect.width(), cache->bubbleRect.height() + verticalMargin);
  return size;
}

// 交互支持方法
bool TtChatMessageDelegate::hitTest(const QPoint& pos, const QModelIndex& index,
                                    QRect* textRect) const {
  QStyleOptionViewItem opt;
  opt.rect = m_view->visualRect(index);  // 关键
  initStyleOption(&opt, index);          // 初始化样式选项
  LayoutCache* cache =
      getLayoutCache(index, opt);  // 需要添加 option 参数    if (!cache)
  // index.data()
  if (!cache) {
    return false;
  }

  // 转换为消息内容坐标系
  // qDebug() << "pos : " << pos; // 每个项内部的 局部 pos  左上角 0, 0
  // qDebug() << "bubble: " << cache->bubbleRect.x() << cache->bubbleRect.y() - opt.rect.top();
  // qDebug() << cache->bubbleRect;
  // qDebug() << cache->test;

  // QPoint contentPos = pos - cache->bubbleRect.topLeft().toPoint();
  // QPoint contentPos = pos - cache->bubbleRect.topLeft().toPoint() - opt.rect.top();
  // qDebug() << "pos.y" << pos.y();
  // 以文本布局判断是否在文本区域上(试验气泡框)
  QPoint contentPos = QPoint(
      (pos.x() - cache->bubbleRect.topLeft().toPoint().x()),
      (pos.y() - cache->bubbleRect.topLeft().toPoint().y() + opt.rect.top()));
  // 文本内容布局
  QRect layoutRect = cache->contentRect.toRect();
  // qDebug() << "layoutRect " << layoutRect;
  // qDebug() << "contentPos " << contentPos;

  if (textRect) {
    *textRect = layoutRect;
  }
  if (layoutRect.contains(contentPos)) {
    // qDebug() << "yes";
    return true;
  } else {
    return false;
  }
}

int TtChatMessageDelegate::posToCursor(const QPointF& pos,
                                       const QModelIndex& index) const {
  // 获取 start
  QStyleOptionViewItem opt;
  initStyleOption(&opt, index);          // 初始化样式选项
  opt.rect = m_view->visualRect(index);  // 从视图获取实际矩形
  LayoutCache* cache = getLayoutCache(index, opt);
  if (!cache) {
    return -1;
  }

  qDebug() << "pos 坐标: " << pos;  // pos 的高度是以每一个项为依据的
  qDebug() << "获取的项的左上角 " << opt.rect.topLeft();
  // const QPointF localPos = pos - cache->bubbleRect.topLeft() - cache->contentRect.topLeft();
  qreal clickPosX = (pos - cache->bubbleRect.topLeft()).x() - horizontalMargin;
  qreal clickPosY = (pos - cache->bubbleRect.topLeft()).y() - verticalMargin;
  clickPosY +=
      opt.rect.toRectF().top();  // 缺少之前项的高度, 气泡高度依赖于之前项的高度
  // 没有问题了
  // const QPointF localPos = (pos - cache->bubbleRect.topLeft() - cache->contentRect.topLeft());
  const QPointF localPos(clickPosX, clickPosY);
  qDebug() << "项: 左上角" << cache->contentRect.topLeft();
  // 气泡的高度是累加项
  qDebug() << "气泡: 左上角" << cache->bubbleRect.topLeft();

  qDebug() << "localPos" << localPos;

  // qDebug() << cache->textLayout.text();
  foreach (const QTextLine& line, cache->lines) {
    // 行数 决定进来几次
    // qDebug() << "line.rect()" << line.rect();
    if (line.rect().contains(localPos)) {
      // qDebug() << "line-Rect contain";
      // int cursorIndex = line.xToCursor(localPos.x(), QTextLine::CursorBetweenCharacters);
      int cursorIndex = line.xToCursor(localPos.x());
      // qDebug() << "cursorIndex First: " << cursorIndex;
      return cursorIndex;
    }
  }
  // 除了第一项, 都没有进来，为什么 ???
  return -1;
}

QRect TtChatMessageDelegate::cursorRect(int cursorPos,
                                        const QModelIndex& index) const {
  QStyleOptionViewItem option;
  initStyleOption(&option, index);          // 初始化样式选项
  option.rect = m_view->visualRect(index);  // 从视图获取实际矩形
  LayoutCache* cache = getLayoutCache(index, option);
  if (!cache || cursorPos < 0)
    return QRect();

  foreach (const QTextLine& line, cache->lines) {
    const int start = line.textStart();
    const int length = line.textLength();
    if (cursorPos >= start && cursorPos <= start + length) {
      qreal x = line.cursorToX(cursorPos);
      // 修改点：添加显式类型转换
      return QRect(x, line.y(), 1, line.height())
          .translated(
              (cache->bubbleRect.topLeft() + cache->contentRect.topLeft())
                  .toPoint()  // QPointF -> QPoint
          );
    }
  }
  return QRect();
}

// 私有方法实现
void TtChatMessageDelegate::drawBubble(QPainter* painter,
                                       const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const {
  TtChatMessage* msg = qobject_cast<TtChatMessage*>(
      index.data(TtChatMessageModel::MessageObjectRole).value<QObject*>());
  LayoutCache* cache = getLayoutCache(index, option);
  if (!cache) {
    return;
  }

  // 使用动态计算的 bubbleRect
  QRectF bubbleRect = cache->bubbleRect;

  // 绘制圆角矩形气泡
  QPainterPath path;
  path.addRoundedRect(bubbleRect, 4, 4);

  path.closeSubpath();

  // 填充气泡
  painter->save();
  painter->setPen(Qt::NoPen);
  painter->setBrush(msg->bubbleColor());
  painter->drawPath(path);
  painter->restore();
}

void TtChatMessageDelegate::drawContent(QPainter* painter,
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const {

  LayoutCache* cache = getLayoutCache(index, option);
  if (!cache) {
    return;
  }

  painter->save();
  // 修正坐标偏移：气泡位置 + 文本边距
  QPointF textPos =
      cache->bubbleRect.topLeft() + QPointF(horizontalMargin, verticalMargin);
  painter->translate(textPos);

  // 绘制文本，不应用额外格式, 空的 format, 文本的默认格式
  QVector<QTextLayout::FormatRange> emptyFormats;
  cache->textLayout.draw(painter, QPointF(0, 0), emptyFormats);

  // cache->textLayout.draw(painter, QPointF(0, 0));
  painter->restore();
}

void TtChatMessageDelegate::drawSelection(QPainter* painter,
                                          const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const {
  // // 起始坐标
  // const int selStart = index.data(ChatMessageModel::SelectionStartRole).toInt();
  // // 结束坐标
  // const int selEnd = index.data(ChatMessageModel::SelectionEndRole).toInt();
  // qDebug() << QString("draw [start] %1 [end] %2").arg(selStart).arg(selEnd);
  // if (selStart == selEnd) {
  //     // 相等, 不需要绘制
  //     return;
  // }

  // LayoutCache *cache = getLayoutCache(index, option); // // 获取当前的缓存记录
  // if (!cache) {
  //     return;
  // }

  // // 已经设置了选中字符的样式
  // QVector<QTextLayout::FormatRange> formats = createSelectionFormats(index);
  // // 正确
  // // 这里设置 format, 无效
  // // cache->textLayout.setFormats(formats);

  // // foreach  (const auto &formatRange, formats) {
  // //     // cache->textLayout.set
  // // }
  // // qDebug() << "--painter--";

  // // 临时绘制选择背景
  // // 应当会被绘制选中字符
  // // bug
  // // 会不会正常刷新的文字挡住了底色变化的文字 ??? 但是隐藏正常的文字, 没有出现
  // painter->save();
  // // 气泡使用的是 bubbleRect

  // // 修正坐标偏移：气泡位置 + 文本边距
  // QPointF textPos = cache->bubbleRect.topLeft() + QPointF(horizontalMargin, verticalMargin);
  // painter->translate(textPos);

  // // painter->translate(cache->bubbleRect.topLeft() + cache->contentRect.topLeft());
  // // 0.0 是气泡框 ??? 还是项 ???
  // // 动态应用此次绘制
  // cache->textLayout.draw(painter, QPoint(0, 0), formats);
  // // qDebug() << "TEST: " << cache->textLayout.text();

  // // cache->textLayout.draw(painter, option.rect.topLeft(), formats);
  // // cache->textLayout.draw(painter, QPointF(0, 0));
  // painter->restore();
  // // qDebug() << "test";

  LayoutCache* cache = getLayoutCache(index, option);  // // 获取当前的缓存记录
  if (!cache) {
    return;
  }

  QVector<QTextLayout::FormatRange> formats = createSelectionFormats(index);

  painter->save();
  QPointF textPos =
      cache->bubbleRect.topLeft() + QPointF(horizontalMargin, verticalMargin);
  painter->translate(textPos);
  // qDebug() << "TEST: " << cache->textLayout.text();

  // 动态应用选择格式
  cache->textLayout.draw(painter, QPointF(0, 0), formats);
  painter->restore();

  // 强制重绘相邻区域（修复视觉残留）
  if (m_view) {
    QRect updateRect =
        cache->bubbleRect.toRect().marginsAdded(QMargins(2, 2, 2, 2));
    m_view->viewport()->update(updateRect);
  }
}

void TtChatMessageDelegate::drawStatusIndicator(
    QPainter* painter, const QStyleOptionViewItem& option,
    const TtChatMessage* msg) const {
  // if (!msg || msg->status() == TtChatMessage::StatusUnknown)
  //     return;

  const int indicatorSize = 14;
  const int margin = 6;
  QRect statusRect;

  // 根据消息方向定位
  if (msg->isOutgoing()) {
    statusRect = QRect(option.rect.right() - indicatorSize - margin,
                       option.rect.bottom() - indicatorSize - margin,
                       indicatorSize, indicatorSize);
  } else {
    statusRect = QRect(option.rect.left() + margin,
                       option.rect.bottom() - indicatorSize - margin,
                       indicatorSize, indicatorSize);
  }

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  // 根据状态选择绘制方式
  switch (msg->status()) {
    case TtChatMessage::Sending:
      drawSendingIndicator(painter, statusRect);
      break;
    case TtChatMessage::Sent:
      drawSingleCheckmark(painter, statusRect);
      break;
    case TtChatMessage::Delivered:
    case TtChatMessage::Read:
      drawDoubleCheckmark(painter, statusRect);
      break;
    case TtChatMessage::Failed:
      drawErrorIndicator(painter, statusRect);
      break;
    default:
      break;
  }

  painter->restore();
}

void TtChatMessageDelegate::drawSingleCheckmark(QPainter* painter,
                                                const QRect& rect) const {
  QPen pen(QColor("#8F8F8F"), 1.8);
  painter->setPen(pen);

  QPainterPath path;
  path.moveTo(rect.left() + 3, rect.center().y());
  path.lineTo(rect.center().x() - 1, rect.bottom() - 3);
  path.lineTo(rect.right() - 3, rect.top() + 4);

  painter->drawPath(path);
}

void TtChatMessageDelegate::drawDoubleCheckmark(QPainter* painter,
                                                const QRect& rect) const {
  QPen pen(QColor("#34B7F1"), 1.8);
  painter->setPen(pen);

  // 第一个勾
  QPainterPath path1;
  path1.moveTo(rect.left() + 2, rect.center().y() - 2);
  path1.lineTo(rect.center().x() - 4, rect.bottom() - 5);
  path1.lineTo(rect.right() - 2, rect.top() + 2);

  // 第二个勾
  QPainterPath path2;
  path2.moveTo(rect.left() + 6, rect.center().y() - 2);
  path2.lineTo(rect.center().x(), rect.bottom() - 5);
  path2.lineTo(rect.right() - 2, rect.top() + 6);

  painter->drawPath(path1);
  painter->drawPath(path2);
}

void TtChatMessageDelegate::drawErrorIndicator(QPainter* painter,
                                               const QRect& rect) const {
  painter->setPen(Qt::NoPen);
  painter->setBrush(QColor("#FF4444"));

  // 绘制圆形背景
  painter->drawEllipse(rect.adjusted(1, 1, -1, -1));

  // 绘制感叹号
  painter->setPen(QPen(Qt::white, 1.5));
  painter->drawLine(rect.center().x(), rect.top() + 4, rect.center().x(),
                    rect.bottom() - 6);
  painter->drawEllipse(QRect(rect.center().x() - 1, rect.bottom() - 4, 2, 2));
}

void TtChatMessageDelegate::drawSendingIndicator(QPainter* painter,
                                                 const QRect& rect) const {
  static int rotation = 0;
  rotation = (rotation + 5) % 360;

  QPen pen(QColor("#AAAAAA"), 1.5);
  painter->setPen(pen);

  QPainterPath path;
  path.addEllipse(rect.adjusted(2, 2, -2, -2));

  painter->save();
  painter->translate(rect.center());
  painter->rotate(rotation);
  painter->translate(-rect.center());

  // 绘制进度圆弧
  painter->drawArc(rect.adjusted(2, 2, -2, -2),
                   0 * 16,   // 起始角度
                   270 * 16  // 跨度角度
  );
  painter->restore();
}

// 缓存管理
TtChatMessageDelegate::LayoutCache* TtChatMessageDelegate::getLayoutCache(
    const QModelIndex& index, const QStyleOptionViewItem& option) const {
  // QMutexLocker locker(&m_cacheMutex);

  const QString key = index.data(TtChatMessageModel::MessageIdRole).toString();
  // qDebug() << "key: " << key;
  // 调整期间允许直接返回旧缓存
  // if (m_isResizing && m_layoutCache.contains(key)) {
  //     qDebug() << "old";
  //     return m_layoutCache.object(key);
  // }

  // if (m_layoutCache.contains(key)) {
  //     return m_layoutCache.object(key);
  // }

  //QObject *obj = index.data(TtChatMessageModel::MessageObjectRole).value<QObject *>();
  //TtChatMessage *msg = qobject_cast<TtChatMessage *>(obj);

  TtChatMessage* msg = qobject_cast<TtChatMessage*>(
      index.data(TtChatMessageModel::MessageObjectRole).value<QObject*>());
  if (!msg) {
    // qDebug() << "nu - msg";
    return nullptr;
  }

  // 创建新的布局缓存
  LayoutCache* cache = new LayoutCache;

  // 设置文本布局参数
  QTextOption textOption;
  // textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere); // 按字符换行
  // textOption.setWrapMode(QTextOption::WordWrap);
  textOption.setWrapMode(QTextOption::WrapAnywhere);
  textOption.setAlignment(Qt::AlignLeft);
  cache->textLayout.setTextOption(textOption);

  cache->textLayout.setText(msg->content());
  cache->textLayout.setFont(msg->font());

  // // 动态计算最大宽度（基于视图宽度）

  // 动态计算最大宽度
  qreal viewportMaxWidth = m_view ? m_view->viewport()->width() * 0.60
                                  : 300.0;  // 展示区域的最大宽度
  QFontMetrics fm(m_view->font());
  qreal textWidth = fm.horizontalAdvance(msg->content());  // 文本的宽度
  // qDebug() << "[textWidth]: " << textWidth;
  qreal maxWidth = qMin(textWidth, viewportMaxWidth);  //
  // qDebug() << "[maxWidth]: " << maxWidth;

  // qDebug() << "view->width" << m_view->width();
  // qDebug() << "maxWidth" << maxWidth;

  cache->textLayout.beginLayout();
  qreal y = 0;
  while (true) {
    QTextLine line = cache->textLayout.createLine();
    if (!line.isValid())
      break;
    // qreal naturalWidth = line.naturalTextWidth(); // 获取自然宽度
    line.setLineWidth(maxWidth + 3);
    // line.setLineWidth(naturalWidth);
    line.setPosition(QPointF(0, y));  // 0, 行高
    y += line.height();               // 行高
    cache->lines.append(line);
  }
  cache->textLayout.endLayout();

  // 气泡尺寸计算
  cache->contentRect = cache->textLayout.boundingRect();

  // QString content = msg->content();
  // QStringList paragraphs = content.split('\n', Qt::SkipEmptyParts); // 分割段落
  // // 计算最终尺寸
  // cache->contentRect = cache->textLayout.boundingRect();

  // qDebug() << cache->contentRect;

  cache->bubbleRect = QRectF(
      msg->isOutgoing() ? option.rect.right() - cache->contentRect.width() -
                              2 * horizontalMargin
                        : option.rect.left() + horizontalMargin,
      option.rect.top() + verticalMargin,  // + 6
      cache->contentRect.width() + 2 * horizontalMargin,
      cache->contentRect.height() + 2 * verticalMargin);
  // qD
  // 获取的没有问题
  // qDebug() << "init: " << cache->bubbleRect;
  // qDebug() << "ORI: " << cache->bubbleRect.x() << cache->bubbleRect.y();

  m_layoutCache.insert(key, cache);
  // cache->test = "TEST: " + QString::number(cache->bubbleRect.topLeft().x()) + " "
  //               + QString::number(cache->bubbleRect.topLeft().y());
  // qDebug() << cache->contentRect << cache->bubbleRect;

  // if (index.row() == 3) {
  // qDebug() << "[Layout] Index:" << index.row() << "\n  Text Size:" << cache->contentRect
  //          << "\n  Bubble Rect:" << cache->bubbleRect
  //          << "\n  Viewport Width:" << (m_view ? m_view->width() : 0);

  // qDebug() << "[Layout] Index:" << index.row() << "\n  Text Size:" << cache->contentRect
  //          << "\n  Bubble Rect:" << cache->bubbleRect;
  // << "\n  Viewport Width:" << (m_view ? m_view->width() : 0);
  // }

  return cache;
}

// 其他辅助方法
QVector<QTextLayout::FormatRange> TtChatMessageDelegate::createSelectionFormats(
    const QModelIndex& index) const {
  // 定义一些列特定应用格式
  QVector<QTextLayout::FormatRange> formats;

  // 起始
  const int selStart =
      index.data(TtChatMessageModel::SelectionStartRole).toInt();
  // 终止
  const int selEnd = index.data(TtChatMessageModel::SelectionEndRole).toInt();
  // qDebug() << QString("[selStart]: %1 [selEnd]: %2").arg(selStart).arg(selEnd);

  if (selStart == selEnd) {
    // 原生样式
    return formats;
  }

  // 某一样式
  QTextLayout::FormatRange selection;
  // 字符起始索引
  selection.start = qMin(selStart, selEnd);
  // 选取长度
  selection.length = qAbs(selEnd - selStart);
  // 背景色
  selection.format.setBackground(QColor(173, 214, 255));
  // 前景色
  // selection.format.setForeground(Qt::white);
  selection.format.setForeground(Qt::black);
  // 追加 ---- 只有一段, 为什么需要 vector
  formats.append(selection);

  return formats;
}

QColor TtChatMessageDelegate::blendColors(QColor c1, QColor c2,
                                          float ratio) const {
  return QColor();
}

void TtChatMessageDelegate::drawSimpleText(QPainter* painter,
                                           const QStyleOptionViewItem& option,
                                           const QModelIndex& index) const {
  QString text = index.data(Qt::DisplayRole).toString();
  painter->setPen(Qt::black);
  painter->drawText(option.rect, Qt::AlignLeft, text);
}

void TtChatMessageDelegate::invalidateCache() {
  m_layoutCache.clear();
}

void TtChatMessageDelegate::setRenderQuality(RenderQuality quality) {
  if (m_renderQuality != quality) {
    m_renderQuality = quality;
    // emit updateEditorGeometries(); // 触发视图更新
    // emit upda
  }
}

void TtChatMessageDelegate::setView(QListView* view) {
  m_view = view;
}

void TtChatMessageDelegate::setResizing(bool resizing) {
  QMutexLocker locker(&m_cacheMutex);
  m_isResizing = resizing;
}

void TtChatMessageDelegate::updateLayoutForIndex(const QModelIndex& index) {
  const QString key = index.data(TtChatMessageModel::MessageIdRole).toString();
  m_layoutCache.remove(key);  // 清除特定项的缓存
}

void TtChatMessageDelegate::handleHoverIndexChanged(const QModelIndex& index) {}

void TtChatMessageDelegate::initStyleOption(QStyleOptionViewItem* option,
                                            const QModelIndex& index) const {
  QStyledItemDelegate::initStyleOption(option, index);
}

#else

TtChatMessageDelegate::TtChatMessageDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

TtChatMessageDelegate::~TtChatMessageDelegate() {}

// 核心绘制方法实现
void TtChatMessageDelegate::paint(QPainter* painter,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const {
  static int t = 0;
  painter->setRenderHint(QPainter::Antialiasing,
                         m_renderQuality >= MediumQuality);
  painter->setRenderHint(QPainter::TextAntialiasing,
                         m_renderQuality >= MediumQuality);

  if (!index.isValid()) {
    return;
  }
  // 初始化样式选项
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  LayoutCache* cache = getLayoutCache(index, option);
  if (!cache) {
    return;
  }

  drawBubble(painter, option, index, cache);     // 1. 绘制气泡背景
  drawSelection(painter, option, index, cache);  // 3. 绘制选择高亮

  TtChatMessage* msg = qobject_cast<TtChatMessage*>(
      index.data(TtChatMessageModel::MessageObjectRole).value<QObject*>());
  // 绘制是否被阅读后的标志
  // drawStatusIndicator(painter, option, msg);
  // QStyledItemDelegate::paint(painter, option, index);
}

QHash<QString, QSize> size_cache_;
int cache_width_;

QSize TtChatMessageDelegate::sizeHint(const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const {
  QString id = index.data(TtChatMessageModel::MessageIdRole).toString();
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);  // 必须初始化以获取正确尺寸
  if (cache_width_ == opt.rect.width() && size_cache_.contains(id)) {
    return size_cache_.value(id);
  }

  // bug 加上这段代码后, 即使不作为返回的 size, 也会导致面积过大
  LayoutCache* cache =
      getLayoutCache(index, option);  // 需要添加 option 参数    if (!cache)

  QSize newSize =
      QSize(opt.rect.width(), cache->bubbleRect.height() + verticalMargin);

  size_cache_[id] = newSize;
  cache_width_ = opt.rect.width();

  return newSize;
}

// 交互支持方法
bool TtChatMessageDelegate::hitTest(const QPoint& pos, const QModelIndex& index,
                                    QRect* textRect) const {
  QStyleOptionViewItem opt;
  opt.rect = m_view->visualRect(index);  // 关键
  initStyleOption(&opt, index);          // 初始化样式选项
  LayoutCache* cache =
      getLayoutCache(index, opt);  // 需要添加 option 参数    if (!cache)
  if (!cache) {
    return false;
  }

  // 转换为消息内容坐标系
  // qDebug() << "pos : " << pos; // 每个项内部的 局部 pos  左上角 0, 0
  // qDebug() << "bubble: " << cache->bubbleRect.x() << cache->bubbleRect.y() -
  // opt.rect.top(); qDebug() << cache->bubbleRect; qDebug() << cache->test;

  // QPoint contentPos = pos - cache->bubbleRect.topLeft().toPoint();
  // QPoint contentPos = pos - cache->bubbleRect.topLeft().toPoint() -
  // opt.rect.top(); qDebug() << "pos.y" << pos.y();
  // 以文本布局判断是否在文本区域上(试验气泡框)
  QPoint contentPos = QPoint(
      (pos.x() - cache->bubbleRect.topLeft().toPoint().x()),
      (pos.y() - cache->bubbleRect.topLeft().toPoint().y() + opt.rect.top()));
  // 文本内容布局
  // qDebug() << "content: " << cache->contentRect.toRect();
  // qDebug() << "bubble: " << cache->bubbleRect.toRect();
  // QRect layoutRect = cache->contentRect.toRect();
  QRect layoutRect = cache->bubbleRect.toRect();
  layoutRect.moveTo(0, 0);
  // QRect layoutRect = cache->bubbleRect.toRect();
  // qDebug() << "layoutRect " << layoutRect;
  // qDebug() << "contentPos " << contentPos;

  return layoutRect.contains(contentPos);
}

int TtChatMessageDelegate::posToCursor(const QPointF& pos,
                                       const QModelIndex& index) const {
  // 获取 start
  QStyleOptionViewItem opt;
  initStyleOption(&opt, index);          // 初始化样式选项
  opt.rect = m_view->visualRect(index);  // 从视图获取实际矩形
  LayoutCache* cache = getLayoutCache(index, opt);
  if (!cache) {
    return -1;
  }
  qreal clickPosX = (pos - cache->bubbleRect.topLeft()).x() - verticalMargin;
  qreal clickPosY = (pos - cache->bubbleRect.topLeft()).y();
  clickPosY +=
      opt.rect.toRectF().top();  // 缺少之前项的高度, 气泡高度依赖于之前项的高度
  // y 值有时会变负数 最高 -4
  QPointF localPos(clickPosX, clickPosY);
  if (localPos.y() < 0) {
    localPos.setY(0);
  }
  if (cache->lines.count() == 1) {
    auto line = cache->lines[0];
    int cursorIndex = line.xToCursor(localPos.x());
    return cursorIndex;

  } else {
    foreach (const QTextLine& line, cache->lines) {
      if (line.lineNumber() == cache->lines.count() - 1) {
        int cursorIndex = line.xToCursor(localPos.x());
        return cursorIndex;
      } else {
        if (line.rect().contains(localPos)) {
          int cursorIndex =
              line.xToCursor(localPos.x(), QTextLine::CursorBetweenCharacters);
          return cursorIndex;
        }
      }
    }
  }
  return -1;
}

void TtChatMessageDelegate::drawBubble(QPainter* painter,
                                       const QStyleOptionViewItem& option,
                                       const QModelIndex& index,
                                       LayoutCache* cache) const {
  TtChatMessage* msg = qobject_cast<TtChatMessage*>(
      index.data(TtChatMessageModel::MessageObjectRole).value<QObject*>());

  // 使用动态计算的 bubbleRect
  QRectF bubbleRect = cache->bubbleRect;

  // 绘制圆角矩形气泡
  QPainterPath path;
  path.addRoundedRect(bubbleRect, 4, 4);

  path.closeSubpath();

  // 填充气泡
  painter->save();
  painter->setPen(Qt::NoPen);
  painter->setBrush(msg->bubbleColor());
  painter->drawPath(path);
  painter->restore();
}

void TtChatMessageDelegate::drawSelection(QPainter* painter,
                                          const QStyleOptionViewItem& option,
                                          const QModelIndex& index,
                                          LayoutCache* cache) const {
  QVector<QTextLayout::FormatRange> formats = createSelectionFormats(index);

  painter->save();
  QPointF textPos =
      cache->bubbleRect.topLeft() + QPointF(horizontalMargin, verticalMargin);
  painter->translate(textPos);

  // 动态应用选择格式
  cache->textLayout.draw(painter, QPointF(0, 0), formats);
  painter->restore();
}

void TtChatMessageDelegate::drawStatusIndicator(
    QPainter* painter, const QStyleOptionViewItem& option,
    const TtChatMessage* msg) const {

  const int indicatorSize = 14;
  const int margin = 6;
  QRect statusRect;

  // 根据消息方向定位
  if (msg->isOutgoing()) {
    statusRect = QRect(option.rect.right() - indicatorSize - margin,
                       option.rect.bottom() - indicatorSize - margin,
                       indicatorSize, indicatorSize);
  } else {
    statusRect = QRect(option.rect.left() + margin,
                       option.rect.bottom() - indicatorSize - margin,
                       indicatorSize, indicatorSize);
  }

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  // 根据状态选择绘制方式
  switch (msg->status()) {
    case TtChatMessage::Sending:
      drawSendingIndicator(painter, statusRect);
      break;
    case TtChatMessage::Sent:
      drawSingleCheckmark(painter, statusRect);
      break;
    case TtChatMessage::Delivered:
    case TtChatMessage::Read:
      drawDoubleCheckmark(painter, statusRect);
      break;
    case TtChatMessage::Failed:
      drawErrorIndicator(painter, statusRect);
      break;
    default:
      break;
  }

  painter->restore();
}

void TtChatMessageDelegate::drawSingleCheckmark(QPainter* painter,
                                                const QRect& rect) const {
  QPen pen(QColor("#8F8F8F"), 1.8);
  painter->setPen(pen);

  QPainterPath path;
  path.moveTo(rect.left() + 3, rect.center().y());
  path.lineTo(rect.center().x() - 1, rect.bottom() - 3);
  path.lineTo(rect.right() - 3, rect.top() + 4);

  painter->drawPath(path);
}

void TtChatMessageDelegate::drawDoubleCheckmark(QPainter* painter,
                                                const QRect& rect) const {
  QPen pen(QColor("#34B7F1"), 1.8);
  painter->setPen(pen);

  // 第一个勾
  QPainterPath path1;
  path1.moveTo(rect.left() + 2, rect.center().y() - 2);
  path1.lineTo(rect.center().x() - 4, rect.bottom() - 5);
  path1.lineTo(rect.right() - 2, rect.top() + 2);

  // 第二个勾
  QPainterPath path2;
  path2.moveTo(rect.left() + 6, rect.center().y() - 2);
  path2.lineTo(rect.center().x(), rect.bottom() - 5);
  path2.lineTo(rect.right() - 2, rect.top() + 6);

  painter->drawPath(path1);
  painter->drawPath(path2);
}

void TtChatMessageDelegate::drawErrorIndicator(QPainter* painter,
                                               const QRect& rect) const {
  painter->setPen(Qt::NoPen);
  painter->setBrush(QColor("#FF4444"));

  // 绘制圆形背景
  painter->drawEllipse(rect.adjusted(1, 1, -1, -1));

  // 绘制感叹号
  painter->setPen(QPen(Qt::white, 1.5));
  painter->drawLine(rect.center().x(), rect.top() + 4, rect.center().x(),
                    rect.bottom() - 6);
  painter->drawEllipse(QRect(rect.center().x() - 1, rect.bottom() - 4, 2, 2));
}

void TtChatMessageDelegate::drawSendingIndicator(QPainter* painter,
                                                 const QRect& rect) const {
  static int rotation = 0;
  rotation = (rotation + 5) % 360;

  QPen pen(QColor("#AAAAAA"), 1.5);
  painter->setPen(pen);

  QPainterPath path;
  path.addEllipse(rect.adjusted(2, 2, -2, -2));

  painter->save();
  painter->translate(rect.center());
  painter->rotate(rotation);
  painter->translate(-rect.center());

  // 绘制进度圆弧
  painter->drawArc(rect.adjusted(2, 2, -2, -2),
                   0 * 16,   // 起始角度
                   270 * 16  // 跨度角度
  );
  painter->restore();
}

// bool
// 缓存管理
TtChatMessageDelegate::LayoutCache* TtChatMessageDelegate::getLayoutCache(
    const QModelIndex& index, const QStyleOptionViewItem& option) const {

  const QString key = index.data(TtChatMessageModel::MessageIdRole).toString();

  QObject* obj =
      index.data(TtChatMessageModel::MessageObjectRole).value<QObject*>();
  TtChatMessage* msg = qobject_cast<TtChatMessage*>(obj);
  if (!msg) {
    return nullptr;
  }

  // 创建新的布局缓存
  LayoutCache* cache = new LayoutCache;

  // 设置文本布局参数
  QTextOption textOption;
  textOption.setWrapMode(
      QTextOption::WrapAtWordBoundaryOrAnywhere);  // 按字符换行
  // textOption.setWrapMode(QTextOption::WordWrap);
  // textOption.setWrapMode(QTextOption::WrapAnywhere);
  textOption.setAlignment(Qt::AlignLeft);
  cache->textLayout.setTextOption(textOption);

  cache->textLayout.setText(msg->content());
  cache->textLayout.setFont(msg->font());


  // 动态计算最大宽度
  qreal viewportMaxWidth = m_view ? m_view->viewport()->width() * 0.72
                                  : 300.0;  // 展示区域的最大宽度

  qreal maxWidth = viewportMaxWidth;  // 直接使用视图最大宽度

  cache->textLayout.beginLayout();
  qreal y = 0;
  qreal actualMaxLineWidth = 0;  // 记录实际布局后的最大行宽
  while (true) {
    QTextLine line = cache->textLayout.createLine();
    if (!line.isValid())
      break;

    line.setLineWidth(maxWidth);
    line.setPosition(QPointF(0, y));
    y += line.height();

    // 更新实际最大行宽
    actualMaxLineWidth = qMax(actualMaxLineWidth, line.naturalTextWidth());

    cache->lines.append(line);
  }
  cache->textLayout.endLayout();

  // 使用实际布局后的最大行宽作为内容宽度
  cache->contentRect = QRectF(0, 0, actualMaxLineWidth, y);

  // 气泡宽度 = 实际最大行宽 + 边距
  qreal bubbleWidth = actualMaxLineWidth + 2 * horizontalMargin;

  cache->bubbleRect =
      QRectF(msg->isOutgoing() ? option.rect.right() - bubbleWidth
                               : option.rect.left(),
             option.rect.top() + verticalMargin, bubbleWidth,
             cache->contentRect.height() + 2 * verticalMargin);

  return cache;
}

// 其他辅助方法
QVector<QTextLayout::FormatRange> TtChatMessageDelegate::createSelectionFormats(
    const QModelIndex& index) const {
  // 定义一些列特定应用格式
  QVector<QTextLayout::FormatRange> formats;

  // 起始
  const int selStart =
      index.data(TtChatMessageModel::SelectionStartRole).toInt();
  // 终止
  const int selEnd = index.data(TtChatMessageModel::SelectionEndRole).toInt();
  // qDebug() << QString("[selStart]: %1 [selEnd]:
  // %2").arg(selStart).arg(selEnd);

  if (selStart == selEnd) {
    // 原生样式
    return formats;
  }

  // 某一样式
  QTextLayout::FormatRange selection;
  // qDebug() << "indx: " << index.row() << selStart << selEnd;
  // 字符起始索引
  selection.start = qMin(selStart, selEnd);
  // 选取长度
  selection.length = qAbs(selEnd - selStart);
  // 背景色
  selection.format.setBackground(QColor(173, 214, 255));
  // 前景色
  // selection.format.setForeground(Qt::white);
  selection.format.setForeground(Qt::black);
  formats.append(selection);

  return formats;
}

QColor TtChatMessageDelegate::blendColors(QColor c1, QColor c2,
                                          float ratio) const {
  return QColor();
}

void TtChatMessageDelegate::drawSimpleText(QPainter* painter,
                                           const QStyleOptionViewItem& option,
                                           const QModelIndex& index) const {
  QString text = index.data(Qt::DisplayRole).toString();
  painter->setPen(Qt::black);
  painter->drawText(option.rect, Qt::AlignLeft, text);
}

void TtChatMessageDelegate::invalidateCache() {
  // m_layoutCache.clear();
}

void TtChatMessageDelegate::setRenderQuality(RenderQuality quality) {
  if (m_renderQuality != quality) {
    m_renderQuality = quality;
    // emit updateEditorGeometries(); // 触发视图更新
    // emit upda
  }
}

void TtChatMessageDelegate::setView(QListView* view) {
  m_view = view;
}

void TtChatMessageDelegate::setResizing(bool resizing) {
  QMutexLocker locker(&m_cacheMutex);
  m_isResizing = resizing;
  // 新增：调整大小时使用低质量渲染
  setRenderQuality(resizing ? LowQuality : HighQuality);
  if (!resizing) {
    invalidateCache();
  }
}

void TtChatMessageDelegate::updateLayoutForIndex(const QModelIndex& index) {
  // const QString key = index.data(TtChatMessageModel::MessageIdRole).toString();
  // m_layoutCache.remove(key); // 清除特定项的缓存
}

void TtChatMessageDelegate::clearCache() {
  // m_layoutCache.clear();
  size_cache_.clear();
}

void TtChatMessageDelegate::handleHoverIndexChanged(const QModelIndex& index) {}

void TtChatMessageDelegate::initStyleOption(QStyleOptionViewItem* option,
                                            const QModelIndex& index) const {
  static int j = 0;
  QStyledItemDelegate::initStyleOption(option, index);
}

#endif

}  // namespace Ui
