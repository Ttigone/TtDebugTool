#include "ui/control/ChatWidget/TtChatMessageModel.h"

#include <QDateTime>
#include <QSet>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrent>

namespace Ui {

TtChatMessageModel::TtChatMessageModel(QObject* parent)
    : QAbstractListModel(parent) {
  initializeBlocks();
}

TtChatMessageModel::~TtChatMessageModel() {
  qDeleteAll(m_messageMap);
}

// 核心模型接口
int TtChatMessageModel::rowCount(const QModelIndex& parent) const {
  return parent.isValid()
             ? 0
             : m_blocks.last().baseIndex + m_blocks.last().messages.size();
}

QVariant TtChatMessageModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  const int row = index.row();
  for (const auto& block : m_blocks) {
    if (row >= block.baseIndex &&
        row < block.baseIndex + block.messages.size()) {
      TtChatMessage* msg = block.messages.at(row - block.baseIndex);
      switch (role) {
        case MessageObjectRole:
          return QVariant::fromValue(
              qobject_cast<QObject*>(msg));  // 返回对象指针
        case MessageIdRole:
          return msg->messageId();
        case ContentRole:
          return msg->content();
        case TimestampRole:
          return msg->timestamp();
        case IsOutgoingRole:
          return msg->isOutgoing();
        case BubbleColorRole:
          return msg->bubbleColor();
        case TextColorRole:
          return msg->textColor();
        case FontRole:
          return msg->font();
        case SelectionStartRole:
          return msg->selectionStart();
        case SelectionEndRole:
          return msg->selectionEnd();
        case MessageTypeRole:
          return msg->type();
        case MessageStatusRole:
          return msg->status();
      }
      break;
    }
  }
  return QVariant();
}

QHash<int, QByteArray> TtChatMessageModel::roleNames() const {
  return {{MessageIdRole, "messageId"},
          {ContentRole, "content"},
          {TimestampRole, "timestamp"},
          {IsOutgoingRole, "isOutgoing"},
          {BubbleColorRole, "bubbleColor"},
          {TextColorRole, "textColor"},
          {FontRole, "font"},
          {SelectionStartRole, "selectionStart"},
          {SelectionEndRole, "selectionEnd"},
          {MessageTypeRole, "messageType"},
          {MessageStatusRole, "messageStatus"}};
}

// 数据操作
void TtChatMessageModel::appendMessages(const QList<TtChatMessage*>& messages) {
  if (messages.isEmpty())
    return;

  beginInsertRows(QModelIndex(), rowCount(), rowCount() + messages.size() - 1);
  for (auto msg : messages) {
    m_messageMap.insert(msg->messageId(), msg);
  }
  m_blocks.last().messages.append(messages);
  endInsertRows();

  // 添加这行来触发布局更新
  // triggerLayoutUpdateForAll();

  trimToMaxCapacity();
}

void TtChatMessageModel::prependMessages(
    const QList<TtChatMessage*>& messages) {
  // if (messages.isEmpty())
  //     return;

  // beginInsertRows(QModelIndex(), 0, messages.size() - 1);
  // for (auto msg : messages) {
  //     m_messageMap.insert(msg->messageId(), msg);
  // }
  // m_blocks.first().messages = messages + m_blocks.first().messages;
  // updateBlockIndices();
  // endInsertRows();
  // trimToMaxCapacity();
}

void TtChatMessageModel::removeMessages(int first, int last) {
  // 移出消息, 当前没有调用 message 的析构
  if (first > last || first < 0 || last >= rowCount())
    return;

  beginRemoveRows(QModelIndex(), first, last);
  for (int i = last; i >= first; --i) {
    QModelIndex idx = index(i);                        // 当前索引
    QString id = data(idx, MessageIdRole).toString();  // 当前消息ID
    TtChatMessage* msg = m_messageMap.take(id);        // 移除消息
    m_blocks.last().messages.removeAll(msg);           // 移除消息
    if (msg) {
      msg->deleteLater();
    }
  }
  endRemoveRows();
}

// 分页加载
bool TtChatMessageModel::canFetchMore(const QModelIndex& parent) const {
  // return !parent.isValid() && (m_hasOlder || m_hasNewer);
  return true;
}

void TtChatMessageModel::fetchMore(const QModelIndex& parent) {
  // qDebug() << "test";
  // if (parent.isValid())
  //   return;

  // QThreadPool::globalInstance()->start([this] {
  //   auto older = QtConcurrent::run([this] {
  //     QList<TtChatMessage *> messages;
  //     // 从数据库或网络加载更旧的消息...
  //     return messages;
  //   });
  //   older.then(this, [this](QList<TtChatMessage *> messages) {
  //     prependMessages(messages);
  //     m_hasOlder = !messages.isEmpty();
  //   });
  // });
}

void TtChatMessageModel::setSelection(const QModelIndex& index, int start,
                                      int end) {
  if (!index.isValid())
    return;

  TtChatMessage* msg = qobject_cast<TtChatMessage*>(
      index.data(MessageObjectRole).value<QObject*>());
  if (!msg)
    return;

  // 设置当前消息的选择范围
  msg->setSelection(start, end);

  // 标记为临时选中状态
  m_tempSelections.insert(msg);

  // emit dataChanged(index, index, {SelectionStartRole, SelectionEndRole});
}

void TtChatMessageModel::clearAllSelections() {
  for (auto msg : m_finalSelections) {
    QModelIndex idx = indexForMessage(msg->messageId());
    if (idx.isValid()) {
      // 清除选择
      msg->clearSelection();
      emit dataChanged(idx, idx, {SelectionStartRole, SelectionEndRole});
    }
  }
}

void TtChatMessageModel::clearMessageSelection(const QModelIndex& index) {
  if (!index.isValid()) {
    return;
  }

  TtChatMessage* msg = qobject_cast<TtChatMessage*>(
      index.data(TtChatMessageModel::MessageObjectRole).value<QObject*>());
  if (msg) {
    msg->clearSelection();
    // 移除
    m_selectedMessages.remove(msg);

    QVector<int> roles{SelectionStartRole, SelectionEndRole};
    // emit dataChanged(index, index, roles);
  }
}

QString TtChatMessageModel::selectedText() const {
  QStringList parts;
  for (auto msg : m_finalSelections) {
    parts << msg->selectedText();
  }
  qDebug() << "SelectText: " << parts;
  return parts.join("\n");
}

void TtChatMessageModel::commitSelections() {
  // 清除旧选中状态
  for (auto msg : m_finalSelections) {
    if (!m_tempSelections.contains(msg)) {
      msg->clearSelection();
      QModelIndex idx = indexForMessage(msg->messageId());
      // emit dataChanged(idx, idx, {SelectionStartRole, SelectionEndRole});
    }
  }

  // 更新最终集合
  m_finalSelections = m_tempSelections;
  m_tempSelections.clear();
}

// 私有方法
void TtChatMessageModel::initializeBlocks() {
  m_blocks.clear();                                     // 清除历史消息快
  m_blocks.append({QList<TtChatMessage*>(), 0, true});  // 当前块
  m_blocks.append({QList<TtChatMessage*>(), 0, false});  // 历史块占位
  m_blocks.append({QList<TtChatMessage*>(), 0, false});  // 新消息块占位
}

QModelIndex TtChatMessageModel::indexForMessage(const QString& id) const {
  for (int i = 0; i < rowCount(); ++i) {
    QModelIndex idx = index(i, 0);
    if (!idx.isValid()) {
      continue;
    }
    if (idx.data(MessageIdRole).toString() == id) {
      return idx;
    }
  }
  // qDebug()
  return QModelIndex();
}

void TtChatMessageModel::updateBlockIndices() {
  // int base = 0;
  // for (auto &block : m_blocks) {
  //     block.baseIndex = base;
  //     base += block.messages.size();
  // }
}
void TtChatMessageModel::clearModelData() {
  ////clear
  //if (m_messageMap.empty()) {
  //  return;
  //}

  //beginResetModel();
  ////for (auto& block : m_blocks) {
  ////  //delete block.messages.removeAll();
  ////  block.messages.removeAll();
  ////}
  ////m_blocks.clear();
  ////m_messageMap.clear();
  //endResetModel();
  beginResetModel();
  clearAllSelections();
  removeMessages(0, rowCount() - 1);
  endResetModel();
  //if (m_messageMap.empty()) {
  //  return;
  //}
  //beginResetModel();  // 通知视图模型即将被重置
  //                    //for (auto& block : m_blocks) {
  //                    //  delete block.messages;  // 删除消息对象
  //// 删除消息对象
  //for (auto& block : m_blocks) {
  //  for (auto msg : block.messages) {
  //    delete msg;
  //  }
  //  block.messages.clear();
  //}
  ////}
  //m_blocks.clear();      // 清空消息块
  //m_messageMap.clear();  // 清空消息映射
  //endResetModel();       // 通知视图模型已经被重置
}

void TtChatMessageModel::loadInitialMessages() {}

void TtChatMessageModel::fetchOlderMessages() {}

void TtChatMessageModel::fetchNewerMessages() {}

void TtChatMessageModel::trimToMaxCapacity() {
  // while (rowCount() > m_maxCapacity) {
  //     int removeCount = rowCount() - m_maxCapacity;
  //     removeMessages(rowCount() - removeCount, rowCount() - 1);
  // }
}

void TtChatMessageModel::triggerLayoutUpdate(const QModelIndex& index) {
  if (index.isValid()) {
    // emit dataChanged(index, index, {Qt::SizeHintRole});
  }
}

void TtChatMessageModel::triggerLayoutUpdateForAll() {
  if (!m_blocks.isEmpty()) {
    emit dataChanged(index(0), index(rowCount() - 1), {Qt::SizeHintRole});
  }
}
}  // namespace Ui
