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
  struct HighlightRule {
    QRegularExpression pattern;
    QTextCharFormat format;
    int state = -1;  // -1表示不改变状态
  };

  QTextCharFormat m_timestampFormat;
  QTextCharFormat m_sendArrowFormat;
  QTextCharFormat m_recvArrowFormat;
  QTextCharFormat m_sendMsgFormat;
  QTextCharFormat m_recvMsgFormat;

  QVector<HighlightRule> m_rules;

  // 状态定义
  enum { NoneState = 0, SendMessageState = 1, RecvMessageState = 2 };
};

#endif  // TTSERIALLEXER_H
