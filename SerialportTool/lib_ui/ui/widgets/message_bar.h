/*****************************************************************/ /**
 * \file   snack_bar.h
 * \brief  
 * 
 * \author C3H3_Ttigone
 * \date   August 2024
 *********************************************************************/

#ifndef UI_WIDGETS_MESSAGE_BAR_H
#define UI_WIDGETS_MESSAGE_BAR_H

QT_BEGIN_NAMESPACE
class QApplication;
class QObject;
class QTimer;
QT_END_NAMESPACE
// #include <QApplication>
// #include <QObject>
// #include <QTimer>

#include "ui/Def.h"
#include "ui/ui_pch.h"

namespace Ui {

class TtMessageBarPrivate;
class Tt_EXPORT TtMessageBar : public QWidget {
  Q_OBJECT
  // 私有类访问 q 指针
  Q_Q_CREATE(TtMessageBar)

 public:
  static void success(TtMessageBarType::PositionPolicy policy, QString title,
                      QString text, int displayMsec, QWidget* parent = nullptr);
  static void warning(TtMessageBarType::PositionPolicy policy, QString title,
                      QString text, int displayMsec, QWidget* parent = nullptr);
  static void information(TtMessageBarType::PositionPolicy policy,
                          QString title, QString text, int displayMsec,
                          QWidget* parent = nullptr);
  static void error(TtMessageBarType::PositionPolicy policy, QString title,
                    QString text, int displayMsec, QWidget* parent = nullptr);

 protected:
  void paintEvent(QPaintEvent* event) override;
  bool eventFilter(QObject* watched, QEvent* event) override;

 private:
  friend class TtMessageBarManager;

  explicit TtMessageBar(TtMessageBarType::PositionPolicy policy,
                        TtMessageBarType::MessageMode messageMode,
                        QString& title, QString& text, int displayMsec,
                        QWidget* parent = nullptr);
  ~TtMessageBar();
};

}  // namespace Ui

#endif  //SNACK_MESSAGE_H
