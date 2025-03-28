#ifndef UI_CONTROLS_TTSERIALLEXER_HPP
#define UI_CONTROLS_TTSERIALLEXER_HPP

#include <Qsci/qscilexercustom.h>
#include <Qsci/qsciscintilla.h>

#include <QStringRef>

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

    // startStyling(start);

    // for (int i = 0; i < lines.size(); ++i) {
    //   QString line = lines[i];
    //   int pos = 0;
    //   int tsEnd = line.indexOf(']', pos);
    //   if (tsEnd != -1) {
    //     setStyling(tsEnd + 1 - pos, TIMESTAMP_STYLE);
    //     pos = tsEnd + 1;
    //   }
    //   int arrowStart = -1;
    //   bool isSend = false;

    //   // 2. 跳过时间戳后的空格
    //   while (pos < line.length() && line[pos].isSpace()) {
    //     pos++;
    //   }

    //   // 3. 检查<<或>>标签
    //   if (pos + 1 < line.length()) {
    //     QString arrow = line.mid(pos, 2);
    //     if (arrow == "<<") {
    //       isSend = true;
    //       arrowStart = pos;
    //     } else if (arrow == ">>") {
    //       isSend = false;
    //       arrowStart = pos;
    //     }
    //   }

    //   // 正文
    //   if (pos < line.length()) {
    //     int messageStyle = isSend ? SEND_MESSAGE_STYLE : RECV_MESSAGE_STYLE;
    //     setStyling(line.length() - pos, messageStyle);
    //   }
    //   if (i < lines.size() - 1) {
    //     setStyling(1, DEFAULT_STYLE);
    //   }
    // }
    int currentPos = start;  // 当前处理位置的绝对偏移

    for (int i = 0; i < lines.size(); ++i) {
      QString line = lines[i];
      int lineLength = line.length();
      int lineStart = currentPos;

      int pos = 0;  // 行内相对位置

      // 1. 处理时间戳 [yyyy-MM-dd hh:mm:ss]
      int tsEnd = line.indexOf(']', pos);
      if (tsEnd != -1) {
        int tsLength = tsEnd + 1 - pos;  // 包括']'
        startStyling(lineStart + pos);
        setStyling(tsLength, TIMESTAMP_STYLE);
        pos = tsEnd + 1;  // 移动到时间戳后
      }

      // 2. 跳过时间戳后的空格
      while (pos < lineLength && line[pos].isSpace()) {
        pos++;
      }

      bool isSend = false;
      int arrowPos = pos;

      // 3. 检查箭头 << 或 >>
      if (pos + 1 < lineLength) {
        QStringRef arrow(&line, pos, 2);
        if (arrow == "<<") {
          isSend = true;
          qDebug() << "发送1";
          // 设置箭头样式
          startStyling(lineStart + pos);
          setStyling(2, SEND_STYLE);
          pos += 2;
        } else if (arrow == ">>") {
          qDebug() << "接收2";
          isSend = false;
          // 设置箭头样式
          startStyling(lineStart + pos);
          setStyling(2, RECV_STYLE);
          pos += 2;
        }
      }

      // 4. 跳过箭头后的空格
      while (pos < lineLength && line[pos].isSpace()) {
        pos++;
      }

      // 5. 处理消息内容
      if (pos < lineLength) {
        int messageStyle = isSend ? SEND_MESSAGE_STYLE : RECV_MESSAGE_STYLE;
        startStyling(lineStart + pos);
        setStyling(lineLength - pos, messageStyle);
      }

      // 处理换行符（假设每行以\n结尾）
      currentPos += lineLength + 1;  // 行内容长度 + 换行符
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
