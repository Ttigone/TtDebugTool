#include "ui/controls/TtLuaInputBox.h"

#include <ui/control/TtListView.h>
#include <ui/control/buttonbox/TtButtonBox.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/session_manager.h>

#include <QsciAPIs.h>
#include <QsciLexerLua.h>

namespace Ui {

TtLuaInputBox::TtLuaInputBox(QWidget* parent) : QDialog(parent) {
  init();
}

TtLuaInputBox::~TtLuaInputBox() {
  qDebug() << "delete inpubot";
}

QString TtLuaInputBox::getLuaCode() {
  // return edit_lua_code_->text();
  return edit_lua_code_->text();
}

void TtLuaInputBox::init() {
  // 显示的编辑组件
  edit_lua_code_ = new QsciScintilla;
  edit_lua_code_->setMarginType(0, QsciScintilla::NumberMargin);

  // QFont font("Consolas", 10);
  // font.setFixedPitch(true);  // 确保使用等宽字体
  // edit_lua_code_->setFont(font);
  // edit_lua_code_->setMarginsFont(font);
  // edit_lua_code_->setMarginWidth(0, "9999");

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

  // Ui::TtTextButton* canclebutton = new Ui::TtTextButton(tr("取消"), this);
  // canclebutton->setCheckedColor(Qt::cyan);
  // Ui::TtTextButton* savebutton = new Ui::TtTextButton(tr("保存"), this);
  // savebutton->setCheckedColor(Qt::cyan);

  QGridLayout* layout = new QGridLayout(this);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);

  QListWidget* addFunctionList = new QListWidget;
  QListWidgetItem* item = new QListWidgetItem();
  // QPushButton* button = new QPushButton("Click Me");

  // 显示获取值并改变函数
  FancyButton* getValueButton = new FancyButton("Click Me");
  connect(getValueButton, &QPushButton::clicked, this, [this] {
    // 获取函数文本
    // edit_lua_code_->append("");
    QString functionTemplate =
        "function getValue(value)\n"
        "    return value + 1\n"
        "end\n";

    // 在当前光标位置插入函数模板
    if (edit_lua_code_) {
      int line, index;
      edit_lua_code_->getCursorPosition(&line, &index);
      edit_lua_code_->insertAt(functionTemplate, line, index);

      // 可选：将光标移动到函数体内部
      edit_lua_code_->setCursorPosition(line + 1, 4);
    }
  });

  item->setSizeHint(getValueButton->sizeHint());
  addFunctionList->addItem(item);
  addFunctionList->setItemWidget(item, getValueButton);

  // QHBoxLayout* buttonLayout = new QHBoxLayout();
  // buttonLayout->addStretch();
  // buttonLayout->addWidget(canclebutton, 0, Qt::AlignRight);
  // buttonLayout->addWidget(savebutton, 0, Qt::AlignRight);

  layout->addWidget(edit_lua_code_, 0, 0, 2, 1);
  layout->addWidget(addFunctionList, 0, 1, 2, 1);

  setLayout(layout);

  // WidgetGroup* buttonGroup = new WidgetGroup(this);

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
  // connect(buttonGroup, &WidgetGroup::currentIndexChanged, this,
  //         [this](const QString& index) {
  //           // 添加不同的 lua 代码到编辑器上
  //           edit_lua_code_->setText(lua_code_.value(index));
  //         });

  // 连接到适当的信号
  connect(edit_lua_code_, &QsciScintilla::textChanged, this,
          &TtLuaInputBox::updateLineNumberWidth);
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

}  // namespace Ui
