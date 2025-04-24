#include "ui/controls/TtLuaInputBox.h"

#include <ui/control/TtListView.h>
#include <ui/control/buttonbox/TtButtonBox.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/session_manager.h>

#include <QsciAPIs.h>
#include <QsciLexerLua.h>

namespace Ui {

TtLuaInputBox::TtLuaInputBox(bool enableSaveSetting, QWidget* parent)
    : QDialog(parent), save_setting_(enableSaveSetting) {
  init();
}

TtLuaInputBox::~TtLuaInputBox() {
  // 每次关闭后, 栈模态都会关闭
  qDebug() << "delete inpubot";
}

void TtLuaInputBox::setLuaCode(const QString& code) {
  edit_lua_code_->append(code);
  // edit_lua_code_->setCursorPosition();
  edit_lua_code_->SendScintilla(QsciScintilla::SCI_DOCUMENTEND);
}

QString TtLuaInputBox::getLuaCode() const {
  return edit_lua_code_->text();
}

// void TtLuaInputBox::setEnableSaveSetting(bool enable) {
//   save_setting_ = enable;
// }

void TtLuaInputBox::applyChanges() {
  // 点击取消, 然后 ChanDialog 点击确定后, 消失

  qDebug() << "apply";
  // [this, script_listview, buttonGroup] {
  //           Ui::TtSpecialDeleteButton* button = new Ui::TtSpecialDeleteButton(
  //               "TEST", ":/sys/displayport.svg", ":/sys/delete.svg", this);
  //           auto uuid = QUuid::createUuid().toString();

  //           script_listview->addAdaptiveWidget("TEST", uuid, button);
  //           buttonGroup->addButton(uuid, button);
  //           // 存储
  //           lua_code_[uuid] = edit_lua_code_->text();
  // if (edit_lua_code_->text().isEmpty()) {
  //   accept();
  // } else {

  //   // 返回 QString code 的代码给 SerialWindow 保存
  // }
  accept();
}

void TtLuaInputBox::init() {
  setWindowTitle(tr("Lua 代码编辑器"));
  resize(400, 280);
  // 显示的编辑组件
  edit_lua_code_ = new QsciScintilla;
  edit_lua_code_->setMarginType(0, QsciScintilla::NumberMargin);

  QsciLexerLua* luaLexer = new QsciLexerLua(edit_lua_code_);
  // 设置关键字颜色等
  luaLexer->setColor(QColor("#0000ff"), QsciLexerLua::Keyword);
  luaLexer->setColor(QColor("#008800"), QsciLexerLua::Comment);
  luaLexer->setColor(QColor("#880000"), QsciLexerLua::String);
  edit_lua_code_->setLexer(luaLexer);
  edit_lua_code_->setFrameStyle(QFrame::NoFrame);

  // 应用词法分析器到编辑器
  edit_lua_code_->setLexer(luaLexer);
  // 创建 API 对象用于代码提示
  QsciAPIs* apis = new QsciAPIs(luaLexer);
  addLuaApis(apis);
  enhanceCompletion(edit_lua_code_);

  // 主布局
  // QGridLayout* layout = new QGridLayout(this);
  Ui::TtHorizontalLayout* layout = new Ui::TtHorizontalLayout(this);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);

  QListWidget* addFunctionList = new QListWidget;
  addFunctionList->setContentsMargins(QMargins());
  addFunctionList->setSpacing(0);
  addFunctionList->setStyleSheet(
      // 设置列表整体背景色
      "QListWidget {"
      "   background-color: white;"
      "   outline: none;"  // 移除焦点虚线框（关键）
      "}"
      // 项选中状态样式
      "QListWidget::item:selected {"
      "   background: transparent;"  // 强制透明背景
      "}"
      // 项悬停状态样式
      "QListWidget::item:hover {"
      "   background: transparent;"  // 防止悬停变色
      "}"
      // 项按下状态样式
      "QListWidget::item:pressed {"
      "   background: transparent;"  // 防止按压变色
      "}");

  // addFunctionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  // addFunctionList->setFixedWidth(200);

  QListWidgetItem* item = new QListWidgetItem(addFunctionList);
  item->setSizeHint(QSize(128, 30));

  QHBoxLayout* buttonLayout = new QHBoxLayout();

  TtCodeButton* codeButton = new TtCodeButton(
      tr("写入数据之后"), ":/sys/code-square.svg", addFunctionList);
  // codeButton->setText(tr("写入数据之后"));
  addFunctionList->setItemWidget(item, codeButton);
  connect(codeButton, &TtCodeButton::clicked, this, [this] {
    // 获取函数文本
    QString functionTemplate =
        "function getValue(value)\n"
        "    return value + 1\n"
        "end\n";

    appendCodeToEnd(edit_lua_code_, functionTemplate);
  });

  layout->addWidget(edit_lua_code_, 3);

  Ui::TtVerticalLayout* vl = new Ui::TtVerticalLayout();
  vl->addWidget(addFunctionList);

  if (save_setting_) {
    Ui::TtTextButton* canclebutton = new Ui::TtTextButton(tr("取消"), this);
    canclebutton->setCheckedColor(Qt::cyan);
    Ui::TtTextButton* savebutton = new Ui::TtTextButton(tr("保存"), this);
    savebutton->setCheckedColor(Qt::cyan);

    buttonLayout->addWidget(canclebutton);
    buttonLayout->addWidget(savebutton);

    vl->addLayout(buttonLayout, 0);

    layout->addLayout(vl, 1);

    connect(canclebutton, &QPushButton::clicked, this, &TtLuaInputBox::reject);
    connect(savebutton, &QPushButton::clicked, this,
            &TtLuaInputBox::applyChanges);
  }

  setLayout(layout);

  // connect(savebutton, &Ui::TtTextButton::clicked, this,
  //         [this, script_listview, buttonGroup] {
  //           Ui::TtSpecialDeleteButton* button = new Ui::TtSpecialDeleteButton(
  //               "TEST", ":/sys/displayport.svg", ":/sys/delete.svg", this);
  //           auto uuid = QUuid::createUuid().toString();

  //           script_listview->addAdaptiveWidget("TEST", uuid, button);
  //           buttonGroup->addButton(uuid, button);
  //           // 存储
  //           lua_code_[uuid] = edit_lua_code_->text();
  //         });

  // // 连接到适当的信号
  // connect(edit_lua_code_, &QsciScintilla::textChanged, this,
  //         &TtLuaInputBox::updateLineNumberWidth);
}

void TtLuaInputBox::addLuaApis(QsciAPIs* apis) {
  // 基本函数
  apis->add("print(value1, value2, ...) -- 打印变量");
  apis->add("tonumber(e [, base]) -- 将e转换为数字");
  apis->add("tostring(e) -- 将e转换为字符串");
  apis->add("type(v) -- 返回v的类型名");

  // 字符串库
  apis->add("string.byte(s [, i [, j]]) -- 返回字符的内部数字表示");
  apis->add("string.char(n1, n2, ...) -- 返回字符的内部表示");
  apis->add("string.dump(function) -- 返回函数的二进制表示");
  apis->add("string.find(s, pattern [, init [, plain]]) -- 查找模式匹配");
  apis->add("string.format(formatstring, ...) -- 格式化字符串");
  apis->add("string.gmatch(s, pattern) -- 返回一个迭代器函数");
  apis->add("string.gsub(s, pattern, repl [, n]) -- 替换字符串");
  apis->add("string.len(s) -- 返回字符串长度");
  apis->add("string.lower(s) -- 转换为小写");
  apis->add("string.match(s, pattern [, init]) -- 匹配模式");
  apis->add("string.rep(s, n [, sep]) -- 重复字符串");
  apis->add("string.reverse(s) -- 反转字符串");
  apis->add("string.sub(s, i [, j]) -- 提取子字符串");
  apis->add("string.upper(s) -- 转换为大写");

  // 表库
  apis->add("table.concat(table [, sep [, i [, j]]]) -- 连接表元素");
  apis->add("table.insert(table, [pos,] value) -- 插入元素");
  apis->add("table.pack(...) -- 创建并返回一个新表");
  apis->add("table.remove(table [, pos]) -- 移除元素");
  apis->add("table.sort(table [, comp]) -- 排序");
  apis->add("table.unpack(list [, i [, j]]) -- 返回列表的元素");

  // 数学库
  apis->add("math.abs(x) -- 绝对值");
  apis->add("math.acos(x) -- 反余弦");
  apis->add("math.asin(x) -- 反正弦");
  apis->add("math.atan(x) -- 反正切");
  apis->add("math.ceil(x) -- 向上取整");
  apis->add("math.cos(x) -- 余弦");
  apis->add("math.deg(x) -- 角度转弧度");
  apis->add("math.exp(x) -- e的x次方");
  apis->add("math.floor(x) -- 向下取整");
  apis->add("math.fmod(x, y) -- 浮点数取模");
  apis->add("math.log(x [, base]) -- 对数");
  apis->add("math.max(x, ...) -- 最大值");
  apis->add("math.min(x, ...) -- 最小值");
  apis->add("math.pi -- π值");
  apis->add("math.rad(x) -- 弧度转角度");
  apis->add("math.random([m [, n]]) -- 随机数");
  apis->add("math.randomseed(x) -- 设置随机数种子");
  apis->add("math.sin(x) -- 正弦");
  apis->add("math.sqrt(x) -- 平方根");
  apis->add("math.tan(x) -- 正切");

  // 关键字
  apis->add("and -- 逻辑与操作符");
  apis->add("break -- 跳出循环");
  apis->add("do -- 开始代码块");
  apis->add("else -- if语句的else部分");
  apis->add("elseif -- if语句的elseif部分");
  apis->add("end -- 结束代码块");
  apis->add("false -- 布尔值false");
  apis->add("for -- 循环");
  apis->add("function -- 函数定义");
  apis->add("if -- 条件语句");
  apis->add("in -- for循环中使用");
  apis->add("local -- 局部变量");
  apis->add("nil -- 空值");
  apis->add("not -- 逻辑非操作符");
  apis->add("or -- 逻辑或操作符");
  apis->add("repeat -- 循环");
  apis->add("return -- 从函数返回");
  apis->add("then -- if语句的then部分");
  apis->add("true -- 布尔值true");
  apis->add("until -- repeat循环的条件");
  apis->add("while -- 循环");

  // 确保准备API
  apis->prepare();
}

void TtLuaInputBox::enhanceCompletion(QsciScintilla* edit_lua_code_) {
  // 设置更低的触发阈值
  edit_lua_code_->setAutoCompletionThreshold(1);  // 输入1个字符就触发

  // 启用所有来源的自动完成
  edit_lua_code_->setAutoCompletionSource(QsciScintilla::AcsAll);

  // 设置自动完成选项
  edit_lua_code_->setAutoCompletionCaseSensitivity(false);  // 不区分大小写
  edit_lua_code_->setAutoCompletionReplaceWord(true);  // 替换当前单词
  edit_lua_code_->setAutoCompletionShowSingle(true);  // 只有一个选项时也显示

  // 设置自动完成区域的视觉样式
  edit_lua_code_->setAutoCompletionFillupsEnabled(false);  // 禁用自动填充

  // 设置函数提示
  edit_lua_code_->setCallTipsStyle(QsciScintilla::CallTipsNoContext);
  edit_lua_code_->setCallTipsVisible(-1);  // 永久显示，直到用户关闭
  edit_lua_code_->setCallTipsPosition(QsciScintilla::CallTipsBelowText);
  edit_lua_code_->setCallTipsBackgroundColor(QColor("#FFF8DC"));  // 米色背景
  edit_lua_code_->setCallTipsForegroundColor(QColor("#000000"));  // 黑色文本
  edit_lua_code_->setCallTipsHighlightColor(QColor("#0000FF"));  // 蓝色高亮
}

void TtLuaInputBox::updateLineNumberWidth() {
  // 获取当前行数
  int lines = edit_lua_code_->lines();

  // 计算所需的数字位数
  int digits = 1;
  int max = 10;
  while (lines >= max) {
    max *= 10;
    ++digits;
  }

  // 计算像素宽度
  QFontMetrics metrics(edit_lua_code_->font());
  int digitWidth = metrics.horizontalAdvance("9");  // 通常9是最宽的数字
  int pixelWidth = digitWidth * (digits + 1) + 10;  // 额外加一些空间

  edit_lua_code_->setMarginWidth(0, pixelWidth);
}

void TtLuaInputBox::appendCodeToEnd(QsciScintilla* editor,
                                    const QString& code) {
  // 获取当前文档的最后一行
  int lastLine = editor->lines() - 1;

  // 确定最后一行的长度(如果需要在最后添加新行)
  int lastLineLength = editor->lineLength(lastLine);
  bool needNewLine = lastLineLength > 0 &&
                     editor->text(lastLine).at(lastLineLength - 1) != '\n';

  // 构建要插入的文本，根据需要添加换行符
  QString textToInsert = (needNewLine ? "\n" : "") + code;

  // 移动到最后一行末尾
  editor->setCursorPosition(lastLine, lastLineLength);

  // 插入代码
  editor->insert(textToInsert);

  // 滚动到插入位置
  editor->ensureCursorVisible();
}

}  // namespace Ui
