#ifndef UI_CONTROLS_TTSERIALLEXER_H
#define UI_CONTROLS_TTSERIALLEXER_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class SerialHighlighter : public QSyntaxHighlighter {
  Q_OBJECT

 public:
  explicit SerialHighlighter(QTextDocument* parent = nullptr);

 protected:
  void highlightBlock(const QString& text) override;

 private:
  // 预编译的正则表达式
  QRegularExpression m_txTagPattern;
  QRegularExpression m_rxTagPattern;
  QRegularExpression m_timeStampPattern;
  QRegularExpression m_hexDataPattern;

  // 格式定义
  QTextCharFormat m_txFormat;
  QTextCharFormat m_rxFormat;
  QTextCharFormat m_timestampFormat;
  QTextCharFormat m_hexDataFormat;
  QTextCharFormat m_txTextFormat;
  QTextCharFormat m_rxTextFormat;

  // 状态定义 - 使用明确的值，避免冲突
  enum {
    NoneState = -1,  // 初始状态
    SendMessageState = 1,
    RecvMessageState = 2,
    SendEndState = 3,  // 发送结尾
    RecvEndState = 4   // 接收结尾
  };
};

#endif  // TTSERIALLEXER_H
