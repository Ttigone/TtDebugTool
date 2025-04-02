#include "TtSerialLexer.h"

SerialHighlighter::SerialHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
  // 初始化文本格式
  m_timestampFormat.setForeground(QColor(0x88, 0x88, 0x88));  // #888888
  m_sendArrowFormat.setForeground(Qt::green);
  m_recvArrowFormat.setForeground(Qt::red);
  m_sendMsgFormat.setForeground(QColor(0x00, 0x7A, 0xCC));  // #007ACC
  m_recvMsgFormat.setForeground(QColor(0x80, 0x00, 0xFF));  // #8000FF

  // 规则定义
  HighlightRule rule;

  // 规则1：时间戳 [yyyy-MM-dd hh:mm:ss.zzz]
  rule.pattern =
      QRegularExpression(R"(\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}\])");
  rule.format = m_timestampFormat;
  m_rules.append(rule);

  // 规则2：发送箭头 <<
  rule.pattern = QRegularExpression(R"(<<)");
  rule.format = m_sendArrowFormat;
  m_rules.append(rule);

  // 规则3：接收箭头 >>
  rule.pattern = QRegularExpression(R"(>>)");
  rule.format = m_recvArrowFormat;
  m_rules.append(rule);

  // 多行消息处理规则
  rule.pattern = QRegularExpression(R"(^[^\[\]]+$)");
  rule.format = m_sendMsgFormat;
  rule.state = SendMessageState;
  m_rules.append(rule);
}

void SerialHighlighter::highlightBlock(const QString& text) {
  // 处理多行状态
  int previousState = previousBlockState();

  // 处理延续消息
  if (previousState == SendMessageState || previousState == RecvMessageState) {
    QTextCharFormat format =
        (previousState == SendMessageState) ? m_sendMsgFormat : m_recvMsgFormat;
    setFormat(0, text.length(), format);
    setCurrentBlockState(previousState);
    return;
  }

  // 应用普通规则
  foreach (const HighlightRule& rule, m_rules) {
    QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
    while (it.hasNext()) {
      QRegularExpressionMatch match = it.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.format);

      if (rule.state != -1) {
        setCurrentBlockState(rule.state);
      }
    }
  }

  // 特殊处理消息行（必须最后处理）
  QRegularExpression messageStart(R"(^\s*<<|>>)");
  if (!messageStart.match(text).hasMatch() &&
      previousBlockState() != SendMessageState &&
      previousBlockState() != RecvMessageState) {
    setCurrentBlockState(NoneState);
  }
}
