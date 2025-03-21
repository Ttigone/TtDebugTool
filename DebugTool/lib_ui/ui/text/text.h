/*****************************************************************//**
 * \file   text.h
 * \brief  文本
 * 
 * \author C3H3_Ttigone
 * \date   August 2024
 *********************************************************************/

#ifndef UI_TEXT_TEXT_H
#define UI_TEXT_TEXT_H

#include "ui/click_handler.h"
#include "ui/style/style_core_types.h"
#include "ui/effects/animation_value.h"

#include <any>
#include "base/flags.h"



/// @brief 文本解析参数
struct TextParseOptions
{
  int32 flags;
  int32 maxw;
  int32 maxh;
  Qt::LayoutDirection dir;
};

// 默认参数
extern const TextParseOptions TDefaultTextOptions;
// 标记符号
extern const TextParseOptions TMarkupTextOptions;
// 纯文本
extern const TextParseOptions TPlainTextOptions;


enum class TextSelectType {
	Letters = 0x01,         // 字母
  Words = 0x02,           // 单词
  Paragraphs = 0x03,      // 段
};

struct TextSelection {
  constexpr TextSelection() = default;
  constexpr TextSelection(uint16 from, uint16 to) : from_(from), to_(to) {}
  bool empty() const { return from_ == to_; }

  uint16 from_ = 0;
  uint16 to_ = 0;
};


inline bool operator==(TextSelection a, TextSelection b) {
  return a.from_ == b.from_ && a.to_ == b.to_;
}

inline bool operator!=(TextSelection a, TextSelection b) { return !(a == b); }


static constexpr TextSelection AllTextSelection = {0, 0xFFFF};

namespace Ui {

inline constexpr auto TFixMax = INT_MAX / 256;

namespace Text {



struct LineGeometry
{
  int left = 0;
  int width = 0;
  bool elided = false;
};

struct GeometryDescriptor {
  std::function<LineGeometry(int line)> layout;
  bool breakEverywhere = false;
  bool *outElided = nullptr;
};


/// @brief
struct StateResult {
  ClickHandlerPtr link;  // 连接
  bool uponSymbol = false;
  bool afterSymbol = false;
  uint16 symbol = 0;  // 标志
};

struct StateRequest {
  enum class Flag {
    BreakEverywhere = (1 << 0),
    LookupSymbol = (1 << 1),
    LookupLink = (1 << 2),
    LookupCustomTooltip = (1 << 3),
  };
  using Flags = base::flags<Flag>;
  friend inline constexpr auto is_flag_type(Flag) { return true; };

  StateRequest() {}

  style::align align = style::al_left;
  Flags flags = Flag::LookupLink;
};

struct StateRequestElided : StateRequest
{
  StateRequestElided()
  {
	  
  }
  StateRequestElided(const StateRequest &request) : StateRequest(request) {}
  int lines = 1;
  int removeFromEnd = 0;
};


/// @brief 自定义 string
class TString {
 public:
  TString(int min_resize_width = TFixMax);
  TString(const QString &text,
          const TextParseOptions &options = TDefaultTextOptions,
          int min_resize_width = TFixMax);

  TString(const QString &text,
          const TextParseOptions &options = TDefaultTextOptions,
          int min_resize_width = TFixMax, const std::any &context = {});
  TString(TString &&other);
  TString &operator=(TString &&other);
  ~TString();

  [[nodiscard]] int countWidth(int width, bool break_every_where = false) const;
  [[nodiscard]] int countHeight(int width,
                                bool break_every_where = false) const;

  struct LineWidthsOptions {
    bool breakEveryWhere = false;
    int reserve = 0;
  };

  [[nodiscard]] std::vector<int> countLineWidths(int width) const;
  [[nodiscard]] std::vector<int> countLineWidths(
      int width, const LineWidthsOptions &opetions) const;

  struct DimensionsResult {
    int width = 0;
    int height = 0;
    std::vector<int> lineWidths;
  };

  struct DimensionsRequest {
    bool breakEverywhere = false;
    bool lineWidths = false;
    int reserve = 0;
  };

  [[nodiscard]] DimensionsResult countDimensions(
      GeometryDescriptor geometry) const;
  [[nodiscard]] DimensionsResult countDimensions(
      GeometryDescriptor geometry, DimensionsRequest request) const;

  void setText(const QString &text,
               const TextParseOptions &options = TDefaultTextOptions);

  void setMarkedText(const TextWithEntities &text_width_entities,
                     const TextParseOptions &options = TDefaultTextOptions,
                     const std::any &context = {});

  [[nodiscard]] bool hasLink();
  void setLink(uint16 index, const ClickHandlerPtr &lnk);

  [[nodiscard]] bool hasSpoilers() const;
  void setSpoilerRevealed(bool revealed, Animation::Type animated);
  void setSpoilerLinkFilter(std::function<bool(const ClickContext &)> filter);

  [[nodiscard]] bool hasCollapsedBlockquots() const;
  [[nodiscard]] bool blockquoteCollapsed(int index) const;
  [[nodiscard]] bool blockquoteExpanded(int index) const;
  void setBlockquoteExpanded(int index, bool expanded);
  void setBlockquoteExpandCallback(std::function<void(int index, bool expanded)> callback);

  [[nodiscard]] bool hasSkipBlock() const;
  bool updateSkipBlock(int width, int height);
  bool removeSkipBlock();

  [[nodiscard]] int maxWidth() const { return max_width_; }
  [[nodiscard]] int minHeight() const { return min_height_; }
  [[nodiscard]] int countMaxMonospaceWidth() const;

  [[nodiscard]] TextSelection adjustSelection(TextSelection selection,
                                              TextSelectType select_type) const;
  [[nodiscard]] bool isFullSelection(TextSelection selection) const;

	[[nodiscard]] bool isEmpty() const;
  [[nodiscard]] bool isNull() const;

  [[nodiscard]] int length() const;

  	[[nodiscard]] QString toString(
      TextSelection selection = AllTextSelection) const;
  [[nodiscard]] TextWithEntities toTextWithEntities(
      TextSelection selection = AllTextSelection) const;
  [[nodiscard]] TextForMimeData toTextForMimeData(
      TextSelection selection = AllTextSelection) const;



private:

  [[nodiscard]] TextForMimeData toText(TextSelection selection,
																			 bool composeExpanded,
																			 bool composeEntities) const;

  int min_resize_width_ = 0;
  int max_width_ = 0;
  int min_height_ = 0;
  uint16 start_quote_index_ = 0;

#if __cplusplus >= 202002L
  bool start_paragraph_LTR_ : 1 = false;
  bool start_paragraph_RTL_ : 1 = false;
  bool has_custom_emoji_ : 1 = false;
  bool is_isolated_emoji_ : 1 = false;
  bool is_only_custom_emoji_ : 1 = false;
  bool has_not_emoji_and_spaces_ : 1 = false;
  bool skip_block_added_new_line_ : 1 = false;
  bool ends_with_quote_ : 1 = false;

#else
  bool start_paragraph_LTR_ = false;
  bool start_paragraph_RTL_ = false;
  bool has_custom_emoji_ = false;
  bool is_isolated_emoji_ = false;
  bool is_only_custom_emoji_ = false;
  bool has_not_emoji_and_spaces_ = false;
  bool skip_block_added_new_line_1 = false;
  bool ends_with_quote_ = false;

  friend class Parser;
  friend class Renderer;

#endif



};

}  // namespace Text
}  // namespace Ui


#endif  // UI_TEXT_TEXT_H
