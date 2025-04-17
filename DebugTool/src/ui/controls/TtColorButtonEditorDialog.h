#ifndef TTCOLORBUTTONEDITORDIALOG_H
#define TTCOLORBUTTONEDITORDIALOG_H

#include <QColorDialog>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <ui/widgets/buttons.h>
#include "TtColorButton.h"
#include "ui/control/TtLineEdit.h"

class TtColorButtonEditorDialog : public QDialog {
  Q_OBJECT
 public:
  // 构造函数传入要编辑的按钮指针
  TtColorButtonEditorDialog(TtColorButton* button, QWidget* parent = nullptr)
      : QDialog(parent), m_button(button) {
    setWindowTitle(tr("编辑按钮属性"));
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    title_edit_ = new Ui::TtLabelLineEdit(tr("名称"), this);
    title_edit_->setText(m_button->getText());
    Ui::TtLabelLineEdit* frameHeader =
        new Ui::TtLabelLineEdit(tr("帧头"), this);
    Ui::TtLabelLineEdit* frameLength =
        new Ui::TtLabelLineEdit(tr("数据帧长度"), this);
    Ui::TtLabelLineEdit* frameEnd = new Ui::TtLabelLineEdit(tr("帧尾"), this);

    mainLayout->addWidget(title_edit_);
    mainLayout->addWidget(frameHeader);
    mainLayout->addWidget(frameLength);
    mainLayout->addWidget(frameEnd);

    Ui::TtFancyButton* scriptBtton =
        new Ui::TtFancyButton(Qt::white, tr("脚本编辑"), this);
    mainLayout->addWidget(scriptBtton);

    connect(scriptBtton, &Ui::TtFancyButton::clicked, this, [this]() {

    });

    // QHBoxLayout* colorLayout = new QHBoxLayout;
    // colorLayout->addWidget(new QLabel(tr("背景色:"), this));
    // m_colorButton =
    //     new Ui::TtFancyButton(m_button->getColor(), tr("选择颜色"), this);
    // connect(m_colorButton, &Ui::TtFancyButton::clicked, this, [this]() {
    //   QColor chosen = QColorDialog::getColor(m_button->getColor(), this,
    //                                          tr("选择背景颜色"));
    //   if (chosen.isValid()) {
    //     m_chosenColor = chosen;
    //     m_colorButton->setColor(m_chosenColor);
    //   }
    // });
    // colorLayout->addWidget(m_colorButton);
    // mainLayout->addLayout(colorLayout);

    QHBoxLayout* checkBlockLayout = new QHBoxLayout;
    checkBlockLayout->addWidget(new QLabel(tr("勾选块色:"), this));

    m_checkBlockButton = new Ui::TtFancyButton(m_button->getCheckBlockColor(),
                                               tr("选择颜色"), this);
    connect(m_checkBlockButton, &Ui::TtFancyButton::clicked, this, [this]() {
      QColor chosen = QColorDialog::getColor(m_button->getCheckBlockColor(),
                                             this, tr("选择背景颜色"));
      if (chosen.isValid()) {
        m_chosenCheckBlockColor = chosen;
        m_checkBlockButton->setColor(m_chosenCheckBlockColor);
      }
    });
    checkBlockLayout->addWidget(m_checkBlockButton);

    mainLayout->addLayout(checkBlockLayout);

    // 4. 底部确定与取消按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    QPushButton* okButton = new QPushButton(tr("确定"), this);
    QPushButton* cancelButton = new QPushButton(tr("取消"), this);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, this,
            &TtColorButtonEditorDialog::applyChanges);
    connect(cancelButton, &QPushButton::clicked, this,
            &TtColorButtonEditorDialog::reject);
  }

 private slots:
  void applyChanges() {
    if (m_button) {
      m_button->setText(title_edit_->currentText());
      // if (m_chosenColor.isValid()) {
      //   m_button->setColors(m_chosenColor);
      // }
      if (m_chosenCheckBlockColor.isValid()) {
        m_button->setCheckBlockColor(m_chosenCheckBlockColor);
      }
    }
    accept();
  }

 private:
  TtColorButton* m_button;
  // Ui::TtLineEdit* m_textEdit;
  Ui::TtLabelLineEdit* title_edit_;
  Ui::TtFancyButton* m_colorButton;
  Ui::TtFancyButton* m_checkBlockButton;
  QColor m_chosenColor;
  QColor m_chosenCheckBlockColor;
};

#endif  // TTCOLORBUTTONEDITORDIALOG_H
