#include "ui/control/ChatWidget/TtChatMessage.h"

#include <QUuid>

namespace Ui {

TtChatMessage::TtChatMessage(QObject* parent)
    : QObject(parent), m_id(QUuid::createUuid().toString()) {
  // 初始化默认样式
  m_bubbleColor = QColor("#DCF8C6");  // 默认浅绿色气泡
  m_textColor = Qt::black;
  m_font.setFamily("Arial");
  m_font.setPointSize(10);
}

TtChatMessage::TtChatMessage(const QString& id, QObject* parent)
    : QObject(parent), m_id(id) {
  // 同上初始化
}

TtChatMessage::~TtChatMessage() {
  // 清理资源（如图片缓存等）
}

// 基础属性实现
QString TtChatMessage::content() const {
  return m_content;
}
QDateTime TtChatMessage::timestamp() const {
  return m_timestamp;
}
bool TtChatMessage::isOutgoing() const {
  return m_outgoing;
  // return true;
}

TtChatMessage::MessageType TtChatMessage::type() const {
  return m_type;
}
QString TtChatMessage::messageId() const {
  return m_id;
}

// 选择状态管理
int TtChatMessage::selectionStart() const {
  return m_selectionStart;
}
int TtChatMessage::selectionEnd() const {
  return m_selectionEnd;
}

QString TtChatMessage::selectedText() const {
  if (m_selectionStart < 0 || m_selectionEnd < 0 ||
      m_selectionStart == m_selectionEnd) {
    // 没有选中的文字
    return QString();
  }

  int start = qMin(m_selectionStart, m_selectionEnd);
  int end = qMax(m_selectionStart, m_selectionEnd);
  return m_content.mid(start, end - start);
}

void TtChatMessage::setSelectionStart(int pos) {
  if (pos != m_selectionStart) {
    m_selectionStart = pos;
    emit selectionChanged();
  }
}

void TtChatMessage::setSelectionEnd(int pos) {
  if (pos != m_selectionEnd) {
    m_selectionEnd = pos;
    emit selectionChanged();
  }
}

void TtChatMessage::setSelection(int start, int end) {
  m_selectionStart = start;
  m_selectionEnd = end;
}

void TtChatMessage::clearSelection() {
  //qDebug() << "message clear select";
  setSelectionStart(-1);
  setSelectionEnd(-1);
}

// 样式管理
QColor TtChatMessage::bubbleColor() const {
  return m_bubbleColor;
}
QColor TtChatMessage::textColor() const {
  return m_textColor;
}
QFont TtChatMessage::font() const {
  return m_font;
}

void TtChatMessage::setBubbleColor(const QColor& color) {
  if (m_bubbleColor != color) {
    m_bubbleColor = color;
    emit bubbleStyleChanged();
  }
}

void TtChatMessage::setTextColor(const QColor& color) {
  if (m_textColor != color) {
    m_textColor = color;
    emit bubbleStyleChanged();
  }
}

void TtChatMessage::setFont(const QFont& font) {
  if (m_font != font) {
    m_font = font;
    emit fontChanged();
  }
}

// 消息状态
TtChatMessage::MessageStatus TtChatMessage::status() const {
  return m_status;
}

void TtChatMessage::setStatus(MessageStatus status) {
  if (m_status != status) {
    m_status = status;
    emit statusChanged();
  }
}

// 数据序列化
QVariantMap TtChatMessage::toVariantMap() const {
  return {{"id", m_id},
          {"content", m_content},
          {"timestamp", m_timestamp},
          {"outgoing", m_outgoing},
          {"type", m_type},
          {"bubbleColor", m_bubbleColor.name()},
          {"textColor", m_textColor.name()},
          {"font", m_font.toString()},
          {"status", m_status}};
}

void TtChatMessage::fromVariantMap(const QVariantMap& data) {
  m_id = data.value("id").toString();
  m_content = data.value("content").toString();
  m_timestamp = data.value("timestamp").toDateTime();
  m_outgoing = data.value("outgoing").toBool();
  m_type = static_cast<MessageType>(data.value("type").toInt());
  m_bubbleColor = QColor(data.value("bubbleColor").toString());
  m_textColor = QColor(data.value("textColor").toString());
  m_font.fromString(data.value("font").toString());
  m_status = static_cast<MessageStatus>(data.value("status").toInt());
}

// 其他属性设置器
void TtChatMessage::setContent(const QString& content) {
  if (m_content != content) {
    m_content = content;
    emit contentChanged();
  }
}

void TtChatMessage::setTimestamp(const QDateTime& timestamp) {
  if (m_timestamp != timestamp) {
    m_timestamp = timestamp;
    emit timestampChanged();
  }
}

void TtChatMessage::setOutgoing(bool outgoing) {
  if (m_outgoing != outgoing) {
    m_outgoing = outgoing;
    emit outgoingChanged();
  }
}
}  // namespace Ui
