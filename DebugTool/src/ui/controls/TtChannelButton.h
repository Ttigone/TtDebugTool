#ifndef TTCHANNELBUTTON_H
#define TTCHANNELBUTTON_H

class TtChannelButton : public QWidget {
  Q_OBJECT
  Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
 public:
  explicit TtChannelButton(QWidget* parent = nullptr);
  explicit TtChannelButton(const QColor& color, const QString& text,
                           QWidget* parent = nullptr);
  ~TtChannelButton();

  QColor getColor() const;
  void setColors(const QColor& color);
  QString getText() const;
  void setText(const QString& text);
  QColor getHoverBackgroundColor() const;
  void setHoverBackgroundColor(const QColor& color);
  QColor getCheckBlockColor() const;
  void setCheckBlockColor(const QColor& color);

  bool isChecked() const;
  void setChecked(bool checked);
  void setEnableHoldToCheck(bool enable);
  void setEnable(bool enabled);

  QString getUuid() const;
  void setUuid(const QString& uuid);

 public slots:
  void modifyText();

 signals:
  void clicked();
  void toggled(bool checked);
  void textChanged(const QString& newText);  // 新增文本修改信号

 protected:
  void paintEvent(QPaintEvent* event) override;
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  bool eventFilter(QObject* watched, QEvent* event) override;
  QSize sizeHint() const override;

 private:
  void clearupEditor();
  QRect checkBlockRect() const;
  bool is_pressed_;
  QSize color_size_;
  bool is_checked_;
  bool enable_hold_to_check_;
  bool checkbox_pressed_ = false;

  QString text_;
  QColor current_color_;
  QColor normal_color_;
  QColor check_block_color_;
  bool is_hovered_;

  QLineEdit* rename_editor_ = nullptr;
  QString original_text_;
  bool ignore_next_release_;

  QString uuid_;
};

#endif  // TTCHANNELBUTTON_H
