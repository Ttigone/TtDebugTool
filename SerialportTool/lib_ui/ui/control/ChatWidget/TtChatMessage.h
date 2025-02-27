#ifndef UI_CONTROL_CHATWIDGET_TTCHATMESSAGE_H
#define UI_CONTROL_CHATWIDGET_TTCHATMESSAGE_H

#include <QColor>
#include <QDateTime>
#include <QFont>
#include <QObject>

#include "ui/Def.h"

namespace Ui {
class Tt_EXPORT TtChatMessage : public QObject {
  Q_OBJECT

  // 基础属性
  Q_PROPERTY(
      QString content READ content WRITE setContent NOTIFY contentChanged)
  Q_PROPERTY(QDateTime timestamp READ timestamp WRITE setTimestamp NOTIFY
                 timestampChanged)
  Q_PROPERTY(
      bool outgoing READ isOutgoing WRITE setOutgoing NOTIFY outgoingChanged)
  Q_PROPERTY(QString messageId READ messageId CONSTANT)  // 唯一消息ID

  // 选择状态
  Q_PROPERTY(int selectionStart READ selectionStart WRITE setSelectionStart
                 NOTIFY selectionChanged)
  Q_PROPERTY(int selectionEnd READ selectionEnd WRITE setSelectionEnd NOTIFY
                 selectionChanged)

  // 样式属性
  Q_PROPERTY(QColor bubbleColor READ bubbleColor WRITE setBubbleColor NOTIFY
                 bubbleStyleChanged)
  Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY
                 bubbleStyleChanged)
  Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)

  // 消息状态
  Q_PROPERTY(
      MessageStatus status READ status WRITE setStatus NOTIFY statusChanged)

 public:
  enum MessageType { TextMessage, ImageMessage, FileMessage, SystemMessage };
  Q_ENUM(MessageType)

  enum MessageStatus { Sending, Sent, Delivered, Read, Failed };
  Q_ENUM(MessageStatus)

  explicit TtChatMessage(QObject* parent = nullptr);
  explicit TtChatMessage(const QString& id, QObject* parent = nullptr);
  ~TtChatMessage();

  // 基础属性访问器
  QString content() const;
  QDateTime timestamp() const;
  bool isOutgoing() const;
  MessageType type() const;
  QString messageId() const;

  // 选择状态
  int selectionStart() const;
  int selectionEnd() const;
  QString selectedText() const;

  // 样式访问器
  QColor bubbleColor() const;
  QColor textColor() const;
  QFont font() const;

  // 消息状态
  MessageStatus status() const;

 public Q_SLOTS:
  void setContent(const QString& content);
  void setTimestamp(const QDateTime& timestamp);
  void setOutgoing(bool outgoing);
  void setSelectionStart(int pos);
  void setSelectionEnd(int pos);
  void setSelection(int start, int end);
  void clearSelection();
  void setBubbleColor(const QColor& color);
  void setTextColor(const QColor& color);
  void setFont(const QFont& font);
  void setStatus(MessageStatus status);

  // 序列化/反序列化
  QVariantMap toVariantMap() const;
  void fromVariantMap(const QVariantMap& data);

 Q_SIGNALS:
  void contentChanged();
  void timestampChanged();
  void outgoingChanged();
  void selectionChanged();
  void bubbleStyleChanged();
  void fontChanged();
  void statusChanged();

 private:
  // 唯一标识
  QString m_id;

  // 基础属性
  QString m_content;
  QDateTime m_timestamp;
  bool m_outgoing = false;
  MessageType m_type = TextMessage;

  // 选择状态
  int m_selectionStart = -1;
  int m_selectionEnd = -1;

  // 样式配置
  QColor m_bubbleColor;
  QColor m_textColor;
  QFont m_font;

  // 消息状态
  MessageStatus m_status = Sent;

  // 其他扩展属性...
};

}  // namespace Ui
#endif  // UI_CONTROL_CHATWIDGET_TTCHATMESSAGE_H
