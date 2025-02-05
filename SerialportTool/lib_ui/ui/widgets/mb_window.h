/*****************************************************************//**
 * \file   mb_window.h
 * \brief  继承自 MbWidget 的窗口类
 * 
 * \author C3H3_Ttigone
 * \date   August 2024
 *********************************************************************/

#ifndef MB_WINDOW_H
#define MB_WINDOW_H

#include "ui/mb_widget.h"
#include "base/flags.h"

namespace Ui {

namespace Platform
{
class BasicWindowHelper;

}  // namespace Platform
}  // namespace Ui

namespace Ui {

class MbWindow : public MbWidget {
 public:
  explicit MbWindow(QWidget *parent = nullptr);
  ~MbWindow();

	[[nodiscard]] not_null<MbWidget *> body(); 
	[[nodiscard]] not_null<MbWidget *> body() const; 
  [[nodiscard]] QMargins frameMargins();

  [[nodiscard]] int addtionalContenPadding() const;
	[[nodiscard]] int addtionalContentPaddingValue() const;



  void setTitle(const QString &title);
  void setNativeFrame(bool enable);
  void setMinimumSize(QSize size);

  void setFixedSize(QSize size);

  void setGeometry(QRect rect);
  void showFullScreen();
  void showNormal();
  void close();


private:
  const std::unique_ptr<Platform::BasicWindowHelper> helper_;

};
}  // namespace Ui

#endif  // MB_WINDOW_H
