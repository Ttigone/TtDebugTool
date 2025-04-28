#include "TtSerialLexer.h"

SerialHighlighter::SerialHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {

  // // 高亮Rx标签
  // QTextCharFormat rxFormat;
  // rxFormat.setForeground(QColor("#0ea5e9"));  // Rx蓝色
  // addRule("\\[Rx\\]", rxFormat);

  // // 高亮Tx标签
  // QTextCharFormat txFormat;
  // txFormat.setForeground(QColor("#25C2A0"));  // Tx绿色
  // addRule("\\[Tx\\]", txFormat);

  // QTextCharFormat timeFormat;
  // timeFormat.setForeground(QColor(128, 128, 0));  // 暗黄色
  // addRule("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}\\]",
  //         timeFormat);

  // // HEX数据 - 第三优先级
  // QTextCharFormat hexFormat;
  // hexFormat.setForeground(Qt::darkMagenta);
  // // 匹配时间戳后的16进制格式数据
  // addRule("^([0-9A-Fa-f]{2}\\s)+[0-9A-Fa-f]{2}$", hexFormat);

  // 初始化正则表达式
  m_txTagPattern = QRegularExpression("\\[Tx\\]");
  m_rxTagPattern = QRegularExpression("\\[Rx\\]");
  m_timeStampPattern = QRegularExpression(
      "\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}\\]");
  m_hexDataPattern = QRegularExpression("^([0-9A-Fa-f]{2}\\s)+[0-9A-Fa-f]{2}$");

  // 初始化格式
  m_txFormat.setForeground(QColor("#25C2A0"));  // 发送标签颜色 - 绿色
  m_txFormat.setFontWeight(QFont::Bold);

  m_rxFormat.setForeground(QColor("#0ea5e9"));  // 接收标签颜色 - 蓝色
  m_rxFormat.setFontWeight(QFont::Bold);

  m_timestampFormat.setForeground(QColor(128, 128, 0));  // 时间戳颜色 - 暗黄色

  m_hexDataFormat.setForeground(Qt::darkMagenta);  // 十六进制数据颜色

  m_txTextFormat.setForeground(Qt::darkBlue);   // 发送文本数据颜色
  m_rxTextFormat.setForeground(Qt::darkGreen);  // 接收文本数据颜色

  // 确保初始块状态为None
  setCurrentBlockState(NoneState);
}

// void SerialHighlighter::highlightBlock(const QString& text) {
//   // 获取先前块的状态
//   int previousState = previousBlockState();

//   // 默认颜色
//   setFormat(0, text.length(), Qt::black);

//   // 检查是否处于延续状态（上一行的继续）
//   if (previousState == SendMessageState || previousState == RecvMessageState) {
//     // 这是消息的后续行（如 \r\n 后的内容）
//     QColor textColor = (previousState == SendMessageState)
//                            ? QColor(Qt::darkBlue)
//                            : QColor(Qt::darkGreen);

//     // 高亮整行为对应的数据颜色
//     setFormat(0, text.length(), textColor);

//     // 保持相同状态
//     setCurrentBlockState(previousState);
//     return;
//   }

//   // 基本的行解析处理
//   QRegularExpression tagPattern("\\[(Tx|Rx)\\]");
//   QRegularExpressionMatch tagMatch = tagPattern.match(text);

//   if (tagMatch.hasMatch()) {
//     // 确定是发送还是接收
//     QString tag = tagMatch.captured(1);
//     bool isTx = (tag == "Tx");

//     // 高亮标签
//     QColor tagColor = isTx ? QColor("#25C2A0") : QColor("#0ea5e9");
//     setFormat(tagMatch.capturedStart(), tagMatch.capturedLength(), tagColor);

//     // 查找时间戳
//     QRegularExpression timePattern(
//         "\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}\\]");
//     QRegularExpressionMatch timeMatch =
//         timePattern.match(text, tagMatch.capturedEnd());

//     if (timeMatch.hasMatch()) {
//       // 高亮时间戳
//       setFormat(timeMatch.capturedStart(), timeMatch.capturedLength(),
//                 QColor(128, 128, 0));

//       // 处理数据部分
//       int dataStart = timeMatch.capturedEnd() + 1;
//       if (dataStart < text.length()) {
//         // 提取数据部分，可能包含 \r\n
//         QString dataContent = text.mid(dataStart);

//         // 检查是否是HEX格式
//         bool isHex = true;
//         QRegularExpression hexSinglePattern("^[0-9A-Fa-f]{2}$");
//         QStringList parts = dataContent.split(" ", Qt::SkipEmptyParts);
//         for (const QString& part : parts) {
//           if (!hexSinglePattern.match(part).hasMatch()) {
//             isHex = false;
//             break;
//           }
//         }

//         // 根据格式应用颜色
//         QColor dataColor =
//             isHex ? Qt::darkMagenta : (isTx ? Qt::darkBlue : Qt::darkGreen);
//         setFormat(dataStart, text.length() - dataStart, dataColor);

//         // 设置块状态 - 标记为发送或接收消息
//         // 这将影响下一行的处理，如果有\r\n
//         setCurrentBlockState(isTx ? SendMessageState : RecvMessageState);

//         // 检查是否有换行符，这意味着下一行应该是相同的消息类型
//         if (dataContent.contains("\\r\\n") || dataContent.contains("\r\n")) {
//           // 明确标记此块，表示消息继续到下一行
//           setCurrentBlockState(isTx ? SendMessageState : RecvMessageState);
//         }
//       }
//     }
//   } else {
//     // 检查是否是未标记的行，但可能是前一行消息的继续
//     // 这处理显式换行的情况，而非转义序列
//     if (previousState == SendMessageState ||
//         previousState == RecvMessageState) {
//       QColor textColor =
//           (previousState == SendMessageState) ? Qt::darkBlue : Qt::darkGreen;
//       setFormat(0, text.length(), textColor);
//       setCurrentBlockState(previousState);
//     } else {
//       // 常规行，没有特殊标记
//       setCurrentBlockState(NoneState);
//     }
//   }
// }

// void SerialHighlighter::addRule(const QString& pattern,
//                                 const QTextCharFormat& format) {
//   HighlightRule rule;
//   rule.pattern = QRegularExpression(pattern);
//   rule.format = format;
//   rules_.append(rule);
// }

// void SerialHighlighter::highlightBlock(const QString& text) {
//   for (const auto& rule : rules_) {
//     auto matches = rule.pattern.globalMatch(text);
//     while (matches.hasNext()) {
//       auto match = matches.next();
//       setFormat(match.capturedStart(), match.capturedLength(), rule.format);
//     }
//   }
// }

void SerialHighlighter::highlightBlock(const QString& text) {
  // 默认格式
  setFormat(0, text.length(), Qt::black);

  // 获取前一个块的状态
  int previousState = previousBlockState();

  // 检查是否在多行数据模式中（上一行的延续）
  if (previousState == SendMessageState || previousState == RecvMessageState) {
    // 这一行是之前消息的继续，格式应保持一致
    QTextCharFormat format =
        (previousState == SendMessageState) ? m_txTextFormat : m_rxTextFormat;

    // 应用与数据类型相匹配的格式
    setFormat(0, text.length(), format);

    // 保持相同的状态
    setCurrentBlockState(previousState);
    return;
  }

  // 检查是否为新消息
  bool isTxMessage = false;
  int tagStart = -1;
  int tagLength = 0;

  // 查找 [Tx] 标签
  QRegularExpressionMatch txMatch = m_txTagPattern.match(text);
  if (txMatch.hasMatch()) {
    tagStart = txMatch.capturedStart();
    tagLength = txMatch.capturedLength();
    isTxMessage = true;
  } else {
    // 查找 [Rx] 标签
    QRegularExpressionMatch rxMatch = m_rxTagPattern.match(text);
    if (rxMatch.hasMatch()) {
      tagStart = rxMatch.capturedStart();
      tagLength = rxMatch.capturedLength();
      isTxMessage = false;
    }
  }

  // 如果找到了标签，就处理这一行
  if (tagStart != -1) {
    // 应用标签格式
    setFormat(tagStart, tagLength, isTxMessage ? m_txFormat : m_rxFormat);

    // 查找时间戳
    QRegularExpressionMatch timeMatch =
        m_timeStampPattern.match(text, tagStart + tagLength);
    if (timeMatch.hasMatch()) {
      int timeStart = timeMatch.capturedStart();
      int timeLength = timeMatch.capturedLength();

      // 应用时间戳格式
      setFormat(timeStart, timeLength, m_timestampFormat);

      // 获取数据部分
      int dataStart = timeStart + timeLength + 1;
      if (dataStart < text.length()) {
        QString dataContent = text.mid(dataStart).trimmed();

        // 检查是否为十六进制数据格式
        QRegularExpressionMatch hexMatch = m_hexDataPattern.match(dataContent);
        bool isHex = hexMatch.hasMatch();

        // 根据消息类型和数据格式应用格式
        if (isHex) {
          setFormat(dataStart, text.length() - dataStart, m_hexDataFormat);
        } else {
          setFormat(dataStart, text.length() - dataStart,
                    isTxMessage ? m_txTextFormat : m_rxTextFormat);
        }

        // 设置块状态，用于跨行消息
        setCurrentBlockState(isTxMessage ? SendMessageState : RecvMessageState);

        // 检查文本中是否包含换行符，如有则下一行将是相同状态
        if (dataContent.contains("\r\n") || dataContent.contains("\\r\\n") ||
            dataContent.contains("\n") || dataContent.contains("\\n")) {
          // 确保设置状态，让下一行继承当前格式
          setCurrentBlockState(isTxMessage ? SendMessageState
                                           : RecvMessageState);
        }
      }
    }
  } else {
    // 普通行，无标签 - 检查是否为前一行的延续
    if (previousState != NoneState) {
      QTextCharFormat format =
          (previousState == SendMessageState) ? m_txTextFormat : m_rxTextFormat;
      setFormat(0, text.length(), format);
      setCurrentBlockState(previousState);
    } else {
      // 完全普通的行
      setCurrentBlockState(NoneState);
    }
  }
}
