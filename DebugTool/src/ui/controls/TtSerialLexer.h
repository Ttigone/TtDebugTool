#ifndef UI_CONTROLS_TTSERIALLEXER_HPP
#define UI_CONTROLS_TTSERIALLEXER_HPP

#include <Qsci/qscilexercustom.h>
#include <Qsci/qsciscintilla.h>

class SerialLexer : public QsciLexerCustom {
 public:
  SerialLexer(QObject* parent = nullptr) : QsciLexerCustom(parent) {
    QFont font("Consolas", 10);
    setColor(QColor("#000000"), DEFAULT_STYLE);
    setColor(QColor("#00FF00"), SEND_STYLE);
    setColor(QColor("#FF0000"), RECV_STYLE);
    setColor(QColor("#007ACC"), SEND_MESSAGE_STYLE);
    setColor(QColor("#8000FF"), RECV_MESSAGE_STYLE);
    setColor(QColor("#888888"), TIMESTAMP_STYLE);
    // 统一字体
    setFont(font, SEND_STYLE);
    setFont(font, RECV_STYLE);
    setFont(font, SEND_MESSAGE_STYLE);
    setFont(font, RECV_MESSAGE_STYLE);
    setFont(font, TIMESTAMP_STYLE);
  }

  const char* language() const override { return "Serial"; }

  QString description(int style) const override {
    switch (style) {
      case SEND_STYLE:
        return "Send";
      case RECV_STYLE:
        return "Receive";
      case TIMESTAMP_STYLE:
        return "Timestamp";
      default:
        return "";
    }
  }

  void styleText(int start, int end) override {
    if (!editor()) {
      return;
    }

    QsciScintilla* sci = editor();
    QString text = sci->text(start, end);
    QStringList lines = text.split('\n', Qt::KeepEmptyParts);

    startStyling(start);

    for (int i = 0; i < lines.size(); ++i) {
      QString line = lines[i];
      int pos = 0;
      int tsEnd = line.indexOf(']', pos);
      if (tsEnd != -1) {
        setStyling(tsEnd + 1 - pos, TIMESTAMP_STYLE);
        pos = tsEnd + 1;
      }
      // 处理标签部分 [SEND] 或 [RECV]
      int tagEnd = line.indexOf(']', pos);
      bool isSend = line.contains("[SEND]");
      if (tagEnd != -1) {
        int tagStyle = isSend ? SEND_STYLE : RECV_STYLE;
        setStyling(tagEnd + 1 - pos, tagStyle);
        pos = tagEnd + 1;
      }
      // 正文
      if (pos < line.length()) {
        int messageStyle = isSend ? SEND_MESSAGE_STYLE : RECV_MESSAGE_STYLE;
        setStyling(line.length() - pos, messageStyle);
      }
      if (i < lines.size() - 1) {
        setStyling(1, DEFAULT_STYLE);
      }
    }
  }

 private:
  enum {
    DEFAULT_STYLE = 0,
    SEND_STYLE = 1,
    RECV_STYLE = 2,
    TIMESTAMP_STYLE = 3,
    SEND_MESSAGE_STYLE = 4,
    RECV_MESSAGE_STYLE = 5
  };
};

#endif  // TTSERIALLEXER_HPP
