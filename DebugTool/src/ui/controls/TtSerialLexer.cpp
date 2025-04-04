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
  // rule.pattern =
  //     QRegularExpression(R"(\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}\])");
  // 规则1：时间戳 [yyyy-MM-dd hh:mm:ss.zzz]
  rule.pattern =
      QRegularExpression(R"(\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}\])");
  rule.format = m_timestampFormat;
  m_rules.append(rule);

  // 规则2：发送箭头行 [时间戳] <<
  HighlightRule arrowRule;
  arrowRule.pattern = QRegularExpression(
      R"(^\s*\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}\]\s*<<\s*$)");
  arrowRule.format = m_sendArrowFormat;
  arrowRule.state = SendMessageState;  // 关键：设置状态
  m_rules.append(arrowRule);

  // 接收箭头行（精确匹配 [时间戳] >>）
  arrowRule.pattern = QRegularExpression(
      R"(^\s*\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}\]\s*>>\s*$)");
  arrowRule.format = m_recvArrowFormat;
  arrowRule.state = RecvMessageState;  // 关键：设置状态
  m_rules.append(arrowRule);

  HighlightRule msgRule;
  msgRule.pattern = QRegularExpression(R"([^\n]+)");  // 匹配非换行内容
  msgRule.format = QTextCharFormat();                 // 临时占位符
  msgRule.state = -1;
  m_rules.append(msgRule);  // 注意：需要调整规则顺序
}

void SerialHighlighter::highlightBlock(const QString& text) {
  // 处理多行状态继承
  const int previousState = previousBlockState();

  // 情况1：当前是消息内容行
  if (previousState == SendMessageState || previousState == RecvMessageState) {
    // 根据前一行状态设置颜色
    const QTextCharFormat format =
        (previousState == SendMessageState) ? m_sendMsgFormat : m_recvMsgFormat;
    setFormat(0, text.length(), format);
    setCurrentBlockState(previousState);  // 保持状态延续
    return;
  }

  // 情况2：处理新消息头（箭头行）
  foreach (const HighlightRule& rule, m_rules) {
    QRegularExpressionMatch match = rule.pattern.match(text);
    if (match.hasMatch()) {
      // 匹配到箭头行时设置格式和状态
      setFormat(0, text.length(), rule.format);
      setCurrentBlockState(rule.state);
      return;
    }
  }

  // 情况3：普通文本行（如空行）
  setCurrentBlockState(NoneState);

  if (previousBlockState() == SendMessageState) {  // 修改为previousBlockState()
    setFormat(0, text.length(), m_sendMsgFormat);
    setCurrentBlockState(SendMessageState);
  } else if (previousBlockState() ==
             RecvMessageState) {  // 修改为previousBlockState()
    setFormat(0, text.length(), m_recvMsgFormat);
    setCurrentBlockState(RecvMessageState);
  }
}
