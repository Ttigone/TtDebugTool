#ifndef WINDOW_COMBOBOX_H
#define WINDOW_COMBOBOX_H
#include <QListView>


namespace Ui {

class TtImageButton;

class TtComboBox : public QComboBox {
 public:
  TtComboBox(QWidget* parent = nullptr);
  ~TtComboBox();

 protected:
  //void showPopup() override {
  //  QListView* listView = qobject_cast<QListView*>(view());
  //  if (listView) {
  //    QRect rect = this->rect();
  //    QPoint pos =
  //        this->mapToGlobal(QPoint(0, rect.height()));  // 设置下拉框位置
  //    qDebug() << "pos: " << pos; // 左上角的坐标
  //    qDebug() << "DPI Scale Factor: " << this->devicePixelRatioF();
  //    QWidget* mainWin = this->window();
  //    QPoint mainPos = mainWin->mapToGlobal(QPoint(0, 0));
  //    qDebug() << "MainWin Global Pos:" << mainPos;
  //    listView->setGeometry(pos.x(), pos.y(), rect.width(), 100);  // 限制高度
  //    listView->move(pos);
  //  }
  //  qDebug() << "showpo";
  //  QComboBox::showPopup();
  //}
};

class TtLabelComboBox : public QWidget {
  Q_OBJECT
 public:
  TtLabelComboBox(Qt::AlignmentFlag flag, const QString &text = "", QWidget *parent = nullptr);
  TtLabelComboBox(const QString &text = "", QWidget *parent = nullptr);
  ~TtLabelComboBox();

  QComboBox *body();
  void addItem(const QString &atext, const QVariant &auserData = QVariant());

  void setCurrentItem(qint8 index);
  QString itemText(int index);
  QString currentText();
  int count();

  void shortCurrentItemText();

 signals:
  void currentIndexChanged(int index);

 private:
  TtComboBox *combo_box_;
  QLabel *label_;
};

class TtLabelBtnComboBox : public QWidget {
  Q_OBJECT
 public:
  TtLabelBtnComboBox(Qt::AlignmentFlag flag, const QString& text,
                     const QString& image_path, QWidget* parent = nullptr);
  TtLabelBtnComboBox(const QString& text = "", QWidget* parent = nullptr);
  ~TtLabelBtnComboBox();

  void addItem(const QString& atext, const QVariant& auserData = QVariant());

  QComboBox* body();
  // 设置当前项
  void setCurrentItem(qint8 index);

  QString itemText(int index);

  QString currentText();

  ///
  /// @brief count item 个数
  ///
  int count();

 signals:
  void clicked();

 private slots:
  void displayCurrentCOMx();

 private:
  TtLabelComboBox* part_;
  // Ui::TtImageButton* refresh_btn_;
};

}  // namespace Ui

#endif  // WINDOW_COMBOBOX_H
