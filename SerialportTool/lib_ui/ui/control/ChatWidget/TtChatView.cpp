#include "ui/control/ChatWidget/TtChatView.h"
#include "ui/control/ChatWidget/TtChatMessageDelegate.h"

#include <QApplication>
#include <QClipboard>
#include <QItemDelegate>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QScrollBar>

namespace Ui {

#if 0
TtChatView::TtChatView(QWidget* parent)
    : QListView(parent),
      m_model(nullptr),
      m_delegate(new TtChatMessageDelegate(this)),
      m_scrollTimer(new QTimer(this)),
      m_fetchTimer(new QTimer(this)) {
  setItemDelegate(m_delegate);
  setVerticalScrollMode(ScrollPerPixel);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSelectionMode(ContiguousSelection);
  setMouseTracking(true);
  viewport()->setAttribute(Qt::WA_InputMethodEnabled);

  // 创建右键菜单
  m_contextMenu = new QMenu(this);
  m_copyAction =
      m_contextMenu->addAction(tr("复制"), this, &TtChatView::copySelectedText);
  m_copyAction->setEnabled(false);

  // 滚动优化定时器
  m_scrollTimer->setSingleShot(true);
  m_scrollTimer->setInterval(100);
  connect(m_scrollTimer, &QTimer::timeout, this,
          &TtChatView::updateVisibleItems);

  // 加载更多防抖定时器
  m_fetchTimer->setSingleShot(true);
  m_fetchTimer->setInterval(300);
  connect(verticalScrollBar(), &QScrollBar::valueChanged, m_fetchTimer,
          QOverload<>::of(&QTimer::start));
  connect(m_fetchTimer, &QTimer::timeout, this, &TtChatView::checkFetchMore);
  // 配置防抖定时器（300ms）
  m_resizeTimer.setSingleShot(true);
  connect(&m_resizeTimer, &QTimer::timeout, this,
          &TtChatView::handleDelayedResize);
}

TtChatView::~TtChatView() {
  // 确保释放时断开所有连接
  if (m_model)
    m_model->disconnect(this);
}

void TtChatView::setModel(TtChatMessageModel* model) {
  // 断开旧模型的连接
  if (m_model) {
    qDebug() << "1";
    disconnect(m_model, &TtChatMessageModel::needFetchOlder, this,
               &TtChatView::needLoadOlderMessages);
    disconnect(m_model, &TtChatMessageModel::needFetchNewer, this,
               &TtChatView::needLoadNewerMessages);
    disconnect(m_model, &TtChatMessageModel::dataChanged, this,
               &TtChatView::onDataChanged);
  }

  // 设置新模型
  QListView::setModel(model);
  m_model = model;

  if (model) {
    // 连接模型信号
    connect(model, &TtChatMessageModel::needFetchOlder, this,
            &TtChatView::needLoadOlderMessages);
    connect(model, &TtChatMessageModel::needFetchNewer, this,
            &TtChatView::needLoadNewerMessages);
    connect(model, &TtChatMessageModel::dataChanged, this,
            &TtChatView::onDataChanged);

    // 初始化委托
    m_delegate->setView(this);
    m_delegate->invalidateCache();  // 清除旧缓存

    // 初始加载检查
    QTimer::singleShot(0, this, [this] {
      checkFetchMore();
      scrollToBottom();
    });
  }
}

TtChatMessageModel* TtChatView::chatModel() const {
  return m_model;
}

// 核心交互实现
void TtChatView::mousePressEvent(QMouseEvent* e) {
  // bug 点击的坐标与绘制存在 边距
  qDebug() << "mousepress";
  if (e->button() == Qt::LeftButton) {
    const QModelIndex index = indexAt(e->pos());
    if (index.isValid()) {
      // 转换为消息内容区域坐标
      QRect itemRect = visualRect(index);
      QPointF contentPos = e->pos() - itemRect.topLeft();
      // 检测是否点击文本区域
      if (m_delegate->hitTest(contentPos.toPoint(), index)) {
        m_selection.anchorIndex = index;
        m_selection.anchorCursorPos =
            m_delegate->posToCursor(contentPos, index);
        // LOG_INFO() << contentPos << index.row() << m_selection.anchorCursorPos;
        m_selection.active = true;

        // 初始化当前选择位置
        m_selection.currentIndex = index;  // 初始化记录
        m_selection.currentCursorPos = m_selection.anchorCursorPos;

        // 更新模型选择状态
        m_model->setSelection(index, m_selection.anchorCursorPos,
                              m_selection.currentCursorPos);
        // return;
      }
    }
  }

  // 处理普通点击（非拖拽选择）
  if (e->button() == Qt::LeftButton && !m_selection.active) {
    const QModelIndex index = indexAt(e->pos());
    if (!index.isValid()) {
      m_model->clearAllSelections();
      viewport()->update();
    }
  }
  QListView::mousePressEvent(e);
}

void TtChatView::mouseMoveEvent(QMouseEvent* e) {
  // mouse press 设置了 true
  if (m_selection.active) {
    const QModelIndex newIndex = indexAt(e->pos());
    if (newIndex.isValid()) {
      // 转换到内容坐标系
      QPointF contentPos = e->pos() - visualRect(newIndex).topLeft();

      // 实时拖动的摁下坐标
      int cursorPos = m_delegate->posToCursor(contentPos, newIndex);
      // 更新当前选择位置
      m_selection.currentIndex = newIndex;
      m_selection.currentCursorPos = cursorPos;

      // 处理跨消息选择
      updateSelections();

      viewport()->update();
    }
    // QPointF contentPos = e->pos() - visualRect(currentIndex).topLeft();

    // // 更新选择终点
    // updateSelections(currentIndex, contentPos);

    // // 自动滚动支持
    // const int edgeMargin = 50;
    // const QRect visRect = viewport()->rect();
    // if (e->pos().y() < visRect.top() + edgeMargin) {
    //     verticalScrollBar()->setValue(verticalScrollBar()->value() - 10);
    // } else if (e->pos().y() > visRect.bottom() - edgeMargin) {
    //     verticalScrollBar()->setValue(verticalScrollBar()->value() + 10);
    // }
    if (e->pos().y() < viewport()->rect().top() + 20) {
      verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
    } else if (e->pos().y() > viewport()->rect().bottom() - 20) {
      verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
    }
  }
  QListView::mouseMoveEvent(e);
}

void TtChatView::mouseReleaseEvent(QMouseEvent* e) {
  if (m_selection.active) {
    m_selection.active = false;
    m_model->commitSelections();  // 确认选中范围
    emit messageSelectionChanged();
  }

  // 点击右键, 然后释放了
  if (e->button() == Qt::RightButton &&
      m_model->selectedText().isEmpty() == false) {
    // 文本不为空
    m_copyAction->setEnabled(true);
    m_contextMenu->exec(e->globalPosition().toPoint());
  } else {
    // qDebug() << "no";
    m_copyAction->setEnabled(false);
  }

  QListView::mouseReleaseEvent(e);
}

void TtChatView::wheelEvent(QWheelEvent* e) {
  QListView::wheelEvent(e);
}

// 高性能滚动优化
void TtChatView::paintEvent(QPaintEvent* e) {
  // 仅绘制可见区域
  QRect exposedRect = e->rect().intersected(viewport()->rect());
  QListView::paintEvent(e);

  // 绘制加载指示器
  if (m_loadingOlder) {
    QPainter p(viewport());
    p.fillRect(exposedRect, QColor(0, 0, 0, 50));
    p.drawText(exposedRect, Qt::AlignCenter, tr("Loading..."));
  }
}

void TtChatView::scrollContentsBy(int dx, int dy) {
  QListView::scrollContentsBy(dx, dy);
  m_scrollTimer->start();  // 延迟更新可见项
}

void TtChatView::handleModelReset() {}

void TtChatView::updateVisibleItems() {}

// 动态数据加载
void TtChatView::checkFetchMore() {
  const int pos = verticalScrollBar()->value();
  if (pos < scrollMargin()) {
    // 添加 QModelIndex() 参数
    if (!m_loadingOlder && m_model->canFetchMore(QModelIndex())) {
      m_loadingOlder = true;
      emit loadingStateChanged();
      // 修改为使用 Qt 原生 fetchMore 签名
      m_model->fetchMore(QModelIndex());
      // 将回调逻辑移到模型的信号中
      m_loadingOlder = false;
      emit loadingStateChanged();
    }
  } else if (pos > verticalScrollBar()->maximum() - scrollMargin()) {
    emit needLoadNewerMessages();
  }
}

// 消息定位
void TtChatView::scrollToMessage(const QString& messageId) {
  for (int i = 0; i < m_model->rowCount(); ++i) {
    QModelIndex idx = m_model->index(i);
    if (idx.data(TtChatMessageModel::MessageIdRole).toString() == messageId) {
      scrollTo(idx, PositionAtCenter);
      return;
    }
  }
}

void TtChatView::copySelectedText() {
  // 槽函数
  QString selected = m_model->selectedText();
  if (!selected.isEmpty()) {
    QApplication::clipboard()->setText(selected);
  }
}

QModelIndexList TtChatView::visibleIndexes() const {
  QModelIndexList indexes;

  // 获取视口可见区域
  const QRect viewportRect = viewport()->rect();
  const int firstVisibleRow = indexAt(viewportRect.topLeft()).row();
  const int lastVisibleRow = indexAt(viewportRect.bottomRight()).row();

  // 遍历可能可见的行
  for (int row = firstVisibleRow; row <= lastVisibleRow; ++row) {
    QModelIndex idx = model()->index(row, 0);
    if (visualRect(idx).intersects(viewportRect)) {
      indexes.append(idx);
    }
  }
  return indexes;
}

void TtChatView::updateSelections() {
  m_model->beginUpdateSelections();

  m_model->clearAllSelections();

  const int anchorRow = m_selection.anchorIndex.row();
  const int currentRow = m_selection.currentIndex.row();
  const bool isForward = anchorRow <= currentRow;

  // 确定遍历方向和范围
  const int startRow = qMin(anchorRow, currentRow);
  const int endRow = qMax(anchorRow, currentRow);

  for (int row = startRow; row <= endRow; ++row) {
    QModelIndex idx = model()->index(row, 0);
    if (!idx.isValid())
      continue;

    const int textLength =
        idx.data(TtChatMessageModel::ContentRole).toString().length();
    int startPos = 0;
    int endPos = textLength;

    // 处理首尾消息的部分选择
    if (row == anchorRow) {
      if (isForward) {
        startPos = m_selection.anchorCursorPos;
        endPos =
            (row == currentRow) ? m_selection.currentCursorPos : textLength;
        // endPos = textLength;
      } else {
        startPos = 0;
        endPos = m_selection.anchorCursorPos;
      }
    } else if (row == currentRow) {
      if (isForward) {
        startPos = 0;
        endPos = m_selection.currentCursorPos;
      } else {
        startPos = m_selection.currentCursorPos;
        endPos = textLength;
      }
    }
    // 中间消息处理
    else {
      if (isForward) {
        // 正向选择：全选中间消息
        startPos = 0;
        endPos = textLength;
      } else {
        // 反向选择：全选中间消息
        startPos = 0;
        endPos = textLength;
      }
    }

    // 边界保护
    startPos = qBound(0, startPos, textLength);
    endPos = qBound(0, endPos, textLength);

    // 交换反向选择的起止位置
    if (startPos > endPos) {
      std::swap(startPos, endPos);
    }

    m_model->setSelection(idx, startPos, endPos);
  }
  m_model->endUpdateSelections();
  viewport()->update();
}

void TtChatView::handleDelayedResize() {
  m_delegate->setResizing(false);
  m_delegate->invalidateCache();
  viewport()->update();  // 触发完整重绘
}

// 样式和布局调整
void TtChatView::resizeEvent(QResizeEvent* e) {
  QListView::resizeEvent(e);
  scheduleDelayedItemsLayout();

  // // 立即更新可见项的布局
  // const QModelIndexList visible = this->visibleIndexes();
  // foreach (const QModelIndex &idx, visible) {
  //     m_delegate->updateLayoutForIndex(idx);
  // }
  // m_delegate->invalidateCache();

  viewport()->update();
}

// 属性访问器
bool TtChatView::isLoadingOlderMessages() const {
  return m_loadingOlder;
}
int TtChatView::scrollMargin() const {
  return m_scrollMargin;
}
void TtChatView::setScrollMargin(int margin) {
  if (m_scrollMargin != margin) {
    m_scrollMargin = margin;
    emit scrollMarginChanged();
  }
}

void TtChatView::triggerResize() {}

void TtChatView::onDataChanged(const QModelIndex& topLeft,
                               const QModelIndex& bottomRight,
                               const QVector<int>& roles) {
  Q_UNUSED(bottomRight)
  // 当选择状态变化时更新显示
  if (roles.contains(TtChatMessageModel::SelectionStartRole) ||
      roles.contains(TtChatMessageModel::SelectionEndRole)) {
    viewport()->update(visualRect(topLeft));
  }
}

#else

TtChatView::TtChatView(QWidget* parent)
    : QListView(parent),
      m_model(nullptr),
      m_delegate(new TtChatMessageDelegate(this)),
      m_scrollTimer(new QTimer(this)),
      m_fetchTimer(new QTimer(this))
{
  // setStyleSheet("background-color: black");
  // setMinimumWidth();
  setFrameShape(QFrame::NoFrame);
  setItemDelegate(m_delegate);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  setUniformItemSizes(false);  // 允许每个项具有不同的大小
  // setMouseTracking(true);
  setSelectionMode(ContiguousSelection);

  // viewport()->setAttribute(Qt::WA_InputMethodEnabled);

  setViewMode(QListView::ListMode);
  setFlow(QListView::TopToBottom);

  // setSpacing(4);
  // setWordWrap(true); // 作用
  setUpdatesEnabled(true);
  // setResizeMode(QListView::Adjust);

  // 创建右键菜单
  m_contextMenu = new QMenu(this);
  m_copyAction =
      m_contextMenu->addAction(tr("复制"), this, &TtChatView::copySelectedText);
  m_copyAction->setEnabled(false);

  // 在 TtChatView 中添加防抖定时器
  // QTimer m_resizeTimer;

  // 初始化时设置单次触发和延迟
  m_resizeTimer.setSingleShot(true);
  m_resizeTimer.setInterval(25);  // 10ms 延迟
  // 连接定时器到更新槽函数
  connect(&m_resizeTimer, &QTimer::timeout, this,
          &TtChatView::handleViewportResize);

  // 滚动优化定时器
  m_scrollTimer->setSingleShot(true);
  m_scrollTimer->setInterval(100);
  connect(m_scrollTimer, &QTimer::timeout, this,
          &TtChatView::updateVisibleItems);

  // 连接滚动条信号到检查函数
  connect(verticalScrollBar(), &QScrollBar::valueChanged, this,
          &TtChatView::checkFetchMore);
}

TtChatView::~TtChatView() {
  // 确保释放时断开所有连接
  if (m_model)
    m_model->disconnect(this);
}

void TtChatView::setModel(TtChatMessageModel* model) {
  // 断开旧模型的连接
  // if (m_model) {
  //     qDebug() << "1";
  //     disconnect(m_model,
  //                &TtChatMessageModel::needFetchOlder,
  //                this,
  //                &TtChatView::needLoadOlderMessages);
  //     disconnect(m_model,
  //                &TtChatMessageModel::needFetchNewer,
  //                this,
  //                &TtChatView::needLoadNewerMessages);
  //     disconnect(m_model, &TtChatMessageModel::dataChanged, this,
  //     &TtChatView::onDataChanged);
  // }

  // 设置新模型
  QListView::setModel(model);
  m_model = model;

  if (model) {
    //     // 连接模型信号
    //     connect(model, &TtChatMessageModel::needFetchOlder, this,
    //     &TtChatView::needLoadOlderMessages);
    // connect(model,
    // &TtChatMessageModel::needFetchNewer, this,
    // &TtChatView::needLoadNewerMessages);
    // connect(model, &TtChatMessageModel::dataChanged, this,
    //         &TtChatView::onDataChanged);

    // 初始化委托
    m_delegate->setView(this);
    // m_delegate->invalidateCache(); // 清除旧缓存

    //     // 初始加载检查
    //     QTimer::singleShot(0, this, [this] {
    //         checkFetchMore();
    //         scrollToBottom();
    //     });
  }
}

TtChatMessageModel* TtChatView::chatModel() const {
  return m_model;
}

// 核心交互实现
void TtChatView::mousePressEvent(QMouseEvent* e) {
  // 处理普通点击（非拖拽选择）
  if (e->button() == Qt::LeftButton && !m_selection.active) {
    // const QModelIndex index = indexAt(e->pos());
    // if (!index.isValid()) {
    m_model->clearAllSelections();
    viewport()->update();
    // }
  }

  // bug 点击的坐标与绘制存在 边距
  if (e->button() == Qt::LeftButton) {
    //qDebug() << "mousepress";
    tmp_index_list_.clear();
    m_isMousePressed = true;
    const QModelIndex index = indexAt(e->pos());
    if (index.isValid()) {
      m_anchorIndex = index;
      // 转换为消息内容区域坐标
      QRect itemRect = visualRect(index);
      QPointF contentPos = e->pos() - itemRect.topLeft();
      // 检测是否点击文本区域
      if (m_delegate->hitTest(contentPos.toPoint(), index)) {
        m_selection.anchorIndex = index;
        m_selection.anchorCursorPos =
            m_delegate->posToCursor(contentPos, index);
        //qDebug() << "press index: " << index;
        m_selection.active = true;

        // 初始化当前选择位置
        m_selection.currentIndex = index;  // 初始化记录
        m_selection.currentCursorPos = m_selection.anchorCursorPos;

        // 更新模型选择状态
        m_model->setSelection(index, m_selection.anchorCursorPos,
                              m_selection.currentCursorPos);
      }
    }
  }
  viewport()->update();
  QListView::mousePressEvent(e);
}

void TtChatView::mouseMoveEvent(QMouseEvent* e) {
  if (m_selection.active) {
    const QModelIndex newIndex = indexAt(e->pos());
    if (newIndex.isValid()) {
      // 转换到内容坐标系
      QPointF contentPos = e->pos() - visualRect(newIndex).topLeft();
      // 实时拖动的摁下坐标
      int cursorPos = m_delegate->posToCursor(contentPos, newIndex);
      // 边界气泡处 返回 -1
      // 更新当前选择位置
      m_selection.currentIndex = newIndex;
      tmp_index_list_.insert(newIndex, newIndex.row());

      auto first = tmp_index_list_.firstKey();
      // 如果先向上, 然后向下, 那么 old 应该比 first 小
      if (old_index_ < first) {
        m_model->setSelection(old_index_, -1, -1);
      }
      // for (auto it : tmp_index_list_.keys()) {
      //     // qDebug() << it.row();
      // }
      // 向上
      if (newIndex.row() > first.row()) {
        m_model->setSelection(tmp_index_list_.firstKey(), -1, -1);
        tmp_index_list_.remove(first);
      }
      auto last = tmp_index_list_.lastKey();
      if (last.isValid()) {
        if (newIndex.row() < last.row()) {
          m_model->setSelection(tmp_index_list_.lastKey(), -1, -1);
          tmp_index_list_.remove(last);
        }
      }
      m_selection.currentCursorPos = cursorPos;

      // 处理跨消息选择
      updateSelections();

      old_index_ = tmp_index_list_.firstKey();
    }

    int scrollDistance = 20;
    if (e->pos().y() < viewport()->rect().top() + scrollDistance) {
      QPropertyAnimation* animation =
          new QPropertyAnimation(verticalScrollBar(), "value");
      animation->setDuration(200);  // 动画时长（毫秒）
      animation->setStartValue(verticalScrollBar()->value());
      animation->setEndValue(verticalScrollBar()->value() - scrollDistance);
      animation->setEasingCurve(QEasingCurve::OutQuad);  // 平滑效果
      animation->start(QPropertyAnimation::KeepWhenStopped);
    } else if (e->pos().y() > viewport()->rect().bottom() - 20) {
      QPropertyAnimation* animation =
          new QPropertyAnimation(verticalScrollBar(), "value");
      animation->setDuration(200);  // 动画时长（毫秒）
      animation->setStartValue(verticalScrollBar()->value());
      animation->setEndValue(verticalScrollBar()->value() + scrollDistance);
      animation->setEasingCurve(QEasingCurve::OutQuad);  // 平滑效果
      animation->start(QPropertyAnimation::KeepWhenStopped);
    }
  }
  QListView::mouseMoveEvent(e);
}

void TtChatView::mouseReleaseEvent(QMouseEvent* e) {
  if (m_selection.active) {
    m_selection.active = false;
    m_model->commitSelections();  // 确认选中范围
                                  // emit messageSelectionChanged();
  }
  if (e->button() == Qt::LeftButton) {
    m_isMousePressed = false;
    m_scrollDirection = 0;
    // 提交选中范围
    // emit messageSelectionChanged();
  }
  // 点击右键, 然后释放了
  if (e->button() == Qt::RightButton &&
      m_model->selectedText().isEmpty() == false) {
    // 文本不为空
    m_copyAction->setEnabled(true);
    m_contextMenu->exec(e->globalPosition().toPoint());
  } else {
    m_copyAction->setEnabled(false);
  }

  QListView::mouseReleaseEvent(e);
}

void TtChatView::wheelEvent(QWheelEvent* e) {
  // 获取滚轮的滚动距离（像素或角度）
  const QPoint delta = e->angleDelta();  // 获取滚轮的角度变化
  const int scrollStep = 40;             // 自定义滚动步长（像素）

  // 计算自定义滚动距离
  int scrollDistance = 0;
  if (!delta.isNull()) {
    // 根据滚轮方向计算滚动距离
    if (delta.y() != 0) {
      scrollDistance = (delta.y() > 0)
                           ? -scrollStep
                           : scrollStep;  // 向上滚动为负，向下滚动为正
    }
    // 使用动画实现平滑滚动
    QPropertyAnimation* animation =
        new QPropertyAnimation(verticalScrollBar(), "value");
    animation->setDuration(200);  // 动画时长（毫秒）
    animation->setStartValue(verticalScrollBar()->value());
    animation->setEndValue(verticalScrollBar()->value() + scrollDistance);
    animation->setEasingCurve(QEasingCurve::OutQuad);  // 平滑效果
    animation->start(QPropertyAnimation::DeleteWhenStopped);
  }

  if (m_isMousePressed) {
    //     // 获取滚动方向
    const int delta = e->angleDelta().y();
    if (delta > 0) {
      m_scrollDirection = -1;  // 向上滚动
    } else if (delta < 0) {
      m_scrollDirection = 1;  // 向下滚动
    }

    const QPoint mousePos = viewport()->mapFromGlobal(QCursor::pos());
    const QModelIndex currentIndex = indexAt(mousePos);
    if (currentIndex.isValid()) {
      const QPointF contentPos = mousePos - visualRect(currentIndex).topLeft();
      const int cursorPos = m_delegate->posToCursor(contentPos, currentIndex);
      // 要么 0, 要么全部, 分左右选项
      m_selection.currentIndex = currentIndex;
      m_selection.currentCursorPos = cursorPos;
      // 更新选中范围
      updateSelections();
    }
  }
  e->accept();
}

// 高性能滚动优化
void TtChatView::paintEvent(QPaintEvent* e) {
  QListView::paintEvent(e);
}

void TtChatView::scrollContentsBy(int dx, int dy) {
  QListView::scrollContentsBy(dx, dy);
  m_scrollTimer->start();  // 延迟更新可见项
}

void TtChatView::handleModelReset() {}

void TtChatView::updateVisibleItems() {
  const auto visible = visibleIndexes();
  if (!visible.isEmpty()) {
    QModelIndex first = visible.first();
    QModelIndex last = visible.last();
    emit m_model->dataChanged(first, last, {Qt::SizeHintRole});
  }
}

// 动态数据加载
void TtChatView::checkFetchMore() {
  const int pos = verticalScrollBar()->value();
  if (pos < scrollMargin()) {
    if (!m_loadingOlder && m_model->canFetchMore(QModelIndex())) {
      m_loadingOlder = true;
      emit loadingStateChanged();
      m_model->fetchMore(QModelIndex());
      m_loadingOlder = false;
      emit loadingStateChanged();
    }
  } else if (pos > verticalScrollBar()->maximum() - scrollMargin()) {
    emit needLoadNewerMessages();
  }
}

// 消息定位
void TtChatView::scrollToMessage(const QString& messageId) {
  for (int i = 0; i < m_model->rowCount(); ++i) {
    QModelIndex idx = m_model->index(i);
    if (idx.data(TtChatMessageModel::MessageIdRole).toString() == messageId) {
      scrollTo(idx, PositionAtCenter);
      return;
    }
  }
}

void TtChatView::copySelectedText() {
  // 槽函数
  QString selected = m_model->selectedText();
  if (!selected.isEmpty()) {
    QApplication::clipboard()->setText(selected);
  }
}

QModelIndexList TtChatView::visibleIndexes() {
  QModelIndexList indexes;

  const QRect viewportRect = viewport()->rect();
  QModelIndex firstVisibleRow = indexAt(viewportRect.topLeft());
  if (!firstVisibleRow.isValid()) {
    return indexes;
  }

  QModelIndex lastVisibleRow = indexAt(viewportRect.bottomRight());
  if (!lastVisibleRow.isValid()) {
    lastVisibleRow = model()->index(model()->rowCount() - 1, 0);
  }
  // 遍历可能可见的行
  for (int row = firstVisibleRow.row(); row <= lastVisibleRow.row(); ++row) {
    QModelIndex idx = model()->index(row, 0);
    if (visualRect(idx).intersects(viewportRect)) {
      indexes.append(idx);
    }
  }
  return indexes;
}

void TtChatView::updateSelections() {
  if (!m_anchorIndex.isValid() || !m_selection.currentIndex.isValid()) {
    return;
  }
  m_model->beginUpdateSelections();

  m_model->clearAllSelections();

  const int anchorRow = m_selection.anchorIndex.row();
  const int currentRow = m_selection.currentIndex.row();
  const bool isForward = (anchorRow >= currentRow);  // 向上

  // 确定遍历方向和范围
  const int startRow = qMin(anchorRow, currentRow);
  const int endRow = qMax(anchorRow, currentRow);

  // 从上往下遍历
  for (int row = startRow; row <= endRow; ++row) {
    QModelIndex idx = model()->index(row, 0);
    if (!idx.isValid())
      continue;

    const int textLength =
        idx.data(TtChatMessageModel::ContentRole).toString().length();
    int startPos = 0;
    int endPos = textLength;

    // 处理首尾消息的部分选择
    if (row == anchorRow) {
      // 最先选择的项
      if (isForward) {
        // 向上
        // qDebug() << "a";
        startPos = m_selection.anchorCursorPos;
        endPos = (row == currentRow) ? m_selection.currentCursorPos : 0;
      } else {
        // qDebug() << "b";
        startPos = m_selection.anchorCursorPos;
        endPos =
            (row == currentRow) ? m_selection.currentCursorPos : textLength;
      }
    } else if (row == currentRow) {
      // 最后选择的项
      if (isForward) {
        // qDebug() << "c";
        startPos = m_selection.currentCursorPos;
        endPos = textLength;
      } else {
        // qDebug() << "d";
        startPos = 0;
        endPos = m_selection.currentCursorPos;
      }
    } else {
      // qDebug() << "f";
      // 中间消息处理
      startPos = 0;
      endPos = textLength;
    }

    // 边界保护
    // startPos = qBound(0, startPos, textLength);
    // endPos = qBound(0, endPos, textLength);

    // 交换反向选择的起止位置
    if (startPos > endPos) {
      std::swap(startPos, endPos);
    }
    // qDebug() << "startIndex: " << startRow;
    // qDebug() << "endIndex: " << endRow;
    // qDebug() << "--------------------";
    // 真正影响绘制
    m_model->setSelection(idx, startPos, endPos);
  }
  m_model->endUpdateSelections();
  viewport()->update();
}

void TtChatView::handleViewportResize() {
  const auto visible = visibleIndexes();

  if (visible.isEmpty()) {
    return;
  }
  // 触发局部 dataChanged 信号，仅更新 SizeHintRole
  QModelIndex first = visible.first();
  QModelIndex last = visible.last();

  // 触发视图重新绘制, 导致卡顿
  emit m_model->dataChanged(first, last, {Qt::SizeHintRole});
}

// 样式和布局调整
void TtChatView::resizeEvent(QResizeEvent* e) {
  // scheduleDelayedItemsLayout(); // 超过 2000 条, 面积改变会卡

  QListView::resizeEvent(e);
  if (e->size().width() != e->oldSize().width()) {
    m_resizeTimer.start();
    m_delegate->setResizing(true);
    QTimer::singleShot(
        200, [this] { m_delegate->setResizing(false); });  // 200ms 后更新
    // 高度变化时, 也会触发
    m_delegate->clearCache();
  }
}

// 属性访问器
bool TtChatView::isLoadingOlderMessages() const {
  return m_loadingOlder;
}
int TtChatView::scrollMargin() const {
  return m_scrollMargin;
}
void TtChatView::setScrollMargin(int margin) {
  if (m_scrollMargin != margin) {
    m_scrollMargin = margin;
    emit scrollMarginChanged();
  }
}

void TtChatView::triggerResize() {}

void TtChatView::onDataChanged(const QModelIndex& topLeft,
                               const QModelIndex& bottomRight,
                               const QVector<int>& roles) {
  Q_UNUSED(bottomRight)
  // 当选择状态变化时更新显示
  //qDebug() << "onDataChanged";
  // if (roles.contains(TtChatMessageModel::SelectionStartRole) ||
  //     roles.contains(TtChatMessageModel::SelectionEndRole)) {
  //     // viewport()->update(visualRect(topLeft));
  // }
}


#endif
}  // namespace Ui  
