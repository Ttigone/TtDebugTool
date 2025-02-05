#include "ui/text/text.h"


namespace Ui {

namespace Text {
TString::TString(int min_resize_width)
{

}

TString::TString(const QString &text, const TextParseOptions &options, int min_resize_width)
{

}

TString::TString(const QString &text, const TextParseOptions &options, int min_resize_width, const std::any &context)
{

}

TString::TString(TString &&other)
{

}

TString &TString::operator=(TString &&other)
{
  return other;
}

TString::~TString()
{

}

void TString::setText(const QString& text, const TextParseOptions& options) {}

TextSelection TString::adjustSelection(TextSelection selection, TextSelectType select_type) const
{
  return TextSelection();
}

TextForMimeData TString::toTextForMimeData(TextSelection selection) const
{
  return toText(selection, true, true);

}

TextForMimeData TString::toText(TextSelection selection, bool composeExpanded, bool composeEntities) const
{
	return TextForMimeData();
}
}  // namespace Text
}  // namespace Ui
