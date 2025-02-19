#include "ui/control/buttonbox/TtButtonBox.h"

#include <QButtonGroup>


namespace Ui {

// Initialize the static stylesheet
const QString TtButtonGroup::styleSheet = R"(
QPushButton {
    min-width: 36;
    min-height: 22;
    max-height: 22;
    border: none;
    border-radius: 0px;
    background-color: #0078d4;
    color: white;
    font-size: 12;
    padding: 10px;
}

QPushButton:checked {
    background-color: #005a9e;
}

QPushButton:hover {
    background-color: #005a9e;
}

QPushButton:pressed {
    background-color: #004578;
}
)";

TtButtonGroup::TtButtonGroup(QWidget* parent)
    : QWidget(parent),
      button1(new QPushButton("TEXT", this)),
      button2(new QPushButton("HEX", this)),
      buttonGroup(new QButtonGroup(this)),
      animation1(nullptr),
      animation2(nullptr) {
  setupUI();
  initConnections();
  //setStyleSheet(styleSheet);

  // 延迟记录初始尺寸，确保布局已完成
  QTimer::singleShot(0, this, [this]() {
    button1InitialSize = button1->size();
    button2InitialSize = button2->size();
  });
}

void TtButtonGroup::setupUI() {
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins(QMargins());

  // Configure buttons
  button1->setCheckable(true);
  button1->setChecked(true);
  button2->setCheckable(true);

  // Add buttons to the button group
  buttonGroup->addButton(button1, 1);
  buttonGroup->addButton(button2, 2);

  // Add buttons to the layout
  layout->addWidget(button1);
  layout->addWidget(button2);

  setLayout(layout);
}

void TtButtonGroup::initConnections() {
  // Connect button group buttonClicked signal to the slot
  connect(buttonGroup,
          QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this,
          &TtButtonGroup::buttonClicked);
}

void TtButtonGroup::buttonClicked(QAbstractButton* button) {
  if (button == button1) {
    emit firstButtonClicked();
  } else if (button == button2) {
    emit secondButtonClicked();
  }
}

void TtButtonGroup::animateButton(QPushButton* button, qreal scale) {
  QSequentialAnimationGroup** animationPtr = nullptr;
  QSize initialSize;

  // Determine which animation group and initial size to use
  if (button == button1) {
    animationPtr = &animation1;
    initialSize = button1InitialSize;
  } else if (button == button2) {
    animationPtr = &animation2;
    initialSize = button2InitialSize;
  } else {
    return;
  }

  // If an animation is already running, stop and delete it
  if (*animationPtr &&
      (*animationPtr)->state() == QAbstractAnimation::Running) {
    (*animationPtr)->stop();
    delete *animationPtr;
    *animationPtr = nullptr;
  }

  // // 禁用按钮, 放置多次点击
  // button->setEnabled(false);

  // Create a new sequential animation group
  QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);

  // Scale down animation
  QPropertyAnimation* scaleDown = new QPropertyAnimation(button, "size");
  scaleDown->setDuration(100);
  scaleDown->setStartValue(button->size());
  scaleDown->setEndValue(initialSize * scale);
  scaleDown->setEasingCurve(QEasingCurve::OutQuad);

  // Scale up animation
  QPropertyAnimation* scaleUp = new QPropertyAnimation(button, "size");
  scaleUp->setDuration(100);
  scaleUp->setStartValue(initialSize * scale);
  scaleUp->setEndValue(initialSize);
  scaleUp->setEasingCurve(QEasingCurve::InQuad);

  // Add animations to the group
  group->addAnimation(scaleDown);
  group->addAnimation(scaleUp);

  // When the animation finishes, reset the animation pointer
  connect(group, &QSequentialAnimationGroup::finished,
          [animationPtr]() { *animationPtr = nullptr; });

  // Store the animation instance
  *animationPtr = group;

  // Start the animation
  group->start(QAbstractAnimation::DeleteWhenStopped);
}

TtButtonBox::TtButtonBox() {}

}  // namespace Ui
