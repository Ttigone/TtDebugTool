#include "TtSerialLexer.h"

// SerialHighlighter::SerialHighlighter(QTextDocument* parent)
//     : QSyntaxHighlighter(parent) {

//   // m_messageStartPattern =
//   //     QRegularExpression("\\[(Tx|Rx)\\]\\s*\\[\\d{4}-\\d{2}-\\d{2}");
//   m_messageStartPattern =
//       QRegularExpression("^\\[(?:Tx|Rx)\\]\\s*\\[\\d{4}-\\d{2}-\\d{2}");

//   // 初始化正则表达式
//   m_txTagPattern = QRegularExpression("\\[Tx\\]");
//   m_rxTagPattern = QRegularExpression("\\[Rx\\]");
//   m_timeStampPattern = QRegularExpression(
//       "\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}\\]");
//   m_hexDataPattern = QRegularExpression("^([0-9A-Fa-f]{2}\\s)+[0-9A-Fa-f]{2}$");

//   // 初始化格式
//   m_txFormat.setForeground(QColor("#25C2A0"));  // 发送标签颜色 - 绿色
//   m_txFormat.setFontWeight(QFont::Bold);

//   m_rxFormat.setForeground(QColor("#0ea5e9"));  // 接收标签颜色 - 蓝色
//   m_rxFormat.setFontWeight(QFont::Bold);

//   m_timestampFormat.setForeground(QColor(128, 128, 0));  // 时间戳颜色 - 暗黄色

//   m_hexDataFormat.setForeground(Qt::darkMagenta);  // 十六进制数据颜色

//   m_txTextFormat.setForeground(Qt::darkBlue);   // 发送文本数据颜色
//   m_rxTextFormat.setForeground(Qt::darkGreen);  // 接收文本数据颜色

//   // 确保初始块状态为None
//   setCurrentBlockState(NoneState);
// }

// // void SerialHighlighter::highlightBlock(const QString& text) {
// //   // 默认格式
// //   setFormat(0, text.length(), Qt::black);

// //   // 自定义的 int 值
// //   // 获取前一个块的状态
// //   int previousState = previousBlockState();

// //   if (isCompleteMessage(text)) {
// //     previousState = NoneState;
// //   }

// //   // 检查是否在多行数据模式中（上一行的延续）
// //   if (previousState == SendMessageState || previousState == RecvMessageState) {
// //     // 这一行是之前消息的继续，格式应保持一致
// //     QTextCharFormat format =
// //         (previousState == SendMessageState) ? m_txTextFormat : m_rxTextFormat;

// //     // 应用与数据类型相匹配的格式
// //     setFormat(0, text.length(), format);

// //     // setCurrentBlockState(previousState);
// //     if (containsNewlineSequence(text)) {
// //       // 保持相同的状态
// //       setCurrentBlockState(previousState);
// //     } else {
// //       // 重置状态
// //       setCurrentBlockState(NoneState);
// //     }
// //     return;
// //   }

// //   if (previousState == SendMessageState || previousState == RecvMessageState) {
// //     setCurrentBlockState(NoneState);
// //     if (!isCompleteMessage(text)) {
// //       // 延续
// //       return;
// //     }
// //   }

// //   // 检查是否为新消息
// //   bool isTxMessage = false;
// //   int tagStart = -1;
// //   int tagLength = 0;

// //   // 查找 [Tx] 标签
// //   QRegularExpressionMatch txMatch = m_txTagPattern.match(text);
// //   if (txMatch.hasMatch()) {
// //     tagStart = txMatch.capturedStart();
// //     tagLength = txMatch.capturedLength();
// //     isTxMessage = true;
// //   } else {
// //     // 查找 [Rx] 标签
// //     QRegularExpressionMatch rxMatch = m_rxTagPattern.match(text);
// //     if (rxMatch.hasMatch()) {
// //       tagStart = rxMatch.capturedStart();
// //       tagLength = rxMatch.capturedLength();
// //       isTxMessage = false;
// //     }
// //   }

// //   // 如果找到了标签，就处理这一行
// //   if (tagStart != -1) {
// //     // 应用标签格式
// //     setFormat(tagStart, tagLength, isTxMessage ? m_txFormat : m_rxFormat);

// //     // 查找时间戳
// //     QRegularExpressionMatch timeMatch =
// //         m_timeStampPattern.match(text, tagStart + tagLength);
// //     if (timeMatch.hasMatch()) {
// //       int timeStart = timeMatch.capturedStart();
// //       int timeLength = timeMatch.capturedLength();

// //       // 应用时间戳格式
// //       setFormat(timeStart, timeLength, m_timestampFormat);

// //       // 获取数据部分
// //       int dataStart = timeStart + timeLength + 1;  // 空白符

// //       if (dataStart < text.length()) {
// //         QString dataContent = text.mid(dataStart).trimmed();

// //         // 检查是否为十六进制数据格式
// //         QRegularExpressionMatch hexMatch = m_hexDataPattern.match(dataContent);
// //         bool isHex = hexMatch.hasMatch();

// //         // 根据消息类型和数据格式应用格式
// //         if (isHex) {
// //           setFormat(dataStart, text.length() - dataStart, m_hexDataFormat);
// //         } else {
// //           setFormat(dataStart, text.length() - dataStart,
// //                     isTxMessage ? m_txTextFormat : m_rxTextFormat);
// //         }

// //         // // 设置块状态，用于跨行消息
// //         // setCurrentBlockState(isTxMessage ? SendMessageState : RecvMessageState);

// //         // // 检查文本中是否包含换行符，如有则下一行将是相同状态
// //         // if (dataContent.contains("\r\n") || dataContent.contains("\\r\\n") ||
// //         //     dataContent.contains("\n") || dataContent.contains("\\n")) {
// //         //   // 确保设置状态，让下一行继承当前格式
// //         //   setCurrentBlockState(isTxMessage ? SendMessageState
// //         //                                    : RecvMessageState);
// //         // }
// //         if (containsNewlineSequence(dataContent)) {
// //           setCurrentBlockState(isTxMessage ? SendMessageState
// //                                            : RecvMessageState);
// //         } else {
// //           setCurrentBlockState(NoneState);
// //         }
// //       }
// //     }
// //     // 处理完第一条标签, 直接返回
// //     return;
// //   } else {
// //     // 普通行，无标签 - 检查是否为前一行的延续
// //     if (previousState != NoneState) {
// //       QTextCharFormat format =
// //           (previousState == SendMessageState) ? m_txTextFormat : m_rxTextFormat;
// //       setFormat(0, text.length(), format);

// //       // 有换行符号, 保持
// //       if (containsNewlineSequence(text)) {
// //         setCurrentBlockState(previousState);
// //       } else {
// //         setCurrentBlockState(NoneState);
// //       }

// //     } else {
// //       // 完全普通的行
// //       setCurrentBlockState(NoneState);
// //     }
// //   }
// // }

// void SerialHighlighter::highlightBlock(const QString& text) {
//   // 默认格式
//   setFormat(0, text.length(), Qt::black);

//   // 自定义的 int 值
//   // 获取前一个块的状态
//   int previousState = previousBlockState();

//   if (isCompleteMessage(text)) {
//     previousState = NoneState;
//   }

//   // 检查是否在多行数据模式中（上一行的延续）
//   if (previousState == SendMessageState || previousState == RecvMessageState) {
//     // 这一行是之前消息的继续，格式应保持一致
//     QTextCharFormat format =
//         (previousState == SendMessageState) ? m_txTextFormat : m_rxTextFormat;

//     // 应用与数据类型相匹配的格式
//     setFormat(0, text.length(), format);

//     // setCurrentBlockState(previousState);
//     if (containsNewlineSequence(text)) {
//       // 保持相同的状态
//       setCurrentBlockState(previousState);
//     } else {
//       // 重置状态
//       setCurrentBlockState(NoneState);
//     }
//     return;
//   }

//   if (previousState == SendMessageState || previousState == RecvMessageState) {
//     setCurrentBlockState(NoneState);
//     if (!isCompleteMessage(text)) {
//       // 延续
//       return;
//     }
//   }

//   // 检查是否为新消息
//   bool isTxMessage = false;
//   int tagStart = -1;
//   int tagLength = 0;

//   // 查找 [Tx] 标签
//   QRegularExpressionMatch txMatch = m_txTagPattern.match(text);
//   if (txMatch.hasMatch()) {
//     tagStart = txMatch.capturedStart();
//     tagLength = txMatch.capturedLength();
//     isTxMessage = true;
//   } else {
//     // 查找 [Rx] 标签
//     QRegularExpressionMatch rxMatch = m_rxTagPattern.match(text);
//     if (rxMatch.hasMatch()) {
//       tagStart = rxMatch.capturedStart();
//       tagLength = rxMatch.capturedLength();
//       isTxMessage = false;
//     }
//   }

//   // 如果找到了标签，就处理这一行
//   if (tagStart != -1) {
//     // 应用标签格式
//     setFormat(tagStart, tagLength, isTxMessage ? m_txFormat : m_rxFormat);

//     // 查找时间戳
//     QRegularExpressionMatch timeMatch =
//         m_timeStampPattern.match(text, tagStart + tagLength);
//     if (timeMatch.hasMatch()) {
//       int timeStart = timeMatch.capturedStart();
//       int timeLength = timeMatch.capturedLength();

//       // 应用时间戳格式
//       setFormat(timeStart, timeLength, m_timestampFormat);

//       // 获取数据部分
//       int dataStart = timeStart + timeLength + 1;  // 空白符
//       QString dataContent = text.mid(dataStart).trimmed();
//       bool isHex = m_hexDataPattern.match(dataContent).hasMatch();
//       setFormat(dataStart, text.length() - dataStart,
//                 isTxMessage ? m_txTextFormat : m_rxTextFormat);

//       if (containsNewlineSequence(dataContent)) {
//         setCurrentBlockState(isTxMessage ? SendMessageState : RecvMessageState);
//       } else {
//         setCurrentBlockState(NoneState);
//       }
//       // 处理完第一条标签, 直接返回
//       return;
//     }
//   } else {
//     // 普通行，无标签 - 检查是否为前一行的延续
//     if (previousState != NoneState) {
//       QTextCharFormat format =
//           (previousState == SendMessageState) ? m_txTextFormat : m_rxTextFormat;
//       setFormat(0, text.length(), format);

//       // 有换行符号, 保持
//       if (containsNewlineSequence(text)) {
//         setCurrentBlockState(previousState);
//       } else {
//         setCurrentBlockState(NoneState);
//       }

//     } else {
//       // 完全普通的行
//       setCurrentBlockState(NoneState);
//     }
//   }
// }

// bool SerialHighlighter::isCompleteMessage(const QString& text) const {
//   // 检查是否是一个完整的消息：如果包含新消息标记
//   return m_messageStartPattern.match(text).hasMatch();
// }

// bool SerialHighlighter::containsNewlineSequence(const QString& text) const {
//   // return text.contains("\r\n") || text.contains("\\r\\n") ||
//   //        text.contains("\n") || text.contains("\\n");
//   return text.contains(QLatin1Char('\r')) || text.contains(QLatin1Char('\n'));
// }

SerialHighlighter::SerialHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
  // 只有在行首同时出现 [Tx]/[Rx] 和后面的时间戳，才视为“新消息”
  m_messageStartPattern = QRegularExpression(
      R"(^\[(?:Tx|Rx)\]\s*\[\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d{3}\])");

  m_txTagPattern = QRegularExpression(R"(\[Tx\])");
  m_rxTagPattern = QRegularExpression(R"(\[Rx\])");
  m_timeStampPattern =
      QRegularExpression(R"(\[\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d{3}\])");
  m_hexDataPattern =
      QRegularExpression(R"(^([0-9A-Fa-f]{2}\s+)*[0-9A-Fa-f]{2}$)");

  // 标签颜色
  m_txFormat.setForeground(QColor("#25C2A0"));
  m_txFormat.setFontWeight(QFont::Bold);
  m_rxFormat.setForeground(QColor("#0ea5e9"));
  m_rxFormat.setFontWeight(QFont::Bold);

  // 时间戳
  m_timestampFormat.setForeground(QColor(128, 128, 0));

  // 数据
  m_hexDataFormat.setForeground(Qt::darkMagenta);
  m_txTextFormat.setForeground(Qt::darkBlue);
  m_rxTextFormat.setForeground(Qt::darkGreen);

  setCurrentBlockState(NoneState);
}

void SerialHighlighter::highlightBlock(const QString& text) {
  // 默认黑色
  setFormat(0, text.length(), Qt::black);

  // 先看上一块是什么状态
  int prevState = previousBlockState();

  // 如果上一块在“发送/接收消息中”，且本行又不是一个新消息起始，就当作延续行
  if ((prevState == SendMessageState || prevState == RecvMessageState) &&
      !m_messageStartPattern.match(text).hasMatch()) {
    auto fmt =
        (prevState == SendMessageState) ? m_txTextFormat : m_rxTextFormat;
    setFormat(0, text.length(), fmt);
    setCurrentBlockState(prevState);
    return;
  }

  // 不做延续：检查本行是否是一个新消息起始
  QRegularExpressionMatch startMatch = m_messageStartPattern.match(text);
  if (!startMatch.hasMatch()) {
    // 既不是延续，也不是新消息：恢复默认状态
    setCurrentBlockState(NoneState);
    return;
  }

  // —— 到这里，本行是新消息的第一行 —— //

  // 1) 先把 [Tx] 或 [Rx] 标签上色
  bool isTx = false;
  int tagStart = -1, tagLen = 0;
  if (auto m = m_txTagPattern.match(text); m.hasMatch()) {
    isTx = true;
    tagStart = m.capturedStart();
    tagLen = m.capturedLength();
  } else if (auto m = m_rxTagPattern.match(text); m.hasMatch()) {
    tagStart = m.capturedStart();
    tagLen = m.capturedLength();
  }
  if (tagStart >= 0) {
    setFormat(tagStart, tagLen, isTx ? m_txFormat : m_rxFormat);
  }

  // 2) 找时间戳并上色
  QRegularExpressionMatch t = m_timeStampPattern.match(text, tagStart + tagLen);
  int tsStart = -1, tsLen = 0;
  if (t.hasMatch()) {
    tsStart = t.capturedStart();
    tsLen = t.capturedLength();
    setFormat(tsStart, tsLen, m_timestampFormat);
  }

  // 3) 数据部分：从时间戳后第一个空格开始一直到行尾
  int dataStart = (tsStart >= 0 ? tsStart + tsLen : tagStart + tagLen);
  // 跳过一个或多个空格
  while (dataStart < text.length() && text[dataStart].isSpace())
    ++dataStart;
  if (dataStart < text.length()) {
    QString payload = text.mid(dataStart);
    bool isHex = m_hexDataPattern.match(payload).hasMatch();
    setFormat(
        dataStart, payload.length(),
        isHex ? m_hexDataFormat : (isTx ? m_txTextFormat : m_rxTextFormat));
  }

  // 4) 新消息第一行永远重新开始状态
  setCurrentBlockState(isTx ? SendMessageState : RecvMessageState);
}
