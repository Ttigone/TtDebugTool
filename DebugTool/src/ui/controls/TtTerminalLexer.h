#ifndef UI_CONTROLS_TTSERIALLEXER_H
#define UI_CONTROLS_TTSERIALLEXER_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

// class SerialHighlighter : public QSyntaxHighlighter {
//   Q_OBJECT

//  public:
//   explicit SerialHighlighter(QTextDocument* parent = nullptr);

//  protected:
//   void highlightBlock(const QString& text) override;

//  private:
//   // 辅助函数
//   bool isCompleteMessage(const QString& text) const;
//   bool containsNewlineSequence(const QString& text) const;

//   QRegularExpression m_messageStartPattern;  // 消息起始检测
//   QRegularExpression m_txTagPattern;
//   QRegularExpression m_rxTagPattern;
//   QRegularExpression m_timeStampPattern;
//   QRegularExpression m_hexDataPattern;

//   QTextCharFormat m_txFormat;
//   QTextCharFormat m_rxFormat;
//   QTextCharFormat m_timestampFormat;
//   QTextCharFormat m_hexDataFormat;
//   QTextCharFormat m_txTextFormat;
//   QTextCharFormat m_rxTextFormat;

//   // 状态定义 - 使用明确的值，避免冲突
//   enum {
//     NoneState = -1,  // 初始状态
//     SendMessageState = 1,
//     RecvMessageState = 2,
//     SendEndState = 3,  // 发送结尾
//     RecvEndState = 4   // 接收结尾
//   };
// };

namespace Ui {

class TtTerminalHighlighter : public QSyntaxHighlighter {
  Q_OBJECT

public:
  explicit TtTerminalHighlighter(QTextDocument *parent = nullptr);
  ~TtTerminalHighlighter();

protected:
  void highlightBlock(const QString &text) override;

private:
  QRegularExpression m_messageStartPattern; // 只匹配行首 “[Tx] [YYYY-MM-DD...”
  QRegularExpression m_txTagPattern;
  QRegularExpression m_rxTagPattern;
  QRegularExpression m_timeStampPattern;
  QRegularExpression m_hexDataPattern;

  QTextCharFormat m_txFormat;
  QTextCharFormat m_rxFormat;
  QTextCharFormat m_timestampFormat;
  QTextCharFormat m_hexDataFormat;
  QTextCharFormat m_txTextFormat;
  QTextCharFormat m_rxTextFormat;

  enum { NoneState = -1, SendMessageState = 1, RecvMessageState = 2 };
};

} // namespace Ui

#endif // TTSERIALLEXER_H
