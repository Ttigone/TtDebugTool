/*****************************************************************/ /**
 * \file   message_textedit.h
 * \brief  消息输入框
 *
 * \author C3H3_Ttigone
 * \date   August 2024
 *********************************************************************/

#ifndef UI_WIDGETS_MESSAGE_TEXTEDIT_H
#define UI_WIDGETS_MESSAGE_TEXTEDIT_H

#include <QTextEdit>

namespace Ui {

class MessageEdit : public QTextEdit {
 public:
  struct MsgInfo {
    QString MsgFlag;  //"text,image,file"
    QString Content;  //表示文件和图像的url,文本信息
    QPixmap Pixmap;   //文件和图片的缩略图
  };

  explicit MessageEdit(QWidget* parent = nullptr);

  ~MessageEdit();

  [[nodiscard]] bool isImage(QString url);

  QVector<MsgInfo> getMsgList();

 signals:
  void sendMsg();

 private slots:
  void textEditChanged();

 protected:
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

 private:
  // void insertFromMimeData(const QMimeData* source) override;
  void insertMsgList(QVector<MsgInfo>& list, const QString flag,
                     const QString& text, const QPixmap pixmap);
  QStringList getUrl(const QString& text);
  QPixmap getFileIconPixmap(const QString& url);
  QString getFileSize(int64 size);

  QVector<MsgInfo> msg_list_;
  QVector<MsgInfo> live_msg_list_;
};

}  // namespace Ui

#endif  //MESSAGE_TEXTEDIT_H
