#include "window/main_window.h"

#include <QActionGroup>
#include <QApplication>
#include <QCoreApplication>
#include <QDockWidget>
#include <QGuiApplication>
#include <QLabel>
#include <QListWidget>
#include <QMargins>
#include <QPushbutton>
#include <QStackedLayout>
#include <QTimer>
#include <QVBoxLayout>
#include <QtWidgets/QStyle>
#include <qtconcurrentrun.h>

#include <QWKWidgets/widgetwindowagent.h>
#include <Qt-Advanced-Stylesheets/src/QtAdvancedStylesheet.h>
#include <qtmaterialsnackbar.h>

#include <ui/effects/animated_drawer.h>
#include <ui/layout/vertical_layout.h>

#include <ui/widgets/buttons.h>
#include <ui/widgets/customizationtabwidget.h>
#include <ui/widgets/labels.h>
#include <ui/widgets/message_bar.h>
#include <ui/widgets/widget_group.h>

#include <ui/window/title/window_button.h>
#include <ui/window/title/windowbar.h>

#include "window/function_selection_window.h"
#include "window/modbus_window.h"
#include "window/mqtt_window.h"
#include "window/popup_window.h"
#include "window/serial_window.h"
#include "window/tcp_window.h"
#include "window/udp_window.h"

#include "ui/widgets/session_manager.h"

#include <ui/window/title/window_button.h>
#include "storage/setting_manager.h"
#include "ui/widgets/window_switcher.h"

#include <ui/widgets/widget_group.h>
#include "window/instruction_window.h"


namespace Window {

static inline void emulateLeaveEvent(QWidget* widget) {
  Q_ASSERT(widget);
  if (!widget) {
    return;
  }
  QTimer::singleShot(0, widget, [widget]() {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    const QScreen* screen = widget->screen();
#else
        const QScreen *screen = widget->windowHandle()->screen();
#endif
    const QPoint globalPos = QCursor::pos(screen);
    if (!QRect(widget->mapToGlobal(QPoint{0, 0}), widget->size())
             .contains(globalPos)) {
      QCoreApplication::postEvent(widget, new QEvent(QEvent::Leave));
      if (widget->testAttribute(Qt::WA_Hover)) {
        const QPoint localPos = widget->mapFromGlobal(globalPos);
        const QPoint scenePos = widget->window()->mapFromGlobal(globalPos);
        static constexpr const auto oldPos = QPoint{};
        const Qt::KeyboardModifiers modifiers =
            QGuiApplication::keyboardModifiers();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
        const auto event = new QHoverEvent(QEvent::HoverLeave, scenePos,
                                           globalPos, oldPos, modifiers);
        Q_UNUSED(localPos);
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
                const auto event =  new QHoverEvent(QEvent::HoverLeave, localPos, globalPos, oldPos, modifiers);
                Q_UNUSED(scenePos);
#else
                const auto event =  new QHoverEvent(QEvent::HoverLeave, localPos, oldPos, modifiers);
                Q_UNUSED(scenePos);
#endif
        QCoreApplication::postEvent(widget, event);
      }
    }
  });
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), central_widget_(new QWidget(this)) {

  qDebug() << "MainWindow constructor thread:" << QThread::currentThread();

  // statusBar()->addWidget(new Ui::TtSvgButton());
  window_agent_ = new QWK::WidgetWindowAgent(this);
  installWindowAgent();
  setProperty("TtBaseClassName", "TtMainWindow");
  setObjectName("TtMainWindow");
  resize(800, 600);

  window_agent_->centralize();

  layout_ = new Ui::TtHorizontalLayout;
  central_widget_->setLayout(layout_);
  central_widget_->setProperty("TtBaseClassName", "CentralWidget");
  central_widget_->setObjectName("CentralWidget");
  setCentralWidget(central_widget_);

  //setAttribute(Qt::WA_DontCreateNativeAncestors);
  // 拖拽右边时, 会出现宽度影响高度的情况

  setWindowTitle(tr("TtSerialPort"));

  loadStyleSheet(Theme::Dark);

  setLeftBar();

  function_select_ = new FunctionSelectionWindow();

  // 初始界面是 test111
  tabWidget_ = new Ui::TabManager(function_select_);

  // 容纳一定了 widget, 以供外部切换, 直接存放 widget, 而非设定
  auto test = new Window::PopUpWindow();

  QSplitter* mainSplitter = new QSplitter(this);
  mainSplitter->addWidget(test);
  test->setMinimumWidth(1);
  mainSplitter->addWidget(tabWidget_);

  mainSplitter->setStretchFactor(0, 1);
  mainSplitter->setStretchFactor(1, 3);

  mainSplitter->setCollapsible(0, true);
  mainSplitter->setCollapsible(1, false);
  // 创建 AnimatedDrawer 控制器
  Ui::TtAnimatedDrawer* controller =
      new Ui::TtAnimatedDrawer(mainSplitter, test, tabWidget_, this);

  {
    QWidget* linkList = new QWidget;
    auto linkListLayout = new QGridLayout(linkList);

    Ui::TtNormalLabel* title = new Ui::TtNormalLabel(tr("连接列表"), linkList);

    QWidget* btnWidget = new QWidget(linkList);
    Ui::TtHorizontalLayout* btnWidgetLayout =
        new Ui::TtHorizontalLayout(btnWidget);
    Ui::TtSvgButton* addBtn =
        new Ui::TtSvgButton(":/sys/plus-circle.svg", btnWidget);  // 刷新
    addBtn->setSvgSize(QSize(18, 18));
    Ui::TtSvgButton* refreshBtn =
        new Ui::TtSvgButton(":/sys/refresh-normal.svg", btnWidget);  // 新建
    refreshBtn->setEnableHoldToCheck(true);
    refreshBtn->setSvgSize(QSize(18, 18));
    refreshBtn->setColors(Qt::black, Qt::blue);
    btnWidgetLayout->addWidget(addBtn);
    btnWidgetLayout->addSpacerItem(new QSpacerItem(5, 0));
    btnWidgetLayout->addWidget(refreshBtn);

    linkListLayout->addWidget(title, 0, 0, Qt::AlignTop);
    linkListLayout->addWidget(btnWidget, 0, 1, Qt::AlignRight);
    Ui::TtNormalLabel* displayInfo = new Ui::TtNormalLabel();
    displayInfo->setPixmap(
        QPixmap::fromImage(QImage(":/sys/tmp.png").scaled(88, 88)));
    Ui::TtNormalLabel* displayWords =
        new Ui::TtNormalLabel(tr("暂时没有可用的连接"));
    auto addLinkBtn = new QPushButton(tr("新建连接"));
    addLinkBtn->setMinimumSize(100, 28);
    auto original_widget_ = new QWidget();
    Ui::TtVerticalLayout* tmpl = new Ui::TtVerticalLayout(original_widget_);
    tmpl->addStretch();
    tmpl->addSpacerItem(new QSpacerItem(0, 20));
    tmpl->addWidget(displayWords, 0, Qt::AlignHCenter);
    tmpl->addWidget(addLinkBtn, 0, Qt::AlignHCenter);
    tmpl->addStretch();
    history_link_list_ = new Ui::SessionManager();
    history_link_list_->setSelectionMode(QAbstractItemView::NoSelection);
    auto stack_ = new QStackedWidget(linkList);
    stack_->addWidget(original_widget_);
    stack_->addWidget(history_link_list_);
    linkListLayout->addWidget(stack_, 1, 0, 1, 2);
    connect(history_link_list_->model(), &QAbstractItemModel::rowsInserted,
            [=](const QModelIndex& parent, int first, int last) {
              if (history_link_list_->count() == 1) {  // 从0变为1
                stack_->setCurrentIndex(1);
              }
            });
    connect(history_link_list_->model(), &QAbstractItemModel::rowsRemoved,
            [=](const QModelIndex& parent, int first, int last) {
              if (history_link_list_->count() == 0) {  // 从1变为0
                stack_->setCurrentIndex(0);
              }
            });
    connect(addLinkBtn, &QPushButton::clicked, [this, controller]() {
      FunctionSelectionWindow* test2 = new FunctionSelectionWindow();
      tabWidget_->addNewTab(test2, tr("新建连接"));
      QObject::connect(test2, &FunctionSelectionWindow::switchRequested,
                       [this](TtProtocolRole::Role role) {
                         tabWidget_->handleButtonClicked(
                             tabWidget_->currentIndex(), role);
                       });
      tabWidget_->setCurrentWidget(test2);
    });
    connect(addBtn, &Ui::TtSvgButton::clicked, [this, controller]() {
      FunctionSelectionWindow* test2 = new FunctionSelectionWindow();
      tabWidget_->addNewTab(test2, tr("新建连接"));
      QObject::connect(test2, &FunctionSelectionWindow::switchRequested,
                       [this](TtProtocolRole::Role role) {
                         tabWidget_->handleButtonClicked(
                             tabWidget_->currentIndex(), role);
                       });
      tabWidget_->setCurrentWidget(test2);
    });
    connect(refreshBtn, &Ui::TtSvgButton::clicked, [this, controller]() {
      Ui::TtMessageBar::information(TtMessageBarType::Top, "无",
                                    tr("连接列表已刷新"), 1000, this);
    });
    test->addPairedWidget(0, linkList);
  }

  {
    QWidget* instructionList = new QWidget;
    instructionList->setObjectName("instructionList");
    auto instructionListLayout = new QGridLayout(instructionList);

    Ui::TtNormalLabel* title =
        new Ui::TtNormalLabel(tr("指令"), instructionList);

    QWidget* btnWidget = new QWidget(instructionList);
    Ui::TtHorizontalLayout* btnWidgetLayout =
        new Ui::TtHorizontalLayout(btnWidget);
    Ui::TtSvgButton* addBtn =
        new Ui::TtSvgButton(":/sys/plus-circle.svg", btnWidget);
    addBtn->setSvgSize(QSize(18, 18));
    Ui::TtSvgButton* refreshBtn =
        new Ui::TtSvgButton(":/sys/refresh-normal.svg", btnWidget);
    refreshBtn->setSvgSize(QSize(18, 18));
    refreshBtn->setColors(Qt::black, Qt::blue);
    refreshBtn->setEnableHoldToCheck(true);
    btnWidgetLayout->addWidget(addBtn);
    btnWidgetLayout->addSpacerItem(new QSpacerItem(5, 0));
    btnWidgetLayout->addWidget(refreshBtn);

    instructionListLayout->addWidget(title, 0, 0, Qt::AlignTop);
    instructionListLayout->addWidget(btnWidget, 0, 1, Qt::AlignRight);

    // Ui::TtNormalLabel* imla = new Ui::TtNormalLabel();
    // imla->setPixmap(QPixmap::fromImage(QImage(":/sys/tmp.png").scaled(88, 88)));
    // il->addStretch();
    // il->addWidget(imla, 0, Qt::AlignCenter);

    Ui::TtNormalLabel* displayWords = new Ui::TtNormalLabel(tr("指令列表未空"));
    auto addLinkBtn = new QPushButton(tr("新建指令"));
    addLinkBtn->resize(100, 28);

    auto original_widget_ = new QWidget();
    Ui::TtVerticalLayout* tmpl = new Ui::TtVerticalLayout(original_widget_);
    tmpl->addStretch();
    tmpl->addSpacerItem(new QSpacerItem(0, 20));
    tmpl->addWidget(displayWords, 0, Qt::AlignHCenter);
    tmpl->addWidget(addLinkBtn, 0, Qt::AlignHCenter);
    tmpl->addStretch();
    history_instruction_list_ = new Ui::SessionManager();
    history_instruction_list_->setSelectionMode(QAbstractItemView::NoSelection);
    auto stack_ = new QStackedWidget(instructionList);
    stack_->addWidget(original_widget_);
    stack_->addWidget(history_instruction_list_);
    instructionListLayout->addWidget(stack_, 1, 0, 1, 2);
    connect(history_instruction_list_->model(),
            &QAbstractItemModel::rowsInserted,
            [=](const QModelIndex& parent, int first, int last) {
              if (history_instruction_list_->count() == 1) {  // 从0变为1
                stack_->setCurrentIndex(1);
              }
            });
    connect(history_instruction_list_->model(),
            &QAbstractItemModel::rowsRemoved,
            [=](const QModelIndex& parent, int first, int last) {
              if (history_instruction_list_->count() == 0) {  // 从1变为0
                stack_->setCurrentIndex(0);
              }
            });
    connect(addLinkBtn, &QPushButton::clicked, [this, controller]() {
      Window::InstructionWindow* instruction = new Window::InstructionWindow;
      tabWidget_->addNewTab(instruction, tr("未命名的指令"));
      tabWidget_->setCurrentWidget(instruction);
    });
    connect(addBtn, &Ui::TtSvgButton::clicked, [this, controller]() {
      Window::InstructionWindow* instruction = new Window::InstructionWindow;
      tabWidget_->addNewTab(instruction, tr("未命名的指令"));
      tabWidget_->setCurrentWidget(instruction);
    });
    connect(refreshBtn, &Ui::TtSvgButton::clicked, [this, controller]() {
      Ui::TtMessageBar::information(TtMessageBarType::Top, "无",
                                    tr("连接列表已刷新"), 1000);
    });
    test->addPairedWidget(1, instructionList);
  }

  {
    QWidget* mockList = new QWidget;
    mockList->setObjectName("mockList");
    auto mockListLayout = new QGridLayout(mockList);

    Ui::TtNormalLabel* title = new Ui::TtNormalLabel(tr("模拟"), mockList);

    QWidget* btnWidget = new QWidget(mockList);
    Ui::TtHorizontalLayout* btnWidgetLayout =
        new Ui::TtHorizontalLayout(btnWidget);
    // // 以下两个会消失
    Ui::TtSvgButton* addBtn =
        new Ui::TtSvgButton(":/sys/plus-circle.svg", btnWidget);  // 刷新
    addBtn->setSvgSize(QSize(18, 18));
    Ui::TtSvgButton* refreshBtn =
        new Ui::TtSvgButton(":/sys/refresh-normal.svg", btnWidget);  // 新建
    refreshBtn->setSvgSize(QSize(18, 18));
    refreshBtn->setColors(Qt::black, Qt::blue);
    btnWidgetLayout->addWidget(addBtn);
    btnWidgetLayout->addSpacerItem(new QSpacerItem(5, 0));
    btnWidgetLayout->addWidget(refreshBtn);

    mockListLayout->addWidget(title, 0, 0, Qt::AlignTop);
    mockListLayout->addWidget(btnWidget, 0, 1, Qt::AlignRight);

    // 首页界面
    // 显示的图片
    Ui::TtNormalLabel* displayInfo = new Ui::TtNormalLabel();
    displayInfo->setPixmap(
        QPixmap::fromImage(QImage(":/sys/tmp.png").scaled(88, 88)));

    // 图片底下的文字
    Ui::TtNormalLabel* displayWords =
        new Ui::TtNormalLabel(tr("没有可用的模拟服务"));
    // 点击后在 tab 弹出一个 新 tab, 用于设置
    auto addLinkBtn = new QPushButton(tr("新建连接"));
    addLinkBtn->setMinimumSize(100, 28);

    // 创建原始界面
    auto original_widget_ = new QWidget();
    Ui::TtVerticalLayout* tmpl = new Ui::TtVerticalLayout(original_widget_);
    tmpl->addStretch();
    tmpl->addSpacerItem(new QSpacerItem(0, 20));
    tmpl->addWidget(displayWords, 0, Qt::AlignHCenter);
    tmpl->addWidget(addLinkBtn, 0, Qt::AlignHCenter);
    tmpl->addStretch();

    // 存储历史保存设置
    history_mock_list_ = new Ui::SessionManager();
    history_mock_list_->setSelectionMode(QAbstractItemView::NoSelection);

    // 使用堆叠布局
    auto stack_ = new QStackedWidget(mockList);
    stack_->addWidget(original_widget_);
    stack_->addWidget(history_mock_list_);

    mockListLayout->addWidget(stack_, 1, 0, 1, 2);

    // 监听 model 的行数变化
    connect(history_mock_list_->model(), &QAbstractItemModel::rowsInserted,
            [=](const QModelIndex& parent, int first, int last) {
              if (history_mock_list_->count() == 1) {  // 从0变为1
                stack_->setCurrentIndex(1);
              }
            });
    connect(history_mock_list_->model(), &QAbstractItemModel::rowsRemoved,
            [=](const QModelIndex& parent, int first, int last) {
              if (history_mock_list_->count() == 0) {  // 从1变为0
                stack_->setCurrentIndex(0);
              }
            });
    // 新增的弹出框
    connect(addLinkBtn, &QPushButton::clicked, [this, controller]() {
      // qDebug() << "add new tab";
      // FunctionSelectionWindow* test2 = new FunctionSelectionWindow();
      SimulateFunctionSelectionWindow* test2 =
          new SimulateFunctionSelectionWindow();
      tabWidget_->addNewTab(test2, tr("创建模拟器"));
      // 连接信号槽
      QObject::connect(test2, &SimulateFunctionSelectionWindow::switchRequested,
                       [this](TtProtocolRole::Role role) {
                         // 处理不同的 id
                         tabWidget_->handleButtonClicked(
                             tabWidget_->currentIndex(), role);
                       });
      // 切换到当前新增 tab
      tabWidget_->setCurrentWidget(test2);
    });
    // 新增的弹出框
    connect(addBtn, &Ui::TtSvgButton::clicked, [this, controller]() {
      // qDebug() << "add new tab";
      SimulateFunctionSelectionWindow* test2 =
          new SimulateFunctionSelectionWindow();
      tabWidget_->addNewTab(test2, tr("创建模拟器"));
      // 连接信号槽
      QObject::connect(test2, &SimulateFunctionSelectionWindow::switchRequested,
                       [this](TtProtocolRole::Role role) {
                         // 处理不同的 id
                         tabWidget_->handleButtonClicked(
                             tabWidget_->currentIndex(), role);
                       });
      // 切换到当前新增 tab
      tabWidget_->setCurrentWidget(test2);
    });

    test->addPairedWidget(2, mockList);
  }

  registerTabWidget();

  // 左边栏
  layout_->addWidget(left_bar_, 0, Qt::AlignLeft);

  layout_->addWidget(mainSplitter);

  connect(left_bar_logic_, &Ui::TtWidgetGroup::widgetClicked,
          [test, controller](int index) {
            // 打开
            if (controller->targetDrawerVisible()) {
              // 重复
              if (!test->switchToWidget(index)) {
                controller->closeDrawer();
              }
            } else {
              test->switchToWidget(index);
              controller->openDrawer();
            }
          });

  buttonGroup = new Ui::WidgetGroup(this);

  // 不是根据 index 切换, 而是特定的 uuid, 如果 tabwidget 不存在 uuid, 那么新建立 tab, 应当传递 uuid
  connect(buttonGroup, &Ui::WidgetGroup::currentIndexChanged, tabWidget_,
          &Ui::TabManager::switchToCurrentIndex);

  connectSignals();
}

MainWindow::~MainWindow() {}

void MainWindow::initLanguageMenu() {}

bool MainWindow::event(QEvent* event) {
  switch (event->type()) {
    case QEvent::WindowActivate: {
      auto menu = menuWidget();
      if (menu) {
        menu->setProperty("bar-active", true);
        style()->polish(menu);
      }
      break;
    }
    case QEvent::WindowDeactivate: {
      auto menu = menuWidget();
      if (menu) {
        menu->setProperty("bar-active", false);
        style()->polish(menu);
      }
      break;
    }
    case QEvent::Close: {
      // 保存设置
      Storage::SettingsManager::instance().saveSettings();
      break;
    }

    default:
      break;
  }
  return QMainWindow::event(event);
}

void MainWindow::installWindowAgent() {
  window_agent_->setup(this);

  // 2. Construct your title bar
  auto menuBar = [this]() {
    auto menuBar = new QMenuBar(this);

    // Virtual menu
    auto file = new QMenu(tr("File(&F)"), menuBar);
    file->addAction(new QAction(tr("New(&N)"), menuBar));
    file->addAction(new QAction(tr("Open(&O)"), menuBar));
    file->addSeparator();

    auto edit = new QMenu(tr("Edit(&E)"), menuBar);
    edit->addAction(new QAction(tr("Undo(&U)"), menuBar));
    edit->addAction(new QAction(tr("Redo(&R)"), menuBar));

    // Theme action
    // auto darkAction = new QAction(tr("Enable dark theme"), menuBar);
    // darkAction->setCheckable(true);

    // connect(darkAction, &QAction::triggered, this, [this](bool checked) {
    //     loadStyleSheet(checked ? Dark : Light); //
    // });
    // connect(this, &MainWindow::themeChanged, darkAction, [this, darkAction]() {
    //     darkAction->setChecked(currentTheme == Dark); //
    // });

#ifdef Q_OS_WIN
    // auto noneAction = new QAction(tr("None"), menuBar);
    // noneAction->setData(QStringLiteral("none"));
    // noneAction->setCheckable(true);
    // noneAction->setChecked(true);

    // auto dwmBlurAction = new QAction(tr("Enable DWM blur"), menuBar);
    // dwmBlurAction->setData(QStringLiteral("dwm-blur"));
    // dwmBlurAction->setCheckable(true);

    // auto acrylicAction = new QAction(tr("Enable acrylic material"), menuBar);
    // acrylicAction->setData(QStringLiteral("acrylic-material"));
    // acrylicAction->setCheckable(true);

    // auto micaAction = new QAction(tr("Enable mica"), menuBar);
    // micaAction->setData(QStringLiteral("mica"));
    // micaAction->setCheckable(true);

    // auto micaAltAction = new QAction(tr("Enable mica alt"), menuBar);
    // micaAltAction->setData(QStringLiteral("mica-alt"));
    // micaAltAction->setCheckable(true);

    // auto winStyleGroup = new QActionGroup(menuBar);
    // winStyleGroup->addAction(noneAction);
    // winStyleGroup->addAction(dwmBlurAction);
    // winStyleGroup->addAction(acrylicAction);
    // winStyleGroup->addAction(micaAction);
    // winStyleGroup->addAction(micaAltAction);
    // connect(winStyleGroup, &QActionGroup::triggered, this, [this, winStyleGroup](QAction *action) {
    //     // Unset all custom style attributes first, otherwise the style will not display correctly
    //     for (const QAction* _act : winStyleGroup->actions()) {
    //         const QString data = _act->data().toString();
    //         if (data.isEmpty() || data == QStringLiteral("none")) {
    //             continue;
    //         }
    //         window_agent_->setWindowAttribute(data, false);
    //     }
    //     const QString data = action->data().toString();
    //     if (data == QStringLiteral("none")) {
    //         setProperty("custom-style", false);
    //     } else if (!data.isEmpty()) {
    //         window_agent_->setWindowAttribute(data, true);
    //         setProperty("custom-style", true);
    //     }
    //     style()->polish(this);
    // });

#elif defined(Q_OS_MAC)
    auto darkBlurAction = new QAction(tr("Dark blur"), menuBar);
    darkBlurAction->setCheckable(true);
    connect(darkBlurAction, &QAction::toggled, this, [this](bool checked) {
      if (!windowAgent->setWindowAttribute(QStringLiteral("blur-effect"),
                                           "dark")) {
        return;
      }
      if (checked) {
        setProperty("custom-style", true);
        style()->polish(this);
      }
    });

    auto lightBlurAction = new QAction(tr("Light blur"), menuBar);
    lightBlurAction->setCheckable(true);
    connect(lightBlurAction, &QAction::toggled, this, [this](bool checked) {
      if (!windowAgent->setWindowAttribute(QStringLiteral("blur-effect"),
                                           "light")) {
        return;
      }
      if (checked) {
        setProperty("custom-style", true);
        style()->polish(this);
      }
    });

    auto noBlurAction = new QAction(tr("No blur"), menuBar);
    noBlurAction->setCheckable(true);
    connect(noBlurAction, &QAction::toggled, this, [this](bool checked) {
      if (!windowAgent->setWindowAttribute(QStringLiteral("blur-effect"),
                                           "none")) {
        return;
      }
      if (checked) {
        setProperty("custom-style", false);
        style()->polish(this);
      }
    });

    auto macStyleGroup = new QActionGroup(menuBar);
    macStyleGroup->addAction(darkBlurAction);
    macStyleGroup->addAction(lightBlurAction);
    macStyleGroup->addAction(noBlurAction);
#endif

    // Real menu
    // auto settings = new QMenu(tr("Settings(&S)"), menuBar);
    // settings->addAction(darkAction);

#ifdef Q_OS_WIN
    // settings->addSeparator();
    // settings->addAction(noneAction);
    // settings->addAction(dwmBlurAction);
    // settings->addAction(acrylicAction);
    // settings->addAction(micaAction);
    // settings->addAction(micaAltAction);
#elif defined(Q_OS_MAC)
    settings->addAction(darkBlurAction);
    settings->addAction(lightBlurAction);
    settings->addAction(noBlurAction);
#endif

    menuBar->addMenu(file);
    menuBar->addMenu(edit);
    // menuBar->addMenu(settings);
    return menuBar;
  }();

  menuBar->setObjectName(QStringLiteral("win-menu-bar"));

  auto titleLabel = new QLabel();
  titleLabel->setAlignment(Qt::AlignCenter);
  titleLabel->setObjectName(QStringLiteral("win-title-label"));

#ifndef Q_OS_MAC
  auto iconButton = new Ui::WindowbarButton();
  iconButton->setObjectName(QStringLiteral("icon-button"));
  iconButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

  auto minButton = new Ui::WindowbarButton();
  minButton->setObjectName(QStringLiteral("min-button"));
  minButton->setProperty("system-button", true);
  minButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

  auto maxButton = new Ui::WindowbarButton();
  maxButton->setCheckable(true);
  maxButton->setObjectName(QStringLiteral("max-button"));
  maxButton->setProperty("system-button", true);
  maxButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

  auto closeButton = new Ui::WindowbarButton();
  closeButton->setObjectName(QStringLiteral("close-button"));
  closeButton->setProperty("system-button", true);
  closeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
#endif

  auto windowBar = new Ui::WindowBar();
#ifndef Q_OS_MAC
  windowBar->setIconButton(iconButton);
  windowBar->setMinButton(minButton);
  windowBar->setMaxButton(maxButton);
  windowBar->setCloseButton(closeButton);
#endif
  windowBar->setMenuBar(menuBar);
  windowBar->setTitleLabel(titleLabel);
  windowBar->setHostWidget(this);

  window_agent_->setTitleBar(windowBar);
#ifndef Q_OS_MAC
  window_agent_->setSystemButton(QWK::WindowAgentBase::WindowIcon, iconButton);
  window_agent_->setSystemButton(QWK::WindowAgentBase::Minimize, minButton);
  window_agent_->setSystemButton(QWK::WindowAgentBase::Maximize, maxButton);
  window_agent_->setSystemButton(QWK::WindowAgentBase::Close, closeButton);
#endif
  window_agent_->setHitTestVisible(menuBar, true);

#ifdef Q_OS_MAC
  windowAgent->setSystemButtonAreaCallback([](const QSize& size) {
    static constexpr const int width = 75;
    return QRect(QPoint(size.width() - width, 0),
                 QSize(width, size.height()));  //
  });
#endif

  setMenuWidget(windowBar);

#ifndef Q_OS_MAC
  connect(iconButton, &QAbstractButton::clicked, window_agent_,
          [this, iconButton] {
            iconButton->setProperty("double-click-close", false);

            // WindowBar's icon show a menu to choose
            QTimer::singleShot(75, window_agent_, [this, iconButton]() {
              if (iconButton->property("double-click-close").toBool()) {
                return;
              }
              window_agent_->showSystemMenu(
                  iconButton->mapToGlobal(QPoint(0, iconButton->height())));
            });
          });

  connect(iconButton, &Ui::WindowbarButton::doubleClicked, this,
          [iconButton, this]() {
            iconButton->setProperty("double-click-close", true);
            close();
          });

  connect(windowBar, &Ui::WindowBar::minimizeRequested, this,
          &QWidget::showMinimized);
  connect(
      windowBar, &Ui::WindowBar::maximizeRequested, this,
      [this, maxButton](bool max) {
        if (max) {
          showMaximized();
        } else {
          showNormal();
        }

        // It's a Qt issue that if a QAbstractButton::clicked triggers a window's maximization,
        // the button remains to be hovered until the mouse move. As a result, we need to
        // manually send leave events to the button.
        emulateLeaveEvent(maxButton);
      });
  connect(windowBar, &Ui::WindowBar::closeRequested, this, &QWidget::close);
#endif
}

void MainWindow::loadStyleSheet(Theme theme) {

#if 0
  //QString AppDir = qApp->applicationDirPath();
  QString AppDir("F:/MyProject/SerialportTool/out/build");
  //qDebug() << AppDir;

  acss::QtAdvancedStylesheet AdvancedStylesheet;

  // first set the directory that contains all your styles
  AdvancedStylesheet.setStylesDirPath(AppDir +
                                      "/../../SerialportTool/res/styles");
  //qDebug() << AdvancedStylesheet.currentStylePath();

  // now set the output folder where the processed styles are stored. The
  // style manager will create a sub directory for each style
  AdvancedStylesheet.setOutputDirPath(AppDir + "/output");

  // set the current style and select a theme. After these two calls, the output
  // folder will contains the generated resoruces and stylesheet.
  // AdvancedStylesheet.setCurrentStyle("qt_material");
  AdvancedStylesheet.setCurrentStyle("modify_qt_material");
  AdvancedStylesheet.setCurrentTheme("dark_teal");

  AdvancedStylesheet.updateStylesheet();
  
  qApp->setStyleSheet(AdvancedStylesheet.styleSheet());
#endif

  // now you can set the generated stylesheet
  //     if (!styleSheet().isEmpty() && theme == currentTheme)
  //         return;
  //     currentTheme = theme;

  //     if (QFile qss(theme == Dark ? QStringLiteral(":/dark-style.qss")
  //                                 : QStringLiteral(":/light-style.qss"));
  //         qss.open(QIODevice::ReadOnly | QIODevice::Text)) {
  //         setStyleSheet(QString::fromUtf8(qss.readAll()));
  // Q_EMIT themeChanged();
  // emit themeChanged();
  //     }
}

void MainWindow::setLeftBar() {
  left_bar_ = new QWidget(this);
  left_bar_logic_ = new Ui::TtWidgetGroup(this);
  left_bar_logic_->setExclusive(true);
  communication_connection =
      new Ui::TtSvgButton(":/sys/communicating-junctions.svg", left_bar_);
  communication_connection->setColors(Qt::black, Qt::blue);
  communication_connection->setEnableHoldToCheck(false);

  communication_instruction =
      new Ui::TtSvgButton(":/sys/Instruction-configuration.svg", left_bar_);
  communication_instruction->setColors(Qt::black, Qt::blue);
  communication_instruction->setEnableHoldToCheck(false);

  realistic_simulation =
      new Ui::TtSvgButton(":/sys/real-time-simulation.svg", left_bar_);
  realistic_simulation->setColors(Qt::black, Qt::blue);
  realistic_simulation->setEnableHoldToCheck(false);

  // communication_connection->setStyleSheet("padding: 5px 10px;");
  // communication_instruction->setStyleSheet("padding: 5px 10px;");

  left_bar_logic_->addWidget(communication_connection);
  left_bar_logic_->addWidget(communication_instruction);
  left_bar_logic_->addWidget(realistic_simulation);

  Ui::TtVerticalLayout* left_bar_layout = new Ui::TtVerticalLayout(left_bar_);

  // 添加按钮
  left_bar_layout->addSpacerItem(new QSpacerItem(0, 10));
  left_bar_layout->addWidget(communication_connection, 0, Qt::AlignTop);
  left_bar_layout->addSpacerItem(new QSpacerItem(0, 20));
  left_bar_layout->addWidget(communication_instruction, 0, Qt::AlignTop);
  left_bar_layout->addSpacerItem(new QSpacerItem(0, 20));
  left_bar_layout->addWidget(realistic_simulation, 0, Qt::AlignTop);
  left_bar_layout->addStretch();

  left_bar_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  left_bar_->setMinimumWidth(40);
}

void MainWindow::connectSignals() {
  //  Ui::SnackBarController::instance()->showMessage("这是一个测试消息", 2000);
  //});
  // 处理点击 close Pop 时, 按钮恢复, 是否需要切换到新的 tab
  //connect(communication_connection_widget, &Ui::PopWidget::isClosed, [this]() {
  //  communication_connection->setState(true);
  //  qDebug() << "also close";
  //});

  connect(history_link_list_, &Ui::SessionManager::uuidsChanged, buttonGroup,
          &Ui::WidgetGroup::updateUuid);
  connect(history_link_list_, &Ui::SessionManager::uuidsChanged, tabWidget_,
          &Ui::TabManager::removeUuidWidget);

  connect(history_mock_list_, &Ui::SessionManager::uuidsChanged, buttonGroup,
          &Ui::WidgetGroup::updateUuid);
  connect(history_mock_list_, &Ui::SessionManager::uuidsChanged, tabWidget_,
          &Ui::TabManager::removeUuidWidget);

  // 点击按钮信号连接
  QObject::connect(function_select_, &FunctionSelectionWindow::switchRequested,
                   [this](TtProtocolRole::Role role) {
                     tabWidget_->handleButtonClicked(tabWidget_->currentIndex(),
                                                     role);
                   });
}

void MainWindow::registerTabWidget() {
  // 注册 串口窗口
  tabWidget_->registerWidget(
      TtProtocolRole::Serial,
      [this]() {
        //base::DetectRunningTime runtime;

        //auto widget = new QWidget();
        //Ui::TtVerticalLayout* layout = new Ui::TtVerticalLayout(widget);

        //// 创建并显示加载动画
        //QLabel* loadingLabel = new QLabel(widget);
        //QMovie* loadingMovie = new QMovie(":/sys/loading.gif");
        //loadingLabel->setMovie(loadingMovie);
        //layout->addWidget(loadingLabel);
        //loadingMovie->start();

        //// 使用 QtConcurrent::run 在后台线程中创建 SerialWindow
        //QFuture<Window::SerialWindow*> future =
        //    QtConcurrent::run([this]() { return new Window::SerialWindow(); });
        //// 使用 QFutureWatcher 监听线程完成
        //QFutureWatcher<Window::SerialWindow*>* watcher =
        //    new QFutureWatcher<Window::SerialWindow*>(this);

        //connect(watcher, &QFutureWatcher<Window::SerialWindow*>::finished,
        //        [this, watcher, layout, loadingLabel, loadingMovie]() {
        //          Window::SerialWindow* d = watcher->result();
        //          connect(d, &Window::SerialWindow::requestSaveConfig,
        //                  [this, d]() {
        //                    Storage::SettingsManager::instance().setSetting(
        //                        "Serial", d->getConfiguration());
        //                    // 向 ListWidget 添加 button
        //                    addDifferentConfiguration(1);
        //                    qDebug() << "save";
        //                    // 只有关闭整个 app 的时候才会保存到本地
        //                  });

        //          // 移除加载动画并添加 SerialWindow
        //          layout->removeWidget(loadingLabel);
        //          loadingMovie->stop();
        //          loadingLabel->deleteLater();
        //          layout->addWidget(d);

        //          qDebug() << "Create SerialWindow finished";
        //          watcher->deleteLater();
        //        });
        //watcher->setFuture(future);

        //qDebug() << "Create SerialWindow: " << runtime.elapseMilliseconds();
        //return widget;

        Window::SerialWindow* serial = new Window::SerialWindow();
        //qDebug() << "Create SerialWindow: " << runtime.elapseMilliseconds();
        connect(
            serial, &Window::SerialWindow::requestSaveConfig, [this, serial]() {

              // 获取当前窗口的独特值, 是否已经保存到 listwidget,
              addDifferentConfiguration(TtFunctionalCategory::Communication,
                                        serial->getTitle(),
                                        tabWidget_->getCurrentWidgetUUid());
              tabWidget_->setTabTitle(serial->getTitle());
              // 只有关闭整个 app 的时候才会保存到本地
              // 如果存在多个串口配置
              Storage::SettingsManager::instance().setSetting(
                  "Serial" + QString::number(tabWidget_->SpecialTypeNums(
                                 TtProtocolRole::Serial)),
                  serial->getConfiguration());
              Ui::TtMessageBar::success(TtMessageBarType::Top, "",
                                        tr("保存成功"), 1000);
            });
        return serial;
      },
      tr("未命名的串口连接"));
  tabWidget_->registerWidget(
      TtProtocolRole::TcpClient,
      [this]() {
        Window::TcpWindow* tcpClient =
            new Window::TcpWindow(TtProtocolType::Client);
        connect(tcpClient, &Window::TcpWindow::requestSaveConfig,
                [this, tcpClient]() {
                  addDifferentConfiguration(TtFunctionalCategory::Communication,
                                            tcpClient->getTitle(),
                                            tabWidget_->getCurrentWidgetUUid());
                  tabWidget_->setTabTitle(tcpClient->getTitle());
                  Storage::SettingsManager::instance().setSetting(
                      "TcpClient" + QString::number(tabWidget_->SpecialTypeNums(
                                        TtProtocolRole::TcpClient)),
                      tcpClient->getConfiguration());
                  Ui::TtMessageBar::success(TtMessageBarType::Top, "",
                                            tr("保存成功"), 1000);
                });
        return tcpClient;
      },
      tr("未命 TCP 客户端连接"));
  tabWidget_->registerWidget(
      TtProtocolRole::UdpClient,
      [this]() {
        Window::UdpWindow* udpClient =
            new Window::UdpWindow(TtProtocolType::Client);
        connect(udpClient, &Window::UdpWindow::requestSaveConfig,
                [this, udpClient]() {
                  addDifferentConfiguration(TtFunctionalCategory::Communication,
                                            udpClient->getTitle(),
                                            tabWidget_->getCurrentWidgetUUid());
                  tabWidget_->setTabTitle(udpClient->getTitle());
                  Storage::SettingsManager::instance().setSetting(
                      "UdpClient" + QString::number(tabWidget_->SpecialTypeNums(
                                        TtProtocolRole::UdpClient)),
                      udpClient->getConfiguration());
                  Ui::TtMessageBar::success(TtMessageBarType::Top, "",
                                            tr("保存成功"), 1000);
                });
        return udpClient;
      },
      tr("未命名 UDP 连接"));
  tabWidget_->registerWidget(
      TtProtocolRole::MqttClient,
      [this]() {
        Window::MqttWindow* mqttClient =
            new Window::MqttWindow(TtProtocolType::Client);
        connect(
            mqttClient, &Window::MqttWindow::requestSaveConfig,
            [this, mqttClient]() {
              addDifferentConfiguration(TtFunctionalCategory::Communication,
                                        mqttClient->getTitle(),
                                        tabWidget_->getCurrentWidgetUUid());
              tabWidget_->setTabTitle(mqttClient->getTitle());
              Storage::SettingsManager::instance().setSetting(
                  "MqttClient" + QString::number(tabWidget_->SpecialTypeNums(
                                     TtProtocolRole::MqttClient)),
                  mqttClient->getConfiguration());
              Ui::TtMessageBar::success(TtMessageBarType::Top, "",
                                        tr("保存成功"), 1000, this);
            });
        return mqttClient;
      },
      tr("未命名的 MQTT 客户端"));
  tabWidget_->registerWidget(
      TtProtocolRole::ModbusClient,
      [this]() {
        Window::ModbusWindow* modbusClient =
            new Window::ModbusWindow(TtProtocolType::Client);
        connect(
            modbusClient, &Window::ModbusWindow::requestSaveConfig,
            [this, modbusClient]() {
              addDifferentConfiguration(TtFunctionalCategory::Communication,
                                        modbusClient->getTitle(),
                                        tabWidget_->getCurrentWidgetUUid());
              tabWidget_->setTabTitle(modbusClient->getTitle());
              Storage::SettingsManager::instance().setSetting(
                  "ModbusClient" + QString::number(tabWidget_->SpecialTypeNums(
                                       TtProtocolRole::ModbusClient)),
                  modbusClient->getConfiguration());
              Ui::TtMessageBar::success(TtMessageBarType::Top, "",
                                        tr("保存成功"), 1000);
            });
        return modbusClient;
      },
      tr("未命名的 Modbus 主机"));
  tabWidget_->registerWidget(
      TtProtocolRole::TcpServer,
      [this]() {
        Window::TcpWindow* tcpServer =
            new Window::TcpWindow(TtProtocolType::Server);
        connect(tcpServer, &Window::TcpWindow::requestSaveConfig,
                [this, tcpServer]() {
                  addDifferentConfiguration(TtFunctionalCategory::Simulate,
                                            tcpServer->getTitle(),
                                            tabWidget_->getCurrentWidgetUUid());
                  tabWidget_->setTabTitle(tcpServer->getTitle());
                  Storage::SettingsManager::instance().setSetting(
                      "TcpServer" + QString::number(tabWidget_->SpecialTypeNums(
                                        TtProtocolRole::TcpServer)),
                      tcpServer->getConfiguration());
                  Ui::TtMessageBar::success(TtMessageBarType::Top, "",
                                            tr("保存成功"), 1000);
                });
        return tcpServer;
      },
      tr("未命名的 TCP 服务模拟端"));

  // 注册 UDP 服务端窗口
  tabWidget_->registerWidget(
      TtProtocolRole::UdpServer,
      [this]() {
        Window::UdpWindow* udpServer =
            new Window::UdpWindow(TtProtocolType::Server);
        connect(udpServer, &Window::UdpWindow::requestSaveConfig,
                [this, udpServer]() {
                  addDifferentConfiguration(TtFunctionalCategory::Simulate,
                                            udpServer->getTitle(),
                                            tabWidget_->getCurrentWidgetUUid());
                  tabWidget_->setTabTitle(udpServer->getTitle());
                  Storage::SettingsManager::instance().setSetting(
                      "UdpServer" + QString::number(tabWidget_->SpecialTypeNums(
                                        TtProtocolRole::UdpServer)),
                      udpServer->getConfiguration());
                  Ui::TtMessageBar::success(TtMessageBarType::Top, "",
                                            tr("保存成功"), 1000, this);
                });
        return udpServer;
      },
      tr("未命名的 UDP 服务模拟端"));
  // tabWidget_->registerWidget(
  //     TtProtocolRole::MqttBroker,
  //     [this]() {
  //       Window::MqttWindow* mqttServer =
  //           new Window::MqttWindow(TtProtocolType::Server);
  //       connect(
  //           mqttServer, &Window::MqttWindow::requestSaveConfig,
  //           [this, mqttServer]() {
  //             addDifferentConfiguration(TtFunctionalCategory::Simulate,
  //                                       mqttServer->getTitle(),
  //                                       tabWidget_->getCurrentWidgetUUid());
  //             tabWidget_->setTabTitle(mqttServer->getTitle());
  //             Storage::SettingsManager::instance().setSetting(
  //                 "MqttServer" + QString::number(tabWidget_->SpecialTypeNums(
  //                                    TtProtocolRole::MqttBroker)),
  //                 mqttServer->getConfiguration());
  //             Ui::TtMessageBar::success(TtMessageBarType::Top, "",
  //                                       tr("保存成功"), 1000, this);
  //           });
  //       return mqttServer;
  //     },
  //     tr("未命名的 MQTT 服务模拟端"));
}

void MainWindow::addDifferentConfiguration(TtFunctionalCategory::Category type,
                                           const QString& title,
                                           const QString& uuid) {
  Ui::TtSpecialDeleteButton* button = new Ui::TtSpecialDeleteButton(
      title, ":/sys/displayport.svg", ":/sys/delete.svg", this);

  switch (type) {
    case TtFunctionalCategory::Communication: {
      history_link_list_->addAdaptiveWidget(title, uuid, button);
      break;
    }
    case TtFunctionalCategory::Instruction: {
      break;
    }
    case TtFunctionalCategory::Simulate: {
      history_mock_list_->addAdaptiveWidget(title, uuid, button);
      break;
    }
    default:
      break;
  }
  buttonGroup->addButton(uuid, button);

}

}  // namespace Window
