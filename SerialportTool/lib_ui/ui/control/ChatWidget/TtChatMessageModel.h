#ifndef UI_CONTROL_TTCHATMESSAGEMODEL_H
#define UI_CONTROL_TTCHATMESSAGEMODEL_H

#include <QAbstractListModel>
#include "ui/control/ChatWidget/TtChatMessage.h"

namespace Ui {

#if 0
class TtChatMessageModel : public QAbstractListModel {
  Q_OBJECT

 public:
  enum CustomRoles {
    MessageObjectRole = Qt::UserRole + 1,  // 新增对象角色
    MessageIdRole,
    ContentRole,
    TimestampRole,
    IsOutgoingRole,
    BubbleColorRole,
    TextColorRole,
    FontRole,
    SelectionStartRole,
    SelectionEndRole,
    MessageTypeRole,
    MessageStatusRole,
    TextLengthRole
  };

  explicit TtChatMessageModel(QObject* parent = nullptr);
  ~TtChatMessageModel();

  // QAbstractItemModel interface
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;
  bool canFetchMore(const QModelIndex& parent) const override;
  void fetchMore(const QModelIndex& parent) override;

  // 高性能数据操作接口
  void appendMessages(const QList<TtChatMessage*>& messages);
  void prependMessages(const QList<TtChatMessage*>& messages);
  void insertMessages(int pos, const QList<TtChatMessage*>& messages);
  void removeMessages(int first, int last);
  void updateMessage(TtChatMessage* message);

  // 选择状态管理
  void setSelection(const QModelIndex& index, int start, int end);
  void clearAllSelections();
  void clearMessageSelection(const QModelIndex& index);
  QString selectedText() const;
  void commitSelections();

  // 分页控制
  void setPageSize(int size);
  bool hasMore() const;

  void updateBlockIndices();

  void beginUpdateSelections() { m_tempSelections.clear(); }

  void endUpdateSelections() {}

 public slots:
  void loadInitialMessages();
  void fetchOlderMessages();
  void fetchNewerMessages();

 signals:
  void needFetchOlder();
  void needFetchNewer();
  void selectionChanged();

 private:
  struct MessageBlock {
    QList<TtChatMessage*> messages;
    int baseIndex = 0;
    bool loaded = false;
  };

  void initializeBlocks();
  QModelIndex indexForMessage(const QString& id) const;
  void trimToMaxCapacity();

  QList<MessageBlock> m_blocks;
  QHash<QString, TtChatMessage*> m_messageMap;  // ID到消息的快速查找
  QSet<TtChatMessage*> m_selectedMessages;

  QSet<TtChatMessage*> m_tempSelections;   // 新增：临时选中集合
  QSet<TtChatMessage*> m_finalSelections;  // 最终确认的选中集合
  TtChatMessage* m_currentSelectedMsg = nullptr;
  int m_pageSize = 50;
  int m_maxCapacity = 10000;  // 内存中最大保留消息数
  bool m_hasOlder = true;
  bool m_hasNewer = true;
};
#else

  class TtChatMessageModel : public QAbstractListModel {
  Q_OBJECT

 public:
  enum CustomRoles {
    MessageObjectRole = Qt::UserRole + 1,  // 新增对象角色
    MessageIdRole,
    ContentRole,
    TimestampRole,
    IsOutgoingRole,
    BubbleColorRole,
    TextColorRole,
    FontRole,
    SelectionStartRole,
    SelectionEndRole,
    MessageTypeRole,
    MessageStatusRole,
    TextLengthRole
  };

  explicit TtChatMessageModel(QObject* parent = nullptr);
  ~TtChatMessageModel();

  // QAbstractItemModel interface
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;
  bool canFetchMore(const QModelIndex& parent) const override;
  void fetchMore(const QModelIndex& parent) override;

  // 高性能数据操作接口
  void appendMessages(const QList<TtChatMessage*>& messages);
  void prependMessages(const QList<TtChatMessage*>& messages);
  void insertMessages(int pos, const QList<TtChatMessage*>& messages);
  void removeMessages(int first, int last);
  void updateMessage(TtChatMessage* message);

  // 选择状态管理
  void setSelection(const QModelIndex& index, int start, int end);
  void clearAllSelections();
  void clearMessageSelection(const QModelIndex& index);
  QString selectedText() const;
  void commitSelections();

  // 分页控制
  void setPageSize(int size);
  bool hasMore() const;

  void updateBlockIndices();

  void beginUpdateSelections() { m_tempSelections.clear(); }

  void endUpdateSelections() {}

  // 添加新方法
  void triggerLayoutUpdate(const QModelIndex& index);
  void triggerLayoutUpdateForAll();

 public slots:
  void loadInitialMessages();
  void fetchOlderMessages();
  void fetchNewerMessages();

 signals:
  void needFetchOlder();
  void needFetchNewer();
  void selectionChanged();

 private:
  struct MessageBlock {
    QList<TtChatMessage*> messages;  // 一系列消息
    int baseIndex = 0;               // 基础索引
    bool loaded = false;             // 是否被加载
  };

  void initializeBlocks();
  QModelIndex indexForMessage(const QString& id) const;
  void trimToMaxCapacity();

  QList<MessageBlock> m_blocks;
  QHash<QString, TtChatMessage*> m_messageMap;  // ID到消息的快速查找
  QSet<TtChatMessage*> m_selectedMessages;

  QSet<TtChatMessage*> m_tempSelections;   // 新增：临时选中集合
  QSet<TtChatMessage*> m_finalSelections;  // 最终确认的选中集合
  TtChatMessage* m_currentSelectedMsg = nullptr;
  int m_pageSize = 50;
  int m_maxCapacity = 10000;  // 内存中最大保留消息数
  bool m_hasOlder = true;
  bool m_hasNewer = true;
};
#endif


}  // namespace Ui  

#endif // UI_CONTROL_TTCHATMESSAGEMODEL_H
