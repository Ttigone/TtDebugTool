#ifndef TTCONTENTDIALOG_H
#define TTCONTENTDIALOG_H

#include <QAbstractNativeEventFilter>
#include <QDialog>

#include "ui/Def.h"

namespace Ui {

class TtContentDialogPrivate;
// class Tt_EXPORT TtContentDialog : public QDialog, QAbstractNativeEventFilter
// {
class Tt_EXPORT TtContentDialog : public QDialog {
  Q_OBJECT
  Q_Q_CREATE(TtContentDialog)

public:
  enum LayoutSelection {
    TWO_OPTIONS,
    THREE_OPTIONS,
  };

  enum ButtonType { LeftButton, MiddleButton, RightButton };

  explicit TtContentDialog(QWidget *parent = nullptr);
  explicit TtContentDialog(
      bool fixSize, LayoutSelection layout = LayoutSelection::TWO_OPTIONS,
      QWidget *parent = nullptr);

  explicit TtContentDialog(
      Qt::WindowModality modality, bool fixSize = true,
      LayoutSelection layout = LayoutSelection::TWO_OPTIONS,
      QWidget *parent = nullptr);

  ~TtContentDialog();

  ButtonType clickButtonType() const { return clicked_type_; }

  /**
   * @brief 设置对话框的中央部件
   * @param centralWidget 要设置的中央部件
   *
   * 注意：本方法不会获取centralWidget的所有权，
   * 调用者负责确保传入的部件在对话框关闭后仍然有效或及时清理
   */
  void setCentralWidget(QWidget *centralWidget);

  void setLeftButtonText(const QString &text);
  void setMiddleButtonText(const QString &text);
  void setRightButtonText(const QString &text);
  void setCenterText(const QString &text);

  void setEnablePointOnMouse(bool enable);

public slots:
  virtual void onLeftButtonClicked();
  virtual void onMiddleButtonClicked();
  virtual void onRightButtonClicked();

signals:
  void leftButtonClicked();
  void middleButtonClicked();
  void rightButtonClicked();

protected:
  void paintEvent(QPaintEvent *event) override;
  void showEvent(QShowEvent *event) override;
  void closeEvent(QCloseEvent *event) override;
  bool eventFilter(QObject *obj, QEvent *event) override;

  // #if (QT_VERSION == QT_VERSION_CHECK(6, 5, 3) || \
//      QT_VERSION == QT_VERSION_CHECK(6, 6, 0))
  //   bool eventFilter(QObject* obj, QEvent* event) override;
  // #endif
  // #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  // //   // virtual bool nativeEventFilter(const QByteArray& eventType, void*
  // message,
  // //   //                                qintptr* result) override;
  // //   virtual bool nativeEventFilter(const QByteArray& eventType, void*
  // message,
  // //                                  qintptr* result) override;
  // // #else
  // //   virtual bool nativeEventFilter(const QByteArray& eventType, void*
  // message,
  // //                                  long* result) override;
  // #endif

private:
  void adjustPosition();
  void startShowAnimation();

  QPoint start_pos_;
  QPropertyAnimation *animation_ = nullptr;
  ButtonType clicked_type_ = LeftButton;
};

} // namespace Ui

#endif // TTCONTENTDIALOG_H
